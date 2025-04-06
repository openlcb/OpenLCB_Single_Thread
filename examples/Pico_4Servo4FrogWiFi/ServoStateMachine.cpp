#include "ServoStateMachine.h"

#define SET_TARGET_ANGLE(t) servoArray.servo[servoNumber].servoEasing.setTargetAngle(t)
#define SET_CURRENT_STATE(c) servoArray.servo[servoNumber].currentState = c
#define EVENT_INDEX_LEAVING(p) servoArray.servo[servoNumber].position[p].eventIndexPositionLeaving
#define EVENT_INDEX_REACHED(p) servoArray.servo[servoNumber].position[p].eventIndexPositionReached
#define POSITION_DESCRIPTION(p) servoArray.servo[servoNumber].position[p].description
#define POSITION_0_DEGREES servoArray.servo[servoNumber].position[0].positionDegrees
#define POSITION_1_DEGREES servoArray.servo[servoNumber].position[1].positionDegrees
#define POSITION_2_DEGREES servoArray.servo[servoNumber].position[2].positionDegrees
#define CURRENT_STATE servoArray.servo[servoNumber].currentState
#define CURRENT_SERVO servoNumber+1
#define PRINT_NEW_STATE Serial.printf("\nServo %d new state is %s", CURRENT_SERVO, returnStateName(CURRENT_STATE))
#define PRINT_EVENT_INDEX_LEAVING(p) Serial.printf("\nServo %d sending event leaving position %d (%s)", CURRENT_SERVO, p+1, POSITION_DESCRIPTION(p))
#define PRINT_EVENT_INDEX_REACHED(p) Serial.printf("\nServo %d sending event reached position %d (%s)", CURRENT_SERVO, p+1, POSITION_DESCRIPTION(p))

// ServoStateMachine::ServoStateMachine() {
// }

const char * ServoStateMachine::returnStateName(State state) {
  switch (state) {
    case UNKNOWN: return (const char *) "UNKNOWN";
    case AT_POSITION_1: return (const char *) "AT_POSITION_1 (THROWN)";
    case AT_POSITION_2: return (const char *) "AT_POSITION_2 (MID_POINT)";
    case AT_POSITION_3: return (const char *) "AT_POSITION_3 (CLOSED)";
    case MOVING_TO_POSITION_1: return (const char *) "MOVING_TO_POSITION_1 (THROWN)";
    case MOVING_TO_POSITION_2: return (const char *) "MOVING_TO_POSITION_2 (MID_POINT)";
    case MOVING_TO_POSITION_3: return (const char *) "MOVING_TO_POSITION_3 (CLOSED)";
    case MOVING_FROM_POSITION_1_TO_POSITION_3: return (const char *) "MOVING_FROM_POSITION_1_(THROWN)_TO_POSITION_3_(CLOSED)";
    case MOVING_FROM_POSITION_3_TO_POSITION_1: return (const char *) "MOVING_FROM_POSITION_3_(CLOSED)_TO_POSITION_1_(THROWN)";
    default: return (const char *) "Invalid state";
  }
}

const char * ServoStateMachine::returnActionName(Action action) {
  switch (action) {
    case MOVE_TO_POSITION_1: return (const char *) "MOVE_TO_THROWN";
    case MOVE_TO_POSITION_2: return (const char *) "MOVE_TO_MID_POINT";
    case MOVE_TO_POSITION_3: return (const char *) "MOVE_TO_CLOSED";
    case MOVE_COMPLETED: return (const char *) "MOVE_COMPLETED";
    default: return (const char *) "Invalid action";
  }
}

