#ifndef OlcbStream_h
#define OlcbStream_h

/**
 * Class for handling OpenLCB Streams
 *
 * This combines Receiver and Transmitter for now;
 * perhaps they need to be refactored separately later.
 *<p>
 * Basic state machine handles transmission and reception.
 *<p>
 * This implementation works with the protocol buffers
 * without keeping a local copy. Instead, it forwards
 * individual frames as they arrive.
 */

class OpenLcbCanBuffer;
class LinkControl;

class OlcbStream {
  public:
  
  OlcbStream(OlcbInterface* b, uint16_t (*rcvData)(uint8_t *tbuf, uint16_t length), LinkControl* link);
  
  void check(); 
  bool receivedFrame(OlcbInterface* rcv);
  
  private:
  OlcbInterface* buffer;
  uint16_t (*callback)(uint8_t *tbuf, uint16_t length); 

};

#endif
