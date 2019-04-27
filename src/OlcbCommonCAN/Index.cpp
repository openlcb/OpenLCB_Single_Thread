//
//  Index.cpp
//  
//
//  Created by David Harris on 2017-11-29.
//
//

#include "Index.h"
#include "lib_debug_print_common.h"

uint16_t Index::hashcalc(void* m, uint16_t s) {
    uint16_t hash = 0;
    for(int i=0; i<s; i++) hash += ((uint8_t*)m)[i];
    return hash;
}
void Index::set(uint16_t i, void* m, uint16_t s) {
    index = i;
    hash = hashcalc(m,s);
}
int Index::sortCompare(const void* aa, const void* bb) {
            //LDEBUG(F("\nIn hashCompare:"));
    Index* a = (Index*)aa;
    Index* b = (Index*)bb;
            //LDEBUG(F(" a=")); LDEBUG2(a->hash,HEX);
            //LDEBUG(F(" b=")); LDEBUG2(b->hash,HEX);
    if(a->hash>b->hash) return 1;
    if(a->hash<b->hash) return -1;
    return 0;
}
int Index::findCompare(const void* aa, const void* bb) {
            //LDEBUG(F("\nIn indexCompare:"));
    int a = *(int*)aa;
    Index* b = (Index*)bb;
            //LDEBUG(F(" a=")); LDEBUG2(a,HEX);
            //LDEBUG(F(" b=")); LDEBUG2(b->hash,HEX);
    if(a>b->hash) return 1;
    if(a<b->hash) return -1;
    return 0;
}

Index* Index::findIndex(void* ms, uint16_t s, uint16_t is, Index* start) {
                //LDEBUG(F("\nIn Index::findIndex"));
                //LDEBUG(F("\nis=")); LDEBUG(is);
    Index* p = start;
    if(start>(this+is)) return nullptr;
    Index hh;
    hh.hash = hashcalc(ms,s);
                //LDEBUG(F("\nms=")); LDEBUG(ms);
                //LDEBUG(F("\nhh=")); LDEBUG2(hh,HEX);
    if(start==0) {
        p = (Index*)bsearch(&hh, this, is, sizeof(Index), findCompare);
        if(!p) return nullptr;
                //LDEBUG(F("\ni=")); LDEBUG(p->index);
                //LDEBUG(F("\nBackup"));
        while((p-1)>=this && (p-1)->hash==hh.hash) {p--;}
                //LDEBUG(F("\np=")); LDEBUG(p->index);
    } else {
                //LDEBUG(F("\nnext:"));
        p++;
        if(start>(this+is)) return nullptr;
                //LDEBUG(F("\np=")); LDEBUG(p->index);
        if(p->hash==hh.hash) return p;
        else return nullptr;
    }
                //LDEBUG(F("\np->hash=")); LDEBUG(p->hash);
    if(hh.hash==p->hash) return p;
    return nullptr;
}


void Index::print() {
    LDEBUG(F("["));LDEBUG2(hash,HEX);
    LDEBUG(F(","));LDEBUG(index);
    LDEBUG(F("]"));
}

void Index::print(uint16_t n) {
    LDEBUG(F("\nIndex::"));
    for(int i=0;i<n;i++) {
        LDEBUG("\n");LDEBUG(i);LDEBUG(" ");
        (this+i)->print();
    }
}
void Index::sort(uint16_t n) {
    //LDEBUG(F("\nsize="));LDEBUG(sizeof(this));
    qsort(this, n, sizeof(*this), Index::sortCompare);
}

