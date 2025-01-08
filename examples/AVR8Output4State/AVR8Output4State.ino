
//==============================================================
// AVR_8outputNode using ACAN
//   A prototype of an 8-channel Output OpenLCB board
//
//   David Harris 2019, adapted from
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//   Updated 2024.09 DPH
//   Updated 2024.12 John Holmes see README for pin allocations
//==============================================================

// Allow direct to JMRI via USB, without CAN controller, comment out for CAN
// Note: disable debugging if this is chosen
#include "GCSerial.h"

// New ACan for MCP2515
#define ACAN_FREQ 8000000UL   // set for crystal freq feeding the MCP2515 chip
#define ACAN_CS_PIN 10        // set for the MCP2515 chip select pin, usually 10 on Nano
#define ACAN_INT_PIN 2        // set for the MCP2515 interrupt pin, usually 2 or 3
//#include "ACan.h"             // uses local ACan class, comment out if using GCSerial

#include <Wire.h>

#define OLCB_NO_BLUE_GOLD     // don't want Blue/Gold

//************ USER DEFINITIONS ************************************

// Node ID --- this must come from a range controlled by the user.
// See: http://registry.openlcb.org/uniqueidranges
// To set a new NODEID edit the line below.
#define NODE_ADDRESS 5,1,1,1,0x8E,0x01   // DIY range example, not for global use.

// Set to 1 to Force Reset to Factory Defaults, else 0.
// Must be done at least once for each new board.
#define RESET_TO_FACTORY_DEFAULTS 1

// Board definitions
#define MANU "OpenLCB"           // The manufacturer of node
#define MODEL "8Consumer"        // The model of the board
#define HWVERSION "0.1"          // Hardware version
#define SWVERSION "0.1"          // Software version

// Application definitions:
// For this example, set the number of channels implemented.
// Each corresponds to an output pin.
#define NUM_CHANNEL 8
// Total number of eventids, in this case there are four per channel,
// a Off, On, Blink, and fastBlink.
#define NUM_EVENT 4*NUM_CHANNEL

//************** End of USER DEFINTIONS *****************************
  
#include "mdebugging.h"           // debugging
#include "processCAN.h"           // Auto-select CAN library
#include "processor.h"            // auto-selects the processor type, and CAN lib, EEPROM lib etc.
#include "OpenLcbCore.h"
#include "OpenLCBHeader.h"        // System house-keeping.

extern "C" {                      // the following are defined as external
  #define N(x) xN(x)              // allows the insertion of value (x)
  #define xN(x) #x                // .. into the CDI string.
// ===== CDI =====
//   Configuration Description Information in xml, **must match MemStruct below**
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
//   CDIheader and CDIFooter contain system-parts, and includes user changable node name and description fields.
     const char configDefInfo[] PROGMEM =
       // vvvvv Enter User definitions below CDIheader line vvvvv
       //       It must match the Memstruct struct{} below
       CDIheader R"(
          <group>
            <name>Output</name>
            <description>Define events associated with Output Pins</description>
            <group replication=')" N(NUM_CHANNEL) R"('>
                <name>Digital Output Pins</name>
                <repname>Output</repname>
                <string size='16'><name>Description</name></string>
                <eventid><name>Set Output Off Event</name></eventid>
                <eventid><name>Set Output On Event</name></eventid>
                <eventid><name>Set Output Blink Event</name></eventid>
                <eventid><name>Set Output fastBlink Event</name></eventid>
            </group>
          </group>
       )" CDIfooter;
       // ^^^^^ Enter User definitions above CDIfooter line ^^^^^
}

// ===== MemStruct =====
//   Memory structure of EEPROM, **must match CDI above**
//     -- nodeVar has system-info, and includes the node name and description fields
    typedef struct {
          EVENT_SPACE_HEADER eventSpaceHeader; // MUST BE AT THE TOP OF STRUCT - DO NOT REMOVE!!!
          char nodeName[20];      // optional node-name, used by ACDI
          char nodeDesc[24];      // optional node-description, used by ACDI
      // vvvvv Enter User definitions below vvvvv
          struct {
            char desc[16];        // description of this output
            EventID setOff;       // Consumed eventID which sets this output-pin
            EventID setOn;        // Consumed eventID which resets this output-pin
            EventID setblink;
            EventID setfastblink;
          } outputs[NUM_CHANNEL];
      // ^^^^^ Enter User definitions above ^^^^^
    } MemStruct;                 // EEPROM memory structure, must match the CDI above

