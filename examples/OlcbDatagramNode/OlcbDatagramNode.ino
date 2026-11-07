//==============================================================
// OlcbDatagramNode
//   Two boards run this sketch. Give each a different NODE_ADDRESS
//   from a range you control. Point each node's Datagram peer event
//   at the other node's Node ID. The first six bytes of that event
//   are the Node ID; the event is not produced or consumed.
//
//   Until that event is non-zero, send() does nothing. Each board
//   then sends a ping every two seconds. A ping is answered once.
//   A reply is not answered with another reply. LED_BUILTIN toggles
//   when a datagram is accepted, on boards that have that pin.
//
//   David Harris 2026, from OlcbBlankNode
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
// The two boards must not use the same value.
#define NODE_ADDRESS 2,1,13,0,0,0x08   // DIY range example, not for global use.

// Upload once with this set to 1, then set it back to 0 and upload again.
// The second upload leaves EEPROM alone, so later JMRI writes survive.
#define RESET_TO_FACTORY_DEFAULTS 1

// Board definitions
#define MANU "OpenLCB"              // The manufacturer of node
#define MODEL "OlcbDatagramNode"    // The model of the board
#define HWVERSION "0.1"             // Hardware version
#define SWVERSION "0.1"             // Software version

// No produced or consumed events. The peer event lives in MemStruct
// and CDI, and is deliberately absent from eidtab.
#define NUM_EVENT 0

// Private datagrams. 0xF0 is in the user-allocated content-id range.
#define DG_CONTENT 0xF0
#define DG_PING    0x01
#define DG_REPLY   0x02

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
         <group>
           <name>Datagram peer</name>
           <description>The first six bytes are the other node's Node ID. This event is not produced or consumed.</description>
           <eventid><name>Datagram peer</name></eventid>
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
          EventID peer;       // first six bytes: other node's Node ID
      // ===== Enter User definitions above =====
    } MemStruct;                 // type definition

extern "C" {

  // ===== eventid Table =====
  //  Empty: the peer event is not produced or consumed.
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

// userInitAll() -- include any initialization after Factory reset "Mfg clear" or "User clear"
//  -- clear or pre-define text variables.
// USER defined
  void userInitAll() {
    NODECONFIG.put(EEADDR(nodeName), ESTRING("OlcbDatagramNode"));
    NODECONFIG.put(EEADDR(nodeDesc), ESTRING("Datagram peer"));
    EventID zero;
    NODECONFIG.put(EEADDR(peer), zero);
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

NodeID nodeid(NODE_ADDRESS);       // this node's nodeid, must be before next line
#include "OpenLCBMid.h"            // System house-keeping, do not delete.
#include "NodeDatagram.h"          // once, after OpenLCBMid.h

// Declared before `link` so the constructor can take the function,
// and defined after `link` so the body can call link.reply().
uint16_t onDatagram(uint8_t contentId, uint8_t *data, uint16_t length, uint16_t from);

NodeDatagram link(EEADDR(peer), onDatagram);

uint16_t onDatagram(uint8_t contentId, uint8_t *data, uint16_t length, uint16_t from) {
  (void)from;
  if (contentId != DG_CONTENT || length < 1) return 1;
  if (data[0] == DG_PING) {
    uint8_t replyByte = DG_REPLY;
    link.reply(DG_CONTENT, &replyByte, 1);
  } else if (data[0] != DG_REPLY) {
    return 1; // not our protocol; let another handler, or the library, decide
  }
#ifdef LED_BUILTIN
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
#endif
  return 0;
}

// ==== Setup does initial configuration =============================
  void setup() {

    #ifdef DEBUG
      // set up serial comm; may not be space for this!
      delay(1000);
      Serial.begin(115200);
      delay(1000);
      dP("\nOlcbDatagramNode\n");
      delay(1000);
    #endif

#ifdef LED_BUILTIN
    pinMode(LED_BUILTIN, OUTPUT);
#endif

    Olcb_init(nodeid, RESET_TO_FACTORY_DEFAULTS);

  }

// ==== MAIN LOOP ===========================================
//  -- this performs system functions, such as CAN alias maintenence
  void loop() {

    Olcb_process();
    NodeDatagram::check();

    static unsigned long nextSend = 0;
    if (millis() > nextSend) {
      nextSend = millis() + 2000;
      uint8_t ping = DG_PING;
      // Returns false, and sends nothing, while the peer event is all zeros.
      link.send(DG_CONTENT, &ping, 1);
    }

}
