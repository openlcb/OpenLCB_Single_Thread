// DUEcanImpl.cpp
// Implementation details for DUEcan

// An implementation to use external EEPROM is needed.
// This will need to allow both for starting the I2C when running alone
// and cooperating with CBUS when the EEPROM is shared.
// As the CBUS EEPROM configuation is in a library it will be easier to implement this here.
// One solution would be to use CBUSconfig library in all cases.


#ifdef __SAM3X8E__

//#define USE_EXTERNAL_EEPROM

#include "DUEcanImpl.h"
#ifdef USE_EXTERNAL_EEPROM
#include "DueEEPROMconfig.h"
DueEEPROMConfig DueEEPROMconfig;
#else
// Details for DueEEPROM need to be added here.
// This version is using flash memory.
DueFlashStorage dueFlashstorage;
#endif
DueEEPROMflash EEPROM;

// These functions are implemented here as they depend on the instance of DueFlashStorage.

uint8_t DueEERef::operator*() const        //   
//{ return dueFlashstorage.read( (uint8_t*) index ); }
{ return dueFlashstorage.read( index ); }

DueEERef &DueEERef::operator=( uint8_t in )   //    
//{ return *this = dueFlashstorage.write( (uint8_t*) index, in ); }
{ return *this = dueFlashstorage.write( index, in ); }


#endif





