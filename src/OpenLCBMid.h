//
//  OpenLCBMid.h
//  
//
//  Created by David Harris on 2018-01-01.
//
//

#ifndef OpenLCBMid_h
#define OpenLCBMid_h

#include "NodeID.h"
#include "EventID.h"
#include "Event.h"

// specific OpenLCB implementations
#include "LinkControl.h"

#ifndef OLCB_NO_DATAGRAM
#include "Datagram.h"
#endif

#ifndef OLCB_NO_STREAM
#include "OlcbStream.h"
#endif

#ifndef OLCB_NO_MEMCONFIG
#include "Configuration.h"
#endif

#include "NodeMemory.h"
#include "PCE.h"
#include "PIP.h"
#include "SNII.h"
#include "BG.h"
#include "ButtonLed.h"
#include "lib_debug_print_common.h"

class Can;
Can olcbcanRx;
Can olcbcanTx;
OlcbCanInterface     rxBuffer(&olcbcanRx);  // CAN receive buffer
OlcbCanInterface     txBuffer(&olcbcanTx);  // CAN send buffer

#define LAST_EEPROM sizeof(MemStruct)

// The variable parts of the SNII protocol are stored in EEPROM, as secribed by MemStruct
MemStruct *pmem = 0;
#define SNII_var_data &pmem->nodeVar.nodeName           // location of SNII_var_data EEPROM, and address of nodeName
#define SNII_var_offset sizeof(pmem->nodeVar.nodeName)  // location of nodeDesc

extern "C" {

    // EIDTab - eventID_Table is stored in Flash
    // It has two fields:
    //   offset = offset to each eventID in the MemStruct in EEPROM
    //   flags = the initial flags indicating whether the eventID showed be announced at startup
    
    uint16_t getOffset(unsigned index) {
        return pgm_read_word(&eidtab[index].offset);
    }

    uint16_t getFlags(unsigned index) {
        return pgm_read_word(&eidtab[index].flags);
    }

//     uint32_t spaceUpperAddr(uint8_t space) {  // return last valid address
//         switch (space) {
//             case 255: return sizeof(configDefInfo) - 1; // CDI (data starts at zero)
//             case 254: return RAMEND; // RAM from Arduino definition
//             case 253: return LAST_EEPROM; // Configuration
//         }
//         return (uint32_t)3;
//     }
//     
    uint8_t getRead(uint32_t address, int space) {
        if (space == 0xFF) { // 255
            // Configuration definition information
            return pgm_read_byte(configDefInfo+address);
        } else if (space == 0xFE) { //254
            // All memory reads from RAM starting at first location in this program
            return *(((uint8_t*)&rxBuffer)+address);
        } else if (space == 0xFD) { //253
            // Configuration space is entire EEPROM
            uint8_t r = EEPROM.read(address);
                        //LDEBUG("\ngetRead "); LDEBUG2(space,HEX);
                        //LDEBUG(":"); LDEBUG2(address,HEX);
                        //LDEBUG("="); LDEBUG2(r,HEX);
            return r;
        } else if (space == 0xFC) { // 252
            // used by ADCDI/SNII for constant data
            return pgm_read_byte(SNII_const_data+address);
        } else if (space == 0xFB) { // 251
            // used by ADCDI/SNII for variable data
            return EEPROM.read((int)SNII_var_data+address);
        } else {
            // unknown space
            return 0;
        }
    }
    
    void getWrite(uint32_t address, int space, uint8_t val) {
                        //LDEBUG("\nolcbinc getWrite");
                        //LDEBUG(" space: "); LDEBUG2(space,HEX);
                        //LDEBUG(":"); LDEBUG2(address,HEX);
                        //LDEBUG("="); LDEBUG2(val,HEX);
        if (space == 0xFE) {
            // All memory
            *(((uint8_t*)&rxBuffer)+address) = val;
        } else if (space == 0xFD) {
            // Configuration space
            EEPROM.write(address, val);
            //eepromDirty = true;                 // ???
        }
        // all other spaces not written
    }
//     
//     void printeidtab() {
//         LDEBUG("\neidtab:\n");
//         for(int i=0;i<NUM_EVENT;i++) {
//             LDEBUG("[");
//             LDEBUG2(getOffset(i),HEX); LDEBUG(", ");
//             LDEBUG2(getFlags(i),HEX); LDEBUG("], ");
//         }
//     }

    // Extras
    void printEventIndexes() {
        LDEBUG(F("\nprintEventIndex\n"));
        for(int i=0;i<NUM_EVENT;i++) {
            LDEBUG2(eventIndex[i],HEX); LDEBUG(F(", "));
        }
    }
    void printEvents() {
        LDEBUG(F("\nprintEvents "));
        LDEBUG(F("\n#  flags  EventID"));
        for(int i=0;i<NUM_EVENT;i++) {
            LDEBUG("\n"); LDEBUG(i);
            LDEBUG(":"); LDEBUG2(getOffset(i),HEX);
            LDEBUG(F(" : ")); LDEBUG2(event[i].flags,HEX);
            LDEBUG(F(" : ")); event[i].eid.print();
        }
    }
//     
//     void printEventids() {
//         LDEBUG("\neventids:");
//         for(int e=0;e<NUM_EVENT;e++) {
//             LDEBUG("\n[");
//             for(int i=0;i<8;i++) {
//                 LDEBUG2(event[e].eid.val[i],HEX); LDEBUG(", ");
//             }
//         }
//     }

    void printSortedEvents() {
        LDEBUG("\nSorted events");
        for(int i=0; i<NUM_EVENT; i++) {
            int e = eventIndex[i];
            LDEBUG("\nEvent Index: "); LDEBUG(i);
            LDEBUG("  EventNum: ");    LDEBUG(e);
            LDEBUG("  EventID:"); 		 event[e].eid.print();
            LDEBUG("  Flags: ");       LDEBUG2(event[e].flags,HEX);
        }
    }

    void printEeprom() {
        LDEBUG("\nEEPROM:");
        LDEBUG(F("\n    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 0123456789ABCDEF"));
        for(unsigned r=0; r<(sizeof(MemStruct)/16+1);r++) {
            int rb = r*16;
            LDEBUG("\n"); if(rb<16) LDEBUG(0); LDEBUG2(rb,HEX); LDEBUG(" ");
            for(int i=rb;i<(rb+16);i++)  {
                uint8_t v = EEPROM.read(i);
                if(v<16) LDEBUG(0); LDEBUG2(v,HEX); LDEBUG(" ");
            }
            for(int i=rb;i<(rb+16);i++)  {
                char c = EEPROM.read(i);
                if( c<' ' || c==0x8F ) LDEBUG('.')
                else LDEBUG(c);
            }
        }
    }
} // end of extern

