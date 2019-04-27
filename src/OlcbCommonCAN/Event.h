#ifndef Event_h
#define Event_h

#include "EventID.h"

class EventID;
class Event {
 public:
  Event();
  Event(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7);
  Event(bool produce, bool consume);

  bool isConsumer();
  bool isProducer();
  
  //bool equals(Event* n);
  void print();
  //EventID getEID(unsigned i);
  static int evCompare(void* a, void* b);

  // RAM copy of EventIDs
  EventID eid;
  // Runtime action flags
  uint16_t flags;
  
  // Mark entry as consumer
  static const int CAN_CONSUME_FLAG = 0x20;
  // Mark entry as producer
  static const int CAN_PRODUCE_FLAG = 0x40;
};

#endif
