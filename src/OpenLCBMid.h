//
//  OpenLCBMid.h
//  
//
//  Created by David Harris on 2018-01-01.
//
//

#ifndef OpenLCBMid_h
#define OpenLCBMid_h

//#pragma message "OpenLCBMid.h"

#include "processCAN.h"
#include "processor.h"
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

////#include "GCSerial.h"
#include "NodeMemory.h"
#include "PIP.h"
#include "SNII.h"
#include "BG.h"
#include "ButtonLed.h"

#include "debugging.h"

#include "OpenLcbCore.h"

NodeID nodeId;
void setEepromDirty();

//class Can;
OlcbCanClass olcbcanRx;
OlcbCanClass olcbcanTx;
OlcbCanInterface     rxBuffer(&olcbcanRx);  // CAN receive buffer
OlcbCanInterface     txBuffer(&olcbcanTx);  // CAN send buffer

// The variable parts of the SNII protocol are stored in EEPROM, as secribed by MemStruct
MemStruct *pmem = 0;
#define SNII_var_data &pmem->nodeName           // location of SNII_var_data EEPROM, and address of nodeName
#define SNII_var_offset sizeof(pmem->nodeName)  // location of nodeDesc

NodeMemory nm(0, sizeof(MemStruct));

extern "C" {

    // EIDTab - eventID_Table is stored in Flash
    // It has two fields:
    //   offset = offset to each eventID in the MemStruct in EEPROM
    //   flags = the initial flags indicating whether the eventID showed be announced at startup
    
    uint16_t getOffset(uint16_t index) {
        return pgm_read_word(&eidtab[index].offset);
        }

    uint16_t getFlags(uint16_t index) {
        return pgm_read_word(&eidtab[index].flags);
    }
    
    uint32_t spaceUpperAddr(uint8_t space) {  // return last valid address
        switch (space) {
            case 255: return sizeof(configDefInfo) - 1; // CDI (data starts at zero)
            case 254: return RAMEND; // RAM from Arduino definition
            case 253: return NODECONFIG.length(); // Configuration
        }
        return (uint32_t)3;
    }
    
    uint8_t getRead(uint32_t address, int space) {
        if (space == 0xFF) { // 255
            // Configuration definition information
            return pgm_read_byte(configDefInfo+address);
        } else if (space == 0xFE) { //254
            // All memory reads from RAM starting at first location in this program
            return *(((uint8_t*)&rxBuffer)+address);
        } else if (space == 0xFD) { //253
            // Configuration space is entire EEPROM
            uint8_t r = NODECONFIG.read(address);
                //dP((String)"\ngetRead="); dPH((uint8_t)space);
                //dP(':'); dPH((uint32_t)address);
                //dP('='); dPH((uint8_t)r);
            return r;
        } else if (space == 0xFC) { // 252
            // used by ADCDI/SNII for constant data
            return pgm_read_byte(SNII_const_data+address);
        } else if (space == 0xFB) { // 251
            // used by ADCDI/SNII for variable data
            return NODECONFIG.read((int)SNII_var_data+address);
        } else {
            // unknown space
            return 0;
        }
    }
    
    void getWrite(uint32_t address, int space, uint8_t val) {
        //dP((String)"\ngetWrite="); dPH((uint8_t)space);
        //dP(':'); dPH((uint32_t)address);
        //dP('='); dPH((uint8_t)val);
        if (space == 0xFE) {
            // All memory
            *(((uint8_t*)&rxBuffer)+address) = val;
        } else if (space == 0xFD) {
            // Configuration space
            NODECONFIG.update(address, val);
            setEepromDirty();
        }
        // all other spaces not written
    }
    // Extras
} // end of extern

void configWritten(uint32_t address, uint16_t length, uint16_t func) {
    //dP("\nconfigWritten: Addr: "); dPH((uint32_t)address);
    //dPS(" length:", (uint16_t)length);
    //dPS(" func:", (uint16_t)func);
    for(uint16_t i=0; i<NUM_EVENT; i++) {
        uint16_t off = getOffset(i);
        if((address == off) && (length >= 8)) {
            OpenLcb.initTables();
            setEepromDirty();
        }
    }
    if(func == CFG_CMD_UPDATE_COMPLETE) {
        if(eepromDirty) {
            Olcb_softReset();   // trigger re-init from EEPROM.  ??? should this be in Olcb_loop()???
            eepromDirty = false;
        }
    }
    if(userConfigWritten)
        userConfigWritten(address, length, func);
}


LinkControl clink(&txBuffer, &nodeId);

#ifndef OLCB_NO_STREAM
    uint16_t streamRcvCallback(uint8_t *rbuf, uint16_t length);
    OlcbStream str(&txBuffer, streamRcvCallback, &clink);
    uint16_t resultcode = 1;  // dummy temp value
    uint16_t streamRcvCallback(uint8_t *rbuf, uint16_t length){
        return resultcode;  // return pre-ordained result
    }
#else
    #define str 0
#endif

