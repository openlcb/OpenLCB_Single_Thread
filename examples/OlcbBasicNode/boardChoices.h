{\rtf1\ansi\ansicpg1252\cocoartf1348\cocoasubrtf170
{\fonttbl\f0\fswiss\fcharset0 Helvetica;}
{\colortbl;\red255\green255\blue255;}
\margl1440\margr1440\vieww10800\viewh8400\viewkind0
\pard\tx566\tx1133\tx1700\tx2267\tx2834\tx3401\tx3968\tx4535\tx5102\tx5669\tx6236\tx6803\pardirnatural

\f0\fs24 \cf0 // Board choices\
\
//#define AVR\
//#define IO 1\
//#define TCH32C\
//#define TCH32P\
//#define TCH24C16P\
//#define TIVA123\
//#define Teensy\
//#define ESP32\
\
#if defined(ARDUINO_AVR_DUEMILANOVE)\
    #define BLUE 18   // BLUE is 18 LEDuino; others defined by board (48 IO, 14 IOuino)\
    #define GOLD 19   // GOLD is 19 LEDuino; others defined by board (49 IO, 15 IOuino)\
    //#pragma message("LEDuino i/o")\
    ButtonLed blue(BLUE, LOW);  // else will trigger reset.  \
    ButtonLed gold(GOLD, LOW);\
    ButtonLed pA(14, LOW); \
    ButtonLed pB(15, LOW);\
    ButtonLed pC(16, LOW);\
    ButtonLed pD(17, LOW);\
#elif defined(IO)\
    //#pragma message("Io i/o")\
    #define BLUE 48\
    #define GOLD 49\
    ButtonLed blue(BLUE, LOW);  // else will trigger reset.  \
    ButtonLed gold(GOLD, LOW);\
    ButtonLed pA(8, LOW);  // prod0\
    ButtonLed pB(9, LOW);  // prod1\
    ButtonLed pC(0, LOW);  // cons0\
    ButtonLed pD(1, LOW);  // cons1\
#elif defined(TCH32C) || defined(TCH32P) || defined(TCH24C16P)\
    #define BLUE 42\
    #define GOLD 43\
    ButtonLed pA(2, LOW);  // prod0\
    ButtonLed pB(3, LOW);  // prod1\
    ButtonLed pC(4, LOW);  // cons0\
    ButtonLed pD(5, LOW);  // cons1\
#elif defined(TIVA123)                    // has two buttons and a RGB led\
    #define BLUE 2                // arbitrarily chosen\
    #define GOLD 3                // arbitrarily chosen\
    ButtonLed blue(BLUE, HIGH);\
    ButtonLed gold(GOLD, HIGH);\
    ButtonLed pA(PUSH1, LOW);         // arbitrary   PUSH1 does not work as it is dedicated to debugger\
    ButtonLed pB(PUSH2, LOW);     // prod1  17   PUSH2 works\
    ButtonLed pC(RED_LED, HIGH);   // cons0  30\
    ButtonLed pD(GREEN_LED, HIGH); // cons1  39\
#elif defined(Teensy)                    // has two buttons and a RGB led\
    #define BLUE A0                // arbitrarily chosen\
    #define GOLD A1               // arbitrarily chosen\
    ButtonLed blue(BLUE, HIGH);\
    ButtonLed gold(GOLD, HIGH);\
    ButtonLed pA(A2, LOW);         // arbitrary   PUSH1 does not work as it is dedicated to debugger\
    ButtonLed pB(A3, LOW);     // prod1  17   PUSH2 works\
    ButtonLed pC(A4, HIGH);   // cons0  30\
    ButtonLed pD(A5, HIGH); // cons1  39\
#elif defined(ESP32)}