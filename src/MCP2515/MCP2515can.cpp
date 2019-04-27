// OpenLCB Adaptation of FlexCAN library
// copyright DPH 2017
#include <Arduino.h>
#include "MCP2515can.h"
#include "spi.h"


//tCAN CAN;              // Olcb buffer
tCAN CAN;       // MCP2515 buffer

#define	BITRATE_10_KBPS	0	// ungetestet
#define	BITRATE_20_KBPS	1	// ungetestet
#define	BITRATE_50_KBPS	2	// ungetestet
#define	BITRATE_100_KBPS	3	// ungetestet
#define	BITRATE_125_KBPS	4
#define	BITRATE_250_KBPS	5
#define	BITRATE_500_KBPS	6
#define	BITRATE_1_MBPS	7

#define ST_RX0IF	0x01	// New Message in Rx Buffer 0
#define ST_RX1IF	0x02	// New Message in Rx Buffer 1
#define ST_TX0REQ	0x04	// Request to Send Tx Buffer 0
#define ST_TX0IF	0x08	// Tx Buffer 0 Sent or has become Empty See Section 7.2
#define ST_TX1REQ	0x10	// Request to Send Tx Buffer 1
#define ST_TX1IF	0x20	// Tx Buffer 1 Sent or has become Empty See Section 7.2
#define ST_TX2REQ	0x40	// Request to Send Tx Buffer 2
#define ST_TX2IF	0x80	// Tx Buffer 2 Sent or has become Empty See Section 7.2

#ifdef	SUPPORT_FOR_MCP2515__
    #ifndef	MCP2515_CLKOUT_PRESCALER
        #error	MCP2515_CLKOUT_PRESCALER not defined!
    #elif MCP2515_CLKOUT_PRESCALER == 0
        #define	CLKOUT_PRESCALER_	0x0
    #elif MCP2515_CLKOUT_PRESCALER == 1
        #define	CLKOUT_PRESCALER_	0x4
    #elif MCP2515_CLKOUT_PRESCALER == 2
        #define	CLKOUT_PRESCALER_	0x5
    #elif MCP2515_CLKOUT_PRESCALER == 4
        #define	CLKOUT_PRESCALER_	0x6
    #elif MCP2515_CLKOUT_PRESCALER == 8
        #define	CLKOUT_PRESCALER_	0x7
    #else
        #error	invaild value of MCP2515_CLKOUT_PRESCALER
    #endif
#endif

// -------------------------------------------------------------------------
void mcp2515_write_register( uint8_t adress, uint8_t data )
{
    RESET(MCP2515_CS);
    spi_putc(SPI_WRITE);
    spi_putc(adress);
    spi_putc(data);
    SET(MCP2515_CS);
}

// -------------------------------------------------------------------------
uint8_t mcp2515_read_register(uint8_t adress)
{
    uint8_t data;
    RESET(MCP2515_CS);
    spi_putc(SPI_READ);
    spi_putc(adress);
    data = spi_putc(0xff);
    SET(MCP2515_CS);
    return data;
}

// -------------------------------------------------------------------------
void mcp2515_bit_modify(uint8_t adress, uint8_t mask, uint8_t data)
{
    RESET(MCP2515_CS);
    spi_putc(SPI_BIT_MODIFY);
    spi_putc(adress);
    spi_putc(mask);
    spi_putc(data);
    SET(MCP2515_CS);
}

// ----------------------------------------------------------------------------
uint8_t mcp2515_read_status(uint8_t type)
{
    uint8_t data;
                    //Serial.print(F("\nmcp2515_read_status="));Serial.print(type,HEX);Serial.print(":");
    RESET(MCP2515_CS);
    spi_putc(type);
    data = spi_putc(0xff);
    SET(MCP2515_CS);
                    //Serial.print(data,HEX);
    return data;
}

// -------------------------------------------------------------------------

