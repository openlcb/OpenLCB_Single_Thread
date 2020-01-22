
//==============================================================
// OlcbIoNode
//   Default sketch for a RailStars Io OpenLCB board
// 
//   David Harris 2019, adapted from
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//==============================================================
#include "boardTest.h"

#define DEBUG    // comment out, if not wanted
//#define OLCB_NO_BLUE_GOLD   // comment out if NO blue-gold is required

//************ USER DEFINITIONS ************************************

// Node ID --- this must come from a range controlled by the user.  
// See: http://registry.openlcb.org/uniqueidranges
// Uncomment the NEW_NODEID line below to force the NodeID to be written to the board 
//    Recomment this line after the NodeID update 
//#define NEW_NODEID 5,2,1,2,2,0xE3 // Io range example, not for global use.  

// Uncomment to Force Reset to Factory Defaults
//   Recomment after the reset.  
//#define RESET_TO_FACTORY_DEFAULTS 1

// Board definitions
#define MANU "OpenLCB"           // The manufacturer of node
#define MODEL "OlcbIoNode"       // The model of the board/sw
#define HWVERSION "0.1"          // Hardware version
#define SWVERSION "0.1"          // Software version

// Application definitions:
// For this example, set the number of channels implemented.  
// Each corresponds to an input or output pin, so 8 inputs and 8 outputs = 16.
#define NUM_CHANNEL 16
// Total number of eventids, in this case there are two per channel, 
//  a set and unset.
#define NUM_EVENT 2*NUM_CHANNEL

//************** End of USER DEFINTIONS *****************************
  
#include "processor.h"            // auto-selects the processor type, and CAN lib, EEPROM lib etc.  
#include "OpenLcbCore.h"
#include "OpenLCBHeader.h"        // System house-keeping.

extern "C" {                      // the following are defined as external

// ===== CDI =====
// The CDI contains a *description* of the user-variable data held by this node, in terms of type and size of each data item. 
//   Variable data includes: node name, eventids, descriptions, parameters, etc.  
//   Configuration Description Information in xml, **must match MemStruct below**
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
//   CDIheader and CDIFooter contain system-parts, and includes user changable node name and description fields. 
     const char configDefInfo[] PROGMEM = 
       // ===== Enter User definitions below CDIheader line =====
       CDIheader R"(
         <group>
           <name>I/O Events</name>
           <description>Define events associated with input and output pins</description>
           <group replication='8'>
             <name>Inputs</name>
             <repname>Input</repname>
             <string size='16'><name>Description</name></string>
             <eventid><name>Activation Event</name></eventid>
             <eventid><name>Inactivation Event</name></eventid>
           </group>
           <group replication='8'>
             <name>Outputs</name>
             <repname>Output</repname>
             <string size='16'><name>Description</name></string>
             <eventid><name>Set Event</name></eventid>
             <eventid><name>Reset Event</name></eventid>
           </group>
         </group>
       )" CDIfooter;
       // ===== Enter User definitions above CDIfooter line =====
}

