// OpenLCB Adaptation of FlexCAN library
// copyright DPH 2017

#include "Arduino.h"

#include "OlcbCan.h"
#include "MCP2515can.h"
#include "MCPcan.h"

//class CanBus;
//CanBus* canbus;
tCAN* canbus;

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
                    //Serial.print("\nIn AT90Can::init()");
    //canbus->init();
    if(!can_init()) Serial.print("\n SPI ERROR!!!");
    return true;
}
uint8_t Can::avail()  {
    bool r = can_check_message();
                    //if(r) Serial.print("\nMCPavail!");
    return r;
    //return can_check_message();
}
uint8_t Can::read()  {
    tCAN m;
    if(avail()) {
                    //Serial.print("\nIn MCPCan::read(): ");
        //canbus->get_buffered_message(&m);
        can_get_message(&m);
                    //Serial.print("[");
        this->id = m.id;
        this->length = m.length;
        memcpy(this->data, m.data, length);
                    //Serial.print(this->id, HEX);
                    //Serial.print("]("); Serial.print(this->length);
                    //Serial.print(") ");
                    //for(int i=0;i<this->length;i++)
                    // {Serial.print(this->data[i],HEX); Serial.print(" "); }
        return true;
    }
    return false;
}
uint8_t Can::txReady()  {
                    //Serial.print("\nIn AT90Can::txReady(): ");
                    //Serial.print("check_free_buffer="); Serial.print(((CanBus*)this)->check_free_buffer());
    //return ((CanBus*)this)->check_free_buffer();
    return can_check_free_buffer();
}
uint8_t Can::write(long timeout)  {
                    //Serial.print("\nIn MCPCan::write(): [");
    tCAN m;
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
}
uint8_t Can::write() { return this->write(0); }
void Can::setL(uint16_t l) { length = l; }

