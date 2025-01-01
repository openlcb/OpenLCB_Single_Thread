# AVR8Output4State node Sketch

Pin Allocations
* Pins 10,11,12,13 used for MCP2515 module
* Pins 4,5,6,7,A0,A1,A2,A3 used for the 8 output channels

This sketch demonstrates a basic output node with eight output channels.  
* It implements 8 outputs
* It has 4 possible states to choose from
* It uses ButtonLED library to implement an input and output on one pin
* It shows how to write a CDI description
* It shows the corresponding EEPROM structure
* It shows how eventids are porcessed to effect changes in the outputs.

Although this is labelled AVR, references the Nano/Uno microprocessor boards, 
it may also compile on the ESP32, Pico, and Tivs boards.  

Obviously it has minimal usefulness, and more functionalty can easily be added
to implement pulsed outputs and repeating patterns.  
