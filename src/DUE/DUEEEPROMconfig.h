// DUEEEPROMconfig.h

#ifdef __SAM3X8E__

#ifdef USE_EXTERNAL_EEPROM

#ifndef DUEEEPROMCONFIG_H
#define DUEEEPROMCONFIG_H
// This code is being selected from CBUSconfig.h for use when it is not used.
// This is for the EEPROM_EXTERNAL case only.

#include <Arduino.h>                // for definition of byte datatype
#include <Wire.h>

// in-memory hash table
static const byte EE_HASH_BYTES = 4;
static const byte HASH_LENGTH = 128;

static const byte EEPROM_I2C_ADDR = 0x50;

enum {
  EEPROM_INTERNAL = 0,
  EEPROM_EXTERNAL = 1,
  EEPROM_USES_FLASH
};

class EEPROMConfig {
	public:
    EEPROMConfig();
    void begin(void) {}
  	bool setEEPROMtype(byte type);
	void setExtEEPROMAddress(byte address, TwoWire *bus = &Wire);

    byte eeprom_type;
    byte external_address;
    TwoWire *I2Cbus;
};


#endif // DUEEEPROMCONFIG_

#endif // USE_EXTERNAL_EEPROM

#endif // __SAM3X8E__