void configWritten(unsigned int address, unsigned int length, unsigned int func) {
    LDEBUG("\nconfigWritten: Addr: "); LDEBUG2(address,HEX); LDEBUG(" func:"); LDEBUG2(func,HEX);
    for(unsigned i=0; i<NUM_EVENT; i++) {
        uint16_t off = getOffset(i);
        if((address == off) && (length >= 8)) 
        	eepromDirty = true;
    }
    if(func == CFG_CMD_UPDATE_COMPLETE) {
        dP("\ncomplete, eepromDirty="); dP(eepromDirty);
        if(eepromDirty) {
            Olcb_softReset();   // trigger re-init from EEPROM.  ??? should this be in Olcb_loop()???
        }
    }
    if(userConfigWritten) userConfigWritten(address, length, func);
}


LinkControl clink(&txBuffer, &nodeid);

#ifndef OLCB_NO_STREAM
    unsigned int streamRcvCallback(uint8_t *rbuf, unsigned int length);
    OlcbStream str(&txBuffer, streamRcvCallback, &clink);
    unsigned int resultcode = 1;  // dummy temp value
    unsigned int streamRcvCallback(uint8_t *rbuf, unsigned int length){
        return resultcode;  // return pre-ordained result
    }
#else
    #define str 0
#endif

#ifndef OLCB_NO_DATAGRAM
    unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from);
    Datagram dg(&txBuffer, datagramCallback, &clink);
    Configuration cfg(&dg, &str, getRead, getWrite, userSoftReset, configWritten);

    unsigned int datagramCallback(uint8_t *rbuf, unsigned int length, unsigned int from) {
        // invoked when a datagram arrives
        //logstr("consume datagram of length ");loghex(length); lognl();
        //for (int i = 0; i<length; i++) printf("%x ", rbuf[i]);
        //printf("\n");
        // pass to consumers
        return cfg.receivedDatagram(rbuf, length, from);
    }
#else
    #define cfg 0
    #define dg  0
#endif

    PCE pce(event, NUM_EVENT, eventIndex, &txBuffer, pceCallback, &clink);

#ifndef OLCB_NO_BLUE_GOLD
    BG bg(&pce, buttons, patterns, NUM_EVENT, &blue, &gold, &txBuffer);
#else
    #define bg 0
#endif

NodeMemory nm(0);

extern "C" {
    
    extern void writeEID(int index) {
        // All write to EEPROM, may have to restore to RAM.
        LDEBUG("\nwriteEID() "); LDEBUG(index);
        eepromDirty = true; // flag eeprom changed
        EEPROM.put(getOffset(index), event[index].eid);
    }
}

