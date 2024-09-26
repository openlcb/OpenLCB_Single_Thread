/* *****************************************************************************
 * The MIT License
 *
 * Copyright (c) 2015 Lembed.org.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * ****************************************************************************/

#ifndef CAN_BUS_H
#define CAN_BUS_H

#if defined(__AVR_AT90CAN128__)

#include "debugging.h"

// #include "Arduino.h"

#include <avr/pgmspace.h>
// #include <stdint.h>
// #include <stdbool.h>
// #include <string.h>
// 
// #include <avr/io.h>
// #include <avr/interrupt.h>
// #include <util/delay.h>

//#pragma message("AT90 CanBus.h");

#define	SUPPORT_EXTENDED_CANID	1
#define	SUPPORT_TIMESTAMPS		0
#define CAN_FORCE_TX_ORDER		1

#if F_CPU != 16000000UL
    #error	!!!! only 16 MHz crystal supported !!!!
#endif


#ifndef	SUPPORT_EXTENDED_CANID
#define	SUPPORT_EXTENDED_CANID
#endif

#ifndef	SUPPORT_TIMESTAMPS
#define	SUPPORT_TIMESTAMPS		0
#endif

#ifndef	CAN_INDICATE_TX_TRAFFIC_FUNCTION
#define	CAN_INDICATE_TX_TRAFFIC_FUNCTION
#endif

#ifndef	CAN_INDICATE_RX_TRAFFIC_FUNCTION
#define	CAN_INDICATE_RX_TRAFFIC_FUNCTION
#endif

#ifndef	CAN_FORCE_TX_ORDER
#define	CAN_FORCE_TX_ORDER		0
#endif

#define	CAN_ALL_FILTER				0xff
#define	ENTER_CRITICAL_SECTION		do { unsigned char sreg_ = SREG; cli();
#define	LEAVE_CRITICAL_SECTION		SREG = sreg_; } while (0);

typedef struct {
#if SUPPORT_EXTENDED_CANID
	uint32_t id;				//!< ID der Nachricht (11 oder 29 Bit)
	struct {
		int rtr : 1;			//!< Remote-Transmit-Request-Frame?
		int extended : 1;		//!< extended ID?
	} flags;
#else
	uint16_t id;				//!< ID der Nachricht (11 Bit)
	struct {
		int rtr : 1;			//!< Remote-Transmit-Request-Frame?
	} flags;
#endif

	uint8_t length;				//!< Anzahl der Datenbytes
	uint8_t data[8];			//!< Die Daten der CAN Nachricht

#if SUPPORT_TIMESTAMPS
	uint16_t timestamp;
#endif
} can_t;


typedef struct {
#if	SUPPORT_EXTENDED_CANID
	uint32_t id;				//!< ID der Nachricht (11 oder 29 Bit)
	uint32_t mask;				//!< Maske
	struct {
		uint8_t rtr : 2;		//!< Remote Request Frame
		uint8_t extended : 2;	//!< extended ID
	} flags;
#else
	uint16_t id;				//!< ID der Nachricht 11 Bits
	uint16_t mask;				//!< Maske
	struct {
		uint8_t rtr : 2;		//!< Remote Request Frame
	} flags;
#endif
} can_filter_t;


typedef struct {
	uint8_t rx;				//!< Empfangs-Register
	uint8_t tx;				//!< Sende-Register
} can_error_register_t;

typedef enum {
	LISTEN_ONLY_MODE,		//!< der CAN Contoller empfängt nur und verhält sich völlig passiv
	LOOPBACK_MODE,			//!< alle Nachrichten direkt auf die Empfangsregister umleiten ohne sie zu senden
	NORMAL_MODE,				//!< normaler Modus, CAN Controller ist aktiv
	SLEEP_MODE					// sleep mode
} can_mode_t;

typedef struct {
	can_t *buf;
	uint8_t size;

	uint8_t used;
	uint8_t head;
	uint8_t tail;
} can_buffer_t;

