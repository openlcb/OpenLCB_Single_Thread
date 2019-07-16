#include <string.h>

#include "PCE.h"

#include "NodeID.h"
#include "EventID.h"
#include "Event.h"
#include "LinkControl.h"
#include "OlcbCanInterface.h"
#include "lib_debug_print_common.h"


extern "C" {
    extern void writeEID(int index);
}

// avr-libc bsearch source code 
// void *
// bsearch(register const void *key, const void *base0, size_t nmemb,
//         register size_t size, register int (*compar)(const void *, const void *))
// {
// 	register const char *base = base0;
// 	register size_t lim;
// 	register int cmp;
// 	register const void *p;
// 
// 	for (lim = nmemb; lim != 0; lim >>= 1) {
// 		p = base + (lim >> 1) * size;
// 		cmp = (*compar)(key, p);
// 		if (cmp == 0)
// 			return ((void *)p);
// 		if (cmp > 0) {	/* key > p: move right */
// 			base = (char *)p + size;
// 			lim--;
// 		}		/* else move left */
// 	}
// 	return (NULL);
// }

// implement findIndexOfEventID based on avr-libc bsearch() code above
 
int16_t PCE::findIndexOfEventID(EventID *key, int16_t startIndex)
{
	register int16_t base = 0;
	register int16_t lim;
	register int16_t p;
	register int cmp;
	
		// First time called startIndex == -1
	if(startIndex == -1)
	{
		for (lim = nEvents; lim != 0; lim >>= 1)
		{
			p = base + (lim >> 1);
			cmp = events[p].eid.compare(key);
			if (cmp == 0)
			{
				// Ok we have a match but step down the list checking for duplicates to find the first match
				while(p > 0)
				{
						// if not equal then we we have the first match
					if(!events[p - 1].eid.equals(key));
						return p;
					p--;
				}
			}	
			if (cmp > 0)
			{
				base = p + 1;
				lim--;
			}
		}
		return -1;
	}
	
		// Already had a match so check the next entry in case there are duplicates 
	else
	{
			// If a duplicate match then return startIndex
		if(events[startIndex].eid.equals(key))
			return startIndex;
			
			// Not a duplicate
		return -1;
	}
}

