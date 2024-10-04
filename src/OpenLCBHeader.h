//
//  OpenLCBHeader.h
//  
//
//  Created by David Harris on 2018-01-01.
//
//

#ifndef OpenLCBHeader_h
#define OpenLCBHeader_h

bool eepromDirty;

#include "processor.h"
#include "OlcbCommonVersion.h"
#include "BG.h"
#include "ButtonLed.h"

#include "OpenLcbCore.h"

// specific OpenLCB implementations
#include "LinkControl.h"

//#ifndef OLCB_NO_DATAGRAM
#include "Datagram.h"
//#endif
//#ifndef OLCB_NO_STREAM
#include "OlcbStream.h"
//#endif
//#ifndef OLCB_NO_MEMCONFIG
#include "Configuration.h"
//#endif
//#ifndef OLCB_NO_MEMCONFIG
#include "Configuration.h"
//#endif

#include "NodeMemory.h"

// ===== CDI System Portions =======================================
#define CDIheader "\
    <cdi xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='http://openlcb.org/trunk/prototypes/xml/schema/cdi.xsd'>\n\
    <identification>\n\
        <manufacturer>" MANU "</manufacturer>\
        <model>" MODEL "</model>\
        <hardwareVersion>" HWVERSION "</hardwareVersion>\
        <softwareVersion>" SWVERSION "</softwareVersion>\
        </identification>\
    <segment origin='4' space='253'> <!-- bypasses magic, nextEID, nodeID -->\
        <group>\
            <name>Node ID</name>\
            <description>User-provided description of the node</description>\
            <string size='20'><name>Node Name</name></string>\
            <string size='24'><name>Node Description</name></string>\
        </group>"
#define CDIfooter "\
    </segment>\
    <segment origin='2' space='253'> <!-- stuff magic to trigger resets -->\
        <name>Reset Control</name>\
        <description>Controls Actions at Next Board Reset. Reloading User or Factory Defaults</description>\
        <int size='1'>\
            <map>\
                <relation><property>238</property><value>Normal Operation</value></relation>\
                <relation><property>51</property><value>Clear Everything but Create New Unused EventIDs</value></relation>\
                <relation><property>255</property><value>Clear Everything back to Factory Defaults</value></relation>\
            </map>\
        </int>\
    </segment>\
    </cdi>"


#define EEADDR(x)  ((uint16_t)(int)(&(((MemStruct*)0)->x)))
#define CEID(x)    {EEADDR(x), (uint16_t)Event::CAN_CONSUME_FLAG | Event::IDENT_FLAG}
#define PEID(x)    {EEADDR(x), (uint16_t)Event::CAN_PRODUCE_FLAG | Event::IDENT_FLAG}
#define PCEID(x)   {EEADDR(x), (uint16_t)Event::CAN_CONSUME_FLAG|Event::CAN_PRODUCE_FLAG | Event::IDENT_FLAG}

// ===== PIP Support ===============================================
#define pSimple       0x80
#define pDatagram     0x40
#define pStream       0x20
#define pMemConfig    0x10
#define pReservation  0x08
#define pPCEvents     0x04
#define pIdent        0x02
#define pTeach        0x01

#define pRemote       0x80
#define pACDI         0x40
#define pDisplay      0x20
#define pSNIP         0x10
#define pCDI          0x08
#define pTraction     0x04
#define pFunction     0x02
#define pDCC          0x01

#define pSimpleTrain  0x80
#define pFuncConfig   0x40
#define pFirmwareUpgrade     0x20
#define pFirwareUpdateActive 0x10

extern void Olcb_softReset() __attribute__((weak));
extern void userUsrClear() __attribute__((weak));
extern void userMfgClear() __attribute__((weak));
extern void userSoftReset() __attribute__((weak));
extern void userHardReset() __attribute__((weak));
extern void userInitAll() __attribute__((weak));
extern void userFactoryReset() __attribute__((weak));
extern void userConfigWritten(uint32_t address, uint16_t length, uint16_t func) __attribute__((weak));
extern void pceCallback(uint16_t index)  __attribute__((weak));
extern void produceFromInputs() __attribute__((weak));


Event event[NUM_EVENT];             // operating flags
uint16_t eventIndex[NUM_EVENT];     // Sorted index to eventids

#endif /* OpenLCBHeader_h */
