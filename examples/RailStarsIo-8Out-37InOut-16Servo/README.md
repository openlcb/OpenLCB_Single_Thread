**RailStarsIo-8Out-37InOut-16Servo**

This example shows how to make use of the other headers on the Railstars Io.  

This sketch implements: 
-  8 digital output pins
-  8 configurable digital input/outout pins
-  8 Port-B input/output pins
-  8 Port-E input/output pins
-  8 Port-F input/output pins
-  5 Port-D/G input/output pins
-  16 Turnout Servo Outputs, using an external I2C board and the Adafruit_PWMServoDriver

Sketch details:
-  The CDI is quite detailed, and shows how to use \<map\>, \<min\>, \<max\>, and \<default\>.  
-  The memStruct reflects the CDI.  
- The eidtab[] definition shows how to use macros to simplify its definition.  
-  This sketch does not rely on ButtonLeds for most of its i/o.  
-  The **pceCallback(index)** definition is complex, but instructive, because of the multitude of ports.  
-  The **produceFromInputs()** definition is also complex but instructive.  
-  The **userConfigWritten()** definition shows how it can be very useful in the UI to provide immediate feedback on servo positioning.  
