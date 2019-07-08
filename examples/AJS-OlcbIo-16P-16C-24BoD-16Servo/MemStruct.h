
#ifndef MemStruct_h
#define MemStruct_h

#include <Arduino.h>

#include "EventID.h"
#include "Event.h"
#include "NodeMemory.h"
#include "NodeID.h"
#include "Index.h"
#include <EEPROM.h>

//#define MEM_MODEL_SMALL   // small but slow, works out of EEPROM
#define MEM_MODEL_MEDIUM  // faster, eventIDs are copied to RAM
//#define MEM_MODEL_LARGE   // fastest but large, EEPROM is mirrored to RAM

// Helper macro to return the EEPROM Addr of MemStruct element
#define EADDR(x) (uint16_t)&pmem->x

void userFillEventOffsets();

/*
 *  Definition of the memory structure of the node's non-volatile memory, usually EEPROM.   
 *  It defines the memory layout of the node's variables.  
 *  It has three parts: 
 *    1. Non-optional node identification: NodeID, the next available eventID, and a magic 
 *         variable that documents the state of this memory.  
 *    2. Optional node identification: node-name and node-description.  These are optional, 
 *         but are useful for making the node's identification human-readable.  They are part 
 *         of ACDI, teh abreviated CDI.  
 *    3. Application data.  THe layout is up to the node designer.  The resulting structure
 *         has to be mirrired in the CDI xml description.  The CDI describes the nature and 
 *         size of the various node-variables/fields.  
 *         
 *  The example below demonstates two inouts, each with a description, and a pair of 
 *     producer-eventIDs, and two outputs, each with a descriptin and a pair of 
 *     consumer-eventIDs.  
 *     
 *  The struct is collected into a typedef MemStruct.  This is used to declare: 
 *    1. pmem -- a zero-based pointer used to access eepro amd ram, as necessary;
 *    2. mem  -- an in-memory structure used to mirrow the eeprom in the large-memory model.  
 */

#define NUM_OUTPUTS     8
#define NUM_INPUTS      8
#define NUM_BOD_INPUTS 24
#define NUM_SERVOS     16

#define FIRST_OUTPUT_EVENT_INDEX 0
#define FIRST_INPUT_EVENT_INDEX  (NUM_OUTPUTS*2)
#define FIRST_BOD_EVENT_INDEX    (FIRST_INPUT_EVENT_INDEX + NUM_INPUTS*2)
#define FIRST_SERVO_EVENT_INDEX  (FIRST_BOD_EVENT_INDEX + NUM_BOD_INPUTS*2)

#define NUM_EVENTS  ((NUM_OUTPUTS*2) + (NUM_INPUTS * 2) + (NUM_BOD_INPUTS * 2) + (NUM_SERVOS * 2))

// vvvvvvvv User defined EEPROM layout vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
typedef struct //__attribute__ ((packed)) 
{ 
  uint32_t magic;         // used to check eeprom status
  uint16_t nextEID;       // the next available eventID for use from this node's set
  uint8_t  nid[6];        // the nodeID
  char     nodeName[20];  // optional node-name, used by ACDI
  char     nodeDesc[24];  // optional node-description, used by ACDI
  struct {
    char desc[16];        // decription of this output
    EventID setLow;       // Consumed eventID which sets this output-pin
    EventID setHigh;      // Consumed eventID which resets this output-pin
  } digitalOutputs[NUM_OUTPUTS];
  struct {
    char desc[16];        // description of this input-pin
    EventID inputLow;     // eventID which is Produced on activation of this input-pin 
    EventID inputHigh;    // eventID which is Produced on deactivation of this input-pin
  } digitalInputs[NUM_INPUTS];
  struct {
    char desc[16];        // description of this BoD input-pin
    EventID empty;        // eventID which is Produced on Block Empty
    EventID occupied;     // eventID which is Produced on Block Occupied 
  } bodInputs[NUM_BOD_INPUTS];
  struct {
    char desc[16];        // description of this Servo Turnout Driver
    EventID thrown;       // consumer eventID which sets turnout to Diverging 
    uint8_t thrownPos;    // position of turount in Diverging
    EventID closed;       // consumer eventID which sets turnout to Main
    uint8_t closedPos;    // position of turnout in Normal
  } servoOutputs[NUM_SERVOS];
} MemStruct;
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// Description of EEPROM memory structure, and the mirrored mem if in MEM_LARGE
extern MemStruct * pmem;
extern Event events[NUM_EVENTS] ;   // repeated for all eight events. 
extern Index eventsIndex[NUM_EVENTS];  // Sorted index to eventids
extern const PROGMEM uint16_t eventidOffset[];
#ifdef MEM_MODEL_MEDIUM
extern EventID eventids[NUM_EVENTS];    // copy of eventids in RAM
#endif

void initTables();
void restore();
void printRawEEPROM();
void initTables();
void printEventsIndex();
void printEvents();
#endif
