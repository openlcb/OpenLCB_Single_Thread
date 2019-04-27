//
//  Index.h
//  
//
//  Created by David Harris on 2017-11-29.
//
//

#ifndef Index_h
#define Index_h

#include <Arduino.h>
#include "EventID.h"


class EventID;

class Index {
public:
    uint16_t hash;
    uint16_t index;
    
    //EventID Index::getEID();
    EventID getEID();
    //EventID getEID(unsigned i);
    
    void set(uint16_t i, void* m, uint16_t s);
    
    uint16_t hashcalc(void* m, uint16_t s);
    //static int Index::sortCompare(const void* aa, const void* bb);
    static int sortCompare(const void* aa, const void* bb);
    //static int Index::findCompare(const void* aa, const void* bb);
    static int findCompare(const void* aa, const void* bb);
    
    //Index* Index::findIndex(void* ms, uint16_t s, uint16_t is, Index* start);
    Index* findIndex(void* ms, uint16_t s, uint16_t is, Index* start);
    //int Index::findIndexOfEventID(EventID* eid, uint16_t is, uint16_t start);
    int findIndexOfEventID(EventID* eid, uint16_t is, uint16_t start);
    
    //void Index::sort(uint16_t n);
    void sort(uint16_t n);

    //virtual void Index::print();
    virtual void print();
    //void Index::print(uint16_t n);
    void print(uint16_t n);
    
};


#endif /* Index_h */
