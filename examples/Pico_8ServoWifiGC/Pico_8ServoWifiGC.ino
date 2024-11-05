
// Note: use "USB stack = Pico SDK"

//==============================================================
// Pico 8Servos Wifi
// MOdified DPH 2024
// Copyright 2019 Alex Shepherd and David Harris
//==============================================================

//==============================================================
// This sketch implentes eight servos on a PicoW
// It can use Wifi to connect to a OpenLCB hub, eg JMRI
// It uses the Servo library that uses PIOs -- max 8.  
//
// If using SW CAN, rxpin=27, txpin=28
// IF using Wifi, enable NOCANble NOCAN
// If using GCSerial, ena
//==============================================================

#define MAX_EASING_SERVOS 12
//#include <ServoEasing.hpp>  // great library for getting slow servo action, including bounces
#include <Servo.h>
//// Debugging -- uncomment to activate debugging statements:
    // dP(x) prints x, 
    // dPH(x) prints x in hex, 
    // dPS(string,x) prints string and x
#define DEBUG Serial

//// Allow direct to JMRI via USB, without CAN controller, comment out for SW CAN
//   Note: disable debugging AND WifiGC if this is chosen
//#include "GCSerial.h"  
//#define NOCAN

// To use Wifi: 
//   (1) Use a PicoW 
//   (2) Insert your Wifi network credentials below.  
//   (3) Comment out the GCSerial.h line
//   (4) You can uncomment the DEBUG line if your want.  
//   (5) Uncomment the following 5 lines  
const char* ssid     = "YourNetwork";     // <-- fill in with your network name
const char* password = "YourPassword";         // <-- fill in with youy network password
const char* openLCB_can  = "openlcb-can";  // <-- change this if necessary
//#include "PicoWifiGC.h"
//#define NOCAN

#include <Arduino.h>

// Board definitions
#define MANU "OpenLCB"  // The manufacturer of node
#define MODEL "Pico8ServosWifi" // The model of the board
#define HWVERSION "0.1"   // Hardware version
#define SWVERSION "0.1"   // Software version

// To Reset the Node Number, Uncomment and edit the next line
// Need to do this at least once.  
#define NODE_ADDRESS  2,1,13,0,0,0x46

// Set to 1 to Force Reset EEPROM to Factory Defaults 
// Need to do this at least once.  
#define RESET_TO_FACTORY_DEFAULTS 1

// User defs
#ifdef NOCAN
  #define NUM_SERVOS 8  // maximum servos IF no other code uses a PIO
#else 
  #define NUM_SERVOS 4  // With SW CAN: note that the SW CAN uses a PIO, and so limits the number of servos to 4
#endif
#define NUM_POS 3

#define NUM_EVENT NUM_SERVOS * NUM_POS

#include "mdebugging.h"           // debugging
#include "processor.h"
#include "processCAN.h"
#include "OpenLCBHeader.h"

#define SERVO_PWM_DEG_0    540 // this is the 'minimum' pulse length count (out of 4096)
#define SERVO_PWM_DEG_180  2400 // this is the 'maximum' pulse length count (out of 4096)

//ServoEasing servo[NUM_SERVOS];
Servo servo[NUM_SERVOS];

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
                <min>500</min><max>2500</max>
                <default>540</default>
                <hints><slider tickSpacing='500' immediate='yes'> </slider></hints>
            </int>
            <int size='2'>
                <name>Servo PWM Max</name>
                <description>PWM Value for Servo 180 Degree Position</description>
                <min>500</min><max>2500</max>
                <default>2400</default>
                <hints><slider tickSpacing='500' immediate='yes'> </slider></hints>
            </int>
        </group>
        <group replication=')" N(NUM_SERVOS) R"('>
            <name>Servos</name>
            <repname>1</repname>
            <string size='8'><name>Description</name></string>
            <group replication=')" N(NUM_POS) R"('>
                <name>Position</name>
                <repname>1</repname>
                <string size='8'><name>Description</name></string>
                <eventid><name>EventID</name></eventid>
                <int size='1'>
                    <name>Servo Position in Degrees</name>
                    <min>0</min><max>180</max>
                    <hints><slider tickSpacing='50' immediate='yes'> </slider></hints>
                </int>
            </group>
        </group>
    </group>
    )" CDIfooter;
