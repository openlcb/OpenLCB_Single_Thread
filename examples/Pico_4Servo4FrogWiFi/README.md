# Pico-4Servo4FrogWifi
This sketch provides;-
- A Pico W using Wifi to connect to an OpenLCB/LCC hub. You must provide local network credentials and then it searches for a hub using mDNS. JMRI has an OpenLCB/LCC hub available.
- Demonstrates using native Servo driver that uses PIO processors.  
  This is limited to 7 while using WiFi, but only 4 are used here.
- When a servo reaches a position it sends a reached event and when it leaves a position it sends a leaving event.
- Outputs provide for frog switching.
- When reset to factory defaults servo positions are as below;-
    - position 1 (Thrown) - set to 80 degrees
    - position 2 (Mid) - set to 90 degrees
    - position 3 (Closed) - set to 100 degrees
- This allows only the smallest of movements which can be adjusted when connected to a turnout.
- The mid point can be used to centre the servo for installation.
- When the module starts it sends out the leaving event for all servos and sets the servo to
the mid position.
