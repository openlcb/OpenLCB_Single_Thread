#ifndef MCPCAN_H
#define MCPCAN_H

#ifndef NO_CAN

//#if SUPPORT_MCP2525 == 1

#pragma message("!!! compiling MCPcan.h ")

#include "OlcbCan.h"

#include "MCP2515/MCP2515can.h"

class McpCan : public OlcbCan {
  public:
    McpCan(){}
    void init();                    // initialization
    uint8_t avail();                // read rxbuffer available
    uint8_t read();                 // read a buffer
    uint8_t txReady();              // write txbuffer available
    uint8_t write(long timeout);    // write, 0= immediately or fail; 0< if timeout occurs fail
    uint8_t write();                // write(0), ie write immediately
    void setL(uint16_t l);
};


#endif // SUPPORT_MCP2515

#endif // MCPCAN_H
