//
//  GCSerial.h
//  
//
//  Created by Dave Harris on 2024-09-13.
//

#ifndef GCSERIAL_H
#define GCSERIAL_H

#define NO_CAN  // disallow processor's CAN, may not work with ESP32 compiler

#pragma message "Direct to Serial selected (GCSerial)"
#define BTYPE "GCSerial"

#include "OlcbCan.h"
#include "debugging.h"
//#include <Arduino.h>
//#include "Energia.h"
//#include <Stream.h>
#include <HardwareSerial.h>

class Can : public OlcbCan {
  int charToHex(char c){
    if(c>='0' && c<='9') return c - '0';
    else if(c>='A' && c<='F') return c - 'A' + 10;
    else if(c>='a' && c<='f') return c - 'a' + 10;
    else return -1;
  }
  uint8_t toCan(uint32_t &id, bool &ext, bool &rtr, uint8_t &len, uint8_t* data, char* buff)  {
    char* p=buff;
    int x;
    id = 0;
    if(*p++ != ':') { return 1; }
    if(*p == 'S') { ext=false; }
    else if(*p == 'X') { ext=true; }
    else return 2;
    p++;
    do {
        int x = charToHex(*p);
        if( x>=0 ) id = 16*id + x;
        else break;
        p++;
    } while(1);
    if(ext==false) id = id>>5;   // GC specific
    //dP("\n  id="); dP2(id, HEX);
    if(*p == 'N') rtr=false;
    else if(*p=='R') rtr=true;
    else { return 3; }// no N or R
    //dPS("\nrtr=", rtr);
    p++;
    for(uint8_t i=0;i<8;i++) {
        if( ( x = charToHex(*p) ) <0 ) break;
        data[i] = x;
        p++;
        if( ( x = charToHex(*p) ) <0 ) return 2;
        data[i] = data[i] * 16 + x;
        p++;
        len = i+1;
    }
    return 0;
  }
  
  uint8_t toGC(uint32_t id, bool ext, bool rtr, uint8_t len, uint8_t* data, char* buff)  {
      char* p = buff;
      if(!ext) p += sprintf(p, ":X%04X", id<<5);  // shift if GC specific for CBUS
      //else p += sprintf(p, ":X%lX", id); // no shift for extended frames
      else p += sprintf(p, ":X%lX", id); // no shift for extended frames
      p += sprintf(p, "%c", (rtr ? 'R' : 'N') );
      for(uint8_t i=0;i<len;i++) {
          p += sprintf(p, "%02X", data[i]);
      }
      p += sprintf(p, "%c\0", ';');
      return 1;
  }
    uint8_t state = 0;
    uint8_t p = 0;
    HardwareSerial* inf;
  public:
    //Can() : inf(&Serial) {}
    Can(HardwareSerial* _inf=&Serial) : inf(_inf) {}
    void init() {                   // initialization
        //inf->begin(115200);while(!inf); delay(1000);
        Serial.begin(115200);while(!Serial); delay(1000);
    }
    void setInf(HardwareSerial* _inf) {
        inf = _inf;
        init();
        delay(200);
    }
    uint8_t avail() {                // read rxbuffer available
      return true;
    }
    uint8_t read() {                 // read a buffer
        int c;
        char buff[40];
        bool ext, rtr;
        if(state==0) {
            //while( (c=inf->read()) >0 ) {
            while( (c=Serial.read()) >0 ) {
                if(c==':') { state = 1; break; }
            }
            if(state!=1) return false;
            buff[0] =':';
            p=1;
        }
        //uint8_t n = inf->readBytesUntil(';', buff+1, 30);
        uint8_t n = Serial.readBytesUntil(';', buff+1, 30);
        buff[n+1] = ';';
        buff[n+2] = '\0';
        toCan(id, ext, rtr, length, data, buff);
        state = 0;
        dP("\n<<<"); dP(buff);
        return true;
    }
    uint8_t txReady() {              // write txbuffer available
        return true;
    }
    uint8_t write(long timeout) {    // write, 0= immediately or fail; 0< if timeout occurs fail
        if(length>8) return false;
        char buff[40];
        toGC(id, true, false, length, data, buff);
        dP("\n>>>");
        //inf->print(buff);
        //inf->flush();  // make sure this is complete
        Serial.print(buff);
        Serial.flush();  // make sure this is complete
        return true;
    }
    uint8_t write() {                // write(0), ie write immediately
        write(200);
        return true;
    }
    void setL(uint16_t l);
};

#endif // GCSERIAL_H

