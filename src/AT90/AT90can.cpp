// OpenLCB Adaptation of FlexCAN library
// copyright DPH 2017

#include "Arduino.h"

#include "OlcbCan.h"
#include "AT90can.h"
#include "CanBus.h"

class CanBus;
CanBus* canbus;

/*
#define	BITRATE_10_KBPS	0	// ungetestet
#define	BITRATE_20_KBPS	1	// ungetestet
#define	BITRATE_50_KBPS	2	// ungetestet
#define	BITRATE_100_KBPS	3	// ungetestet
#define	BITRATE_125_KBPS	4
#define	BITRATE_250_KBPS	5
#define	BITRATE_500_KBPS	6
#define	BITRATE_1_MBPS	7
*/

void Can::init()  {
            Serial.print("\nIn AT90Can::init()");
    canbus->init();
    return true;
}
uint8_t Can::avail()  {
    return canbus->check_message();
}
uint8_t Can::read()  {
    can_t m;
    if(avail()) {
        canbus->get_buffered_message(&m);
                    //Serial.print("\nIn AT90Can::read(): [");
        this->id = m.id;
        this->length = m.length;
        memcpy(this->data, m.data, length);
                    //Serial.print(this->id, HEX);
                    //Serial.print("]("); Serial.print(this->length);
                    //Serial.print(") ");
                    //for(int i=0;i<this->length;i++)
                    // { Serial.print(this->data[i],HEX); Serial.print(" "); }
        return true;
    }
    return false;
}
uint8_t Can::txReady()  {
                    //Serial.print("\nIn AT90Can::txReady(): ");
                    //Serial.print("check_free_buffer="); Serial.print(((CanBus*)this)->check_free_buffer());
    return ((CanBus*)this)->check_free_buffer();
}
uint8_t Can::write(long timeout)  {
                    //Serial.print("\nIn AT90Can::write(): [");
    can_t m;
    m.id = this->id;
    m.length = this->length;
    m.flags.extended = 1;
    m.flags.rtr=0;
    memcpy(m.data,this->data,length);
                    //Serial.print(m.id, HEX);
                    //Serial.print("]("); Serial.print(m.length);
                    //Serial.print(") ");
                    //for(int i=0;i<m.length;i++)
                    //    { Serial.print(m.data[i],HEX); Serial.print(" "); }
    if(timeout==0 && this->txReady()) {
        return canbus->send_buffered_message(&m);
        active = true;
    }
    long to = millis() + timeout;
    while(millis()<to) {
        if(this->txReady()) {
            active = true;
            return canbus->send_buffered_message(&m);
        }
    }
    return false;
}
uint8_t Can::write() { return this->write(0); }
void Can::setL(uint16_t l) { length = l; }

