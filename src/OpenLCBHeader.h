//
//  OpenLCBHeader.h
//  
//
//  Created by David Harris on 2018-01-01.
//
//

#ifndef OpenLCBHeader_h
#define OpenLCBHeader_h

//#pragma message "Compiling OpenLCBHeader.h"

bool eepromDirty;

#include "processor.h"
#include "OlcbCommonVersion.h"
#include "BG.h"
#include "ButtonLed.h"

#include "OpenLcbCore.h"

// specific OpenLCB implementations
//#include "LinkControl.h"

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
        <softwareVersion>" SWVERSION " Lib:" OlcbCommonVersion "</softwareVersion>\
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
extern void pceCallback(uint16_t index) __attribute__((weak));
extern void produceFromInputs() __attribute__((weak));
extern uint8_t userState(uint16_t) __attribute__((weak));

// Optional sketch hook for datagrams other than Memory Configuration.
//
// The library has one Datagram transport, the global `dg` created in
// OpenLCBMid.h. Olcb_process() gives it every datagram frame addressed to
// this node. Memory Configuration, CDI, and SNII own content ID 0x20
// (CONFIGURATION_DATAGRAM_CODE). This hook is called only for a datagram
// whose first byte is not 0x20, and for a datagram that has no bytes.
// Define it in the sketch the same way as userConfigWritten(). Leave it
// undefined and those datagrams are rejected with 0x1040 (permanent error,
// datagram type not accepted), the same result as before this hook existed.
//
// Use this `dg` object for sketch datagrams. A second Datagram object that
// also receives frames will ACK or NAK the same transfer as the library,
// and CDI reads and writes then fail.
//
// Return 0 to accept the datagram. The transport then sends Datagram
// Received OK. Any non-zero return is sent as Datagram Rejected 0x1040.
// The returned number is not put on the wire, so this hook cannot ask for
// a temporary "buffer busy, please resend". Copy `data` before returning.
// That array is the reassembly buffer, and the next datagram overwrites it.
//
// Send a reply from loop(), after Olcb_process() has run, with the same
// `dg`. The name `dg` exists only below #include "OpenLCBMid.h".
//
//     uint8_t *b = dg.getTransmitBuffer();
//     if (b) {
//         b[0] = 0xF0;             // content ID, then the payload
//         // b[1..] = six-byte protocol id, then your bytes
//         dg.sendTransmitBuffer(n, destAlias);
//     }
//
// getTransmitBuffer() returns null while a CDI read reply still holds the
// one transmit buffer. Retry on a later pass of loop(). Olcb_process()
// already calls dg.check(), which fragments the datagram and releases the
// buffer when the other node sends Datagram Received OK. A sketch that
// keeps the buffer and never gets that OK stalls CDI reads. Writes to
// configuration memory do not need the transmit buffer.
//
// Byte 0 is the Datagram Content ID (OpenLCB S-9.7.3.2). Assigned one-byte
// IDs include:
//     0x01 Log Request, 0x02 Log Reply,
//     0x20 Memory Configuration (CDI, configuration memory, SNII),
//     0x21 Remote Button, 0x28 Display, 0x30 Train Control.
// For a private protocol between nodes you control, start with 0xF0-0xFF
// and follow it with six bytes from a unique-ID range you already own,
// then the payload. That form is the user-allocated range in the OpenLCB
// datagram protocol allocation table, so a later one-byte assignment will
// not take it. Leave the PIP flags alone. A private type has no PIP bit.
//
// `from` is the source node's current CAN alias. Aliases are reused, so
// send a reply while that alias still belongs to the node you heard.
extern uint16_t userDatagramReceived(uint8_t *data, uint16_t length, uint16_t from) __attribute__((weak));


Event event[NUM_EVENT];             // operating flags
uint16_t eventIndex[NUM_EVENT];     // Sorted index to eventids

#endif /* OpenLCBHeader_h */
