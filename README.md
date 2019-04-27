# OpenLCB_Single_Thread
OpenLCB stock based on ArduinoIDE

# **** DO NOT USE ****

This is a refresh of the original Arduino code base developed by Dr. Bob Jacobsen.  It has been modified to make it easier for the developer to match a project's CDI xml to its internal memory structure, by making the two parallel in structure.  In addition, eventid searching uses a sorted table and a binary search. 

# Platforms supported:
* ATMega series, with MCP2515 CAN support; 
* AT90CAN series, with native CAN support; 
* TI Tiva series, with native CAN support; 
* Teensy series, with native CAN support; 
* ESP32, with native CAN supprt.  

Using a specific platform requires downloading of the appropriate compiler support.  
A platform is automagically selected in the processor.h file, allowing the same sketch to be used on multiple platforms.  Platform specific items are included in the processor.h file.  


