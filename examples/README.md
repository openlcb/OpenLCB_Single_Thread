# Example Applications
The provided examples will give some ideas of how to accomplish sample projects.  They can form the basis of, or  be adapted to, a new Application, or just used for inspiration.  <br>

## External Examples
In addition to these examples, there are some excellent examples provided by: 
 - **https://github.com/johnmholmes/Arduino_ESP32_LCC_Journey**
   This is an extensive narrative of exploration with includes an excellent set of specific sketches from which one can choose. 

 - **https://github.com/JohnCallingham/Pico_4Servo_4Frog_Wifi**
 - **https://github.com/JohnCallingham/Pico_4ToF_WiFi**
   These two repositories have sketches that implement four servos on a Pico. One with frog polarity, and the other interfaces with Time of Flight sensors. These are good examples of a practical solutions to common MRR requirements. They have a robust Wifi implementation, and use the Pico's PIO processors to implement servos.  

## Examples

- **AVR-8Servo**<br>
    Implements an eight servo node. This example uses the Adafruit_PWMServoDriver.h and demonstrates how one might implement a node using an external library.

- **AVR2ServoNIO**<br>
    Implements 2 servos and 8 i/o
    Implements MCP2515 connectivity with the ACNA2515 library, and allows setting of the
      MCP2515 clock frequancy, chip select pin, and interrupt pin. 
    Implements 2 servos and 8 i/o
    Implemetns toggle inputs
    Implements delays on producer events
    Implements detach servo at the end of a movement    

- **OlcbBlankNode**<br>
    The example shows the minimal node code without any additional logic.  

- **OlcbBasicNode**<br>
    Implements a simple node which exercises most of the protocols.  It has **two inputs** and **two outputs**.  Each input has two Producer-eventIDs and each output has two Consumer-eventIDs, so **8 eventIDs in total**.  This Application makes use of the ButtonLed library to control **two buttons** and **two LEDs**.  In addition, it implements the BG (Blue-Gold) protocol to allow the **teaching** of eventIDs between this node and others.  
        The example connects the BUILTIN_LED to the first output, so triggering the two associated events will turn that LED on and off.  

- **Olcb328_8InputNode**<br>
    Implements an eight input node. Similarly to the above, example. 
        
- **Olcb328_8OutputNode**<br>
    Implements an eight output node. Similarly to the above, example. 

- **OlcbIoNode**<br>
    Implements the equivalent function to the base code in the Railstars Io and associated DevKit.  It uses this frame work and implements 8 inputs and 8 outputs, with two eventids per i/o.  

- **RailStarsIo-8Out-38InOut-16Servo**<br>
    Creates a node using the Railstars Io board (DevPack board) which implements 8 outputs (consumers), 8 inputs (producers), 24 BOD inputs (producers), and 16 servo outputs (consumers).  The latter uses a PCA8695 PWM chip (see: https://www.adafruit.com/product/815).  It shows how to write a different **pceCallback()**.  It also uses **userConfigWrite()** to allow real-time updating of a servo positions from a **UI Tool**, such as **JMRI** or **Model Railroad System**. 
    
- **Tiva123-8Out-8In-16BoD-16Servo**<br>
    Creates a node using the Tiva123 Launchpad board from TI (see: http://www.ti.com/tool/EK-TM4C123GXL) which implements 8 outputs (consumers), 8 inputs (producers), 16 BOD inputs (producers), and 16 servo outputs (consumers).  The latter uses a PCA8695 PWM chip (see: https://www.adafruit.com/product/815).  One will want to uncomment the INITIALIZE_TO_NODE_ADDRESS and RESET_TO_FACTORY_DEFAULTS the first time to initialize the EEPROM, and then they should be recommented to minimize EEPROM rewrites.  
    
- **OlcbBlankNode**<br>
    This is a blank node which can be used to develop a new node.

## Servo Examples

See the Servos directory
   
