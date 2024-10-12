

#if defined(ATOM)
  #define SDA 26
  #define SCL 32
#elif defined(NANOC6)
  #define SDA 2
  #define SCL 1
#else
  #error "Needs ESP32, like Atom or NanoC6"
#endif

#include "debugging.h"

//#define USE_NO_SERVO_LIB
#define USE_USER_PROVIDED_SERVO_LIB

#include "M5_UNIT_8SERVO.h"
M5_UNIT_8SERVO unit_8servo;

#define MIN_PULSE_WIDTH       544     // the shortest pulse sent to a servo
#define MAX_PULSE_WIDTH      2400     // the longest pulse sent to a servo
#define DEFAULT_PULSE_WIDTH  1500     // default pulse width when servo is attached
#define REFRESH_INTERVAL    20000     // minimum time to refresh servos in microseconds
#define SERVO_MIN() (MIN_PULSE_WIDTH - this->min * 4)  // minimum value in us for this servo
#define SERVO_MAX() (MAX_PULSE_WIDTH - this->max * 4)  // maximum value in us for this servo

int nservos = 0;
class Servo {
 public:
  Servo(){}
  void begin() {
    dP((String)"\nServo::begin()");
    //while (!unit_8servo.begin(&Wire, SDA, SCL, M5_UNIT_8SERVO_DEFAULT_ADDR)) {
    if (!unit_8servo.begin(&Wire, SDA, SCL, M5_UNIT_8SERVO_DEFAULT_ADDR)) {
      dP((String)"\nextio Connect Error");
      delay(100);
    }
      unit_8servo.setAllPinMode(SERVO_CTL_MODE);
    while(0) {
      unit_8servo.setServoAngle(0, 10); Serial.print("\n10");
      delay(1000);
      unit_8servo.setServoAngle(0, 90); Serial.print("\n90");
      delay(1000);
      unit_8servo.setServoAngle(0, 170); Serial.print("\n170");
      delay(1000);
    }
    dPS("\nNumber of servos = ", (uint8_t)nservos);
  }
  uint8_t attach(int pin) { return attach(pin, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH); } // attach the given pin to the next free channel, sets pinMode, returns channel number or INVALID_SERVO if failure
  uint8_t attach(int pin, int mn, int mx) {                          // as above but also sets min and max values for writes.
    //min=mn; max=mx; 
    servoIndex = nservos++;
    dPS("\nattach ", pin);
    dPS(", ", servoIndex);
    dPS(", ", min);
    dPS(", ", max);
    min = mn;
    max = mx;
    return servoIndex;
  } 
  void detach() {}
  void write(int value) {            // if value is < 200 it's treated as an angle, otherwise as pulse width in microseconds
    dPS("\nwrite ", (uint16_t)value);
    dPS(", ", (uint8_t)servoIndex);
    if(value < MIN_PULSE_WIDTH) {  // treat values less than 544 as angles in degrees (valid values in microseconds are handled as microseconds)
      if(value < 0) value = 0;
      if(value > 180) value = 180;
      value = map(value, 0, 180, SERVO_MIN(),  SERVO_MAX());
    }
    writeMicroseconds(value);
  }
  void writeMicroseconds(int value) { // Write pulse width in microseconds
    //Serial.printf("\nwritems %d, %d", value, servoIndex);
    if( value < SERVO_MIN() ) value = SERVO_MIN();
    else if( value > SERVO_MAX() ) value = SERVO_MAX();
    pulsewidth = value;
    unit_8servo.setServoPulse(servoIndex, pulsewidth);
  }
  int read(){                       // returns current pulse width as an angle between 0 and 180 degrees
    return map( readMicroseconds()+1, SERVO_MIN(), SERVO_MAX(), 0, 180);
  }
  int readMicroseconds(){ return pulsewidth; }            // returns current pulse width in microseconds for this servo (was read_us() in first release)
  bool attached(){ return true; }                   // return true if this servo is attached, otherwise false
private:
   uint8_t servoIndex;               // index into the channel data for this servo
   uint16_t pulsewidth;              // pulse width in microseconds
   int8_t min;                       // minimum is this value times 4 added to MIN_PULSE_WIDTH
   int8_t max;                       // maximum is this value times 4 added to MAX_PULSE_WIDTH
};