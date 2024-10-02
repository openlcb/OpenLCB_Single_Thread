
#ifdef __SAM3X8E__

#ifndef DUECAN_H
#define DUECAN_H

#ifndef NOCAN

#pragma message("!!! compiling DUEcan.h ")

#include <SPI.h>

#include "OlcbCan.h"
//#include "DUEcanImpl.h"
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

//class OlcbCanClass : public OlcbCan {
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


//////////////////////////////////////

// Adapting to support DUE
// Skeleton at the moment.

// Note: Can has to be CanX to avoid the definition of Can in the DUE code.

//#if defined __SAM3X8E__

//#pragma message("!!! compiling ESPcan.cpp ")


//#include "Arduino.h"

//#include "OlcbCan.h"
//#include "DUEcan.h"
//#include <due_can.h>          // Due CAN library header file
//#include "DUEcanImpl.h"
// This needs to have the member functions populated.

// https://forum/arduino.cc/t/due-software-reset/332764/9
void due_restart() {
    RSTC->RSTC_CR = 0xA5000005; // Reset processor and internal peripherals.
}


// ----------------------------------------------------------------------------
/**
 * \ingroup  can_interface
 * \brief Initialisierung des CAN Interfaces
 *
 * \param bitrate Gewuenschte Geschwindigkeit des CAN Interfaces
 *
 * \return  false falls das CAN Interface nicht initialisiert werden konnte,
 *      true ansonsten.
 */
void CanX::init() {
    // CBUS Arguments supplied as variables
    //Serial.print("\nIn DUE Can::init\n");
    //Serial.flush();
    // I don't think these do anything as they are not references in the CBUS code.
    //bool poll = false;
    //SPIClass spi = SPI;
    // The CBUS version monitors the next two and returns false if there is a failure.
    // That option is not available here.
    uint32_t init_ret;
    int init_watch;
    // init CAN instance
    init_ret = _can->begin(CAN_BPS_125K, 255);
    if (!init_ret) {
        Serial.print("\nCan error from init()"); //ret = " << init_ret << endl;
        Serial.flush();
        return; // false;
    }
    //Serial.print("init_ret "); Serial.print(init_ret);
    //Serial.flush();
    // set filter to permissive
    init_watch = _can->watchFor();
    if (init_watch == -1) {
        Serial.print("\nCan error from watchFor()"); // ret = " << init_watch << endl;
        Serial.flush();
        return; // false;
    }
    //Serial.print("\ninit_watch "); Serial.print(init_watch);
    //Serial.print("\nExiting DUE Can::init");
    //Serial.flush();
    return;
}

uint8_t CanX::avail() {
    // The number of available frames.
    return _can->available();
}

uint8_t CanX::read() {
    //Serial.print("\nIn DUE Can::read()");
    uint32_t ret;
    CAN_FRAME cf;
    if (!_can->available()) return 0;
    
    ret = _can->read(cf);
    
    if (ret != CAN_MAILBOX_TRANSFER_OK) {
        // Serial << "> CAN read did not return CAN_MAILBOX_TRANSFER_OK, instance = " << _instance << ", ret = " << ret << endl;
        return 0;
    } else {
        this->id = cf.id;
        this->flags.extended = cf.extended;
        this->flags.rtr = cf.rtr;
        this->length = cf.length;
        memcpy(this->data, cf.data.byte, cf.length);
        
        return 1;
    }
    return 0;
}

uint8_t CanX::txReady() {
    // I cannot find the equivalent in due_can.
    bool b = true;
    return b;
}

uint8_t CanX::write(long timeout) {
    bool ret;
    CAN_FRAME cf;                         // library-specific CAN message structure
    //Serial.print("\n DUE Can::write()");
    cf.id = this->id;
    cf.length = this->length;
    cf.rtr = this->flags.rtr;
    cf.extended = this->flags.extended;
    
    memcpy(cf.data.bytes, this->data, this->length);
    //Serial.print(id, HEX);
    //Serial.print("]("); Serial.print(length);
    //Serial.print(") ");
    //for(int i=0;i<length;i++)
    //    { Serial.print(data[i],HEX); Serial.print(" "); }
    ret = _can->sendFrame(cf);
    /*
     if(timeout==0 && this->txReady()) {
     active = true;
     return esp32can.write(id, length, data);
     }
     //Serial.print("\n     TivaCan::write()#C");
     long to = millis() + timeout;
     while(millis()<to) {
     if(this->txReady()) {
     active = true;
     return esp32can.write(id, length, data);
     }
     } */
    return ret;
}
uint8_t CanX::write() { return this->write(0); }

//
/// set the CAN controller peripheral instance, there are two, default is zero
//

void CanX::setControllerInstance(byte instance) {
    
    // Serial << "> setting CAN controller instance to " << instance << endl;
    _instance = instance;
    _can = (_instance == 0) ? &Can0 : &Can1;
}

void CanX::setL(uint16_t l) { length = l; }

//#endif // __SAM3X8E__

#endif // NOCAN

#endif // DUECAN_H

#endif //__SAM3X8E__
