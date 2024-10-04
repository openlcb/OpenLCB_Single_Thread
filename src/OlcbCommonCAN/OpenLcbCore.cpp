#include <Arduino.h>
#include <string.h>

#include "OpenLcbCore.h"
#include "NodeMemory.h"
#include "NodeID.h"
#include "Event.h"
#include "LinkControl.h"

#include "debugging.h"


extern "C" {
	uint16_t getOffset(uint16_t index);
	uint16_t getFlags(uint16_t index);
	extern void writeEID(int index, EventID eid);
}
extern void setEepromDirty();
extern void userInitAll();
extern void pceCallback(uint16_t index)  __attribute__((weak));

OpenLcbCore::OpenLcbCore(Event* events, uint16_t numEvents, uint16_t* eIndex, const EIDTab* eidTab, OlcbCanInterface* b, LinkControl* li)
{
	eidOffsetsTable = eidTab;
	NODECONFIG.get(0, header);
	
	      //events = evts;
    this->events = events;
    eventsIndex = eIndex;
    this->numEvents = numEvents;
    buffer = b;
    link = li;
       
	// mark as needing transmit of IDs, otherwise not interesting
	// ToDo: Is this needed if requiring newEvent?
	for (int i = 0; i < numEvents; i++)
	{
	 if (events[i].flags & ( Event::CAN_PRODUCE_FLAG | Event::CAN_CONSUME_FLAG ))
		events[i].flags |= Event::IDENT_FLAG;
	}
	sendEvent = 0;
}
    
uint16_t OpenLcbCore::getOffset(uint16_t index)
{
    return pgm_read_word(&eidOffsetsTable[index].offset);
}

uint16_t OpenLcbCore::getFlags(uint16_t index)
{
    return pgm_read_word(&eidOffsetsTable[index].flags);
}

void OpenLcbCore::forceFactoryReset()
{
  //dP(F("\n >>>>OpenLcbCore::forceFactoryReset()"));
  eventConfigState |= RESET_FACTORY_DEFAULTS;
  
  header.resetControl = RESET_FACTORY_DEFAULTS_VAL;
  header.pack = 0;
  NODECONFIG.put(0, header);
  //dP(F("\ndirty Core factoryReset()"));
}

void OpenLcbCore::forceNewEventIDs() {
  dP(F("\n >>>>NodeMemory::forceNewEventIDs()"));
  eventConfigState |= RESET_NEW_EVENTS;

  header.resetControl = RESET_NEW_EVENTS_VAL;
  header.pack = 1;
  NODECONFIG.put(0, header);
}


void OpenLcbCore::init()
{
    //dP(F("\n >>>>OpenLcbCore::init()  State: "));
    //dPH(eventConfigState);
    NODECONFIG.get(0, header);

    //dP(F"\n header.resetControl = "); dPH(header.resetControl);

    if(header.resetControl == RESET_NORMAL_VAL) {
        eventConfigState |= RESET_NORMAL;
        //dP(F("\nRESET_NORMAL"));
    }
		
    else if(header.resetControl == RESET_NEW_EVENTS_VAL) {
        eventConfigState |= RESET_NEW_EVENTS;
        //dP(F("\nRESET_NEW_EVENTS"));
    }
		
    else {
        eventConfigState |= RESET_FACTORY_DEFAULTS;
        //dP(F("\nRESET_FACTORY_DEFAULTS"));
    }
    
	if(eventConfigState & RESET_NORMAL)
	{
        //dP(F("\n OpenLcbCore::init: Reset Normal, Exiting "));
		return; // Nothing to do
	}
    
	else if(eventConfigState & RESET_NEW_EVENTS)
	{
        //dP(F("\nOpenLcbCore::init: Reset New Events"));
        //dP(F("\nwriteNewEventIDs(events, numEvents)"));
        writeNewEventIDs(events, numEvents);
        dP(F("\nuserInitAll()"));

        userInitAll();

		header.resetControl = RESET_NORMAL_VAL;
        //dP(F("\nNODECONFIG.put(0, header)"));

        NODECONFIG.put(0, header);
        setEepromDirty();
	}
	else if (eventConfigState & RESET_FACTORY_DEFAULTS)
    {	//clear EEPROM
        //dP(F("\nOpenLcbCore::init: Reset Factory Defaults"));
        //dP(F("\nnextEID=0"));
        header.nextEID = 0;
        //dP(F("\nnm.eraseAll"));
        nm.eraseAll();
        // handle the rest
        //dP(F("\nwriteNewEventIDs(events, numEvents)"));
        writeNewEventIDs(events, numEvents);
        dP(F("\nuserInitAll()"));
        userInitAll();

        header.resetControl = RESET_NORMAL_VAL;
        //dP(F("\nNODECONFIG.put(0, header)"));
        NODECONFIG.put(0, header);
        setEepromDirty();
    }
}


