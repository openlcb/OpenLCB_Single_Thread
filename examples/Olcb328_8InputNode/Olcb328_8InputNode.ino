//==============================================================
// Olcb328_8ProducerNode
//   A prototype of a basic 8-channel Input OpenLCB board
//
//   David Harris 2019, adapted from
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//   Updated 2024.09 DPH
//==============================================================

//// To activate Debugging staements uncomment the next line:
//#define DEBUG Serial

//// Allow direct to JMRI via USB, without CAN controller, comment out for CAN
//#include "GCSerial.h"

#define OLCB_NO_BLUE_GOLD  // don't want Blue/Gold

//************ USER DEFINITIONS ************************************

// Node ID --- this must come from a range controlled by the user.
// See: http://registry.openlcb.org/uniqueidranges
//// To set a new NODEID edit the line below to force the NodeID to be written to the board
#define NODE_ADDRESS 2,1,13,0,0,0x0B   // DIY range example, not for global use.

//// Set to 1 to Force Reset to Factory Defaults, else 0 -- must do this at least once
#define RESET_TO_FACTORY_DEFAULTS 1

// Board definitions
#define MANU "OpenLCB"           // The manufacturer of node
#define MODEL "Olcb328_8InputNode"    // The model of the board
#define HWVERSION "0.1"          // Hardware version
#define SWVERSION "0.1"          // Software version

// Application definitions:
// For this example, set the number of channels implemented.
// Each corresponds to an input or output pin.
#define NUM_CHANNEL 8
// Total number of eventids, in this case there are two per channel,
//  a set and unset.
#define NUM_EVENT 2*NUM_CHANNEL

//************** End of USER DEFINTIONS *****************************
  
#include "mdebugging.h"           // debugging
#include "processCAN.h"           // auto-select CAN library
#include "processor.h"            // auto-selects the processor type, and CAN lib, EEPROM lib etc.
#include "OpenLcbCore.h"
#include "OpenLCBHeader.h"        // System house-keeping.

extern "C" {                      // the following are defined as external
    #define N(x) xN(x)     // allow the insertion of value (x) ..
    #define xN(x) #x       // .. into the CDI string.
// ===== CDI =====
//   Configuration Description Information in xml, **must match MemStruct below**
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
//   CDIheader and CDIFooter contain system-parts, and includes user changable node name and description fields.
     const char configDefInfo[] PROGMEM =
       // vvvvv Enter User definitions below CDIheader line vvvvv
       //       It must match the Memstruct struct{} below
       CDIheader R"(
          <group>
            <name>Input</name>
            <description>Define events associated with Output Pins</description>
            <group replication=')" N(NUM_CHANNEL) R"('>
                <name>Digital Input Pins</name>
                <repname>Input</repname>
                <string size='16'><name>Description</name></string>
                <eventid><name>Event sent on input Low</name></eventid>
                <eventid><name>Event sent on input High</name></eventid>
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
          char nodeName[20];  // optional node-name, used by ACDI
          char nodeDesc[24];  // optional node-description, used by ACDI
      // vvvvv Enter User definitions below vvvvv
          struct {
            char desc[16];        // description of this output
            EventID setLow;       // Consumed eventID which sets this output-pin
            EventID setHigh;      // Consumed eventID which resets this output-pin
          } inputs[NUM_CHANNEL];
      // ^^^^^ Enter User definitions above ^^^^^
    } MemStruct;                  // EEPROM memory structure, must match the CDI above

extern "C" {
  
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
  //    -- each eventid needs to be identified as a consumer, a producer or both.
  //    -- PEID = Producer-EID, CEID = Consumer, and PC = Producer/Consumer
  //    -- note matching references to MemStruct.
      #define REG_INPUT(s) PEID(inputs[s].setLow), PEID(inputs[s].setHigh)
      const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        REG_INPUT(0), REG_INPUT(1), REG_INPUT(2), REG_INPUT(3),
        REG_INPUT(4), REG_INPUT(5), REG_INPUT(6), REG_INPUT(7)
      };
 
  // SNIP Short node description for use by the Simple Node Information Protocol
  // See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.3-SimpleNodeInformation-2016-02-06.pdf
        extern const char SNII_const_data[] PROGMEM = "\001" MANU "\000" MODEL "\000" HWVERSION "\000" OlcbCommonVersion ; // last zero in double-quote

}; // end extern "C"

