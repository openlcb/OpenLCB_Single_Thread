#ifndef ESPCAN_H
#define ESPCAN_H

//#pragma message("!!! In ESPcan.h ")

#ifndef NOCAN
#pragma message("!!! compiling ESPcan.h ")

#include "OlcbCan.h"
#include "ESP32can.h"

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

// From ESPcan.cpp

class ESP32can;
ESP32can esp32can;

void OlcbCanClass::init() {
    //Serial.print("\nIn ESP EspOlcbCanClass::init");
    if (!esp32can.begin(125E3)) {
        Serial.println("Starting EspOlcbCanClass:: failed!");
        while (1);
    }
    return;
}

uint8_t OlcbCanClass::avail() {
    uint8_t r = esp32can.avail();
                //Serial.print("\n EspOlcbCanClass::avail()="); Serial.print(r);
    return r;
    //return esp32can.avail();
}

uint8_t OlcbCanClass::read() {
    //Serial.print("\nIn ESPcan::read()");
    if(!esp32can.avail()) return 0;
    bool extended;
    esp32can.read(id, length, data, extended);
    if(flags.rtr) return 0;
    //if(m.err!=0) return 0;
    //this->id = m.id;
    flags.extended = extended;
    //this->flags.rtr = m.rtr;
    //this->length = m.dlc;
    //for(int i=0;i<m.dlc;i++) this->data[i] = m.buf[i];
    return 1;
}

uint8_t OlcbCanClass::txReady() {
            //Serial.print("\n ESPcan::txReady()");
    bool b = esp32can.txReady();
            //Serial.print("\n esp EspOlcbCanClass::txReady()=");
            //Serial.print(b );
    return b;
}

uint8_t OlcbCanClass::write(long timeout) {
    //CAN_message_t m;
            //Serial.print("\n ESP OlcbCanClass::write()");
    //m.id = this->id;
    //m.dlc = this->length;
    //m.eff = 1;
    //m.rtr=0;
    //memcpy(m.buf,this->data,m.dlc);
            //Serial.print(id, HEX);
            //Serial.print("]("); Serial.print(length);
            //Serial.print(") ");
            //for(int i=0;i<length;i++)
            //    { Serial.print(data[i],HEX); Serial.print(" "); }
    if(timeout==0 && this->txReady()) {
        active = true;
        return esp32can.write(id, length, data);
    }
            //Serial.print("\n     EspOlcbCanClass::write()#C");
    long to = millis() + timeout;
    while(millis()<to) {
        if(this->txReady()) {
            active = true;
            return esp32can.write(id, length, data);
        }
    }
    return false;
}
uint8_t OlcbCanClass::write() { return this->write(0); }

#endif // NOCAN


#endif // ESPCAN_H
