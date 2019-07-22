#ifndef OpenLcbCore_H
#define OpenLcbCore_H

#include "Event.h"
#include "PCE.h"

// ===== eventidOffset Support =====================================
//   Note: this allows system routines to initialize event[]
//         since eventOffsets are constant in flash.

typedef struct {
    uint16_t offset;
    uint16_t flags;
} EIDTab;

#define RESET_NORMAL_VAL			0xEE
#define RESET_NEW_EVENTS_VAL		0x33
#define RESET_FACTORY_DEFAULTS_VAL	0xFF

#define RESET_NORMAL				0x01
#define RESET_NEW_EVENTS			0x02
#define RESET_FACTORY_DEFAULTS		0x04

typedef struct
{
	uint16_t	nextEID;
	uint8_t		resetControl;
} EVENT_SPACE_HEADER;

class OpenLcbCore : public PCE
{
	public:
		OpenLcbCore(Event* events, int numEvents, uint16_t* eIndex, const EIDTab* eidTab, OlcbCanInterface* b, LinkControl* li);

		uint16_t getOffset(uint16_t index);
		uint16_t getFlags(unsigned index);
		
		/**
		* Make sure ready to go.  NodeID should have a default
		* value already in case this is the first time.
		*
		* events is address in RAM to load from EEPROM
		* copy N=extraBytes of memory from end of EEPROM to RAM.
		*
		* If the EEPROM is corrupt, all that is reloaded after  
		* N=clearBytes of memory is cleared to e.g. 
		* clear name strings. This count EEPROM address 0 (e.g. _not_ starting
		* at end of event strings; this is the full memory clear)   
		*/
		void init();

		/** 
		* For debug and test, this forces a reset to factory defaults at next restart
		*/
		void forceFactoryReset();

		/** 
		* Reload a complete set of new events on next restart.
		*/
		void forceNewEventIDs();
    
		/*
		* Get a new, forever unique EventID and put in 
		* given EventID location. Does not do a EEPROM store,
		* which must be done separately. unique ID build using
		* this node's nodeID. 
		*/
		//void setToNewEventID(NodeID* nodeID, EventID* eventID);
		void setToNewEventID(NodeID* nid, uint16_t eOff);
		void setToNewEventID(NodeID* nid, int n);  //dph
		
		void processEvent(unsigned int eventIndex);
		
		void printEventIndexes();
		void printEvents();
		void printEventids();
		void printSortedEvents();
		
		void initTables();

	private:
		int static cmpfunc (const void * a, const void * b);
		void writeNewEventIDs(Event* events, uint8_t numEvents);

		uint8_t	eventConfigState;
		const EIDTab* eidOffsetsTable;
		EVENT_SPACE_HEADER	header;
};

extern OpenLcbCore OpenLcb;

#endif