const uint8_t _mcp2515_cnf[8][3] PROGMEM = {
    // Bit-timing constatnts
    { 0x04, 0xb6, 0xe7 },    // 10 kbps
    { 0x04, 0xb6, 0xd3 },    // 20 kbps
    { 0x04, 0xb6, 0xc7 },    // 50 kbps
    { 0x04, 0xb6, 0xc3 },    // 100 kbps
    { (1<<PHSEG21), 					// CNF3      // 125 kbps
      (1<<BTLMODE)|(1<<PHSEG11),		// CNF2
      (1<<BRP2)|(1<<BRP1)|(1<<BRP0)	// CNF1
    },
    { 0x03, 0xac, 0x81 },    // 250 kbps
    { 0x03, 0xac, 0x80 },    // 500 kbps
    { (1<<PHSEG21), (1<<BTLMODE)|(1<<PHSEG11), 0 }    // 1 Mbps
};

void mcp2515_write_id(const uint32_t *id, uint8_t extended) {
    uint8_t tmp;
    if (extended) {
        spi_start(*((uint16_t *) id + 1) >> 5);
        // naechsten Werte berechnen
        tmp  = (*((uint8_t *) id + 2) << 3) & 0xe0;
        tmp |= (1 << IDE);
        tmp |= (*((uint8_t *) id + 2)) & 0x03;
        // warten bis der vorherige Werte geschrieben wurde
        spi_wait();
        // restliche Werte schreiben
        spi_putc(tmp);
        spi_putc(*((uint8_t *) id + 1));
        spi_putc(*((uint8_t *) id));
    }
    else {
        spi_start(*((uint16_t *) id) >> 3);
        // naechsten Werte berechnen
        tmp = *((uint8_t *) id) << 5;
        spi_wait();
        spi_putc(tmp);
        spi_putc(0);
        spi_putc(0);
    }
}

uint8_t mcp2515_read_id(uint32_t *id) {
    uint8_t first;
    uint8_t tmp;
    first = spi_putc(0xff);
    tmp   = spi_putc(0xff);
    if (tmp & (1 << IDE)) {
        spi_start(0xff);
        *((uint16_t *) id + 1)  = (uint16_t) first << 5;
        *((uint8_t *)  id + 1)  = spi_wait();
        spi_start(0xff);
        *((uint8_t *)  id + 2) |= (tmp >> 3) & 0x1C;
        *((uint8_t *)  id + 2) |=  tmp & 0x03;
        *((uint8_t *)  id)      = spi_wait();
        return TRUE;
    } else {
        spi_start(0xff);
        *((uint16_t *) id) = (uint16_t) first << 3;
        spi_wait();
        spi_start(0xff);
        *((uint8_t *) id) |= tmp >> 5;
        spi_wait();
        return FALSE;
    }
}


