# OpenLCB_Single_Thread
## OpenLCB stack based on ArduinoIDE

## **** Can be used, but this repository will be depreciated, and the contents moved elsewhere. ****

This is a refresh of the original Arduino code base, developed by Dr. Bob Jacobsen.  It uses a single-threaded model, with an initialization step and a endless loop to do the processing.  These are the familiar setup() and loop() of the Arduino IDE.  Much of the standard processing for OpenLCB protocols is handled by 'systems' code.  This includes obtaing and managing an node Alias, receiving and vetting eventids, scheduling an sending eventids, CDI, configuration, Datagram management, etc.  

The original codebase has been modified to make it easier for the developer to match a project's CDI xml to its internal memory structure, by making the two have parallel structures.  In addition, eventid searching uses a sorted table index and a binary search.  (David Harris and Alex Shepherd)

# Platforms supported:
* ATMega series, with MCP2515 CAN support; 
* AT90CAN series, with native CAN support; 
* TI Tiva series, with native CAN support; 
* Teensy series, with native CAN support; 
* ESP32, with native CAN supprt.  

Using a specific platform requires downloading of the appropriate compiler support.  
A platform is automagically selected in the processor.h file, allowing the same sketch to be used on multiple platforms.  Platform specific items are included in the processor.h file.  

NB: support for Nucleo boards is pending.  

### At this point, the example sketch OlcbBasicNode compiles in the supported platforms.  

## Changes: 
1. Added support for multiple processors: AVR/MCP2515, AT90CAN, Teensy, Tiva, and ESP32. 
   - Each set of files specific to a CAN-processor is kept in its own directory.   
   - The processor is automatically selected in the processor.h file. 
2. A sorted eventIndex[] is used to speed eventID processing.  
3. Simplified the definition of CDI/xml for the node by matching a struct{} to the xml structure, see the example below.   

e.g.: This CDI/xml, which self-describes a node to the system, having 8 channels with a pair of eventids each:
```xml
    <cdi>
        ...
        <group replication='8'>
        <name>Channels</name>
            <eventid><name>event0</name></eventid>
            <eventid><name>event1</name></eventid>
        </group>
        ...
    </cdi>
```
parallels this program structure:    
```c++
    typedef struct {
        ...
        struct {
            EventID event0;
            EventID event1;
        } channels[8];
        ...
    } MemStruct;
```

In addition, EIDtab[] is constructed with offsets to *every* eventid in EEPROM, and its type: producer, consumer, or both.  
```c++
  // ===== eventid Table =====
  //  Array of the offsets to every eventID in MemStruct/EEPROM/mem, and P/C flags
  //    -- each eventid needs to be identified as a consumer, a producer or both.  
  //    -- PEID = Producer-EID, CEID = Consumer, and PC = Producer/Consumer
  //    -- note matching references to MemStruct.  
       const EIDTab eidtab[NUM_EVENT] PROGMEM = {
        PEID(channels[0].event0),   PEID(channels[0].event1),  // 1st channel - input, ie producer
        PEID(channels[1].event0),   PEID(channels[1].event1),  // 2nd channel - input, ie producer
        PEID(channels[2].event0),   PEID(channels[2].event1),  // 3rd channel - input, ie producer
        PEID(channels[3].event0),   PEID(channels[3].event1),  // 4th channel - input, ie producer
        CEID(channels[4].event0),   CEID(channels[4].event1),  // 5th channel - output, ie consumer
        CEID(channels[5].event0),   CEID(channels[5].event1),  // 6th channel - output, ie consumer
        CEID(channels[6].event0),   CEID(channels[6].event1),  // 7th channel - output, ie consumer
        CEID(channels[7].event0),   CEID(channels[7].event1),  // 8th channel - output, ie consumer
      };
```
In this case, the first four pairs of eventids are producers, and the remaining are consumers.  The type is used by internal processing to allow eventids produced by this node to be scheduled and send, and to indetify received eventids as being consumed by this node and therefore passed to the application code.  

## Memory Model:
Eventids are read from eeprom into event[].  Their location in EEPROM is held in EIDtab[], see above.  

The index into eventid[] is then sorted into eventIndex[] --- event[] and EIDtab entries remain in the original order.  Binary search on eventIndex[] is then used to match received eventids to their entries in event[].  Diagrammatically:

     eventIndex[]--->EIDtab[offset,flags]-->EEPROM
     eventIndex[]--->event[eventid]
 

#### In Flash:<br>
EEPROM, or equivalent, is laid out in accordance with Memstruct, which matches the CDI xml, see above.  
    
#### In RAM:
events[] holds a copy of the node's eventids initialized from EEPROM.  
EIDtab[] holds the offsets to the eventids in EEPROM, this is built using Memstruct to calculate the eventid-offsets.  
eventIndex[] indexes into to event[] and EIDtab[] in ascending sorted order.  

## More about OpenLCB/LCC - what is it?

