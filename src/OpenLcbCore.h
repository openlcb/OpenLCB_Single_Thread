#ifndef OpenLcbCore_H
#define OpenLcbCore_H

#include "Event.h"
#include "OlcbCanInterface.h"

/**
 * Class for handling P/C Events.
 *
 * Maintains a single list of events, which could be
 * either produced or consumed (indicated by flags).
 *<p>
 * Basic state machine handles e.g. Request Producer/Consumer/event
 * messages for you, including at initialization.
 */
 
typedef struct {
    uint16_t offset;
    uint16_t flags;
} EIDTab;

typedef uint16_t Index;

class NodeID;
class LinkControl;
class Event;

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

class OpenLcbCore
{
public:
	OpenLcbCore(Event* events, uint16_t numEvents, uint16_t* eIndex, const EIDTab* eidTab, OlcbCanInterface* b, LinkControl* li);

	/**
	* Produce the ith event
	* 
	* Return true if done; return false to require retry later.
	* Note: It's an error to do this to an event that's not
	* marked as for production.
	*/
	void produce(int i);

	/**
	* Handle any routine processing that needs to be done.
	* Go through this in loop() to e.g. send pending messages
	*/
	void check();

	/**
	* When a CAN frame is received, it should
	* be transferred to the PCER object via this method
	* so that it can handle the verification protocol.
	*/
	bool receivedFrame(OlcbCanInterface* rcv);

	/**
	* A new event has been defined, and we should
	* do the necessary notifications, etc on the OpenLCB link
	*
	* index is the 0-based index of the newly defined
	* event in the array provided to the ctor earlier.
	*/
	void newEvent(int index, bool produce, bool consume);

	/**
	* Mark a particular slot to acquire the event 
	* from the next learn message.
	* true marks, false unmarks.
	*
	* index is the 0-based index of the desired
	* event in the array provided to the ctor earlier.
	*
	* ToDo: This doesn't force the non-volatile memory
	* to be stored after the learn message is received.
	*/
	void markToLearn(int index, bool mark);

	/**
	* Send a learn frame for a particular slot's event.
	*
	* index is the 0-based index of the desired
	* event in the array provided to the ctor earlier.
	*/
	void sendTeach(int index);
	void sendTeach(EventID e);
	bool isMarkedToLearn(int index);

	void processEvent(unsigned int eventIndex); 

	uint16_t getOffset(uint16_t index);
	uint16_t getFlags(uint16_t index);
	
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
	
	int16_t findIndexOfEventID(EventID *key, int16_t startIndex);
	Event* events; // array
	Index* eventsIndex; // array
	int numEvents;
	LinkControl* link;
	OlcbCanInterface* buffer;
	NodeID* nid;
	void handlePCEventReport(OlcbCanInterface* rcv);
	void handleLearnEvent(OlcbCanInterface* rcv);
	int sendEvent; // index of next identified event to send, or -1
};

extern OpenLcbCore OpenLcb;

#endif