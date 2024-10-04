//==============================================================
// AVR 2Servos NIO
//
// Coprright 2024 David P Harris
// derived from work by Alex Shepherd and David Harris
// 
//==============================================================
// - 2 Servo channels, each wirh 
//     - three settable positions
//     - three set position events 
// - N input/output channels:
//     - type: None, 
//             Input, Input inverted, 
//             Input with pull-up, Input with pull-up inverted, 
//             Output, Output inverted.
//     - duration: how long the ouput is set, from 10ms - 2550ms, 9 means forever
//     - period: the period until a repeat pulse
//     - an On-event and an Off-event, produced or consumed depending on the channel type.
//
//==============================================================

// Debugging -- uncomment to activate debugging statements:
    // dP(x) prints x, dPH(x) prints x in hex,
    //  dPS(string,x) prints string and x
//#define DEBUG Serial

// Allow direct to JMRI via USB, without CAN controller, comment out for CAN
//   Note: disable debugging if this is chosen
//#include "GCSerial.h"

#include <Wire.h>

// Board definitions
#define MANU "OpenLCB"      // The manufacturer of node
#define MODEL "AVR2ServoNIO"   // The model of the board
#define HWVERSION "0.1"     // Hardware version
#define SWVERSION "0.1"     // Software version

// To set a new nodeid edit the next line
#define NODE_ADDRESS  2,1,13,0,0,0x77

// To Force Reset EEPROM to Factory Defaults set this value t0 1, else 0.
// Need to do this at least once.
#define RESET_TO_FACTORY_DEFAULTS 1

// User defs
#define NUM_SERVOS 2
#define NUM_POS 3
#define NUM_IO 8

#define NUM_EVENT NUM_SERVOS * NUM_POS + NUM_IO*2

#include "mdebugging.h"           // debugging
#include "processCAN.h"           // Auto-select CAN library
#include "processor.h"            // auto-selects the processor type, EEPROM lib etc.
#include "OpenLCBHeader.h"        // System house-keeping.


// CDI (Configuration Description Information) in xml, must match MemStruct
// See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
extern "C" {
    #define N(x) xN(x)     // allow the insertion of the value (x) ..
    #define xN(x) #x       // .. into the CDI string.
const char configDefInfo[] PROGMEM =
// ===== Enter User definitions below =====
  CDIheader R"(
    <name>Application Configuration</name>
    <group>
        <name>Turnout Servo Configuration</name>
        <int size='1'>
          <name>Speed 1-255 (delay between steps)</name>
          <hints><slider tickSpacing='50' immediate='yes'> </slider></hints>
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
                <hints><slider tickSpacing='45' immediate='yes'> </slider></hints>
            </int>
        </group>
    </group>
    <group replication=')" N(NUM_IO) R"('>
        <name>Input/Output</name>
        <repname>IO#</repname>
        <string size='8'><name>Description</name></string>
        <int size='1'>
            <name>Channel type</name>
            <map>
                <relation><property>0</property><value>None</value></relation> 
                <relation><property>1</property><value>Input</value></relation> 
                <relation><property>2</property><value>Input Inverted</value></relation> 
                <relation><property>3</property><value>Input with pull-up</value></relation>
                <relation><property>4</property><value>Input with pull-up Inverted</value></relation>
                <relation><property>5</property><value>Output</value></relation>
                <relation><property>6</property><value>Output Inverted</value></relation>
            </map>
        </int>
        <int size='1'>
            <name>On-Duration 1-255 = 100ms-25.5s, 0=steady-state</name>
            <hints><slider tickSpacing='50' immediate='yes'> </slider></hints>

        </int>
        <int size='1'>
            <name>Off-Period 1-255 = 100ms-25.5s, 0=No repeat</name>
            <hints><slider tickSpacing='50' immediate='yes'> </slider></hints>
        </int>
        <eventid><name>On-Event</name></eventid>
        <eventid><name>Off-Event</name></eventid>
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
          uint8_t servodelay;
          struct {
            char desc[8];        // description of this Servo Turnout Driver
            struct {
              EventID eid;       // consumer eventID
              uint8_t angle;       // position
            } pos[NUM_POS];
          } servos[NUM_SERVOS];
          struct {
            char desc[8];
            uint8_t type;
            uint8_t duration;    // 100ms-25.5s, 0=solid
            uint8_t period;      // 100ms-25.5s, 0=no repeat
            EventID onEid;
            EventID offEid;
          } io[NUM_IO];
      // ===== Enter User definitions above =====
      // items below will be included in the EEPROM, but are not part of the CDI
      uint8_t curpos[NUM_SERVOS];  // save current positions of servos
    } MemStruct;                 // type definition


