# AVRPCA9685Servos

This sketch is primary for the Nano, but can be adapteded to other processors.  

- It uses the ServoEasing library and its facility to use the PCA9685 chip to drive servos.

  servo[outputIndex]->startEaseTo(servoPosDegrees);
  
- It uses slider hints to show sliders to set variables in the JMRI GUI

      <name>Servo Position in Degrees</name>
      <min>0</min><max>180</max>
      <hints><slider tickSpacing='30' immediate='yes' ></slider></hints>

- An example of a compatible PCA9685 board is here: https://www.adafruit.com/search?q=pca9685

  ![image](https://github.com/user-attachments/assets/d98242f3-8fe4-4a61-8f54-b0047e23e389)

