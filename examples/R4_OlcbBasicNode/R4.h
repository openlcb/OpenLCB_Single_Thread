/* 
  OlcbCanClass for Uno R4 Minima
    Transceiver connections:
      D4 = CanTx
      D5 = CanRx

  (c) David P Harris 2025
*/

#if !defined(R4CAN_H) && !defined(NOCAN)
#define R4CAN_H

#ifndef ARDUINO_UNOR4_MINIMA
  #error "R4CAN is only available on ARDUINO_UNOR4_MINIMA."
#else 
  #pragma message "Using R4CAN class"
#endif 

#include <Arduino_CAN.h>
#include "OlcbCan.h"
#include "debugging.h"

class OlcbCanClass : public OlcbCan {
  public:
  void init() {
    if (!CAN.begin(CanBitRate::BR_125k)) {
      Serial.println("CAN.begin(...) failed.");
      while(1);
    }
    //CAN.enableInternalLoopback();
    dP("\nCAN initialized ok");
  }
  uint8_t avail() { return CAN.available(); }
  uint8_t read() {
    if(!CAN.available()) return 0;
    CanMsg const msg = CAN.read();
    id = msg.id & 0x1FFFFFFFL;
    length = msg.data_length;
    memcpy( data, msg.data, length );
    dP("\n<<<"); print();
    return 1;
  }
  uint8_t txReady() { return 1; }

  uint8_t write(long timeout) {
    return write();
    if(timeout==0) return write();
    static long last = millis();
    while( (millis()-last) < timeout ) {
      if( write() ) { dP( "write success"); return 1; }
    }
    dP( "write failed");
    return 0;
  }
  uint8_t write() {
    CanMsg const msg(CanExtendedId(id), length, data);
    delay(1);
    int ret = CAN.write(msg); // 1=ok, else -rc
    active = true;
    dP("\n>>>"); print(); dP(" ret="); dP((uint16_t)ret);
    return ret==1;
  }
  void print() {
    dPH(id);
    for(int i=0; i<length; i++) { dP(" "); dPH(data[i]); }
  }
};

#if 0
OlcbCanClass cantx, canrx;
void setup() {
  Serial.begin(115200); while(!Serial); delay(1000);
  dP("\n\nYo!");
  cantx.init();
  cantx.id=0x195B4123;
  cantx.length = 8;
  for(int i=0;i<8;i++) cantx.data[i] = (i+1)*0x11;
}
void loop() {
  static long last =0;
  if( (millis()-last) >2000 ) {
    last = millis();
    cantx.write();
    cantx.data[7]++;
  }
  if(canrx.avail()) {
    canrx.read();
  }
}
#endif

#endif // R4CAN_H
