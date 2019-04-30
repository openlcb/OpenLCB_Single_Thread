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

#if defined(__AVR_AT90CAN128__)

#include "CanBus.h"
#include "Arduino.h"

// -------------------------------------------------------------
bool copy_mob_to_message(can_t *msg) {
	// read status
	uint8_t cancdmob = CANCDMOB;
	// read length
	msg->length = cancdmob & 0x0f;
    
#if SUPPORT_EXTENDED_CANID
	if(cancdmob & (1 << IDE)) {
		// extended identifier
		uint32_t tmp;
		uint8_t *ptr = (uint8_t *) &tmp;
		*ptr       = CANIDT4;
		*(ptr + 1) = CANIDT3;
		*(ptr + 2) = CANIDT2;
		*(ptr + 3) = CANIDT1;
		msg->id = tmp >> 3;
		/* equivalent to:
		msg->id  = (uint8_t)  CANIDT4 >> 3;
		msg->id |= (uint32_t) CANIDT3 << 5;
		msg->id |= (uint32_t) CANIDT2 << 13;
		msg->id |= (uint32_t) CANIDT1 << 21;
		*/
		msg->flags.extended = 1;
	} else {
		// standard identifier
		uint16_t id;
		id  = (uint8_t)  CANIDT2 >> 5;
		id |= (uint16_t) CANIDT1 << 3;
		msg->id = (uint32_t) id;
		msg->flags.extended = 0;
	}
#else
	if(cancdmob & (1 << IDE)) {
		return false;
	} else {
		// standard identifier
		msg->id  = (uint8_t)  CANIDT2 >> 5;
		msg->id |= (uint16_t) CANIDT1 << 3;
	}
#endif

	if(CANIDT4 & (1 << RTRTAG)) {
		msg->flags.rtr = 1;
	} else {
		msg->flags.rtr = 0;
		// read data
		uint8_t *p = msg->data;
		for (uint8_t i = 0; i < msg->length; i++) {
			*p++ = CANMSG;
		}
	}

#if SUPPORT_TIMESTAMPS
	msg->timestamp = CANSTM;
#endif

	return true;
}

// ----------------------------------------------------------------------------

uint8_t get_message(can_t *msg) {
	bool found = false;
	uint8_t mob;
	// check if there is any waiting message
	if(!_check_message()) return 0;
	// find the MOb with the received message
	for(mob = 0; mob < 15; mob++) {
		CANPAGE = mob << 4;
		if (CANSTMOB & (1 << RXOK)) {
			found = true;
			// clear flags
			CANSTMOB &= 0;
			break;
		}
	}

	if(!found) return 0;		// should never happen
	found = copy_mob_to_message( msg );

    // re-enable interrupts
	_enable_mob_interrupt( mob );

    // clear flags
	CANCDMOB = (1 << CONMOB1) | (CANCDMOB & (1 << IDE));
	if (found) return (mob + 1);
	else	   return 0;  // only if SUPPORT_EXTENDED_CANID=0 and a extended message was received
}

void copy_message_to_mob(const can_t *msg) {
	// write DLC (Data Length Code)
	CANCDMOB = msg->length;

#if SUPPORT_EXTENDED_CANID
	if (msg->flags.extended) {
		// extended CAN ID
		CANCDMOB |= (1 << IDE);
		CANIDT4 = (uint8_t)  msg->id << 3;
		uint32_t temp = msg->id << 3;
		uint8_t *ptr = (uint8_t *) &temp;
		CANIDT3 = *(ptr + 1);
		CANIDT2 = *(ptr + 2);
		CANIDT1 = *(ptr + 3);
	} else {
		// standard CAN ID
		CANIDT4 = 0;
		CANIDT3 = 0;
		CANIDT2 = (uint8_t)  msg->id << 5;
		CANIDT1 = (uint16_t) msg->id >> 3;
	}
#else
	CANIDT4 = 0;
	CANIDT3 = 0;
	CANIDT2 = (uint8_t)  msg->id << 5;
	CANIDT1 = (uint16_t) msg->id >> 3;
#endif

	if (msg->flags.rtr) {
		CANIDT4 |= (1 << RTRTAG);
	} else {
		const uint8_t *p = msg->data;
		for (uint8_t i = 0; i < msg->length; i++) {
			CANMSG = *p++;
		}
	}
}

