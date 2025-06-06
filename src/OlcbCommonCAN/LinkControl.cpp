// The timing here is done in an Arduino-specific way via the 
// "millis" function, here advance defined
//#include <Arduino.h>

#include "LinkControl.h"

#include "OpenLcbCan.h"

#include "OlcbCan.h"

#include "NodeID.h"

#include "debugging.h"

// state machine definitions
#define STATE_INITIAL 0
#define STATE_WAIT_CONFIRM 10
#define STATE_ALIAS_ASSIGNED 20
#define STATE_PERMITTED 25
#define STATE_INITIALIZED 30

// time to wait between last CIM and RIM
#define CONFIRM_WAIT_TIME 200

//extern "C" {
//    extern bool can_init();
//    extern bool can_check_message(void);
//    extern bool can_check_free_buffer(void);
//    extern uint8_t can_send_message(const Can *msg);
//    extern uint8_t can_get_message(Can *msg);
//}

LinkControl::LinkControl(OlcbCanInterface* b, NodeID* n) {
  txBuffer = b;
  nid = n;
}

void LinkControl::nextAlias() {
    do {
       // step the PRNG
       // First, form 2^9*val
       uint32_t temp1 = ((lfsr1<<9) | ((lfsr2>>15)&0x1FF)) & 0xFFFFFF;
       uint32_t temp2 = (lfsr2<<9) & 0xFFFFFF;
       
       // add
       lfsr2 = lfsr2 + temp2 + 0x7A4BA9l;
       lfsr1 = lfsr1 + temp1 + 0x1B0CA3l;
       
       // carry
       lfsr1 = (lfsr1 & 0xFFFFFF) + ((lfsr2&0xFF000000) >> 24);
       lfsr2 = lfsr2 & 0xFFFFFF;

    } while (getAlias() == 0);  // force advance again if get zero alias
}

void LinkControl::reset() {
    //dP("\nIn LinkControl::reset");
  // initialize sequence from node ID
  lfsr1 = (((uint32_t)nid->val[0]) << 16) | (((uint32_t)nid->val[1]) << 8) | ((uint32_t)nid->val[2]);
  lfsr2 = (((uint32_t)nid->val[3]) << 16) | (((uint32_t)nid->val[4]) << 8) | ((uint32_t)nid->val[5]);
  
  if (getAlias() == 0) nextAlias(); // advancing one step here if get zero
}

void LinkControl::restart() {
  state = STATE_INITIAL;
  // advance the sequence
  nextAlias();
}

// send the next CIM message.  "i" is the 0-3 ordinal number of the message, which
// becomes F-C in the CIM itself. Returns true if successfully sent.
bool LinkControl::sendCIM(uint8_t i) {
    //dP("\nIn LinkControl::sendCIM");
  uint16_t fragment;
  switch (i) {
    case 0:  fragment = ( (nid->val[0]<<4)&0xFF0) | ( (nid->val[1] >> 4) &0xF);
             break;
    case 1:  fragment = ( (nid->val[1]<<8)&0xF00) | ( nid->val[2] &0xFF);
             break;
    case 2:  fragment = ( (nid->val[3]<<4)&0xFF0) | ( (nid->val[4] >> 4) &0xF);
             break;
    default:
    case 3:  fragment = ( (nid->val[4]<<8)&0xF00) | ( nid->val[5] &0xFF);
             break;
  }
  txBuffer->setCIM(i,fragment,getAlias());
  return sendFrame();
}

bool LinkControl::sendRIM() {
    //dP("\nIn LinkControl::sendRIM");
  txBuffer->setRIM(getAlias());
  return sendFrame();
}

bool LinkControl::sendInitializationComplete() {
    //dP("\n LinkControl::sendAMD()");
  txBuffer->setSourceAlias(getAlias());
  txBuffer->setInitializationComplete(nid);
  return sendFrame();
}

bool LinkControl::sendAMD() {
    //dP("\n LinkControl::sendAMD()");
  txBuffer->setAMD(getAlias(), nid);
  return sendFrame();
}

