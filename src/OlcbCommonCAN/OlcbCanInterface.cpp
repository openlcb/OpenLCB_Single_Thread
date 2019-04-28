//
//  OlcbCanInterface.cpp
//  CAN Interface
//
//  Created by David Harris on 2018-01-22.
//
//

#include <string.h>

#ifdef debug
#pragma message("!!! compiling OlcbCanInterface.cpp")
#endif


// The following line is needed because the Arduino environment
// won't search a library directory unless the library is included
// from the top level file (this file)
//#include "can.h"

#include "OpenLcbCan.h"

#include "OlcbCanInterface.h"
#include "OlcbCan.h"
#include "NodeID.h"
#include "EventID.h"

// for definiton, see
// http://openlcb.sf.net/trunk/documents/can/index.html
//
// In the following masks, bit 0 of the frame is 0x10000000L
//

// bit 1
#define MASK_FRAME_TYPE 0x08000000L

// bit 17-28
#define MASK_SRC_ALIAS 0x00000FFFL

// bit 2-16
#define MASK_VARIABLE_FIELD 0x07FFF000L
#define SHIFT_VARIABLE_FIELD 12

// bit 2-4, at the top of the variable field
#define MASK_OPENLCB_FORMAT 0x07000L
#define SHIFT_OPENLCB_FORMAT 12


OlcbCanInterface::OlcbCanInterface(OlcbCan* _net) {
    net = _net;
}

void OlcbCanInterface::init(uint16_t alias) {
    // set default header: extended frame w low priority
    net->flags.extended = 1;
    // no data yet
    net->length = 0;
    // all bits in header default to 1 except MASK_SRC_ALIAS
    net->id = 0x1FFFF000 | (alias & MASK_SRC_ALIAS);
}
void OlcbCanInterface::init(NodeID nid) {
    //init(nid.alias);
}

// start of basic message structure

void OlcbCanInterface::setFrameTypeOpenLcb() {
    net->id |= MASK_FRAME_TYPE;
}

bool OlcbCanInterface::isFrameTypeOpenLcb() {
    return (net->id & MASK_FRAME_TYPE) == MASK_FRAME_TYPE;
}


void OlcbCanInterface::setSource(NodeID nid) {
    //setSourceAlias(nid.alias);
}
NodeID OlcbCanInterface::getSource() {
    //return NodeID(net->id & MASK_SRC_ALIAS);
    return NodeID();
}


// end of basic message structure



// start of OpenLCB format support



void OlcbCanInterface::setOpenLcbMTI(uint16_t mti) {
    setFrameTypeOpenLcb();
    setVariableField(mti | (FRAME_FORMAT_NORMAL_MTI<<SHIFT_OPENLCB_FORMAT));
}

uint16_t OlcbCanInterface::getOpenLcbMTI() {
    return getVariableField() & 0xFFF;
}

bool OlcbCanInterface::isOpenLcbMTI(uint16_t mti) {
    return isFrameTypeOpenLcb()
    && ( getVariableField() == (mti | (FRAME_FORMAT_NORMAL_MTI<<SHIFT_OPENLCB_FORMAT) ) );
}


bool OlcbCanInterface::isForHere(uint16_t alias) {  // includes frame level, the more common test
    if (!isFrameTypeOpenLcb()) return true; // CAN level is always for here
    // check for global
    uint8_t format = getOpenLcbFormat();
    if (format == FRAME_FORMAT_NORMAL_MTI && (getVariableField() & MTI_ADDRESS_PRESENT_MASK) == 0 ) {
        return true;
    } else {
        // else check addressed here
        return alias == getDestAlias();
    }
}

bool OlcbCanInterface::isMsgForHere(uint16_t alias) {
    if (!isFrameTypeOpenLcb()) return false;
    return isForHere(alias);
}


bool OlcbCanInterface::isForHere(NodeID* thisNode) {  // includes
//    if (!isFrameTypeOpenLcb()) return true; // CAN level is always for here
//    // check for global
//    uint8_t format = getOpenLcbFormat();
//    if (format == FRAME_FORMAT_NORMAL_MTI && (getVariableField() & MTI_ADDRESS_PRESENT_MASK) == 0 ) {
//        return true;
//    } else {
//        // else check addressed here
//        return thisNode->alias == getDestAlias();
//    }
    return 0;
}

//bool OlcbCanInterface::isMsgForHere(uint16_t alias) {
bool OlcbCanInterface::isMsgForHere(NodeID* thisNode) {
    if (!isFrameTypeOpenLcb()) return false;
    //return isForHere(alias);
    return isForHere(thisNode);
}

bool OlcbCanInterface::isAddressedMessage() {  // checks for message carries address, not that it's for here
    if (!isFrameTypeOpenLcb()) return false;
    // check for global
    uint8_t format = getOpenLcbFormat();
    if (format == FRAME_FORMAT_NORMAL_MTI && (getVariableField() & MTI_ADDRESS_PRESENT_MASK) == 0 ) {
        return false;
    } else {
        // all other formats are addressed
        return true;
    }
}

void OlcbCanInterface::getEventID(EventID* evt) {
    memcpy(evt->val, net->data, 8);
}

