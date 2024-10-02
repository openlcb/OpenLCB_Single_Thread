//
//  mockCan.cpp
//  
//
//  Created by David Harris on 2018-01-15.
//
//

#if defined MOCK

#pragma message("Compiling mockCan.cpp")

#include <stdio.h>
#include "OlcbCan.h"
#include "mockCan.h"

void OlcbCanClass::init(){
    Serial.print("\nIn mockCan::init()");
    return;
}

uint8_t OlcbCanClass::avail() {
    Serial.print("\nIn mockCan::avail()");
    return 1;
}

uint8_t OlcbCanClass::read(){
    Serial.print("\nIn mockCan::init()");
    return 1;
}

uint8_t OlcbCanClass::txReady(){
    Serial.print("\nIn mockCan::read()");
    return 1;
}

uint8_t OlcbCanClass::write(long timeout){
    Serial.print("\nIn mockCan::write(t)");
    return 1;
}

uint8_t OlcbCanClass::write(){
    return write(0L);
}

#endif // MOCK