extern "C" {
  
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
  //    -- each eventid needs to be identified as a consumer, a producer or both.
  //    -- PEID = Producer-EID, CEID = Consumer, and PCEID = Producer/Consumer
  //    -- note matching references to MemStruct.
  // This line defines a useful macro to make filling the table easier
    #define REG_OUTPUT(s) CEID(outputs[s].setOff), CEID(outputs[s].setOn), CEID(outputs[s].setblink),CEID(outputs[s].setfastblink)
    const EIDTab eidtab[NUM_EVENT] PROGMEM = {
      REG_OUTPUT(0), REG_OUTPUT(1), REG_OUTPUT(2), REG_OUTPUT(3),
      REG_OUTPUT(4), REG_OUTPUT(5), REG_OUTPUT(6), REG_OUTPUT(7)
    };
 
  // SNIP Short node description for use by the Simple Node Information Protocol
  // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
    extern const char SNII_const_data[] PROGMEM = "\001" MANU "\000" MODEL "\000" HWVERSION "\000" OlcbCommonVersion ; // last zero in double-quote
    //extern const char SNII_const_data[] PROGMEM = "\001RailStars\000Io 8-Out 32-InOut 16-Servo\0001.0\0002.0" ; // last zero in double-quote
    ////extern const char SNII_const_data[] PROGMEM = "\001OpenLCB\0008Ouput\0001.0\0002.0\000"; // last zero in double-quote

}; // end extern "C"

// PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
// See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
uint8_t protocolIdentValue[6] = {     // 0xD7,0x58,0x00,0,0,0};
        pSimple | pDatagram | pMemConfig | pPCEvents | !pIdent    | pTeach     | !pStream   | !pReservation, // 1st byte
        pACDI   | pSNIP     | pCDI       | !pRemote  | !pDisplay  | !pTraction | !pFunction | !pDCC        , // 2nd byte
        0, 0, 0, 0                                                                                           // remaining 4 bytes
};


#ifndef OLCB_NO_BLUE_GOLD
  // ===== Blue/Gold =====
  // Blue-gold refers to two standard buttons offering a rudimentary control ssystem for an node.
  //   Features: teaching/learning of eventids; node identification; node reset.

  // Board choices, each has differing i/o choices
    #include "boardChoices.h"

  // This section uses the ButtonLed lib to muliplex an input and output onto a single pin.
  // It includes sampling every 32 ms, and blink patterns.

  // Patterns
  // Each pattern is 32 bits, each bit is used sequencely to blink the LED on and off, at 64 ms per bit.
    #define ShortBlinkOn   0x00010001L
    #define ShortBlinkOff  0xFFFEFFFEL
    uint32_t patterns[NUM_EVENT] = { // two per channel, one per event
      ShortBlinkOn
    };
  //ButtonLed pA(14, LOW);
  // An array of buttons/leds.
    ButtonLed* buttons[NUM_EVENT] = {
       // One for each event; each channel is a pair
       &pA
    };
#endif // OLCB_NO_BLUE_GOLD   // this app uses ButtonLed lib for its i/o.


int pins[] = { 4,5,6,7,A0,A1,A2,A3 };  // 8 outputs
// ===== Process Consumer-eventIDs =====
// USER defined

// === Blink State Variables ===
bool blinking[NUM_CHANNEL] = {false};                // Tracks which channels are blinking
unsigned long blinkStartTimes[NUM_CHANNEL] = {0};    // Start times for blinking channels
const unsigned long blinkInterval = 500;    

// === fastBlink State Variables ===
bool blinking1[NUM_CHANNEL] = {false};               // Tracks which channels are blinking
unsigned long blinkStartTimes1[NUM_CHANNEL] = {0};   // Start times for blinking channels
const unsigned long blinkInterval1 = 250;   

