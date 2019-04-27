
//==============================================================
// OlcbBasicNode
//   A prototype of a basic 4-channel OpenLCB board
// 
//   David Harris 2019, adapted from
//   Bob Jacobsen 2010, 2012
//      based on examples by Alex Shepherd and David Harris
//==============================================================

#define DEBUG    // comment out, if not wanted
//#define OLCB_NO_BLUE_GOLD

//************ USER DEFINITIONS ************************************

// Node ID --- this must come from a range controlled by the user.  
// See: http://registry.openlcb.org/uniqueidranges
#define NODEID 2,1,13,0,0,0x07   // DIY range example, not for global use.  

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
  
#include "processor.h"            // auto-selects the processor type, and CAN lib, EEPROM lib etc.  
#include "OpenLCBHeader.h"        // System house-keeping.

NodeID nodeid(NODEID);            // instansiate the nodeid

extern "C" {                      // the following are defined as external

// ===== CDI =====
//   Configuration Description Information in xml, must match MemStruct below
//   See: http://openlcb.com/wp-content/uploads/2016/02/S-9.7.4.1-ConfigurationDescriptionInformation-2016-02-06.pdf
//   CDIheader and CDIFooter contain system-parts, and includes user changable node name and description fields. 
     const char configDefInfo[] PROGMEM = 
       // ===== Enter User definitions below CDIheader line =====
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
       // ===== Enter User definitions above CDIfooter line =====
}

