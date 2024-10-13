
//==============================================================
// AVR 8Servos
//
// Modified to allow connection via Wifi to an OpenLCB hub.  DPH 20230331
//
// Copyright 2019 Alex Shepherd and David Harris
//==============================================================

#if !defined(ARDUINO_ARCH_ESP32)
  #error "This sketch is primarily for the ESP32 series, like the Atom"
#endif
 
//#define DEBUG Serial    // uncomment to allow debug info to Serial
//#include "GCSerial.h"   // uncomment to send CAN GC messages to Serial (allows interactive or cox to JMRI)
//#include "WifiGC.h"     // uncomment to send CAN GC messages via Wifi to LCC hub, eg JMRI
//#define NOCAN           // uncomment to prevent HW CAN


// Board definitions
#define MANU "OpenLCB"  // The manufacturer of node
#define MODEL "ESP32PCA9585Servos(Wifi)"        // The model of the board
#define HWVERSION "0.1"   // Hardware version
#define SWVERSION "0.1"   // Software version

// To Reset the Node Number, Uncomment and edit the next line
// Need to do this at least once.
#define NODE_ADDRESS  2,1,13,0,0,0x47

// Set to 1 to Force Reset EEPROM to Factory Defaults
// Need to do this at least once.
#define RESET_TO_FACTORY_DEFAULTS 1

// User defs
#define NUM_SERVOS 8
#define NUM_POS 3

#define NUM_EVENT NUM_SERVOS * NUM_POS

#include "mdebugging.h"
#include "OpenLCBHeader.h"
//#include "OpenLcbCore.h" // needed for some reason
//#include "NodeMemory.h"

#define SERVO_PWM_DEG_0    120 // this is the 'minimum' pulse length count (out of 4096)
#define SERVO_PWM_DEG_180  590 // this is the 'maximum' pulse length count (out of 4096)

#include "Wire.h"
#define USE_PCA9685_SERVO_EXPANDER
#define ENABLE_EASE_CUBIC // 0x82

#ifdef DEBUG
  #undef DEBUG
  #include "ServoEasing.hpp"   // great library for gettin slow servo action, including bounces
  #define DEBUG Serial
#else
  #include "ServoEasing.hpp"   // great library for gettin slow servo action, including bounces
#endif

//uint8_t spos[NUM_POS][NUM_SERVOS];   // 0=firstPos, 1=secondPos, 2=currentPos  This is saved to EEPROM

ServoEasing* servo[NUM_SERVOS];

// CDI (Configuration Description Information) in xml, must match MemStruct
// See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
extern "C" {
const char configDefInfo[] PROGMEM =
// ===== Enter User definitions below =====
  CDIheader R"(
    <group>
        <group replication='8'>
            <name>Servos</name>
            <repname>Servo</repname>
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
          struct {
            char desc[8];        // description of this Servo Turnout Driver
            struct {
              EventID eid;       // consumer eventID
              uint8_t pos;       // position
            } pos[NUM_POS];
          } servos[NUM_SERVOS];
      // ===== Enter User definitions above =====
          uint8_t curpos[NUM_SERVOS];
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

uint8_t curpos[]  = {  0,  0,  0,  0,  0,  0,  0,  0,
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

void userInitAll() {
  //dP("\nuserInitAll()");
  NODECONFIG.put(EEADDR(nodeName), ESTRING("ESP32"));
  NODECONFIG.put(EEADDR(nodeDesc), ESTRING("PCA9685 Servos"));

  for(uint8_t i = 0; i < NUM_SERVOS; i++) {
    NODECONFIG.put(EEADDR(servos[i].desc), ESTRING("abcdefg"));
    for(int p=0; p<NUM_POS; p++) {
      NODECONFIG.write( EEADDR(servos[i].pos[p].pos), (uint8_t)(10+((p*160)/(NUM_POS-1))) );
    }
    NODECONFIG.write(EEADDR(curpos[i]), 1);
  }
}


// Set servo i's position to p= 1,2,3,4
void servoSet(uint8_t outputIndex, uint8_t outputState) {
  uint8_t servoPosDegrees = NODECONFIG.read(EEADDR(servos[outputIndex].pos[outputState].pos));
  servo[outputIndex]->easeTo(servoPosDegrees);
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
    curpos[outputIndex] = outputState;
    NODECONFIG.write( EEADDR(curpos[outputIndex]), outputState);
    servoSet(outputIndex, outputState);
}

void produceFromInputs() {
    // called from loop(), this looks at changes in input pins and
    // and decides which events to fire
    // by calling pce.produce(i);
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
  //dP(F("\nuserConfigWritten: Addr: ")); dP((uint32_t)address);
  //dP("  Len: "); dP((uint16_t)length);
  //dP("  Func: "); dP((uint8_t)func);
  for(int s=0; s<NUM_SERVOS; s++) servoSet(s, NODECONFIG.read( EEADDR(curpos[s]) ) );
}

uint8_t servopin[] = { 25, 21, 33, 23, 19, 22, 0, 0 }; // these do not matter for teh PCA9685
// ==== Setup does initial configuration ======================
void setup()
{
  #ifdef DEBUG
    Serial.begin(115200);
    while(!Serial);
    delay(1000);
    dP(F("\n " __FILE__));
  #endif

  NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
  Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);

  Wire.setPins(26, 32);  // SDA, SCL Atom=26,32
  Wire.begin();

  for(uint8_t i = 0; i < NUM_SERVOS; i++) {
      servo[i] = new ServoEasing(PCA9685_DEFAULT_ADDRESS);
      if (servo[i]->attach(i, 90) == INVALID_SERVO) { Serial.print("\nNo servo"); while(0); }
      servo[i]->setEasingType(0x82);
      servo[i]->setSpeed(50);
      curpos[i] = NODECONFIG.read( ( EEADDR(curpos[i]) ) );
      servoSet(i, curpos[i]);
  }

  dP(F("\n NUM_EVENT=")); dP(NUM_EVENT);

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
  
  //produceFromInputs();  // Not needed in this sketch

}
