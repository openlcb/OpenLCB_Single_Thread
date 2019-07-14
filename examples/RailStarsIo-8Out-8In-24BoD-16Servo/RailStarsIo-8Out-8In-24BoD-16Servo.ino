//==============================================================
// RailStars Upgraded 8-Outputs 8-Inputs 24-BoDs 16-Servos 
// 
// Copyright 2019 Alex Shepherd and David Harris
//==============================================================
#if !defined(ARDUINO_AVR_RS_IO)
  #error "Not an Io / AT90CAN"
#endif

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// Board definitions
#define MANU "RailStars"  // The manufacturer of node
#define MODEL "Io"        // The model of the board
#define HWVERSION "1.0"   // Hardware version
#define SWVERSION "2.0"   // Software version

// To Reset the RailStars Io Node Number, Uncomment and edit the next line
//#define RESET_NODE_ADDRESS  0x24

// User defs
#define NUM_OUTPUTS     8
#define NUM_INPUTS      8
#define NUM_BOD_INPUTS 24
#define NUM_SERVOS     16

#define FIRST_OUTPUT_EVENT_INDEX 0
#define FIRST_INPUT_EVENT_INDEX  (NUM_OUTPUTS*2)
#define FIRST_BOD_EVENT_INDEX    (FIRST_INPUT_EVENT_INDEX + NUM_INPUTS*2)
#define FIRST_SERVO_EVENT_INDEX  (FIRST_BOD_EVENT_INDEX + NUM_BOD_INPUTS*2)

#define NUM_EVENT  ((NUM_OUTPUTS*2) + (NUM_INPUTS * 2) + (NUM_BOD_INPUTS * 2) + (NUM_SERVOS * 2))

#include "OpenLCBHeader.h"

#define ENABLE_DEBUG_PRINT
#ifdef ENABLE_DEBUG_PRINT
  #define DEBUG_BAUD_RATE 115200

  #define DEBUG(x) Serial.print(x)
  #define DEBUGL(x) Serial.println(x);
  #define DEBUGHEX(x,y) Serial.print(x,y);
#else
  #define DEBUG(x)
  #define DEBUGL(x)
  #define DEBUGHEX(x,y)
#endif

#define SERVO_PWM_DEG_0    120 // this is the 'minimum' pulse length count (out of 4096)
#define SERVO_PWM_DEG_180  590 // this is the 'maximum' pulse length count (out of 4096)

#define SERVO_POS_DEG_THROWN  60
#define SERVO_POS_DEG_CLOSED  120

 // CDI (Configuration Description Information) in xml, must match MemStruct
 // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
    extern "C" { 
        const char configDefInfo[] PROGMEM = 
            // ===== Enter User definitions below =====
            CDIheader R"(
                <group>
                  <name>I/O</name>
                  <description>Define events associated with input and output pins</description>
                  <group replication='8'>
                      <name>Digital Outputs</name>
                      <repname>Output</repname>
                      <string size='16'><name>Description</name></string>
                      <eventid><name>Set Output Low Event</name></eventid>
                      <eventid><name>Set Output High Event</name></eventid>
                  </group>
                  <group replication='8'>
                      <name>Digital Inputs</name>
                      <repname>Input</repname>
                      <string size='16'><name>Description</name></string>
                      <eventid><name>Input Low Event</name></eventid>
                      <eventid><name>Input High Event</name></eventid>
                  </group>
                  <group replication='24'>
                      <name>Block Occupancy Detector Inputs</name>
                      <repname>Block</repname>
                      <string size='16'><name>Description</name></string>
                      <eventid><name>Block Empty Event</name></eventid>
                      <eventid><name>Block Occupied Event</name></eventid>
                  </group>
                  <group>
                      <name>Turnout Servo PWM Calibration</name>
                      <int size='2'>
                          <min>0</min>
                          <max>4095</max>
                          <default>120</default>
                          <name>Servo PWM Min</name>
                          <description>PWM Value for Servo 0 Degree Position</description>
                      </int>
                      <int size='2'>
                          <min>0</min>
                          <max>4095</max>
                          <default>590</default>
                          <name>Servo PWM Max</name>
                          <description>PWM Value for Servo 180 Degree Position</description>
                      </int>
                   </group>
                   <group replication='16'>
                      <name>Turnout Servo Control</name>
                      <repname>Servo</repname>
                      <string size='16'><name>Description</name></string>
                      <eventid><name>Servo Thrown Event</name></eventid>
                      <int size='1'>
                          <min>0</min>
                          <max>180</max>
                          <default>60</default>
                          <name>Servo Thrown Position</name>
                          <description>Position in Degrees (0-180)</description>
                      </int>
                      <eventid><name>Servo Closed Event</name></eventid>
                      <int size='1'>
                          <min>0</min>
                          <max>180</max>
                          <default>115</default>
                          <name>Servo Closed Position</name>
                          <description>Position in Degrees (0-180)</description>
                      </int>
                  </group>
                </group>
            )" CDIfooter;
          // ===== Enter User definitions above =====
    } // end extern

