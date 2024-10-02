// DUEcanImpl.h
// Implementation details for DUEcan
// Most of this code is an implementation of the Arduino EEPROM library for a DUE.
// At the moment it uses flash memory to simulate EEPROM.
// This will have an option added to use external EEPROM.

#ifdef __SAM3X8E__

#ifndef DUECANIMPL_H
#define DUECANIMPL_H

//#pragma message("!!! compiling DUEcanImpl.h ")
#include <SPI.h>
#include <processor.h>        // to get E2END
#include <due_can.h>          // Due CAN library header file
#ifdef USE_EXTERNAL_EEPROM
#include "DUEEEPROMConfig.h"
#else
#include <DueFlashStorage.h>  // use Due eeprom emulation library, will overwrite every time program is uploaded !
extern "C" char* sbrk(int incr);
#endif

// Unfortunately DueFlashStorage does not have the same interface as the Arduino EEPROM.
// I have implemented something which matches.


/***
    DueEERef class. This is renamed from the EERef class in EEPROM.h
    
    This object references an EEPROM cell.
    Its purpose is to mimic a typical byte of RAM, however its storage is the EEPROM.
    This class has an overhead of two bytes, similar to storing a pointer to an EEPROM cell.
***/

struct DueEERef{

    DueEERef( const int index )
        : index( index )                 {}
    
    //Access/read members.
    uint8_t operator*() const ;        //   { return DueFlashStorage::read( (uint8_t*) index ); }
    operator uint8_t() const             { return **this; }
    
    //Assignment/write members.
    DueEERef &operator=( const DueEERef &ref ) { return *this = *ref; }
    DueEERef &operator=( uint8_t in );   //    { return DueFlashStorage::write( (uint8_t*) index, in ), *this;  }
    DueEERef &operator +=( uint8_t in )     { return *this = **this + in; }
    DueEERef &operator -=( uint8_t in )     { return *this = **this - in; }
    DueEERef &operator *=( uint8_t in )     { return *this = **this * in; }
    DueEERef &operator /=( uint8_t in )     { return *this = **this / in; }
    DueEERef &operator ^=( uint8_t in )     { return *this = **this ^ in; }
    DueEERef &operator %=( uint8_t in )     { return *this = **this % in; }
    DueEERef &operator &=( uint8_t in )     { return *this = **this & in; }
    DueEERef &operator |=( uint8_t in )     { return *this = **this | in; }
    DueEERef &operator <<=( uint8_t in )    { return *this = **this << in; }
    DueEERef &operator >>=( uint8_t in )    { return *this = **this >> in; }
    
    DueEERef &update( uint8_t in )          { return  in != *this ? *this = in : *this; }
    
    /** Prefix increment/decrement **/
    DueEERef& operator++()                  { return *this += 1; }
    DueEERef& operator--()                  { return *this -= 1; }
    
    /** Postfix increment/decrement **/
    uint8_t operator++ (int){ 
        uint8_t ret = **this;
        return ++(*this), ret;
    }

    uint8_t operator-- (int){ 
        uint8_t ret = **this;
        return --(*this), ret;
    }
    
    int index; //Index of current EEPROM cell.
};

/***
    DueEEPtr class. This is renamed from the EEPtr class in EEPROM.h
    
    This object is a bidirectional pointer to EEPROM cells represented by DueEERef objects.
    Just like a normal pointer type, this can be dereferenced and repositioned using 
    increment/decrement operators.
***/

struct DueEEPtr{

    DueEEPtr( const int index )
        : index( index )                {}
        
    operator int() const                { return index; }
    DueEEPtr &operator=( int in )          { return index = in, *this; }
    
    //Iterator functionality.
    bool operator!=( const DueEEPtr &ptr ) { return index != ptr.index; }
    DueEERef operator*()                   { return index; }
    
    /** Prefix & Postfix increment/decrement **/
    DueEEPtr& operator++()                 { return ++index, *this; }
    DueEEPtr& operator--()                 { return --index, *this; }
    DueEEPtr operator++ (int)              { return index++; }
    DueEEPtr operator-- (int)              { return index--; }

    int index; //Index of current EEPROM cell.
};

/***
    EEPROMClass class.
    
    This object represents the entire EEPROM space.
    It wraps the functionality of EEPtr and DueEERef into a basic interface.
    This class is also 100% backwards compatible with earlier Arduino core releases.
***/


// Copied from Arduino EEPROM.h and renamed.
struct DueEEPROMclass{

    //Basic user access methods.
    DueEERef operator[]( const int idx )    { return idx; }
    uint8_t read( int idx )              { return DueEERef( idx ); }
    void write( int idx, uint8_t val )   { (DueEERef( idx )) = val; }
    void update( int idx, uint8_t val )  { DueEERef( idx ).update( val ); }
    
    //STL and C++11 iteration capability.
    DueEEPtr begin()                        { return 0x00; }
    DueEEPtr end()                          { return length(); } //Standards requires this to be the item after the last valid entry. The returned pointer is invalid.
    uint16_t length()                    { return E2END + 1; }
    
    //Functionality to 'get' and 'put' objects to and from EEPROM.
    template< typename T > T &get( int idx, T &t ){
        DueEEPtr e = idx;
        uint8_t *ptr = (uint8_t*) &t;
        for( int count = sizeof(T) ; count ; --count, ++e )  *ptr++ = *e;
        return t;
    }
    
    template< typename T > const T &put( int idx, const T &t ){
        DueEEPtr e = idx;
        const uint8_t *ptr = (const uint8_t*) &t;
        for( int count = sizeof(T) ; count ; --count, ++e )  (*e).update( *ptr++ );
        return t;
    }
};

// Supply the things missing from DueFlashStorage by making an outer class
// This class is being used for now instead of the code above where write does not work.
// put and get need to be populated.
class DueEEPROMflash : public DueFlashStorage {
	public:
	    uint16_t length()                    { return E2END + 1; }
		void update( int idx, uint8_t val )  { if (read(idx) != val ) write(idx,val); } // Change to only write changed values.
        template< typename T > T &get( int idx, T &t ){
		    byte *b = readAddress(idx);
			memcpy(&t, b, sizeof(T));
		    return t; 
		}
	    template< typename T > const T &put( int idx, const T &t ){ 
		    //write(idx,(byte*) &t, sizeof(T));
			int e = idx;
			const uint8_t *ptr = (const uint8_t*) &t;
  			for( int count = sizeof(T) ; count ; --count, ++e ) { update(e, *ptr++); }
			return t; 
		}
};


//#define dueEEPROMclass EEPROM

//extern DueEEPROMclass EEPROM;

extern DueEEPROMflash EEPROM;

#endif // DUECANIMPL_H



#endif // __SAM3X8E__