// ===== Enter User definitions above =====
} // end extern
/* 
  adding speed, easing profiles, frog switch
            <int size=1>
              <name>Speed of movement</name>
              <default>1</default> 
              <map>
                <relation><property>40</property><value>Slow</value></relation> 
                <relation><property>90</property><value>Normal</value></relation> 
                <relation><property>150</property><value>Fast</value></relation> 
              </map>
            </int>
            <int size=1>
              <name>Easing profile 1->2,3 or 2->3 (up)</name>
              <default>0</default> 
              <map>
                <relation><property>0</property><value>Linear</value></relation> 
                <relation><property>72</property><value>Ease</value></relation> 
                <relation><property>75</property><value>Elastic</value></relation> 
              </map>
            </int>
            <int size=1>
              <name>Easing profile 3->2,1 or 2->1 (Down)</name>
              <default>0</default> 
              <map>
                <relation><property>0</property><value>Linear</value></relation> 
                <relation><property>72</property><value>Ease</value></relation> 
                <relation><property>12</property><value>Bounce</value></relation> 
              </map>
            </int>
            <int size=1>
              <name>Activate frog-polarity relay on: (NB only use pos1 and pos3)</name>
              <default>0</default> 
              <map>
                <relation><property>0</property><value>None</value></relation> 
                <relation><property>8</property><value>Pin 8</value></relation> 
                <relation><property>9</property><value>Pin 9</value></relation> 
                <relation><property>10</property><value>Pin 10</value></relation> 
                <relation><property>11</property><value>Pin 11</value></relation> 
                <relation><property>12</property><value>Pin 12</value></relation> 
                <relation><property>13</property><value>Pin 13</value></relation> 
                <relation><property>14</property><value>Pin 14</value></relation> 
                <relation><property>15</property><value>Pin 15</value></relation> 
              </map>
            </int>
        <group replication='8'>
            <name>Inputs</name>
            <repname>Input</repname>
            <string size='8'><name>Description</name></string>
            <int size=1>
              <name>Input is on:</name>
              <default>0</default> 
              <map>
                <relation><property>0</property><value>None</value></relation> 
                <relation><property>8</property><value>Pin 8</value></relation> 
                <relation><property>9</property><value>Pin 9</value></relation> 
                <relation><property>10</property><value>Pin 10</value></relation> 
                <relation><property>11</property><value>Pin 11</value></relation> 
                <relation><property>12</property><value>Pin 12</value></relation> 
                <relation><property>13</property><value>Pin 13</value></relation> 
                <relation><property>14</property><value>Pin 14</value></relation> 
                <relation><property>15</property><value>Pin 15</value></relation> 
              </map>
            </int>
            <eventid><name>Input high</name><description>Send this event when the input goes high</description> </eventid>
            <eventid><name>Input low</name><description>Send this event when the input goes low</description> </eventid>
        </group>
*/
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
            char servodesc[8];        // description of this Servo Turnout Driver
            struct {
              char positiondesc[8];      // description of this Servo Position
              EventID eid;       // consumer eventID
              uint8_t pos;       // position
            } pos[NUM_POS];
          } servos[NUM_SERVOS];
          uint8_t servoState[NUM_SERVOS];
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition

void userInitAll()
{
  
  NODECONFIG.put(EEADDR(nodeName), ESTRING("PicoW"));
  NODECONFIG.put(EEADDR(nodeDesc), ESTRING("8ServosWifi"));
  
  NODECONFIG.update16(EEADDR(ServoPwmMin), SERVO_PWM_DEG_0);
  NODECONFIG.update16(EEADDR(ServoPwmMax), SERVO_PWM_DEG_180);

  for(uint8_t i = 0; i < NUM_SERVOS; i++) {
    NODECONFIG.put(EEADDR(servos[i].servodesc), ESTRING(""));
    for(int p=0; p<NUM_POS; p++) {
      NODECONFIG.put(EEADDR(servos[i].pos[p].positiondesc), ESTRING(""));
      NODECONFIG.put(EEADDR(servos[i].pos[p].pos), (uint8_t)((p*180)/(NUM_POS-1)));
    }
    NODECONFIG.write( EEADDR(servoState[i]), 1);  // mid position
  }
}

