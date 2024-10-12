// TODO: detach and reattach to stop pulses to a servo, and save juice. 
//  Have a servoProcess() that loops through each servom and if !isMoving() detaches the servo.  
//  Before each move reattach the servo.  

// CHOOSE ATOM ESP32
//==============================================================
// Esp32_8ServoUnitWifiGC
// Uses M5Stack 8Servo-Unit
//
// Modified from AVR 8Servos
//
// Modified to connect via Wifi to an OpenLCB hub.  DPH 20230331
//
// Copyright 2019 Alex Shepherd and David Harris
//==============================================================

/*==============================================================
Implements 8 servo drives using an M5Stack 8Servo-Unit
This connects via I2C and contains a STM32F030 processor

 8 Servos
 - each servo has 4 positions
 - movement up and down can be assigned easing profiles

 Caution: this uses NV memory for EEPROM, make sure the partition is big enough:
   Default 4M with Spiffs works for this sketch.  
*///==============================================================

/*================================================================

Instructions: 
The servo position default to 0, 83, 166, and 250.
(1) Adjust 'servomin' and 'servomax' to give the maximum range that you want all
    servos to be able to achieve.  Normally, one would ajust these to move the
    servos to there maimmum positions *without* binding.  
(2) Decide how many positions you wan to use, for a servo this might be 
    positions 1 and 4, for a semaphore it might be postions 1,2, ans 3. 
(3) Choose the speed of travel, 50 is the default and is a middle of the
    road value. 1 is very slow, and 250 quite fast, but not extreme.  
(4) Choose the movemet profile for moving up the positions, eg 1->2->3->4, and 
    a profile for moving down the positions, eg 4->3->2->1.  Bounce and 
    double-pull are designed for semaphore blade movements.  Typically for
    turnouts one would choose 'easing' as this has a smooth start and end to
    turnout blade movement.  
(5) Have fun.  

*///==============================================================



#define ATOM
//#define NANOC6

#if !defined(ATOM) && !defined(NANOC6)
  #error "This sketch is primarily for the M5 ESP32 series, like the Atom"
#endif

//// Debugging -- uncomment to activate debugging statements:
    // dP(x) prints x, dPH(x) prints x in hex, 
    //  dPS(string,x) prints string and x
#define DEBUG Serial

//// Allow direct to JMRI via USB, without CAN controller, comment out for CAN
//   Note: disable debugging AND WifiGC if this is chosen
//#include "GCSerial.h"  

// To use Wifi: You need to connect to the node's AP at "WifiGCAP" 
//   and it should auto-open a window.  If not go to 192.168.4.1
//   (1) Use a ESP32  
//   (2) Comment out the GCSerial.h line
//   (4) You can uncomment the DEBUG line if your want.  
//   (5) Uncomment the following 2 lines  
//const char* openLCB_can  = "openlcb-can";  // <-- uncomment and change this, if necessary
#include "WifiGC.h"

// Uncomment the next line if either of the above two options above are chosen
#define NOCAN

#include <Wire.h>
#include "servo.h"
#include <ServoEasing.hpp>  // great library for gettin slow servo action, including bounces

// Board definitions
#define MANU "OpenLCB"  // The manufacturer of node
#define MODEL "ESP32ServosServoUnit"        // The model of the board
#define HWVERSION "0.1"   // Hardware version
#define SWVERSION "0.1"   // Software version

// To Reset the Node Number, edit  the next line
#define NODE_ADDRESS  2,1,13,0,0,0x4F

// Set to 1 to Force Reset EEPROM to Factory Defaults, else 0 
// Need to do this at least once.  
#define RESET_TO_FACTORY_DEFAULTS 1

// User defs
#define NUM_SERVOS 8
#define NUM_POS 4
#define MAXANGLE 250

#define NUM_EVENT NUM_SERVOS * NUM_POS

#include "mdebugging.h"           // debugging
#include "OpenLCBHeader.h"        // System house-keeping.

#define SERVO_PWM_MIN  1000 // this is the 'minimum' servo pulse
#define SERVO_PWM_MAX  2400 // this is the 'maximum' servo pulse