void ServoStateMachine::initialise(ServoArray servoArray) {
  // Copy the supplied array to our private object.
  this->servoArray = servoArray;

  // For all servos convert their speed to a delay in mS for the servo easing to work.
  // Also set the servo's current and target angles to the servo's configured mid position.
  for(int i=0; i<NUM_SERVOS; i++) {
    this->servoArray.servo[i].servoEasing.setDelaymS(300/this->servoArray.servo[i].speedDegreesPerSecond); // Changed 1000 to 300 to cope with inaccurate millis().

    // Set the target and current angles to stop the servo moving when initialised.
    this->servoArray.servo[i].servoEasing.setCurrentAngle(this->servoArray.servo[i].position[1].positionDegrees);
    this->servoArray.servo[i].servoEasing.setTargetAngle(this->servoArray.servo[i].position[1].positionDegrees);

    // Attach the servo to its pin and set its initial angle.
    this->servoArray.servo[i].servoEasing.initialise(this->servoArray.servo[i].position[1].positionDegrees);
  }
}

int ServoStateMachine::update(uint8_t servoNumber) {
  if (servoArray.servo[servoNumber].servoEasing.update()) {
    // The servo has reached its target position.
    return processStateTransition(servoNumber, MOVE_COMPLETED);
  }

  return -1;
}

