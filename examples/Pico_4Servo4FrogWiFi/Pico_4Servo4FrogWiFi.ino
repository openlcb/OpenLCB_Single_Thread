//==============================================================
// Pico 4Servo4FrogWifi based on
// https://github.com/openlcb/OpenLCB_Single_Thread/tree/master/examples/Pico_8ServoWifiGC
//
// Modified John Callingham 2025, added;-
// - servo easing.
// - events produced for servo reaching and leaving positions.
// - outputs for control of frog polarity.
// MOdified DPH 2024
// Copyright 2019 Alex Shepherd and David Harris
//==============================================================

#include "Global.h"

#define NOCAN // There is no CAN controller, just WiFi.
#define OLCB_NO_BLUE_GOLD // We are not using the blue and gold buttons.

const char* ssid     = "RPi-JMRI";     // <-- fill in with your network name
const char* password = "rpI-jmri";         // <-- fill in with youy network password
const char* openLCB_can  = "openlcb-can";  // <-- change this if necessary
#include "PicoWifiGC.h"

// Board definitions
#define MANU "OpenLCB"  // The manufacturer of node
#define MODEL "Pico8ServosWifi" // The model of the board
#define HWVERSION "0.1"   // Hardware version
#define SWVERSION "0.1"   // Software version

// Use a unique Node ID.
#define NODE_ADDRESS  5,1,1,1,0x91,0x02

// Set to 1 to Force Reset EEPROM to Factory Defaults 
// Need to do this at least once.  
#define RESET_TO_FACTORY_DEFAULTS 0

#include <Arduino.h>

#include "mdebugging.h"           // debugging
#include "processor.h"
#include "processCAN.h"
#include "OpenLCBHeader.h"

#include "ServoStateMachine.h"
ServoStateMachine servoStateMachine;

// CDI (Configuration Description Information) in xml, must match MemStruct
// See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
extern "C" {
    #define N(x) xN(x)     // allow the insertion of the value (x) ..
    #define xN(x) #x       // .. into the CDI string. 
const char configDefInfo[] PROGMEM =
// ===== Enter User definitions below =====
  CDIheader R"(
        <group replication=')" N(NUM_SERVOS) R"('>
            <name>Servos</name>
            <repname>1</repname>
            <string size='16'><name>Description</name></string>
            <group replication=')" N(NUM_POS) R"('>
                <name>Position</name>
                <repname>1</repname>
                <string size='16'><name>Description</name></string>
                <eventid>
                  <name>Move event</name>
                  <description>Send this event to start this servo moving to this position</description>
                </eventid>
                <int size='1'>
                  <name>Servo Position in Degrees</name>
                  <min>0</min><max>180</max>
                  <hints><slider tickSpacing='45' immediate='yes' showValue='true'></slider></hints>
                </int>
                <eventid>
                  <name>Reached event</name>
                  <description>This event will be sent when this servo reaches this position</description>
                </eventid>
                <eventid>
                  <name>Leaving event</name>
                  <description>This event will be sent when this servo leaves this position</description>
                </eventid>
            </group>
        </group>
        <group>
            <name>Outputs</name>
            <description>Output events to control frog polarity</description>
            <group replication=')" N(NUM_OUTPUTS) R"('>
              <name>Digital Output Pins</name>
              <repname>1</repname>
              <string size='16'><name>Description</name></string>
              <eventid>
                <name>Low Event</name>
                <description>Receiving this event sets this output low</description>
              </eventid>
              <eventid>
                <name>High Event</name>
                <description>Receiving this event sets this output high</description>
              </eventid>
            </group>
        </group>
    )" CDIfooter;
// ===== Enter User definitions above =====
} // end extern

