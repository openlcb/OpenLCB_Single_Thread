//  Stuff that helps debugging
#ifndef LIB_ENABLE_DEBUG_PRINT_INC
#define LIB_ENABLE_DEBUG_PRINT_INC

// Uncomment the line below to enable Library Common Debug Printing
#define LIB_DEBUG_PRINT_ENABLE
#ifdef LIB_DEBUG_PRINT_ENABLE

#include <Stream.h>
static Stream *DebugStream = NULL;
	static void setDebugStream(Stream *newStream) { DebugStream = newStream; }

  #define LDEBUG(x) if(DebugStream) DebugStream->print(x);
  #define LDEBUGL(x) if(DebugStream) DebugStream->println(x);
  #define LDEBUG2(x,y) if(DebugStream) DebugStream->print(x,y);
#else
	#define setDebugStream(x)
  #define LDEBUG(x)
  #define LDEBUGL(x)
  #define LDEBUG2(x,y)
#endif

#endif
