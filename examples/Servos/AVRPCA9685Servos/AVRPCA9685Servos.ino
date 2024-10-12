
//==============================================================
// AVR PCA9685 Servos
//
// Modified to allow connection via Wifi to an OpenLCB hub.  DPH 20230331
//
// Copyright 2019 Alex Shepherd and David Harris
//==============================================================

#if !defined(__AVR__)
  #error "This sketch is primarily for the AVR series, like the Nano"
#endif

// The following few lines provide options for how this sketch functions. 
// Normally the processor.h and processCAN.h select libraries for EEPROM 
// and HW CAN.
// However, it can also run as a standalone sketch for debugging purposes, or
// connect directly to JMRI via USB, using GCSerial.h.  These options interact, 
// so for example DEBUG cannot be used with DEBUG as the DEBUG messages would
// pollute the GC stream.  
#define DEBUG Serial  // uncomment to allow debug info to Serial
#include "GCSerial.h"   // uncomment to send CAN GC messages to Serial (allows interactive or cox to JMRI)
#define NOCAN         // uncomment to prevent HW CAN 

#include <Wire.h>
#include <Servo.h>
#define USE_PCA9685_SERVO_EXPANDER
#define MAX_EASING_SERVOS 8
#define servo ServoEasing::ServoEasingArray
#include <ServoEasing.hpp>  // great library for gettin slow servo action, including bounces

#include <string.h>
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

// Board definitions
#define MANU "OpenLCB"    // The manufacturer of node
#define MODEL "AVRnativeServo"      // The model of the board
#define HWVERSION "0.1"   // Hardware version
#define SWVERSION "0.1"   // Software version

// To Reset the Node Number, Uncomment and edit the next line
// Need to do this at least once.  
#define NODE_ADDRESS  2,1,13,0,0,0x57

// Set to 1 to Force Reset EEPROM to Factory Defaults 
// Need to do this at least once.  
#define RESET_TO_FACTORY_DEFAULTS 1

// User defs
#define NUM_SERVOS 8
#define NUM_POS 3

#define NUM_EVENT NUM_SERVOS * NUM_POS

#include "mdebugging.h"
#include "OpenLCBHeader.h"

#define SERVO_PWM_DEG_0    1000 
#define SERVO_PWM_DEG_180  2000 
uint16_t servoPwmMin = SERVO_PWM_DEG_0;
uint16_t servoPwmMax = SERVO_PWM_DEG_180;

//Servo servo[NUM_SERVOS];
//ServoEasing servo[NUM_SERVOS](PCA9685_DEFAULT_ADDRESS);
//ServoEasing servo1(PCA9685_DEFAULT_ADDRESS);
//ServoEasing servo[NUM_SERVOS] = new ServoEasing(PCA9685_DEFAULT_ADDRESS);
//ServoEasing servo[NUM_SERVOS](PCA9685_DEFAULT_ADDRESS);
//ServoEasing servo[NUM_SERVOS];