// -------------------------------------------------------------------------
//bool mcp2515_init(uint8_t bitrate)
bool mcp2515_init()
{
    uint8_t bitrate = 4;
    if (bitrate >= 8)
        return false;
    SET(MCP2515_CS);
    SET_OUTPUT(MCP2515_CS);
    // Activate the SPI pins        - Aktivieren der Pins fuer das SPI Interface
    RESET(P_SCK);
    RESET(P_MOSI);
    RESET(P_MISO);
    SET_OUTPUT(P_SCK);
    SET_OUTPUT(P_MOSI);
    SET_INPUT(P_MISO);
    // SPI settings  -  SPI Einstellung setzen
    mcp2515_spi_init();
    // Reset the MCP2515            -- MCP2515 per Software Reset zuruecksetzten,
    // it is then in config mode    -- danach ist er automatisch im Konfigurations Modus
    RESET(MCP2515_CS);
    spi_putc(SPI_RESET);
    SET(MCP2515_CS);
    // delay while it restarts      -- ein bisschen warten bis der MCP2515 sich neu gestartet hat
    _delay_ms(0.1);
    // CNF1-3 are bit-timing regs   -- CNF1..3 Register laden (Bittiming)
    RESET(MCP2515_CS);
    spi_putc(SPI_WRITE);
    spi_putc(CNF3);
    uint8_t i;
    for (i=0; i<3 ;i++ ) {
        spi_putc(pgm_read_byte(&_mcp2515_cnf[bitrate][i]));
    }
    // enable interrupts            -- aktivieren/deaktivieren der Interrupts
    spi_putc(MCP2515_INTERRUPTS);
    SET(MCP2515_CS);
    // set Tx and RTS as inputs     -- TXnRTS Bits als Inputs schalten
    mcp2515_write_register(TXRTSCTRL, 0);
#if defined(MCP2515_DISABLE_MASKS_AND_FILTERS)
    mcp2515_write_register(RXB0CTRL, (1<<RXM1)|(1<<RXM0)|(1<<BUKT));
    mcp2515_write_register(RXB1CTRL, (1<<RXM1)|(1<<RXM0));
#endif
#if defined(MCP2515_INT)
    SET_INPUT(MCP2515_INT);
    SET(MCP2515_INT);
#endif
#ifdef RXnBF_FUNKTION
    SET_INPUT(MCP2515_RX0BF);
    SET_INPUT(MCP2515_RX1BF);
    SET(MCP2515_RX0BF);
    SET(MCP2515_RX1BF);
    // Acivate TX)BF and RX1BF pins     -- Aktivieren der Pin-Funktionen fuer RX0BF und RX1BF
    mcp2515_write_register(BFPCTRL, (1<<B0BFE)|(1<<B1BFE)|(1<<B0BFM)|(1<<B1BFM));
#else
    // Disable RXnBF to hi-Z            -- Deaktivieren der Pins RXnBF Pins (High Impedance State)
    mcp2515_write_register(BFPCTRL, 0);
#endif
    // See if we can access the writable regs   -- Testen ob das auf die beschreibenen Register zugegriffen werden kann
    // (Can we talk to the chip?)                -- (=> ist der Chip ueberhaupt ansprechbar?)
    bool error = false;
    if (mcp2515_read_register(CNF1) != pgm_read_byte(&_mcp2515_cnf[bitrate][2])) {
        error = true;
    }
    // Set 'Normal' mode                -- Device zurueck in den normalen Modus versetzten
    // and enable clkoyt pin            -- und aktivieren/deaktivieren des Clkout-Pins
    mcp2515_write_register(CANCTRL, CLKOUT_PRESCALER_);
    if (error) {
        return false;
    }
    else
    {
        while ((mcp2515_read_register(CANSTAT) & 0xe0) != 0) {
            // Wait for new mode to be adopted.     -- warten bis der neue Modus uebernommen wurde
        }
        
        return true;
    }
}
bool mcp2515_check_free_buffer(void) {
    uint8_t status = mcp2515_read_status(SPI_READ_STATUS);
    if ((status & 0x54) == 0x54)
        return false;		// all buffers used
    else
        return true;
}

// ----------------------------------------------------------------------------
// get the current combined Tx and Rx Buffer Status
bool can_check_free_buffer() {
    uint8_t status = mcp2515_read_status(SPI_READ_STATUS);
    //  Check to see if Tx Buffer 0,1 and 2 are all free
    if ((status & (ST_TX0REQ|ST_TX1REQ|ST_TX2REQ)) == 0)
        return true;   // All empty, nothing to send
    else
        return false;  // Any full
}
//uint8_t can_buffers_status(void) {
//    return mcp2515_read_status(SPI_READ_STATUS);
//}



// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Setzen eines Filters
 * 
 * Für einen MCP2515 sollte die Funktion can_static_filter() bevorzugt werden.
 *
 * \param	number	Position des Filters
 * \param	filter	zu setzender Filter
 *
 * \return	false falls ein Fehler auftrat, true ansonsten
 */