//Servo servo[NUM_SERVOS];
uint8_t spos[NUM_POS][NUM_SERVOS];   // 0=firstPos, 1=secondPos, 2=currentPos  This is saved to EEPROM
//ServoEasing servo[NUM_SERVOS](PCA9685_DEFAULT_ADDRESS);
ServoEasing servo[NUM_SERVOS];

  // user defined easing fnc - this implements a rope-pull pause
  #define ENABLE_EASE_USER
  float mUserEaseInFunction(float aFactorOfTimeCompletion, void* aUserDataPointer) {
    if(aFactorOfTimeCompletion<0.45) return aFactorOfTimeCompletion*1.25;
    if(aFactorOfTimeCompletion<0.55) return 0.55 - (aFactorOfTimeCompletion-0.45)*1;
    return 0.45 + (aFactorOfTimeCompletion-0.6)*1.25;
  }


// CDI (Configuration Description Information) in xml, must match MemStruct
// See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
extern "C" {
    #define N(x) xN(x)     // allow the insertion of the value (x) ..
    #define xN(x) #x       // .. into the CDI string. 
const char configDefInfo[] PROGMEM =
// ===== Enter User definitions below =====
  CDIheader R"(
    <group>
        <name>SERVOS</name>
        <group>
            <name>Servo PWM Calibration, CAUTION: changes will move all servos</name>
            <int size='2'>
                <name>Servo PWM Min</name>
                <description>PWM Value for Servo 0 Position</description>
                <min>400</min><max>3500</max>
                <default>800</default>
                <hints><slider tickSpacing='500' immediate='yes' ></slider></hints>
            </int>
            <int size='2'>
                <name>Servo PWM Max</name>
                <description>PWM Value for Servo 250 Position</description>
                <min>400</min><max>3500</max>
                <default>2200</default>
                <hints><slider tickSpacing='500' immediate='yes'> </slider></hints>
            </int>
        </group>
        <group replication=')" N(NUM_SERVOS) R"('>
            <name>Individual Servo</name>
            <repname>Servo</repname>
            <string size='16'><name>Description</name></string>
            <int size='1'>
               <name>Servo speed, degrees per second (1-250)</name>
               <min>1</min><max>250</max>
               <hints><slider tickSpacing='50' immediate='yes'> </slider></hints>
            </int>
            <int size='1'>
              <name>Upwards Easing profile 1->2,3 or 2->3 (up)</name>
              <default>0</default> 
              <map>
                <relation><property>0</property><value>Linear</value></relation> 
                <relation><property>72</property><value>Ease</value></relation> 
                <relation><property>12</property><value>Bounce</value></relation> 
                <relation><property>70</property><value>Double-Pull</value></relation> 
              </map>
            </int>
            <int size='1'>
              <name>Downwards Easing profile 3->2,1 or 2->1 (Down)</name>
              <default>0</default> 
              <map>
                <relation><property>0</property><value>Linear</value></relation> 
                <relation><property>72</property><value>Ease</value></relation> 
                <relation><property>12</property><value>Bounce</value></relation> 
              </map>
            </int>
            <group replication=')" N(NUM_POS) R"('>
                <name>Servo Positions</name>
                <repname>Position</repname>
                <eventid><name>EventID</name></eventid>
                <int size='1'>
                    <name>Servo Position (0-250)</name>
                    <min>0</min><max>250</max>
                    <hints><slider tickSpacing='50' immediate='yes'> </slider></hints>
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
            char desc[16];       // description of this Servo Turnout Driver
            uint8_t sspeed;      // Servo speed, degrees per second (1-250)
            uint8_t upp;         // Easing profile 'up'
            uint8_t downp;       // Easing profile 'down'
            struct {
              EventID eid;       // consumer eventID
              uint8_t angle;     // 'angle' of this position 0-250 (MAXANGLE)
            } pos[NUM_POS];
          } servos[NUM_SERVOS];
      // ===== Enter User definitions matching the CDI above =====
      uint8_t curpos[NUM_SERVOS];
    } MemStruct;                 // type definition

