// Extras
/*
void printEventIndexes() {
  Serial.print(F("\nprintEventIndex\n"));
  for(int i=0;i<NUM_EVENT;i++) {
    Serial.print(eventIndex[i],HEX); Serial.print(F(", ")); 
    //eventsIndex[i].print();
    //Serial.print(F("\n hash: ")); Serial.print(eventIndex[i].hash,HEX);
    //Serial.print(F("  index: ")); Serial.print(eventIndex[i].index,HEX);
  }
}
void printEvents() {
  Serial.print(F("\nprintEvents "));
  //Serial.print(MEM_MODEL);
  for(int i=0;i<8;i++) {
    //Serial.print(F("\n  offset: ")); Serial.print(events[i].offset,HEX);
    Serial.print(F(" flags: ")); Serial.print(event[i].flags,HEX);
    #ifdef MEM_MODEL_MEDIUM
       Serial.print(F(" eventID: ")); eventids[i].print();
    #endif
  }
}
*/

void printEeprom() {
    Serial.print("\nEEPROM:");
    Serial.print(F("\n    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 0123456789ABCDEF"));
    for(unsigned r=0; r<(sizeof(MemStruct)/16+1);r++) {
        int rb = r*16;
        Serial.print("\n"); if(rb<16) Serial.print(0); Serial.print(rb,HEX); Serial.print(" ");
        for(int i=rb;i<(rb+16);i++)  {
            uint8_t v = EEPROM.read(i);
            if(v<16) Serial.print(0); Serial.print(v,HEX); Serial.print(" ");
        }
        for(int i=rb;i<(rb+16);i++)  {
            char c = EEPROM.read(i);
            if( c<' ' || c==0xFF ) Serial.print('.');
            else Serial.print(c);
        }
    }
}
