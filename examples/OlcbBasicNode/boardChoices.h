// Board choices

//#define AVR
//#define IO 1
//#define TCH32C
//#define TCH32P
//#define TCH24C16P
//#define TIVA123
//#define Teensy
//#define ESP32

#ifdef ARDUINO_ARCH_AVR
//#if defined(ARDUINO_AVR_DUEMILANOVE) || defined(ARDUINO_AVR_NANO) || defined(ARDUINO_AVR_UNO)
#define BLUE 18   // BLUE is 18 LEDuino; others defined by board (48 IO, 14 IOuino)
#define GOLD 19   // GOLD is 19 LEDuino; others defined by board (49 IO, 15 IOuino)
//#pragma message("LEDuino i/o")
ButtonLed blue(BLUE, LOW);  // else will trigger reset.
ButtonLed gold(GOLD, LOW);
ButtonLed pA(14, LOW);
ButtonLed pB(15, LOW);
ButtonLed pC(LED_BUILTIN, HIGH);  // builtin led for testing without using extra hardware
ButtonLed pD(17, LOW);
#elif defined(ARDUINO_AVR_RS_IO)
#pragma message("Io i/o")
#define BLUE 48
#define GOLD 49
ButtonLed blue(BLUE, LOW);  // else will trigger reset.
ButtonLed gold(GOLD, LOW);
ButtonLed pA(8, LOW);  // prod0
ButtonLed pB(9, LOW);  // prod1
ButtonLed pC(0, LOW);  // cons0
ButtonLed pD(1, LOW);  // cons1
#elif defined(ARDUINO_AVR_TCH_CONSUMER) || defined(ARDUINO_AVR_TCH_PRODUCER) || defined(ARDUINO_AVR_TCH_PRODUCER_CONSUMER)
#pragma message("TCH i/o")
#define BLUE 42
#define GOLD 43
ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);
ButtonLed pA(2, LOW);  // prod0
ButtonLed pB(3, LOW);  // prod1
ButtonLed pC(4, LOW);  // cons0
ButtonLed pD(5, LOW);  // cons1
#elif defined(TIVA123)            // has two buttons and a RGB led
#pragma message("TIVA123 i/o")
#define BLUE 2                // arbitrarily chosen
#define GOLD 3                // arbitrarily chosen
ButtonLed blue(BLUE, HIGH);
ButtonLed gold(GOLD, HIGH);
ButtonLed pA(PUSH1, LOW);     // PUSH1
ButtonLed pB(PUSH2, LOW);     // prod2
ButtonLed pC(RED_LED, HIGH);  // cons0  30
ButtonLed pD(GREEN_LED, HIGH);// cons1  39
#elif defined(Teensy)
#pragma message("Teensy i/o")
#define BLUE A0               // arbitrarily chosen
#define GOLD A1               // arbitrarily chosen
ButtonLed blue(BLUE, HIGH);
ButtonLed gold(GOLD, HIGH);
ButtonLed pA(A2, LOW);        // arbitrary
ButtonLed pB(A3, LOW);        // arbitrary
ButtonLed pC(A4, HIGH);       // arbitrary
ButtonLed pD(A5, HIGH);       // arbitrary
#elif defined(ESP32)
#pragma message("ESP32 i/o")
#define BLUE A0               // arbitrarily chosen
#define GOLD A1               // arbitrarily chosen
ButtonLed blue(34, HIGH);
ButtonLed gold(35, HIGH);
ButtonLed pA(36, LOW);        // arbitrary
ButtonLed pB(39, LOW);        // arbitrary
ButtonLed pC(32, HIGH);       // arbitrary
ButtonLed pD(33, HIGH);       // arbitrary
#elif defined(ARDUINO_SAM_DUE)
#pragma message("DUE i/o")
#define BLUE A0               // arbitrarily chosen
#define GOLD A1               // arbitrarily chosen
ButtonLed blue(34, HIGH);
ButtonLed gold(35, HIGH);
ButtonLed pA(36, LOW);        // arbitrary
ButtonLed pB(39, LOW);        // arbitrary
ButtonLed pC(32, HIGH);       // arbitrary
ButtonLed pD(33, HIGH);       // arbitrary
#else
#pragma error("No board selected");
#endif
