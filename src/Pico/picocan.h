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

#include "OlcbCan.h"
#include "can2040.h"

class Can : public OlcbCan {
  public:
    void init();                    // initialization
    uint8_t avail();                // read rxbuffer available
    uint8_t read();                 // read a buffer
    uint8_t txReady();              // write txbuffer available
    uint8_t write(long timeout);    // write, 0= immediately or fail; 0< if timeout occurs fail
    uint8_t write();                // write(0), ie write immediately
    void setL(uint16_t l);
};

#endif // PICOCAN_H

#endif // TARGET_RP2040




