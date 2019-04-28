//
//  OlcbCan.h
//  Interface for CAN libraries
//
//  Created by David Harris on 2018-01-22.
//
//

#ifndef OlcbCan_h
#define OlcbCan_h

//#pragma message("!!! compiling OlcbCan_h")

#include "OlcbNet.h"

class OlcbCan : public OlcbNet {
public:
    union {
        uint32_t id;            // CAN header
        uint8_t  idb[4];
        struct {                             // standard messages: 19xxxsss
            uint32_t dummy1       : 3;
            uint32_t CANMti       : 17;     // CAN MTI: 19xxx
            uint32_t source       : 12;     // 12-bit source: sss
        };
        struct {                             // special forms: DG and Streams
            uint32_t dummy2       : 3;
            uint32_t specialType  : 5;      // 1A-1D=DG, 1F=STream, else reserved
            uint32_t dst          : 12;     // destiantion
            uint32_t src          : 12;     // CAN MTI
        };
    };
    uint8_t length;
    uint8_t data[8];
    struct {
        int rtr                   : 1;  // Remote-Transmit-Request-Frame?
        int extended              : 1;  // extended ID?
    } flags;
    //uint16_t getLength();
};

#endif //OlcbCan_h