//
//  mockCan.cpp
//  
//
//  Created by David Harris on 2018-01-15.
//
//


#pragma message("Compiling mockCan.cpp")

#include <stdio.h>
#include "OlcbCan.h"
#include "mockCan.h"

void Can::init(){
    Serial.print("\nIn mockCan::init()");
    return;
}

uint8_t Can::avail() {
    Serial.print("\nIn mockCan::avail()");
    return 1;
}

uint8_t Can::read(){
    Serial.print("\nIn mockCan::init()");
    return 1;
}

uint8_t Can::txReady(){
    Serial.print("\nIn mockCan::read()");
    return 1;
}

uint8_t Can::write(long timeout){
    Serial.print("\nIn mockCan::write(t)");
    return 1;
}

uint8_t Can::write(){
    return write(0L);
}

