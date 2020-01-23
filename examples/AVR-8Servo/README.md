# AVR 8Servo Example

This sketch was written for a ATMega328.  In my case I used a LEDuino with built-in CAN.  However, any AVR, with an appropriate  MCP2515 based CAN board, can be used.  Here is an example CAN board: https://www.ebay.com/itm/For-Arduino-MCP2515-CAN-Bus-Module-TJA1050-Receiver-SPI-Module/311520457612?hash=item488810f38c:g:GuQAAOSwySlaMPc6

This sketch only uses one half of a PCA9685 16-channel PWM board, contolled via I2C, to control 8 servos.  The number of servos is limited due to memory space.  A larger AVR would allow more servo, or more postitions per servo.  

This sketch has: 
* Each of the eight servos has 3 positions.  
* Each can be set to a position between 0-180 degrees.  
* The end-points of the servos can be set.  

It demonstrates: 
* CDI
* memstruct of EEPROM reflecting the CDI structure
* setting flags to refect whether eveentids are used as consumers, producers, or both, see: *const EIDTab eidtab[NUM_EVENT] PROGMEM*
* Initialization routine to initialize the EEPROM, see: userInitAll()
* Eventid processing to set a servo's position, see: pceCallback(unsigned int index)
