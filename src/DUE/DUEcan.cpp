
// Adapting to support DUE
// Skeleton at the moment.

// Note: Can has to be OlcbCanClass to avoid the definition of Can in the DUE code.
#if 0
#if defined __SAM3X8E__

//#pragma message("!!! compiling ESPcan.cpp ")


#include "Arduino.h"

#include "OlcbCan.h"
#include "DUEcan.h"
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
void OlcbCanClass::init() {
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

uint8_t OlcbCanClass::avail() {
    // The number of available frames.
    return _can->available();
}

uint8_t OlcbCanClass::read() {
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

uint8_t OlcbCanClass::txReady() {
    // I cannot find the equivalent in due_can.
    bool b = true;
    return b;
}

uint8_t OlcbCanClass::write(long timeout) {
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
uint8_t OlcbCanClass::write() { return this->write(0); }

//
/// set the CAN controller peripheral instance, there are two, default is zero
//

void OlcbCanClass::setControllerInstance(byte instance) {
    
    // Serial << "> setting CAN controller instance to " << instance << endl;
    _instance = instance;
    _can = (_instance == 0) ? &Can0 : &Can1;
}

void Can::setL(uint16_t l) { length = l; }


#endif // __SAM3X8E__
#endif // 0