// ===== MemStruct =====
//   Memory structure of EEPROM, must match CDI above
    typedef struct { 
          EVENT_SPACE_HEADER eventSpaceHeader; // MUST BE AT THE TOP OF STRUCT - DO NOT REMOVE!!!
          
          char nodeName[20];  // optional node-name, used by ACDI
          char nodeDesc[24];  // optional node-description, used by ACDI
      // ===== Enter User definitions below =====
          // uint16_t ServoPwmMin;
          // uint16_t ServoPwmMax;
          struct {
            char servodesc[DESCRIPTION_LENGTH];        // description of this Servo Turnout Driver
            struct {
              char positiondesc[DESCRIPTION_LENGTH];      // description of this Servo Position
              EventID eid;       // consumer eventID
              uint8_t pos;       // position
              EventID rid;        // eventID produced when position reached
              EventID lid;        // eventID produced when position left
            } pos[NUM_POS];
          } servos[NUM_SERVOS];
          struct {
            char desc[DESCRIPTION_LENGTH];        // description of this output
            EventID setLow;       // Consumed eventID which sets this output pin
            EventID setHigh;      // Consumed eventID which resets this output pin
          } outputs[NUM_OUTPUTS];
      // ===== Enter User definitions above =====
      // items below will be included in the EEPROM, but are not part of the CDI
      // uint8_t servoLastAngle[NUM_SERVOS];
    } MemStruct;                 // type definition

// This is called to initialize the EEPROM during Factory Reset.
void userInitAll() {

  NODECONFIG.put(EEADDR(nodeName), ESTRING("PicoW 02"));
  NODECONFIG.put(EEADDR(nodeDesc), ESTRING("8ServosWifi"));
  
  for(uint8_t i = 0; i < NUM_SERVOS; i++) {
    NODECONFIG.put(EEADDR(servos[i].servodesc), ESTRING(""));
    NODECONFIG.put(EEADDR(servos[i].pos[0].positiondesc), ESTRING("Thrown"));
    NODECONFIG.put(EEADDR(servos[i].pos[1].positiondesc), ESTRING("Mid"));
    NODECONFIG.put(EEADDR(servos[i].pos[2].positiondesc), ESTRING("Closed"));

    // Factory reset positions.
    NODECONFIG.put(EEADDR(servos[i].pos[0].pos), 80); // Thrown
    NODECONFIG.put(EEADDR(servos[i].pos[1].pos), 90); // Mid point
    NODECONFIG.put(EEADDR(servos[i].pos[2].pos), 100); // Closed
  }

  for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
    char description[DESCRIPTION_LENGTH];
    uint8_t frog = i/2;
    char polarity;
    if ((i % 2) == 0) {
      polarity = 'J';
    } else {
      polarity = 'K';
    }
    sprintf(description, "Fr%d - %c", frog+1, polarity);
    NODECONFIG.put(EEADDR(outputs[i].desc), ESTRING(description));
  }
}

extern "C" {
    // ===== eventid Table =====
    // useful macro to help fill the table

    // each position has 3 eventids
    #define REG_POS(s,p) CEID(servos[s].pos[p].eid), PEID(servos[s].pos[p].rid), PEID(servos[s].pos[p].lid)  

    // each servo has three positions
    #define REG_SERVO(s) REG_POS(s,0), REG_POS(s,1), REG_POS(s,2)  

    // Each output has two events.
    #define REG_OUTPUT(s) CEID(outputs[s].setLow), CEID(outputs[s].setHigh)
    
    //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
    const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        REG_SERVO(0), REG_SERVO(1), REG_SERVO(2), REG_SERVO(3), 
        REG_OUTPUT(0), REG_OUTPUT(1), REG_OUTPUT(2), REG_OUTPUT(3),
        REG_OUTPUT(4), REG_OUTPUT(5), REG_OUTPUT(6), REG_OUTPUT(7)
    };
    
    // SNIP Short node description for use by the Simple Node Information Protocol
    // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
    extern const char SNII_const_data[] PROGMEM = "\001" MANU "\000" MODEL "\000" HWVERSION "\000" OlcbCommonVersion ; // last zero in double-quote
} // end extern "C"

// PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
// See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
uint8_t protocolIdentValue[6] = {   //0xD7,0x58,0x00,0,0,0};
        pSimple | pDatagram | pMemConfig | pPCEvents | !pIdent    | pTeach     | !pStream   | !pReservation, // 1st byte
        pACDI   | pSNIP     | pCDI       | !pRemote  | !pDisplay  | !pTraction | !pFunction | !pDCC        , // 2nd byte
        0, 0, 0, 0                                                                                           // remaining 4 bytes
    };

