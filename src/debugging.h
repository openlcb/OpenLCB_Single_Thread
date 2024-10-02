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

#ifndef debugging_h
#define debugging_h
#include "WString.h"

extern void dP(const __FlashStringHelper* x);
extern void dP(char x);
extern void dP(const char* x);
extern void dP(String x);
extern void dP(bool x);
extern void dP(uint8_t x);
extern void dP(uint16_t x);
extern void dP(uint32_t x);
extern void dPH(int x);
extern void dP(long x);

extern void dPH(uint8_t x);
extern void dPH(uint16_t x);
extern void dPH(uint32_t x);
extern void dPH(int x);

void dPS(const char* x, uint8_t y);
void dPS(const char* x, uint16_t y);
void dPS(const char* x, uint32_t y);
//void dPS(const char* x, unsigned int y) {   } //nano did not like
void dPS(const char* x, int y);
extern void dPS(String x, uint8_t y);
extern void dPS(String x, uint16_t y);
extern void dPS(String x, uint32_t y);
extern void dPS(String x, int y);


//#endif

#endif /* debugging_h */