// ----------------------------------------------------------------------------
//uint8_t send_message(const can_t *msg)
uint8_t canbus_send_message( const can_t *msg)
{
	// check if there is any free MOb
	uint8_t mob = _find_free_mob();
	if (mob >= 15)
		return 0;

	// load corresponding MOb page ...
	CANPAGE = (mob << 4);

	// clear flags
	CANSTMOB &= 0;

	// ... and copy the data
	copy_message_to_mob( msg );

	// enable interrupt
	_enable_mob_interrupt(mob);

	ENTER_CRITICAL_SECTION;
#if CAN_FORCE_TX_ORDER
	_transmission_in_progress = 1;
#endif
	LEAVE_CRITICAL_SECTION;

	// enable transmission
	CANCDMOB |= (1 << CONMOB0);

	return (mob + 1);
}


// -----------------------------------------------------------------------------
void can_buffer_init(can_buffer_t *buf, uint8_t size, can_t *list) {
	ENTER_CRITICAL_SECTION;
	buf->size = size;
	buf->buf = list;
	buf->head = 0;
	buf->tail = 0;
	buf->used = 0;
	LEAVE_CRITICAL_SECTION;
}

// -----------------------------------------------------------------------------
bool can_buffer_empty(can_buffer_t *buf) {
	uint8_t used;
	ENTER_CRITICAL_SECTION;
	used = buf->used;
	LEAVE_CRITICAL_SECTION;
	if(used == 0) return true;
	else return false;
}

// -----------------------------------------------------------------------------
bool can_buffer_full(can_buffer_t *buf)
{
                //CAN_DEBUG("\nIn CanBus  can_buffer_full:");
	uint8_t used;
	uint8_t size;
	ENTER_CRITICAL_SECTION;
	used = buf->used;
	size = buf->size;
	LEAVE_CRITICAL_SECTION;
                //CAN_DEBUG("used="); CAN_DEBUG(used);
                //CAN_DEBUG(" size="); CAN_DEBUG(size);
	if(used >= size) return true;
	else	return false;
}

// -----------------------------------------------------------------------------
bool can_buffer_get_enqueue_ptr(can_buffer_t *buf, can_t * ptr) {
	if(can_buffer_full( buf )) return NULL;
	ptr = &buf->buf[buf->head];
	return true;
}

// -----------------------------------------------------------------------------
void can_buffer_enqueue(can_buffer_t *buf) {
	ENTER_CRITICAL_SECTION;
	buf->used ++;
	if (++buf->head >= buf->size)
		buf->head = 0;
	LEAVE_CRITICAL_SECTION;
}

// -----------------------------------------------------------------------------
bool can_buffer_get_dequeue_ptr(can_buffer_t *buf, can_t *ptr) {
	if(can_buffer_empty( buf )) return NULL;
	ptr = &buf->buf[buf->tail];
	return true;
}

// -----------------------------------------------------------------------------
void can_buffer_dequeue(can_buffer_t *buf) {
	ENTER_CRITICAL_SECTION;
	buf->used --;
	if(++buf->tail >= buf->size)
		buf->tail = 0;
	LEAVE_CRITICAL_SECTION;
}


// ----------------------------------------------------------------------------
// get next free MOb
uint8_t _find_free_mob(void) {
  #if CAN_FORCE_TX_ORDER
	if(_transmission_in_progress) return 0xff;
  #endif
	uint8_t i;
	for(i = 0; i < 15; i++) {
		// load MOb page
		CANPAGE = i << 4;

		// check if MOb is in use
		if ((CANCDMOB & ((1 << CONMOB1) | (1 << CONMOB0))) == 0)
			return i;
	}

	return 0xff;
}

// ----------------------------------------------------------------------------
// disable interrupt of corresponding MOb

void _disable_mob_interrupt(uint8_t mob) {
	if(mob < 8) CANIE2 &= ~(1 << mob);
	else CANIE1 &= ~(1 << (mob - 8));
}

// ----------------------------------------------------------------------------
// enable interrupt of corresponding MOb