#ifndef OLCB_NO_DATAGRAM
    uint16_t datagramCallback(uint8_t *rbuf, uint16_t length, uint16_t from);
    Datagram dg(&txBuffer, datagramCallback, &clink);
    Configuration cfg(&dg, &str, getRead, getWrite, userSoftReset, configWritten);

    uint16_t datagramCallback(uint8_t *rbuf, uint16_t length, uint16_t from) {
        // invoked when a datagram arrives
        // pass to consumers
        //dP("consume datagram of length ");dP((uint32_t)length); dP("\n");
        //for (int i = 0; i<length; i++) { dPH(rbuf[i]); dP(" "); }
        //dP("\n");
        return cfg.receivedDatagram(rbuf, length, from);
    }
#else
    #define cfg 0
    #define dg  0
#endif

OpenLcbCore OpenLcb(event, NUM_EVENT, eventIndex, eidtab, &txBuffer, &clink);

#ifndef OLCB_NO_BLUE_GOLD
    BG bg(&OpenLcb, buttons, patterns, NUM_EVENT, &blue, &gold, &txBuffer);
#else
    #define bg 0
#endif


extern "C" {
    
    extern void writeEID(int index, EventID eid) {
        // All write to EEPROM, may have to restore to RAM.
        //dPS("\nwriteEID() ", (uint16_t)index);
        NODECONFIG.put(getOffset(index), eid);
        setEepromDirty(); // flag Eeprom changed
    }
}

// ===== System Interface
//void Olcb_init(uint8_t forceFactoryReset) {       // was setup()
void Olcb_init(NodeID nid, uint8_t forceFactoryReset) {
    dP(F("\nOpenLCBMid.h/Olcb_init()"));
    //dP(F(" nid=")); nid.print();
    //dP(F(" forceFactoryReset=")); dP((bool)forceFactoryReset);
    EEPROMbegin;
    eepromDirty = false;

    if(forceFactoryReset)
        OpenLcb.forceFactoryReset();  // factory reset
        
    	// Read the NodeID from EEPROM
    nm.loadAndValidate();  // moved from nm constructor
    nm.getNodeID(&nodeId);
    
    //dP((String)"\n    Stored nid="); nodeId.print();
	//nm.print(sizeof(MemStruct));
    //dP((String)"\n    New nid=")); nid.print();
    
    if( !nodeId.equals(&nid) ) {
        nm.changeNodeID(nid);
        setEepromDirty();
    }

    OpenLcb.init();
    OpenLcb.initTables();
    OpenLcb.printEvents();
	OpenLcb.printSortedEvents();
    nm.print(sizeof(MemStruct));

    PIP_setup(&txBuffer, &clink);
    SNII_setup((uint8_t)sizeof(SNII_const_data), SNII_var_offset, &txBuffer, &clink);
    delay(50);
    olcbcanTx.init();
    clink.reset();
    EEPROMcommit;
    //dP((String)"\n    eeprom.commit()");
}

// Soft reset, reinitiatize from EEPROM, but maintain present CAN Link.
void Olcb_softReset() {
    //dP(F("\nIn olcb_softReset"));
    OpenLcb.init();
    //dP(F("\nIn olcb::softreset nm.setup()"));
//AJS Fix    initTables();  // dph?? is fixed?
}

unsigned long eepromupdatedue = 0;
void setEepromDirty() {
    //dP("\nmemory is dirty");
    eepromDirty = true;
    eepromupdatedue = millis();
}

// Main processing loop
//
bool Olcb_process() {   // was loop()
    //dP("\nprocess");
   #if 0
    if( eepromDirty && (millis()-eepromupdatedue)>10000 ) {
        EEPROMcommit;
        eepromDirty = false;
        dP(F("\nOlcb_process() eeprom saved "));
    }
   #endif

    bool rcvFramePresent = rxBuffer.net->read();
    clink.check();

    bool handled = false;  // start accumulating whether it was processed or skipped
    if (rcvFramePresent) {
        handled = clink.receivedFrame(&rxBuffer);
    }

    if (clink.linkInitialized()) {
        if (rcvFramePresent && rxBuffer.isForHere(clink.getAlias()) ) {
#ifndef OLCB_NO_DATAGRAM
            handled |= dg.receivedFrame(&rxBuffer);  // has to process frame level
#endif
            if(rxBuffer.isFrameTypeOpenLcb()) {  // skip if not OpenLCB message (already for here)
                handled |= OpenLcb.receivedFrame(&rxBuffer);
#ifndef OLCB_NO_STREAM
                handled |= str.receivedFrame(&rxBuffer); // suppress stream for space
#endif
                handled |= PIP_receivedFrame(&rxBuffer);
                handled |= SNII_receivedFrame(&rxBuffer);
                if (!handled && rxBuffer.isAddressedMessage()) clink.rejectMessage(&rxBuffer, 0x2000);
            }
        }
        OpenLcb.check();
#ifndef OLCB_NO_DATAGRAM
        dg.check();
#endif
#ifndef OLCB_NO_STREAM
        str.check();
#endif
#ifndef OLCB_NO_MEMCONFIG
        cfg.check();
#endif
#ifndef OLCB_NO_BLUE_GOLD
        bg.check();
#endif
        PIP_check();
        SNII_check();
    }
    return rcvFramePresent;
}

#endif /* OpenLCBMid_h */