void OlcbCanInterface::getNodeID(NodeID* nid) {
    memcpy(nid->val, net->data, 6);
}

bool OlcbCanInterface::matchesNid(NodeID* nid) {
    return
    nid->val[0] == net->data[0] &&
    nid->val[1] == net->data[1] &&
    nid->val[2] == net->data[2] &&
    nid->val[3] == net->data[3] &&
    nid->val[4] == net->data[4] &&
    nid->val[5] == net->data[5];
}


// end of OpenLCB format and decode support

// start of OpenLCB messages

void OlcbCanInterface::setInitializationComplete(NodeID* nid) {
    setOpenLcbMTI(MTI_INITIALIZATION_COMPLETE);
    net->length=6;
    memcpy(net->data, nid->val, 6);
}

bool OlcbCanInterface::isInitializationComplete() {
    return isOpenLcbMTI(MTI_INITIALIZATION_COMPLETE);
}

void OlcbCanInterface::setPCEventReport(EventID* eid) {
    setOpenLcbMTI(MTI_PC_EVENT_REPORT);
    net->length=8;
    loadFromEid(eid);
}

bool OlcbCanInterface::isPCEventReport() {
    return isOpenLcbMTI(MTI_PC_EVENT_REPORT);
}

void OlcbCanInterface::setLearnEvent(EventID* eid) {
    setOpenLcbMTI(MTI_LEARN_EVENT);
    net->length=8;
    loadFromEid(eid);
}

bool OlcbCanInterface::isLearnEvent() {
    return isOpenLcbMTI(MTI_LEARN_EVENT);
}


bool OlcbCanInterface::isVerifyNID() {
    if (isOpenLcbMTI(MTI_VERIFY_NID_GLOBAL)) return true;
    else return isOpenLcbMTI(MTI_VERIFY_NID_ADDRESSED);
}

void OlcbCanInterface::setVerifiedNID(NodeID* nid) {
    setOpenLcbMTI(MTI_VERIFIED_NID);
    net->length=6;
    memcpy(net->data, nid->val, 6);
}

bool OlcbCanInterface::isVerifiedNID() {
    return isOpenLcbMTI(MTI_VERIFIED_NID);
}

void OlcbCanInterface::setOptionalIntRejected(OlcbInterface* rcv, uint16_t){}
void OlcbCanInterface::setOptionalIntRejected(OlcbCanInterface* rcv, uint16_t code) {
    setOpenLcbMTI(MTI_OPTION_INT_REJECTED);
    setDestAlias(rcv->getSourceAlias());
    net->length=6;
    
    net->data[2] = (code>>8)&0xFF;
    net->data[3] =  code    &0xFF;
    
    uint16_t mti = rcv->getOpenLcbMTI();
    net->data[4] = ((mti>>8)&0xFF);
    net->data[5] = mti&0xFF;
    
}

bool OlcbCanInterface::isIdentifyConsumers() {
    return isOpenLcbMTI(MTI_IDENTIFY_CONSUMERS);
}

void OlcbCanInterface::setConsumerIdentified(EventID* eid) {
    setOpenLcbMTI(MTI_CONSUMER_IDENTIFIED);
    net->length=8;
    loadFromEid(eid);
}

void OlcbCanInterface::setConsumerIdentifyRange(EventID* eid, EventID* mask) {
    // does send a message, but not complete yet - RGJ 2009-06-14
    setOpenLcbMTI(MTI_IDENTIFY_CONSUMERS_RANGE);
    net->length=8;
    loadFromEid(eid);
}

bool OlcbCanInterface::isIdentifyProducers() {
    return isOpenLcbMTI(MTI_IDENTIFY_PRODUCERS);
}

void OlcbCanInterface::setProducerIdentified(EventID* eid) {
    setOpenLcbMTI(MTI_PRODUCER_IDENTIFIED);
    net->length=8;
    loadFromEid(eid);
}

void OlcbCanInterface::setProducerIdentifyRange(EventID* eid, EventID* mask) {
    // does send a message, but not complete yet - RGJ 2009-06-14
    setOpenLcbMTI(MTI_IDENTIFY_PRODUCERS_RANGE);
    net->length=8;
    loadFromEid(eid);
}

bool OlcbCanInterface::isIdentifyEvents() {
    if  (isOpenLcbMTI(MTI_IDENTIFY_EVENTS_GLOBAL)) return true;
    return isOpenLcbMTI(MTI_IDENTIFY_EVENTS_ADDRESSED);
}

// general, but not efficient
bool OlcbCanInterface::isDatagramFrame() {
    uint16_t fmt = getOpenLcbFormat();
    return isFrameTypeOpenLcb()
    && ( (fmt == FRAME_FORMAT_ADDRESSED_DATAGRAM_ALL)
        || (fmt == FRAME_FORMAT_ADDRESSED_DATAGRAM_FIRST)
        || (fmt == FRAME_FORMAT_ADDRESSED_DATAGRAM_MID)
        || (fmt == FRAME_FORMAT_ADDRESSED_DATAGRAM_LAST) );
}

