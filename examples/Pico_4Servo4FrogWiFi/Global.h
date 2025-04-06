#ifndef Global_h
#define Global_h

#include "JCServoEasing.h"
#include <Servo.h>

// WiFi uses one PIO so only seven servos are available. Only four are needed for this sketch.
#define NUM_SERVOS 4
#define NUM_POS 3

#define NUM_OUTPUTS 8

#define NUM_SERVO_EVENTS NUM_SERVOS * NUM_POS * 3
#define NUM_OUTPUT_EVENTS NUM_OUTPUTS * 2
#define NUM_EVENT NUM_SERVO_EVENTS + NUM_OUTPUT_EVENTS

#define DESCRIPTION_LENGTH 16 // Used for all description char arrays.

#define SERVO_PWM_DEG_0    540 // this is the 'minimum' pulse length count (out of 4096)
#define SERVO_PWM_DEG_180  2400 // this is the 'maximum' pulse length count (out of 4096)
#define SERVO_SPEED 45 // servo speed in degrees per second.

#endif
