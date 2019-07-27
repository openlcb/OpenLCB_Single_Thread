//==============================================================
// RailStars Upgraded 8-Outputs, 32 Configurable Inputs / Outputs, 16-Servos 
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

// Uncomment to Force Reset to Factory Defaults
//#define RESET_TO_FACTORY_DEFAULTS

// User defs
#define NUM_OUTPUTS     8
#define NUM_IOS        32
#define NUM_SERVOS     16

#define FIRST_OUTPUT_EVENT_INDEX 0
#define FIRST_IOS_EVENT_INDEX  (NUM_OUTPUTS * 2)
#define FIRST_SERVO_EVENT_INDEX  (FIRST_IOS_EVENT_INDEX + NUM_IOS * 2)

#define NUM_EVENT  ((NUM_OUTPUTS*2) + (NUM_IOS * 2) + (NUM_SERVOS * 2))

#include "OpenLcbCore.h"
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

#define SERVO_POS_DEG_THROWN  75
#define SERVO_POS_DEG_CLOSED  115

 // CDI (Configuration Description Information) in xml, must match MemStruct
 // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
    extern "C" { 
        const char configDefInfo[] PROGMEM = 
            // ===== Enter User definitions below =====
            CDIheader R"(
                <group>
                  <name>I/O</name>
                  <description>Define events associated with Input and Output Pins</description>
                  <group replication='8'>
                      <name>Digital Outputs</name>
                      <repname>Output</repname>
                      <string size='16'><name>Description</name></string>
                      <eventid><name>Set Output Low Event</name></eventid>
                      <eventid><name>Set Output High Event</name></eventid>
                  </group>
                  <group replication='32'>
                      <name>Configurable Digital Inputs / Outputs</name>
                      <repname>IOs</repname>
                      <string size='16'><name>Description</name></string>
                      <int size="1">
                        <name>Pin Mode</name>
                        <default>0</default>
                        <map>
                          <relation>
                            <property>0</property>
                            <value>Input</value>
                          </relation>
                          <relation>
                            <property>1</property>
                            <value>Output</value>
                          </relation>
                         </map>
                      </int> 
                      <eventid><name>Low Event</name></eventid>
                      <eventid><name>High Event</name></eventid>
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

typedef enum{
  IO_INPUT = 0,
  IO_OUTPUT = 1
} IO_MODE;

// ===== MemStruct =====
//   Memory structure of NODECONFIG (EEPROM), must match CDI above
    typedef struct {
          EVENT_SPACE_HEADER eventSpaceHeader; // MUST BE AT THE TOP OF STRUCT - DO NOT REMOVE!!!
          
          char nodeName[20];  // optional node-name, used by ACDI
          char nodeDesc[24];  // optional node-description, used by ACDI
       // ===== Enter User definitions below =====
          struct {
            char desc[16];        // description of this output
            EventID setLow;       // Consumed eventID which sets this output-pin
            EventID setHigh;      // Consumed eventID which resets this output-pin
          } digitalOutputs[NUM_OUTPUTS];
          struct {
            char desc[16];        // description of this input-pin
            IO_MODE mode;         // Pin Mode
            EventID inputLow;     // eventID which is Produced on activation of this input-pin 
            EventID inputHigh;    // eventID which is Produced on deactivation of this input-pin
          } digitalIOs[NUM_IOS];
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

extern "C" {
  // ===== eventid Table =====
      #define REG_OUTPUT(s)       CEID(digitalOutputs[s].setLow), CEID(digitalOutputs[s].setHigh) 
      #define REG_IO(s)           PEID(digitalIOs[s].inputLow), PEID(digitalIOs[s].inputHigh)  
      #define REG_SERVO_OUTPUT(s) CEID(servoOutputs[s].thrown), CEID(servoOutputs[s].closed) 
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
      const EIDTab eidtab[NUM_EVENT] PROGMEM = {
         REG_OUTPUT(0), REG_OUTPUT(1), REG_OUTPUT(2), REG_OUTPUT(3), REG_OUTPUT(4), REG_OUTPUT(5), REG_OUTPUT(6), REG_OUTPUT(7),
         REG_IO(0), REG_IO(1), REG_IO(2), REG_IO(3), REG_IO(4), REG_IO(5), REG_IO(6), REG_IO(7),
         REG_IO(8), REG_IO(9), REG_IO(10), REG_IO(11), REG_IO(12), REG_IO(13), REG_IO(14), REG_IO(15),
         REG_IO(16), REG_IO(17), REG_IO(18), REG_IO(19), REG_IO(20), REG_IO(21), REG_IO(22), REG_IO(23),
         REG_IO(24), REG_IO(25), REG_IO(26), REG_IO(27), REG_IO(28), REG_IO(29), REG_IO(30), REG_IO(31),
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

#include "OpenLCBMid.h"

const uint8_t outputPinNums[] = { 0,  1,  2,  3,  4,  5,  6,  7};

const uint8_t ioPinNums[]  =  { 8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47};
IO_MODE       ioPinModes[NUM_IOS];
uint8_t       ioPinStates[NUM_IOS];
uint8_t       servoStates[NUM_SERVOS];

ButtonLed blue(BLUE, LOW);
ButtonLed gold(GOLD, LOW);

void userInitAll()
{
  NODECONFIG.update16(EEADDR(ServoPwmMin), SERVO_PWM_DEG_0);
  NODECONFIG.update16(EEADDR(ServoPwmMax), SERVO_PWM_DEG_180);

  uint8_t posThrown = SERVO_POS_DEG_THROWN;
  uint8_t posClosed = SERVO_POS_DEG_CLOSED;
  
  for(uint8_t i = 0; i < NUM_SERVOS; i++)
  {
    NODECONFIG.update(EEADDR(servoOutputs[i].thrownPos), posThrown);
    NODECONFIG.update(EEADDR(servoOutputs[i].closedPos), posClosed);
  }
}



Adafruit_PWMServoDriver servoPWM = Adafruit_PWMServoDriver();

uint16_t servoPwmMin = SERVO_PWM_DEG_0;
uint16_t servoPwmMax = SERVO_PWM_DEG_180;

// Set servo i's position to p
void servoSet(uint8_t outputIndex, uint8_t outputState)
{
  uint8_t servoPosDegrees = outputState ? NODECONFIG.read(EEADDR(servoOutputs[outputIndex].closedPos)) : NODECONFIG.read(EEADDR(servoOutputs[outputIndex].thrownPos)); 
  uint16_t servoPosPWM = map(servoPosDegrees, 0, 180, servoPwmMin, servoPwmMax);
  DEBUG(F("Write Servo: ")); DEBUG(outputIndex); DEBUG(F(" Pos: ")); DEBUG(servoPosDegrees); DEBUG(F(" PWM: ")); DEBUGL(servoPosPWM);
  servoPWM.setPWM(outputIndex, 0, servoPosPWM);
}


// ===== Process Consumer-eventIDs =====
void pceCallback(unsigned int index)
{
  // Invoked when an event is consumed; drive pins as needed
  // from index of all events.  
  // Sample code uses inverse of low bit of pattern to drive pin all on or all off.  
  // The pattern is mostly one way, blinking the other, hence inverse.
  //
  DEBUG(F("\npceCallback: Event Index: ")); DEBUGL(index);
   
  if(index < FIRST_IOS_EVENT_INDEX)
  {
    uint8_t outputIndex = index / 2;
    uint8_t outputState = index % 2;
    DEBUG(F("Write Output: ")); DEBUG(outputIndex); DEBUG(F(" State: ")); DEBUGL(outputState);
    digitalWrite(outputPinNums[outputIndex], outputState);
  }
  
  else if ( (index >= FIRST_IOS_EVENT_INDEX) && (index < (FIRST_IOS_EVENT_INDEX + (NUM_IOS * 2) ) ) )
  {
    uint8_t ioIndex = (index - FIRST_IOS_EVENT_INDEX) / 2;
    uint8_t ioState = (index - FIRST_IOS_EVENT_INDEX) % 2;
    if(ioPinModes[ioIndex] == IO_OUTPUT)
    {
      uint8_t ioPin = ioPinNums[ioIndex];
      DEBUG(F("Write I/O Index: ")); DEBUG(ioIndex); DEBUG(F(" Pin: ")); DEBUG(ioPin); DEBUG(F(" State: ")); DEBUGL(ioState);
      digitalWrite(ioPin, ioState);
    }
  }
      
  else if ( (index >= FIRST_SERVO_EVENT_INDEX) && (index < (FIRST_SERVO_EVENT_INDEX + (NUM_SERVOS * 2) ) ) )
  {
    uint8_t servoIndex = (index - FIRST_SERVO_EVENT_INDEX) / 2;
    uint8_t servoState = (index - FIRST_SERVO_EVENT_INDEX) % 2;
    servoStates[servoIndex] = servoState;
    
    DEBUG(F("Write Servo: ")); DEBUG(servoIndex); DEBUG(F(" State: ")); DEBUGL(servoState);

    servoSet(servoIndex, servoState);
  }
}


void produceFromInputs() {
  // called from loop(), this looks at changes in input pins and 
  // and decides which events to fire
  // with OpenLcb.produce(i);
  // The first event of each pair is sent on button down,
  // and second on button up.
  // 
  // To reduce latency, only MAX_INPUT_SCAN inputs are scanned on each loop
  //    (Should not exceed the total number of inputs, nor about 4)

  static uint8_t ioScanIndex = 0;
  
  #define MAX_INPUT_SCAN 4
  for (int i = 0; i<(MAX_INPUT_SCAN); i++)
  {

//    DEBUG("produceFromInputs: "); DEBUGL(ioScanIndex);
    
    if(ioScanIndex < NUM_IOS)
    {
      if(ioPinModes[ioScanIndex] == IO_INPUT)
      {
        uint8_t inputVal = digitalRead( ioPinNums[ioScanIndex]);
        if(ioPinStates[ioScanIndex] != inputVal)
        {
          ioPinStates[ioScanIndex] = inputVal;
          DEBUG("produceFromInputs: Input: "); DEBUG(ioScanIndex); DEBUG(" NewValue: "); DEBUGL(inputVal);
  
          if(inputVal)
            OpenLcb.produce(FIRST_IOS_EVENT_INDEX + (ioScanIndex * 2));
          else
            OpenLcb.produce(FIRST_IOS_EVENT_INDEX + (ioScanIndex * 2) + 1);
        }
        ioScanIndex++;
      }
    }
    else
      ioScanIndex = 0;
  }
}

void userSoftReset() {}
void userHardReset() {}

// Callback from a Configuration write
// Use this to detect changes in the ndde's configuration
// This may be useful to take immediate action on a change.
// 

void userConfigWritten(unsigned int address, unsigned int length, unsigned int func)
{
  DEBUG("\nuserConfigWritten: Addr: "); DEBUG(address); DEBUG("  Len: "); DEBUG(length); DEBUG("  Func: "); DEBUGL(func);
  if(address == offsetof(MemStruct, ServoPwmMin) && (length >= sizeof(uint16_t)))
  {
    servoPwmMin = NODECONFIG.read16(EEADDR(ServoPwmMin));
    DEBUG("Changed: ServoPwmMin: "); DEBUGL(servoPwmMin); 
  }
  
  else if(address == offsetof(MemStruct, ServoPwmMax) && (length >= sizeof(uint16_t)))
  {
    servoPwmMax = NODECONFIG.read16(EEADDR(ServoPwmMax));
    DEBUG("Changed: ServoPwmMax: "); DEBUGL(servoPwmMax);
  }

  else if(address >= offsetof(MemStruct, servoOutputs))
  {
    DEBUGL("Changed: Servo Data");

    for(uint8_t i = 0; i < NUM_SERVOS; i++)
      servoSet(i, servoStates[i]);
  }
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

  // Setup I/O Pins
  for(uint8_t i = 0; i < NUM_IOS; i++)
  {
    ioPinModes[i] = (IO_MODE) NODECONFIG.read(EEADDR(digitalIOs[i].mode));
    if(ioPinModes[i] == IO_INPUT)
      pinMode(ioPinNums[i], INPUT_PULLUP);

    else if(ioPinModes[i] == IO_OUTPUT)
      pinMode(ioPinNums[i], OUTPUT);
  }
  
#ifdef RESET_NODE_ADDRESS
  NodeID newNodeID(0x05, 0x02, 0x01, 0x02, 0x02, RESET_NODE_ADDRESS);
  nm.changeNodeID(&newNodeID);
#endif

#ifdef RESET_TO_FACTORY_DEFAULTS  
  Olcb_init(1);
#else
  Olcb_init(0);
#endif

  servoPWM.begin();
  servoPWM.setPWMFreq(60);

  servoPwmMin = NODECONFIG.read16(EEADDR(ServoPwmMin));
  servoPwmMax = NODECONFIG.read16(EEADDR(ServoPwmMax));

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
    