// just checks 1st, assumes datagram already checked.
bool OlcbCanInterface::isLastDatagramFrame() {
    return (getOpenLcbFormat() == FRAME_FORMAT_ADDRESSED_DATAGRAM_LAST)
    || (getOpenLcbFormat() == FRAME_FORMAT_ADDRESSED_DATAGRAM_ALL);
}

uint8_t OlcbCanInterface::getOpenLcbFormat() {
    return (getVariableField() & MASK_OPENLCB_FORMAT) >> SHIFT_OPENLCB_FORMAT;
}

void OlcbCanInterface::setOpenLcbFormat(uint8_t i) {
    uint16_t now = getVariableField() & ~MASK_OPENLCB_FORMAT;
    setVariableField( ((i << SHIFT_OPENLCB_FORMAT) & MASK_OPENLCB_FORMAT) | now);
}

void OlcbCanInterface::loadFromEid(EventID* eid) {
    memcpy(net->data, eid->val, 8);
}

// ==== Start of CAN-level messages ================================

void OlcbCanInterface::setFrameTypeCAN() {
    net->id &= ~MASK_FRAME_TYPE;
}

bool OlcbCanInterface::isFrameTypeCAN() {
    return (net->id & MASK_FRAME_TYPE) == 0x00000000L;
}

void OlcbCanInterface::setFrameTypeCAN(uint16_t alias, uint16_t var) {
    init(alias);
    setFrameTypeCAN();
    setVariableField(var);
    net->length=0;
}

void OlcbCanInterface::setSourceAlias(uint16_t a) {
                    //Serial.print("\nOlcbCanInterface::setSourceAlias() alias=");
                    //Serial.print(a,HEX);
    net->id &= ~MASK_SRC_ALIAS;
    net->id |= a & MASK_SRC_ALIAS;
                    //Serial.print(" id="); Serial.print(net->id,HEX);
}
uint16_t OlcbCanInterface::getSourceAlias() {
                    //Serial.print("\nOlcbCanInterface::getSourceAlias() id=");
                    //Serial.print(net->id,HEX);
                    //Serial.print(" srcAlias=");
                    //Serial.print(net->id & MASK_SRC_ALIAS,HEX);
        return net->id & MASK_SRC_ALIAS;
}

void OlcbCanInterface::setAMD(uint16_t alias, NodeID* nid) {
    setFrameTypeCAN(alias, AMD_VAR_FIELD);
    net->length=6;
    memcpy(net->data, nid->val, 6);
}

bool OlcbCanInterface::isAMD(uint16_t alias) {
    return isFrameTypeCAN() && (getVariableField() == AMD_VAR_FIELD)
    && (alias == getSourceAlias());
}

void OlcbCanInterface::setAMR(uint16_t alias, NodeID* nid) {
    setFrameTypeCAN(alias, AMR_VAR_FIELD);
    net->length=6;
    memcpy(net->data, nid->val, 6);
}

bool OlcbCanInterface::isAMR(uint16_t alias) {
    return isFrameTypeCAN() && (getVariableField() == AMR_VAR_FIELD)
    && (alias == getSourceAlias());
}

void OlcbCanInterface::setCIM(uint8_t i, uint16_t testval, uint16_t alias) {
    uint16_t var =  (( (0x7-i) & 7) << 12) | (testval & 0xFFF);
    setFrameTypeCAN(alias, var);
}

bool OlcbCanInterface::isCIM() {
    return isFrameTypeCAN() && (getVariableField()&0x7000) >= 0x4000;
}

void OlcbCanInterface::setRIM(uint16_t alias) {
    setFrameTypeCAN(alias, RIM_VAR_FIELD);
}

bool OlcbCanInterface::isRIM() {
    return isFrameTypeCAN() && getVariableField() == RIM_VAR_FIELD;
}


// end of CAN-level messages


// ==== CAN specific =====================================
void OlcbCanInterface::setVariableField(uint16_t f) {
    net->id &= ~MASK_VARIABLE_FIELD;
    uint32_t temp = f;  // ensure 32 bit arithmetic
    net->id |=  ((temp << SHIFT_VARIABLE_FIELD) & MASK_VARIABLE_FIELD);
}

uint16_t OlcbCanInterface::getVariableField() {
    return (net->id & MASK_VARIABLE_FIELD) >> SHIFT_VARIABLE_FIELD;
}
void OlcbCanInterface::setDestAlias(uint16_t a) {
    uint8_t format = getOpenLcbFormat();
    if (format == FRAME_FORMAT_NORMAL_MTI) {
        net->data[0] = (a>>8)&0xFF;
        net->data[1] = a&0xFF;
        if (net->length < 2) net->length = 2;
    } else {
        setVariableField(a | (format << SHIFT_OPENLCB_FORMAT));
    }
}
// CAN specific
uint16_t OlcbCanInterface::getDestAlias() {
    uint8_t format = getOpenLcbFormat();
    if (format == FRAME_FORMAT_NORMAL_MTI)
        return ((net->data[0]&0xF)<<8)|(net->data[1]&0xFF);
    else
        return getVariableField() & 0xFFF;
}

