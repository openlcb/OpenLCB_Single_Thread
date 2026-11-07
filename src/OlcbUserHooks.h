#ifndef OlcbUserHooks_h
#define OlcbUserHooks_h

#include <stdint.h>

// Optional sketch functions. Each reference must be weak: a plain extern
// in a .cpp forces the linker to require the function. A missing function
// has address 0, so call sites test the pointer before calling.
// C++ linkage. A normal sketch definition overrides the weak symbol.

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
extern uint8_t userState(uint16_t index) __attribute__((weak));

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
// NodeDatagram.h is an optional client of this same `dg`. Include that
// header once, after OpenLCBMid.h. It defines userDatagramReceived and
// userAliasChanged, so a sketch that includes it does not define those
// hooks itself. A sketch that does not include it keeps the weak hooks.
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

// Optional sketch hook for CAN alias map changes.
//
// LinkControl::receivedFrame calls this after its Alias Map Enquiry reply
// when the frame is an Alias Map Definition or Alias Map Reset with at
// least six data bytes. `nid` is those six bytes, in the order JMRI shows.
// For an AMD, `alias` is the source alias. For an AMR, `alias` is 0 and
// that Node ID no longer owns an alias. Aliases are temporary.
// LinkControl::getAlias() is this node only; the library keeps no map of
// other nodes. The pointer is null when the sketch does not define this,
// and the call site does not call it then. NodeDatagram.h defines it.
class NodeID;
extern void userAliasChanged(NodeID *nid, uint16_t alias) __attribute__((weak));

#endif
