//
//  OlcbNet.h
//  Interface for CAN libraries
//
//  Created by David Harris on 2018-01-22.
//
//

#ifndef OlcbNet_h
#define OlcbNet_h

//#pragma message("Compiling OlcbNet")

#include <Arduino.h>

class OlcbNet {
  public:
    virtual void init() = 0;                    // initialization
    virtual uint8_t avail() = 0;                // read rxbuffer available
    virtual uint8_t read() = 0;                 // read a buffer
    virtual uint8_t txReady() = 0;              // write txbuffer available
    virtual uint8_t write(long timeout) = 0;    // write, 0= immediately or fail = 0; 0< if timeout occurs fail
    virtual uint8_t write() {               // write(0), ie write immediately
        return write((long) 0);
    };
    bool active;                          // flag net activity
};

#endif // OlcbNet_h