extern "C" {
    // ===== eventid Table =====
    #define REG_SERVO_OUTPUT(s) CEID(servos[s].pos[0].eid), CEID(servos[s].pos[1].eid), CEID(servos[s].pos[2].eid)
    #define REG_IO(i) PCEID(io[i].onEid), PCEID(io[i].offEid)
    
    //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
    const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        REG_SERVO_OUTPUT(0), REG_SERVO_OUTPUT(1),
        REG_IO(0), REG_IO(1), REG_IO(2), REG_IO(3), REG_IO(4), REG_IO(5), REG_IO(6), 
        REG_IO(7), //REG_IO(8), REG_IO(9), REG_IO(10), REG_IO(11), REG_IO(12), REG_IO(13), 
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
#include <Servo.h>
Servo servo[2];
uint8_t servodelay;
uint8_t servopin[NUM_SERVOS] = {A4,A5};
uint8_t servoActual[NUM_SERVOS] = { 90, 90 };
uint8_t servoTarget[NUM_SERVOS] = { 90, 90 };
uint8_t iopin[NUM_IO] = {13,5,6,9,A0,A1,A2,A3};
bool iostate[NUM_IO] = {0};
long next[NUM_IO] = {0};

// This is called to initialize the EEPROM on a new node
void userInitAll()
{ 
  NODECONFIG.put(EEADDR(nodeName), ESTRING("AVR"));
  NODECONFIG.put(EEADDR(nodeDesc), ESTRING("2ServosNIO"));
  
  NODECONFIG.put(EEADDR(servodelay), 50);
  for(uint8_t i = 0; i < NUM_SERVOS; i++) {
    NODECONFIG.put(EEADDR(servos[i].desc), ESTRING(""));
    for(int p=0; p<NUM_POS; p++) {
      NODECONFIG.put(EEADDR(servos[i].pos[p].angle), (uint8_t)((p*180)/(NUM_POS-1)));
    }
  }
  for(uint8_t i = 0; i < NUM_IO; i++) {
    NODECONFIG.put(EEADDR(io[i].desc), ESTRING(""));
    NODECONFIG.write(EEADDR(io[i].type), 0);
    NODECONFIG.write(EEADDR(io[i].duration), 0);
    NODECONFIG.write(EEADDR(io[i].period), 0);
    NODECONFIG.write(EEADDR(curpos[i]), 1);
  }  
}

// ===== Process Consumer-eventIDs =====
void pceCallback(uint16_t index) {
// Invoked when an event is consumed; drive pins as needed
// from index of all events.
// Sample code uses inverse of low bit of pattern to drive pin all on or all off.
// The pattern is mostly one way, blinking the other, hence inverse.
//
  #define PV(x) { dP(" " #x "="); dP(x); }
  dP("\npceCallback: Event Index: "); dP((uint16_t)index);
    if(index<NUM_SERVOS*NUM_POS) {
      uint8_t outputIndex = index / 3;
      uint8_t outputState = index % 3;
      NODECONFIG.write( EEADDR(curpos[outputIndex]), outputState);
      servoTarget[outputIndex] = NODECONFIG.read( EEADDR(servos[outputIndex].pos[outputState].angle) );
      //servoSet(outputIndex, outputState);
    } else {
      uint8_t n = index-NUM_SERVOS*NUM_POS;
      PV(n);
      uint8_t c= n&1;
      PV(!c);
      n = n/2;
      uint8_t type = NODECONFIG.read(EEADDR(io[n].type));
      PV(type);
      if(type==5 || type==6) {
        dP("\ndw!"); PV(iopin[n]); PV(!c);
        digitalWrite( iopin[n], !c );
        iostate[n] = !c;
        uint8_t durn = NODECONFIG.read(EEADDR(io[n].duration));
        if(!c && durn) next[n] = millis() + 100*durn; // note duration==0 means forever
        else next[n]=0;
          PV(millis()); PV(next[n]);
      }
    }
}

// === Process servos ===
void servoProcess() {
  static long last = 0;
  //if( (millis()-last) < 200 ) return;
  if( (millis()-last) < servodelay ) return;
  last = millis();
  for(int i=0; i<NUM_SERVOS; i++) {
    if(servoTarget[i] > servoActual[i] ) {
      dP("\nservo>"); PV(i); PV(servoTarget[i]); PV(servoActual[i]);
      servo[i].write(servoActual[i]++);
      /*
      if((servoTarget[i]-servoActual[i])<10) 
        servo[i].write(servoActual[i]++);
      else {
        servoActual[i] += 5;
        servo[i].write(servoActual[i]);
      }
      */
    } else if(servoTarget[i] < servoActual[i] ) {
      dP("\nservo<"); PV(i); PV(servoTarget[i]); PV(servoActual[i]);
      servo[i].write(servoActual[i]--);
      /*
      if((servoActual[i]-servoTarget[i])<10) 
        servo[i].write(servoActual[i]--);
      else {
        servoActual[i] -= 5;
        servo[i].write(servoActual[i]);   
      } 
      */
    } 
  }
}

// ==== Process Inputs ====
void produceFromInputs() {
    // called from loop(), this looks at changes in input pins and
    // and decides which events to fire
    // with pce.produce(i);
    const uint8_t base = NUM_SERVOS*NUM_POS;
    static uint8_t c = 0;
    static unsigned long last = 0;
    if((millis()-last)<(50/NUM_IO)) return;
    uint8_t t = NODECONFIG.read(EEADDR(io[c].type));
    if(t==1 || t==2) {
      bool s = digitalRead(iopin[c]);
      if(s^iostate[c]) {
        iostate[c] = s;
        OpenLcb.produce(base+c*2+s);
      }
    }
    if(++c>NUM_IO) c = 0;
}

void userSoftReset() {}
void userHardReset() {}

#include "OpenLCBMid.h"    // Essential, do not move or delete

// Callback from a Configuration write
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
void userConfigWritten(uint32_t address, uint16_t length, uint16_t func)
{
  dPS("\nuserConfigWritten: Addr: ", (uint32_t)address);
  dPS(" Len: ", (uint16_t)length);
  dPS(" Func: ", (uint8_t)func);
  setupPins();
  servoSetup();
}

// reinitialize servos to their current positions
void servoSetup() {
  servodelay = NODECONFIG.read( EEADDR(servodelay));
  for(uint8_t i = 0; i < NUM_SERVOS; i++) {
    uint8_t cpos = NODECONFIG.read( EEADDR(curpos[i]) );
    servoTarget[i] = NODECONFIG.read( EEADDR(servos[i].pos[cpos].angle) );
  }
}

// ==== Setup does initial configuration ======================
void setup()
{
  #ifdef DEBUG
    delay(1000);
    Serial.begin(115200);
    delay(1000);
    dP("\n AVR-2Servo14IO");
  #endif

  NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
  Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);

  // attach and put servos to last known position
  for(uint8_t i = 0; i < NUM_SERVOS; i++) 
    servo[i].attach(servopin[i]);
  servoSetup();
  setupPins();
  dP("\n NUM_EVENT="); dP(NUM_EVENT);

  // for testing
  NODECONFIG.write( EEADDR(io[0].type), 5);      // output
  NODECONFIG.write( EEADDR(io[0].duration), 5 ); // 500ms pulse
  NODECONFIG.write( EEADDR(io[0].period), 10);   // every second
  
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
  
  produceFromInputs();
  appProcess();
  servoProcess();

}
void setupPins() {
  for(uint8_t i=0; i<NUM_IO; i++) {
    uint8_t type = NODECONFIG.read( EEADDR(io[i].type));
    switch (type) {
      case 1: case 2: 
        pinMode(iopin[i], INPUT); 
        iostate[i] = type&1;
        break;
      case 3: case 4:
        pinMode(iopin[i], INPUT_PULLUP); 
        iostate[i] = type&1;
        break;
      case 5: case 6:
        pinMode(iopin[i], OUTPUT); 
        iostate[i] = type&1;
        digitalWrite(iopin[i], type&i);
        break;

    }
  }
}

void appProcess() {
  uint8_t base = NUM_SERVOS * NUM_POS;
  long now = millis();
  for(int i=0; i<NUM_IO; i++) {
    uint8_t type = NODECONFIG.read(EEADDR(io[i].type));
    if(type >= 5) {
      if( next[i] && ((now-next[i]) >= 0) ) {
        dP("\nprocess:"); PV(i); PV(type); PV(now); PV(next[i]); PV(now-next[i]);
        dP(" dw "); PV(iostate[i]);
        if(iostate[i]) {
          PV(LOW);
          digitalWrite(iopin[i], LOW);
          iostate[i] = 0;
          if( NODECONFIG.read(EEADDR(io[i].period)) > 0 ) 
            next[i] = now + 100*NODECONFIG.read(EEADDR(io[i].period));
          else next[i] = 0;
            PV(next[i]);
        } else {
          PV(HIGH);
          digitalWrite(iopin[i], HIGH);
          iostate[i] = 1;
          if( NODECONFIG.read(EEADDR(io[i].duration)) > 0 )
            next[i] = now + 100*NODECONFIG.read(EEADDR(io[i].duration));
          else next[i] = 0;
            PV(next[i]);
        }
      }
    }
  }
}