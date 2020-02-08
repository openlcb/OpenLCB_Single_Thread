#include <Arduino.h>
#include <string.h>

#include "OpenLcbCore.h"
#include "NodeMemory.h"
#include "NodeID.h"
#include "Event.h"
#include "LinkControl.h"
#include "lib_debug_print_common.h"

extern "C" {
	uint16_t getOffset(uint16_t index);
	uint16_t getFlags(unsigned index);
	extern void writeEID(int index, EventID eid);
}

extern void userInitAll();
extern void pceCallback(unsigned int index)  __attribute__((weak));

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
  LDEBUG("\nNodeMemory::forceFactoryReset()");

  eventConfigState |= RESET_FACTORY_DEFAULTS;
  
  header.resetControl = RESET_FACTORY_DEFAULTS_VAL;
  NODECONFIG.put(0, header);
}

void OpenLcbCore::forceNewEventIDs() {
  LDEBUG("\nNodeMemory::forceNewEventIDs()");

  eventConfigState |= RESET_NEW_EVENTS;

  header.resetControl = RESET_NEW_EVENTS_VAL;
  NODECONFIG.put(0, header);
}


void OpenLcbCore::init()
{
  LDEBUG("\nNodeMemory::forceNewEventIDs()  State: "); LDEBUG2(eventConfigState, HEX); LDEBUGL();

	if(header.resetControl == RESET_NORMAL_VAL)
		eventConfigState |= RESET_NORMAL;
		
	else if(header.resetControl == RESET_NEW_EVENTS_VAL) 
		eventConfigState |= RESET_NEW_EVENTS;
		
	else 
		eventConfigState |= RESET_FACTORY_DEFAULTS;
		
	if(eventConfigState & RESET_NORMAL)
	{
	  LDEBUGL("NodeMemory: Reset Normal, Exiting ");

		return; // Nothing to do
	}
	
	else if(eventConfigState & RESET_NEW_EVENTS)
	{
	  LDEBUGL("NodeMemory: Reset New Events");

		writeNewEventIDs(events, numEvents);
		userInitAll();

		header.resetControl = RESET_NORMAL_VAL;
		NODECONFIG.put(0, header);
	}

	else if (eventConfigState & RESET_FACTORY_DEFAULTS)
	{	//clear EEPROM
	  LDEBUGL("NodeMemory: Reset Factory Defaults");

		header.nextEID = 0;
		nm.eraseAll();			
		// handle the rest
		writeNewEventIDs(events, numEvents);
		userInitAll();
		
		header.resetControl = RESET_NORMAL_VAL;
		NODECONFIG.put(0, header);
	}
}


// write to EEPROM new set of eventIDs and then magic, nextEID and nodeID
void OpenLcbCore::writeNewEventIDs(Event* events, uint8_t numEvents)
{
  LDEBUGL("\nNodeMemory::writeNewEventIDs()");
	EventID newEventId;
	
	NodeID myNodeId;
	nm.getNodeID(&myNodeId);
	
	newEventId.setNodeIdPrefix(&myNodeId);

	for(uint16_t e = 0; e < numEvents; e++)
	{
		newEventId.setEventIdSuffix(header.nextEID++);
		
		NODECONFIG.put(getOffset(e), newEventId);
	}
	// Save the latest value of nextEID
	NODECONFIG.put(0, header);
}



void OpenLcbCore::processEvent(unsigned int eventIndex)
{
	LDEBUG(F("\nOpenLcbCore::processEvent: Index")); LDEBUGL(eventIndex);
	if(pceCallback)
		pceCallback(eventIndex);
}

void OpenLcbCore::printEventIndexes()
{
	LDEBUG(F("\nprintEventIndex\n"));
	for(int i = 0; i < numEvents; i++)
	{
		LDEBUG2(eventsIndex[i],HEX); LDEBUG(F(", "));
	}
}

void OpenLcbCore::printEvents()
{
	LDEBUG(F("\nprintEvents "));
	LDEBUG(F("\n#  flags  EventID"));
	for(int i = 0; i < numEvents; i++)
	{
		LDEBUG("\n"); LDEBUG(i);
		LDEBUG(":"); LDEBUG2(getOffset(i),HEX);
		LDEBUG(F(" : ")); LDEBUG2(events[i].flags,HEX);
		LDEBUG(F(" : ")); events[i].eid.print();
	}

	LDEBUGL();
}
    
void OpenLcbCore::printEventids()
{
	LDEBUG("\neventids:");
	for(int e = 0; e < numEvents; e++)
	{
		LDEBUG("\n[");
		for(int i = 0; i < 8; i++)
			LDEBUG2(events[e].eid.val[i],HEX); LDEBUG(", ");
	}

	LDEBUGL();
}

