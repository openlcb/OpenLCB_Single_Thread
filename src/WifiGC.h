//
//  WifiGC.h
//
//
//  Created by Dave Harris on 2024-09-17.
//
//  This uses WifiManager library:
//  to provision the node.  To do this on the first execution Wifimanager
//  will open an AP called "WifiGCAP" with a password of "password".
//  You need to point your Wifi at it, either on your computer or phone,
//  and it should open a new browser window.  If it doesn't, then open a
//  a browser and type 192.168.4.1 into the address bar.
//  The window will allow you to enter your local Wifi cerdentials, then
//  the node should reboot and search for a OpenLCB hub, called "openlcb-can".
//  The JMRI OpenLCB/LCC hub will work.
//
//  Notes:
//    1. You will likely need to uncomment the NOCAN define statement in the
//       main sketch (not below) to prevent processCAN.h from choosing the
//       native CAN library.
//    2. You will need to comment out the GCSerial define, but can uncomment the
//       DEBUG define.

#ifndef WIFIGC_H
#define WIFIGC_H

//#pragma message("!!! compiling WifiGC.h ")

#define NOCAN // disallow processor's CAN, this seems to fail on ESP32 compiler
#define BTYPE "WifiGC"

#include "OlcbCan.h"

#include <WiFi.h>
#include <ESPmDNS.h>
WiFiClient client;

class OlcbCanClass : public OlcbCan {
 public:
    OlcbCanClass(){}
  void init();
  uint8_t  avail();
  uint8_t  read();
  uint8_t  txReady();
  uint8_t  write(long timeout);
  uint8_t  write();
  uint8_t  close();
};

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

void wifigc_connectOpenLcb() {
    if (!MDNS.begin("ESP32_NodeNumber")) {
        Serial.println("Error setting up MDNS responder!");
        while(1){ delay(1000); }
    }

    //int n = MDNS.queryService(openLCB_can, "tcp");
    int n = MDNS.queryService("openlcb-can", "tcp");
    delay(1000);
    for( int i=0; i<n; i++) {
      // Use WiFiClient class to create TCP connections
      if (client.connect(MDNS.address(i), 12021)) break;
      Serial.println("\nconnection failed");
      if(i==n) {
        i=0;
        delay(4000);
      }
    }
    Serial.println("\nConnected to OpenLCB/LCC Hub");
}

void wifigc_init() {
    // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.
    pinMode(39, INPUT_PULLUP);
    // put your setup code here, to run once:
    Serial.begin(115200); delay(1000); Serial.print("\nWifiMngr");
    
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // This uses the front button on an Atom to trigger wiping
    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    if(digitalRead(39)==0) wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result
    bool res;
    wm.setConfigPortalTimeout(40);
    wm.setWiFiAutoReconnect(true);
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("WifiGCAP","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();  // we will just retry
    }
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected to Wifi");
    }
    //Serial.print("\nConnected to "); Serial.print(ssid);
    Serial.print("\nIP address: ");
    Serial.print(WiFi.localIP());
    wifigc_connectOpenLcb();
}

// if hub loses connection, try again
void wifigc_process() {
  if(!client.connected()) {
    wifigc_connectOpenLcb();
  }
}

int readHex(char* b) {
  int t=0;
  if(b[0]>='0' && b[0]<='9') t += b[0]-'0';
  else if(b[0]>='A' && b[0]<='F') t += b[0]-'A'+10;
  else return -1;
  t *= 16;
  if(b[1]>='0' && b[1]<='9') t += b[1]-'0';
  else if(b[1]>='A' && b[1]<='F') t += b[1]-'A'+10;
  else return -1;
  return t;
}
int fromGC(OlcbCanClass *m, char* b) {
  int x;
  int p=0;
  while(b[p]!=':') { p++; }
  p++;
  if(b[p++]!='X') return -2;
  m->id = 0;
  for(int i=0; i<4; i++) {
    x = readHex(&b[p]); 
    if(x<0) break;
    m->id = m->id*256+x; 
    p+=2;
  }
  if(b[p++]!='N') return -3;
  m->length = 0;
  for(int i=0; i<8; i++) {
    m->data[i] = 0;
    x = readHex(&b[p]); 
    if(x<0) break;
    m->data[i] = m->data[i]*256 + x;
    m->length++;
    p += 2;
  }
  if(b[p]!=';') return -4;
  return 1;
}

int wifigc_read(OlcbCanClass *m) {
  enum State { sIDLE, sPACKET };
  static State state = sIDLE;
  static char buff[40];
  static int p;
  if(!client.available()) return 0;
  int c = client.read();
  if(state==sIDLE) {
    p = 0;
    if(c!=':') return 0;
    buff[p++]=':';
    state = sPACKET;
    return 0;
  } else {
    buff[p++] = c;
    if(c!=';') return 0;
    p = 0;
    state = sIDLE;
    //Serial.print(">>>"); Serial.print(buff);
    return fromGC(m, buff);
  }
}

#define GP(x) client.print(x)
#define GPH(x) client.print(x,HEX)
#define GP8(x) { if(x<16) GP(0); GPH(x); }
int wifigc_write(OlcbCanClass *m) {
  GP(":X");
  GP8((uint8_t)(m->id>>24));
  GP8((uint8_t)(m->id>>16));
  GP8((uint8_t)(m->id>>8));
  GP8((uint8_t)(m->id));
  GP("N");
  for(int i=0; i<m->length; i++) GP8(m->data[i]);
  GP(";\r\n");
  return 1;
}

  void OlcbCanClass::init() { wifigc_init(); }
  uint8_t  OlcbCanClass::avail() { return 1; }
  uint8_t  OlcbCanClass::read() {
    //wifigc_process();  //// dph check this out
    return wifigc_read((OlcbCanClass*)this);
  }
  uint8_t  OlcbCanClass::txReady() { return 1; }
  uint8_t  OlcbCanClass::write(long timeout) { return wifigc_write((OlcbCanClass*)this); }
  uint8_t  OlcbCanClass::write() { return wifigc_write((OlcbCanClass*)this); }
  uint8_t  OlcbCanClass::close() { return 1; }

#endif //WIFIGC_H
