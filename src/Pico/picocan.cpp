
// OpenLCB Adaptation of can2040 library
// copyright DPH 2022

// Using 'native' CAN library:
//     Software CANbus implementation for rp2040
//     Copyright (C) 2022  Kevin O'Connor <kevin@koconnor.net>
//     This file may be distributed under the terms of the GNU GPLv3 license.

#if defined(ARDUINO_ARCH_RP2040)
#include "processor.h"
#ifndef PICONOCAN
#pragma message("!!! compiling picocan.cpp ")

#include "Arduino.h"
//#include "processor.h"

//#include "OlcbCan.h"
#include "can2040.h"
#include "picocan.h"

#include <ArduinoQueue.h>
struct can2040 rcan;                            // native-CAN object
const uint8_t maxmsg = 4;                       // number of message buffers
ArduinoQueue<struct can2040_msg> rmsg(maxmsg);  // FIFO

Can msg;    // buffer
bool rflag;
int8_t errors;                     // number of errors  /////NEED TO FIGURE OUT ERRORS

// callback for a receive
void can2040_callback(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg) {
  //rmsg.enqueue(*msg);
    if( CAN2040_NOTIFY_ERROR == notify ) {
      //errors++;
      //errormsg(errors);
      return;
    } else if( CAN2040_NOTIFY_RX == notify ) {
      if(rmsg.isFull()) {
        //errors++;
        //Serial.print("\n Buffer overflow!");
      } else {
        rmsg.enqueue(*msg);   // queue, process later
      }
    }
  return;
}

void PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&rcan);
}

void Can::init() {
    //Serial.print("\n picocan::init");
    uint32_t pio_num = 0;
    uint32_t sys_clock = 125000000, bitrate = 125000;
    uint32_t gpio_rx = 4, gpio_tx = 5;
    //uint32_t gpio_rx = 27, gpio_tx = 28;
    // Setup canbus
    can2040_setup(&rcan, pio_num);
    can2040_callback_config(&rcan, can2040_callback);
    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0_IRQn, PIOx_IRQHandler);
    NVIC_SetPriority(PIO0_IRQ_0_IRQn, 1);
    NVIC_EnableIRQ(PIO0_IRQ_0_IRQn);
    // Start canbus
    can2040_start(&rcan, sys_clock, bitrate, gpio_rx, gpio_tx);

    //static struct can2040_msg msg0 = {(uint32_t)0x195b4555 | CAN2040_ID_EFF, 8, 1,2,3,4,5,6,7,8};
    //while(1) {
    //    can2040_transmit(&rcan, &msg0);
    //    delay(1000);
    //}
    return;
}

uint8_t Can::avail() {
    if(rmsg.isEmpty()) return 0;
    return 1;
}

uint8_t Can::read() {
    //Serial.print("\nIn picocan::read()");
    struct can2040_msg msg;
    if(rmsg.isEmpty()) return 0;
    noInterrupts();
    msg = rmsg.dequeue();               // protect the dequeue
    interrupts();
    if(msg.id & CAN2040_ID_RTR) {
      //lcb.write( 0, 0,0,0,0,0,0,0,0); // send our response
      return 0;
    }
    this->id = msg.id;
    this->flags.rtr = msg.id & CAN2040_ID_RTR;
    this->flags.extended = msg.id & CAN2040_ID_EFF;
    this->length = msg.dlc;
    memcpy( this->data, msg.data, length);
    return 1;
}

uint8_t Can::txReady() {
    //Serial.print("\n picocan::txReady()");
    return (uint8_t) can2040_check_transmit(&rcan);
}

uint8_t Can::write(long timeout) {
    //Serial.print("\n picocan::write(long timeout)");
    struct can2040_msg rmsg;
    rmsg.id = id;
    if( flags.rtr ) rmsg.id |= CAN2040_ID_RTR;
    if( flags.extended ) rmsg.id |= CAN2040_ID_EFF;
    rmsg.dlc = length;
    memcpy( rmsg.data, data, length);
    //Serial.print("\nCan::write()");
    can2040_transmit(&rcan, &rmsg);
    return 1;
}

uint8_t Can::write() {
    Serial.print("\n picocan::write()");
    return this->write(0);
}
#endif // PICONOCAN
#endif // TARGET_RP2040

