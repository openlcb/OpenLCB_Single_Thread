//
//  OpenLCBHeader.h
//  
//
//  Created by David Harris on 2018-01-01.
//
//

#ifdef DEBUG
    #define dP(x) Serial.print(x)
    #define dPH(x) Serial.print(x,HEX)
    #define dPL Serial.println()
    #define dPN(x) Serial.print("\n" #x ":") Serial.print(x)
#else
    #define dP(x)
    #define dPH(x)
    #define dPL
    #define dPN(x)
#endif

#ifndef OpenLCBHeader_h
#define OpenLCBHeader_h

#include "OlcbCommonVersion.h"
#include "NodeID.h"

typedef struct NodeVar_ {
    uint32_t magic;         // used to check eeprom status
    uint16_t nextEID;       // the next available eventID for use from this node's set
    NodeID   nid;        // the nodeID
    char     nodeName[20];  // optional node-name, used by ACDI
    char     nodeDesc[24];  // optional node-description, used by ACDI
} NodeVar;

bool eepromDirty;

// ===== eventidOffset Support =====================================
//   Note: this allows system routines to initialize event[]
//         since eventOffsets are constant in flash.
typedef struct {
    uint16_t offset;
    uint16_t flags;
} EIDTab;


#include "OlcbCanInterface.h"
#include "PIP.h"
#include "SNII.h"
#include "PCE.h"

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
#include "PCE.h"
#include "PIP.h"
#include "SNII.h"
#include "BG.h"
#include "ButtonLed.h"
#include "Event.h"



// ===== CDI System Portions =======================================
#define CDIheader "\
    <cdi xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance' xsi:noNamespaceSchemaLocation='http://openlcb.org/trunk/prototypes/xml/schema/cdi.xsd'>\n\
    <identification>\n\
        <manufacturer>" MANU "</manufacturer>\
        <model>" MODEL "</model>\
        <hardwareVersion>" HWVERSION "</hardwareVersion>\
        <softwareVersion>" SWVERSION "</softwareVersion>\
        </identification>\
    <segment origin='12' space='253'> <!-- bypasses magic, nextEID, nodeID -->\
        <group>\
            <name>Node ID</name>\
            <description>User-provided description of the node</description>\
            <string size='20'><name>Node Name</name></string>\
            <string size='24'><name>Node Description</name></string>\
        </group>"
#define CDIfooter "\
    </segment>\
    <segment origin='0' space='253'> <!-- stuff magic to trigger resets -->\
        <name>Reset</name>\
        <description>Controls reloading and clearing node memory. Board must be restarted for this to take effect.</description>\
        <int size='4'>\
            <map>\
                <relation><property>3998572261</property><value>(No reset)</value></relation>\
                <relation><property>3998561228</property><value>User clear: New default EventIDs, blank strings</value></relation>\
                <relation><property>0</property><value>Mfg clear: Reset all, including Node ID</value></relation>\
            </map>\
        </int>\
    </segment>\
    </cdi>"


#define IDENT_FLAG 0x01

#define EEADDR(x)  ((uint16_t)(int)(&(((MemStruct*)0)->x)))
#define CEID(x)    {EEADDR(x), (uint16_t)Event::CAN_CONSUME_FLAG | IDENT_FLAG}
#define PEID(x)    {EEADDR(x), (uint16_t)Event::CAN_PRODUCE_FLAG | IDENT_FLAG}
#define PCEID(x)   {EEADDR(x), (uint16_t)Event::CAN_CONSUME_FLAG|Event::CAN_PRODUCE_FLAG | IDENT_FLAG}

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

#define FORCE_ALL_INIT 0

extern void configWritten(unsigned int address, unsigned int length, unsigned int func);
extern void Olcb_softReset() __attribute__((weak));
extern void userUsrClear() __attribute__((weak));
extern void userMfgClear() __attribute__((weak));
extern void userSoftReset() __attribute__((weak));
extern void userHardReset() __attribute__((weak));
extern void userInitAll() __attribute__((weak));
extern void userFactoryReset() __attribute__((weak));
extern void userConfigWritten(unsigned int address, unsigned int length, unsigned int func) __attribute__((weak));
extern void pceCallback(unsigned int index)  __attribute__((weak));
extern void produceFromInputs() __attribute__((weak));

extern PCE pce;

Event event[NUM_EVENT];             // operating flags
uint16_t eventIndex[NUM_EVENT];     // Sorted index to eventids


#endif /* OpenLCBHeader_h */
