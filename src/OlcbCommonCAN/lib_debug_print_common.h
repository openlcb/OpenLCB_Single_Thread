//  Stuff that helps debugging
#ifndef LIB_ENABLE_DEBUG_PRINT_INC
#define LIB_ENABLE_DEBUG_PRINT_INC

// Uncomment the line below to enable Library Common Debug Printing
//#define LIB_DEBUG_PRINT_ENABLE
#ifdef LIB_DEBUG_PRINT_ENABLE
  #define LDEBUG(x) Serial.print(x)
  #define LDEBUGL(x) Serial.println(x);
  #define LDEBUG2(x,y) Serial.print(x,y);
#else
  #define LDEBUG(x)
  #define LDEBUGL(x)
  #define LDEBUG2(x,y)
#endif

#endif