// Define the pins for the servos.
uint8_t servoPin[] = { 12, 13, 14, 15 };

// Define the pins for the outputs.
uint8_t outputPin[] = { 26, 22, 21, 20, 19, 18, 17, 16 };

// ===== Process Consumer-eventIDs =====
void pceCallback(uint16_t index) {
  // Invoked when an event is consumed.
  //dP("\neventid callback: index="); dP((uint16_t)index);
  Serial.printf("\n\nevent received: index=0x%02X", index);

  // The first NUM_SERVO_EVENTS are for servos. The remaining NUM_OUTPUT_EVENTS are for outputs.
  if (index < NUM_SERVO_EVENTS) {
    // This is a servo event.

    // Update the servo state according to the servo number and position requested.
    uint8_t servoNumber = index / 9; // servoNumber is 0 - 7. 3 positions per servo, 3 events per position.
    uint8_t servoPosition = (index % 9) / 3; // servoPosition is 0 - 2.
    //NODECONFIG.write(EEADDR(servoState[servoNumber]), servoPosition);

    // Update the state for this servo.
    int eventToSend;
    switch (servoPosition) {
      case 0:
        eventToSend = servoStateMachine.processStateTransition(servoNumber, MOVE_TO_POSITION_1);
        break;
      case 1:
        eventToSend = servoStateMachine.processStateTransition(servoNumber, MOVE_TO_POSITION_2);
        break;
      case 2:
        eventToSend = servoStateMachine.processStateTransition(servoNumber, MOVE_TO_POSITION_3);
        break;
    }
    if (eventToSend != -1) {
      OpenLcb.produce(eventToSend);
    }
  } else {
    // This is an output event.
    uint8_t outputEvent = index - NUM_SERVO_EVENTS; // there are 16 output events. 4 frogs, each frog has J low, J high, K low and K high.
    Serial.printf("\n\noutputEvent = %d", outputEvent);

    uint8_t outputNumber = (outputEvent) / 2; // outputNumber is 0 - 7.
    //Serial.printf("\noutputNumber = %d", outputNumber);

    uint8_t frogNumber = outputNumber / 2; // frogNumber is 0 - 3.
    //Serial.printf("\nfrogNumber = %d", frogNumber);

    uint8_t outputState = (outputEvent) % 2; // outputState: 0 is low, 1 is high.
    //Serial.printf("\noutputState = %d", outputState);

    if (outputState == 1) {
      // Turn both frog outputs low to ensure that a frog cannot have both J and K outputs high at the same time.
      uint8_t outputPinJ = (frogNumber * 2) + 0; // the output pin for the J connection for this frog.
      //Serial.printf("\noutputPinJ = %d", outputPinJ);
      digitalWrite(outputPin[outputPinJ], LOW);
      uint8_t outputPinK = (frogNumber * 2) + 1; // the output pin for the K connection for this frog.
      //Serial.printf("\noutputPinK = %d", outputPinK);
      digitalWrite(outputPin[outputPinK], LOW);

      // Turn the required output high.
      digitalWrite(outputPin[outputNumber], HIGH);
    } else {
      digitalWrite(outputPin[outputNumber], LOW);
    }
  }
}

void produceFromInputs() {
    // called from loop(), this looks at changes in input pins and
    // and decides which events to fire
    // with pce.produce(i);
}

void userSoftReset() {}
void userHardReset() {}

#include "OpenLCBMid.h"   // Essential - do not move or delete