void OpenLcbCore::printSortedEvents()
{
	LDEBUG("\nSorted events");
	for(int i = 0; i < numEvents; i++)
	{
		int e = eventsIndex[i];
		LDEBUG("\nEvent Index: "); LDEBUG(i);
		LDEBUG("  EventNum: ");    LDEBUG(e);
		LDEBUG("  EventID:"); 		 events[e].eid.print();
		LDEBUG("  Flags: ");       LDEBUG2(events[e].flags,HEX);
	}

	LDEBUGL();
}

// Compare function to compare two Event Index entries by comparing the EventIDs they point to
int OpenLcbCore::cmpfunc (const void * a, const void * b)
{
	uint16_t indexA = *(uint16_t*)a;
	uint16_t indexB = *(uint16_t*)b;
	Event * pEventA = &OpenLcb.events[indexA];
	Event * pEventB = &OpenLcb.events[indexB];
	int cmp = pEventB->eid.compare(&pEventA->eid);
	
// 	LDEBUG("\nCompare A: "); pEventA->eid.print();
// 	LDEBUG(" Compare B: ");  pEventB->eid.print();
// 	LDEBUG(" Result: ");
// 	LDEBUG(cmp);
	
	return cmp;
}

void OpenLcbCore::initTables()
{
	for(unsigned int e = 0; e < numEvents; e++)
	{
		eventsIndex[e] = e;
		NODECONFIG.get(getOffset(e), events[e].eid);
		events[e].flags |= getFlags(e);
	}
//     LDEBUG("\nSort eventIndex");
	qsort(eventsIndex, numEvents, sizeof(uint16_t), cmpfunc);
//     LDEBUG("\nSorted\n");
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
 
int16_t OpenLcbCore::findIndexOfEventID(EventID *key, int16_t startIndex)
{
	register int16_t base = 0;
	register int16_t lim;
	register int16_t p;
	register int16_t eventIndex;
	register int cmp;
	
		// First time called startIndex == -1
	if(startIndex == -1)
	{
// 		LDEBUG("\nfindIndexOfEventID: Begin")
		for (lim = numEvents; lim != 0; lim >>= 1)
		{
			p = base + (lim >> 1);
			eventIndex = eventsIndex[p];
			cmp = events[eventIndex].eid.compare(key);
// 			LDEBUG("\nCompare: p: "); LDEBUG(p); LDEBUG(" Key: "); key->print(); LDEBUG(" Event: "); events[eventIndex].eid.print(); LDEBUG(" cmp: "); LDEBUG(cmp);
			if (cmp == 0)
			{
				if(p == 0)
				{
// 					LDEBUG("\nMatch at beginning of list");
					return p;
				}	
				else
				{
						// Ok we have a match but step down the list checking for duplicates to find the lowest match
// 					LDEBUG("\nMatch. Step Down List Checking for Duplicates");
					while(p > 0)
					{
						eventIndex = eventsIndex[p - 1];
						uint8_t equal = events[eventIndex].eid.equals(key);
// 						LDEBUG("\nEqual: p - 1: "); LDEBUG(p - 1); LDEBUG(" Key: "); key->print(); LDEBUG(" Event: "); events[eventIndex].eid.print(); LDEBUG(" cmp: "); LDEBUG(equal);

							// if not equal then we already had the first match to return p
						if(!equal)
							return p ;

						p--;
// 						LDEBUG("\nCompare: Equal Step Down p: "); LDEBUGL(p);
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
                //LDEBUG(F("\nIn OpenLcbCore::check()"));
                //LDEBUG("\nEvent::CAN_PRODUCE_FLAG");
                //LDEBUG2(Event::CAN_PRODUCE_FLAG,HEX);
                //LDEBUG("\nEvent::CAN_CONSUME_FLAG");
                //LDEBUG2(Event::CAN_CONSUME_FLAG,HEX);
      
     while (sendEvent < numEvents) {
         // OK to send, see if marked for some cause
         // ToDo: This only sends _either_ producer ID'd or consumer ID'd, not both
         EventID ev = events[sendEvent].eid;
         
                //LDEBUG("\nOpenLcbCore::check "); LDEBUG(ev.val[7]);
                //LDEBUG(" I:"); LDEBUG(0!=(events[sendEvent].flags & Event::IDENT_FLAG));
                //LDEBUG(" cP:"); LDEBUG(0!=(events[sendEvent].flags & Event::CAN_PRODUCE_FLAG));
                //LDEBUG(" cC:"); LDEBUG(0!=(events[sendEvent].flags & Event::CAN_CONSUME_FLAG));
                //LDEBUG(" P:"); LDEBUG(0!=(events[sendEvent].flags & PRODUCE_FLAG));
                //LDEBUG(" E:"); LDEBUG(0!=(events[sendEvent].flags & EMPTY_FLAG));
         if ( (events[sendEvent].flags & (Event::IDENT_FLAG | Event::CAN_PRODUCE_FLAG)) == (Event::IDENT_FLAG | Event::CAN_PRODUCE_FLAG)) {
                    //LDEBUG(F("\nOpenLcbCore::check() produceIdent"));
           events[sendEvent].flags &= ~Event::IDENT_FLAG;    // reset flag
           buffer->setProducerIdentified(&ev);
           //OpenLcb_can_queue_xmt_wait(buffer);  // wait until buffer queued, but OK due to earlier check
           buffer->net->write(200);  // wait until buffer queued, but OK due to earlier check
                    //LDEBUG(F("\nOpenLcbCore::check() produceident2"));
           break; // only send one from this loop
         } else if ( (events[sendEvent].flags & (Event::IDENT_FLAG | Event::CAN_CONSUME_FLAG)) == (Event::IDENT_FLAG | Event::CAN_CONSUME_FLAG)) {
                    //LDEBUG(F("\nOpenLcbCore::check() consumeident"));
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
                    //LDEBUG(F("\nIn OpenLcbCore::checkLoop")); //while(0==0){}
     }
                    //LDEBUG(F("\nLeaving OpenLcbCore::check()")); //while(0==0){}
  }

  void OpenLcbCore::newEvent(int index, bool p, bool c) {
      //LDEBUG("\nnewEvent i=");
    events[index].flags |= Event::IDENT_FLAG;
    sendEvent = sendEvent < index ? sendEvent : index;
    if (p) events[index].flags |= Event::CAN_PRODUCE_FLAG;
    if (c) events[index].flags |= Event::CAN_CONSUME_FLAG;
      //LDEBUG(index); LDEBUG(" ");LDEBUG2(events[index].flags,HEX);
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
                //LDEBUG("\nIn receivedFrame");
                //LDEBUG("\nIn OpenLcbCore::receivedFrame()");
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
                        //LDEBUG("\nrcv->isPCEventReport!");
        handlePCEventReport(rcv);
    } else if (rcv->isLearnEvent()) {
        // found a teaching frame, apply to selected
        handleLearnEvent(rcv);
    } else return false;
    return true;
  }

  void OpenLcbCore::handlePCEventReport(OlcbCanInterface* rcv) {
//                 LDEBUG("\nIn handlePCEventReport");
      EventID eventid;
      rcv->getEventID(&eventid);
//                 LDEBUG("\nIn handlePCEventReport: ");eventid.print();
      // find matching eventID
      int index = -1;
      while((index = findIndexOfEventID(&eventid, index)) != -1)
      {
        uint16_t eindex = eventsIndex[index];
//                 LDEBUG("\nhandlePCRep index: "); LDEBUG(index);
//                 LDEBUG("\nhandlePCRep eindex: "); LDEBUG(eindex);
//                 LDEBUG("\numEvents[index].flags: "); LDEBUG2(events[index].flags,HEX);
        if (events[eindex].flags & Event::CAN_CONSUME_FLAG)
        {
//           LDEBUG("\nFound Consumer: Index: ");LDEBUG(index);
//           LDEBUG(", EIndex: ");LDEBUG(eindex);
          processEvent(eindex);
        }
        index++;
        if(index>=numEvents) break;
      }
  }

  void OpenLcbCore::handleLearnEvent(OlcbCanInterface* rcv) {
                //LDEBUG("\nIn OpenLcbCore::handleLearnEvent");
                //LDEBUG("\n rcv=");
                //for(int i=0;i<14;i++) { LDEBUG2( ((uint8_t*)rcv)[i],HEX ); LDEBUG(" "); }
        //bool save = false;
        EventID eid;
        rcv->getEventID(&eid);
                //LDEBUG("\neid:"); eid.print();
                //LDEBUG("\nEvent::LEARN_FLAG:"); LDEBUG2(Event::LEARN_FLAG,HEX);
        for (int i=0; i<numEvents; i++) {
                //LDEBUG("\ni:"); LDEBUG(i);
                //LDEBUG("\numEvents[i].flags:"); LDEBUG2(events[i].flags,HEX);
            if ( (events[i].flags & Event::LEARN_FLAG ) != 0 ) {
                //rcv->getEventID(events+i);
                //LDEBUG("\ni:"); LDEBUG(i);
                //LDEBUG("\neid.writeEID(i):");
                //LDEBUG2(i,HEX);
                writeEID(i, eid);
                events[i].flags |= Event::IDENT_FLAG; // notify new eventID
                events[i].flags &= ~Event::LEARN_FLAG; // enough learning
                sendEvent = sendEvent < i ? sendEvent : i;
                //save = true;
            }
        }
        // eeprom flagged dirty in writeEID()  ????
  }


