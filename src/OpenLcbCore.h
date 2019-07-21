#ifndef OpenLcbCore_H
#define OpenLcbCore_H

#include "Event.h"
#include "PCE.h"

class OpenLcbCore : public PCE
{
	public:
		OpenLcbCore(Event* events, int numEvents, uint16_t* eIndex, OlcbCanInterface* b, LinkControl* li);
		
		void processEvent(unsigned int eventIndex);
		
		void printEventIndexes();
		void printEvents();
		void printEventids();
		void printSortedEvents();
		
		void initTables();

	private:
		int static cmpfunc (const void * a, const void * b);

// 		MemStruct *pmem;
};

extern OpenLcbCore OpenLcb;

#endif