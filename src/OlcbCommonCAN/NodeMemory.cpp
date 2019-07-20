
#include "NodeMemory.h"

#include "EventID.h"
#include "Event.h"
#include "NodeID.h"
#include "processor.h"

#include "lib_debug_print_common.h"

extern "C" {
  extern uint16_t getOffset(uint16_t index);
  extern void writeEID(int index);
}

extern void userInitAll();

NodeMemory::NodeMemory(int eepromBaseAddress, uint16_t userConfigSize)
{
    baseAddress = eepromBaseAddress;
    bytesUsed = userConfigSize;
    
    loadAndValidate();
}


uint8_t NodeMemory::loadAndValidate()
{
	EEPROM.get(baseAddress, header);
	
	nodeHeaderState = 0;
	
	if(header.nodeId.nodeIdMarker == NODE_ID_MARKER)
		nodeHeaderState |= NODE_ID_MARKER_VALID;
		
	uint8_t checkSum = 0xFF;
	uint8_t *pByte = &header.nodeId.nodeIdMarker;
	for(uint8_t i = 0; i < 8; i++)
		checkSum ^= *pByte++;
	
	if(checkSum == 0)
		nodeHeaderState |= NODE_ID_CHECKSUM_VALID;

	uint16_t	nodeResetControl;

	EEPROM.get(baseAddress + sizeof(NODE_HEADER), nodeResetControl);

	if(nodeResetControl == RESET_NORMAL_VAL)
		nodeHeaderState |= RESET_NORMAL;
		
	else if(nodeResetControl == RESET_NEW_EVENTS_VAL) 
		nodeHeaderState |= RESET_NEW_EVENTS;
		
	else 
		nodeHeaderState |= RESET_FACTORY_DEFAULTS;
		
	return nodeHeaderState;
}


uint8_t NodeMemory::getNodeID(NodeID *nodeIdBuffer)
{
	*nodeIdBuffer = header.nodeId.nodeId;
		
	return nodeHeaderState;
}


void NodeMemory::changeNodeID(NodeID *newNodeId)
{
	header.nodeId.nodeIdMarker = NODE_ID_MARKER;
	header.nodeId.nodeId = *newNodeId;
	
	uint8_t checkSum = 0xFF;
	uint8_t *pByte = &header.nodeId.nodeIdMarker;
	for(uint8_t i = 0; i < 7; i++)
		checkSum ^= *pByte++;
	
	header.nodeId.nodeIdCheckSum = checkSum;
	
	EEPROM.put(baseAddress, header.nodeId);
  EEPROMcommit;
}


void NodeMemory::forceFactoryReset()
{
    //LDEBUG("\nforceInitAll");
  uint16_t	nodeResetControl = RESET_FACTORY_DEFAULTS_VAL;
  EEPROM.put(baseAddress + sizeof(NODE_HEADER), nodeResetControl);
  EEPROMcommit;
  LDEBUG("\n NodeMemory::forceFactoryReset()");
}

void NodeMemory::forceNewEventIDs() {
    //LDEBUG("\nforceInitEvents");
  uint16_t	nodeResetControl = RESET_NEW_EVENTS_VAL;
  EEPROM.put(baseAddress + sizeof(NODE_HEADER), nodeResetControl);
  EEPROMcommit;
  LDEBUG("\n NodeMemory::forceNewEventIDs()");
}


void NodeMemory::init(Event* events, uint8_t numEvents)
{
	if(nodeHeaderState & NODE_ID_OK)
	{
		if(nodeHeaderState & RESET_NORMAL)
			return; // Nothing to do

		else if(nodeHeaderState & RESET_NEW_EVENTS)
		{
			writeNewEventIDs(events, numEvents);

			uint16_t	nodeResetControl = RESET_NORMAL_VAL;
			EEPROM.put(baseAddress + sizeof(NODE_HEADER), nodeResetControl);
			EEPROMcommit;
		}

		else if (nodeHeaderState & RESET_FACTORY_DEFAULTS)
		{	//clear EEPROM
			for(uint16_t i = baseAddress + sizeof(NODE_HEADER); i < (baseAddress + sizeof(NODE_HEADER) + bytesUsed); i++)
					EEPROM.update(i, 0);
			
			header.nextEID = 0;
			
			// handle the rest
			writeNewEventIDs(events, numEvents);
			userInitAll();
			
			uint16_t	nodeResetControl = RESET_NORMAL_VAL;
			EEPROM.put(baseAddress + sizeof(NODE_HEADER), nodeResetControl);
			EEPROMcommit;
		}
	}
}


// write to EEPROM new set of eventIDs and then magic, nextEID and nodeID
void NodeMemory::writeNewEventIDs(Event* events, uint8_t numEvents)
{
	EventID newEventId;
	
	newEventId.setNodeIdPrefix(&header.nodeId.nodeId);

	for(uint16_t e = 0; e < numEvents; e++)
	{
		uint16_t eepromAddress = baseAddress + sizeof(NODE_HEADER) + getOffset(e); 
		
		newEventId.setEventIdSuffix(header.nextEID++);
		
		EEPROM.put(eepromAddress, newEventId);
	}
	// Save the latest value of nextEID
	EEPROM.put(baseAddress + sizeof(NODE_ID_STORE), header.nextEID);
}


void NodeMemory::print()
{
	LDEBUG("\nEEPROM:");
	LDEBUG(F("\n    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 0123456789ABCDEF"));
	for(unsigned r = 0; r < (bytesUsed / 16 + 1); r++)
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
				LDEBUG('.')
			else
				LDEBUG(c);
		}
	}
}