extern bool can_set_filter(uint8_t number, const tCANFilter *filter);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Filter deaktivieren
 *
 * \param	number	Nummer des Filters der deaktiviert werden soll,
 *			0xff deaktiviert alle Filter.
 * \return	false falls ein Fehler auftrat, true ansonsten
 *
 * \warning Wird nur vom AT90CAN32/64/128 unterstuetzt.
 */
extern bool can_disable_filter(uint8_t number);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Setzt die Werte für alle Filter
 *
 * \param	*filter_array	Array im Flash des AVRs mit den Initialisierungs-
 *							werten für die Filter des MCP2515
 * 
 * \see		MCP2515_FILTER_EXTENDED()
 * \see		MCP2515_FILTER()
 * \warning	Wird nur vom MCP2515 unterstuetzt.
 */
extern void can_static_filter(const uint8_t *filter_array);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * 
 * \~german
 * \brief	Filterdaten auslesen
 *
 * \param	number	Nummer des Filters dessen Daten man haben moechte
 * \param	*filter	Pointer in den die Filterstruktur geschrieben wird
 *
 * \return	\b 0 falls ein Fehler auftrat, \
 *			\b 1 falls der Filter korrekt gelesen werden konnte, \
 *			\b 2 falls der Filter im Moment nicht verwendet wird (nur AT90CAN), \
 *			\b 0xff falls gerade keine Aussage moeglich ist (nur AT90CAN).
 *
 * \warning	Da der SJA1000 nicht feststellen kann ob der ausgelesene Filter
 *			nun zwei 11-Bit Filter oder ein 29-Bit Filter ist werden nicht
 *			die Filter sondern die Registerinhalte direkt zurück gegeben.
 *			Der Programmierer muss dann selbst entscheiden was er mit den 
 * 			Werten macht.
 *
 * \~english
 * \warning SJA1000 doesn't return the filter and id directly but the content
 *			of the corresponding registers because it is not possible to
 *			check the type of the filter.
 */
extern uint8_t can_get_filter(uint8_t number, tCANFilter *filter);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Ueberpruefen ob neue CAN Nachrichten vorhanden sind
 *
 * \return	true falls neue Nachrichten verfuegbar sind, false ansonsten.
 */
bool mcp2515_check_message(void) {
    #if defined(MCP2515_INT)
     return ((!IS_SET(MCP2515_INT)) ? true : false);
    #else
     #ifdef RXnBF_FUNKTION
      if (!IS_SET(MCP2515_RX0BF) || !IS_SET(MCP2515_RX1BF))
        return true;
      else
        return false;
     #else
     return ((mcp2515_read_status(SPI_RX_STATUS) & 0xC0) ? true : false);
     #endif
    #endif
}


// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Ueberprueft ob ein Puffer zum Versenden einer Nachricht frei ist.
 *
 * \return	true falls ein Sende-Puffer frei ist, false ansonsten.
 */

uint8_t mcp2515_read_status_r;

bool can_check_message(void)
{
                    //Serial.print("\nIn mcp2515 can_check_free_buffer:");
    //return true;  //**********!!!!!!!!!!!!!!************
    #if defined(MCP2515_INT)
     return ((!IS_SET(MCP2515_INT)) ? true : false);
    //#pragma message("defined(MCP2515_INT enabled")
    #else
       #ifdef RXnBF_FUNKTION
        if (!IS_SET(MCP2515_RX0BF) || !IS_SET(MCP2515_RX1BF))
            return true;
        else
            return false;
       #else
        return ((mcp2515_read_status_r = mcp2515_read_status(SPI_RX_STATUS) & 0xC0) ? true : false);
       #endif
    #endif
}

extern uint8_t can_buffers_status(void);
// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Verschickt eine Nachricht über den CAN Bus
 *
 * \param	msg	Nachricht die verschickt werden soll
 * \return	FALSE falls die Nachricht nicht verschickt werden konnte, \n
 *			ansonsten der Code des Puffes in den die Nachricht gespeichert wurde
 *           Returns buffer number or 0
 */
