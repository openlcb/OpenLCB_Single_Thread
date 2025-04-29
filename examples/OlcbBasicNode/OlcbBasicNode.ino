
//==============================================================
// OlcbBasicNode
//   A prototype of a basic 4-channel OpenLCB board
//
//   David Harris 2019, adapted from
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//   Updated 2024.09 DPH
//==============================================================

//// Debugging -- uncomment to activate debugging statements:
    // dP(x) prints x, dPH(x) prints x in hex,
    //  dPS(string,x) prints string and x
//#define DEBUG Serial

//// Send GC to Serial
//// To allow direct to JMRI via USB, without CAN controller,
//   note: disable debugging if this option is chosen
//#include "GCSerial.h"

//#define NOCAN    // this may be needed if errors occur with GCSerial

//#define OLCB_NO_BLUE_GOLD       // uncomment to disable blue/gold

//************ USER DEFINITIONS ************************************

// Node ID --- this must come from a range controlled by the user.
// See: http://registry.openlcb.org/uniqueidranges
// To choose a new NODEID edit this address
#define NODE_ADDRESS 2,1,13,0,0,0x08   // DIY range example, not for global use.

// Set to 1 to Force Reset to Factory Defaults, else 0.
// This needs to be done at least once for each new board.
#define RESET_TO_FACTORY_DEFAULTS 1

// Board definitions
#define MANU "OpenLCB"           // The manufacturer of node
#define MODEL "OlcbBasicNode"    // The model of the board
#define HWVERSION "0.1"          // Hardware version
#define SWVERSION "0.1"          // Software version

// Application definitions:
// For this example, set the number of channels implemented.
// Each corresponds to an input or output pin.
#define NUM_CHANNEL 4
// Total number of eventids, in this case there are two per channel,
//  a set and unset.
#define NUM_EVENT 2*NUM_CHANNEL

//************** End of USER DEFINTIONS *****************************
#include "mdebugging.h"           // debugging
#include "processCAN.h"           // Auto-select CAN library
#include "processor.h"            // auto-selects the processor type, EEPROM lib etc.
#include "OpenLCBHeader.h"        // System house-keeping.
//#include "OpenLcbCore.h"

extern "C" {                      // the following are defined as external

// ===== CDI =====
//   Configuration Description Information in xml, **must match MemStruct below**
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
//   CDIheader and CDIFooter contain system-parts, and includes user changable node name and description fields.
     const char configDefInfo[] PROGMEM =
       // vvvvv Enter User definitions below CDIheader line vvvvv
       //       It must match the Memstruct struct{} below
       CDIheader R"(
         <group>
           <name>I/O Events</name>
           <description>Define events associated with input and output pins</description>
           <group replication='2'>
             <name>Inputs</name>
             <repname>Input</repname>
             <string size='16'><name>Description</name></string>
             <eventid><name>Activation Event</name></eventid>
             <eventid><name>Inactivation Event</name></eventid>
           </group>
           <group replication='2'>
             <name>Outputs</name>
             <repname>Output</repname>
             <string size='16'><name>Description</name></string>
             <eventid><name>Set Event</name></eventid>
             <eventid><name>Reset Event</name></eventid>
           </group>
         </group>
       )" CDIfooter;
       // ^^^^^ Enter User definitions above CDIfooter line ^^^^^
}

// ===== MemStruct =====
//   Memory structure of EEPROM, **must match CDI above**
    typedef struct {
          EVENT_SPACE_HEADER eventSpaceHeader; // MUST BE AT THE TOP OF STRUCT - DO NOT REMOVE!!!
          char nodeName[20];  // optional node-name, used by ACDI
          char nodeDesc[24];  // optional node-description, used by ACDI
      // vvvvv Enter User definitions below vvvvv
            struct {
              char desc[16];        // description of this input-pin
              EventID activation;   // eventID which is Produced on activation of this input-pin
              EventID inactivation; // eventID which is Produced on inactivation of this input-pin
            } inputs[2];            // 2 inputs
            struct {
              char desc[16];        // decription of this output
              EventID setEvent;     // Consumed eventID which sets this output-pin
              EventID resetEvent;   // Consumed eventID which resets this output-pin
            } outputs[2];           // 2 outputs
      // ^^^^^ Enter User definitions above ^^^^^
    } MemStruct;                    // EEPROM memory structure, must match the CDI above

