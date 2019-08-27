# AVR 8Servo Example

This sketch was written for a ATMega328.  In my case I used a LEDuino with built-in CAN.  However, any AVR, with an approprite  MCP2515 based CAN board, can be used.  Here is an example CAN board: https://www.ebay.com/itm/For-Arduino-MCP2515-CAN-Bus-Module-TJA1050-Receiver-SPI-Module/311520457612?hash=item488810f38c:g:GuQAAOSwySlaMPc6

This sketch only uses one half of a PCA9685 16-channel PWM board, contolled via I2C, to control 8 servos.  The number of servos is limited due to memory space.  A larger AVR would allow more servo, or more postitions per servo.  

This sketch has: 
* Each of the eight servos has 3 positions.  
* Each can be set to a position between 0-180 degrees.  
* The end-points of teh servos can be set.  

I demonstrates: 
* CDI
* memstruct of EEPROM reflecting the CDI structure
* setting flags to refect whether eveentids are used as consumers, producers, or both
* Initialization routine to initialize the EEPROM
* Eventid processign to set a servo's position
