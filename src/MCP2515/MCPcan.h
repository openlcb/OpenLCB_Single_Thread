#ifndef MCPCAN_H
#define MCPCAN_H

//#pragma message("!!! compiling MCPcan.h ")

#include "../OlcbCommonCAN/OlcbCan.h"

#define SUPPORT_MCP2515 1

#include "MCP2515/MCP2515can.h"

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




#endif // MCPCAN_H