extern "C" {
  
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
  //    -- each eventid needs to be identified as a consumer, a producer or both.
  //    -- PEID = Producer-EID, CEID = Consumer, and PC = Producer/Consumer
  //    -- note matching name-references to MemStruct.
       const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        PEID(inputs[0].activation), PEID(inputs[0].inactivation),  // 1st channel - input, ie producer
        PEID(inputs[1].activation), PEID(inputs[1].inactivation),  // 2nd channel - input
        CEID(outputs[0].setEvent),  CEID(outputs[0].resetEvent),   // 3rd channel - output, ie consumer
        CEID(outputs[1].setEvent),  CEID(outputs[1].resetEvent),   // 4th channel - output
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
// It includes sampling the pin every 32 ms, and implements blink patterns.

// Patterns
// Each pattern is 32 bits, each bit is used sequencely to blink the LED on and off, at 64 ms per bit.
    #define ShortBlinkOn   0x00010001L
    #define ShortBlinkOff  0xFFFEFFFEL
    
    uint32_t patterns[] = { // two per channel, one per event
      ShortBlinkOff,ShortBlinkOn,
      ShortBlinkOff,ShortBlinkOn,
      ShortBlinkOff,ShortBlinkOn,
      ShortBlinkOff,ShortBlinkOn
    };

// An array of buttons/leds, pA etc are defined in boardChoices.h
    ButtonLed* buttons[] = {
      &pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD // One for each event; each channel is a pair
    };


// ===== Process Consumer-eventIDs =====
// USER defined
    void pceCallback(uint16_t index) {
      // Invoked when an event is consumed; drive pins as needed
      // from index of all events.
      // Sample code uses inverse of low bit of pattern to drive pin all on or all off.
      // The pattern is mostly one way, blinking the other, hence inverse.
      //
      dP("\n In pceCallback index="); dP((uint16_t)index);
      buttons[index]->on( patterns[index]&0x1 ? 0x0L : ~0x0L );  // off : on
      //buttons[index]->on( patterns[index]&0x1 ? 0xF0F0F0F0L : ~0xFFFF0000L );
    }

bool states[] = {false, false, false, false}; // current input states; report when changed

// ===== Process inputs and Producers eventids =====
// USER defined
void produceFromInputs() {
  // assumes eventids are paired per channel
  #define MAX_INPUT_SCAN 2
  static int scanIndex = 0;
    // step through each channel
    if(eidtab[scanIndex*2].flags&&Event::CAN_PRODUCE_FLAG) {; // only process producers
      if (states[scanIndex] != buttons[scanIndex*2]->state) {
        states[scanIndex] = buttons[scanIndex*2]->state;
        if (states[scanIndex]) {
                  dP("\n produce true");
          OpenLcb.produce(scanIndex*2);
        } else {
                   dP("\n produce false");
          OpenLcb.produce(scanIndex*2+1);
        }
      }
    }
    if ((++scanIndex) >= NUM_CHANNEL) scanIndex = 0;
}
#else
    // routine without reference to blue/gold
    void pceCallback(uint16_t index) {
      // Invoked when an event is consumed; drive pins as needed
      // from index of all events.
      // Sample code uses inverse of low bit of pattern to drive pin all on or all off.
      // The pattern is mostly one way, blinking the other, hence inverse.
      //
      dP("\n In pceCallback index="); dP((uint16_t)index);
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
    P("/nEEADDRs\n");
    P("EEADDR(nodeVar.nodeName)");PH(EEADDR(nodeVar.nodeName));
    P("EEADDR(nodeVar.nodeDesc)");PH(EEADDR(nodeVar.nodeDesc));
    P("EEADDR(inputs[0].desc)");PH(EEADDR(inputs[0].desc));
    P("EEADDR(inputs[1].desc)");PH(EEADDR(inputs[1].desc));
    P("EEADDR(outputs[0].desc)");PH(EEADDR(outputs[0].desc));
    P("EEADDR(outputs[1].desc)");PH(EEADDR(outputs[1].desc));
  */
    
    NODECONFIG.put(EEADDR(nodeName), ESTRING( "OlcbBasicNode" ) );
    NODECONFIG.put(EEADDR(nodeDesc), ESTRING( "Testing" ) );
    //NODECONFIG.put(EEADDR(inputs[0].desc), ESTRING( "" ));
    //NODECONFIG.put(EEADDR(inputs[1].desc), ESTRING( "" ));
    //NODECONFIG.put(EEADDR(outputs[0].desc), ESTRING( "" ));
    //NODECONFIG.put(EEADDR(outputs[1].desc), ESTRING( "" ));
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
    dP("\nuserConfigWritten "); dPH((uint32_t)address);
    dP(" length="); dPH((uint16_t)length);
    dP(" func="); dPH((uint8_t)func);
    for(uint8_t i=0; i<length; i++) {
      dP(" "); dP(NODECONFIG.read(address));
    }

  // Another example: if a servo's position changed, then update it immediately
  // uint8_t posn;
  // for(uint16_t i=0; i<NCHANNEL; i++) {
  //    uint16_t pposn = &pmem->channel[i].posn;
  //    if( (address<=pposn) && (pposn<(address+length) ) posn = NODECONFIG.read(pposn);
  //    servo[i].set(i,posn);
  // }
}

#include "OpenLCBMid.h"           // System house-keeping

// ==== Setup does initial configuration =============================
void setup() {
  
  #ifdef DEBUG
    // set up serial comm; may not be space for this!
    Serial.begin(115200); while(!Serial); delay(250);
    dP("\nOlcbBasicNode\n");
    delay(1000);
  #endif

  NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
  Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);

  dP("\neeprom:\n");
  for(int i=0; i<(sizeof(MemStruct)+8); i++) {
    if( i!=0 && (i%16==0) ) { dP("\n"); dP(i); dP(": "); }
    uint8_t x = EEPROM.read(i);
    if(x<16)dP(0); dPH(x); dP(" ");
  }

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
       //dP("\n.");
    }
  #endif

  #ifndef OLCB_NO_BLUE_GOLD
    if (activity) {
      // blink blue to show that the frame was received
      //P("\nrcv");
      blue.blink(0x1);
    }
    if (olcbcanTx.active) { // set when a frame sent
      gold.blink(0x1);
      //P("\nsnd");
      olcbcanTx.active = false;
    }
    // handle the status lights
    blue.process();
    gold.process();
  
  // process inputs
  if(produceFromInputs) produceFromInputs();
  #endif //OLCB_NO_BLUE_GOLD

}
