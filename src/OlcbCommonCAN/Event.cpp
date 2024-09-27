#include "Event.h"

#include "debugging.h"

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
    //dP(F("offset:")); dPH(offset);
    //dP(F("flags:")); dPH(flags);
}

/* For ranges:
int Event::compare(Event *key)
{
    Event diff;
    uint8_t nbits = flags>11;
    uint8_t B = nbits/8;
    uint8_t b = nbits%8;
    uint8_t mask= 1-(1<<(b+1));
    for(int i=0; i<(7-B); i++) {
        if(key->val[i] > val[i]) return 1;
        if(key->val[i] < val[i]) return -1;
    }
    if(nbits>0) {
        if((key->val[i]&mask) > (val[i]&mask)) return 1;
        if((key->val[i]&mask) < (val[i]&mask)) return -1;
    }
    return 0;
}
*/

