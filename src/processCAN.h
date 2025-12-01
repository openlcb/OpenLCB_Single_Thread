//
//  processCAN.h
//  
//
//  CANlibrary.h
//  Selects appropriate CAN library for specific processor present
//  Can probably be folded back into processor.h
//
//  Created by Dave Harris on 2024-09-15.
//

#ifndef processCAN_h
#define processCAN_h

    // Uncomment the next line to enable #pragma messages
    #define ENABLE_MESSAGE_PRAGMAS

    #ifdef ENABLE_MESSAGE_PRAGMAS
        #pragma message("!!! compiling processorCAN_h")
    #endif

    #ifndef BTYPE
        #define BTYPE "CAN"
    #endif

    // AVRs
    // Tinys
    #if defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
        #ifndef NOCAN
            #include <can.h>
        #endif // NOCAN


    #elif defined(ARDUINO_ARCH_AVR)
       #ifdef ENABLE_MESSAGE_PRAGMAS
          //#pragma message("CAN ARDUINO_ARCH-AVR selected")
       #endif
       // AT90CAN -- TCH
       #if defined(ARDUINO_AVR_TCH_CONSUMER) \
        || defined(ARDUINO_AVR_TCH_PRODUCER) \
        || defined(ARDUINO_AVR_TCH_PRODUCER_CONSUMER)
          #define AT90CAN
          #ifndef NOCAN
            #ifdef ENABLE_MESSAGE_PRAGMAS
                #pragma message("AT90CAN selected for TCH board")
            #endif
            #include <AT90/AT90can.h>
          #endif // NOCAN

       #else
          #define ATMEGA
          #ifndef NOCAN
            #ifdef ENABLE_MESSAGE_PRAGMAS
              #pragma message("CAN ARDUINO_ARCH-AVR MCP2515 selected")
            #endif
            #include "MCP2515/MCPcan.h"
            #define OlcbCanClass McpCan
          #endif // NOCAN
       #endif

    // **8 ... 168 and 328 Arduinos
    #elif defined(__AVR_ATmega8__)  || \
          defined(__AVR_ATmega48__) || \
          defined(__AVR_ATmega88__) || \
          defined(__AVR_ATmega168__) || \
          defined(__AVR_ATmega168P__) || \
          defined(__AVR_ATmega328P__) || \
          defined(__AVR_ATmega328P__) || \
          defined(ARDUINO_AVR_NANO)
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("ATMega *68 selected")
        #endif
        #define ATMEGA
        #ifndef NOCAN
            #include "MCP2515/MCPcan.h"
            #define OlcbCanClass McpCan
        #endif // NOCAN

    // Mega 16, 32
    #elif defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__)
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("ATMega 16/32 selected")
        #endif
        #ifndef NOCAN
            #include "MCP2515/MCPcan.h"
            #define OlcbCanClass McpCan
        #endif // NOCAN

    // Mega 128, 1280 & 2560
    #elif defined(__AVR_ATmega128__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("ATMega 128/1280/2580 selected")
        #endif
        #ifndef NOCAN
            #include "MCP2515/MCPcan.h"
            #define OlcbCanClass McpCan
        #endif // NOCAN

    // Sanguino
    #elif defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("ATMega 644/644P/1284P selected")
        #endif
        #ifndef NOCAN
            #include "MCP2515/MCPcan.h"
            #define OlcbCanClass McpCan
        #endif // NOCAN

    // AT90CAN
    #elif defined(__AVR_AT90CAN32__) || defined(__AVR_AT90CAN64__) || defined(__AVR_AT90CAN128__)
        #define AT90CAN
        #ifndef NOCAN
            #ifdef ENABLE_MESSAGE_PRAGMAS
                #pragma message("AT90CAN selected")
            #endif
            #include <AT90/AT90can.h>
        #endif // NOCAN

    // Teensies  /////// THESE APPEAR WRONG
    #elif defined(__AVR_ATmega32U4__)
        // Teensy 2.0
        #ifdef CORE_TEENSY
        #ifndef NOCAN
            #include <can.h>
        #endif // NOCAN
        // Teensy
        #else
            #define CHIPSET ATmega_32U4_B
        #endif

    // Teensy++ 1.0 & 2.0
    #elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
        #ifndef NOCAN
            #include <can.h>
        #endif // NOCAN

    // Teensy ARM
    #elif defined(__MK20DX128__)
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("!!! __MK20DX128__ selected ")
        #endif
        #ifndef NOCAN
            #include "TeesyARM/flexcan.h"
        #endif // NOCAN
        #define reboot _reboot_Teensyduino_()

    #elif  defined(__MK20DX256__)
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("!!! __MK20DX256__ selected ")
        #endif
        #define Teensy
        #ifndef NOCAN
            #include "TeesyARM/flexcan.h"
        #endif // NOCAN

    #elif defined(__MK64FX512__)
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("!!! __MK64FX512__ selected ")
        #endif
        #ifndef NOCAN
            #include "TeesyARM/flexcan.h"
        #endif // NOCAN

    #elif defined(__MK66FX1M0__)
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("!!! __MK66FX1M0__ selected ")
        #endif
        #ifndef NOCAN
            #include "TeesyARM/flexcan.h"
        #endif // NOCAN

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
        #ifndef NOCAN
            #include "Tiva/tivacan.h"
        #endif // NOCAN

    #elif defined ENERGIA_EK-TM4C1294XL
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("!!! ENERGIA_EK-TM4C1294XL selected ")
        #endif
        #ifndef NOCAN
            #include <can.h>
            #include "Tiva/tivacan.h"
        #endif // NOCAN

    // ESP32
    #elif defined ARDUINO_ARCH_ESP32
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("ARDUINO_ARCH_ESP32 selected ")
        #endif
        #ifndef NOCAN
            #include "ESP32/ESPcan.h"
        #endif // NOCAN

    // PICO
    #elif defined(ARDUINO_ARCH_RP2040)
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("!!! TARGET_RP2040 selected ok")
        #endif
        #define PICO
        #ifndef NOCAN
            #include "Pico/picocan.h"
        #endif // NOCAN

    // SAM
    #elif defined ARDUINO_SAM_DUE
        #ifdef ENABLE_MESSAGE_PRAGMAS
            #pragma message("ARDUINO_DUE selected ")
        #endif
        #define DUE
        #ifndef NOCAN
            #include "DUE/DUEcan.h"
            #define OlcbCanClass CanX
        #endif // NOCAN

    #else
        #pragma messge "No processor detected"
        #define reboot

    #endif // CANlibrary_h

#endif /* processCAN_h */
