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
  
  OlcbStream(OlcbInterface* b, unsigned int (*rcvData)(uint8_t *tbuf, unsigned int length), LinkControl* link);
  
  void check(); 
  bool receivedFrame(OlcbInterface* rcv);
  
  private:
  OlcbInterface* buffer;
  unsigned int (*callback)(uint8_t *tbuf, unsigned int length); 

};

#endif
