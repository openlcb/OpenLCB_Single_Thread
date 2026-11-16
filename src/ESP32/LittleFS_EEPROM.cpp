#if defined(ARDUINO_ARCH_ESP32)

#include <Arduino.h>
#include <EEPROM.h>
#include <LittleFS.h>
#include "LittleFS_EEPROM.h"

// Live image, a complete replacement written first, and the previous live
// image kept until the replacement is in place.
static const char kImagePath[] = "/openlcb_eeprom.bin";
static const char kTempPath[] = "/openlcb_eeprom.tmp";
static const char kBakPath[] = "/openlcb_eeprom.bak";

// NVS stores one blob. Stay inside the size the core EEPROM object accepts.
static const size_t kNvsEepromMax = 4096;

LittleFSEEPROM OpenLCB_EEPROM;

// NODE_ID_STORE at raw address 0: marker 'N', 6-byte NodeID, checksum.
// Same test as NodeMemory::loadAndValidate(): 0xFF xor those 8 bytes is 0.
static bool nvsNodeIdValid(const uint8_t* image, size_t n) {
    if (n < 8 || image[0] != 'N') return false;
    uint8_t sum = 0xFF;
    for (uint8_t i = 0; i < 8; i++) sum ^= image[i];
    return sum == 0;
}

bool LittleFSEEPROM::mountFs() {
    if (LittleFS.begin(false)) return true;
    // A stock "spiffs" partition is not a LittleFS volume until formatted.
    Serial.println("    LittleFS mount failed, formatting the spiffs partition");
    return LittleFS.begin(true);
}

bool LittleFSEEPROM::loadFile(const char* path) {
    File f = LittleFS.open(path, "r");
    if (!f) return false;
    size_t got = f.read(_buffer, _size);
    f.close();
    if (got == 0) return false;
    if (got < _size) {
        memset(_buffer + got, 0xFF, _size - got);
        _dirty = true;
    }
    return true;
}

bool LittleFSEEPROM::importNvsImage() {
    size_t nvsSize = _size < kNvsEepromMax ? _size : kNvsEepromMax;
    if (!EEPROM.begin(nvsSize)) return false;

    for (size_t i = 0; i < nvsSize; i++) _buffer[i] = EEPROM.read((int)i);
    if (nvsSize < _size) memset(_buffer + nvsSize, 0xFF, _size - nvsSize);
    // end() commits only if the core object is dirty. A read leaves it clean.
    EEPROM.end();

    if (!nvsNodeIdValid(_buffer, nvsSize)) {
        memset(_buffer, 0xFF, _size);
        return false;
    }
    _dirty = true;
    return true;
}

bool LittleFSEEPROM::begin(size_t size) {
    if (size == 0) return false;
    if (_initialized && _size >= size) return true;
    if (_initialized && _dirty && !commit()) return false;

    uint8_t* fresh = new uint8_t[size];
    if (!fresh) return false;

    delete[] _buffer;
    _buffer = fresh;
    _size = size;
    _dirty = false;
    _initialized = false;

    if (!mountFs()) {
        delete[] _buffer;
        _buffer = nullptr;
        _size = 0;
        return false;
    }

    bool live = loadFile(kImagePath);
    bool bak = !live && loadFile(kBakPath);
    if (live || bak) {
        if (bak) _dirty = true;
        _initialized = true;
        return true;
    }

    memset(_buffer, 0xFF, _size);
    _dirty = true;
    if (importNvsImage()) {
        Serial.println("    copied NVS EEPROM image to LittleFS");
    }
    _initialized = true;
    return true;
}

uint8_t LittleFSEEPROM::read(int address) {
    if (!_initialized || address < 0 || (size_t)address >= _size) return 0xFF;
    return _buffer[address];
}

void LittleFSEEPROM::write(int address, uint8_t val) {
    if (!_initialized || address < 0 || (size_t)address >= _size) return;
    if (_buffer[address] == val) return;
    _buffer[address] = val;
    _dirty = true;
}

void LittleFSEEPROM::update(int address, uint8_t val) {
    write(address, val);
}

bool LittleFSEEPROM::commit() {
    if (!_initialized) return false;
    if (!_dirty) return true;

    File f = LittleFS.open(kTempPath, "w");
    if (!f) return false;
    size_t written = f.write(_buffer, _size);
    f.flush();
    f.close();
    if (written != _size) {
        LittleFS.remove(kTempPath);
        return false;
    }

    // Move the live file aside, then put the new file in its place.
    // A reset in between leaves the previous image under the .bak name,
    // which begin() loads.
    if (LittleFS.exists(kBakPath)) LittleFS.remove(kBakPath);
    if (LittleFS.exists(kImagePath)) {
        if (!LittleFS.rename(kImagePath, kBakPath)) {
            LittleFS.remove(kTempPath);
            return false;
        }
    }
    if (!LittleFS.rename(kTempPath, kImagePath)) {
        if (LittleFS.exists(kBakPath)) LittleFS.rename(kBakPath, kImagePath);
        return false;
    }
    LittleFS.remove(kBakPath);
    _dirty = false;
    return true;
}

size_t LittleFSEEPROM::length() {
    return _size;
}

#endif
