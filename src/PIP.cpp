#include <string.h>

#include "PIP.h"

#include "LinkControl.h"
#include "OlcbCanInterface.h"
#include "OpenLcbCan.h"

static LinkControl* clink;
static OlcbCanInterface* buffer;
static uint16_t dest;
static bool queued;

#define MTI_PIP_REQUEST 0x828
#define MTI_PIP_REPLY   0x668

//extern "C" {
extern uint8_t protocolIdentValue[6];
//}


void PIP_setup(OlcbCanInterface* b, LinkControl* li) {
      buffer = b;
      clink = li;
      queued = false;
  }
  
  void PIP_check() {
    if (queued) {
        if (buffer->net->txReady()) {
            buffer->setOpenLcbMTI(MTI_PIP_REPLY);
            buffer->setDestAlias(dest);
            memcpy( &(buffer->net->data[0])+2, protocolIdentValue, 6);
            buffer->net->length = 8;
            buffer->net->write(100);
            queued = false;
        }
    }
    return;
  }
    
  bool PIP_receivedFrame(OlcbCanInterface* rcv) {
    if ( rcv->isOpenLcbMTI(MTI_PIP_REQUEST) )  {
                    //Serial.print("\n PIP_receivedFrame");
        // previously checked for this node
        // check to start bit set active, ignore if not
        if ( ((rcv->net->data[0]) & 0x20) != 0) return true;
        queued = true;
        dest = rcv->getSourceAlias();
        return true;
    }
    return false;
  }