bool LinkControl::sendAMR() {
    //dP(" LinkControl::sendAMR()");
  txBuffer->setAMR(getAlias(), nid);
  return sendFrame();
}

bool LinkControl::sendFrame() {
    //dP("\n LinkControl::sendFrame()");

    if (!txBuffer->net->txReady()) return false; // couldn't send just now
    //dP("\n LinkControl::sendframe()#B");

    txBuffer->net->write(200); // wait for queue, but earlier check says will succeed
  return true;
}
#define P(x) {dP(" " #x "="); Serial.print(x);}
void LinkControl::check() {
    //dP("\nIn LinkControl::check");
  // find current state and act
  if (state == STATE_INITIALIZED) return;
  switch (state) {
  case STATE_INITIAL+0:
  case STATE_INITIAL+1:
  case STATE_INITIAL+2:
  case STATE_INITIAL+3:
    // send next CIM message if possible
    if (sendCIM(state-STATE_INITIAL))
      state++;
    return;
  case STATE_INITIAL+4:
    // last CIM, sent, wait for delay
    timer = millis();
    state = STATE_WAIT_CONFIRM; 

    return;
  case STATE_WAIT_CONFIRM:
          //dP("\n STATE_WAIT_CONFIRM");
          //P(millis());
          //P(timer+CONFIRM_WAIT_TIME);
          //delay(1);
    if ( (millis() > timer+CONFIRM_WAIT_TIME) && sendRIM()) {
      state = STATE_ALIAS_ASSIGNED;
    }
    return;
  case STATE_ALIAS_ASSIGNED:
          //dP("\n STATE_ALIAS_ASSIGNED");
    // send AMD to go to Permitted
    if (sendAMD()) {
        state = STATE_PERMITTED;
    }
  case STATE_PERMITTED:
          //dP("\n STATE_PERMITTED");
    // send init
    if (sendInitializationComplete()) {
      state = STATE_INITIALIZED;
    }
    return;
  default:
    return;
  }
}

bool LinkControl::linkInitialized() {
    //dP("\nIn LinkControl::reset");
  return state == STATE_INITIALIZED;
}

uint16_t LinkControl::getAlias() {
  return (lfsr1 ^ lfsr2 ^ (lfsr1>>12) ^ (lfsr2>>12) )&0xFFF;
}

void LinkControl::rejectMessage(OlcbCanInterface* rcv, uint16_t code) {
     // send OptionalInterationRejected; should be threaded, but isn't
     txBuffer->setOptionalIntRejected(rcv, code);
    txBuffer->net->write(200);
}

bool LinkControl::receivedFrame(OlcbCanInterface* rcv) {
    //dP("\nIn LinkControl::receivedFrame");
    uint16_t alias = getAlias();
   // check received message
   // see if this is a frame with our alias
   if ( alias == rcv->getSourceAlias()) {
     // somebody else trying to use this one, see to what extent
     if (rcv->isCIM()) {
       // somebody else trying to allocate, tell them
         //dP("\nreceivedFrame C");
       while (!sendRIM()) {}  // insist on sending it now.
     } else {
       // RIM frame or not RIM Frame, do same thing: Send AMR & restart
       sendAMR();
         //dP("\nreceivedFrame oops");
       restart();
     }
   }
   // check for aliasMapEnquiry
   else if (rcv->isFrameTypeCAN() && (rcv->getVariableField() == AME_VAR_FIELD)) {
      // check node ID matches or no node ID present
      if (rcv->net->length != 0 && (!rcv->matchesNid(nid))) return false;
      // reply
       //dP("\nreceivedFrame D");
      txBuffer->setAMD(alias, nid);
      txBuffer->net->write(200);
   }
   // check for aliasMapDefinition
   // check for aliasMapReset

   // see if this is a Verify request to us; first check type
   else if (rcv->isVerifyNID()) {
     // reply; should be threaded, but isn't
       //dP("\nisVerifyNID");
       //dP("\nreceivedFrame E");
     txBuffer->setVerifiedNID(nid);
     txBuffer->net->write(200);
     return true;
   }
   
   return false;
}

