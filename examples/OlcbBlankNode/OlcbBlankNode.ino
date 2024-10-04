
//==============================================================
// OlcbBlankNode
//   A prototype of a basic no-channel OpenLCB board
// 
//   David Harris 2019, adapted from
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//==============================================================

//// Debugging -- uncomment to activate debugging statements:
    // dP(x) prints x, dPH(x) prints x in hex, 
    //  dPS(string,x) prints string and x
//#define DEBUG Serial

//// Allow direct to JMRI via USB, without CAN controller, comment out for CAN
//   Note: disable debugging AND WifiGC if this is chosen
//#include "GCSerial.h"  

#define OLCB_NO_BLUE_GOLD

//************ USER DEFINITIONS ************************************

// Node ID --- this must come from a range controlled by the user.  
// See: http://registry.openlcb.org/uniqueidranges
#define NODE_ADDRESS 2,1,13,0,0,0x08   // DIY range example, not for global use.

// Uncomment to Force Reset to Factory Defaults
// This MUST be done at least once.  
#define RESET_TO_FACTORY_DEFAULTS 1

// Board definitions
#define MANU "OpenLCB"           // The manufacturer of node
#define MODEL "OlcbBlankNode"    // The model of the board
#define HWVERSION "0.1"          // Hardware version
#define SWVERSION "0.1"          // Software version

// Application definitions:
// For this example, set the number of channels implemented.  
// Each corresponds to an input or output pin.
#define NUM_CHANNEL 0
// Total number of eventids, in this case there are two per channel, 
//  a set and unset.
#define NUM_EVENT 2*NUM_CHANNEL

//************** End of USER DEFINTIONS *****************************
  
#include "mdebugging.h"           // debugging
#include "processor.h"            // auto-selects the processor type, EEPROM lib etc.  
#include "processCAN.h"           // auto-selects the CAN lib from the processor type  
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
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition

extern "C" {
  
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
  //    -- each eventid needs to be identified as a consumer, a producer or both.  
  //    -- PEID = Producer-EID, CEID = Consumer, and PC = Producer/Consumer
  //    -- note matching references to MemStruct.  
       const EIDTab eidtab[NUM_EVENT] PROGMEM = {
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


// ===== Process Consumer-eventIDs =====
// USER defined
    void pceCallback(uint16_t index) {
      // Invoked when an event is consumed; drive pins as needed
      // from index of all events.  
      // Sample code uses inverse of low bit of pattern to drive pin all on or all off.  
      // The pattern is mostly one way, blinking the other, hence inverse.
      //
}

bool states[] = {false, false, false, false}; // current input states; report when changed

// ===== Process inputs and Producers eventids =====
// USER defined
void produceFromInputs() {
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
    NODECONFIG.put(EEADDR(nodeName), ESTRING("OlcbBlankNode"));
    NODECONFIG.put(EEADDR(nodeDesc), ESTRING("Testing"));
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
      dP("\nuserConfigWritten "); dPH(address);
      dP(" length="); dPH(length);
      dP(" func="); dPH(func);
      for(uint8_t i=0; i<length; i++) {
        dP(" ");dP(NODECONFIG.read(address));
      }
  }

#include "OpenLCBMid.h"           // System house-keeping, do not delete!

// ==== Setup does initial configuration =============================
  void setup() {

    #ifdef DEBUG
      // set up serial comm; may not be space for this!
      delay(1000);
      Serial.begin(115200);
      delay(1000);
      dP("\nOlcbBlankNode\n");
      delay(1000);
    #endif

    NodeID nodeid(NODE_ADDRESS);       // this node's nodeid
    Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);

  }

// ==== MAIN LOOP ===========================================
//  -- this performs system functions, such as CAN alias maintenence
  void loop() {
    
    bool activity = Olcb_process();     // System processing happens here, with callbacks for app action.
    static long nextdot = 0;
    if(millis()>nextdot) {
      nextdot = millis()+2000;
      dP("\n.");
    }
  
    #ifndef OLCB_NO_BLUE_GOLD
      if (activity) {
        blue.blink(0x1); // blink blue to show that the frame was received
      }
      if (olcbcanTx.active) { // set when a frame sent
        gold.blink(0x1);
        olcbcanTx.active = false;
      }
      // handle the status lights  
      blue.process();
      gold.process(); 
    #endif //OLCB_NO_BLUE_GOLD

    // process inputs
    if(produceFromInputs) produceFromInputs();

}
