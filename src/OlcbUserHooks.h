#ifndef OlcbUserHooks_h
#define OlcbUserHooks_h

#include <stdint.h>

// Optional sketch functions. Each reference must be weak: a plain extern
// in a .cpp forces the linker to require the function. A missing function
// has address 0, so call sites test the pointer before calling.
// C++ linkage. A normal sketch definition overrides the weak symbol.

extern void Olcb_softReset() __attribute__((weak));
extern void userUsrClear() __attribute__((weak));
extern void userMfgClear() __attribute__((weak));
extern void userSoftReset() __attribute__((weak));
extern void userHardReset() __attribute__((weak));
extern void userInitAll() __attribute__((weak));
extern void userFactoryReset() __attribute__((weak));
extern void userConfigWritten(uint32_t address, uint16_t length, uint16_t func) __attribute__((weak));
extern void pceCallback(uint16_t index) __attribute__((weak));
extern void produceFromInputs() __attribute__((weak));
extern uint8_t userState(uint16_t index) __attribute__((weak));

#endif