int ServoStateMachine::processStateTransition(uint8_t servoNumber, Action action) {
  Serial.printf("\nServo %d", CURRENT_SERVO);
  Serial.printf(" current state is %s", returnStateName(CURRENT_STATE));
  Serial.printf(" action is %s", returnActionName(action));

  switch (CURRENT_STATE) {
    case UNKNOWN:
      switch (action) {
        case MOVE_TO_POSITION_1: // currentState is UNKNOWN.
          Serial.printf("\nServo %d moving to position 1 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(0), POSITION_0_DEGREES);
          SET_TARGET_ANGLE(POSITION_0_DEGREES);
          SET_CURRENT_STATE(MOVING_TO_POSITION_1);
          PRINT_NEW_STATE;
          break;
        case MOVE_TO_POSITION_2: // currentState is UNKNOWN.
          Serial.printf("\nServo %d moving to position 2 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(1), POSITION_1_DEGREES);
          SET_TARGET_ANGLE(POSITION_1_DEGREES);
          SET_CURRENT_STATE(MOVING_TO_POSITION_2);
          PRINT_NEW_STATE;
          break;
        case MOVE_TO_POSITION_3: // currentState is UNKNOWN.
          Serial.printf("\nServo %d moving to position 3 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(2), POSITION_2_DEGREES);
          SET_TARGET_ANGLE(POSITION_2_DEGREES);
          SET_CURRENT_STATE(MOVING_TO_POSITION_3);
          PRINT_NEW_STATE;
          break;
        default: // currentState is UNKNOWN.
          Serial.printf("\nServo %d invalid transition", CURRENT_SERVO);
          break;
      }
      break;

    case MOVING_TO_POSITION_1:
      if (action == MOVE_COMPLETED) {
        //NODECONFIG.write(EEADDR(servoLastAngle[servoNumber]), servoPos0Degrees); // Store the position for restarting.
        //EEPROM.commit();
        Serial.printf("\nServo %d move completed to position 1 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(0), POSITION_0_DEGREES);
        SET_CURRENT_STATE(AT_POSITION_1);
        PRINT_NEW_STATE;
        PRINT_EVENT_INDEX_REACHED(0);
        return EVENT_INDEX_REACHED(0);
      } else {
        Serial.printf("\nServo %d invalid transition", CURRENT_SERVO);
      }
      break;

    case MOVING_TO_POSITION_2:
      if (action == MOVE_COMPLETED) {
        // NODECONFIG.write(EEADDR(servoLastAngle[servoNumber]), servoPos1Degrees); // Store the position for restarting.
        // EEPROM.commit();
        Serial.printf("\nServo %d move completed to position 2 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(1), POSITION_1_DEGREES);
        SET_CURRENT_STATE(AT_POSITION_2);
        PRINT_NEW_STATE;
        PRINT_EVENT_INDEX_REACHED(1);
        return EVENT_INDEX_REACHED(1);
      } else {
        Serial.printf("\nServo %d invalid transition", CURRENT_SERVO);
      }
      break;
    
    case MOVING_TO_POSITION_3:
      if (action == MOVE_COMPLETED) {
        // NODECONFIG.write(EEADDR(servoLastAngle[servoNumber]), servoPos2Degrees); // Store the position for restarting.
        // EEPROM.commit();
        Serial.printf("\nServo %d move completed to position 3 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(2), POSITION_2_DEGREES);
        SET_CURRENT_STATE(AT_POSITION_3);
        PRINT_NEW_STATE;
        PRINT_EVENT_INDEX_REACHED(2);
        return EVENT_INDEX_REACHED(2); // Send the 'reached position 3' event index.
      } else {
        Serial.printf("\nServo %d invalid transition", CURRENT_SERVO);
      }
      break;

    case MOVING_FROM_POSITION_1_TO_POSITION_3: // Thrown to Closed
      if (action == MOVE_COMPLETED) {
        //Serial.printf("\nServo %d move completed to position 2 (%d degrees)", CURRENT_SERVO, POSITION_1_DEGREES);
        Serial.printf("\nServo %d move completed to position 2 (%s) (%d degrees), now moving to position 3 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(1), POSITION_1_DEGREES, POSITION_DESCRIPTION(2), POSITION_2_DEGREES);
        //Serial.printf("\nServo %d moving to position 3 (%d degrees)", CURRENT_SERVO, servoPos2Degrees);
        SET_TARGET_ANGLE(POSITION_2_DEGREES);
        SET_CURRENT_STATE(MOVING_TO_POSITION_3);
        PRINT_NEW_STATE;
        PRINT_EVENT_INDEX_LEAVING(1);
        return EVENT_INDEX_LEAVING(1); // Send the 'leaving position 2' event.
      } else {
        Serial.printf("\nServo %d invalid transition", CURRENT_SERVO);
      }
      break;

    case MOVING_FROM_POSITION_3_TO_POSITION_1: // Closed to Thrown
      if (action == MOVE_COMPLETED) {
        //Serial.printf("\nServo %d move completed to position 2 (%d degrees)", CURRENT_SERVO, POSITION_1_DEGREES);
        Serial.printf("\nServo %d move completed to position 2 (%s) (%d degrees), now moving to position 1 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(1), POSITION_1_DEGREES, POSITION_DESCRIPTION(0), POSITION_0_DEGREES);
        //Serial.printf("\nServo %d moving to position 1 (%d degrees)", CURRENT_SERVO, servoPos0Degrees);
        SET_TARGET_ANGLE(POSITION_0_DEGREES);
        SET_CURRENT_STATE(MOVING_TO_POSITION_1);
        PRINT_NEW_STATE;
        PRINT_EVENT_INDEX_REACHED(1);
        return EVENT_INDEX_REACHED(1); // Send the 'reached position 2' event.
      } else {
        Serial.printf("\nServo %d invalid transition", CURRENT_SERVO);
      }
      break;

    case AT_POSITION_1:
      switch (action) {
        case MOVE_TO_POSITION_1: // currentState is AT_POSITION_1.
          Serial.printf("\nServo %d already at position 1 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(0), POSITION_0_DEGREES);
          SET_CURRENT_STATE(AT_POSITION_1);
          PRINT_NEW_STATE;
          PRINT_EVENT_INDEX_REACHED(0);
          return EVENT_INDEX_REACHED(0); // Send the 'reached position 1' event.
        case MOVE_TO_POSITION_2: // currentState is AT_POSITION_1.
          Serial.printf("\nServo %d moving to position 2 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(1), POSITION_1_DEGREES);
          SET_TARGET_ANGLE(POSITION_1_DEGREES);
          SET_CURRENT_STATE(MOVING_TO_POSITION_2);
          PRINT_NEW_STATE;
          PRINT_EVENT_INDEX_LEAVING(0);
          return EVENT_INDEX_LEAVING(0); // Send the 'leaving position 1' event.
        case MOVE_TO_POSITION_3: // currentState is AT_POSITION_1.
          Serial.printf("\nServo %d moving to position 2 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(1), POSITION_1_DEGREES);
          SET_TARGET_ANGLE(POSITION_1_DEGREES);
          SET_CURRENT_STATE(MOVING_FROM_POSITION_1_TO_POSITION_3);
          PRINT_NEW_STATE;
          PRINT_EVENT_INDEX_LEAVING(0);
          return EVENT_INDEX_LEAVING(0); // Send the 'leaving position 1' event.
        case MOVE_COMPLETED: // currentState is AT_POSITION_1.
          Serial.printf("\nServo %d invalid transition", CURRENT_SERVO);
          break;
      }
      break;

    case AT_POSITION_2:
      switch (action) {
        case MOVE_TO_POSITION_1: // currentState is AT_POSITION_2.
          Serial.printf("\nServo %d moving to position 1 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(0), POSITION_0_DEGREES);
          SET_TARGET_ANGLE(POSITION_0_DEGREES);
          SET_CURRENT_STATE(MOVING_TO_POSITION_1);
          PRINT_NEW_STATE;
          PRINT_EVENT_INDEX_LEAVING(1);
          return EVENT_INDEX_LEAVING(1); // Send the 'leaving position 2' event.
        case MOVE_TO_POSITION_2: // currentState is AT_POSITION_2.
          Serial.printf("\nServo %d already at position 2 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(1), POSITION_1_DEGREES);
          SET_CURRENT_STATE(AT_POSITION_2);
          PRINT_NEW_STATE;
          PRINT_EVENT_INDEX_REACHED(1);
          return EVENT_INDEX_REACHED(1); // Send the 'reached position 2' event.
        case MOVE_TO_POSITION_3: // currentState is AT_POSITION_2.
          Serial.printf("\nServo %d moving to position 3 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(2), POSITION_2_DEGREES);
          SET_TARGET_ANGLE(POSITION_2_DEGREES);
          SET_CURRENT_STATE(MOVING_TO_POSITION_3);
          PRINT_NEW_STATE;
          PRINT_EVENT_INDEX_LEAVING(1);
          return EVENT_INDEX_LEAVING(1);
        case MOVE_COMPLETED: // currentState is AT_POSITION_2.
          Serial.printf("\nServo %d invalid transition", CURRENT_SERVO);
          break;
      }
      break;

    case AT_POSITION_3:
      switch (action) {
        case MOVE_TO_POSITION_1: // currentState is AT_POSITION_3.
          Serial.printf("\nServo %d moving to position 2 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(1), POSITION_1_DEGREES);
          SET_TARGET_ANGLE(POSITION_1_DEGREES);
          SET_CURRENT_STATE(MOVING_FROM_POSITION_3_TO_POSITION_1);
          PRINT_NEW_STATE;
          PRINT_EVENT_INDEX_LEAVING(2);
          return EVENT_INDEX_LEAVING(2); // Send the 'leaving position 3' event.
        case MOVE_TO_POSITION_2: // currentState is AT_POSITION_3.
          Serial.printf("\nServo %d moving to position 2 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(1), POSITION_1_DEGREES);
          SET_TARGET_ANGLE(POSITION_1_DEGREES);
          SET_CURRENT_STATE(MOVING_TO_POSITION_2);
          PRINT_NEW_STATE;
          PRINT_EVENT_INDEX_LEAVING(2);
          return EVENT_INDEX_LEAVING(2);
        case MOVE_TO_POSITION_3: // currentState is AT_POSITION_3.
          Serial.printf("\nServo %d already at position 3 (%s) (%d degrees)", CURRENT_SERVO, POSITION_DESCRIPTION(2), POSITION_2_DEGREES);
          SET_CURRENT_STATE(AT_POSITION_3);
          PRINT_NEW_STATE;
          PRINT_EVENT_INDEX_REACHED(2);
          return EVENT_INDEX_REACHED(2);
        case MOVE_COMPLETED: // currentState is AT_POSITION_3.
          Serial.printf("\nServo %d invalid transition", CURRENT_SERVO);
          break;
      }
      break;
  }

  return -1;
}
