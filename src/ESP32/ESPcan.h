#ifndef ESPCAN_H
#define ESPCAN_H

//#pragma message("!!! compiling ESPcan.h ")

#include "OlcbCan.h"
#include "ESP32can.h"

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

#endif // ESPCAN_H
