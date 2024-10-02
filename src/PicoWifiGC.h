
#ifndef PICOWIFIGC_H
#define PICOWIFIGC_H

#pragma message("!!! compiling PicoWifiGC.h ")

#define NOCAN
#define BTYPE "PicoWifiGC"

#include "OlcbCan.h"
#include "debugging.h"

#include <WiFi.h>
#include <LEAmDNS.h>
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

void wifigc_lcc() {
  dP("\nwifigc_lcc");
  bool connected = false;
  while( !connected ) {
    int n = MDNS.queryService(openLCB_can, "tcp");
    delay(500);
    if(n>0) {
      for( int i=0; i<n; i++) {
        // Use WiFiClient class to create TCP connections
        if ( client.connect(MDNS.IP(i), 12021) ) {
          dP("\nConnected to OpenLCB/LCC Hub");
          return;
        } else
          dP("connection failed");
      }
    } else
       dP("\nNo hub available, trying again...");
    delay(4000);
  }
}

void wifigc_init() {
  static bool opened = false;
  if( WiFi.status() != WL_CONNECTED ) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
    }
    Serial.print("\nConnected to ");
    Serial.print(ssid);
    Serial.print("\nIP address: ");
    Serial.print(WiFi.localIP());
  }

  if(!client.connected()) {
    if (!MDNS.begin("ESP32_NodeNumber")) {
        dP("Error setting up MDNS responder!");
        while(1){ delay(1000); }
    }
  }
  wifigc_lcc();
}

void wifigc_process() {
  if( !client.connected() ) wifigc_lcc();
}

int readHex(char* b) {
  int t=0;
  if(b[0]>='0' && b[0]<='9') t += b[0]-'0';
  else if(b[0]>='A' && b[0]<='F') t += b[0]-'A'+10;
  else return -1;
  //Serial.print("\n 1:"); Serial.print(t,HEX);
  t *= 16;
  if(b[1]>='0' && b[1]<='9') t += b[1]-'0';
  else if(b[1]>='A' && b[1]<='F') t += b[1]-'A'+10;
  else return -1;
  //Serial.print("\n 2:"); Serial.print(t,HEX);
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
  //Serial.print("\nr");
  if(!client.available()) return 0;
  int c = client.read();
  if(state==sIDLE) {
    p = 0;
    //if(c==-1) return 0;
    if(c!=':') return 0;
    buff[p++]=':';
    dP("\n<<<<:");
    state = sPACKET;
    return 0;
  } else {
    buff[p++] = c;
    dP((char)c);
    if(c!=';') return 0;
    p = 0;
    state = sIDLE;
    return fromGC(m, buff);
  }
}

#define GP(x) client.print(x)
#define GPH(x) client.print(x,HEX)
#define GP8(x) { if(x<16) GP(0); GPH(x); }
#define PP(x) Serial.print(x)
#define PPH(x) Serial.print(x,HEX)
#define PP8(x) { if(x<16) PP(0); PPH(x); }
int wifigc_write(OlcbCanClass *m) {
  GP(":X");
  GP8((uint8_t)(m->id>>24));
  GP8((uint8_t)(m->id>>16));
  GP8((uint8_t)(m->id>>8));
  GP8((uint8_t)(m->id));
  GP("N");
  for(int i=0; i<m->length; i++) GP8(m->data[i]);
  GP(";\r\n");

  PP("\n>>>>:X");
  PP8((uint8_t)(m->id>>24));
  PP8((uint8_t)(m->id>>16));
  PP8((uint8_t)(m->id>>8));
  PP8((uint8_t)(m->id));
  PP("N");
  for(int i=0; i<m->length; i++) PP8(m->data[i]);
  PP(";");
  return 1;
}

  void OlcbCanClass::init() { wifigc_init(); }
  uint8_t  OlcbCanClass::avail() { return 1; }
  uint8_t  OlcbCanClass::read() { wifigc_process(); return wifigc_read(this); }
  uint8_t  OlcbCanClass::txReady() { return 1; }
  uint8_t  OlcbCanClass::write(long timeout) { return wifigc_write((OlcbCanClass*)this); }
  uint8_t  OlcbCanClass::write() { return wifigc_write((OlcbCanClass*)this); }
  uint8_t  OlcbCanClass::close() { return 1; }

  #endif // PICOWIFIGC_H