// Compare function to compare two Event Index entries by comparing the EventIDs they point to
int cmpfunc (const void * a, const void * b)
{
	uint16_t indexA = *(uint16_t*)a;
	uint16_t indexB = *(uint16_t*)b;
	Event * pEventA = &event[indexA];
	Event * pEventB = &event[indexB];
	int cmp = pEventB->eid.compare(&pEventA->eid);
	
// 	LDEBUG("\nCompare A: "); pEventA->eid.print();
// 	LDEBUG(" Compare B: ");  pEventB->eid.print();
// 	LDEBUG(" Result: ");
// 	LDEBUG(cmp);
	
	return cmp;
}

extern void initTables(){        // initialize tables
    dP("\n initTables");
    for(unsigned int e=0; e<NUM_EVENT; e++) {
        eventIndex[e] = e;
        EEPROM.get(getOffset(e), event[e].eid);
        event[e].flags |= getFlags(e);
    }
//     LDEBUG("\nSort eventIndex");
    qsort(eventIndex, NUM_EVENT, sizeof(uint16_t), cmpfunc);
//     LDEBUG("\nSorted\n");
}

// ===== System Interface
void Olcb_init(uint8_t forceEEPROMInit) {       // was setup()
            //LDEBUG("\nIn olcb::init");
    EEPROMbegin;       // defined in processor.h
    if(forceEEPROMInit)
        nm.forceInitAll();  // factory reset
    
    eepromDirty = false;
    nm.setup(&nodeid, event, NUM_EVENT, (uint16_t)sizeof(MemStruct));
            //LDEBUG("\nIn olcb::init1");
    
    initTables();
//     printEventIndexes();
    printSortedEvents();
//     printEvents();
            //LDEBUG("\nIn olcb::init2");

    PIP_setup(&txBuffer, &clink);
    SNII_setup((uint8_t)sizeof(SNII_const_data), SNII_var_offset, &txBuffer, &clink);
            //LDEBUG("\nIn olcb::init4");
    olcbcanTx.init();
            //LDEBUG("\nIn olcb::init5");
    clink.reset();
            //LDEBUG("\nIn olcb::init6");
}

// Soft reset, reinitiatize from EEPROM, but maintain present CAN Link.
void Olcb_softReset() {
    dP(F("\nIn olcb_softReset"));
    nm.setup(&nodeid, event, NUM_EVENT, (uint16_t)sizeof(MemStruct));
    dP(F("\nIn olcb::softreset nm.setup()"));
    initTables();
}


// Main processing loop
//
bool Olcb_process() {   // was loop()
                //LDEBUG(F("\nIn Olcb_process()"));

    bool rcvFramePresent = rxBuffer.net->read();
                //LDEBUG(F("\n Olcb_process() 1"));

    clink.check();
                //LDEBUG(F("\n Olcb_process() 2"));

    bool handled = false;  // start accumulating whether it was processed or skipped
    if (rcvFramePresent) {
        handled = clink.receivedFrame(&rxBuffer);
    }

    if (clink.linkInitialized()) {
        if (rcvFramePresent && rxBuffer.isForHere(clink.getAlias()) ) {
                    //LDEBUG(F("\nOlcb_process got one"));
#ifndef OLCB_NO_DATAGRAM
            //#pragma message("!!! DG active ")
            handled |= dg.receivedFrame(&rxBuffer);  // has to process frame level
#endif
            if(rxBuffer.isFrameTypeOpenLcb()) {  // skip if not OpenLCB message (already for here)
                handled |= pce.receivedFrame(&rxBuffer);
#ifndef OLCB_NO_STREAM
                handled |= str.receivedFrame(&rxBuffer); // suppress stream for space
#endif
                handled |= PIP_receivedFrame(&rxBuffer);
                handled |= SNII_receivedFrame(&rxBuffer);
                if (!handled && rxBuffer.isAddressedMessage()) clink.rejectMessage(&rxBuffer, 0x2000);
            }
        }
        pce.check();
                    //LDEBUG(F("\nLeft PCE::check()")); //while(0==0){}
#ifndef OLCB_NO_DATAGRAM
        dg.check();
                    //LDEBUG(F("\nLeft dg.check()")); //while(0==0){}
#endif
#ifndef OLCB_NO_STREAM
        str.check();
                    //LDEBUG(F("\nLeft str.check()")); //while(0==0){}
#endif
#ifndef OLCB_NO_MEMCONFIG
        cfg.check();
                    //LDEBUG(F("\nLeft cfg.check()")); //while(0==0){}
#endif
#ifndef OLCB_NO_BLUE_GOLD
        bg.check();
                    //LDEBUG(F("\nLeft bg.check()")); //while(0==0){}
#endif
        PIP_check();
                    //LDEBUG(F("\nLeft PIP_check()")); //while(0==0){}
        SNII_check();
                    //LDEBUG(F("\nLeft SNII_check()")); while(0==0){}
        //produceFromInputs();  ??
    }
    return rcvFramePresent;
}

#endif /* OpenLCBMid_h */
