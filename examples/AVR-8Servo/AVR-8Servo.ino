//==============================================================
// AVR 8Servos
//
// Copyright 2019 Alex Shepherd and David Harris
// Updated 2024.09 DPH
//==============================================================

// Debugging -- uncomment to activate debugging statements:
    // dP(x) prints x, dPH(x) prints x in hex,
    //  dPS(string,x) prints string and x
//#define DEBUG Serial

// Allow direct to JMRI via USB, without CAN controller, comment out for CAN
//   Note: disable debugging if this is chosen
//#include "GCSerial.h"

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Board definitions
#define MANU "OpenLCB"      // The manufacturer of node
#define MODEL "AVRServos"   // The model of the board
#define HWVERSION "0.1"     // Hardware version
#define SWVERSION "0.1"     // Software version

// To set a new nodeid edit the next line
#define NODE_ADDRESS  2,1,13,0,0,0x21

// To Force Reset EEPROM to Factory Defaults set this value t0 1, else 0.
// Need to do this at least once.
#define RESET_TO_FACTORY_DEFAULTS 1

// User defs
#define NUM_SERVOS 8
#define NUM_POS 3

#define NUM_EVENT NUM_SERVOS * NUM_POS

#include "mdebugging.h"           // debugging
#include "processCAN.h"           // Auto-select CAN library
#include "processor.h"            // auto-selects the processor type, EEPROM lib etc.
#include "OpenLCBHeader.h"        // System house-keeping.

#define SERVO_PWM_DEG_0    120 // this is the 'minimum' pulse length count (out of 4096)
#define SERVO_PWM_DEG_180  590 // this is the 'maximum' pulse length count (out of 4096)