// write to EEPROM new set of eventIDs and then magic, nextEID and nodeID
void OpenLcbCore::writeNewEventIDs(Event* events, uint8_t numEvents)
{
    //dP(F("\nOpenLcbCore::init::writeNewEventIDs())"));
	EventID newEventId;
	
	NodeID myNodeId;
	nm.getNodeID(&myNodeId);
	
	newEventId.setNodeIdPrefix(&myNodeId);

	for(uint16_t e = 0; e < numEvents; e++)
	{
        //dP(F("\n eid")); dPH(header.nextEID);
		newEventId.setEventIdSuffix(header.nextEID++);
        NODECONFIG.put(getOffset(e), newEventId);
	}
	// Save the latest value of nextEID
	NODECONFIG.put(0, header);
    setEepromDirty();
}

void OpenLcbCore::processEvent(uint16_t eventIndex)
{
    //dP(F("\nOpenLcbCore::processEvent: Index")); dP((uint16_t)eventIndex);
    if(pceCallback) {
        pceCallback(eventIndex);
    }
}

void OpenLcbCore::printEventIndexes()
{
    dP(F("\nprintEventIndex\n"));
	for(int i = 0; i < numEvents; i++)
	{
        dPH(eventsIndex[i]); dP(", \n");
	}
}

void OpenLcbCore::printEvents()
{
    dP(F("\nprintEvents "));
    dP(F("\n#  flags  EventID"));
	for(uint16_t i = 0; i < numEvents; i++)
	{
        dP("\n"); dP(i);
        dP(":"); dPH(getOffset(i));
        dP(" : "); dPH(events[i].flags);
        dP(" : "); events[i].eid.print();
	}
}
    
void OpenLcbCore::printEventids()
{
    dP(F("\neventids:"));
	for(uint16_t e = 0; e < numEvents; e++)
	{
        dP("\n[");
		for(int i = 0; i < 8; i++)
        {
            dPH(events[e].eid.val[i]);
            dP(", ");
        }
	}
}

void OpenLcbCore::printSortedEvents()
{
    dP(F("\nSorted events"));
	for(uint16_t i = 0; i < numEvents; i++)
	{
		uint16_t e = eventsIndex[i];
        dP(F("\nEvent Index: ")); dP(i);
        dP(F("  EventNum: "));    dP(e);
        dP(F("  EventID:"));          events[e].eid.print();
        dP(F("  Flags: "));       dPH(events[e].flags);
	}
}

// Compare function to compare two Event Index entries by comparing the EventIDs they point to
int OpenLcbCore::cmpfunc (const void * a, const void * b)
{
	uint16_t indexA = *(uint16_t*)a;
	uint16_t indexB = *(uint16_t*)b;
	Event * pEventA = &OpenLcb.events[indexA];
	Event * pEventB = &OpenLcb.events[indexB];
	int cmp = pEventB->eid.compare(&pEventA->eid);
	
    //dP("\nCompare A: "); pEventA->eid.print();
    //dP(" Compare B: ");  pEventB->eid.print();
    //dP(" Result: ");
    //dP(cmp);

	return cmp;
}