// ===== MemStruct =====
//   Memory structure of EEPROM, must match CDI above
    typedef struct { 
      NodeVar nodeVar;         // must remain
      // ===== Enter User definitions below =====
          struct {
            char desc[16];        // description of this output
            EventID setLow;       // Consumed eventID which sets this output-pin
            EventID setHigh;      // Consumed eventID which resets this output-pin
          } digitalOutputs[NUM_OUTPUTS];
          struct {
            char desc[16];        // description of this input-pin
            EventID inputLow;     // eventID which is Produced on activation of this input-pin 
            EventID inputHigh;    // eventID which is Produced on deactivation of this input-pin
          } digitalInputs[NUM_INPUTS];
          struct {
            char desc[16];        // description of this BoD input-pin
            EventID empty;        // eventID which is Produced on Block Empty
            EventID occupied;     // eventID which is Produced on Block Occupied 
          } bodInputs[NUM_BOD_INPUTS];
          uint16_t ServoPwmMin;
          uint16_t ServoPwmMax;
          struct {
            char desc[16];        // description of this Servo Turnout Driver
            EventID thrown;       // consumer eventID which sets turnout to Diverging 
            uint8_t thrownPos;    // position of turount in Diverging
            EventID closed;       // consumer eventID which sets turnout to Main
            uint8_t closedPos;    // position of turnout in Normal
          } servoOutputs[NUM_SERVOS];
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition

#define E16BITVAL(x)  ESTRING((x>>8)|x<<8)

void userInitAll()
{
  EEPROM.put(EEADDR(ServoPwmMin), E16BITVAL(SERVO_PWM_DEG_0));
  EEPROM.put(EEADDR(ServoPwmMax), E16BITVAL(SERVO_PWM_DEG_180));

  uint8_t posThrown = SERVO_POS_DEG_THROWN;
  uint8_t posClosed = SERVO_POS_DEG_CLOSED;
  
  for(uint8_t i = 0; i < NUM_SERVOS; i++)
  {
    EEPROM.put(EEADDR(servoOutputs[i].thrownPos), posThrown);
    EEPROM.put(EEADDR(servoOutputs[i].closedPos), posClosed);
  }
}


extern "C" {
  // ===== eventid Table =====
      #define REG_OUTPUT(s)       CEID(digitalOutputs[s].setLow), CEID(digitalOutputs[s].setHigh) 
      #define REG_INPUT(s)        PEID(digitalInputs[s].inputLow), PEID(digitalInputs[s].inputHigh)  
      #define REG_BOD_INPUT(s)    PEID(bodInputs[s].empty), PEID(bodInputs[s].occupied) 
      #define REG_SERVO_OUTPUT(s) CEID(servoOutputs[s].thrown), CEID(servoOutputs[s].closed) 
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
      const EIDTab eidtab[NUM_EVENT] PROGMEM = {
         REG_OUTPUT(0), REG_OUTPUT(1), REG_OUTPUT(2), REG_OUTPUT(3), REG_OUTPUT(4), REG_OUTPUT(5), REG_OUTPUT(6), REG_OUTPUT(7),
         REG_INPUT(0), REG_INPUT(1), REG_INPUT(2), REG_INPUT(3), REG_INPUT(4), REG_INPUT(5), REG_INPUT(6), REG_INPUT(7),
         REG_BOD_INPUT(0), REG_BOD_INPUT(1), REG_BOD_INPUT(2), REG_BOD_INPUT(3), REG_BOD_INPUT(4), REG_BOD_INPUT(5), REG_BOD_INPUT(6), REG_BOD_INPUT(7),
         REG_BOD_INPUT(8), REG_BOD_INPUT(9), REG_BOD_INPUT(10), REG_BOD_INPUT(11), REG_BOD_INPUT(12), REG_BOD_INPUT(13), REG_BOD_INPUT(14), REG_BOD_INPUT(15),
         REG_BOD_INPUT(16), REG_BOD_INPUT(17), REG_BOD_INPUT(18), REG_BOD_INPUT(19), REG_BOD_INPUT(20), REG_BOD_INPUT(21), REG_BOD_INPUT(22), REG_BOD_INPUT(23),
         REG_SERVO_OUTPUT(0), REG_SERVO_OUTPUT(1), REG_SERVO_OUTPUT(2), REG_SERVO_OUTPUT(3), REG_SERVO_OUTPUT(4), REG_SERVO_OUTPUT(5), REG_SERVO_OUTPUT(6), REG_SERVO_OUTPUT(7), 
         REG_SERVO_OUTPUT(8), REG_SERVO_OUTPUT(9), REG_SERVO_OUTPUT(10), REG_SERVO_OUTPUT(11), REG_SERVO_OUTPUT(12), REG_SERVO_OUTPUT(13), REG_SERVO_OUTPUT(14), REG_SERVO_OUTPUT(15) 
      };
      
 // SNIP Short node description for use by the Simple Node Information Protocol 
 // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
    extern const char SNII_const_data[] PROGMEM = "\001RailStars\000Io 8-Out 8-In 24-BoD 16-Servo\0001.0\0002.0" ; // last zero in double-quote

} // end extern "C"

// PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
// See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
uint8_t protocolIdentValue[6] = {0xD7,0x58,0x00,0,0,0};
      // PIP, Datagram, MemConfig, P/C, ident, teach/learn, 
      // ACDI, SNIP, CDI

      /* whole set: 
       *  Simple, Datagram, Stream, MemConfig, Reservation, Events, Ident, Teach
       *  Remote, ACDI, Display, SNIP, CDI, Traction, Function, DCC
       *  SimpleTrain, FuncConfig, FirmwareUpgrade, FirwareUpdateActive,
       *  ... additional ones may be added
       */

#define OLCB_NO_BLUE_GOLD

const uint8_t outputPinNums[] = { 0,  1,  2,  3,  4,  5,  6,  7};

const uint8_t inputPinNums[]  = { 8,  9, 10, 11, 12, 13, 14, 15};
uint8_t inputStates[]         = { 0,  0,  0,  0,  0,  0,  0,  0}; // current input states; report when changed

const uint8_t bodPinNums[]    = {16, 17, 18, 19, 20, 21, 22, 23, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};

uint8_t BoDStates[]           = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

uint8_t servoStates[]           = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);

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
      DEBUG(F("\npceCallback: Event Index: ")); DEBUGL(index);
       
      if(index < FIRST_INPUT_EVENT_INDEX)
      {
        uint8_t outputIndex = index / 2;
        uint8_t outputState = index % 2;
        DEBUG(F("Write Output: ")); DEBUG(outputIndex); DEBUG(F(" State: ")); DEBUGL(outputState);
        digitalWrite(outputPinNums[outputIndex], outputState);
      }
        
      else if ( (index >= FIRST_SERVO_EVENT_INDEX) && (index < (FIRST_SERVO_EVENT_INDEX + (NUM_SERVOS * 2) ) ) )
      {
        uint8_t outputIndex = (index - FIRST_SERVO_EVENT_INDEX) / 2;
        uint8_t outputState = (index - FIRST_SERVO_EVENT_INDEX) % 2;
        servoStates[outputIndex] = outputState;
        
        servoSet(outputIndex, outputState);
      }
   }

