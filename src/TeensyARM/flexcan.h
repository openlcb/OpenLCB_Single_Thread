#ifndef FLEXCAN_H
#define FLEXCAN_H

//#pragma message("!!! compiling flexcan.h ")

#include "OlcbCan.h"
#include "FlexCANv1.h"

class OlcbCanClass : public OlcbCan {
public:
    void init();                    // initialization
    uint8_t avail();                // read rxbuffer available
    uint8_t read();                 // read a buffer
    uint8_t txReady();              // write txbuffer available
    uint8_t write(long timeout);    // write, 0= immediately or fail; 0< if timeout occurs fail
    uint8_t write();                // write(0), ie write immediately
    void setL(uint16_t l);
};




#endif // FLEXCAN_H
