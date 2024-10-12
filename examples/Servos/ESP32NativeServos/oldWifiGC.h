
//#pragma message("!!! compiling WifiGC.h ")

#include "OlcbCan.h"

#include <WiFi.h>
#include <ESPmDNS.h>
WiFiClient client;

const char* ssid     = "Den Network";
const char* password = "JbnJbqJbnJbq";

/*
class Can {
  public:
  uint32_t id;
  uint8_t len;
  uint8_t data[8];

  Can(){}
  virtual int open();
  virtual int avail();
  virtual int read();
  virtual int write(int timeout);
  virtual int write();
  virtual int close();
};
*/

class Can : public OlcbCan {
//class WifiGC : public Can {
 public:
  Can(){}
  void init();
  uint8_t  avail();
  uint8_t  read();
  uint8_t  txReady();
  uint8_t  write(long timeout);
  uint8_t  write();
  uint8_t  close();
};


void wifigc_init() {
  static bool opened = false;
  if(opened) return;
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
    }
    Serial.print("\nConnected to ");
    Serial.print(ssid);
    Serial.print("\nIP address: ");
    Serial.print(WiFi.localIP());
    if (!MDNS.begin("ESP32_NodeNumber")) {
        Serial.println("Error setting up MDNS responder!");
        while(1){ delay(1000); }
    }

    int n = MDNS.queryService("openlcb-can", "tcp");
    delay(1000);
    for( int i=0; i<n; i++) {
      // Use WiFiClient class to create TCP connections
      if (client.connect(MDNS.address(i), 12021)) break;
      Serial.println("connection failed");
      if((i+1)==n) while(1);
    }
    Serial.println("\nConnected to OpenLCB/LCC Hub");
    opened = true;
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
int fromGC(Can *m, char* b) {
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
int wifigc_read(Can *m) {
  enum State { sIDLE, sPACKET };
  static State state = sIDLE;
  static char buff[40];
  static int p;
  if(!client.available()) return 0;
  int c = client.read();
  if(state==sIDLE) {
    p = 0;
    //if(c==-1) return 0;
    if(c!=':') return 0;
    buff[p++]=':';
    state = sPACKET;
    return 0;
  } else {
    buff[p++] = c;
    if(c!=';') return 0;
    p = 0;
    state = sIDLE;
    return fromGC(m, buff);
  }
}

#define GP(x) client.print(x)
#define GPH(x) client.print(x,HEX)
#define GP8(x) { if(x<16) GP(0); GPH(x); }
int wifigc_write(Can *m) {
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

  void Can::init() { wifigc_init(); }
  uint8_t  Can::avail() { return 1; }
  uint8_t  Can::read() { return wifigc_read(this); }
  uint8_t  Can::txReady() { return 1; }
  uint8_t  Can::write(long timeout) { return wifigc_write((Can*)this); }
  uint8_t  Can::write() { return wifigc_write((Can*)this); }
  uint8_t  Can::close() { return 1; }
