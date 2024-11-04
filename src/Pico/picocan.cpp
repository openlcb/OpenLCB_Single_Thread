#if 0
// OpenLCB Adaptation of can2040 library
// copyright DPH 2022

// Using 'native' CAN library:
//     Software CANbus implementation for rp2040
//     Copyright (C) 2022  Kevin O'Connor <kevin@koconnor.net>
//     This file may be distributed under the terms of the GNU GPLv3 license.

#if defined(ARDUINO_ARCH_RP2040)
#include "processCAN.h"
#ifndef NOCAN

#include "Arduino.h"
#include "debugging.h"
//#include "can2040.h"
#include "picocan.h"

#include <ArduinoQueue.h>
#pragma message("!!! compiling picocan.cpp ")

struct can2040* pcan;

const uint8_t maxmsg = 4;                       // number of message buffers
ArduinoQueue<struct can2040_msg> rmsg(maxmsg);  // FIFO

OlcbCanClass msg;    // buffer
bool rflag;
int8_t errors;                     // number of errors  /////NEED TO FIGURE OUT ERRORS

// callback for a receive
void can2040_callback(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg) {
  //rmsg.enqueue(*msg);
    if( CAN2040_NOTIFY_ERROR == notify ) {
      //errors++;
      //errormsg(errors);
        dP("\nCallback error");
      return;
    } else if( CAN2040_NOTIFY_RX == notify ) {
      if(rmsg.isFull()) {
          dP("\nRead buffer is full");
        //errors++;
        //Serial.print("\n Buffer overflow!");
      } else {
        rmsg.enqueue(*msg);   // queue, process later
      }
    }
  return;
}

static void PIOx_IRQHandler(void) {
    can2040_pio_irq_handler(pcan);
}

void OlcbCanClass::init() {
    //Serial.print("\n picocan::init");
    pcan = new struct can2040;
    uint32_t pio_num = 0;
    uint32_t sys_clock = 125000000UL;
    uint32_t bitrate = 125000UL;
    gpio_rx = 27; // rxpin
    gpio_tx = 28; // txpin
    // Setup canbus
    can2040_setup(pcan, pio_num);
    can2040_callback_config(pcan, can2040_callback);
    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0_IRQn, PIOx_IRQHandler);
    //irq_set_exclusive_handler(PIO0_IRQ_0, PIOx_IRQHandler);
    NVIC_SetPriority(PIO0_IRQ_0_IRQn, 1);
    //NVIC_SetPriority(PIO0_IRQ_0, 1);
    NVIC_EnableIRQ(PIO0_IRQ_0_IRQn);
    //NVIC_EnableIRQ(PIO0_IRQ_0);
    // Start canbus
    can2040_start(pcan, sys_clock, bitrate, gpio_rx, gpio_tx);

    #if 0
        static struct can2040_msg msg0 = {(uint32_t)0x195b4555 | CAN2040_ID_EFF, 8, 1,2,3,4,5,6,7,8};
        while(1) {
            can2040_transmit(&rcan, &msg0);
            delay(1000);
        }
    #endif
    return;
}

// Note -- no way to apply this.
void OlcbCanClass::setpins(uint32_t rx, uint32_t tx) {
    gpio_rx = rx;
    gpio_tx = tx;
}

uint8_t OlcbCanClass::avail() {
    if(rmsg.isEmpty()) return 0;
    return 1;
}

uint8_t OlcbCanClass::read() {
    struct can2040_msg msg;
    if(rmsg.isEmpty()) return 0;
    Serial.print("\nIn picocan::read()");
    noInterrupts();
    msg = rmsg.dequeue();               // protect the dequeue
    interrupts();
    if(msg.id & CAN2040_ID_RTR) {
      //lcb.write( 0, 0,0,0,0,0,0,0,0); // send our response
        dP("\nRTR?!");
      return 0;
    }
    this->id = msg.id & 0x1FFFFFFF;
    this->flags.rtr = msg.id & CAN2040_ID_RTR;
    this->flags.extended = msg.id & CAN2040_ID_EFF;
    this->length = msg.dlc;
    memcpy( this->data, msg.data, length);
    dP("\n["); dPH((uint32_t)id); dP("]");
    for(int i=0;i<length;i++) {
        if(data[i]<16); dP((uint8_t)0);
        dPH((uint8_t)data[i]);
    }
    return 1;
}

uint8_t OlcbCanClass::txReady() {
    //Serial.print("\n picocan::txReady()");
    return (uint8_t) can2040_check_transmit(pcan);
}

uint8_t OlcbCanClass::write(long timeout) {
    Serial.print("\n picocan::write(long timeout)");
    dP("\n["); dPH((uint32_t)id);
    dP("]");
    for(int i=0; i<length; i++) {
        if(data[i]<16) dP((uint8_t)0);
        dP((uint8_t)data[i]);
    }
    struct can2040_msg txmsg;
    txmsg.id = id;
    if( flags.rtr ) txmsg.id |= CAN2040_ID_RTR;
    if( flags.extended ) txmsg.id |= CAN2040_ID_EFF;
    dP(" txmsg.id="); dPH((uint32_t)txmsg.id);
    txmsg.dlc = length;
    memcpy( txmsg.data, data, length);
    //Serial.print("\OlcbCanClass::write()");
    can2040_transmit(pcan, &txmsg);
    return 1;
}

uint8_t OlcbCanClass::write() {
    Serial.print("\n picocan::write()");
    return this->write(0);
}
#endif // NOCAN
#endif // TARGET_RP2040

#endif // 0/1
