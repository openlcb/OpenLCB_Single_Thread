#ifndef NodeMemory_h
#define NodeMemory_h

/**
 * Class for persisting node memory
 * in a non-volative memory, e.g. EEPROM.
 *
 * This class loads from and stores to EEPROM
 * a NodeID and two arrays of Events; it's up to you 
 * to store everything else.
 * 
 * The memory can either be 
 * blank, needing complete initialization or
 * OK at startup. Call setup(...) with a valid NodeID
 * to either load or create EventIDs.
 * 
 * If you change those, call store(...) when done.
 * 
 * When you "reset" the memory, you're 
 * putting _new_ unique EventIDs in place.
 *
 * The first four bytes of memory contain flag values:
 * 0xEE 0x55 0x5E 0xE5 - all memory valid, use as is
 * 0xEE 0x55 0x33 0xCC - Node ID valid, rest must be initialized
 * Any other flag means no memory valid.
 *
 * TODO: Add a "dirty" bit to make store logic easier for external code?
 *
 */
#include <stdint.h>
//#include "Event.h"
#include "NodeID.h"

#define RESET_NORMAL_VAL						0xEE55
#define RESET_NEW_EVENTS_VAL				0x33CC
#define RESET_FACTORY_DEFAULTS_VAL	0xFFFF

#define RESET_NORMAL						0x01
#define RESET_NEW_EVENTS				0x02
#define RESET_FACTORY_DEFAULTS	0x03

#define NODE_ID_MARKER_VALID		0x10
#define NODE_ID_CHECKSUM_VALID	0x20
#define NODE_ID_OK (NODE_ID_MARKER_VALID | NODE_ID_CHECKSUM_VALID)

#define NODE_ID_MARKER					'N'

typedef struct
{
	uint8_t 	nodeIdMarker;
	NodeID 		nodeId;
	uint8_t 	nodeIdCheckSum;
} NODE_ID_STORE;

typedef struct
{
	NODE_ID_STORE 	nodeId;
	uint16_t				nextEID;
} NODE_HEADER;

class NodeID;
class Event;
class EventID;

class NodeMemory {
  public:

  /**
   * Define starting address in EEPROM
   */
  NodeMemory(int eepromBaseAddress, uint16_t userConfigSize);  // doesn't do anything
    
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
  void init(Event* events, uint8_t numEvents);
    
  /** 
   * For debug and test, this forces a reset to factory defaults at next restart
   */
  void forceFactoryReset();
    
  /** 
   * Reload a complete set of new events on next restart.
   */
  void forceNewEventIDs();
    
  uint8_t getNodeID(NodeID *nodeIdBuffer);

  void changeNodeID(NodeID *newNodeId);
    
  /*
   * Get a new, forever unique EventID and put in 
   * given EventID location. Does not do a EEPROM store,
   * which must be done separately. unique ID build using
   * this node's nodeID. 
   */
  //void setToNewEventID(NodeID* nodeID, EventID* eventID);
  void setToNewEventID(NodeID* nid, uint16_t eOff);
  void setToNewEventID(NodeID* nid, int n);  //dph
    
  private:
		void 				print();
		uint8_t			loadAndValidate();	
	  void 				writeNewEventIDs(Event* events, uint8_t numEvents);
		
		uint16_t		baseAddress; // address of 1st byte in EEPROM
		uint16_t 		bytesUsed; // size of EEPROM storage used in bytes
		uint8_t 		nodeHeaderState;
		NODE_HEADER	header;
};


#endif