// Set servo i's position to p
void servoSet(uint8_t outputIndex, uint8_t outputState)
{
  uint8_t servoPosDegrees = outputState ? EEPROM.read(EEADDR(servoOutputs[outputIndex].closedPos)) : EEPROM.read(EEADDR(servoOutputs[outputIndex].thrownPos)); 
  uint16_t servoPosPWM = map(servoPosDegrees, 0, 180, servoPwmMin, servoPwmMax);
  DEBUG(F("Write Servo: ")); DEBUG(outputIndex); DEBUG(F(" Pos: ")); DEBUG(servoPosDegrees); DEBUG(F(" PWM: ")); DEBUGL(servoPosPWM);
  servoPWM.setPWM(outputIndex, 0, servoPosPWM);
}



void produceFromInputs() {
  // called from loop(), this looks at changes in input pins and 
  // and decides which events to fire
  // with pce.produce(i);
  // The first event of each pair is sent on button down,
  // and second on button up.
  // 
  // To reduce latency, only MAX_INPUT_SCAN inputs are scanned on each loop
  //    (Should not exceed the total number of inputs, nor about 4)

  static uint8_t inputsScanIndex = 0;
  static uint8_t bodScanIndex = 0;
  
  #define MAX_INPUT_SCAN 4
  for (int i = 0; i<(MAX_INPUT_SCAN); i++)
  {

//    DEBUG("produceFromInputs: "); DEBUG(inputsScanIndex); DEBUG(" - "); DEBUGL(bodScanIndex);
    
    if(inputsScanIndex < NUM_INPUTS)
    {
      uint8_t inputVal = digitalRead( inputPinNums[inputsScanIndex]);
      if(inputStates[inputsScanIndex] != inputVal)
      {
        inputStates[inputsScanIndex] = inputVal;
        DEBUG("produceFromInputs: Input: "); DEBUG(inputsScanIndex); DEBUG(" NewValue: "); DEBUGL(inputVal);

        if(inputVal)
          pce.produce(FIRST_INPUT_EVENT_INDEX + (inputsScanIndex * 2));
        else
          pce.produce(FIRST_INPUT_EVENT_INDEX + (inputsScanIndex * 2) + 1);
      }
      inputsScanIndex++;
    }
    else if(bodScanIndex < NUM_BOD_INPUTS)
    {
      // The UART0 is on pins 32 & 33 so while the Serial object is being used we need to skip checking these pins
#ifdef ENABLE_DEBUG_PRINT
      if(bodPinNums[bodScanIndex] == 32) bodScanIndex+=2;
#endif      

      uint8_t inputVal = digitalRead( bodPinNums[bodScanIndex]);
      //Serial.print(" NewValue: "); Serial.println(inputVal);
      if(BoDStates[bodScanIndex] != inputVal)
      {
        BoDStates[bodScanIndex] = inputVal;
        DEBUG("produceFromInputs: BODInput: "); DEBUG(bodScanIndex); DEBUG(" NewValue: "); DEBUGL(inputVal);

        if(inputVal)
          pce.produce(FIRST_BOD_EVENT_INDEX + (bodScanIndex * 2));
        else
          pce.produce(FIRST_BOD_EVENT_INDEX + (bodScanIndex * 2) + 1);
      }
      bodScanIndex++;
    }
    else
    {
      inputsScanIndex = 0;
      bodScanIndex = 0;
    }
  }
}

