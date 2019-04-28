//
//  OlcbEthInterface.cpp
//  Interface Net libraries
//
//  Created by David Harris on 2018-01-22.
//

/* commented out for now - dph

//#pragma message("!!! compiling OlcbEthInterface.cpp")

#include "OlcbNet.h"

class NodeID;
class EventID;


// Class to handle transforming OpenLCB (S9.6) frames to/from std Ethernet frames.
// <p>
// We're trying to localize the formating of frames to/from the node here,
// so that only this class needs to change when/if the wire protocol changes.

// Initialize a buffer for transmission
    void OlcbEthInterface::init(NodeID _nid){
        thisNid = nid;
        // start Ethernet?
    }

// Start of basic message structure

    void OlcbEthInterface::setFrameTypeOpenLcb() {
        mti |= OPENLCBFRAME;
    }
    bool OlcbEthInterface::isFrameTypeOpenLcb() {
        return mti & OPENLCBFRAME;
    }
    void OlcbEthInterface::setSource(NodeID nid) {
        src = nid;
    }
    NodeID OlcbEthInterface::getSource() {
        return src;
    }

// End of basic message structure

// Start of OpenLCB format support

    uint8_t getOpenLcbFormat() {  // 0-7 value
        return (mti>>OPENLCBFORMATSHIFT)&0x7;
    }
    void setOpenLcbFormat(uint8_t i) {  // 0-7 value
        this->mti |= i<<OPENLCBFORMATSHIFT;
    }

    void setDest(NodeID a) {
        dst = a;
    }
    NodeID uint32_t getDest() {
        return dst;
    }

    void setOpenLcbMTI(uint16_t _mti) {  // 12 bit MTI, but can take 16
        mti = _mti;
    }
    uint16_t getOpenLcbMTI() {
        return mti;
    }
    bool isOpenLcbMTI(uint16_t mti) {  // efficient check
        //??
    }

//virtual bool isForHere(uint16_t thisAlias);  // include OpenLCB messages and CAN control frames
    bool isForHere(NodeID _thisNode) { // include OpenLCB messages and CAN control frames
        return thisNode.nid = _thisNode.nid;
    }

//virtual bool isMsgForHere(uint16_t thisAlias);  // OpenLCB messages only
    bool isMsgForHere(NodeID _thisNode) {  // OpenLCB messages only
        return thisNode.nid = _thisNode.nid;
    }

    bool isAddressedMessage() {  // OpenLCB messages only
        return (mti>>ADDRESSEDMESSAGESHIFT)&0x1;
    }
    void getEventID(EventID* evt) { // loads return value into given variable
        memcpy(evt, data, 8);
    }
    void getNodeID(NodeID* nid) { // loads return value into given variable
        nid = thisNid;
    }
    bool matchesNid(NodeID* nid) {
        return thisNod.nid == nid.nid;
    }

// End of OpenLCB format support

// Start of OpenLCB messages
//
// These neither set nor test the source/destination addresses.
// Check separately for whether this frame is addressed to
// the current node (or global).
//
    void setInitializationComplete(NodeID* nid) {
        mti = INITIALIZATIONCOMPLETE;
    }
    bool isInitializationComplete() {
        return mti==INITIALIZATIONCOMPLETE;
    }

    void setPCEventReport(EventID* eid) {
        mti = PCEREPORT;
        memcpy(data,eid,8);
    }
    bool isPCEventReport() {
        return mti==PCEREPORT;
    }

virtual void setLearnEvent(EventID* eid);
virtual bool isLearnEvent();

virtual bool isVerifyNID();

virtual void setVerifiedNID(NodeID* nid);
virtual bool isVerifiedNID();

//virtual void setOptionalIntRejected(OpenLcbCanBuffer* rcv, uint16_t code);
virtual void setOptionalIntRejected(OlcbNet* rcv, uint16_t code);

virtual bool isIdentifyConsumers();

virtual void setConsumerIdentified(EventID* eid);

// Mask uses an EventID data structure; 1 bit means mask out when routing
virtual void setConsumerIdentifyRange(EventID* eid, EventID* mask);

virtual bool isIdentifyProducers();

virtual void setProducerIdentified(EventID* eid);

// Mask uses an EventID data structure; 1 bit means mask out when routing
virtual void setProducerIdentifyRange(EventID* eid, EventID* mask);

virtual bool isIdentifyEvents();

virtual void setDatagram(NodeID src, NodeID dst, uint16_t len, void* data);
virtual void isDatagram();
virtual void setDatagramReply(NodeID src, NodeID dst);
virtual void isDatagramReply();
virtual void setDatagramAck(NodeID src, NodeID dst);
virtual void isDatagramAck();
virtual void setDatagramNak(NodeID src, NodeID dst);
virtual void isDatagramNak();


virtual bool isDatagramFrame();
virtual bool isLastDatagramFrame();


protected:
//unsigned int nodeAlias;   // Initialization complete sets, all later use

// service routines

// copy content (0-7) to a previously-allocated Eid
virtual void loadFromEid(EventID* eid);

};
*/