// PIP Protocol Identification Protocol uses a bit-field to indicate which protocols this node supports
// See 3.3.6 and 3.3.7 in http://openlcb.com/wp-content/uploads/2016/02/S-9.7.3-MessageNetwork-2016-02-06.pdf
uint8_t protocolIdentValue[6] = {   //0xD7,0x58,0x00,0,0,0};
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
    
    uint32_t patterns[] = { // two per channel, one per event
    };

// An array of buttons/leds.
    ButtonLed* buttons[] = {
       // One for each event; each channel is a pair
    };
#endif // OLCB_NO_BLUE_GOLD   // this app uses ButtonLed lib for its i/o.

// LEDuino 328 pins:
//             ARef
//             Gnd
//        (SCK)13 Used by CAN
//       (MISO)12 Used by CAN
// Reset (MOSI)11 Used by CAN
// 3.3V    (SS)10 Used by CAN
// 5V           9
// Gnd     (ICP)8 Potentially used by LN In
// Gnd
// Vin          7 Potentially used by LN Out
//        (OC0A)6
// 14(A0)   (T1)5
// 15(A1)   (T0)4
// 16(A2) (Int1)3 Potentially used by CAN interrupt
// 17(A3) (Int0)2
// 18(A4)   (Tx)1 Used by Serial
// 19(A5)   (Rx)0 Used by Serial

int pins[] = { 4,5,14,15,16,17,18,19 };  // 8 input-pins

// ===== Process Consumer-eventIDs =====
// USER defined
  void pceCallback(uint16_t index) {
    // Invoked when an event is consumed; drive pins as needed
    // from index of all events.
  }

bool states[] = {false, false, false, false, false, false, false, false }; // current input states; report when changed

// ===== Process inputs and Producers eventids =====
// USER defined
  void produceFromInputs() {
    const int DEBOUNCE_PERIOD = 50;
    static long next = 0;
    if(next>millis())  return;              // Only process every DEBOUNCE_PERIOD
    next += DEBOUNCE_PERIOD;                // debounce time
    for(int i=0; i<NUM_CHANNEL; i++) {      // Check each channel
      bool v = digitalRead( pins[i] );      //   read it
      if(v != states[i] ) {                 //   if it has changed:
        states[i] = v;                      //     remember this state
        OpenLcb.produce( i*2 + (v?1:0) );  //     produce one of the eventid pair
      }
    }
  }

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
    NODECONFIG.put(EEADDR(nodeName), ESTRING("Olcb328_8InputNode"));
    NODECONFIG.put(EEADDR(nodeDesc), ESTRING("Testing"));
    for(int i=0; i<NUM_CHANNEL; i++) {
      NODECONFIG.put(EEADDR(inputs[i].desc), ESTRING("Input"));
    }
  }

// userSoftReset() - include any initialization after a soft reset, ie after configuration changes.
// USER defined
  void userSoftReset() {
    dP("\n In userSoftReset()"); Serial.flush();
    REBOOT;  // defined in processor.h for each mpu
  }

// userHardReset() - include any initialization after a hard reset, ie on boot-up.
// USER defined
  void userHardReset() {
    dP("\n In userHardReset()"); Serial.flush();
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
    #ifdef DEBUG
      Serial.print("\nuserConfigWritten "); Serial.print(address,HEX);
      Serial.print(" length="); Serial.print(length,HEX);
      Serial.print(" func="); Serial.print(func,HEX);
      Serial.print('[');
      for(uint8_t i=0; i<length; i++) {
        int v = NODECONFIG.read(address+i);
        Serial.print(" ");
        if(v<16) Serial.print(0);
        Serial.print(v, HEX);
      }
      Serial.print("]:");
      for(uint8_t i=0; i<length; i++) {
        char c = NODECONFIG.read(address+i);
        if(c<' ' || c==0x8F) Serial.print('.');
        else Serial.print(c);
      }
    #endif
  }

#include "OpenLCBMid.h"           // System house-keeping

// ==== Setup does initial configuration =============================
  void setup() {
    #ifdef DEBUG
      // set up serial comm; may not be space for this!
      delay(1000);
      Serial.begin(115200);
      Serial.print(F("\n8InputNode\n"));
      delay(100);
    #endif

    NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
    Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);
  
    #ifdef DEBUG
      Serial.print("\n    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 0123456789ABDCEF");
      int v;
      char c;
      for(int l=0; l<20; l++) {
        Serial.println();
        Serial.print(l/16,HEX);
        Serial.print(l%16,HEX);
        Serial.print(':');
        for(int a=0; a<16; a++) {
          v = EEPROM.read(l*16+a);
          if(v<16) Serial.print(0);
          Serial.print(v,HEX);
          Serial.print(" ");
        }
        for(int a=0; a<16; a++) {
          c = EEPROM.read(l*16+a);
          if(c<' ' || c==0x8F) Serial.print('.');
          else Serial.print(c);
        }
      }
    
      Serial.print("\n eventIndex-->event");
      for(int i=0; i<NUM_EVENT; i++) {
        Serial.println();
        Serial.print(eventIndex[i]);
        for(int j=0;j<8;j++) {
          Serial.print(" ");
          int v = event[eventIndex[i]].eid.val[j];
          if(v<16) Serial.print("0");
          Serial.print(v, HEX);
        }
        Serial.print(" ");
        Serial.print(event[eventIndex[i]].flags, HEX);
      }
    #endif // debug
    
    // set pins to inputs
    for(int i=0; i<NUM_CHANNEL; i++) {
      pinMode( pins[i], INPUT);
    }
  }

// ==== MAIN LOOP ===========================================
//  -- this performs system functions, such as CAN alias maintenence
void loop() {
  
  bool activity = Olcb_process();     // System processing happens here, with callbacks for app action.
  #ifdef DEBUG
    static unsigned long T = millis()+5000;
    if(millis()>T) {
       T+=5000;
       Serial.print("\n.");
    }
  #endif

  #ifndef OLCB_NO_BLUE_GOLD
    if (activity) {
      // blink blue to show that the frame was received
      //Serial.print("\nrcv");
      blue.blink(0x1);
    }
    if (olcbcanTx.active) { // set when a frame sent
      gold.blink(0x1);
      //Serial.print("\nsnd");
      olcbcanTx.active = false;
    }
    // handle the status lights
    blue.process();
    gold.process();
  #endif //OLCB_NO_BLUE_GOLD

  // process inputs
  if(produceFromInputs) produceFromInputs();

}
