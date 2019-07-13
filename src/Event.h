#ifndef Event_h
#define Event_h

#include "EventID.h"

class Event {
 public:
  Event();
  Event(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7);
  Event(bool produce, bool consume);

  bool isConsumer();
  bool isProducer();
  
  void print();
  static int evCompare(void* a, void* b);

  // RAM copy of EventIDs
  EventID eid;

  // Runtime action flags
  uint16_t flags;

	// Mark as waiting to have Identify sent
  static const int IDENT_FLAG = 0x01;
  
	// Mark produced event for send
  static const int PRODUCE_FLAG = 0x02;
  
	// Mark entry as really empty, ignore
  static const int EMPTY_FLAG = 0x04;
  
	// Mark entry to written from next learn message
  static const int LEARN_FLAG = 0x08;
  
	// Mark entry to send a learn message
  static const int TEACH_FLAG = 0x10;
  
  // Mark entry as consumer
  static const int CAN_CONSUME_FLAG = 0x20;
  
  // Mark entry as producer
  static const int CAN_PRODUCE_FLAG = 0x40;
};

#endif
