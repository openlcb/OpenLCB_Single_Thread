/*


// OpenLCB Adaptation of FlexCAN library
// copyright DPH 2017

#include "Arduino.h"

#include "OlcbCan.h"
#include "ESPcan.h"
#include "ESP32can.h"

class ESP32can;
ESP32can esp32can;

// ----------------------------------------------------------------------------

void Can::init() {
    //Serial.print("\nIn tivacan can_init");
    esp32can.begin(125000);
    return;
}

uint8_t Can::avail() {
    //return 0!=tivaCAN.available();
    return esp32can.avail();
}

uint8_t Can::read() {
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

uint8_t Can::txReady() {
            //Serial.print("\n ESP32can::txReady()#A");
    bool b = esp32can.txReady();
            //Serial.print("\n     ESP32can->tx_idle()=");
            //Serial.print(b );
    return b;
}

uint8_t Can::write(long timeout) {
    //CAN_message_t m;
            //Serial.print("\n     TivaCan::write()#A");
    //m.id = this->id;
    //m.dlc = this->length;
    //m.eff = 1;
    //m.rtr=0;
    //memcpy(m.buf,this->data,m.dlc);
            //Serial.print(m.id, HEX);
            //Serial.print("]("); Serial.print(m.length);
            //Serial.print(") ");
            //for(int i=0;i<m.length;i++)
            //    { Serial.print(m.data[i],HEX); Serial.print(" "); }
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
    }
    return false;
}
uint8_t Can::write() { return this->write(0); }
*/