// CDI (Configuration Description Information) in xml, must match MemStruct
// See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
extern "C" {
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
                <default>400</default>
            </int>
            <int size='2'>
                <name>Servo PWM Max</name>
                <description>PWM Value for Servo 180 Degree Position</description>
                <min>0</min><max>4095</max>
                <default>2500</default>
            </int>
        </group>
        <group replication='8'>
            <name>Servos</name>
            <repname>#</repname>
            <string size='8'><name>Description</name></string>
            <group replication='3'>
                <repname>Position</repname>
                <eventid><name>EventID</name></eventid>
                <int size='1'>
                    <name>Servo Position in Degrees</name>
                    <min>0</min><max>180</max>
                    <hints><slider tickSpacing='30' immediate='yes' ></slider></hints>
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
          uint8_t curpos[NUM_SERVOS];   // This is outside the CDI, but is saved to EEPROM
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition


extern "C" {
    // ===== eventid Table =====
    #define REG_SERVO_OUTPUT(s) CEID(servos[s].pos[0].eid), CEID(servos[s].pos[1].eid), CEID(servos[s].pos[2].eid)
    
    //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
    const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        REG_SERVO_OUTPUT(0), REG_SERVO_OUTPUT(1), REG_SERVO_OUTPUT(2), REG_SERVO_OUTPUT(3), REG_SERVO_OUTPUT(4), REG_SERVO_OUTPUT(5), REG_SERVO_OUTPUT(6), REG_SERVO_OUTPUT(7),
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

uint8_t servoStates[NUM_SERVOS]  = {  0,  0,  0,  0,  0,  0,  0,  0, };

#define OLCB_NO_BLUE_GOLD  // This disables blue/gold.  
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


//Adafruit_PWMServoDriver servoPWM = Adafruit_PWMServoDriver();

void userInitAll() {
  NODECONFIG.put(EEADDR(nodeName), ESTRING( "AVRnat") );
  NODECONFIG.put(EEADDR(nodeDesc), ESTRING( "123" ) );
  
  NODECONFIG.put(EEADDR(ServoPwmMin), servoPwmMin);
  NODECONFIG.put(EEADDR(ServoPwmMax), servoPwmMax);

  for(uint8_t i = 0; i < NUM_SERVOS; i++) {
    NODECONFIG.put(EEADDR(curpos[i]), 0);
    NODECONFIG.put(EEADDR(servos[i].desc), ESTRING(""));
    for(int p=0; p<NUM_POS; p++) {
      NODECONFIG.put(EEADDR(servos[i].pos[p].pos), (uint8_t)((p*180)/(NUM_POS-1)));
    }
  }
}

// ===== Process Consumer-eventIDs =====
void pceCallback(uint16_t index) {
// Invoked when an event is consumed; drive pins as needed
// from index of all events.
// Sample code uses inverse of low bit of pattern to drive pin all on or all off.
// The pattern is mostly one way, blinking the other, hence inverse.
//
  dP(F("\npceCallback: Event Index: ")); dP((uint16_t)index);
    uint8_t outputIndex = index / 3;
    uint8_t outputState = index % 3;
    NODECONFIG.write(EEADDR(curpos[outputIndex]), outputState);
    servoSet(outputIndex, outputState);
}

// Set servo i's position to pos 1,2, or 3. 
void servoSet(uint8_t outputIndex, uint8_t outputState) {
  uint8_t servoPosDegrees = NODECONFIG.read(EEADDR(servos[outputIndex].pos[outputState].pos)); 
  servoPwmMin = NODECONFIG.read16(EEADDR(ServoPwmMin));
  servoPwmMax = NODECONFIG.read16(EEADDR(ServoPwmMax));
  uint16_t servoPosPWM = map(servoPosDegrees, 0, 180, servoPwmMin, servoPwmMax);
  dP(F("\nWrite Servo: ")); dP((uint16_t)outputIndex+1); 
  dP(F(" Pos: ")); dP((uint16_t)servoPosDegrees);  dP("\n");
  servo[outputIndex]->startEaseTo(servoPosDegrees);
}

void produceFromInputs() {
    // called from loop(), this should detect changes in input pins and
    // and decides which events to fire by calling pce.produce(i);
    // Since this node only has outputs, this is null.   
}

void userSoftReset() {}  // Hook for specific actions on a soft reset.
void userHardReset() {}  // Hook for specific actions an a hard reset.

#include "OpenLCBMid.h"  // This mandated, do not remove. 

// Callback from a Configuration write
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
void userConfigWritten(uint32_t address, uint16_t length, uint16_t func)
{
  dP(F("\nuserConfigWritten: Addr: ")); dP((uint32_t)address);
  dP("  Len: "); dP((uint16_t)length); 
  dP("  Func: "); dP((uint8_t)func);
  // The simplest way to make sure any changes are reflect in the servos 
  // is to just update all of them.
  for(int i=0; i<NUM_SERVOS; i++) {
    servoSet(i, NODECONFIG.read( EEADDR(curpos[i]) ) );
  }
}

// Servo output drive pins
// Choose appropriate pins for servo drive
uint8_t servopin[NUM_SERVOS] = { 2, 3, 4, 5, 6, 7, 8, 9 };  

// ==== Setup does initial configuration ======================
void setup() {   
  #ifdef DEBUG
    Serial.begin(115200);
    while(!Serial);
  #endif  

  dP(F("\n File: ")); dP(__FILENAME__); 
  #if 0
  dP(F("\n BType: ")); dP(BTYPE);
  dP(F("\n Manufacturer: ")); dP(MANU);
  dP(F("\n Model: ")); dP(MODEL);
  dP(F("\n HW: ")); dP(HWVERSION);
  dP(F("\n SW: ")); dP(SWVERSION);
  #endif

  NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
  Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);

  //servo.begin();
  //servoPWM.setPWMFreq(60);
  Wire.begin();
    //dP("\nD"); while(1);
  for(uint8_t i = 0; i < NUM_SERVOS; i++) {
    ServoEasing* p = new ServoEasing(PCA9685_DEFAULT_ADDRESS);
        //dP("\nE"); while(1);
    if( p->attach(i) == INVALID_SERVO ) dP("\nError failed to attach");
    dP("\nC"); while(1);
    //servo[i].setPeriodHertz(50);
    //servo[i].attach(servopin[i], 1000, 2000);
    servo[i]->setEasingType(72);    // easing
    servo[i]->setSpeed(50);
    dP("\nB"); while(1);
    servoSet(i, NODECONFIG.read( EEADDR(curpos[i]) ) );
  }
  dP("\nA"); while(1);

  #if 0
  dP(F("\n NUM_EVENT=")); dP(NUM_EVENT);
  for(int i=0; i<sizeof(MemStruct); i+=16) {
    dP("\n"); dPH(i); 
    for(int j=0; j<16; j++) {
      uint8_t x = EEPROM.read(i+j);
      dP(" "); if(x<16)dP(0); dPH(x);
    }
    for(int j=0; j<16; j++) {
      uint8_t x = EEPROM.read(i+j);
      if(x>=32&&x<126) dP((char)x);
      else dP(".");
    }
  }
  #endif

}

// ==== Loop ==========================
void loop() {
  
  bool activity = Olcb_process();
  static long nextdot = 0;
  if(millis()>nextdot) {
    nextdot = millis()+2000;
    //dP("\n.");
  }
  
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
