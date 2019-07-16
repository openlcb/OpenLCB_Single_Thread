#include <Arduino.h>
#include <EEPROM.h>

#include "OpenLcbCore.h"
#include "lib_debug_print_common.h"
#include "Event.h"

extern uint16_t getOffset(uint16_t index);
extern uint16_t getFlags(unsigned index);

OpenLcbCore::OpenLcbCore(Event* events, int numEvents, uint16_t* eIndex, OlcbCanInterface* b, LinkControl* li)
	: PCE(events, numEvents, eIndex, b, li)
{
}

void OpenLcbCore::processEvent(unsigned int eventIndex)
{
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
		EEPROM.get(getOffset(e), events[e].eid);
		events[e].flags |= getFlags(e);
	}
//     LDEBUG("\nSort eventIndex");
	qsort(eventsIndex, numEvents, sizeof(uint16_t), cmpfunc);
//     LDEBUG("\nSorted\n");
}
