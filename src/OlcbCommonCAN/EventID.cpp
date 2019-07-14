//
//  EventID.cpp
//  
//
//  Created by David Harris on 2017-11-19.
//
//

#include "EventID.h"

#include "lib_debug_print_common.h"

EventID::EventID() {
    val[0] = 0;
    val[1] = 0;
    val[2] = 0;
    val[3] = 0;
    val[4] = 0;
    val[5] = 0;
    val[6] = 0;
    val[7] = 0;
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

bool EventID::equals(EventID* n) {
                        //LDEBUG("\nequals("); this->print();
                        //n->print(); LDEBUG(")");
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

void EventID::print()
{
	LDEBUG(' ');
	for (int i=0;i<8;i++)
	{
		if(i>0)
			LDEBUG(".");
		if(val[i] < 16)
			LDEBUG('0');
		LDEBUG2(val[i],HEX);
	}
}
