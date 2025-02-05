# OlcbBasicNode Sketch

Adapted to the Uno Minima

This sketch demonstrates most of the functionality of this library.  

* It implements 2 inputs and 2 outputs
* It shows how to write a CDI description
* It shows the corresponding EEPROM structure
* It shows how inputs are processed to send eventids
* It shows how eventids are porcessed to effect output changes
* It shows the use of a shim class to use the built-in CAN processor

This adaptation uses:
 - channel:   0,1   2,3
 -  pin[4] = { 2,3, 13,12} - 2 and 3 are inputs, and 13 and 12 are outputs

In userInitAll: 
 - Channels 0 and 1 are initialized as INPUT_PULLUP 
 - Channels 2 and 3 are initialized as OUTPUTS.

In produceFromInputs():
 - Inputs are scanned avery 50 ms
 - if one has changed one of its pair of eventids is sent.
  
pceCallback(index) is called with any consumed and matched eventid's index:
 - In this case indexes 4 and 5 control channel 2/pin 13 (LED), 
 - Indexes 6 and 7 control channel 3/pin 12.  
  
userInitAll() is called if the node is being set to manufacturer's default
 - Any CDI fields can be intiialized.
userConfigWritten(addr, len, fnc) is called if a Tool hs changed the CDI. 
 - The passed address can be matched to the CDI addresses, and action taken.
 - Sometimes it is easier to just reintialize everything. 
 - In this case it just logs that it has been called. 