void _enable_mob_interrupt(uint8_t mob) {
	if(mob < 8) 	CANIE2 |= (1 << mob);
	else CANIE1 |= (1 << (mob - 8));
}

// ----------------------------------------------------------------------------
// Checks if there is any waiting message in the registers

bool _check_message(void) {
    //CAN_DEBUG("\nIn CanBus  _check_message:");
    //bool r = !can_buffer_empty( &can_rx_buffer );
    //CAN_DEBUG(r);
    //return r;
	return !can_buffer_empty( &can_rx_buffer );
}

// ----------------------------------------------------------------------------
bool _check_free_buffer(void) {
    //CAN_DEBUG("\nIn CanBus  _check_free_buffer:");
    //bool r = !can_buffer_full( &can_tx_buffer );
    //CAN_DEBUG(r);
    //while(0==0){}
    //return r;
	return !can_buffer_full( &can_tx_buffer );
}
//-----------------------------------------------------------------------------//
// INTERRUPT ROUTINES

#ifdef OVRIT_vect 
ISR(OVRIT_vect) {}
#endif

#ifdef CANIT_vect
ISR(CANIT_vect) {
	uint8_t canpage;
	uint8_t mob;
	if((CANHPMOB & 0xF0) != 0xF0) {
		// save MOb page register
		canpage = CANPAGE;
		// select MOb page with the highest priority
		CANPAGE = CANHPMOB & 0xF0;
		mob = (CANHPMOB >> 4);
		// a interrupt is only generated if a message was transmitted or received
		if(CANSTMOB & (1 << TXOK)) {
			// clear MOb
			CANSTMOB &= 0;
			CANCDMOB = 0;
			bool result = can_buffer_get_dequeue_ptr(&can_tx_buffer, &buf);
			// check if there are any another messages waiting
			if(result) {
				copy_message_to_mob( &buf );
				can_buffer_dequeue(&can_tx_buffer);

				// enable transmission
				CANCDMOB |= (1 << CONMOB0);
			} else {
				// buffer underflow => no more messages to send
				_disable_mob_interrupt(mob);
				_transmission_in_progress = 0;
			}
			CAN_INDICATE_TX_TRAFFIC_FUNCTION;
		} else {
			// a message was received successfully
			bool result = can_buffer_get_enqueue_ptr(&can_rx_buffer, &buf);
			if(result) {
				// read message
				copy_mob_to_message( &buf );
				// push it to the list
				can_buffer_enqueue(&can_rx_buffer);
			} else {
				// buffer overflow => reject message
				// FIXME inform the user
			}
			// clear flags
			CANSTMOB &= 0;
			CANCDMOB = (1 << CONMOB1) | (CANCDMOB & (1 << IDE));
			CAN_INDICATE_RX_TRAFFIC_FUNCTION;
		}
		// restore MOb page register
		CANPAGE = canpage;
	} else {
		// no MOb matches with the interrupt => general interrupt
		CANGIT |= 0;
	}
}
#endif //CANIT_vect

// ---CLASS DEFINITIONS-------------------------------------------------------------------------

CanBus::CanBus() {}

// -----------------------------------------------------------------------------
// check buffers
bool CanBus::check_message(void) {
    //CAN_DEBUG("\nIn CanBus::check_message:");
    bool r = _check_message();
    //CAN_DEBUG(r);
    return r;
    //return _check_message();
}
bool CanBus::check_free_buffer(void) {
    //CAN_DEBUG("\nIn CanBus::check_free_buffer:");
    bool r = _check_free_buffer();
    //CAN_DEBUG(r);
    return r;
    //return _check_free_buffer();
}


// ----------------------------------------------------------------------------
// disable mob

bool CanBus::disable_filter(uint8_t number) {
	if(number > 14) {
		if(number == CAN_ALL_FILTER) {
			// disable interrupts
			CANIE1 = 0;
			CANIE2 = 0;
			// disable all MObs
			for(uint8_t i = 0; i < 15; i++) {
				CANPAGE = (i << 4);
				// disable MOb (read-write required)
				CANCDMOB &= 0;
				CANSTMOB &= 0;
			}
  #if CAN_TX_BUFFER_SIZE == 0
			_free_buffer = 15;
  #endif
			return true;
		}
		// it is only possible to serve a maximum of 15 filters
		return false;
	}
	// set CAN Controller to standby mode
	_enter_standby_mode();
	CANPAGE = number << 4;
	// reset flags
	CANSTMOB &= 0;
	CANCDMOB = 0;
	_disable_mob_interrupt(number);
	// re-enable CAN Controller
	_leave_standby_mode();
	return true;
}