void userInitAll()
{
  NODECONFIG.put(EEADDR(nodeName), ESTRING("Servos"));
  NODECONFIG.put(EEADDR(nodeDesc), ESTRING("AtomM58ServosUnit"));
  
  NODECONFIG.update16(EEADDR(ServoPwmMin), SERVO_PWM_MIN);
  NODECONFIG.update16(EEADDR(ServoPwmMax), SERVO_PWM_MAX);

  for(uint8_t i = 0; i < NUM_SERVOS; i++) {
    NODECONFIG.put(EEADDR(servos[i].desc), ESTRING(""));
    NODECONFIG.write(EEADDR(servos[i].sspeed), 50);
    NODECONFIG.write(EEADDR(servos[i].upp), 0);
    NODECONFIG.write(EEADDR(servos[i].downp), 0);
    for(int p=0; p<NUM_POS; p++) {
      NODECONFIG.write( EEADDR(servos[i].pos[p].angle) , (uint8_t)((p*MAXANGLE)/(NUM_POS-1)) );
    }
    NODECONFIG.write( EEADDR(curpos[i]), 2 );  // note, not part of the CDI
  }
}

extern "C" {
    // ===== eventid Table =====
    #define REG_SERVO_OUTPUT(s) CEID(servos[s].pos[0].eid), CEID(servos[s].pos[1].eid), CEID(servos[s].pos[2].eid), CEID(servos[s].pos[3].eid)
    
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

uint8_t servoPositions[]         = {  0,  0,  0,  0,  0,  0,  0,  0,
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

//Adafruit_PWMServoDriver servoPWM = Adafruit_PWMServoDriver();

uint16_t servoPwmMin = SERVO_PWM_MIN;
uint16_t servoPwmMax = SERVO_PWM_MAX;

// ===== Process Consumer-eventIDs =====
void pceCallback(uint16_t index) {
// Invoked when an event is consumed; drive pins as needed
// from index of all events.
// Sample code uses inverse of low bit of pattern to drive pin all on or all off.
// The pattern is mostly one way, blinking the other, hence inverse.
//
  dPS("\npceCallback: Event Index: ", (uint16_t) index);
    uint8_t outputIndex = index / NUM_POS;
    uint8_t outputPosition = index % NUM_POS;
    servoSet(outputIndex, outputPosition);                 // set the position
}

// Set servo outputIndex's position to outputIndex
void servoSet(uint8_t outputIndex, uint8_t outputPosition)
{
  uint8_t servoPosDegrees = NODECONFIG.read(EEADDR(servos[outputIndex].pos[outputPosition].angle)); 
  uint16_t servoPosPWM = map(servoPosDegrees, 0, MAXANGLE, servoPwmMin, servoPwmMax);
  dPS("\nWrite Servo: ", outputIndex+1); 
  dPS(" Pos: ", outputPosition+1); 
  dPS(" Degrees: ", servoPosDegrees); 
  dPS(" PWM: ", servoPosPWM);
  //servoPWM.setPWM(outputIndex, 0, servoPosPWM);
  uint8_t sspeed = NODECONFIG.read(EEADDR(servos[outputIndex].sspeed));
  if(sspeed<1) sspeed = 1;
  uint8_t upp = NODECONFIG.read(EEADDR(servos[outputIndex].upp));
  uint8_t downp = NODECONFIG.read(EEADDR(servos[outputIndex].downp));
  uint8_t curpos = NODECONFIG.read(EEADDR(curpos[outputIndex]));
  dPS(" upp=", upp); 
  dPS(" downp=", downp); 
  dPS(" outputPosition=", outputPosition+1); 
  dPS(" curpos=", curpos+1); 
  dP((String)"\n");
  if(outputPosition>curpos) servo[outputIndex].setEasingType(upp); 
  else servo[outputIndex].setEasingType(downp); 
  //servo[outputIndex].startEaseTo(servoPosDegrees,sspeed);
  servo[outputIndex].startEaseTo(servoPosPWM,sspeed);
  servoPositions[outputIndex] = outputPosition;          // update local copy
  NODECONFIG.write(EEADDR(curpos[outputIndex]), outputPosition); // update EEPROM curpos
  eepromDirty = true;
}



void produceFromInputs() {
    // called from loop(), this looks at changes in input pins and
    // and decides which events to fire
    // with pce.produce(i);
}

void userSoftReset() {}
void userHardReset() {}

#include "OpenLCBMid.h"

// Callback from a Configuration write
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
// 

void userConfigWritten(uint32_t address, uint16_t length, uint16_t func)
{
  dPS("\nuserConfigWritten: Addr: ", (uint16_t)address); 
  dPS("  Len: ", (uint16_t)length); 
  dPS("  Func: ", (uint16_t)func);
  for(int s=0; s<NUM_SERVOS; s++) {
    for(int p=0; p<NUM_POS; p++) {
      if( address == EEADDR(servos[s].pos[p].angle)) {
        uint8_t angle = NODECONFIG.read(address);
        //Serial.printf("\nupdate servo %d, position %d to %d", s+1, p+1, angle);
        dPS(", position ", p+1);
        dPS(" to ", angle);
        dPS(" update servo ", s+1);
        servoSet(s, p);
        //servo[s].write(angle);
      }
    }
  }
  if( address == EEADDR(ServoPwmMin) 
   || address == EEADDR(ServoPwmMax) ) {
    dP((String)"\nupdated min max, so update all servo positions");
    servoPwmMin = NODECONFIG.read16(EEADDR(ServoPwmMin));
    servoPwmMax = NODECONFIG.read16(EEADDR(ServoPwmMax));
    dPS(" servoPwmMin=", servoPwmMin); dPS(" servoPwmMax=", servoPwmMax);
    for(int s=0; s<NUM_SERVOS; s++) {
      servoSet(s, servoPositions[s]);
    }
  }
  for(int s=0; s<NUM_SERVOS; s++) {
    for(int p=0; p<NUM_POS; p++) {
      if( address == EEADDR(servos[s].pos[p].eid) ) {
        //OpenLcb.initTables(); // added to OpenLCBMid.h configwritten
        //OpenLcb.printEvents();
        //OpenLcb.printSortedEvents();
      } 
    }
  }
  eepromDirty = true;
  //nm.print(2000);
}

uint8_t servopin[] = { 1,2,3,4,5,6,7,8 };  // arbitrary


void setup() {   
  #ifdef DEBUG
    delay(1000);
    Serial.begin(115200);
    delay(1000);
    dP((String)"\n ESP32-8Servo");
  #endif  
    dPS("\n Size of MemStruct=", (uint16_t)sizeof(MemStruct));
  
    NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
    dP((String)"\nnew nodeid="); nodeid.print();
    dPS("\nRESET_TO_FACTORY_DEFAULTS=", RESET_TO_FACTORY_DEFAULTS);
    Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);
  

  dP((String)"\n initialization finished");

  servo[0].begin();
  
  servoPwmMin = NODECONFIG.read16(EEADDR(ServoPwmMin));
  servoPwmMax = NODECONFIG.read16(EEADDR(ServoPwmMax));

  for(uint8_t s = 0; s < NUM_SERVOS; s++) {
    servo[s].setEasingType(EASE_CUBIC_IN_OUT); 
    servo[s].attach(servopin[s], 1000, 2000);
    servoPositions[s] = NODECONFIG.read(EEADDR(curpos[s]));
    servoSet(s, servoPositions[s]);
    servo[s].registerUserEaseInFunction(mUserEaseInFunction, 0);
  }

  dPS("\n NUM_EVENT=", NUM_EVENT);
  //nm.print(2000);

}

// ==== Loop ==========================
void loop() {
  
  bool activity = Olcb_process();
  static long nextdot = 0;
  if(millis()>nextdot) {
    nextdot = millis()+2000;
    //dP((String)"\n.");
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
  
  produceFromInputs();

  //wifigc_process();
}