void userSoftReset() {}
void userHardReset() {}

#include "OpenLCBMid.h"

// Callback from a Configuration write
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
// 
#define BYTE_SWAP(x) x = (x >> 8) | (x << 8)

void userConfigWritten(unsigned int address, unsigned int length, unsigned int func)
{
  DEBUG("\nuserConfigWritten: Addr: "); DEBUG(address); DEBUG("  Len: "); DEBUG(length); DEBUG("  Func: "); DEBUGL(func);
  if(address == offsetof(MemStruct, ServoPwmMin) && (length >= sizeof(uint16_t)))
  {
    EEPROM.get(EEADDR(ServoPwmMin), servoPwmMin); BYTE_SWAP(servoPwmMin);
    DEBUG("Changed: ServoPwmMin: "); DEBUGL(servoPwmMin); 
  }
  
  else if(address == offsetof(MemStruct, ServoPwmMax) && (length >= sizeof(uint16_t)))
  {
    EEPROM.get(EEADDR(ServoPwmMax), servoPwmMax); BYTE_SWAP(servoPwmMax);
    DEBUG("Changed: ServoPwmMax: "); DEBUGL(servoPwmMax);
  }

  else if(address >= offsetof(MemStruct, servoOutputs))
  {
    DEBUGL("Changed: Servo Data");

    for(uint8_t i = 0; i < NUM_SERVOS; i++)
      servoSet(i, servoStates[i]);
  }
}