// ----------------------------------------------------------------------------
bool CanBus::read_error_register(can_error_register_t error) {
	error.tx = CANTEC;
	error.rx = CANREC;
	return true;
}

// ----------------------------------------------------------------------------
uint8_t CanBus::get_buffered_message(can_t *msg) {
	// get pointer to the first buffered message
	bool result = can_buffer_get_dequeue_ptr(&can_rx_buffer, &buf);
	if(!result)
		return 0;
	// copy the message
	memcpy( msg, &buf, sizeof(can_t) );
	// delete message from the queue
	can_buffer_dequeue(&can_rx_buffer);
	return 0xff;
}

// ----------------------------------------------------------------------------

uint8_t CanBus::get_filter(uint8_t number, can_filter_t *filter)
{
	if (number > 14) {
		// it is only possible to serve a maximum of 15 filters
		return 0;
	}

	// load corresponding MOb page
	CANPAGE = number << 4;

	if ((CANCDMOB & 0xc0) == 0) {
		// MOb is currently not used.
		return 2;
	}  else if ((CANCDMOB & 0xc0) == (1 << CONMOB1))	{
		// MOb is configured to receive message => read filter.
		if (CANIDM4 & (1 << RTRMSK)) {
			if (CANIDT4 & (1 << RTRMSK)) {
				// receive only messages with RTR-bit set
				filter->flags.rtr = 0x3;
			}  else {
				filter->flags.rtr = 0x2;
			}
		}  else {
			// receive all message, independent from RTR-bit
			filter->flags.rtr = 0;
		}
  #if SUPPORT_EXTENDED_CANID
		if((CANIDM4 & (1 << IDEMSK)) && (CANCDMOB & (1 << IDE))) {
			filter->flags.extended = 0x3;
			// extended id
			uint32_t mask;
			mask  = (uint8_t)  CANIDM4 >> 3;
			mask |= (uint16_t) CANIDM3 << 5;
			mask |= (uint32_t) CANIDM2 << 13;
			mask |= (uint32_t) CANIDM1 << 21;
			filter->mask = mask;
			uint32_t id;
			id  = (uint8_t)  CANIDT4 >> 3;
			id |= (uint16_t) CANIDT3 << 5;
			id |= (uint32_t) CANIDT2 << 13;
			id |= (uint32_t) CANIDT1 << 21;
			// only the bits set in the mask are vaild for the id
			filter->id = id & mask;
		} else {
			if(CANIDM4 & (1 << IDEMSK)) filter->flags.extended = 0x2;
			else filter->flags.extended = 0;
			uint16_t mask;
			mask  = (uint8_t)  CANIDM2 >> 5;
			mask |= (uint16_t) CANIDM1 << 3;
			filter->mask = mask;
			uint16_t id;
			id  = (uint8_t)  CANIDT2 >> 5;
			id |= (uint16_t) CANIDT1 << 3;
			filter->id = id & mask;
		}
  #else
		uint16_t mask;
		mask  = (uint8_t)  CANIDM2 >> 5;
		mask |= (uint16_t) CANIDM1 << 3;
		filter->mask = mask;
		uint16_t id;
		id  = (uint8_t)  CANIDT2 >> 5;
		id |= (uint16_t) CANIDT1 << 3;

		filter->id = id & mask;
  #endif
		return 1;
	}

	// MOb is currently used to transmit a message.
	return 0xff;
}