// ===== MemStruct =====
//   Memory structure of EEPROM, **must match CDI above**
//   MemStruct is the programmatic structure of the user-variable data held by this node.  
//     -- nodeVar has system-info, and includes the node name and description fields
    typedef struct { 
          EVENT_SPACE_HEADER eventSpaceHeader; // MUST BE AT THE TOP OF STRUCT - DO NOT REMOVE!!!       
          char nodeName[20];  // optional node-name, used by ACDI
          char nodeDesc[24];  // optional node-description, used by ACDI
      // ===== Enter User definitions below =====
            struct {
              char desc[16];        // description of this input-pin
              EventID activation;   // eventID which is Produced on activation of this input-pin 
              EventID inactivation; // eventID which is Produced on inactivation of this input-pin
            } inputs[8];            // 2 inputs
            struct {
              char desc[16];        // decription of this output
              EventID setEvent;     // Consumed eventID which sets this output-pin
              EventID resetEvent;   // Consumed eventID which resets this output-pin
            } outputs[8];           // 2 outputs
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition

extern "C" {
  
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
  //    -- each eventid needs to be identified as a consumer, a producer or both.  
  //    -- PEID() is a macro that creates an entry in the table: offset and flags.  
  //    -- PEID = Producer-EID, CEID = Consumer, and PCEID = Producer/Consumer
  //    -- note matching references to MemStruct.  
       const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        PEID(inputs[0].activation), PEID(inputs[0].inactivation),  // 1st channel - input, ie producer
        PEID(inputs[1].activation), PEID(inputs[1].inactivation),  // 2nd channel - input
        PEID(inputs[2].activation), PEID(inputs[2].inactivation),  // 3rd channel - input
        PEID(inputs[3].activation), PEID(inputs[3].inactivation),  // 4th channel - input
        PEID(inputs[4].activation), PEID(inputs[4].inactivation),  // 5th channel - input
        PEID(inputs[5].activation), PEID(inputs[5].inactivation),  // 6th channel - input
        PEID(inputs[6].activation), PEID(inputs[6].inactivation),  // 7th channel - input
        PEID(inputs[7].activation), PEID(inputs[7].inactivation),  // 8th channel - input
        CEID(outputs[0].setEvent),  CEID(outputs[0].resetEvent),   // 9th channel - output, ie consumer
        CEID(outputs[1].setEvent),  CEID(outputs[1].resetEvent),   // 10th channel - output
        CEID(outputs[2].setEvent),  CEID(outputs[2].resetEvent),   // 11th channel - output
        CEID(outputs[3].setEvent),  CEID(outputs[3].resetEvent),   // 12th channel - output
        CEID(outputs[4].setEvent),  CEID(outputs[4].resetEvent),   // 13th channel - output
        CEID(outputs[5].setEvent),  CEID(outputs[5].resetEvent),   // 14th channel - output
        CEID(outputs[6].setEvent),  CEID(outputs[6].resetEvent),   // 15th channel - output
        CEID(outputs[7].setEvent),  CEID(outputs[7].resetEvent),   // 16th channel - output
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
// Blue-gold refers to two standard buttons offering a rudimentary control system for an node.
//   Features: teaching/learning of eventids; node identification; node reset.

#define BLUE 48
#define GOLD 49
ButtonLed blue(BLUE, LOW);  // else will trigger reset.
ButtonLed gold(GOLD, LOW);
ButtonLed pA(8, LOW);  // prod0
ButtonLed pB(9, LOW);  // prod1
ButtonLed pC(10, LOW);  // prod1
ButtonLed pD(11, LOW);  // prod1
ButtonLed pE(12, LOW);  // prod1
ButtonLed pF(13, LOW);  // prod1
ButtonLed pG(14, LOW);  // prod1
ButtonLed pH(15, LOW);  // prod1
ButtonLed pI(0, HIGH);  // cons0
ButtonLed pJ(1, HIGH);  // cons1
ButtonLed pK(2, HIGH);  // cons1
ButtonLed pL(3, HIGH);  // cons1
ButtonLed pM(4, HIGH);  // cons1
ButtonLed pN(5, HIGH);  // cons1
ButtonLed pO(6, HIGH);  // cons1
ButtonLed pP(7, HIGH);  // cons1

// This section uses the ButtonLed lib to muliplex an input and output onto a single pin.
// It includes sampling every 32 ms, and implements blink patterns.

// Patterns
// patterns are used on indicator leds to indentify modes, etc.  
// Each pattern is 32 bits, each bit is used sequencely to blink the LED on and off, at 64 ms per bit.
    #define ShortBlinkOn   0x00010001L
    #define ShortBlinkOff  0xFFFEFFFEL

    // patterns[] contains one pattern for each event
    uint32_t patterns[NUM_EVENT] = { // two per channel, one per event
      ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,
      ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,
      ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,
      ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,ShortBlinkOff,ShortBlinkOn,
    };

// An array of buttons/leds.
    ButtonLed* buttons[] = {  // One for each event; each channel is a pair
      &pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD,&pE,&pE,&pF,&pF,&pG,&pG,&pH,&pH, // producers
      &pI,&pI,&pJ,&pJ,&pK,&pK,&pL,&pL,&pM,&pM,&pN,&pN,&pO,&pO,&pP,&pP  // consumers
    };


// ===== Process Consumer-eventIDs =====
// USER defined
    void pceCallback(unsigned int index) {
      // Invoked when an event is consumed; drive pins as needed
      // from index of all events.  
      // Sample code uses the low bit of index to drive pin all on or all off.  
      //
      Serial.print("\n In pceCallback index="); Serial.print(index);
      buttons[index]->on( index&0x1 ? 0x0L : ~0x0L );  // off : on
      //buttons[index]->on( patterns[index]&0x1 ? 0xF0F0F0F0L : ~0xFFFF0000L );  // fast and slow flashing
    }
#endif // OLCB_NO_BLUE_GOLD   // this app uses ButtonLed lib for its i/o.


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
  /*
    Serial.print("/nEEADDRs\n");
    Serial.print("EEADDR(nodeVar.nodeName)");Serial.println(EEADDR(nodeVar.nodeName),HEX);
    Serial.print("EEADDR(nodeVar.nodeDesc)");Serial.println(EEADDR(nodeVar.nodeDesc),HEX);
    Serial.print("EEADDR(inputs[0].desc)");Serial.println(EEADDR(inputs[0].desc),HEX);
    Serial.print("EEADDR(inputs[1].desc)");Serial.println(EEADDR(inputs[1].desc),HEX);
    Serial.print("EEADDR(outputs[0].desc)");Serial.println(EEADDR(outputs[0].desc),HEX);
    Serial.print("EEADDR(outputs[1].desc)");Serial.println(EEADDR(outputs[1].desc),HEX);
  */

    // write default values to each datum in the cdi
    NODECONFIG.put(EEADDR(nodeName), ESTRING("OlcbIoNode"));
    NODECONFIG.put(EEADDR(nodeDesc), ESTRING("Testing"));
    NODECONFIG.put(EEADDR(inputs[0].desc), ESTRING("Input1"));
    NODECONFIG.put(EEADDR(inputs[1].desc), ESTRING("Input2"));
    NODECONFIG.put(EEADDR(inputs[2].desc), ESTRING("Input3"));
    NODECONFIG.put(EEADDR(inputs[3].desc), ESTRING("Input4"));
    NODECONFIG.put(EEADDR(inputs[4].desc), ESTRING("Input5"));
    NODECONFIG.put(EEADDR(inputs[5].desc), ESTRING("Input6"));
    NODECONFIG.put(EEADDR(inputs[6].desc), ESTRING("Input7"));
    NODECONFIG.put(EEADDR(inputs[7].desc), ESTRING("Input8"));
    NODECONFIG.put(EEADDR(outputs[0].desc), ESTRING("Output1"));
    NODECONFIG.put(EEADDR(outputs[1].desc), ESTRING("Output2")); 
    NODECONFIG.put(EEADDR(outputs[2].desc), ESTRING("Output3")); 
    NODECONFIG.put(EEADDR(outputs[3].desc), ESTRING("Output4")); 
    NODECONFIG.put(EEADDR(outputs[4].desc), ESTRING("Output5")); 
    NODECONFIG.put(EEADDR(outputs[5].desc), ESTRING("Output6")); 
    NODECONFIG.put(EEADDR(outputs[6].desc), ESTRING("Output7")); 
    NODECONFIG.put(EEADDR(outputs[7].desc), ESTRING("Output8")); 
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
    for(uint8_t i=0; i<length; i++) {
      Serial.print(" ");Serial.print(NODECONFIG.read(address));
    }
  #endif
  // Another example: if a servo's position changed, then update it immediately
  // uint8_t posn;
  // for(unsigned i=0; i<NCHANNEL; i++) {
  //    unsigned int pposn = &pmem->channel[i].posn; 
  //    if( (address<=pposn) && (pposn<(address+length) ) posn = NODECONFIG.read(pposn);
  //    servo[i].set(i,posn);
  // }
}

#include "OpenLCBMid.h"           // System house-keeping --- **must be here**

#ifndef OLCB_NO_BLUE_GOLD

// input state, compared against to detect changes
bool states[] = {false, false, false, false, false, false, false, false}; // current input states; report when changed
// ===== Process inputs and Producers eventids =====
// USER defined
// used to poll inputs and produce events if one changes
void produceFromInputs() {
  //static long next = millis() + 10;
  //if(millis()<next) return;
  //next += 10;
  // assumes eventids are paired per channel
  static int scanIndex = 0;
    // step through each channel
    if(getFlags(scanIndex*2) && Event::CAN_PRODUCE_FLAG) {; // only process producers
      if (states[scanIndex] != buttons[scanIndex*2]->state) {
        states[scanIndex] = buttons[scanIndex*2]->state;
        if (states[scanIndex]) {
                  Serial.print("\n produce true");
          OpenLcb.produce(scanIndex*2);
        } else {
                   Serial.print("\n produce false");
          OpenLcb.produce(scanIndex*2+1);
        }
      }
    }
    if ((++scanIndex) >= NUM_CHANNEL/2) scanIndex = 0;
}
#endif // OLCB_NO_BLUE_GOLD   // this app uses ButtonLed lib for its i/o.

// ==== Setup does initial configuration =============================
void setup() {
  #ifdef DEBUG
    // set up serial comm; may not be space for this!
    while(!Serial){}
    delay(250);Serial.begin(115200);dP(F("\nOlcbIoNode\n"));
    delay(1000);
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
  //while(0==0){}
    //pC.pattern = 0xC0000000L;
    //pD.pattern = 0xF0F0F0F0L;

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

  #ifndef OLCB_NO_BLUE_GOLD  // need blue/gold code to implement indicators.  
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
  
  // process inputs
  if(produceFromInputs) produceFromInputs();
  #endif //OLCB_NO_BLUE_GOLD

}
