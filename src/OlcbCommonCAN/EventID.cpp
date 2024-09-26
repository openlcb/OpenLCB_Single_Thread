//
//  EventID.cpp
//  
//
//  Created by David Harris on 2017-11-19.
//
//

#include "EventID.h"

#include "debugging.h"

EventID::EventID() {
    memset(val, 0, sizeof(val));
}

EventID::EventID(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7) {
    val[0] = b0;
    val[1] = b1;
    val[2] = b2;
    val[3] = b3;
    val[4] = b4;
    val[5] = b5;
    val[6] = b6;
    val[7] = b7;
}

void EventID::setNodeIdPrefix(NodeID *nodePrefix)
{
	memcpy(val, nodePrefix->val, sizeof(nodePrefix->val));
}

void EventID::setEventIdSuffix(uint16_t suffix)
{
	val[6] = (suffix >> 8) & 0xFF;
	val[7] = suffix & 0xFF;
}

bool EventID::equals(EventID* n) {
        //dP("\nequals("); this->print();
        //n->print(); dP(")");
    return  (val[0]==n->val[0])&&(val[1]==n->val[1])
         &&(val[2]==n->val[2])&&(val[3]==n->val[3])
         &&(val[4]==n->val[4])&&(val[5]==n->val[5])
         &&(val[6]==n->val[6])&&(val[7]==n->val[7]);
}

int EventID::compare(EventID *key)
{
    for(int i=0; i<8; i++) {
        if(key->val[i] > val[i])
        		return 1;
        		
        if(key->val[i] < val[i]) 
        		return -1;
    }    
		return 0;
}
/* For ranges: ** better to compare Events.  
int EventID::compare(EventID *key, uint8_t nbits)
{
    EventID diff;
    uint8_t B = nbits/8;
    uint8_t b = nbits%8;
    uint8_t mask= 1-(1<<(b+1));
    for(int i=0; i<B; i++) {
    if(key->val[i] > val[i]) return 1;
    if(key->val[i] < val[i]) return -1;
    }
    if(B>0) {
        if((key->val[i]&mask) > (val[i]&mask)) return 1;
        if((key->val[i]&mask) < (val[i]&mask)) return -1;
    }
    return 0;
}
*/

#include <Arduino.h>
void EventID::print()
{
    dP(' ');
    for (int i=0;i<8;i++) {
        if(i>0) dP('.');
        if(val[i] < 16) dP('0');
        dPH(val[i]);
    }
}
