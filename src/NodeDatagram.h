#ifndef NodeDatagram_h
#define NodeDatagram_h

// Optional client of the one Datagram transport created in OpenLCBMid.h.
//
// Include this header once, after #include "OpenLCBMid.h". It defines
// userDatagramReceived and userAliasChanged. Those definitions stay in
// this header: Arduino compiles every src/*.cpp, and a strong symbol in
// a library .cpp would collide with a sketch that defines the hook itself.
// A sketch that does not include this header keeps the weak hooks.
//
// Do not construct a second Datagram. Memory Configuration uses `dg` for
// content id 0x20. This client queues other datagrams and transmits them
// from check(), after the receive handler has returned. The library sends
// Datagram Received OK with txBuffer as soon as that handler returns.
//
// Several NodeDatagram objects form one linked list. Each is constructed
// with an EEPROM address and a receive handler:
//
//     NodeDatagram(uint16_t eventAddr, Handler handler)
//
// eventAddr is a CDI event id that is not listed in eidtab, so this node
// does not produce or consume it. The first six bytes are the peer Node ID
// in the order JMRI shows and EventID::setNodeIdPrefix writes. The last
// two event bytes are ignored. send() reads the six bytes from EEPROM at
// call time via NODECONFIG.read. An all-zero id makes send() return false.
// There is no userConfigWritten hook; a CDI edit applies on the next send().
//
// send(contentId, data, len) and reply(contentId, data, len) copy the
// payload and return. `data` is the bytes after the content id. Both refuse
// content id 0x20 and a datagram longer than DATAGRAM_LENGTH (72, including
// the content id). One datagram is queued per object. reply() is valid only
// from inside the receive handler. It captures the source alias immediately
// and replaces a send that is still queued, so a pending ping cannot block
// the answer.
//
// check() does nothing until clink.linkInitialized(). Call it from loop()
// after Olcb_process(). To send to a Node ID it transmits an Alias Map
// Enquiry and retries about once a second until an Alias Map Definition
// arrives. Each object stores only the alias it resolved. While a datagram
// transmit is reserved, check() does not send an enquiry. One CAN frame is
// started per check() pass, either the enquiry or the datagram.
//
// On receive, 0x20 never reaches a handler. The handler sees the bytes
// after the content id and returns 0 to accept. The first handler that
// returns 0 accepts. If every handler returns non-zero, the library rejects
// the datagram with 0x1040. Copy anything you need out of the buffer before
// returning; it is the reassembly buffer.

#include <string.h>
#include <Arduino.h>
#include "NodeID.h"
#include "NodeMemory.h"
#include "LinkControl.h"
#include "Datagram.h"
#include "OpenLcbCan.h"

class NodeDatagram {
  public:
  // contentId is the datagram's first byte. data/length are the bytes
  // after it. from is the source alias. Return 0 to accept.
  typedef uint16_t (*Handler)(uint8_t contentId, uint8_t *data, uint16_t length, uint16_t from);

  NodeDatagram(uint16_t eventAddr, Handler handler);
  bool send(uint8_t contentId, const uint8_t *data, uint16_t len);
  bool reply(uint8_t contentId, const uint8_t *data, uint16_t len);
  static void check();

  private:
  enum { MEMORY_CONFIGURATION = 0x20 };

  static NodeDatagram *head;
  static bool inHandler;
  static uint16_t handlerFrom;

  NodeDatagram *next;
  uint16_t eventAddr;
  Handler handler;

  uint8_t peer[6];
  bool peerKnown;
  bool aliasValid;
  uint16_t alias;
  unsigned long lastAme;
  bool ameStarted;

  bool pending;
  bool pendingToPeer;       // false: reply to pendingAlias
  uint16_t pendingAlias;
  uint8_t pendingLen;
  uint8_t pendingBuf[DATAGRAM_LENGTH];

  bool payloadOk(uint8_t contentId, uint16_t len) const;
  bool readPeer(uint8_t id[6]) const;
  void store(uint8_t contentId, const uint8_t *data, uint16_t len);
  void noteAlias(NodeID *nid, uint16_t reported);
  bool ameDue() const;
  void sendAme();
  void transmit();

  static uint16_t receive(uint8_t *data, uint16_t length, uint16_t from);
  static void aliasChanged(NodeID *nid, uint16_t alias);

  friend uint16_t userDatagramReceived(uint8_t *data, uint16_t length, uint16_t from);
  friend void userAliasChanged(NodeID *nid, uint16_t alias);
};

NodeDatagram *NodeDatagram::head = 0;
bool NodeDatagram::inHandler = false;
uint16_t NodeDatagram::handlerFrom = 0;

NodeDatagram::NodeDatagram(uint16_t eventAddr, Handler handler)
  : next(0), eventAddr(eventAddr), handler(handler),
    peerKnown(false), aliasValid(false), alias(0),
    lastAme(0), ameStarted(false),
    pending(false), pendingToPeer(false), pendingAlias(0), pendingLen(0) {
  memset(peer, 0, sizeof(peer));
  memset(pendingBuf, 0, sizeof(pendingBuf));
  if (!head) {
    head = this;
  } else {
    NodeDatagram *p = head;
    while (p->next) p = p->next;
    p->next = this;
  }
}