extern "C" {
    // ===== eventid Table =====
    // useful macro to help fill the table
    #define REG_SERVO_OUTPUT(s) CEID(servos[s].pos[0].eid), CEID(servos[s].pos[1].eid), CEID(servos[s].pos[2].eid)
    
    //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
    const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        REG_SERVO_OUTPUT(0), REG_SERVO_OUTPUT(1), REG_SERVO_OUTPUT(2), REG_SERVO_OUTPUT(3), 
        //REG_SERVO_OUTPUT(4), REG_SERVO_OUTPUT(5), REG_SERVO_OUTPUT(6), REG_SERVO_OUTPUT(7),
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

uint16_t servoPwmMin = SERVO_PWM_DEG_0;
uint16_t servoPwmMax = SERVO_PWM_DEG_180;

// ===== Process Consumer-eventIDs =====
void pceCallback(uint16_t index) {
// Invoked when an event is consumed; drive pins as needed
// from index of all events.
// Sample code uses inverse of low bit of pattern to drive pin all on or all off.
// The pattern is mostly one way, blinking the other, hence inverse.
//
  //dPS((const char*)"\npceCallback: Event Index: ", index);
  dP("\neventid callback: index="); dP((uint16_t)index);
    uint8_t outputIndex = index / 3;
    uint8_t outputState = index % 3;
    NODECONFIG.write( EEADDR(servoState[outputIndex]), outputState);
    servoSet(outputIndex, outputState);
}

// Set servo i's position to p
void servoSet(uint8_t outputIndex, uint8_t outputState)
{
  uint8_t servoPosDegrees = NODECONFIG.read(EEADDR(servos[outputIndex].pos[outputState].pos)); 
  uint16_t servoPosPWM = map(servoPosDegrees, 0, 180, servoPwmMin, servoPwmMax);
  dPS("\nWrite Servo: ", outputIndex+1); 
  dPS(" Pos: ", outputState+1); 
  dPS(" position: ", servoPosDegrees); 
  dPS(" PWM: ", servoPosPWM);
  dP("\n");
  //servo[outputIndex].startEaseTo(servoPosDegrees);
  servo[outputIndex].write(servoPosDegrees);
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
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
void userConfigWritten(uint32_t address, uint16_t length, uint16_t func)
{
  dPS("\nuserConfigWritten: Addr: ", (uint32_t)address); 
  dPS("  Len: ", (uint16_t)length); 
  dPS("  Func: ", (uint8_t)func);
  for(int s=0; s<NUM_SERVOS; s++) {
    for(int p=0; p<NUM_POS; p++) {
      if(address == EEADDR(servos[s].pos[p].pos)) {
        servoSet(s, p);
        break;
      }
    }
  }
}

uint8_t servopin[] = { 2,3,4,5,6,7,8,10 }; // eight servo limit on Pico, pins 0&1 are for the UART

// ==== Setup does initial configuration ======================
void setup()
{   
  #ifdef DEBUG
    uint32_t stimer = millis();
    Serial.begin(115200);
    while (!Serial && (millis() - stimer < 5000));   // wait for 5 secs for USB/serial connection to be established
    dP("\n Pico-8ServoWifiGC");
    delay(1000);
  #endif  

  NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
  Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);

  dP("\n initialization finished");

  servoPwmMin = NODECONFIG.read16(EEADDR(ServoPwmMin));
  servoPwmMax = NODECONFIG.read16(EEADDR(ServoPwmMax));
  
  #if 1
  for(int i=0; i<NUM_SERVOS; i++) {                    // and setup and update the servos
    dPS("\nattach ", i); dPS(" on pin #", servopin[i]);
    //if(servo[i].attach(servopin[i], servoPwmMin, servoPwmMax)<0) dPS("***Servo failed to attach #", i);
    servo[i].attach(servopin[i]);
    //servo[i].setSpeed(90);
    //servo[i].setEasingType(EASE_QUADRATIC_IN_OUT);     // user choice, see the ServoEasing library
    //servoSet(i, NODECONFIG.read( EEADDR(servoState[i]) ) ); 
  }
  #endif

  #if 0
    ///////////////////////////////// try 4 servos, starting at 4
    // test servo 8 on pin9
    while(1) {
      for(int i=0; i<=180; i+=10) {dP("\n6 ");dP(i); servo[0].write(i); delay(200);}//servo[0].easeTo(i); delay(500); }
      for(int i=180; i>=0; i-=10) {dP("\n6 ");dP(i); servo[0].write(i); delay(200);}//servo[1].easeTo(i); delay(500); }
      //for(int i=10; i<=170; i+=30) {dP("\n7 ");dP(i); servo[1].write(i); delay(500);}//servo[0].write(i); delay(500); }
      //for(int i=170; i>=10; i-=30) {dP("\n7 ");dP(i); servo[71].write(i); delay(500);}//servo[1].write(i); delay(500); }
    }
  #endif

  dP("\n NUM_EVENT="); dP(NUM_EVENT);

}

// ==== Loop ==========================
void loop() {
  //MDNS.update();  // IS THIS NEEDED?
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
  
  //produceFromInputs();  // process inputs no needed

}
