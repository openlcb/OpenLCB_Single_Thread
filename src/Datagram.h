#ifndef Datagram_h
#define Datagram_h

//#pragma message("!!! compiling Datagram_h")

/**
 * Class for handling Datagrams
 *
 * This combines Receiver and Transmitter for now;
 * perhaps they need to be refactored separately later.
 *<p>
 * Basic state machine handles transmission and reception.
 */

#define DATAGRAM_LENGTH 72

// A partial datagram is abandoned after this long. S-9.7.3 says a loss
// timeout shall not be shorter than 3 seconds. Override for a faster bus.
#ifndef DATAGRAM_REASSEMBLY_TIMEOUT_MS
#define DATAGRAM_REASSEMBLY_TIMEOUT_MS 3000
#endif

// Datagram Rejected error codes, from S-9.7.3.
// Bit 0x2000 means temporary: the sender may resend.

#define DATAGRAM_REJECTED_PERMANENT_ERROR        0x1000
#define DATAGRAM_REJECTED_INFORMATION_LOGGED     0x1010
#define DATAGRAM_REJECTED_SOURCE_NOT_PERMITTED   0x1020
#define DATAGRAM_REJECTED_DATAGRAM_TYPE_NOT_ACCEPTED 0x1040
#define DATAGRAM_REJECTED_INVALID_ARGUMENTS      0x1080
#define DATAGRAM_REJECTED_BUFFER_FULL            0x2000
#define DATAGRAM_REJECTED_REASSEMBLY_TIMEOUT     0x2011
#define DATAGRAM_REJECTED_BUFFER_UNAVAILABLE     0x2020
#define DATAGRAM_REJECTED_OUT_OF_ORDER           0x2041

#define DATAGRAM_REJECTED_NO_RESEND_MASK         0x1000
#define DATAGRAM_REJECTED_RESEND_MASK            0x2000
#define DATAGRAM_REJECTED_TRANSPORT_ERROR_MASK   0x4000

#include "OlcbCanInterface.h"

class LinkControl;

class Datagram {
  public:
  /**
   * Ask for the transmit buffer. If
   * it's in use, indicated by returning a 0 pointer,
   * you must repeat the request later
   */
  uint8_t* getTransmitBuffer();
  
  /**
   * Send the filled transmit buffer.
   */
  void sendTransmitBuffer(int length, uint16_t destID);
  
  /**
   * Handle any routine processing that needs to be done.
   * Go through this in loop() to e.g. send pending messages
   */
  void check();

  /**
   * True when no transmit is reserved and no fragment is waiting.
   * getTransmitBuffer() sets reservedXmt, so do not call it only to probe.
   */
  bool transmitIdle() { return !reservedXmt && sendcount < 0; }
  
  /**
   * When a CAN frame is received, it should
   * be transferred to the PCER object via this method
   * so that it can handle the verification protocol.
   */
  //bool receivedFrame(OpenLcbCanBuffer* rcv);
  bool receivedFrame(OlcbCanInterface* rcv);
  
  Datagram(OlcbCanInterface* b, uint16_t (*callback)(uint8_t tbuf[DATAGRAM_LENGTH], uint16_t length, uint16_t from), LinkControl* link);
  
  private:
  OlcbCanInterface* buffer;
  LinkControl* link;
  
  uint8_t tbuf[DATAGRAM_LENGTH];
  int sendcount;
  int resendcount;
  uint16_t dest;
  uint8_t* tptr;
  bool first;
  bool reservedXmt;  // buffer reserved for transmission
  bool receiving;
  bool overlong;             // fragment did not fit in rbuf
  unsigned long startTime;   // millis() when reassembly began

  uint16_t fromAlias;
  
  uint8_t rbuf[DATAGRAM_LENGTH];
  uint8_t* rptr;
  
  void setDatagramReply(uint16_t srcAlias, uint16_t dstAlias);
  void setDatagramAck(uint16_t srcAlias, uint16_t dstAlias);
  void setDatagramNak(uint16_t srcAlias, uint16_t dstAlias, uint16_t code);
  void abortReceive();
  
  // returns 0 for used, OK; > 0 for specific error message
    uint16_t (*callback)(uint8_t tbuf[DATAGRAM_LENGTH], uint16_t length, uint16_t from);   // void callback(int index) pointer
};

#endif