typedef enum {
	BITRATE_10_KBPS	= 0,	// ungetestet
	BITRATE_20_KBPS	= 1,	// ungetestet
	BITRATE_50_KBPS	= 2,	// ungetestet
	BITRATE_100_KBPS = 3,	// ungetestet
	BITRATE_125_KBPS = 4,
	BITRATE_250_KBPS = 5,	// ungetestet
	BITRATE_500_KBPS = 6,	// ungetestet
	BITRATE_1_MBPS = 7,		// ungetestet
} can_bitrate_t;

typedef struct {
	uint8_t b4;		// lsb
	uint8_t b3;
	uint8_t b2;
	uint8_t b1;		// msb
} long_to_byte_t;

#if defined(__AVR_AT90CAN128__)
  const uint8_t _speed_cnf[8][3] PROGMEM = {
	{ 0x7E, 0x6E, 0x7F },   // 10 kbps
	{ 0x62, 0x0C, 0x37 },   // 20 kbps
	{ 0x26, 0x0C, 0x37 },   // 50 kbps
	{ 0x12, 0x0C, 0x37 },   // 100 kbps
	{ 0x0E, 0x0C, 0x37 },   // 125 kbps
	{ 0x06, 0x0C, 0x37 },   // 250 kbps
	{ 0x02, 0x0C, 0x37 },   // 500 kbps
	{ 0x00, 0x0C, 0x36 }    // 1 Mbps
};
#else
  const uint8_t _speed_cnf[8][3] PROGMEM = {};
#endif

// -------------------------------------------------------------------------
//	buffer setting and variables
// Number of CAN messages which are buffered in RAM additinally to the MObs

#define CAN_RX_BUFFER_SIZE		16
// #define CAN_TX_BUFFER_SIZE		8
#define CAN_TX_BUFFER_SIZE		1	// Enforce Tx Ordering by limiting Tx Buffer Size to 1 frame

static can_buffer_t can_rx_buffer;
static can_t can_rx_list[CAN_RX_BUFFER_SIZE];

static can_t buf;

static can_buffer_t can_tx_buffer;
static can_t can_tx_list[CAN_TX_BUFFER_SIZE];

static volatile uint8_t _transmission_in_progress = 0;

// -------------------------------------------------------------------------
// private buffer functions

uint8_t _find_free_mob(void);
void _disable_mob_interrupt(uint8_t mob);
void _enable_mob_interrupt(uint8_t mob);
uint8_t send_message(const can_t *msg);
uint8_t get_message(can_t *msg);
void copy_message_to_mob(const can_t *msg);
bool copy_mob_to_message(can_t *msg);
bool _check_message(void);
bool _check_free_buffer(void);

void can_buffer_init(can_buffer_t *buf, uint8_t size, can_t *list);
bool can_buffer_empty(can_buffer_t *buf);
bool can_buffer_full(can_buffer_t *buf);
bool can_buffer_get_enqueue_ptr(can_buffer_t *buf, can_t * ptr);
void can_buffer_enqueue(can_buffer_t *buf);
bool can_buffer_get_dequeue_ptr(can_buffer_t *buf, can_t * ptr);
void can_buffer_dequeue(can_buffer_t *buf);

class CanBus {
  public:
	CanBus();
	//bool init(uint8_t bitrate);
    bool init();
    
    bool check_message(void);
    bool check_free_buffer(void);
    
	bool set_filter(uint8_t number, const can_filter_t *filter);

	uint8_t get_filter(uint8_t number, can_filter_t *filter);
	bool disable_filter(uint8_t number);

	uint8_t send_buffered_message(const can_t *msg);
	uint8_t get_buffered_message(can_t *msg);

	bool read_error_register(can_error_register_t error);
	void set_mode(can_mode_t mode);

  private:
	void _enter_standby_mode(void);
	void _leave_standby_mode(void);
};


#endif // 128 only
#endif // CAN_BUS_H