void OpenLcbCore::initTables()
{
    //dP(F("\nOpenLcbCore::initTables()"));
	for(int e = 0; e < numEvents; e++)
	{
		eventsIndex[e] = e;
		NODECONFIG.get(getOffset(e), events[e].eid);
		events[e].flags |= getFlags(e);
	}
	qsort(eventsIndex, numEvents, sizeof(uint16_t), cmpfunc);
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

// Alternate, slightly more efficient algorithm.
// For explanation see https://en.wikipedia.org/wiki/Binary_search
// function binary_search_alternative(A, n, T) is
//    L := 0
//    R := n − 1
//    while L != R do
//        m := ceil((L + R) / 2)
//        if A[m] > T then  <<== Note, no == comparison, until the end!
//            R := m − 1
//        else:
//            L := m
//    if A[L] = T then
//        return L
//    return unsuccessful

// implement findIndexOfEventID based on avr-libc bsearch() code above
 
int16_t OpenLcbCore::findIndexOfEventID(EventID *key, int16_t startIndex)
{
    int16_t base = 0;
    int16_t lim;
    int16_t p;
    int16_t eventIndex;
    int cmp;
	
    // First time called startIndex == -1
	if(startIndex == -1)
	{
        //dP(F"\nfindIndexOfEventID: Begin"))
		for (lim = numEvents; lim != 0; lim >>= 1)
		{
			p = base + (lim >> 1);
			eventIndex = eventsIndex[p];
			cmp = events[eventIndex].eid.compare(key);
            //dP(F("\nCompare: p: ")); dP(p); dP(F(" Key: ")); key->print(); dP(F(" Event: ")); events[eventIndex].eid.print(); dP(F(" cmp: ")); dP(cmp);
			if (cmp == 0)
			{
				if(p == 0)
				{
//                  dP(F("\nMatch at beginning of list"));
					return p;
				}	
				else
				{
                    // Ok we have a match but step down the list checking for duplicates to find the lowest match
                    //dP(F("\nMatch. Step Down List Checking for Duplicates"));
					while(p > 0)
					{
                        eventIndex = eventsIndex[p - 1];
                        uint8_t equal = events[eventIndex].eid.equals(key);
                        //P(F("\nEqual: p - 1: ")); dP(p - 1); dP(F(" Key: ")); key->print();
                          dP(F(" Event: ")); events[eventIndex].eid.print();
                          dP(F(" cmp: ")); dP(equal);

                        // if not equal then we already had the first match to return p
                        if(!equal)
                            return p ;

                        p--;
                        //dP(F("\nCompare: Equal Step Down p: ")); dP(p);
					}
					
                    // If we get here we're at the beginning of the list so return p
					if(p == 0)
						return p;
				}
			}	
			if (cmp > 0)
			{
				base = p + 1;
				lim--;
			}
		}
        // return -p;
		return -1;
	}
	
    // Already had a match so check the next entry in case there are duplicates
	else
	{
        // If a duplicate match then return startIndex
		if(events[eventsIndex[startIndex]].eid.equals(key))
			return startIndex;
        // Not a duplicate
		return -1;
	}
}

 
  void OpenLcbCore::produce(int i) {
    // ignore if not producer
    if ((events[i].flags & Event::CAN_PRODUCE_FLAG) == 0) return;
    // mark for production
    events[i].flags |= Event::PRODUCE_FLAG;
    sendEvent = sendEvent<i ? sendEvent : i;
  }

  void OpenLcbCore::check() {
     // see in any replies are waiting to send
     while (sendEvent < numEvents) {
         
         // Throttling
         static long nextcheck = 0;
         if( (millis()-nextcheck) <0 ) return;
         nextcheck = millis() + 50;
         
         // OK to send, see if marked for some cause
         // ToDo: This only sends _either_ producer ID'd or consumer ID'd, not both
         EventID ev = events[sendEvent].eid;
                //dP("\nOpenLcbCore::check "); dP(ev.val[7]);
                //dP(" I:"); dP(0!=(events[sendEvent].flags & Event::IDENT_FLAG));
                //dP(" cP:"); dP(0!=(events[sendEvent].flags & Event::CAN_PRODUCE_FLAG));
                //dP(" cC:"); dP(0!=(events[sendEvent].flags & Event::CAN_CONSUME_FLAG));
                //dP(" P:"); dP(0!=(events[sendEvent].flags & PRODUCE_FLAG));
                //dP(" E:"); dP(0!=(events[sendEvent].flags & EMPTY_FLAG));
         if ( (events[sendEvent].flags & (Event::IDENT_FLAG | Event::CAN_PRODUCE_FLAG)) == (Event::IDENT_FLAG | Event::CAN_PRODUCE_FLAG)) {
                //dP(F("\nOpenLcbCore::check() produceIdent"));
           events[sendEvent].flags &= ~Event::IDENT_FLAG;    // reset flag
           buffer->setProducerIdentified(&ev);
           //OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           buffer->net->write(200);  // wait until buffer queued, but OK due to earlier check
             //dP(F("\nOpenLcbCore::check() produceident2"));
           break; // only send one from this loop
         } else if ( (events[sendEvent].flags & (Event::IDENT_FLAG | Event::CAN_CONSUME_FLAG)) == (Event::IDENT_FLAG | Event::CAN_CONSUME_FLAG)) {
             //dP(F("\nOpenLcbCore::check() consumeident"));
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
         //dP(F("\nIn OpenLcbCore::checkLoop")); //while(1);
     }
     //dP(F("\nLeaving OpenLcbCore::check()")); //while(1);
  }

  void OpenLcbCore::newEvent(int index, bool p, bool c) {
      //dP("\nnewEvent i=");
    events[index].flags |= Event::IDENT_FLAG;
    sendEvent = sendEvent < index ? sendEvent : index;
    if (p) events[index].flags |= Event::CAN_PRODUCE_FLAG;
    if (c) events[index].flags |= Event::CAN_CONSUME_FLAG;
    setEepromDirty();
  }
  
  void OpenLcbCore::markToLearn(int index, bool mark) {
    if (mark)
        events[index].flags |= Event::LEARN_FLAG;
    else
        events[index].flags &= ~Event::LEARN_FLAG;
  }


bool OpenLcbCore::isMarkedToLearn(int index) {
	return events[index].flags==Event::LEARN_FLAG;
}

void OpenLcbCore::sendTeach(EventID e) {   /// DPH added for Clock
	buffer->setLearnEvent(&e);
	handleLearnEvent(buffer);
    buffer->net->write(200);  // wait until buffer queued, but OK due to earlier check
}


  void OpenLcbCore::sendTeach(int index) {
    events[index].flags |= Event::TEACH_FLAG;
    sendEvent = sendEvent < index ? sendEvent : index;
  }
  
  bool OpenLcbCore::receivedFrame(OlcbCanInterface* rcv) {
      //dP("\nIn receivedFrame");
      //dP("\nIn OpenLcbCore::receivedFrame()");
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
          if(index>=numEvents) break;
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
          if(index>=numEvents) break;
        }
        // ToDo: add identify flags so that events that are both produced and consumed
        // have only one form sent in response to a specific request.
    } else if (rcv->isIdentifyEvents()) {
        // if so, send _all_ ProducerIdentified, ConsumerIdentified
        // via the "check" periodic call
        for (int i = 0; i < numEvents; i++) {
          events[i].flags |= Event::IDENT_FLAG;
        }
        sendEvent = 0;  
    } else if (rcv->isPCEventReport()) {
        // found a PC Event Report, see if we consume it
        handlePCEventReport(rcv);
    } else if (rcv->isLearnEvent()) {
        // found a teaching frame, apply to selected
        handleLearnEvent(rcv);
    } else return false;
    return true;
  }

  void OpenLcbCore::handlePCEventReport(OlcbCanInterface* rcv) {
//                 dP("\nIn handlePCEventReport");
      EventID eventid;
      rcv->getEventID(&eventid);
//                 dP("\nIn handlePCEventReport: ");eventid.print();

      // find matching eventID
      int index = -1;
      while((index = findIndexOfEventID(&eventid, index)) != -1)
      {
        uint16_t eindex = eventsIndex[index];
//                 dP("\nhandlePCRep index: "); dP(index);
//                 dP("\nhandlePCRep eindex: "); dP(eindex);
//                 dP("\numEvents[index].flags: "); dPH(events[index].flags);
          if (events[eindex].flags & Event::CAN_CONSUME_FLAG)
        {
//           dP("\nFound Consumer: Index: ");dP(index);
//           dP(", EIndex: ");dP(eindex);
          processEvent(eindex);
        }
        index++;
        if(index>=numEvents) break;
      }
  }

  void OpenLcbCore::handleLearnEvent(OlcbCanInterface* rcv) {
            //dP("\nIn OpenLcbCore::handleLearnEvent");
            //dP("\n rcv=");
            //for(int i=0;i<14;i++) { dPH( ((uint8_t*)rcv)[i] ); dP(" "); }
        //bool save = false;
        EventID eid;
        rcv->getEventID(&eid);
            //dP("\neid:"); eid.print();
            //dP("\nEvent::LEARN_FLAG:"); dPH(Event::LEARN_FLAG);
        for (int i=0; i<numEvents; i++) {
                //dP("\ni:"); dP(i);
                //dP("\numEvents[i].flags:"); dPH(events[i].flags);
            if ( (events[i].flags & Event::LEARN_FLAG ) != 0 ) {
                //rcv->getEventID(events+i);
                //dP("\ni:"); dP(i);
                //dP("\neid.writeEID(i):"); dPH(i);
                writeEID(i, eid);
                events[i].flags |= Event::IDENT_FLAG; // notify new eventID
                events[i].flags &= ~Event::LEARN_FLAG; // enough learning
                sendEvent = sendEvent < i ? sendEvent : i;
                //save = true;
            }
        }
  }