#include <util/delay.h>
uint8_t can_send_message(const tCAN *msg)
{
                        //Serial.print("\nIn mcp2515 can_send_message:");
                        //Serial.print(msg->id,HEX);
    // Status des MCP2515 auslesen
    uint8_t status = mcp2515_read_status(SPI_READ_STATUS);
    /* Statusbyte:
     *
     * Bit	Funktion
     *  2	TXB0CNTRL.TXREQ
     *  4	TXB1CNTRL.TXREQ
     *  6	TXB2CNTRL.TXREQ
     */
    uint8_t address;
    if (_bit_is_clear(status, 2)) {
        address = 0x00;
    }
    else if (_bit_is_clear(status, 4)) {
        address = 0x02;
    }
    else if (_bit_is_clear(status, 6)) {
        address = 0x04;
    }
    else {
        // Alle Puffer sind belegt,
        // Nachricht kann nicht verschickt werden
        return 0;
    }
    RESET(MCP2515_CS);
    spi_putc(SPI_WRITE_TX | address);
    #if SUPPORT_EXTENDED_CANID
     mcp2515_write_id(&msg->id, msg->flags.extended);
    #else
     mcp2515_write_id(&msg->id);
    #endif
    uint8_t length = msg->length & 0x0f;
    // Ist die Nachricht ein "Remote Transmit Request" ?
    if (msg->flags.rtr)
    {
        // Ein RTR hat zwar eine Laenge,
        // enthaelt aber keine Daten
        // Nachrichten Laenge + RTR einstellen
        spi_putc((1<<RTR) | length);
    }
    else
    {
        // Nachrichten Laenge einstellen
        spi_putc(length);
        // Daten
        uint8_t i;
        for (i=0;i<length;i++) {
            spi_putc(msg->data[i]);
        }
    }
    SET(MCP2515_CS);
    _delay_us(1);
    // CAN Nachricht verschicken
    // die letzten drei Bit im RTS Kommando geben an welcher
    // Puffer gesendet werden soll.
    RESET(MCP2515_CS);
    address = (address == 0) ? 1 : address;
    spi_putc(SPI_RTS | address);
    SET(MCP2515_CS);
    return address;
}



// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Liest eine Nachricht aus den Empfangspuffern des CAN Controllers
 *
 * \param	msg	Pointer auf die Nachricht die gelesen werden soll.
 * \return	FALSE falls die Nachricht nicht ausgelesen konnte,
 *			ansonsten Filtercode welcher die Nachricht akzeptiert hat.
 *           Returns filter code or 0
 */