// -----------------------------------------------------------------------------
uint8_t CanBus::send_buffered_message(const can_t *msg) {
	// check if there is any free buffer left
                        //Serial.print("\nIn CanBus::send_buffered_message id=");
                        //Serial.print(msg->id,HEX);
  #if CAN_FORCE_TX_ORDER
	if (_transmission_in_progress)
  #else
	if (_find_free_mob() == 0xff)
  #endif
	{
		bool result = can_buffer_get_enqueue_ptr(&can_tx_buffer, &buf);
		if (!result) return 0;		// buffer full
		// copy message to the buffer
		memcpy( &buf, msg, sizeof(can_t) );
		// In the interrupt it is checked if there are any waiting messages
		// in the queue, otherwise the interrupt will be disabled.
		// So, if the transmission finished while we are in this routine the
		// message will be queued but not send.
		// Therefore interrupts have to disabled while putting the message
		// to the queue.
		bool enqueued = false;
  #if CAN_FORCE_TX_ORDER
		ENTER_CRITICAL_SECTION;
		if(_transmission_in_progress) {
			can_buffer_enqueue(&can_tx_buffer);
			enqueued = true;
		}
		LEAVE_CRITICAL_SECTION;
  #endif
		if (enqueued) return 1;
        else return canbus_send_message( msg ); // buffer gets free while we where preparing the message, so send message directly
    } else return canbus_send_message( msg );
}

// ----------------------------------------------------------------------------
bool CanBus::set_filter(uint8_t number, const can_filter_t *filter) {
	if(number > 14) return false;	// it is only possible to serve a maximum of 15 filters
	// set CAN Controller to standby mode
	_enter_standby_mode();
	CANPAGE = number << 4;
	CANSTMOB = 0;
	CANCDMOB = 0;
  #if SUPPORT_EXTENDED_CANID
	if(filter->flags.extended == 0x3) {
		// extended identifier
		CANIDT4 = (uint8_t)  filter->id << 3;
		CANIDT3 = 			 filter->id >> 5;
		CANIDT2 =            filter->id >> 13;
		CANIDT1 =            filter->id >> 21;
		CANIDM4 = ((uint8_t) filter->mask << 3) | (1 << IDEMSK);
		CANIDM3 = 			 filter->mask >> 5;
		CANIDM2 =            filter->mask >> 13;
		CANIDM1 =            filter->mask >> 21;
		CANCDMOB |= (1 << IDE);
	} else {
		CANIDT4 = 0;
		CANIDT3 = 0;
		CANIDT2 = (uint8_t)  filter->id << 5;
		CANIDT1 = (uint16_t) filter->id >> 3;
		if(filter->flags.extended) CANIDM4 = (1 << IDEMSK);		// receive only standard frames
		else CANIDM4 = 0;					// receive all frames
		CANIDM3 = 0;
		CANIDM2 = (uint8_t)  filter->mask << 5;
		CANIDM1 = (uint16_t) filter->mask >> 3;
	}
  #else
	CANIDT4 = 0;
	CANIDT3 = 0;
	CANIDT2 = (uint8_t)  filter->id << 5;
	CANIDT1 = (uint16_t) filter->id >> 3;
	CANIDM4 = (1 << IDEMSK);
	CANIDM3 = 0;
	CANIDM2 = (uint8_t)  filter->mask << 5;
	CANIDM1 = (uint16_t) filter->mask >> 3;
  #endif
	if(filter->flags.rtr & 0x2) {
		CANIDM4 |= (1 << RTRMSK);
		if(filter->flags.rtr & 0x1)
			CANIDT4 |= (1 << RTRMSK);		// only RTR-frames
	}
	CANCDMOB |= (1 << CONMOB1);
	_enable_mob_interrupt(number);
	// re-enable CAN Controller
	_leave_standby_mode();
	return true;
}

void CanBus::set_mode(can_mode_t mode) {
	if(mode == LISTEN_ONLY_MODE || mode == LOOPBACK_MODE) CANGCON |= (1 << LISTEN);
	else CANGCON &= ~(1 << LISTEN);
}

