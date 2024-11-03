# AVR 2-Servo N-IO Example, with toggle

This sketch was written for a Nano or other ATMega328.  
However, any AVR, with an appropriate  MCP2515 based CAN board, can be used.  
Here is an example CAN board: 
  https://www.amazon.ca/Comidox-MCP2515-Receiver-Controller-Development/dp/B07J9KZ4L4
In addition, direct via USB using GCSerial or connection via Wifi are possible on a ESP32 or Pico-W.  

This sketch implements:
* two servos, each with three positions
   Positions cabe set to angles 0-180
   The 0 and 180 end-points of the servos can be configured.
* N i/o channels, each of which can be an input or an output,
   If an output it may be solid, pulse or flashing, and consumes an on- and 0ff-event. 
   If an input it produces an on- and off-event, which may each be delayed.  The 
     inputs can be with or with pullups, inverted or not, or can be toggle. 

It demonstrates: 
* CDI
* memstruct of EEPROM reflecting the CDI structure
* setting flags to refect whether eveentids are used as consumers, producers, or both, see: **const EIDTab eidtab[NUM_EVENT] PROGMEM**
* Initialization routine to initialize the EEPROM, see: **userInitAll()**
* Eventid processing to set a servo's position, see: **pceCallback(unsigned int index)**
* Sampling of inputs and producing events.

This sketch will run on the AVR Mega, ESP32, Pico, and Tiva processor boards.  These 
all have more memory and the sketches capabilities could be extended.  

Pin usage will need to be adapted to the board. Sample pin usage is included for the Nano. 


