// DUEEEPROMconfig.cpp

#ifdef __SAM3X8E__

#ifdef USE_EXTERNAL_EEPROM

#include "DUEEEPROMconfig.h"
/*
// This pulls in a lot of CBUS stuff not needed except in the bridge case.
// It would not be needed if the CBUS code had already setup a CBUSConfig object.
// The alternative would be to extract the EEPROM code from CBUSconfig as a separate set of codes.
// These would be used when CBUS has not been configured.
// Work needed.
//#include <CBUSconfig.h>
CBUSConfig config;                    // configuration object
void setupEEPROM() {
  // This has to come before setEEPROMtype.
  // Default EEPROM_I2C_ADDR = 0x50 defined in CBUSconfig.h
  config.setExtEEPROMAddress(EEPROM_I2C_ADDR,&Wire1);
  config.setEEPROMtype(EEPROM_EXTERNAL);
}
*/

EEPROMConfig::EEPROMConfig() {
  eeprom_type = EEPROM_INTERNAL;
  I2Cbus = &Wire;
}

//
/// set the EEPROM type for event storage - on-chip or external I2C bus device
/// external EEPROM must use 16-bit addresses !!
//

bool EEPROMConfig::setEEPROMtype(byte type) {

  bool ret = true;
  byte result;
  eeprom_type = EEPROM_INTERNAL;

  switch (type) {
  case EEPROM_EXTERNAL:
    // test accessibility of external EEPROM chip
    I2Cbus->begin();
    I2Cbus->beginTransmission(external_address);
    result = I2Cbus->endTransmission();

    if (result == 0) {
      eeprom_type = type;
      // DEBUG_SERIAL << F("> external EEPROM selected") << endl;
    } else {
      // DEBUG_SERIAL << F("> external EEPROM not found") << endl;
      eeprom_type = EEPROM_INTERNAL;
      ret = false;
    }
    break;

  case EEPROM_USES_FLASH:

// #ifdef __AVR_XMEGA__
#if defined(DXCORE)
    eeprom_type = EEPROM_USES_FLASH;
#else
    eeprom_type = EEPROM_INTERNAL;
    // DEBUG_SERIAL << F("> internal EEPROM selected") << endl;
#endif
    break;
  }

  return ret;
}

//
/// set the bus address of an external EEPROM chip
//

void EEPROMConfig::setExtEEPROMAddress(byte address, TwoWire *bus) {
  external_address = address;
  I2Cbus = bus;
}


#endif // USE_EXTERNAL_EEPROM

#endif // __SAM3X8E__
