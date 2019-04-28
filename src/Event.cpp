#include "Event.h"
#include "lib_debug_print_common.h"

Event::Event() {
}

Event::Event(bool produce, bool consume) {
    if (produce) flags |= Event::CAN_PRODUCE_FLAG;
    if (consume) flags |= Event::CAN_CONSUME_FLAG;
}

bool Event::isConsumer() {
    return flags&Event::CAN_CONSUME_FLAG;
}

bool Event::isProducer() {
    return flags&Event::CAN_PRODUCE_FLAG;
}

void Event::print() {
    //LDEBUG("offset:"); LDEBUG2(offset,HEX);
    //LDEBUG("flags:"); LDEBUG2(flags,HEX);    
}

