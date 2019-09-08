
#include "NodeMemory.h"

#include "EventID.h"
#include "Event.h"
#include "NodeID.h"
#include "processor.h"

#include "lib_debug_print_common.h"

NodeMemory::NodeMemory(int eepromBaseAddress, uint16_t userConfigSize)
{
    eepromBase = eepromBaseAddress;
    userConfigBase   = eepromBaseAddress + sizeof(NODE_ID_STORE);
    this->userConfigSize = userConfigSize;

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
	for(uint16_t i = userConfigBase; i < (userConfigBase + userConfigSize); i++)
		EEPROM.update(i, 0);
}


uint8_t NodeMemory::loadAndValidate()
{
	EEPROM.get(eepromBase, nodeIdCache);
	
	nodeIdState = 0;
	
	if(nodeIdCache.Marker == NODE_ID_MARKER)
		nodeIdState |= NODE_ID_MARKER_VALID;
		
	uint8_t checkSum = 0xFF;
	uint8_t *pByte = &nodeIdCache.Marker;
	for(uint8_t i = 0; i < 8; i++)
		checkSum ^= *pByte++;
	
	if(checkSum == 0)
		nodeIdState |= NODE_ID_CHECKSUM_VALID;

	return nodeIdState;
}


uint8_t NodeMemory::getNodeID(NodeID *nodeIdBuffer)
{
	*nodeIdBuffer = nodeIdCache.Id;
		
	return nodeIdState;
}


void NodeMemory::changeNodeID(NodeID *newNodeId)
{
	nodeIdCache.Marker = NODE_ID_MARKER;
	nodeIdCache.Id = *newNodeId;
	
	uint8_t checkSum = 0xFF;
	uint8_t *pByte = &nodeIdCache.Marker;
	for(uint8_t i = 0; i < 7; i++)
		checkSum ^= *pByte++;
	
	nodeIdCache.CheckSum = checkSum;
	
	EEPROM.put(eepromBase, nodeIdCache);
}


void NodeMemory::print()
{
	LDEBUG("\nEEPROM: Base Offset: "); LDEBUG(eepromBase); LDEBUG(" User Base: "); LDEBUG(userConfigBase); LDEBUG(" User Size: "); LDEBUG(userConfigSize);
	LDEBUG(F("\n    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 0123456789ABCDEF"));
	for(unsigned r = 0; r < (userConfigSize / 16 + 1); r++)
	{
		int rb = r * 16;
		LDEBUG("\n");
		if(rb < 16)
			LDEBUG(0);
			
		LDEBUG2(rb,HEX);
		LDEBUG(" ");
		
		for(int i = rb; i < (rb + 16); i++)
		{
			uint8_t v = EEPROM.read(i);
			if(v < 16)
				LDEBUG(0);
			LDEBUG2(v,HEX);
			LDEBUG(" ");
		}
		
		for(int i = rb; i < (rb + 16); i++)
		{
			char c = EEPROM.read(i);
			if( c<' ' || c==0x8F )
			{
				LDEBUG('.')
			}
			else
			{
				LDEBUG(c);
			}
		}
	}
	LDEBUGL();
}

