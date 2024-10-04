#include "OlcbInterface.h"

#include "LinkControl.h"
#include "OlcbStream.h"

// ToDo: Implementation incomplete

OlcbStream::OlcbStream(OlcbInterface* b, uint16_t (*cb)(uint8_t *tbuf, uint16_t length), LinkControl* ln) {
}

void OlcbStream::check() {
    // see if can send.
}

bool OlcbStream::receivedFrame(OlcbInterface* rcv) {
    // check for init stream
    // check for stream data transfer headed here
    
    // check for init stream reply
    // check for stream ack to send more
    return false;
}

