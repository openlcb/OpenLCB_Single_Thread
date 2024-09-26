#include <Arduino.h>
#include "NodeMemory.h"

#include "EventID.h"
#include "Event.h"
#include "NodeID.h"
#include "processor.h"

#include "debugging.h"

extern void setEepromDirty();
extern bool eepromDirty;

NodeMemory::NodeMemory(int eepromBaseAddress, uint16_t userConfigSize)
{
    eepromBase = eepromBaseAddress;
    userConfigBase   = eepromBaseAddress + sizeof(NODE_ID_STORE);
    this->userConfigSize = userConfigSize;
}

void NodeMemory::init()
{
    loadAndValidate();
}

uint8_t NodeMemory::read( int idx )
{
	return EEPROM.read(idx + userConfigBase);
}

void NodeMemory::write( int idx, uint8_t val )
{
	EEPROM.write(idx + userConfigBase, val);
}

uint16_t NodeMemory::length(void)
{
	return userConfigSize;
}

void NodeMemory::eraseAll(void)
{
    //dP(F("\n >>>>NodeMemory::eraseAll"));
	for(uint16_t i = userConfigBase; i < (userConfigBase + userConfigSize); i++)
		EEPROMupdate(i, 0x00);
    EEPROMcommit;
    eepromDirty = false;
    //dP(F("\nNM eraseAll()"));
}


uint8_t NodeMemory::loadAndValidate()
{
    //dP(F("\n >>>>nm::loadAndValidate()"));
	EEPROM.get(eepromBase, nodeIdCache);
    //dP(F("\n     nodeIdCache.Marker= ")); dPH(nodeIdCache.Marker);

	nodeIdState = 0;
    
    //dP(F("\n   nodeIdCache.Marker == NODE_ID_MARKER=")); dP(nodeIdCache.Marker == NODE_ID_MARKER);
	if(nodeIdCache.Marker == NODE_ID_MARKER)
		nodeIdState |= NODE_ID_MARKER_VALID;
		
	uint8_t checkSum = 0xFF;
	uint8_t *pByte = &nodeIdCache.Marker;
	for(uint8_t i = 0; i < 8; i++)
		checkSum ^= *pByte++;
	
	if(checkSum == 0)
		nodeIdState |= NODE_ID_CHECKSUM_VALID;
    //dP(F("\n nodeIdState (0x30 means valid= ")); dPH(nodeIdState);
	return nodeIdState;
}


uint8_t NodeMemory::getNodeID(NodeID *nodeIdBuffer)
{
	*nodeIdBuffer = nodeIdCache.Id;
		
	return nodeIdState;
}


void NodeMemory::changeNodeID(NodeID newNodeId)
{
    //dP(F("\n >>>>NodeMemory::changeNodeID()")); newNodeId.print();
	nodeIdCache.Marker = NODE_ID_MARKER;
	nodeIdCache.Id = newNodeId;
    //dP(F("nodeIdCache.Id=")); dP(NODE_ID_MARKER);
	uint8_t checkSum = 0xFF;
	uint8_t *pByte = &nodeIdCache.Marker;
	for(uint8_t i = 0; i < 7; i++)
		checkSum ^= *pByte++;
	
	nodeIdCache.CheckSum = checkSum;
    //dP(F("nodeIdCache.CheckSum = ")); dP(checkSum);
    //dP(F("\neeprombase= ")); dP(eepromBase);
	EEPROM.put(eepromBase, nodeIdCache);
    setEepromDirty();
    //dP(F("\nNM changeNodeID()"));
}


void NodeMemory::print(int n)
{
    dP(F("\nEEPROM: Base Offset: ")); dP(eepromBase); dP(" User Base: "); dP(userConfigBase); dP(" User Size: "); dP(userConfigSize);
    dP(F("\n    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 0123456789ABCDEF"));
	//for(unsigned r = 0; r < (userConfigSize / 16 + 1); r++)
    for(unsigned r = 0; r < (n / 16 + 1); r++)
	{
		int rb = r * 16;
        dP(F("\n"));
        if(rb < 16) dP((uint8_t)0); dPH(rb); dP(" ");

		for(int i = rb; i < (rb + 16); i++)
		{
			uint8_t v = EEPROM.read(i);
            if(v < 16) dP((uint8_t)0); dPH(v); dP(" ");
		}
		
		for(int i = rb; i < (rb + 16); i++)
		{
			char c = EEPROM.read(i);
            if( c<' ' || c==0x8F ) dP('.');
            else dP(c);
		}
	}
    dP(F("\n"));
}

