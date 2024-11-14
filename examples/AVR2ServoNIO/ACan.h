
//
// ACan.h
// Shim class
//
//  Created by David Harris 2024.11.02
//  Updated 2024.11.14
//

#ifndef ACan_H
#define ACan_H

#pragma message("Compiling ACan.h")

#include <stdio.h>
#include "OlcbCan.h"
#include "debugging.h"

#if !defined(ACAN_FREQ) || !defined(ACAN_CS_PIN) || !defined(ACAN_INT_PIN)
  #pragma message "Must define ACAN_FREQ, ACAN_CS_PIN, ACAN_INT_PIN"
#endif
#ifndef ACAN_RX_NBUF
  #define ACAN_RX_NBUF 4
#endif
#ifndef ACAN_TX_NBUF
  #define ACAN_TX_NBUF 2
#endif

#pragma message ACan for MCP2515

#include <ACAN2515.h>
#include <SPI.h>

#define NOCAN

ACAN2515 acan_can(ACAN_CS_PIN, SPI, ACAN_INT_PIN);

class OlcbCanClass : public OlcbCan {
  public:
    static uint32_t mpu_freq;       // mpu frequency
    static uint32_t bus_speed;      // bus speed
    static uint8_t cs_pin;          // chip select pin
    static uint8_t int_pin;         // chip interrupt pin
    //ACAN2515* acan_can;
    void init();                    // initialization
    uint8_t avail();                // read rxbuffer available
    uint8_t read();                 // read a buffer
    uint8_t txReady();              // write txbuffer available
    uint8_t write(long timeout);    // write, 0= immediately or fail; 0< if timeout occurs fail
    uint8_t write();                // write immediately or fail
    void process();
};

void OlcbCanClass::init(){
    SPI.begin();
    //Serial.print ("ACAN_FREQ: ") ;
    //Serial.println (ACAN_FREQ) ;
    ACAN2515Settings settings(ACAN_FREQ, 125000UL);
    settings.mRequestedMode = ACAN2515Settings::NormalMode;
    settings.mReceiveBufferSize = ACAN_RX_NBUF;
    settings.mTransmitBuffer0Size = ACAN_TX_NBUF;
    const uint16_t errorCode = acan_can.begin (settings, [] { acan_can.isr () ; }) ;
    #if 0
    Serial.begin(115200); delay(500); 
    if (errorCode == 0) {
      #if 0
      Serial.print ("Bit Rate prescaler: ") ;
      Serial.println (settings.mBitRatePrescaler) ;
      Serial.print ("Propagation Segment: ") ;
      Serial.println (settings.mPropagationSegment) ;
      Serial.print ("Phase segment 1: ") ;
      Serial.println (settings.mPhaseSegment1) ;
      Serial.print ("Phase segment 2: ") ;
      Serial.println (settings.mPhaseSegment2) ;
      Serial.print ("SJW: ") ;
      Serial.println (settings.mSJW) ;
      Serial.print ("Triple Sampling: ") ;
      Serial.println (settings.mTripleSampling ? "yes" : "no") ;
      Serial.print ("Actual bit rate: ") ;
      Serial.print (settings.actualBitRate ()) ;
      Serial.println (" bit/s") ;
      Serial.print ("Exact bit rate ? ") ;
      Serial.println (settings.exactBitRate () ? "yes" : "no") ;
      Serial.print ("Sample point: ") ;
      Serial.print (settings.samplePointFromBitStart ()) ;
      Serial.println ("%") ;
      #endif
    }else{
      Serial.print ("Configuration error 0x") ;
      Serial.println (errorCode, HEX) ;
    }
    #endif
}
uint8_t OlcbCanClass::avail() {
    return 1;
}
uint8_t OlcbCanClass::read(){
    //acan_can.poll();
    CANMessage frame;
    if(!acan_can.receive(frame)) return 0;
    if(frame.rtr) return 0;
    if(!frame.ext) return 0;
    id = frame.id;
    length = frame.len;
    memcpy(data, frame.data, length);
    return 1;
}
uint8_t OlcbCanClass::txReady(){
    return 1;
}
uint8_t OlcbCanClass::write(long timeout){
    #if 0
      Serial.print("\n>>>");
      Serial.print(id,HEX);
      Serial.print(":");
      Serial.print(length);
      Serial.print(":");
      for(int i=0;i<length;i++) {
        if(data[i]<16) Serial.print(0);
        Serial.print(data[i],HEX);
      }
    #endif
    CANMessage frame;
    frame.id = id;
    frame.ext = true;
    frame.rtr = false;
    frame.len = length;
    memcpy(frame.data, data, length);
    while(!acan_can.tryToSend(frame));
    delay(10);
    return 1;
}
uint8_t OlcbCanClass::write(){
    return write(0L);
}
void OlcbCanClass::process() {
  acan_can.poll();
}

#endif // ACan_H
