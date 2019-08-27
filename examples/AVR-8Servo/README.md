# AVR 8Servo Example

This sketch was written for a ATMega328.  In my case this included built-in CAN as I used a LEDuino.  However, a CAN board based on teh MCP2515 can be used, such as https://www.ebay.com/itm/For-Arduino-MCP2515-CAN-Bus-Module-TJA1050-Receiver-SPI-Module/311520457612?hash=item488810f38c:g:GuQAAOSwySlaMPc6

This sketch only uses one half of a PCA9685 16-channel PWM board, contolled via I2C, because of space constraints.  A larger AVR would allow more servo or more postitions per servo.  

This sketch has: 
* Each of the eight servos has 3 positions.  
* Each can be set to a position between 0-180 degrees.  
* The end-points of teh servos can be set.  
