# OlcbBasicNode Sketch

This sketch demonstrates most of the functionality of this library.  

* It implements 2 inputs and 2 outputs
* It uses ButtonLED library to implement an input and output on one pin
* It shows how to write a CDI description
* It shows the corresponding EEPROM structure
* It shows how inputs are processed to send eventids
* It shows how eventids are porcessed to effect output changes

It has a supporting chooseBoard.h which tries to change the pinout mapping based on the 
board type in use.  This will likely need to be modified.  
For example, the AVR328 expects buttonleds on pins 14-19, while a Tiva123 uses its two buttons as inputs and the RYB LED as outputs.  
