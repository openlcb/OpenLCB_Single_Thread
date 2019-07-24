#include <Arduino.h>
#include <EEPROM.h>

#include "OpenLcbCore.h"
#include "lib_debug_print_common.h"
#include "Event.h"
#include "NodeMemory.h"

extern "C" {
	uint16_t getOffset(uint16_t index);
	uint16_t getFlags(unsigned index);
	extern void writeEID(int index);
}

extern void userInitAll();
extern void pceCallback(unsigned int index)  __attribute__((weak));

OpenLcbCore::OpenLcbCore(Event* events, uint16_t numEvents, uint16_t* eIndex, const EIDTab* eidTab, OlcbCanInterface* b, LinkControl* li)
	: PCE(events, numEvents, eIndex, b, li)
{
	eidOffsetsTable = eidTab;
	NODECONFIG.get(0, header);
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