//bool CanBus::init(uint8_t bitrate) {
bool CanBus::init() {
    //CAN_DEBUG("\nIn CanBus::init");
                        //Serial.print("\nIn CanBus::init");
    uint8_t bitrate = 4; // fixed at 125k
	if (bitrate >= 8) return false;
	// switch CAN controller to reset mode
	CANGCON |= (1 << SWRES);
	// set CAN Bit Timing
	// (see datasheet page 260)
	CANBT1 = pgm_read_byte(&_speed_cnf[bitrate][0]);
	CANBT2 = pgm_read_byte(&_speed_cnf[bitrate][1]);
	CANBT3 = pgm_read_byte(&_speed_cnf[bitrate][2]);
	// activate CAN transmit- and receive-interrupt
	CANGIT = 0;
	CANGIE = (1 << ENIT) | (1 << ENRX) | (1 << ENTX);
	// set timer prescaler to 199 which results in a timer
	// frequency of 10 kHz (at 16 MHz)
	CANTCON = 199;
	// disable all filters
	disable_filter( 0xff );

	#if CAN_RX_BUFFER_SIZE > 0
	can_buffer_init( &can_rx_buffer, CAN_RX_BUFFER_SIZE, can_rx_list );
	#endif
	
	#if CAN_TX_BUFFER_SIZE > 0
	can_buffer_init( &can_tx_buffer, CAN_TX_BUFFER_SIZE, can_tx_list );
	#endif
	
		//Clear all mailboxes. DEG 22 May 2011
	uint8_t i;
	for(i = 0; i < 15; ++i)
	{
		CANPAGE = (i << 4);
		//clear any interrupt flags
		CANSTMOB = 0;
	//DEG 23 May 2011. Configure each mailbox.
	// From Datasheet, 19.5.2:
	// "There is no default mode after RESET.
        // "Every MOb has its own fields to control the operating mode.
        // "Before enabling the CAN peripheral, each MOb must be
	// "configured (ex: disabled mode - CONMOB=00)."
	// Just going to configure them all as receive. Don't know that
	// This is corect. Maybe they should be set to disable, but one?
	// H/T http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&t=107164
		//IDT, RTRTAG, RBnTAG
		CANIDT1 = 0;                        //ID 
		CANIDT2 = 0; 
		CANIDT3 = 0; 
		CANIDT4 = 0; 
		//IDMSK, IDEMSK, RTRMSK
		CANIDM1 = 0;                        //get all messages 
		CANIDM2 = 0;                        //1 = check bit 
		CANIDM3 = 0;                        //0 = ignore bit 
		CANIDM4 = 0; //(1<<IDEMSK); 		// do not ignore standard frames
		//set to receive, DLC, IDE
		//set MOBs 5-14 to receive. Is this anything like ideal?
		if(i >= 5)
			CANCDMOB = (1 << CONMOB1) | (1 << IDE);
		else
			CANCDMOB = 0; //(1 << IDE);
	}
	
    //CAN_DEBUG("\nCAN_RX_BUFFER_SIZE=");    CAN_DEBUG(CAN_RX_BUFFER_SIZE);
    //CAN_DEBUG("  CAN_TX_BUFFER_SIZE=");    CAN_DEBUG(CAN_TX_BUFFER_SIZE);
	// activate CAN controller
	CANGCON = (1 << ENASTB);
	
		//DEG 26 May 2011
	//enable individual MOB interrupts!
	//For whatever reason, this does not take effect when the 
	//CAN bus is disabled!?
	CANIE1 = 0x7F;
	CANIE2 = 0xFF;

	return true;
}

// ----------------------------------------------------------------------------
// The CANPAGE register have to be restored after usage, otherwise it
// could cause trouble in the application programm.
void CanBus::_enter_standby_mode(void) {
	// request abort
	CANGCON = (1 << ABRQ);
	// wait until receiver is not busy
	while (CANGSTA & (1 << RXBSY));
	// request standby mode
	CANGCON = 0;
	// wait until the CAN Controller has entered standby mode
	while (CANGSTA & (1 << ENFG));
}

// leave standby mode => CAN Controller is connected to CAN Bus
void CanBus::_leave_standby_mode(void) {
	// save CANPAGE register
	uint8_t canpage = CANPAGE;
	// reenable all MObs
	for(uint8_t i = 0; i < 15; i++) {
		CANPAGE = i << 4;
		CANCDMOB = CANCDMOB;
	}
	// restore CANPAGE
	CANPAGE = canpage;
	// request normal mode
	CANGCON = (1 << ENASTB);
	// wait until the CAN Controller has left standby mode
	while ((CANGSTA & (1 << ENFG)) == 0);
}

#endif // AT90

