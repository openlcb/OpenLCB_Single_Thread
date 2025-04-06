#ifndef ServoStateMachine_h
#define ServoStateMachine_h

#include <Arduino.h>
#include "Global.h"
#include "JCServoEasing.h"

// States and Actions required to maintain the state of each servo.
enum State {
  UNKNOWN,
  AT_POSITION_1,
  AT_POSITION_2,
  AT_POSITION_3,
  MOVING_TO_POSITION_1,
  MOVING_TO_POSITION_2,
  MOVING_TO_POSITION_3,
  MOVING_FROM_POSITION_1_TO_POSITION_3,
  MOVING_FROM_POSITION_3_TO_POSITION_1
};

enum Action {
  MOVE_TO_POSITION_1,
  MOVE_TO_POSITION_2,
  MOVE_TO_POSITION_3,
  MOVE_COMPLETED
};

struct ServoPosition {
  char description[DESCRIPTION_LENGTH];
  uint8_t positionDegrees;
  uint16_t eventIndexPositionLeaving;
  uint16_t eventIndexPositionReached;
};

struct ServoArray {
  struct {
    ServoPosition position[NUM_POS];
    State currentState;
    uint8_t speedDegreesPerSecond;
    JCServoEasing servoEasing;
  } servo[NUM_SERVOS];
};

class ServoStateMachine {
  public:
    /**
     * Copies servoArray to the class's internal copy.
     */
    void initialise(ServoArray servoArray);

    /**
     * Checks to see if servoNumber needs moving and if it has reached its target position.
     * Returns the index of any event to send, or -1 if there is no event to send.
     */
    int update(uint8_t servoNumber);

    /**
     * Performs the following functions;-
     * - updates the current state for servoNumber.
     * - if required, causes the servo to start moving.
     * - if required, returns the index of the event to be sent, or -1 if there is no event to send.
     */
    int processStateTransition(uint8_t servoNumber, Action action);

    ServoArray servoArray;

  private:
    const char *returnStateName(State state);
    const char *returnActionName(Action action);

};

#endif
