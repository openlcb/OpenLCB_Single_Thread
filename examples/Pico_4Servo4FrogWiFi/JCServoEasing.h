#ifndef JCServoEasing_h
#define JCServoEasing_h

#include <Arduino.h>
#include <Servo.h>
#include "Global.h"

/**
 * Represents one servo.
 */
class JCServoEasing {
  public:
    void setServoPin(uint8_t servoPin) { this->servoPin = servoPin; }
    void setCurrentAngle(uint8_t currentAngle) { this->currentAngle = currentAngle; }
    void setTargetAngle(uint8_t targetAngle) { this->targetAngle = targetAngle; }
    void setDelaymS(uint16_t delaymS) { this->delaymS = delaymS; }

    /**
     * Initialise the servo by attaching it to its pin.
     */
    void initialise(int initialAngle);

    /**
     * Checks to see if the servo needs to have its angle changed.
     * Returns false if there is no need to move the servo, or it is still moving.
     * Returns true if the servo has reached its target angle.
     */
    bool update();

  private:
    Servo servo;

    pin_size_t servoPin; // The pin to which this servo is attached.
    int currentAngle;
    int targetAngle;
    uint16_t delaymS; // Used to slow down the servo movement.

    long nextUpdate = 0;

};


#endif
