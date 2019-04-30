#ifndef TIVACAN_H
#define TIVACAN_H

//#pragma message("!!! compiling tivacan.h ")

#include "OlcbCan.h"
#include "TivaCANv0.h"

class Can : public OlcbCan {
  public:
    //class CANClass;
    //CANClass canbus(0);
    void init();                    // initialization
    uint8_t avail();                // read rxbuffer available
    uint8_t read();                 // read a buffer
    uint8_t txReady();              // write txbuffer available
    uint8_t write(long timeout);    // write, 0= immediately or fail; 0< if timeout occurs fail
    uint8_t write();                // write(0), ie write immediately
    void setL(uint16_t l);
};

#endif // TIVACAN_H



