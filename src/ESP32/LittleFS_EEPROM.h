#pragma once

// ESP32 configuration image stored as a LittleFS file.
// The core EEPROM object keeps the image in one NVS blob, which cannot
// grow past 4096 bytes. This stand-in has the read/write/get/put/commit
// calls NodeMemory uses, and the image can be as large as EEPROMSIZE.
//
// Set the size before including processor.h:
//   #define EEPROMSIZE 8192
// Define OLCB_USE_NATIVE_EEPROM to keep the core NVS EEPROM instead.

#if defined(ARDUINO_ARCH_ESP32)

#include <Arduino.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

class LittleFSEEPROM {
public:
    LittleFSEEPROM() : _buffer(0), _size(0), _dirty(false), _initialized(false) {}

    bool begin(size_t size);
    uint8_t read(int address);
    void write(int address, uint8_t val);
    void update(int address, uint8_t val);
    bool commit();
    size_t length();

    template<typename T>
    T &get(int idx, T &t) {
        if (_initialized && idx >= 0 && (size_t)idx + sizeof(T) <= _size) {
            memcpy(&t, _buffer + idx, sizeof(T));
        }
        return t;
    }

    template<typename T>
    const T &put(int idx, const T &t) {
        if (_initialized && idx >= 0 && (size_t)idx + sizeof(T) <= _size) {
            if (memcmp(_buffer + idx, &t, sizeof(T)) != 0) {
                memcpy(_buffer + idx, &t, sizeof(T));
                _dirty = true;
            }
        }
        return t;
    }

private:
    LittleFSEEPROM(const LittleFSEEPROM&);
    LittleFSEEPROM& operator=(const LittleFSEEPROM&);

    bool mountFs();
    bool loadFile(const char* path);
    bool importNvsImage();

    uint8_t* _buffer;
    size_t _size;
    bool _dirty;
    bool _initialized;
};

extern LittleFSEEPROM OpenLCB_EEPROM;

#endif
