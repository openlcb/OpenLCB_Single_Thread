#ifndef EventID_h
#define EventID_h

#include "stdint.h"

//#include <Arduino.h>
class Event;
class EventsIndex;

class EventID;

class EventID {
  public: 
    uint8_t val[8];
    
    EventID();
    
    EventID(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7);
    
    bool equals(EventID* n);
    
    void print();
    
    //void writeEID(int index);
    
    int findIndexInArray(uint16_t* events, int len, int start);
    
};


#endif