// CDI (Configuration Description Information) in xml, must match MemStruct
// See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
extern "C" {
    #define N(x) xN(x)     // allow the insertion of the value (x) ..
    #define xN(x) #x       // .. into the CDI string.
const char configDefInfo[] PROGMEM =
// ===== Enter User definitions below =====
  CDIheader R"(
    <group>
        <group>
            <name>Turnout Servo PWM Calibration</name>
            <int size='2'>
                <name>Servo PWM Min</name>
                <description>PWM Value for Servo 0 Degree Position</description>
                <min>0</min><max>4095</max>
                <default>120</default>
            </int>
            <int size='2'>
                <name>Servo PWM Max</name>
                <description>PWM Value for Servo 180 Degree Position</description>
                <min>0</min><max>4095</max>
                <default>590</default>
            </int>
        </group>
        <group replication=')" N(NUM_SERVOS) R"('>
            <name>Servos</name>
            <repname>Servo</repname>
            <string size='8'><name>Description</name></string>
            <group replication=')" N(NUM_POS) R"('>
                <repname>Position</repname>
                <eventid><name>EventID</name></eventid>
                <int size='1'>
                    <name>Servo Position in Degrees</name>
                    <min>0</min><max>180</max>
                </int>
            </group>
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
          uint16_t ServoPwmMin;
          uint16_t ServoPwmMax;
          struct {
            char desc[8];        // description of this Servo Turnout Driver
            struct {
              EventID eid;       // consumer eventID
              uint8_t pos;       // position
            } pos[NUM_POS];
          } servos[NUM_SERVOS];
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition

void userInitAll()
{
  
  NODECONFIG.put(EEADDR(nodeName), ESTRING("AVR"));
  NODECONFIG.put(EEADDR(nodeDesc), ESTRING("Servos"));
  
  NODECONFIG.update16(EEADDR(ServoPwmMin), SERVO_PWM_DEG_0);
  NODECONFIG.update16(EEADDR(ServoPwmMax), SERVO_PWM_DEG_180);

  for(uint8_t i = 0; i < NUM_SERVOS; i++) {
    NODECONFIG.put(EEADDR(servos[i].desc), ESTRING(""));
    for(int p=0; p<NUM_POS; p++) {
      NODECONFIG.put(EEADDR(servos[i].pos[p].pos), (uint8_t)((p*180)/(NUM_POS-1)));
    }
  }
}

extern "C" {
    // ===== eventid Table =====
    #define REG_SERVO_OUTPUT(s) CEID(servos[s].pos[0].eid), CEID(servos[s].pos[1].eid), CEID(servos[s].pos[2].eid)
    
    //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
    const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        REG_SERVO_OUTPUT(0), REG_SERVO_OUTPUT(1), REG_SERVO_OUTPUT(2), REG_SERVO_OUTPUT(3),
        REG_SERVO_OUTPUT(4), REG_SERVO_OUTPUT(5), REG_SERVO_OUTPUT(6), REG_SERVO_OUTPUT(7),
        //REG_SERVO_OUTPUT(8), REG_SERVO_OUTPUT(9), REG_SERVO_OUTPUT(10), REG_SERVO_OUTPUT(11), REG_SERVO_OUTPUT(12), REG_SERVO_OUTPUT(13), REG_SERVO_OUTPUT(14), REG_SERVO_OUTPUT(15)
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

uint8_t servoStates[]         = {  0,  0,  0,  0,  0,  0,  0,  0,
                                   0,  0,  0,  0,  0,  0,  0,  0 };
#define OLCB_NO_BLUE_GOLD
#ifndef OLCB_NO_BLUE_GOLD
    #define BLUE 40  // built-in blue LED
    #define GOLD 39  // built-in green LED
    ButtonLed blue(BLUE, LOW);
    ButtonLed gold(GOLD, LOW);
    
    uint32_t patterns[8] = { 0x00010001L, 0xFFFEFFFEL }; // two per channel, one per event
    ButtonLed pA(13, LOW);
    ButtonLed pB(14, LOW);
    ButtonLed pC(15, LOW);
    ButtonLed pD(16, LOW);
    ButtonLed* buttons[8] = { &pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD };
#endif // OLCB_NO_BLUE_GOLD

Adafruit_PWMServoDriver servoPWM = Adafruit_PWMServoDriver();

uint16_t servoPwmMin = SERVO_PWM_DEG_0;
uint16_t servoPwmMax = SERVO_PWM_DEG_180;

// ===== Process Consumer-eventIDs =====
void pceCallback(unsigned int index) {
// Invoked when an event is consumed; drive pins as needed
// from index of all events.
// Sample code uses inverse of low bit of pattern to drive pin all on or all off.
// The pattern is mostly one way, blinking the other, hence inverse.
//
  dP("\npceCallback: Event Index: "); dP((uint16_t)index);
    uint8_t outputIndex = index / 4;
    uint8_t outputState = index % 4;
    servoStates[outputIndex] = outputState;
    servoSet(outputIndex, outputState);
}

// Set servo i's position to p
void servoSet(uint8_t outputIndex, uint8_t outputState){
  uint8_t servoPosDegrees = NODECONFIG.read(EEADDR(servos[outputIndex].pos[outputState].pos));
  uint16_t servoPosPWM = map(servoPosDegrees, 0, 180, servoPwmMin, servoPwmMax);
  dPS("\nWrite Servo: ", (uint16_t)outputIndex);
  dPS(" Pos: ", servoPosDegrees);
  dPS(" PWM: ", servoPosPWM);
  servoPWM.setPWM(outputIndex, 0, servoPosPWM);
}

void produceFromInputs() {
    // called from loop(), this looks at changes in input pins and
    // and decides which events to fire
    // with pce.produce(i);
}

void userSoftReset() {}
void userHardReset() {}

#include "OpenLCBMid.h"    // Essential, do not move or delete

// Callback from a Configuration write
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
//

void userConfigWritten(unsigned int address, unsigned int length, unsigned int func)
{
  dPS("\nuserConfigWritten: Addr: ", (uint32_t)address);
  dPS(" Len: ", (uint16_t)length);
  dPS(" Func: ", (uint8_t)func);
}

// ==== Setup does initial configuration ======================
void setup()
{
  #ifdef DEBUG
    delay(1000);
    Serial.begin(115200);
    delay(1000);
    dP("\n AVR-8Servo");
  #endif

  NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
  Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);

  servoPWM.begin();
  servoPWM.setPWMFreq(60);

  servoPwmMin = NODECONFIG.read16(EEADDR(ServoPwmMin));
  servoPwmMax = NODECONFIG.read16(EEADDR(ServoPwmMax));

  for(uint8_t i = 0; i < NUM_SERVOS; i++)
    servoSet(i, 0);

  dP("\n NUM_EVENT="); dP(NUM_EVENT);

}

// ==== Loop ==========================
void loop() {
  
  bool activity = Olcb_process();
  
  #ifndef OLCB_NO_BLUE_GOLD
    if (activity) {
      blue.blink(0x1); // blink blue to show that the frame was received
    }
    if (olcbcanTx.active) {
      gold.blink(0x1); // blink gold when a frame sent
      olcbcanTx.active = false;
    }
    // handle the status lights
    gold.process();
    blue.process();
  #endif // OLCB_NO_BLUE_GOLD
  
  //produceFromInputs();

}
