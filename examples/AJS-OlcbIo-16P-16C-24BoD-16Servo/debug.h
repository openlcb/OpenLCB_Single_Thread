//  Stuff that helps debugging
#define ENABLE_DEBUG_PRINT
#ifdef ENABLE_DEBUG_PRINT
  #define DEBUG(x) Serial.print(x)
  #define DEBUGL(x) Serial.println(x);
  #define DEBUGHEX(x,y) Serial.print(x,y);
#else
  #define DEBUG(x)
  #define DEBUGL(x)
  #define DEBUGHEX(x,y)
#endif

// Place messages reported at compile time
//#define Strinize(L) #L
//#define MakeString(M,L) M(L)
//#define $Line MakeString( Stringize, __LINE__ )
//#define Reminder __FILE__ "(" $Line ") : "
// Add reminders by:
// #pragma message(Reminder "Fix this problem")   --- do NOT use 'error' or 'warning' in your message

// add printf functionality.  

//  #include <stdarg.h>
//  void _printf(const char *fmt, ... ){
//        char tmp[64]; // resulting string limited to 64 chars
//        va_list args;
//        va_start (args, fmt );
//        vsnprintf(tmp, 64, fmt, args);
//        va_end (args);
//        Serial.print(tmp);
//  }


// changing putchar to use printf
//extern "C" {
//  int putchar(int c) {
//    Serial.write((uint8_t)c);
//    return c;
//  }
//}
// OR
//extern "C" {
//  int _write(int fd, char * str, int len)
//  {
//    for (int i = 0; i < len; i++) {
//      Serial.write((uint8_t)str[i]);
//    }
//    return len;
//  }
//}
//
// Discussion about small print libs: http://www.stm32duino.com/viewtopic.php?f=18&t=1014&sid=5c5245c090173bb2c480248a537d973f
