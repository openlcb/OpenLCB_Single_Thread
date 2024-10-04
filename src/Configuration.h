#ifndef Configuration_h
#define Configuration_h

/**
 * Class for handling Configuration protocol
 *
 * 
 */
#include <stdint.h>

#include "Datagram.h"

#define CONFIGURATION_DATAGRAM_CODE 0x20

// define the operation codes, full byte
#define CFG_CMD_WRITE                       0x00
#define CFG_CMD_READ                        0x40
#define CFG_CMD_OPERATION                   0x80

#define CFG_CMD_READ_REPLY                  0x50

#define CFG_CMD_GET_CONFIG                  0x80
#define CFG_CMD_GET_CONFIG_REPLY            0x82
#define CFG_CMD_GET_ADD_SPACE_INFO          0x84
#define CFG_CMD_GET_ADD_SPACE_NOT_PRESENT   0x86
#define CFG_CMD_GET_ADD_SPACE_PRESENT       0x87

#define CFG_CMD_LOCK                        0x88
#define CFG_CMD_LOCK_REPLY                  0x8A
#define CFG_CMD_GET_UNIQUEID                0x8C
#define CFG_CMD_GET_UNIQUEID_REPLY          0x8D

#define CFG_CMD_UNFREEZE                    0xA0
#define CFG_CMD_FREEZE                      0xA1
//#define CFG_CMD_INDICATE                    0xA4
#define CFG_CMD_UPDATE_COMPLETE             0xA8
#define CFG_CMD_RESETS                      0xA9
#define CFG_CMD_REINIT_FACTORYRESET         0xAA

#define CFG_SPACE_EEPROM    0xFD

class OlcbStream;

class Configuration {
  public:
  
  //*****
  // The "getRead", "getWrite" and "reset" methods
  // are a very simple interface to the actual
  // device.  Redo them for the specific memory
  // map being used.
  // (We're trying for now to avoid virtual methods)
  // ****
  
  Configuration(Datagram* datagramHandler, OlcbStream *streamHandler,
                        uint8_t (*getRead)(uint32_t address, int space),
                        void (*getWrite)(uint32_t address, int space, uint8_t val),
                        void (*restart)(),
                        void (*wCB)(uint32_t address, uint16_t length, uint16_t func)
            );
            
  void check(); 
  int receivedDatagram(uint8_t* data, uint16_t length, uint16_t from);
  
  private:
  void processRead(uint8_t* data, int length);
  void processWrite(uint8_t* data, int length);
  void processCmd(uint8_t* data, int length);
  
  Datagram* dg;
  OlcbStream* str;
  uint8_t buffer[DATAGRAM_LENGTH];
  int length;
    uint16_t from;
  bool request;
  
  uint32_t getAddress(uint8_t* data);
  int decodeLen(uint8_t* data);
  int decodeSpace(uint8_t* data);
  
  uint8_t (*getRead)(uint32_t address, int space);
  void (*getWrite)(uint32_t address, int space, uint8_t val);
  void (*restart)();
  void (*writeCB)(uint32_t address, uint16_t length, uint16_t func);
 
};

// optional members, 0 if not appearing; does this work with clang compiler?
extern "C" {
uint32_t spaceUpperAddr(uint8_t space) __attribute__((weak));
};

#endif
