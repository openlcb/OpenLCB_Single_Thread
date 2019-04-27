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
    virtual void init();                    // initialization
    virtual uint8_t avail();                // read rxbuffer available
    virtual uint8_t read();                 // read a buffer
    virtual uint8_t txReady();              // write txbuffer available
    virtual uint8_t write(long timeout);    // write, 0= immediately or fail; 0< if timeout occurs fail
    virtual uint8_t write() {               // write(0), ie write immediately
        return write((long) 0);
    };
    bool active;                          // flag net activity
};

#endif // OlcbNet_h
