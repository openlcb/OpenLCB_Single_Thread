// OpenLCB Adaptation of FlexCAN library
// copyright DPH 2017

#include "Arduino.h"

#include "OlcbCan.h"
#include "FlexCANv1.h"
#include "flexcan.h"

FlexCANv1 canbus(125000);

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
                //Serial.print("\nIn flexcan::init()");
    canbus.begin();
                //Serial.print("\nOut flexcan::init()");
    //return true;
}
uint8_t Can::avail()  {
                    //Serial.print("\nIn flexcan::avail()");
    return canbus.available();
}
uint8_t Can::read()  {
    CAN_message_t m;
                    //Serial.print("\nIn flexcan::read(): ");
    if(avail()) {
        canbus.read(m);
                        //Serial.print("\n r [");
                        //Serial.print(m.id, HEX);
                        //Serial.print("]("); Serial.print(m.len);
                        //Serial.print(") ");
                        //for(int i=0;i<m.len;i++)
                        //    { Serial.print(m.buf[i],HEX); Serial.print(" "); }
        this->id = m.id;
        this->length = m.len;
        memcpy(this->data, m.buf, length);
        return true;
    }
    return false;
}
uint8_t Can::txReady()  {
                //Serial.print("\nIn flexcan::txReady(): ");
    //Serial.print("check_free_buffer="); Serial.print(((CanBus*)this)->check_free_buffer());
    //return ((CanBus*)this)->check_free_buffer();
    return true; // KLUDGE
}
uint8_t Can::write(long timeout)  {
                    //Serial.print("\nIn flexcan::write(): [");
    CAN_message_t m;
    m.timeout = timeout;
    m.id = this->id;
    m.len = this->length;
    m.ext = 1;
    //m.flags.rtr=0;
    memcpy(m.buf,this->data,length);
                    //Serial.print(m.id, HEX);
                    //Serial.print("]("); Serial.print(m.len);
                    //Serial.print(") ");
                    //for(int i=0;i<m.len;i++)
                    //    { Serial.print(m.buf[i],HEX); Serial.print(" "); }
    return canbus.write(m);

    /*
    if(timeout==0 && this->txReady()) {
        //return canbus->send_buffered_message(&m);
        return can_send_message(&m);
        active = true;
    }
    long to = millis() + timeout;
    while(millis()<to) {
        if(this->txReady()) {
            active = true;
            //return canbus->send_buffered_message(&m);
            return can_send_message(&m);
        }
    }
    return false;
    */
}
uint8_t Can::write() { return this->write(0); }
void Can::setL(uint16_t l) { length = l; }