bool NodeDatagram::payloadOk(uint8_t contentId, uint16_t len) const {
  if (contentId == MEMORY_CONFIGURATION) return false;
  if (len > (uint16_t)(DATAGRAM_LENGTH - 1)) return false;
  return true;
}

bool NodeDatagram::readPeer(uint8_t id[6]) const {
  bool any = false;
  for (uint8_t i = 0; i < 6; i++) {
    id[i] = NODECONFIG.read(eventAddr + i);
    if (id[i] != 0) any = true;
  }
  return any;
}

void NodeDatagram::store(uint8_t contentId, const uint8_t *data, uint16_t len) {
  pendingBuf[0] = contentId;
  if (len && data) memcpy(pendingBuf + 1, data, len);
  pendingLen = (uint8_t)(len + 1);
}

bool NodeDatagram::send(uint8_t contentId, const uint8_t *data, uint16_t len) {
  if (!payloadOk(contentId, len)) return false;
  uint8_t id[6];
  if (!readPeer(id)) return false;
  if (pending) return false;
  if (!peerKnown || memcmp(peer, id, 6) != 0) {
    memcpy(peer, id, 6);
    peerKnown = true;
    aliasValid = false;
    ameStarted = false;
  }
  store(contentId, data, len);
  pendingToPeer = true;
  pending = true;
  return true;
}

bool NodeDatagram::reply(uint8_t contentId, const uint8_t *data, uint16_t len) {
  if (!inHandler) return false;
  if (!payloadOk(contentId, len)) return false;
  store(contentId, data, len);
  pendingAlias = handlerFrom;
  pendingToPeer = false;
  pending = true;
  return true;
}

void NodeDatagram::noteAlias(NodeID *nid, uint16_t reported) {
  if (!peerKnown || !nid) return;
  if (memcmp(peer, nid->val, 6) != 0) return;
  if (reported == 0) {
    aliasValid = false;
    ameStarted = false;
    return;
  }
  alias = reported;
  aliasValid = true;
}

bool NodeDatagram::ameDue() const {
  if (!ameStarted) return true;
  return (millis() - lastAme) >= 1000;
}

void NodeDatagram::sendAme() {
  if (!peerKnown) return;
  if (!dg.transmitIdle()) return;
  if (!txBuffer.net->txReady()) return;
  txBuffer.setFrameTypeCAN(clink.getAlias(), AME_VAR_FIELD);
  memcpy(txBuffer.net->data, peer, 6);
  txBuffer.net->length = 6;
  txBuffer.net->write(200);
  lastAme = millis();
  ameStarted = true;
}

void NodeDatagram::transmit() {
  if (pendingToPeer && !aliasValid) return;
  if (!dg.transmitIdle()) return;
  if (!txBuffer.net->txReady()) return;
  uint8_t *b = dg.getTransmitBuffer();
  if (!b) return;
  memcpy(b, pendingBuf, pendingLen);
  uint16_t dest = pendingToPeer ? alias : pendingAlias;
  dg.sendTransmitBuffer(pendingLen, dest);
  pending = false;
}

void NodeDatagram::check() {
  if (!clink.linkInitialized()) return;
  // A reserved datagram transmit owns txBuffer. Do not enquire, and do
  // not call getTransmitBuffer() until the transport is idle.
  if (!dg.transmitIdle()) return;

  NodeDatagram *enquiry = 0;
  for (NodeDatagram *p = head; p; p = p->next) {
    if (!p->pending) continue;
    if (!p->pendingToPeer || p->aliasValid) {
      if (txBuffer.net->txReady()) p->transmit();
      return; // one datagram this pass
    }
    if (!enquiry && p->ameDue()) enquiry = p;
  }
  if (enquiry && txBuffer.net->txReady()) enquiry->sendAme();
}

uint16_t NodeDatagram::receive(uint8_t *data, uint16_t length, uint16_t from) {
  if (!data || length < 1) return 1;
  uint8_t contentId = data[0];
  uint8_t *payload = data + 1;
  uint16_t plen = (uint16_t)(length - 1);
  inHandler = true;
  handlerFrom = from;
  uint16_t result = 1;
  for (NodeDatagram *p = head; p; p = p->next) {
    if (p->handler && p->handler(contentId, payload, plen, from) == 0) {
      result = 0;
      break;
    }
  }
  inHandler = false;
  return result;
}

void NodeDatagram::aliasChanged(NodeID *nid, uint16_t reported) {
  for (NodeDatagram *p = head; p; p = p->next)
    p->noteAlias(nid, reported);
}

// Strong definitions of the weak hooks. Included once by the sketch.
uint16_t userDatagramReceived(uint8_t *data, uint16_t length, uint16_t from) {
  return NodeDatagram::receive(data, length, from);
}

void userAliasChanged(NodeID *nid, uint16_t alias) {
  NodeDatagram::aliasChanged(nid, alias);
}

#endif
