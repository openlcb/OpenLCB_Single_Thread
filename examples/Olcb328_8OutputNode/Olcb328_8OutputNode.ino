
//==============================================================
// Olcb328_8ConsumerNode
//   A prototype of an 8-channel Output OpenLCB board
// 
//   David Harris 2019, adapted from
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//==============================================================

#define DEBUG    // comment out, if not wanted
#define OLCB_NO_BLUE_GOLD  // don't want Blue/Gold

//************ USER DEFINITIONS ************************************

// Node ID --- this must come from a range controlled by the user.  
// See: http://registry.openlcb.org/uniqueidranges
// Uncomment the NEW_NODEID line below to force the NodeID to be written to the board 
#define NEW_NODEID 2,1,13,0,0,0xF1   // DIY range example, not for global use.

// Uncomment to Force Reset to Factory Defaults
#define RESET_TO_FACTORY_DEFAULTS 1

// Board definitions
#define MANU "OpenLCB"           // The manufacturer of node
#define MODEL "8Consumer"    // The model of the board
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
  
#include "processor.h"            // auto-selects the processor type, and CAN lib, EEPROM lib etc.  
#include "OpenLcbCore.h"
#include "OpenLCBHeader.h"        // System house-keeping.

extern "C" {                      // the following are defined as external

// ===== CDI =====
//   Configuration Description Information in xml, **must match MemStruct below**
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
//   CDIheader and CDIFooter contain system-parts, and includes user changable node name and description fields. 
     const char configDefInfo[] PROGMEM = 
       // ===== Enter User definitions below CDIheader line =====
       CDIheader R"(
          <group>
            <name>Output</name>
            <description>Define events associated with Output Pins</description>
            <group replication='8'>
                <name>Digital Output Pins</name>
                <repname>Output</repname>
                <string size='16'><name>Description</name></string>
                <eventid><name>Set Output Low Event</name></eventid>
                <eventid><name>Set Output High Event</name></eventid>
            </group>
          </group>
       )" CDIfooter;
       // ===== Enter User definitions above CDIfooter line =====
}

// ===== MemStruct =====
//   Memory structure of EEPROM, **must match CDI above**
//     -- nodeVar has system-info, and includes the node name and description fields
    typedef struct { 
          EVENT_SPACE_HEADER eventSpaceHeader; // MUST BE AT THE TOP OF STRUCT - DO NOT REMOVE!!!  
          char nodeName[20];  // optional node-name, used by ACDI
          char nodeDesc[24];  // optional node-description, used by ACDI
      // ===== Enter User definitions below =====
          struct {
            char desc[16];        // description of this output
            EventID setLow;       // Consumed eventID which sets this output-pin
            EventID setHigh;      // Consumed eventID which resets this output-pin
          } outputs[NUM_CHANNEL];
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition

extern "C" {
  
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
  //    -- each eventid needs to be identified as a consumer, a producer or both.  
  //    -- PEID = Producer-EID, CEID = Consumer, and PCEID = Producer/Consumer
  //    -- note matching references to MemStruct.  
    #define REG_OUTPUT(s) CEID(outputs[s].setLow), CEID(outputs[s].setHigh) 
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

// Arduino 328 pins:
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

// Choose wisely:
int pins[] = { 4,5,14,15,16,17,18,19 };  // 8 outputs
// ===== Process Consumer-eventIDs =====
// USER defined
void pceCallback(unsigned int index) {
  // Invoked when an event is consumed; drive pins as needed
  // from index of all events.  
  int ch = index / 2;  // two events per channel
  int ev = index % 2;
  Serial.print("\n pceCallback: "); Serial.print(index);
  Serial.print("\n pin: "); Serial.print(pins[ch]);
  if(ev) Serial.print(" set HIGH"); 
  else   Serial.print(" set LOW");
  digitalWrite( pins[ch], (ev?HIGH:LOW) ); // if even index set low, else high
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
    NODECONFIG.put(EEADDR(nodeName), ESTRING("328_8OutputNode"));
    NODECONFIG.put(EEADDR(nodeDesc), ESTRING("Testing"));
    for(int i=0; i<NUM_CHANNEL; i++) {
      NODECONFIG.put(EEADDR(outputs[i].desc), ESTRING("Output"));      
    }
}

// userSoftReset() - include any initialization after a soft reset, ie after configuration changes.
// USER defined
void userSoftReset() {
  #ifdef DEBUG  
    Serial.print("\n In userSoftReset()"); Serial.flush(); 
  #endif
  REBOOT;  // defined in processor.h for each mpu
}

// userHardReset() - include any initialization after a hard reset, ie on boot-up.
// USER defined
void userHardReset() {
  #ifdef DEBUG  
    Serial.print("\n In userHardReset()"); Serial.flush(); 
  #endif
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
void userConfigWritten(unsigned int address, unsigned int length, unsigned int func) {
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
      Serial.print(']:');
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
    delay(1000); 
    Serial.print("\n8OutputNode\n");
  #endif

  #ifdef NEW_NODEID
    NodeID newNodeID(NEW_NODEID);
    nm.changeNodeID(&newNodeID);
  #endif

  #ifdef RESET_TO_FACTORY_DEFAULTS
    Olcb_init(1);
  #else
    Olcb_init(0);
  #endif

  #ifdef DEBUG
    nm.print();
  #endif

  // set pins to outputs
  for(int i=0; i<NUM_CHANNEL; i++) {
    pinMode( pins[i], OUTPUT);
  }
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
  #endif
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
  //if(produceFromInputs) produceFromInputs();  // no inputs

}