uint8_t can_get_message(tCAN *msg) {
                    //Serial.print(F("\nIn mcp2515 can_get_message0"));
    uint8_t lastRxBuf1Status = 0;
    uint8_t addr;
    #ifdef	RXnBF_FUNKTION
                    //Serial.print(F("\nIn mcp2515 can_get_message1"));while(0==0){}
      if (!IS_SET(MCP2515_RX0BF))
        addr = SPI_READ_RX;
      else if (!IS_SET(MCP2515_RX1BF))
        addr = SPI_READ_RX | 0x04;
      else {
        return 0;
      }
    #else
      // read status
                    //Serial.print(F("\nIn mcp2515 can_get_message2")); delay(500);
      uint8_t status = mcp2515_read_status(SPI_RX_STATUS);
                   //Serial.print(F("\n status=")); Serial.print(status);delay(500);
      if (lastRxBuf1Status & status) {
        // message in buffer 1 from last time so read it first this time
        addr = SPI_READ_RX | 0x04;
        lastRxBuf1Status = 0;
      } else if (_bit_is_set(status,6)) {
        // message in buffer 0
        addr = SPI_READ_RX;
        lastRxBuf1Status = status & (uint8_t)0x80;
      } else if (_bit_is_set(status,7)) {
        // message in buffer 1
        addr = SPI_READ_RX | 0x04;
        lastRxBuf1Status = 0;
      } else {
        // Error: no message available
                        //Serial.print(F("\nIn mcp2515 2return0"));delay(500);
        return 0;
      }
                        //Serial.print(F("\nIn mcp2515 2addr="));Serial.print(addr); delay(500);
    #endif
                        //Serial.print(F("\nIn mcp2515 can_get_message4")); delay(500);
    RESET(MCP2515_CS);
    spi_putc(addr);
    // CAN ID auslesen und ueberpruefen
    uint8_t tmp = mcp2515_read_id(&msg->id);
    #if SUPPORT_EXTENDED_CANID
        msg->flags.extended = tmp & 0x01;
    #else
        if (tmp & 0x01) {
            // Nachrichten mit extended ID verwerfen
            SET(MCP2515_CS);
            #ifdef	RXnBF_FUNKTION
            if (!IS_SET(MCP2515_RX0BF))
            #else
            if (_bit_is_set(status, 6))
            #endif
            mcp2515_bit_modify(CANINTF, (1<<RX0IF), 0);
        else
            mcp2515_bit_modify(CANINTF, (1<<RX1IF), 0);
            return 0;
        }
    #endif
    // read DLC
    uint8_t length = spi_putc(0xff);
    #ifdef RXnBF_FUNKTION
     if (!(tmp & 0x01))
        msg->flags.rtr = (tmp & 0x02) ? 1 : 0;
     else
        msg->flags.rtr = (length & (1<<RTR)) ? 1 : 0;
    #else
     msg->flags.rtr = (_bit_is_set(status, 3)) ? 1 : 0;
    #endif
    length &= 0x0f;
    msg->length = length;
    // read data
    uint8_t i;
    for (i=0;i<length;i++) {
        msg->data[i] = spi_putc(0xff);
    }
                        //Serial.print(F("\nIn mcp2515 can_get_message5")); delay(500);
    SET(MCP2515_CS);
    // clear interrupt flag
    #ifdef RXnBF_FUNKTION
     if (!IS_SET(MCP2515_RX0BF))
    #else
     if (_bit_is_set(status, 6))
    #endif
        mcp2515_bit_modify(CANINTF, (1<<RX0IF), 0);
     else
        mcp2515_bit_modify(CANINTF, (1<<RX1IF), 0);
    #ifdef RXnBF_FUNKTION
     return 1;
    #else
     return (status & 0x07) + 1;
    #endif
}

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 *
 * \~german
 * \brief   Liest den Inhalt der Fehler-Register
 *
 * \~english
 * \brief	Reads the Contents of the CAN Error Counter
 */
extern tCANErrorRegister can_read_error_register(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup can_interface
 *
 * \~german
 * \brief   Überprüft ob der CAN Controller im Bus-Off-Status
 *
 * \return  true wenn der Bus-Off-Status aktiv ist, false ansonsten
 *
 * \warning aktuell nur auf dem SJA1000
 */
extern bool can_check_bus_off(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 *
 * \~german
 * \brief	Setzt einen Bus-Off Status zurück und schaltet den CAN Controller
 *			wieder aktiv
 *
 * \warning	aktuell nur auf dem SJA1000
 */
extern void can_reset_bus_off(void);

// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Setzt den Operations-Modus
 *
 * \param	mode	Gewünschter Modus des CAN Controllers
 */
extern void can_set_mode(tCANMode mode);

////////////////////////////////////////////////////
// ----------------------------------------------------------------------------
/**
 * \ingroup	can_interface
 * \brief	Initialisierung des CAN Interfaces
 *
 * \param	bitrate	Gewuenschte Geschwindigkeit des CAN Interfaces
 *
 * \return	false falls das CAN Interface nicht initialisiert werden konnte,
 *			true ansonsten.
 */
//bool can_init(uint8_t bitrate) {
bool can_init() {
                        //Serial.print("\nIn MCP2515 can_init");
                        //mcp2515_init(BITRATE_125_KBPS);
    mcp2515_init();
    return true;
}



extern void can_regdump(void);

//#if defined (__cplusplus)
//}
//#endif
