
#ifdef __SAM3X8E__


#ifndef DUECAN_H
#define DUECAN_H


#pragma message("!!! compiling DUEcan.h ")
#include <SPI.h>

#include "OlcbCan.h"
#include "DUEcanImpl.h"
#include <due_can.h>          // Due CAN library header file

// These are here for information. These are the macros defined by the Arduino DUE system.
// They are pointers to the two instances of the Can type used to define the Can0 and Can1 for the user.
//CAN0 #define CAN0       ((Can    *)0x400B4000U) /**< \brief (CAN0      ) Base Address */
//CAN1 #define CAN1       ((Can    *)0x400B8000U) /**< \brief (CAN1      ) Base Address */

//#include "DUE32can.h" does not yet exist and may not be needed.

// https://forum/arduino.cc/t/due-software-reset/332764/9 
void due_restart();
//{
//	RSTC->RSTC_CR = 0xA5000005; // Reset processor and internal peripherals.
//}

class CanX : public OlcbCan {
public:
    void init();                    // initialization
    uint8_t avail();                // read rxbuffer available
    uint8_t read();                 // read a buffer
    uint8_t txReady();              // write txbuffer available
    uint8_t write(long timeout);    // write, 0= immediately or fail; 0< if timeout occurs fail
    uint8_t write();                // write(0), ie write immediately
    void setL(uint16_t l);

    // This method is specific to this implementation
    // It is not declared or implemented by the base OlcbCan class
	// The function is to choose between CAN0 and CAN1 ports.
	// The default action is CAN0.
    void setControllerInstance(byte instance = 0);

private:
    byte _instance;
    CANRaw *_can;

};


#endif // DUECAN_H

#endif //__SAM3X8E__