PCE::PCE(Event* evts, int nEvt, uint16_t* eIndex, OlcbCanInterface* b, LinkControl* li)
{
      //events = evts;
    events = evts;
    eventsIndex = eIndex;
    nEvents = nEvt;
    buffer = b;
    link = li;
       
      // mark as needing transmit of IDs, otherwise not interesting
      // ToDo: Is this needed if requiring newEvent?
      for (int i = 0; i < nEvents; i++) {
         if (events[i].flags & ( Event::CAN_PRODUCE_FLAG | Event::CAN_CONSUME_FLAG ))
            events[i].flags |= Event::IDENT_FLAG;
      }
      sendEvent = 0;
  }
  
  void PCE::produce(int i) {
    // ignore if not producer
    if ((events[i].flags & Event::CAN_PRODUCE_FLAG) == 0) return;
    // mark for production
    events[i].flags |= Event::PRODUCE_FLAG;
    sendEvent = sendEvent<i ? sendEvent : i;
  }

  void PCE::check() {
     // see in any replies are waiting to send
                //LDEBUG(F("\nIn PCE::check()"));
                //LDEBUG("\nEvent::CAN_PRODUCE_FLAG");
                //LDEBUG2(Event::CAN_PRODUCE_FLAG,HEX);
                //LDEBUG("\nEvent::CAN_CONSUME_FLAG");
                //LDEBUG2(Event::CAN_CONSUME_FLAG,HEX);
      
     while (sendEvent < nEvents) {
         // OK to send, see if marked for some cause
         // ToDo: This only sends _either_ producer ID'd or consumer ID'd, not both
         EventID ev = events[sendEvent].eid;
         
                //LDEBUG("\nPCE::check "); LDEBUG(ev.val[7]);
                //LDEBUG(" I:"); LDEBUG(0!=(events[sendEvent].flags & Event::IDENT_FLAG));
                //LDEBUG(" cP:"); LDEBUG(0!=(events[sendEvent].flags & Event::CAN_PRODUCE_FLAG));
                //LDEBUG(" cC:"); LDEBUG(0!=(events[sendEvent].flags & Event::CAN_CONSUME_FLAG));
                //LDEBUG(" P:"); LDEBUG(0!=(events[sendEvent].flags & PRODUCE_FLAG));
                //LDEBUG(" E:"); LDEBUG(0!=(events[sendEvent].flags & EMPTY_FLAG));
         if ( (events[sendEvent].flags & (Event::IDENT_FLAG | Event::CAN_PRODUCE_FLAG)) == (Event::IDENT_FLAG | Event::CAN_PRODUCE_FLAG)) {
                    //LDEBUG(F("\nPCE::check() produceIdent"));
           events[sendEvent].flags &= ~Event::IDENT_FLAG;    // reset flag
           buffer->setProducerIdentified(&ev);
           //OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           buffer->net->write(200);  // wait until buffer queued, but OK due to earlier check
                    //LDEBUG(F("\nPCE::check() produceident2"));
           break; // only send one from this loop
         } else if ( (events[sendEvent].flags & (Event::IDENT_FLAG | Event::CAN_CONSUME_FLAG)) == (Event::IDENT_FLAG | Event::CAN_CONSUME_FLAG)) {
                    //LDEBUG(F("\nPCE::check() consumeident"));
           events[sendEvent].flags &= ~Event::IDENT_FLAG;    // reset flag
           buffer->setConsumerIdentified(&ev);
           //OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           buffer->net->write(200);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else if (events[sendEvent].flags & Event::PRODUCE_FLAG) {
           events[sendEvent].flags &= ~Event::PRODUCE_FLAG;    // reset flag
           buffer->setPCEventReport(&ev);
           handlePCEventReport(buffer);
             //OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           buffer->net->write(200);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else if (events[sendEvent].flags & Event::TEACH_FLAG) {
           events[sendEvent].flags &= ~Event::TEACH_FLAG;    // reset flag
           buffer->setLearnEvent(&ev);
           handleLearnEvent(buffer);
           //OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           buffer->net->write(200);  // wait until buffer queued, but OK due to earlier check
           break; // only send one from this loop
         } else {
           // just skip
           sendEvent++;
         }
                    //LDEBUG(F("\nIn PCE::checkLoop")); //while(0==0){}
     }
                    //LDEBUG(F("\nLeaving PCE::check()")); //while(0==0){}
  }

  void PCE::newEvent(int index, bool p, bool c) {
      //LDEBUG("\nnewEvent i=");
    events[index].flags |= Event::IDENT_FLAG;
    sendEvent = sendEvent < index ? sendEvent : index;
    if (p) events[index].flags |= Event::CAN_PRODUCE_FLAG;
    if (c) events[index].flags |= Event::CAN_CONSUME_FLAG;
      //LDEBUG(index); LDEBUG(" ");LDEBUG2(events[index].flags,HEX);
  }
  
  void PCE::markToLearn(int index, bool mark) {
    if (mark)
        events[index].flags |= Event::LEARN_FLAG;
    else
        events[index].flags &= ~Event::LEARN_FLAG;
  }


bool PCE::isMarkedToLearn(int index) {
	return events[index].flags==Event::LEARN_FLAG;
}