// Callback from a Configuration write
// Use this to detect changes in the node's configuration
// This may be useful to take immediate action on a change.
void userConfigWritten(uint32_t address, uint16_t length, uint16_t func)
{
  Serial.printf("\nuserConfigWritten: Addr: %d", (uint32_t) address); 
  Serial.printf("  Len: %d", (uint16_t) length); 
  Serial.printf("  Func: %d", (uint8_t) func);

  // Need to do a EEPROM commit as this doesn't always appear to happen!!
  EEPROM.commit();

  // Find the servo and position which has been changed.
  for (int s=0; s<NUM_SERVOS; s++) {
    for (int p=0; p<NUM_POS; p++) {
      if (address == EEADDR(servos[s].pos[p].pos)) {

        // Find the new angle for this servo's position.
        uint8_t servoPosDegrees = NODECONFIG.read(EEADDR(servos[s].pos[p].pos)); 

        // Change the servo's angle to match the change.
        // Causes the servo to move as the user adjusts the configuration data in JMRI.
        servoStateMachine.servoArray.servo[s].servoEasing.setTargetAngle(servoPosDegrees);

        // Update the state machine.
        servoStateMachine.servoArray.servo[s].position[p].positionDegrees = servoPosDegrees;

        break;
      }
    }
  }
}

// ==== Setup does initial configuration ======================
void setup() { 
  Serial.begin(115200);
  //while (!Serial);
  delay(1000);
  Serial.print("\nStarted");

  // Turn off the Pico's LED to indicate that initialisation has not completed.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Set all output pins to outputs.
  for (uint8_t i = 0; i < NUM_OUTPUTS; i++) {
    pinMode(outputPin[i], OUTPUT);
    digitalWrite(outputPin[i], LOW);
  }

  NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
  Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);

  /*
  * Initialise the ServoArray in the ServoStateMachine class.
  */
  ServoArray servoArray; // Temporary variable used to initialise the copy in ServoStateMachine.
  for(int i=0; i<NUM_SERVOS; i++) {
    servoArray.servo[i].currentState = UNKNOWN;
    servoArray.servo[i].speedDegreesPerSecond = SERVO_SPEED;
    servoArray.servo[i].servoEasing.setServoPin(servoPin[i]);

    for (int j=0; j<NUM_POS; j++) {
      for (int k=0; k<DESCRIPTION_LENGTH; k++) {
        servoArray.servo[i].position[j].description[k] = NODECONFIG.read(EEADDR(servos[i].pos[j].positiondesc[k]));
      }
      servoArray.servo[i].position[j].positionDegrees = NODECONFIG.read(EEADDR(servos[i].pos[j].pos)); 
      servoArray.servo[i].position[j].eventIndexPositionReached = (i * 9) + (j * 3) + 1;
      servoArray.servo[i].position[j].eventIndexPositionLeaving = (i * 9) + (j * 3) + 2;

      // Send the leaving event for all positions of all servos to initialise the JMRI sensor objects.
      OpenLcb.produce(servoArray.servo[i].position[j].eventIndexPositionLeaving);
    }
  }
  servoStateMachine.initialise(servoArray);

  // Turn on the Pico's LED to indicate initialisation has completed.
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.printf("\n initialization finished");
}

// ==== Loop ==========================
void loop() {
  // Variables to control how often a check is made that the LCC hub is still connected.
  static unsigned long previousMillis = 0;
  const long interval = 10000; // Check every 10 seconds.

  //MDNS.update();  // IS THIS NEEDED?
  bool activity = Olcb_process();
  
  // Make an occasional check to see if we still have a connection to the LCC hub.
  // Not very reliable as this code is not called when the library is looping around attempting to connect to the LCC hub.
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    static int n = 0;
    n = MDNS.queryService(openLCB_can, "tcp");
    //Serial.printf("\nn=%d", n);
    //if (MDNS.queryService(openLCB_can, "tcp") == 0) {
      // There are no services, so turn the LED off.
      digitalWrite(LED_BUILTIN, n);
    //} else {
      //digitalWrite(LED_BUILTIN, HIGH);
    //}
  }

  //produceFromInputs();  // process inputs not needed

  // Move servos if needed and update their states.
  int eventToSend;
  for (uint8_t i=0; i<NUM_SERVOS; i++) {
    eventToSend = servoStateMachine.update(i);
    if (eventToSend != -1) {
      //Serial.printf("\nServo %d sending event 'reached position %d' (index=%d)", servoNumber+1, position+1, index); // TO DO: == need to update ===
      OpenLcb.produce(eventToSend);
    }
  }
}
