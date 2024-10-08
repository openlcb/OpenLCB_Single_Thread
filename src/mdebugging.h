//
//  mdebugging.h
//  
//
//  Created by Dave Harris on 2024-09-14.
//

// This is the main debuggin file to be included in the main file
// It DEBUG is defined, then it defines three functions:
//   - dP(x)
//   - dP2(x,y), where y can be HEX, BIN
//   - dPV(n,x), where n is a name char*, and x
// Else they are stobs.
//
// DEBUG has to be defined as a Stream, eg Serial, Serial4

#ifndef mdebugging_h
#define mdebugging_h
#include "WString.h"

#ifdef DEBUG
  //#pragma message "mdebugging.h debugging included"
    void dP(const __FlashStringHelper* x) { DEBUG.print((__FlashStringHelper*)x); DEBUG.flush();}
    void dP(char x) { DEBUG.print((char)x); DEBUG.flush();}
    void dP(const char* x) { DEBUG.print((const char*)x); DEBUG.flush();}
    void dP(String x) { DEBUG.print((String)x); DEBUG.flush();}
    void dP(bool x) { DEBUG.print((bool)x); DEBUG.flush();}
    void dP(uint8_t x) { DEBUG.print((uint8_t)x); DEBUG.flush();}
    void dP(uint16_t x) { DEBUG.print((uint16_t)x); DEBUG.flush();}
    void dP(uint32_t x) { DEBUG.print((uint32_t)x); DEBUG.flush();}
    void dP(int x) { DEBUG.print((int)x); DEBUG.flush();}
    void dP(long x) { DEBUG.print((long)x); DEBUG.flush();}
    void dP(float x) { DEBUG.print((float)x); DEBUG.flush();}

    void dPH(uint8_t x) { DEBUG.print((uint8_t)x,HEX); DEBUG.flush();}
    void dPH(uint16_t x) { DEBUG.print((uint16_t)x,HEX); DEBUG.flush();}
    void dPH(uint32_t x) { DEBUG.print((uint32_t)x,HEX); DEBUG.flush();}
        void dPH(int x) { DEBUG.print(x,HEX); }

    void dPS(const char* x, uint8_t y) { DEBUG.print(x); DEBUG.print(y); DEBUG.flush();}
    void dPS(const char* x, uint16_t y) { DEBUG.print(x); DEBUG.print(y); DEBUG.flush();}
    void dPS(const char* x, uint32_t y) { DEBUG.print(x); DEBUG.print(y); DEBUG.flush();}
        //void dPS(const char* x, unsigned int y) { DEBUG.print(x); DEBUG.print(y); DEBUG.flush();}
    void dPS(const char* x, int y) { DEBUG.print(x); DEBUG.print(y); DEBUG.flush();}
    void dPS(String x, uint8_t y) { DEBUG.print(x); DEBUG.print(y); DEBUG.flush();}
    void dPS(String x, uint16_t y) { DEBUG.print(x); DEBUG.print(y); DEBUG.flush();}
    void dPS(String x, uint32_t y) { DEBUG.print(x); DEBUG.print(y); DEBUG.flush();}
    void dPS(String x, int y) { DEBUG.print(x); DEBUG.print(y); DEBUG.flush();}
#else
  //#pragma message "mdebugging.h debugging excluded"
    void dP(const __FlashStringHelper* x) { }
    void dP(char x) {  }
    void dP(const char* x) { }
    void dP(String x) {  }
    void dP(bool x) {  }
    void dP(uint8_t x) {  }
    void dP(uint16_t x) {  }
    void dP(uint32_t x) {  }
    void dP(int x) {  }
    void dP(long x) { }
    void dP(float x) { }

    void dPH(uint8_t x) {  }
    void dPH(uint16_t x) {  }
    void dPH(uint32_t x) {  }
    void dPH(int x) {  }

    void dPS(const char* x, uint8_t y) {   }
    void dPS(const char* x, uint16_t y) {   }
    void dPS(const char* x, uint32_t y) {   }
    //void dPS(const char* x, unsigned int y) {   } //nano did not like
    void dPS(const char* x, int y) {   }
    void dPS(String x, uint8_t y) {   }
    void dPS(String x, uint16_t y) {   }
    void dPS(String x, uint32_t y) {   }
    void dPS(String x, int y) {   }
#endif

#endif /* mdebugging_h */
