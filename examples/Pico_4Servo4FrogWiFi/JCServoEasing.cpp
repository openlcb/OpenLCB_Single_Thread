#include "JCServoEasing.h"

void JCServoEasing::initialise(int initialAngle) {
  int retval = servo.attach(servoPin, SERVO_PWM_DEG_0, SERVO_PWM_DEG_180);
  Serial.printf("\nServo attached to pin %d, retval is %d", servoPin, retval);
  servo.write(initialAngle); // Position the servo to the mid position.
}

bool JCServoEasing::update() {
  // Implement a non blocking delay for delaymS.
  if (millis() < nextUpdate) return false;
  nextUpdate = millis() + delaymS;

  // The delay has expired so its time to check the servo.

  // Check if any movement is required.
  if (currentAngle == targetAngle) return false;

  if (currentAngle < targetAngle) {
    // currentAngle needs to increase.
    currentAngle++;
    servo.write(currentAngle);
    //Serial.printf("\n%d: Servo pin %d, delay=%d, angle = %d", millis(), this->servoPin, this->delaymS, this->currentAngle);
  
    // Have we reached the target yet?
    if (currentAngle == targetAngle) {
      return true;
    } else {
      return false;
    }
  }

  if (currentAngle > targetAngle) {
    // currentAngle needs to decrease.
    currentAngle--;
    servo.write(currentAngle);
    //Serial.printf("\n%d: Servo pin %d, delay=%d, angle = %d", millis(), this->servoPin, this->delaymS, this->currentAngle);
  
    // Have we reached the target yet?
    if (currentAngle == targetAngle) {
      return true;
    } else {
      return false;
    }
  }

  // If we get here the servo must be still moving or has already reached its target angle.
  // SHOULD NEVER GET HERE !!!
  return false;
}