void pceCallback(uint16_t index) {
  int ch = index / 4;
  int ev = index % 4;

  if (ev == 0) {
    digitalWrite(pins[ch], LOW);     // Set Output Low
    blinking[ch] = false;            // Stop blinking
    blinking1[ch] = false;
  } else if (ev == 1) {
    digitalWrite(pins[ch], HIGH);    // Set Output High
    blinking[ch] = false;            // Stop blinking
    blinking1[ch] = false;
  } else if (ev == 2) {
    blinking[ch] = true;             // Start blinking
    blinking1[ch] = false;
    blinkStartTimes[ch] = millis();  // Record the start time
  } else if (ev == 3) {
    blinking1[ch] = true;            // Start blinking
    blinking[ch] = false;
    blinkStartTimes1[ch] = millis(); // Record the start time
  }
}

//bool states[] = {false, false, false, false}; // current input states; report when changed

// ===== Process inputs and Producers eventids =====
// USER defined
void produceFromInputs() {}  // no inputs, so ignore

/* Config tool:
 *  More: Reset/Reboot -- causes a reboot --> reads nid, and reads and sorts eids --> userInit();
 *  More: Update Complete -- no reboot --> reads and sorts eids --> userInit();
 *  Reset segment: Usr clear -- new set of new set of eids and blank strings (in system code), doesn't have to reboot --> userClear();
 *  Reset segment: Mfr clear -- original set of eids, blank strings --> userMfrClear().
 *
 */

// userInitAll() -- include any initialization after Factory reset "Mfg clear" or "User clear"
//  -- clear or pre-define text variables.
// USER defined
void userInitAll() {
    NODECONFIG.put(EEADDR(nodeName), ESTRING("AVR Nano"));
    NODECONFIG.put(EEADDR(nodeDesc), ESTRING("8OutputNode"));
    for(int i=0; i<NUM_CHANNEL; i++) {
      NODECONFIG.put(EEADDR(outputs[i].desc), ESTRING(""));
    }
}

// userSoftReset() - include any initialization after a soft reset, ie after configuration changes.
// USER defined
void userSoftReset() {
  REBOOT;  // defined in processor.h for each mpu
}

// userHardReset() - include any initialization after a hard reset, ie on boot-up.
// USER defined
void userHardReset() {
  REBOOT;  // defined in processor.h for each mpu
}

// ===== Callback from a Configuration write =====
// Use this to detect changes in the node's configuration
// This may be useful to take immediate action on a change.
// param address - address in space of change
// param length  - length of change
// NB: if address=0 and length==0xffff, then user indicated UPDATE_COMPLETE
//
// USER defined
void userConfigWritten(uint32_t address, uint16_t length, uint16_t func) {
}

 
#include "OpenLCBMid.h"           // System house-keeping

// Function to handle blinking
void handleBlink() {
  for (int i = 0; i < NUM_CHANNEL; i++) {
    if (blinking[i]) {
      unsigned long currentTime = millis();
      if (currentTime - blinkStartTimes[i] >= blinkInterval) {
        int currentState = digitalRead(pins[i]);    // Get the current state
        digitalWrite(pins[i], !currentState);       // Toggle the state
        blinkStartTimes[i] = currentTime;           // Reset the blink timer
      }
    }
  }
}

// Function to handle fast blinking
void handleFastBlink() {
  for (int i = 0; i < NUM_CHANNEL; i++) {
    if (blinking1[i]) {
      unsigned long currentTime = millis();
      if (currentTime - blinkStartTimes1[i] >= blinkInterval1) {
        int currentState = digitalRead(pins[i]);    // Get the current state
        digitalWrite(pins[i], !currentState);       // Toggle the state
        blinkStartTimes1[i] = currentTime;          // Reset the blink timer
      }
    }
  }
}

// ==== Setup does initial configuration =============================
void setup() {

  NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
  Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);

  // set pins to outputs
  for(int i=0; i<NUM_CHANNEL; i++) {
    pinMode( pins[i], OUTPUT);
  }
}

// ==== MAIN LOOP ===========================================
//  -- this performs system functions, such as CAN alias maintenence
void loop() {
  
  bool activity = Olcb_process();     // System processing happens here, with callbacks for app action.
  handleBlink();
  handleFastBlink();

}
