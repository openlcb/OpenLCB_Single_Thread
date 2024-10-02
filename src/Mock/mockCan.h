//
//  mockCan.h
//  
//
//  Created by David Harris on 2018-01-15.
//
//

#ifndef MOCKCAN_H
#define MOCKCAN_H

#if defined MOCK

#pragma message("Compiling mockCan.h")

#include "OlcbCan.h"

class OlcbCanClass : public OlcbCan {
  public:
    void init();                    // initialization
    uint8_t avail();                // read rxbuffer available
    uint8_t read();                 // read a buffer
    uint8_t txReady();              // write txbuffer available
    uint8_t write(long timeout);    // write, 0= immediately or fail; 0< if timeout occurs fail
    uint8_t write();                // write immediately or fail
};


#endif // MOCK
#endif // MOCKCAN_H
