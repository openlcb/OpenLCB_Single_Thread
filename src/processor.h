/*
 *  CANlibrary.h
 *  Selects appropriate CAN library for specific processor present
 */

#ifndef processor_h
#define processor_h

// Uncomment the next line to enable #pragma messages
// #define ENABLE_MESSAGE_PRAGMAS

//#pragma message("!!! compiling processor_h")

#define EEPROMbegin
#define EEPROMcommit
#define ESTRING(s) s          // default conversion - nil
//#define ESTRING(s) F(s)     // alternate def
//#define ESTRING(s) PSTR(s)  // alternate def


// AVRs
#ifdef ARDUINO_ARCH_AVR
#ifdef ENABLE_MESSAGE_PRAGMAS 
    //#pragma message("AVR Selected")
#endif
    #include <EEPROM.h>
    #include <avr/wdt.h>
    #define REBOOT                   \
            wdt_enable(WDTO_15MS);   \
            for(;;){}
#endif
// Tinys
#if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    #include <can.h>

// **8 ... 168 and 328 Arduinos
#elif defined(__AVR_ATmega8__)  || defined(__AVR_ATmega48__) || defined(__AVR_ATmega88__) || \
   defined(__AVR_ATmega168__) ||defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega328P__)
#ifdef ENABLE_MESSAGE_PRAGMAS 
    #pragma message("ATMega selected")
#endif
    #define ATMEGA
    #include "MCP2515/MCPcan.h"

// Mega 16, 32
#elif defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__) 
    #include <can.h>

// Mega 128, 1280 & 2560
#elif defined(__AVR_ATmega128__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    #include <can.h>

// Sanguino
#elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
    #include <can.h>

// AT90CAN
#elif defined(__AVR_AT90CAN32__) || defined(__AVR_AT90CAN64__) || defined(__AVR_AT90CAN128__)
#ifdef ENABLE_MESSAGE_PRAGMAS 
    #pragma message("AT90CAN selected")
#endif
    #define AT90CAN
    #include <AT90/AT90can.h>

// Teensies
#elif defined(__AVR_ATmega32U4__)
  // Teensy 2.0
  #ifdef CORE_TEENSY 
    #include <can.h>
  // Teensy
  #else
    #define CHIPSET ATmega_32U4_B
  #endif
// Teensy++ 1.0 & 2.0
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
    #include <can.h>

// Teensy ARM
#elif defined(__MK20DX128__)
#ifdef ENABLE_MESSAGE_PRAGMAS 
    #pragma message("!!! __MK20DX128__ selected ")
#endif
    #include <EEPROM.h>
    #include "TeesyARM/flexcan.h"
    #define reboot _reboot_Teensyduino_()
#elif  defined(__MK20DX256__)
#ifdef ENABLE_MESSAGE_PRAGMAS 
    #pragma message("!!! __MK20DX256__ selected ")
#endif
    #define Teensy
    #include <EEPROM.h>
    #include "TeesyARM/flexcan.h"
    #define RAMEND 0x3FFFF
    #define REBOOT _reboot_Teensyduino_()
#elif defined(__MK64FX512__)
#ifdef ENABLE_MESSAGE_PRAGMAS 
    #pragma message("!!! __MK64FX512__ selected ")
#endif
    #include <EEPROM.h>
    #include "TeesyARM/flexcan.h"
    #define reboot _reboot_Teensyduino_()
#elif defined(__MK66FX1M0__)
#ifdef ENABLE_MESSAGE_PRAGMAS 
    #pragma message("!!! __MK66FX1M0__ selected ")
#endif
    #include <EEPROM.h>
    #include "TeesyARM/flexcan.h"
    #define reboot _reboot_Teensyduino_()

// Teensy 3.5 & 3.6

// Tiva Lauchpads
//#elif defined ENERGIA_EK-TM4C123GXL  // LM4f120 comes here, too
//#elif defined(TARGET_IS_BLIZZARD_RB1)  // LM4f120 comes here, too
#elif defined(ENERGIA_ARCH_TIVAC)
    // LM4f120 comes here, too
#ifdef ENABLE_MESSAGE_PRAGMAS 
    #pragma message("!!! ENERGIA_EK-TM4C123GXL selected ok")
#endif
    #define TIVA123
    #define RAMEND 0x7FFF
    #define E2END 0x7FF
    #define REBOOT HWREG(NVIC_APINT) = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ
    #define ESTRING(s) PSTR(s)
    #include "Tiva/tivaeeprom.h"
    #include "Tiva/tivacan.h"
#elif defined ENERGIA_EK-TM4C1294XL
#ifdef ENABLE_MESSAGE_PRAGMAS 
    #pragma message("!!! ENERGIA_EK-TM4C1294XL selected ")
#endif
    #define RAMEND 0x3FFFF
    #define E2END 0x17FF
    #define REBOOT HWREG(NVIC_APINT) = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ
    //#define EEPROMEND 0x17FF
    #include <can.h>
    #include "Tiva/tivaeeprom.h"
    #include "Tiva/tivacan.h"
    #define ESTRING(s) PSTR(s)

// ESP32
#elif defined ARDUINO_ARCH_ESP32
#ifdef ENABLE_MESSAGE_PRAGMAS 
    #pragma message("ARDUINO_ARCH_ESP32 selected ")
#endif
    #define ESP32
    #include "ESP32/ESPcan.h"
    #define RAMEND 0x7FFFF
    #define REBOOT esp_restart()
    //#include "ESPeeprom.h"
    #include "EEPROM.h"
    #define EEPROMbegin EEPROM.begin(1000)
    #define EEPROMcommit EEPROM.commit()

#else
    #define reboot

#endif

#endif // CANlibrary_h
