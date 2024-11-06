//
//  GCSerial.h
//  
//
//  Created by Dave Harris on 2024-09-13.
//

/*
2.7.2 Normal CAN Message in GRIDCONNECT format
  A normal CAN message consists of the type (11-bit or 29-bit), identifier, length, and data bytes and is
  encoded as follows:
  Normal CAN Message Syntax
     : <S | X> <IDENTIFIER> <N> <DATA-0> <DATA-1> ... <DATA-7> ;
  - The first character,‘:’, is for synchronization and allows the CAN USB-232 parser to detect the beginning of
    a command string.
  - The following character is either ‘S’ for standard 11-bit, or ‘X’ for extended 29-bit identifier type.
  - The ‘IDENTIFIER’ field consists of from one to eight hexadecimal digits, indicating the value of the
    identifier. Note that if a 29-bit value is entered and an 11-bit value was specified, the command will be
    treated as invalid and ignored.
  - The character ‘N’ indicates that the message is a normal (non-RTR) transmission.
  - Each ‘DATA-n’ field is a pair of hexadecimal digits defining the data byte to be sent. If no data is to be sent
    (length = zero), then the data bytes are omitted.
  - Each data byte specified must be a hexadecimal pair in order to eliminate ambiguity.
  - The terminating character ‘;’ signals the end of the message.
RTR CAN Message
  A RTR CAN message consists of the type (11-bit or 29-bit), identifier, length, does not have any data bytes,
  and is encoded as follows:
  RTR CAN Message Syntax
     : <S | X> <IDENTIFIER> <R> <LENGTH> ;
  - The first character ,‘:’, is for synchronization and allows the CAN USB-232 parser to detect the beginning
    of a command string.
  - The following character is either ‘S’ for standard 11-bit, or ‘X’ for extended 29-bit identifier type.
  - The ‘IDENTIFIER’ field consists of from one to eight hexadecimal digits, indicating the value of the
    identifier. Note that if a 29-bit value is entered and an 11-bit value was specified, the command will be
    treated as invalid and ignored.
  - The character ‘R’ indicates that the message is a RTR transmission.
  - The ‘LENGTH’ field is a single ASCII decimal character from ‘0’ to ‘8’ that specifies the length of the
    message.
  - The terminating character ‘;’ signals the end of the message.
*/


#ifndef GCSERIAL_H
#define GCSERIAL_H

#define NOCAN  // disallow processor's CAN, may not work with ESP32 compiler

#pragma message "Direct to Serial selected (GCSerial)"
#define BTYPE "GCSerial"

#include "OlcbCan.h"
#include "debugging.h"
#include <HardwareSerial.h>

class OlcbCanClass : public OlcbCan {
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
      if(!ext) p += sprintf(p, ":X%04X", (uint16_t)id<<5);  // shift if GC specific for CBUS
      //else p += sprintf(p, ":X%lX", id); // no shift for extended frames
      else p += sprintf(p, ":X%lX", id); // no shift for extended frames
      p += sprintf(p, "%c", (rtr ? 'R' : 'N') );
      for(uint8_t i=0;i<len;i++) {
          p += sprintf(p, "%02X", data[i]);
      }
      p += sprintf(p, "%c%c", ';', '\0');
      return 1;
  }
    uint8_t state = 0;
    uint8_t p = 0;
    HardwareSerial* inf;
  public:
    OlcbCanClass(HardwareSerial* _inf=&Serial) : inf(_inf) {}
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

