# AVRNativeServos with Sliders and Easing

This sketch implenents servos on the AVR, but can also be compiled on ESP32, Pico, etc.  

- It uses the ServoEasing library, which is an excellent library for adding ariations to servo movements.
  In this case it just does fixed easing, so the motion eases into its final position.
  
        servo[outputIndex].startEaseTo(servoPosDegrees);
  
- It also shows how to add slider hints to the CDI so that values can be set with a slider
  in the JMRI GUI:
  
        <name>Servo Position in Degrees</name>
        <min>0</min><max>180</max>
        <hints><slider tickSpacing='30' immediate='yes' ></slider></hints>

