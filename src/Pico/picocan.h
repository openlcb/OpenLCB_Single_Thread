// OpenLCB Adaptation of can2040 library
// copyright DPH 2022

// Using 'native' CAN library:
//     Software CANbus implementation for rp2040
//     Copyright (C) 2022  Kevin O'Connor <kevin@koconnor.net>
//     This file may be distributed under the terms of the GNU GPLv3 license.


#if defined(ARDUINO_ARCH_RP2040)
#pragma message("!!! compiling picocan.h ")

#ifndef PICOCAN_H
#define PICOCAN_H
// Pico SDK headers - supplied with arduino-pico core
#include "RP2040.h" // hw_set_bits
#include "hardware/regs/dreq.h" // DREQ_PIO0_RX1
#include "hardware/structs/dma.h" // dma_hw
#include "hardware/structs/iobank0.h" // iobank0_hw
#include "hardware/structs/padsbank0.h" // padsbank0_hw
#include "hardware/structs/pio.h" // pio0_hw
#include "hardware/structs/resets.h" // RESETS_RESET_PIO0_BITS
// additional SDK header required for successful compilation
#include <hardware/irq.h>
#include "OlcbNet.h"
#include "OlcbCan.h"

// can2040 header
extern "C" {
  #include "can2040.h"
}

class OlcbCanClass : public OlcbCan {
  public:
    void init();                    // initialization
    uint8_t avail();                // read rxbuffer available
    uint8_t read();                 // read a buffer
    uint8_t txReady();              // write txbuffer available
    uint8_t write(long timeout);    // write, 0= immediately or fail; 0< if timeout occurs fail
    uint8_t write();                // write(0), ie write immediately
    void setL(uint16_t l);
    void setpins(uint32_t rx, uint32_t tx); // set pins
private:
    uint32_t gpio_rx = 4;
    uint32_t gpio_tx = 5;
    //struct can2040 _pbus;
    void (*_callback)(struct can2040 * cd, uint32_t notify, struct can2040_msg * msg);
};

#endif // PICOCAN_H

#endif // TARGET_RP2040