void setRailStartIoNodeId(uint8_t lastByteValue)
{
  NodeID newNodeID(0x05, 0x02, 0x01, 0x02, 0x02, lastByteValue);
  nm.store(&newNodeID);
}

// ==== Setup does initial configuration ======================
void setup()
{ 
#ifdef DEBUG_BAUD_RATE
  Serial.begin(DEBUG_BAUD_RATE);DEBUGL(F("\nRailStars Io 8-Out 8-In 24-BoD 16-Servo"));
  setDebugStream(&Serial);
#endif  

  // Setup Output Pins
  for(uint8_t i = 0; i < NUM_OUTPUTS; i++)
    pinMode(outputPinNums[i], OUTPUT);

  // Setup Input Pins
  for(uint8_t i = 0; i < NUM_INPUTS; i++)
    pinMode(inputPinNums[i], INPUT_PULLUP);

  // Setup BoD Input Pins
  for(uint8_t i = 0; i < NUM_BOD_INPUTS; i++)
    pinMode(bodPinNums[i], INPUT_PULLUP);

#ifdef RESET_NODE_ADDRESS  
  setRailStartIoNodeId(RESET_NODE_ADDRESS);
#endif

// #define FORCE_ALL_INIT 1  // uncomment to force all inint of EEPROM
  Olcb_init(FORCE_ALL_INIT);

  servoPWM.begin();
  servoPWM.setPWMFreq(60);

  EEPROM.get(EEADDR(ServoPwmMin), servoPwmMin); BYTE_SWAP(servoPwmMin);
  EEPROM.get(EEADDR(ServoPwmMax), servoPwmMax); BYTE_SWAP(servoPwmMax);

  for(uint8_t i = 0; i < NUM_SERVOS; i++)
    servoSet(i, 0);
}

// ==== Loop ==========================
    void loop() {    
        bool activity = Olcb_process();
        if (activity) {
          blue.blink(0x1); // blink blue to show that the frame was received
        }
        if (olcbcanTx.active) { 
          gold.blink(0x1); // blink gold when a frame sent
          olcbcanTx.active = false;
        }
        
        // handle the status lights  
        blue.process();
        gold.process();

        produceFromInputs();
    }
    
