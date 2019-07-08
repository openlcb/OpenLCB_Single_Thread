/*

#include "MemStruct.h"
#include "Event.h"
#include "EventID.h"
#include "debug.h"


//#pragma message( "!!! compiling MemStruct.cpp")

//  Set of routines that are common to the Memory Models.

  // All write to EEPROM, may have to restore to RAM.
void EventID::writeEID(int index)
{
  DEBUG("\nwriteEID() "); DEBUG(index);
  uint8_t offset = eventidOffset[index];
  for (int i=0; i < 8; i++) 
    EEPROM.update(offset + i, this->val[i]);
}

EventID getEID(unsigned i); // Forward declaration - definition below
EventID blog(unsigned i) { return getEID(i); }

// define eventsOffset array in flash      (Balaz's idea) Note: this negates the need for userInitEventIDOffsets()
#define ADDR_EID(x)         ((unsigned int)&pmem->x)
#define REG_OUTPUT(s)       ADDR_EID(digitalOutputs[s].setLow), ADDR_EID(digitalOutputs[s].setHigh) 
#define REG_INPUT(s)        ADDR_EID(digitalInputs[s].inputLow), ADDR_EID(digitalInputs[s].inputHigh)  
#define REG_BOD_INPUT(s)    ADDR_EID(bodInputs[s].empty), ADDR_EID(bodInputs[s].occupied) 
#define REG_SERVO_OUTPUT(s) ADDR_EID(servoOutputs[s].thrown), ADDR_EID(servoOutputs[s].closed) 

const PROGMEM uint16_t eventidOffset[NUM_EVENTS] = { 
   REG_OUTPUT(0), REG_OUTPUT(1), REG_OUTPUT(2), REG_OUTPUT(3), REG_OUTPUT(4), REG_OUTPUT(5), REG_OUTPUT(6), REG_OUTPUT(7),
   REG_INPUT(0), REG_INPUT(1), REG_INPUT(2), REG_INPUT(3), REG_INPUT(4), REG_INPUT(5), REG_INPUT(6), REG_INPUT(7),
   REG_BOD_INPUT(0), REG_BOD_INPUT(1), REG_BOD_INPUT(2), REG_BOD_INPUT(3), REG_BOD_INPUT(4), REG_BOD_INPUT(5), REG_BOD_INPUT(6), REG_BOD_INPUT(7),
   REG_BOD_INPUT(8), REG_BOD_INPUT(9), REG_BOD_INPUT(10), REG_BOD_INPUT(11), REG_BOD_INPUT(12), REG_BOD_INPUT(13), REG_BOD_INPUT(14), REG_BOD_INPUT(15),
   REG_BOD_INPUT(16), REG_BOD_INPUT(17), REG_BOD_INPUT(18), REG_BOD_INPUT(19), REG_BOD_INPUT(20), REG_BOD_INPUT(21), REG_BOD_INPUT(22), REG_BOD_INPUT(23),
   REG_SERVO_OUTPUT(0), REG_SERVO_OUTPUT(1), REG_SERVO_OUTPUT(2), REG_SERVO_OUTPUT(3), REG_SERVO_OUTPUT(4), REG_SERVO_OUTPUT(5), REG_SERVO_OUTPUT(6), REG_SERVO_OUTPUT(7), 
   REG_SERVO_OUTPUT(8), REG_SERVO_OUTPUT(9), REG_SERVO_OUTPUT(10), REG_SERVO_OUTPUT(11), REG_SERVO_OUTPUT(12), REG_SERVO_OUTPUT(13), REG_SERVO_OUTPUT(14), REG_SERVO_OUTPUT(15) 
}; 

// zero-based pointer into the MemStruct structure
MemStruct * pmem = 0;

Event events[NUM_EVENTS] = { 
    // 8 x 2 Output Events
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),

    // 8 x 2 Input Events
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),

    // 24 x 2 BOD Input Events
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),

    // 16 x 2 Servo Output Events
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),
  Event(), Event(), Event(), Event(), Event(), Event(), Event(), Event(),
  };
 
   
// Sorted index to eventids
Index eventsIndex[NUM_EVENTS];  // Sorted index to eventids

// === LARGE ============================================

#ifdef MEM_MODEL_LARGE
MemStruct mem = EEPROM.get(0,mem);

 void printRawMem() {
  uint8_t* m = (uint8_t*)&mem;
  int rows = sizeof(MemStruct)/16 + 1;
  uint8_t c;
  //for(int r=0; r<64; r++) {
    DEBUG("\nprintRawMem()");
    for(int r=0; r<rows; r++) {
    DEBUG("\n");
    if(r==0) DEBUG(0);
    DEBUGHEX(r*16,HEX);
    for(int i=0;i<16;i++) {
      c = m[r*16+i];
      DEBUG(" ");
      if(c<16) DEBUG(0);
      DEBUGHEX(c,HEX);
    }
    DEBUG("->");
    for(int i=0;i<16;i++) {
      char c = m[r*16+i];
      if(c>' '&&c<'~') DEBUG(c);
      else DEBUG(".");
    }
  }
 }


  // Initialize the events[], eventsIndex[] tables
  //  - initializes the hash, index and flag fields 
  void initTables() {
    //EEPROM.get(0, mem);
    printRawMem();
    //userInitEventIDOffsets();
    uint8_t* m = (uint8_t*)&mem;
    for (int e=0; e < NUM_EVENTS; e++) {
      events[e].flags = 0;
      unsigned int evOffset = eventidOffset[e];
      EventID* p = (EventID*)&m[evOffset];
      eventsIndex[e].hash = p->hash();
      eventsIndex[e].index = e;
    }
    qsort( eventsIndex, NUM_EVENTS, sizeof(Index), Index::sortCompare);
    //printEventsIndex();
    //printEvents();
    //DEBUG(F("\nOut initTables Large"));
  }
 
 EventID getEID(unsigned index) {
    EventID r;
    uint8_t* m = (uint8_t*)&mem;
    uint16_t offset = eventidOffset[index];
    for(int i=0;i<8;i++) r.val[i] = m[i+offset];
    return r;
 }
 
 void initEventOffsets() {
        DEBUG("\ninitEventOffsets - Large");
    //for (int i=0;i<sizeof(MemStruct;i++) 
    EEPROM.get(0,mem);
    printRawMem();
    userFillEventOffsets();
 }
 
 void restore() {
   DEBUG("\nIn restore() Large");
   EEPROM.get(0,mem);  // reload all of eeprom
 }
#endif

// === MEDIUM ===========================================

#ifdef MEM_MODEL_MEDIUM
  EventID eventids[NUM_EVENTS];    // copy of eventids in RAM

  // Initialize the events[], eventsIndex[] tables
  //  - initializes the hash, index and flag fields 
  void initTables() {
    DEBUG("\nIn initTables Medium");
    //printRawEEPROM();  while(1==1){}
    //userInit(); AJS Why is this called from here?
    for (int e=0; e<NUM_EVENTS; e++) {
      events[e].flags = 0;
      EEPROM.get(eventidOffset[e], eventids[e]);
      eventsIndex[e].hash = eventids[e].hash();
      eventsIndex[e].index = e;
    }
    //printEventsIndex();
    //printEvents();
    qsort( eventsIndex, NUM_EVENTS, sizeof(Index), Index::sortCompare);
    //printEventsIndex();
    //printEvents();
    DEBUG("\nOut initTables M");
  }
   
 EventID getEID(unsigned i) {
           //DEBUG("\ngetEID Medium index="); DEBUG(i);
    return eventids[i];
 }

 void restore() {}  // nil to do
#endif

// === SMALL =================================================

#ifdef MEM_MODEL_SMALL

  void initTables() {
    DEBUG("\ninitTables");
    //userInitEventIDOffsets();        //  ?????  trial
    for (int e=0; e < NUM_EVENTS; e++) {
      EventID eid;
      EEPROM.get(eventidOffset[e], eid);
      eventsIndex[e].hash = eid.hash();
      eventsIndex[e].index = e;
      events[e].flags = 0;
    }
    eventsIndex->sort(NUM_EVENTS);
    //qsort( eventsIndex, NUM_EVENT, sizeof(Index), Index::sortCompare);
    //printEventsIndex();
    //printEvents();
    DEBUG(F("\nOut initTables S"));
  }
 EventID getEID(unsigned i) {
             //DEBUG(F("\ngetEID small"));
    int offset = eventidOffset[i];
    EventID r;
    for (int i=0; i < 8; i++)
      r.val[i] = EEPROM.read(offset+i);
    return r;
 }

 void restore() {
   DEBUG(F("\nIn restore() Small"));
   // restore EEPROM -- nil to do, they remain in EEPROM
 }
#endif
// ^^^^^^^^^^^^^^^^^^^^^^^^ SMALL ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// Extras

void printRawEEPROM() {
  DEBUG(F("\nprintRawEEPROM"));
  uint8_t c;
  int rows = sizeof(MemStruct)/16 + 1;
  //for(int r=0; r<64; r++) {
  for(int r=0; r<rows; r++) {
    DEBUG("\n");
    if(r==0) DEBUG(0);
    DEBUGHEX(r*16,HEX);
    for(int i=0;i<16;i++) {
      c = EEPROM.read(r*16+i);
      DEBUG(" ");
      if(c<16) DEBUG(0);
      DEBUGHEX(c,HEX);
    }
    DEBUG(F("->"));
    for(int i=0;i<16;i++) {
      char c = EEPROM.read(r*16+i);
      if(c>' '&&c<'~') DEBUG(c);
      else DEBUG(".");
    }
  }
}

void printEventsIndex() {
  DEBUG(F("\nprintEventsIndex"));
  for(int i=0; i < NUM_EVENTS; i++) {
    DEBUG("\n eventsIndex["); DEBUG(i); DEBUG("] ");
    eventsIndex[i].print();
  }
}
void printEvents() {
  DEBUG(F("\nprintEvents "));
  for(int i=0; i < NUM_EVENTS; i++) {
    DEBUG(F("\nIndex: ")); DEBUG(i);
    DEBUG(F(" flags: ")); DEBUGHEX(events[i].flags,HEX);
#ifdef MEM_MODEL_MEDIUM
    DEBUG(F(" eventID: ")); eventids[i].print();
#endif
  }
}
*/
