//
//  OlcbCanInterface.h
//  Interface Net libraries
//
//  Created by David Harris on 2018-01-22.
//
//
#ifndef OLCBCANINTERFACE_H
#define OLCBCANINTERFACE_H

#include "NodeID.h"
#include "OlcbInterface.h"
#include "OlcbCan.h"

 class OlcbCanInterface : public OlcbInterface {
   public:

    OlcbCan* net;
    OlcbCanInterface(OlcbCan* _net);

    // Initialize a buffer for transmission
    void init(uint16_t alias);
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

    void setDest(NodeID* a){};  // needs format already set; sets length >= 2 if needed
    NodeID getDest(){ return NodeID();};

        void setOpenLcbMTI(uint16_t mti);  // 12 bit MTI, but can take 16
        uint16_t getOpenLcbMTI();
        bool isOpenLcbMTI(uint16_t mti);  // efficient check

        bool isForHere(NodeID* thisNode);  // include OpenLCB messages and CAN control frames

        bool isMsgForHere(NodeID* thisNode);  // OpenLCB messages only

        bool isAddressedMessage();  // OpenLCB messages only

        void getEventID(EventID* evt); // loads return value into given variable
        void getNodeID(NodeID* nid); // loads return value into given variable
        bool matchesNid(NodeID* nid);

        // End of OpenLCB format support

        // Start of OpenLCB messages
        //
        // These neither set nor test the source/destination aliases.
        // Check separately for whether this frame is addressed to
        // the current node (or global).
        //
        void setInitializationComplete(NodeID* nid);
        bool isInitializationComplete();

        void setPCEventReport(EventID* eid);
        bool isPCEventReport();

        void setLearnEvent(EventID* eid);
        bool isLearnEvent();

        bool isVerifyNID();

        void setVerifiedNID(NodeID* nid);
        bool isVerifiedNID();

        void setOptionalIntRejected(OlcbInterface* rcv, uint16_t code);
        void setOptionalIntRejected(OlcbCanInterface* rcv, uint16_t code);

        bool isIdentifyConsumers();

        void setConsumerIdentified(EventID* eid);

        // Mask uses an EventID data structure; 1 bit means mask out when routing
        void setConsumerIdentifyRange(EventID* eid, EventID* mask);

        bool isIdentifyProducers();

        void setProducerIdentified(EventID* eid);

        // Mask uses an EventID data structure; 1 bit means mask out when routing
        void setProducerIdentifyRange(EventID* eid, EventID* mask);

        bool isIdentifyEvents();

        bool isDatagramFrame();
        bool isLastDatagramFrame();

        

      private:
        // service routines

        // copy content (0-7) to a previously-allocated Eid
        void loadFromEid(EventID* eid);


      public:
        // CAN specific messages

        void setFrameTypeCAN();
        bool isFrameTypeCAN();

        void setFrameTypeCAN(uint16_t alias, uint16_t varField);

        void setVariableField(uint16_t f);
        uint16_t getVariableField();

        void setSourceAlias(uint16_t a);
        uint16_t getSourceAlias();

        void setDestAlias(uint16_t a);  // needs format already set; sets length >= 2 if needed
        uint16_t getDestAlias();

        bool isForHere(uint16_t thisAlias);  // include OpenLCB messages and CAN control frames
        bool isMsgForHere(uint16_t thisAlias);  // OpenLCB messages only


        void setCIM(uint8_t i, uint16_t testval, uint16_t alias);
        bool isCIM();

        void setRIM(uint16_t alias);
        bool isRIM();

        void setAMD(uint16_t alias,NodeID* nid);
        bool isAMD(uint16_t alias);

        void setAMR(uint16_t alias,NodeID* nid);
        void setAMR(NodeID* nid);
        bool isAMR(uint16_t alias);


    };


#endif // OLCBCANINTERFACE_H