// ===== MemStruct =====
//   Memory structure of EEPROM, must match CDI above
//     -- nodeVar has system-info, and includes the node name and description fields
    typedef struct { 
      NodeVar nodeVar;         // must remain
      // ===== Enter User definitions below =====
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
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition

extern "C" {
  
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
  //    -- each eventid needs to be identified as a consumer, a producer or both.  
  //    -- PEID = Producer-EID, CEID = Consumer, and PC = Producer/Consumer
  //    -- note matching references to MemStruct.  
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
// input/output pin drivers
// 14, 15, 16, 17 for LEDuino with standard shield
// 16, 17, 18, 19 for IOduino to clear built-in blue and gold
// Io 0-7 are outputs & LEDs, 8-15 are inputs
// Define pins
    //#define BLUE 14   // BLUE is 18 LEDuino; others defined by board (48 IO, 14 IOuino)
    //#define GOLD 15   // GOLD is 19 LEDuino; others defined by board (49 IO, 15 IOuino)
    //#define BLUE 42   // BLUE is 18 TCH Consumer
    //#define GOLD 43   // GOLD is 19 TCH Consumer

// Board choices
#define LEDUINO
//#define IO 1
//#define TCH32C
//#define TCH32P
//#define TCH24C16P
//#define TIVA123
//#define Teensy
//#define ESP32

#if defined(LEDUINO)
    #define BLUE 18   // BLUE is 18 LEDuino; others defined by board (48 IO, 14 IOuino)
    #define GOLD 19   // GOLD is 19 LEDuino; others defined by board (49 IO, 15 IOuino)
    //#pragma message("LEDuino i/o")
    ButtonLed blue(BLUE, LOW);  // else will trigger reset.  
    ButtonLed gold(GOLD, LOW);
    ButtonLed pA(14, LOW); 
    ButtonLed pB(15, LOW);
    ButtonLed pC(16, LOW);
    ButtonLed pD(17, LOW);
#elif defined(IO)
    //#pragma message("Io i/o")
    #define BLUE 48
    #define GOLD 49
    ButtonLed blue(BLUE, LOW);  // else will trigger reset.  
    ButtonLed gold(GOLD, LOW);
    ButtonLed pA(8, LOW);  // prod0
    ButtonLed pB(9, LOW);  // prod1
    ButtonLed pC(0, LOW);  // cons0
    ButtonLed pD(1, LOW);  // cons1
#elif defined(TCH32C) || defined(TCH32P) || defined(TCH24C16P)
    #define BLUE 42
    #define GOLD 43
    ButtonLed pA(2, LOW);  // prod0
    ButtonLed pB(3, LOW);  // prod1
    ButtonLed pC(4, LOW);  // cons0
    ButtonLed pD(5, LOW);  // cons1
#elif defined(TIVA123)                    // has two buttons and a RGB led
    #define BLUE 2                // arbitrarily chosen
    #define GOLD 3                // arbitrarily chosen
    ButtonLed blue(BLUE, HIGH);
    ButtonLed gold(GOLD, HIGH);
    ButtonLed pA(PUSH1, LOW);         // arbitrary   PUSH1 does not work as it is dedicated to debugger
    ButtonLed pB(PUSH2, LOW);     // prod1  17   PUSH2 works
    ButtonLed pC(RED_LED, HIGH);   // cons0  30
    ButtonLed pD(GREEN_LED, HIGH); // cons1  39
#elif defined(Teensy)                    // has two buttons and a RGB led
    #define BLUE A0                // arbitrarily chosen
    #define GOLD A1               // arbitrarily chosen
    ButtonLed blue(BLUE, HIGH);
    ButtonLed gold(GOLD, HIGH);
    ButtonLed pA(A2, LOW);         // arbitrary   PUSH1 does not work as it is dedicated to debugger
    ButtonLed pB(A3, LOW);     // prod1  17   PUSH2 works
    ButtonLed pC(A4, HIGH);   // cons0  30
    ButtonLed pD(A5, HIGH); // cons1  39
#elif defined(ESP32)
    #define BLUE 26                // arbitrarily chosen
    #define GOLD 27                // arbitrarily chosen
    ButtonLed blue(BLUE, HIGH);
    ButtonLed gold(GOLD, HIGH);
    ButtonLed pA(34, LOW);     // prod1  34 is input-only
    ButtonLed pB(35, LOW);     // prod2  35 is input-only
    ButtonLed pC(2, HIGH);     // cons1  onboard led
    ButtonLed pD(13, HIGH);    // cons2  13 arbitrary
#else
  #error No board chosen! 
#endif

    #define ShortBlinkOn   0x00010001L
    #define ShortBlinkOff  0xFFFEFFFEL
    
    uint32_t patterns[] = { // two per channel, one per event
      ShortBlinkOff,ShortBlinkOn,
      ShortBlinkOff,ShortBlinkOn,
      ShortBlinkOff,ShortBlinkOn,
      ShortBlinkOff,ShortBlinkOn
    };
    ButtonLed* buttons[] = {  
      &pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD // One for each event; each channel is a pair
    };
    

// ===== Process Consumer-eventIDs =====
    void pceCallback(unsigned int index) {
      // Invoked when an event is consumed; drive pins as needed
      // from index of all events.  
      // Sample code uses inverse of low bit of pattern to drive pin all on or all off.  
      // The pattern is mostly one way, blinking the other, hence inverse.
      //
      Serial.print("\n In pceCallback index="); Serial.print(index);
      buttons[index]->on( patterns[index]&0x1 ? 0x0L : ~0x0L );  // off : on
      //buttons[index]->on( patterns[index]&0x1 ? 0xF0F0F0F0L : ~0xFFFF0000L );
    }

bool states[] = {false, false, false, false}; // current input states; report when changed

void produceFromInputs() {
  // assumes eventids are paired per channel
  #define MAX_INPUT_SCAN 2
  static int scanIndex = 0;
    // step through each channel
    if(eidtab[scanIndex*2].flags&&Event::CAN_PRODUCE_FLAG) {; // only process producers
      if (states[scanIndex] != buttons[scanIndex*2]->state) {
        states[scanIndex] = buttons[scanIndex*2]->state;
        if (states[scanIndex]) {
                  Serial.print("\n produce true");
          pce.produce(scanIndex*2);
        } else {
                   Serial.print("\n produce false");
          pce.produce(scanIndex*2+1);
        }
      }
    }
    if ((++scanIndex) >= NUM_CHANNEL) scanIndex = 0;
}
#endif // OLCB_NO_BLUE_GOLD

/* Config tool: 
 *  More: Reset/Reboot -- causes a reboot --> reads nid, and reads and sorts eids --> userInit();
 *  More: Update Complete -- no reboot --> reads and sorts eids --> userInit();
 *  Reset segment: Usr clear -- new set of new set of eids and blank strings (in system code), doesn't have to reboot --> userClear();
 *  Reset segment: Mfr clear -- original set of eids, blank strings --> userMfrClear().
 *  
 */

// userInitAll() -- include any initialization after Factory reset "Mfg clear" or "User clear"
//  -- clear or pre-define text variables.  
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
    // initialize descriptions
    // default                           tiva, mega328
    //#define ESTRING(s) F(s)         // not tiva, AT90
    //#define ESTRING(s) PSTR(s)      // tiva
    
    EEPROM.put(EEADDR(nodeVar.nodeName), ESTRING("OlcbBasicNode"));
    EEPROM.put(EEADDR(nodeVar.nodeDesc), ESTRING("Testing"));
    EEPROM.put(EEADDR(inputs[0].desc), ESTRING("Input1"));
    EEPROM.put(EEADDR(inputs[1].desc), ESTRING("Input2"));
    EEPROM.put(EEADDR(outputs[0].desc), ESTRING("Output1"));
    EEPROM.put(EEADDR(outputs[1].desc), ESTRING("Output2")); 
}

// userSoftReset() - include any initialization after a soft reset, ie after configuration changes.
void userSoftReset() {
  #ifdef DEBUG  
    Serial.print("\n In userSoftReset()"); Serial.flush(); 
  #endif
  REBOOT;  // defined in processor.h for each mpu
}

// userHardReset() - include any initialization after a hard reset, ie on boot-up.
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
void userConfigWritten(unsigned int address, unsigned int length, unsigned int func) {
  #ifdef DEBUG 
    Serial.print("\nuserConfigWritten "); Serial.print(address,HEX);
    Serial.print(" length="); Serial.print(length,HEX);
    Serial.print(" func="); Serial.print(func,HEX);
    for(uint8_t i=0; i<length; i++) {
      Serial.print(" ");Serial.print(EEPROM.read(address));
    }
  #endif
  //Serial.print(":"); Serial.print(length,HEX);
  // example: if a servo's position changed, then update it immediately
  // uint8_t posn;
  // for(unsigned i=0; i<NCHANNEL; i++) {
  //    unsigned int pposn = &pmem->channel[i].posn; 
  //    if( (address<=pposn) && (pposn<(address+length) ) posn = EEPROM.read(pposn);
  //    servo[i].set(i,posn);
  // }
}

#include "OpenLCBMid.h"           // System house-keeping
#include "extras.h"               // extras

// ==== Setup does initial configuration =============================
void setup() {
  #ifdef DEBUG
    // set up serial comm; may not be space for this!
    while(!Serial){}
    delay(250);Serial.begin(115200);dP(F("\nOlcbBasicNode\n"));
    delay(1000);
  #endif

  //#define FORCE_ALL_INIT 1  // uncomment to force all inint of EEPROM
  Olcb_init(FORCE_ALL_INIT);

  #ifdef DEBUG
    printEeprom();
  #endif
  //while(0==0){}
    //pC.pattern = 0xC0000000L;
    //pD.pattern = 0xF0F0F0F0L;

}

// ==== MAIN LOOP ===========================================
//  -- this performs system functions, such as CAN alias maintenence
void loop() {
  
  bool activity = Olcb_process();
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
  
  // process inputs
  if(produceFromInputs) produceFromInputs();
  #endif //OLCB_NO_BLUE_GOLD

}
