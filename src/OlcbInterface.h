//
//  OlcbInterfaceClass.h
//  Interface Net libraries
//
//  Created by David Harris on 2018-01-22.
//
//
#ifndef OLCBINTERFACECLASS_H
#define OLCBINTERFACECLASS_H

//#pragma message("!!! compiling OlcbInterfaceClass.h")

    #include "OlcbNet.h"

    class NodeID;
    class EventID;
    
    /**
     * Class to handle transforming OpenLCB (S9.6) frames to/from std CAN frames.
     * <p>
     * We're trying to localize the formating of frames to/from the node here,
     * so that only this class needs to change when/if the wire protocol changes.
     */
class OlcbInterface { // }: public OlcbNet {
  public:

    OlcbNet* net;
    // Initialize a buffer for transmission
    virtual void init(NodeID nid) = 0;
        
    // Start of basic message structure
        
    virtual void setFrameTypeOpenLcb() = 0;
    virtual bool isFrameTypeOpenLcb() = 0;
        
        virtual void setSource(NodeID nid) = 0;
        virtual NodeID getSource() = 0;
        
        // End of basic message structure
        
        // Start of OpenLCB format support
        
        virtual uint8_t getOpenLcbFormat() = 0;  // 0-7 value
        virtual void setOpenLcbFormat(uint8_t i) = 0;   // 0-7 value
        
        //virtual void setDest(uint32_t a) = 0;  // needs format already set = 0; sets length >= 2 if needed
        virtual void setDest(NodeID* nid) = 0;  // needs format already
        virtual NodeID getDest() = 0;
        
        virtual void setOpenLcbMTI(uint16_t mti) = 0;  // 12 bit MTI, but can take 16
        virtual uint16_t getOpenLcbMTI() = 0;
        virtual bool isOpenLcbMTI(uint16_t mti) = 0;  // efficient check
        
        //virtual bool isForHere(uint16_t thisAlias) = 0;  // include OpenLCB messages and CAN control frames
        virtual bool isForHere(NodeID* thisNode) = 0;  // include OpenLCB messages and CAN control frames
        
        //virtual bool isMsgForHere(uint16_t thisAlias) = 0;  // OpenLCB messages only
        virtual bool isMsgForHere(NodeID* thisNode) = 0;  // OpenLCB messages only
        
        virtual bool isAddressedMessage() = 0;  // OpenLCB messages only
        
        virtual void getEventID(EventID* evt) = 0; // loads return value into given variable
        virtual void getNodeID(NodeID* nid) = 0; // loads return value into given variable
        virtual bool matchesNid(NodeID* nid) = 0;
        
        // End of OpenLCB format support
        
        // Start of OpenLCB messages
        //
        // These neither set nor test the source/destination aliases.
        // Check separately for whether this frame is addressed to
        // the current node (or global).
        //
        virtual void setInitializationComplete(NodeID* nid) = 0;
        virtual bool isInitializationComplete() = 0;
        
        virtual void setPCEventReport(EventID* eid) = 0;
        virtual bool isPCEventReport() = 0;
        
        virtual void setLearnEvent(EventID* eid) = 0;
        virtual bool isLearnEvent() = 0;
        
        virtual bool isVerifyNID() = 0;
        
        virtual void setVerifiedNID(NodeID* nid) = 0;
        virtual bool isVerifiedNID() = 0;
        
        //virtual void setOptionalIntRejected(OpenLcbCanBuffer* rcv, uint16_t code) = 0;
        virtual void setOptionalIntRejected(OlcbInterface* rcv, uint16_t code) = 0;
        
        virtual bool isIdentifyConsumers() = 0;
        
        virtual void setConsumerIdentified(EventID* eid, uint8_t state) = 0;
        
        // Mask uses an EventID data structure = 0; 1 bit means mask out when routing
        virtual void setConsumerIdentifyRange(EventID* eid, EventID* mask) = 0;
        
        virtual bool isIdentifyProducers() = 0;
        
        virtual void setProducerIdentified(EventID* eid, uint8_t state) = 0;
        
        // Mask uses an EventID data structure; 1 bit means mask out when routing
        virtual void setProducerIdentifyRange(EventID* eid, EventID* mask) = 0;
        
        virtual bool isIdentifyEvents() = 0;
/* not implemented, yet
        virtual void setDatagram(NodeID src, NodeID dst, uint16_t len, void* data) = 0;
        virtual void isDatagram() = 0;
        virtual void setDatagramReply(NodeID src, NodeID dst) = 0;
        virtual void isDatagramReply() = 0;
        virtual void setDatagramAck(NodeID src, NodeID dst) = 0;
        virtual void isDatagramAck() = 0;
        virtual void setDatagramNak(NodeID src, NodeID dst) = 0;
        virtual void isDatagramNak() = 0;
*/

        virtual bool isDatagramFrame() = 0;
        virtual bool isLastDatagramFrame() = 0;

    
    protected:
        //uint16_t nodeAlias;   // Initialization complete sets, all later use
        
        // service routines
        
        // copy content (0-7) to a previously-allocated Eid
        virtual void loadFromEid(EventID* eid) = 0;
    
};

#endif // OLCBINTERFACECLASS_H
