// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef ARDUINO_ARCH_ESP32

#ifndef ESP32CAN_H
#define ESP32CAN_H

//#pragma message("!!! compiling ESP32can.h ")

#define DEFAULT_CAN_RX_PIN GPIO_NUM_4
#define DEFAULT_CAN_TX_PIN GPIO_NUM_5

#include "ESP32can.h"

class ESP32can {

public:
   ESP32can();
   ~ESP32can();

   virtual int begin(long baudRate);
   virtual void end();

   virtual int txReady();
   virtual int write(uint32_t _txId, uint8_t _txLength, uint8_t* _txData, bool _txExtended);
   virtual int write(uint32_t _txId, uint8_t _txLength, uint8_t* _txData);
   virtual int avail();
   virtual int read(uint32_t &_rxId, uint8_t &_rxDlc, uint8_t* _rxData, bool &_rxExtended);

   virtual void onReceive(void(*callback)(int));

  //using CANControllerClass::filter;
   virtual int filter(int id, int mask);
  //using CANControllerClass::filterExtended;
   virtual int filterExtended(long id, long mask);

   virtual int observe();
   virtual int loopback();
   virtual int sleep();
   virtual int wakeup();

   void setPins(int rx, int tx);

   void dumpRegisters(Stream& out);

private:
  void reset();

  void handleInterrupt();

  uint8_t readRegister(uint8_t address);
  void modifyRegister(uint8_t address, uint8_t mask, uint8_t value);
  void writeRegister(uint8_t address, uint8_t value);

  static void onInterrupt(void* arg);

private:
  gpio_num_t _rxPin;
  gpio_num_t _txPin;
  bool _loopback;
  intr_handle_t _intrHandle;
};

extern ESP32can CAN;

#endif //ESP32CAN_H

#endif //ARDUINO_ARCH_ESP32
