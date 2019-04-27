//
//  OlcbEthInterface.h
//  Interface Net libraries
//
//  Created by David Harris on 2018-01-22.
//
//
#ifndef OLCBETHINTERFACE_H
#define OLCBETHINTERFACE_H

//#pragma message("!!! compiling OlcbEthInterface.h")

#include "OlcbNet.h"

class NodeID;
class EventID;

/**
* Class to handle transforming OpenLCB (S9.6) frames to/from std Ethernet frames.
* <p>
* We're trying to localize the formating of frames to/from the node here,
* so that only this class needs to change when/if the wire protocol changes.
*/
class OlcbInterface : public OlcbNet {
public:
    NodeID thisNid;
    NodeiD src;
    NodeID dst;
    NodeID eid;
    int len;
    uint8_t* data;



// Initialize a buffer for transmission
void init(NodeID nid);

// Start of basic message structure

 void setFrameTypeOpenLcb();
 bool isFrameTypeOpenLcb();

 void setSource(NodeID nid);
 NodeID getSource();

// End of basic message structure

// Start of OpenLCB format support

 uint8_t getOpenLcbFormat();  // 0-7 value
 void setOpenLcbFormat(uint8_t i);   // 0-7 value

 void setDest(NodeID nid);  // needs format already
 NodeID getDest();

virtual void setOpenLcbMTI(uint16_t mti);  // 12 bit MTI, but can take 16
virtual uint16_t getOpenLcbMTI();
virtual bool isOpenLcbMTI(uint16_t mti);  // efficient check

//virtual bool isForHere(uint16_t thisAlias);  // include OpenLCB messages and CAN control frames
virtual bool isForHere(NodeID* thisNode);  // include OpenLCB messages and CAN control frames

//virtual bool isMsgForHere(uint16_t thisAlias);  // OpenLCB messages only
virtual bool isMsgForHere(NodeID* thisNode);  // OpenLCB messages only

virtual bool isAddressedMessage();  // OpenLCB messages only

virtual void getEventID(EventID* evt); // loads return value into given variable
virtual void getNodeID(NodeID* nid); // loads return value into given variable
virtual bool matchesNid(NodeID* nid);

// End of OpenLCB format support

// Start of OpenLCB messages
//
// These neither set nor test the source/destination aliases.
// Check separately for whether this frame is addressed to
// the current node (or global).
//
virtual void setInitializationComplete(NodeID* nid);
virtual bool isInitializationComplete();

virtual void setPCEventReport(EventID* eid);
virtual bool isPCEventReport();

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

    const mti[] = {
        OPENLCBFRAME = 0x100000;
        OPENLCBFORMATSHIFT = 0x0;
        ADDRESSEDMESSAGESHIFT = 0x0;

        INITIALIZATIONCOMPLETE = 0x0000;
    };



};

#endif // OLCBETHINTERFACE_H