void PCE::sendTeach(EventID e) {   /// DPH added for Clock
	buffer->setLearnEvent(&e);
	handleLearnEvent(buffer);
    buffer->net->write(200);  // wait until buffer queued, but OK due to earlier check
}


  void PCE::sendTeach(int index) {
    events[index].flags |= Event::TEACH_FLAG;
    sendEvent = sendEvent < index ? sendEvent : index;
  }
  
  bool PCE::receivedFrame(OlcbCanInterface* rcv) {
                //LDEBUG("\nIn receivedFrame");
                //LDEBUG("\nIn PCE::receivedFrame()");
    EventID eventid;
    if (rcv->isIdentifyConsumers()) {
        // see if we consume the listed event
        //Event event;
        rcv->getEventID(&eventid);
        int index = -1; //
        // find consumers of event
        while((index = findIndexOfEventID(&eventid, index)) != -1)
        {
          // yes, we have to reply with ConsumerIdentified
          if (events[index].flags & Event::CAN_CONSUME_FLAG) {
             events[index].flags |= Event::IDENT_FLAG;
             sendEvent = sendEvent < index ? sendEvent : index;
          }
          index++;
          if(index>=nEvents) break;
        }
    } else if (rcv->isIdentifyProducers()) {
        // see if we produce the listed event
        EventID eventid;
        rcv->getEventID(&eventid);
        int index = -1;
        // find producers of event
        while((index = findIndexOfEventID(&eventid, index)) != -1)
        {
          // yes, we have to reply with ProducerIdentified
          if (events[index].flags & Event::CAN_PRODUCE_FLAG) {
             events[index].flags |= Event::IDENT_FLAG;
             sendEvent = sendEvent < index ? sendEvent : index;
          }
          index++;
          if(index>=nEvents) break;
        }
        // ToDo: add identify flags so that events that are both produced and consumed
        // have only one form sent in response to a specific request.
    } else if (rcv->isIdentifyEvents()) {
        // if so, send _all_ ProducerIdentified, ConsumerIdentified
        // via the "check" periodic call
        for (int i = 0; i < nEvents; i++) {
          events[i].flags |= Event::IDENT_FLAG;
        }
        sendEvent = 0;  
    } else if (rcv->isPCEventReport()) {
        // found a PC Event Report, see if we consume it
                        //LDEBUG("\nrcv->isPCEventReport!");
        handlePCEventReport(rcv);
    } else if (rcv->isLearnEvent()) {
        // found a teaching frame, apply to selected
        handleLearnEvent(rcv);
    } else return false;
    return true;
  }

  void PCE::handlePCEventReport(OlcbCanInterface* rcv) {
                //LDEBUG("\nIn handlePCEventReport");
      EventID eventid;
      rcv->getEventID(&eventid);
//                 LDEBUG("\nIn handlePCEventReport: ");eventid.print();
      // find matching eventID
      int index = -1;
			while((index = findIndexOfEventID(&eventid, index)) != -1)
			{
        uint16_t eindex = eventsIndex[index];
                //LDEBUG("\nhandlePCRep ind: "); LDEBUG(ind);
                //LDEBUG("\nhandlePCRep Index: "); LDEBUG(index);
                //LDEBUG("\nevents[index].flags: "); LDEBUG2(events[index].flags,HEX);
                //LDEBUG("\nhandlePCRep index: "); LDEBUG(index);
                //LDEBUG("\nhandlePCRep eindex: "); LDEBUG(eindex);
                //LDEBUG("\nevents[index].flags: "); LDEBUG2(events[index].flags,HEX);
        if (events[eindex].flags & Event::CAN_CONSUME_FLAG)
        {
                //LDEBUG(F("Found Consumer: Index: ")); LDEBUG(index);
                //LDEBUG("\nFound Consumer: Index: ");LDEBUG(index);
                //LDEBUG(", EIndex: ");LDEBUG(eindex);
          processEvent(eindex);
        }
        index++;
        if(index>=nEvents) break;
      }
  }

  void PCE::handleLearnEvent(OlcbCanInterface* rcv) {
                //LDEBUG("\nIn PCE::handleLearnEvent");
                //LDEBUG("\n rcv=");
                //for(int i=0;i<14;i++) { LDEBUG2( ((uint8_t*)rcv)[i],HEX ); LDEBUG(" "); }
        //bool save = false;
        EventID eid;
        rcv->getEventID(&eid);
                //LDEBUG("\neid:"); eid.print();
                //LDEBUG("\nEvent::LEARN_FLAG:"); LDEBUG2(Event::LEARN_FLAG,HEX);
        for (int i=0; i<nEvents; i++) {
                //LDEBUG("\ni:"); LDEBUG(i);
                //LDEBUG("\nevents[i].flags:"); LDEBUG2(events[i].flags,HEX);
            if ( (events[i].flags & Event::LEARN_FLAG ) != 0 ) {
                //rcv->getEventID(events+i);
                //LDEBUG("\ni:"); LDEBUG(i);
                //LDEBUG("\neid.writeEID(i):");
                //LDEBUG2(i,HEX);
                writeEID(i);
                events[i].flags |= Event::IDENT_FLAG; // notify new eventID
                events[i].flags &= ~Event::LEARN_FLAG; // enough learning
                sendEvent = sendEvent < i ? sendEvent : i;
                //save = true;
            }
        }
        // eeprom flagged dirty in writeEID()  ????
  }