OpenLCB/LCC is a set of heirarchical protocols to **let nodes talk to each other**. <br>
For more information on OpenLCB, see: [OpenLCB.org](http://openlcb.org)<br>
For protocol descriptions, see: [Adopted LCC Documents](http://openlcb.org/openlcb-and-lcc-documents/layout-command-control-lcc/)

These protocols consist of: 
 - **Systems/Application Messaging**<br>
    These are the 'workhorse' messages on which most applications will be built, they are useful for sytems-messaging as well, and for building further systems-protocols.  
   - PCE - Produver/Consumer Event Messages
     - These are *unaddressed* EventID messages.
     - They implement *Producer/Consumer Events* (64-bit)
     - EventIDs are globally unique 64-bit numbers.
     - These are one-to-many messages.
   - Datagram Messages
     - These are *addressed* messages containing up to 70-bytes of data in a single message.
     - These are one-to-one messages.
   - Stream Messages
     - These are *addressed* messages carrying unlimited data in multiple messages.
     - These are one-to-one messages.
 - **Systems/Housekeeping**<br>
    These are the 'behind-the-scenes' protocol that enables and ensures the system's integrity. 
   - Link - establishes and maintains the node's link to the network
     - Announces state of Node
     - Announcement of *Intialization Complete*
     - Announcement of *Consumed-* and *Produced-eventIDs*
     - *NodeID reporting* on request.
     - *EventID reporting* on request.
     - On the CAN-implementation, this maintains *alias assignment and maintenance*;
   - SNIP - Simple Node Information Protocol
     - Provides a brief description of the node for *UI Tools* to use.
   - PIP - Protocol Identification Protocol
     - Defines which protocols the node uses, and it is reported as a bit-map.  
   - CDI - Configuration Description Information
     - *Reports the node's CDI/xml* on request.
   - Memory Configuration
     - Reading and writing to the node's memory spaces, including Configuration, RAM and EEPROM spaces.
 - **Additional Protocols**<br>
    These protocols extend the base-system.
   - Teaching -- teaching an eventID from one node to one or more others.  
   - Traction Control -- train control.
 - **Additional Utility-Libraries**<br>
    These libraries implement useful functionality.  
   - BG - Blue/Green -- node health indicators and system buttons.
   - ButtonLed -- implements controlling a button and LED from a single processor pin.

## How the Above Translates to the Codebase
The 'codebase' is a set of libraries and functions that implement the basic protocols of OpenLCB/LCC. <br> 
Each protocol has corresponding **code**, usually in the form of a **class**, and implemented as a pair of *.h and *.cpp  files. <br> 
The codebase tries to hide some of the complexity in #include files.  <br>

However, each protocol needs to have: 
 - **initialization**, and
 - **processing**
    
For example there are some selected lines of code from the OlcbBasicNode example used for **initialization**: 
```c++
  NodeID nodeid(5,1,1,1,3,255);    // This node's default ID; must be valid 
  const char SNII_const_data[] PROGMEM = "\001OpenLCB\000DPHOlcbBasicNode\0001.0\000" OlcbCommonVersion ; 
  uint8_t protocolIdentValue[6] = {0xD7,0x58,0x00,0,0,0};
  ButtonLed* buttons[] = { &pA,&pA,&pB,&pB,&pC,&pC,&pD,&pD };
```
Most of the **processing** is hidden as functions in the #include files, specifiaclly OpenLCBHeader.h and OpenLCBMid.h.  

## How Does the Application Interact with the Codebase?
The programmer of the Application must: 
 - Decide what and how the new Application works, ie how eventids and other node variables are used to build the Application.  
 - Choose the **NodeID** - this must be from **a range controlled** by the manufacturer - **ie you**.  
 - Write the **CDI/xml** describing the node and its node-variables, including its eventIDs. 
 - Write a **MemStruct{}** that matches the xml description.  
 - Write code to flag each eventID as a **Producer, Consumer, or Both** into EIDtab[].  
 - Write **pceCallback()** to process received eventIDs passed to it, ie eventIDs which are to be consumed, and causing whatever action is required, eg a LED being lit or extinguished.  
 - Write **produceFromInputs()** which scans the node's inputs and, if appropriate, flags an evenItD to be sent.  
 - Write **userConfigWrite()** which is called whenever a UI Tool writes to the node's memory.  This code can then compare the memory address range of the change to the node's variables, and take whatever action is appropriate, e.g. update a servo position.
 - Write additional support and glue code for the Application.  

## Example Applications
The provided examples will give some ideas of how to accomplish sample projects.  They can form the basis of, or  be adapted to, a new Application, or just used for inspiration.  <br>
 - **OlcbBasicNode**<br>
    Implements a simple node which exercises most of the protocols.  It has **two inputs** and **two outputs**.  Each input has two Producer-eventIDs and each output has two Consumer-eventIDs, so **8 eventIDs in total**.  This Application makes use of the ButtonLed library to control **two buttons** and **two LEDs**.  In addition, it implements the BG (Blue-Gold) protocol to allow the **teaching** of eventIDs between this node and others.  
    
- **RailStarsIo-8Out-38InOut-16Servo**<br>
    Creates a node using the Railstars Io board (DevPack board) which implements 8 outputs (consumers), 8 inputs (producers), 24 BOD inputs (producers), and 16 servo outputs (consumers).  The latter uses a PCA8695 PWM chip (see: https://www.adafruit.com/product/815).  It shows how to write a different **pceCallback()**.  It also uses **userConfigWrite()** to allow real-time updating of a servo positions from a **UI Tool**, such as **JMRI** or **Model Railroad System**. 
    
- **Tiva123-8Out-8In-16BoD-16Servo**<br>
    Creates a node using the Tiva123 Launchpad board from TI (see: http://www.ti.com/tool/EK-TM4C123GXL) which implements 8 outputs (consumers), 8 inputs (producers), 16 BOD inputs (producers), and 16 servo outputs (consumers).  The latter uses a PCA8695 PWM chip (see: https://www.adafruit.com/product/815).  One will want to uncomment the INITIALIZE_TO_NODE_ADDRESS and RESET_TO_FACTORY_DEFAULTS the first time to initialize the EEPROM, and then they should be recommented to minimize EEPROM rewrites.  
    
- **OlcbIoNode**<br>
    Implements the equivalent function to the base code in the Railstars Io and associated DevKit.  It uses this frame work and implements 8 inputs and 8 outputs, with two eventids per i/o.  
    
## In prgress, but not uploaded
- **OlcbBlankNode**<br>
    This is a blank node which can be used to develop a new node.  
   


