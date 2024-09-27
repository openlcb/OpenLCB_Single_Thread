# Olcb328_8inputNode Sketch

This sketch demonstrates a very basic input board with eight channels.  

* It implements 8 inputs
* It uses ButtonLED library to implement an input and output on one pin
* It shows how to write a CDI description
* It shows the corresponding EEPROM structure
* It shows how inputs are processed to send eventids

This sketch complements the Olcb328_8outputNode, but is limited in its 
function.  Things like inverting inouts, or adding an option for pullups.  

Although it was originally written for the AVR series, it can be compiled 
on the ESP32, Pico, and Tiva processors, using native and firmware CAN drivers.  

In addition, GCSerial can be used to connect via USB to JMRI, say.  Also, Wifi
on the ESP32 and Pico can be used to connect to OpenLCB/LCC via a OpenLCB hub, 
such as JMRI's.  
