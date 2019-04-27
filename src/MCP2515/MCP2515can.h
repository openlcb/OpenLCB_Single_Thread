//#pragma message("!!! In MCP2515can.h ")

#ifndef CAN_H
#define CAN_H

//#pragma message("!!! compiling MCP2515can.h ")

#if defined (__cplusplus)
    extern "C" {
#endif
    
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <inttypes.h>
#include <stdbool.h>
    
//#include "can.h"
#include "utils.h"
#include "spi.h"


#if defined(SUPPORT_MCP2515) && (SUPPORT_MCP2515 == 1)
    #if defined(__AVR_ATmega16__) || defined(__AVR_ATmega32__) || defined(__AVR_ATmega644__)
        //#pragma message("!!! 16,32 ")
        #define	P_MOSI	B,5
        #define	P_MISO	B,6
        #define	P_SCK	B,7
        #define	SUPPORT_FOR_MCP2515__
    #elif defined(__AVR_ATmega8__)  || defined(__AVR_ATmega48__) || \
        defined(__AVR_ATmega88__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
        //#pragma message("!!! 8,48,88,168,328P ")
        #define	P_MOSI	B,3
        #define	P_MISO	B,4
        #define	P_SCK	B,5
        #define	SUPPORT_FOR_MCP2515__
    #elif defined(__AVR_ATmega128__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        //#pragma message("!!! 128,1280,2560 ")
        #define	P_MOSI	B,2
        #define	P_MISO	B,3
        #define	P_SCK	B,1
        #define	SUPPORT_FOR_MCP2515__
    #elif defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
        //#pragma message("!!! tiny45 ")
        #define	P_MOSI	B,0
        #define	P_MISO	B,1
        #define	P_SCK	B,2
        #define	USE_SOFTWARE_SPI		1
        #define	SUPPORT_FOR_MCP2515__
    #else
        #error	choosen AVR-type is not supported yet!
    #endif
#endif
    
    


        
        
        
#ifdef  SUPPORT_FOR_MCP2515__
    
    #define MCP2515_DISABLE_MASKS_AND_FILTERS 1
    // ----------------------------------------------------------------------------
    // load some default values
    
    #ifndef	MCP2515_CLKOUT_PRESCALER
        #define	MCP2515_CLKOUT_PRESCALER	0
    #endif
    #ifndef	MCP2515_INTERRUPTS
        #define	MCP2515_INTERRUPTS			(1<<RX1IE)|(1<<RX0IE)
    #endif
#endif
    
#include "mcp2515_defs.h"
    
#ifndef	MCP2515_CS
    #error	MCP2515_CS ist nicht definiert!
#endif
    
#if defined(MCP2515_RX0BF) && !defined(MCP2515_RX1BF)
    #warning	only MCP2515_RX0BF but not MCP2515_RX1BF defined!
#elif !defined(MCP2515_RX0BF) && defined(MCP2515_RX1BF)
    #warning	only MCP2515_RX1BF but not MCP2515_RX0BF defined!
#elif defined(MCP2515_RX0BF) && defined(MCP2515_RX1BF)
    #define	RXnBF_FUNKTION
#endif

//#define	SUPPORT_TIMESTAMPS
    
    typedef struct
    {
        uint32_t id;				//!< ID der Nachricht (11 oder 29 Bit)
        struct {
            int rtr : 1;			//!< Remote-Transmit-Request-Frame?
            int extended : 1;		//!< extended ID?
        } flags;
        uint8_t length;				//!< Anzahl der Datenbytes
        uint8_t data[8];			//!< Die Daten der CAN Nachricht
         #if SUPPORT_TIMESTAMPS
          uint16_t timestamp;
         #endif
    } tCAN;
//#pragma message("!!! AT90can.h tCAN")

    typedef struct
    {
        uint32_t id;				//!< ID der Nachricht (11 oder 29 Bit)
        uint32_t mask;				//!< Maske
        struct {
            uint8_t rtr : 2;		//!< Remote Request Frame
            uint8_t extended : 2;	//!< extended ID
        } flags;
    } tCANFilter;
    
    typedef struct {
        uint8_t rx;				//!< Empfangs-Register
        uint8_t tx;				//!< Sende-Register
    } tCANErrorRegister;
    
    typedef enum {
        eLISTEN_ONLY_MODE,		//!< der CAN Contoller empfängt nur und verhält sich völlig passiv
        eLOOPBACK_MODE,			//!< alle Nachrichten direkt auf die Empfangsregister umleiten ohne sie zu senden
        eNORMAL_MODE				//!< normaler Modus, CAN Controller ist aktiv
    } tCANMode;
    

    extern bool can_init();
    extern bool can_set_filter(uint8_t number, const tCANFilter *filter);
    extern bool can_disable_filter(uint8_t number);
    extern void can_static_filter(const uint8_t *filter_array);
    extern uint8_t can_get_filter(uint8_t number, tCANFilter *filter);

    extern bool can_check_message(void);
    extern bool can_check_free_buffer(void);
    extern uint8_t can_send_message(const tCAN *msg);
    extern uint8_t can_get_message(tCAN *msg);

    extern tCANErrorRegister can_read_error_register(void);
    extern bool can_check_bus_off(void);
    extern void can_reset_bus_off(void);
    extern void can_set_mode(tCANMode mode);
    extern void can_regdump(void);
    
#if defined (__cplusplus)
    }
#endif

#endif // CAN_H
