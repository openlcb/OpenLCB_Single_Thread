
#if 0

/*
 EEPROM.h - EEPROM library
 Original Copyright (c) 2006 David A. Mellis.  All right reserved.
 New version by Christopher Andrews 2015.
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef EEPROM_h
#define EEPROM_h

//#pragma message("!!! compiling tivaeeprom.h ")

//#include "Energia.h"
#include "driverlib/eeprom.h"
#include <driverlib/sysctl.h>

struct EERef{
    
    union U {
        uint32_t u32;
        uint8_t  u8[4];
    } U;
    uint8_t read(const uint32_t idx ) const {
        union U v;
        EEPROMRead((uint32_t*)&v.u32, (uint32_t)idx, sizeof(v.u32));
        return v.u8[idx%4];
    }
    void write( int idx, uint8_t val )  const  {
        union U v;
        uint32_t r;
        EEPROMRead(&r, (uint32_t)idx, sizeof(r));
        v.u32 = r;
        v.u8[idx%4] = val;
        if(r!=v.u32) EEPROMProgram(&v.u32, (uint32_t)idx, sizeof(v.u32));
    }
    void update( int idx, uint8_t val )  {
        write(idx,val);
    }
    
    /*
     uint8_t eeprom_read_byte( uint8_t* address) const {
     Serial.print("\nIn eeprom_read_byte");
     uint32_t byteAddr = ((uint32_t) address)>>2;
     Serial.print("\nbyteAddr = "); Serial.print(byteAddr,HEX);
     //uint32_t byteAddr = address - (address % BYTES_PER_WORD);
     //int block = address / (BYTES_PER_WORD * WORDS_PER_BLOCK);
     //int word = (address / BYTES_PER_WORD) % WORDS_PER_BLOCK;
     uint32_t wordVal = 0;
     //EEPROMRead(&wordVal, byteAddr, 4);
     EEPROMRead(&wordVal, byteAddr, 4);
     Serial.print("\wordVal = "); Serial.print(wordVal,HEX);
     return ((unsigned int)wordVal >> ((*address&0x03) << 3)) & 0xff;
     }
     
     void eeprom_write_byte(uint8_t* address, uint8_t value) {
     Serial.print("\n\nIn eeprom_write_byte");
     uint32_t byteAddr = (uint32_t) address;
     Serial.print("\nbyteAddr0 = "); Serial.print(byteAddr,HEX);
     //byteAddr >>= BYTES_PER_WORD;
     byteAddr >>= 2;
     //uint32_t byteAddr = ((uint32_t)address)>>BYTES_PER_WORD;
     Serial.print("\nbyteAddr shifted by "); Serial.print(2,HEX);
     Serial.print(" = "); Serial.print(byteAddr,HEX);
     uint32_t wordVal = 0;
     //EEPROMRead(&wordVal, byteAddr, 4);
     EEPROMRead(&wordVal, byteAddr, 4);
     Serial.print("\npre-wordVal = "); Serial.print(wordVal,HEX);
     uint8_t bpos = ((uint32_t)address) % BYTES_PER_WORD;
     Serial.print("\nbpos = "); Serial.print(bpos,HEX);
     wordVal &= ~(0xFF << (8*bpos));
     Serial.print("\n&-wordVal = "); Serial.print(wordVal,HEX);
     wordVal += value << (8*bpos);
     Serial.print("\npost-wordVal = "); Serial.print(wordVal,HEX);
     EEPROMProgram(&wordVal, byteAddr, 4);
     }
     */
    
    EERef( const int index )
    : index( index )                 {}
    
    //Access/read members.
    //uint8_t operator*() const            { return eeprom_read_byte( (uint8_t*) index ); }
    uint8_t operator*() const            { return read( (const uint32_t) index ); }
    operator const uint8_t() const       { return **this; }
    
    //Assignment/write members.
    EERef &operator=( const EERef &ref ) { return *this = *ref; }
    //EERef &operator=( uint8_t in )       { return eeprom_write_byte( (uint8_t*) index, in ), *this;  }
    EERef &operator=( uint8_t in )       { return write( (uint32_t) index, in ), *this;  }
    EERef &operator +=( uint8_t in )     { return *this = **this + in; }
    EERef &operator -=( uint8_t in )     { return *this = **this - in; }
    EERef &operator *=( uint8_t in )     { return *this = **this * in; }
    EERef &operator /=( uint8_t in )     { return *this = **this / in; }
    EERef &operator ^=( uint8_t in )     { return *this = **this ^ in; }
    EERef &operator %=( uint8_t in )     { return *this = **this % in; }
    EERef &operator &=( uint8_t in )     { return *this = **this & in; }
    EERef &operator |=( uint8_t in )     { return *this = **this | in; }
    EERef &operator <<=( uint8_t in )    { return *this = **this << in; }
    EERef &operator >>=( uint8_t in )    { return *this = **this >> in; }
    
    EERef &update( uint8_t in )          { return  in != *this ? *this = in : *this; }
    
    /** Prefix increment/decrement **/
    EERef& operator++()                  { return *this += 1; }
    EERef& operator--()                  { return *this -= 1; }
    
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
 EEPtr class.
 
 This object is a bidirectional pointer to EEPROM cells represented by EERef objects.
 Just like a normal pointer type, this can be dereferenced and repositioned using
 increment/decrement operators.
 ***/

struct EEPtr{
    
    EEPtr( const int index )
    : index( index )                {}
    
    operator const int() const          { return index; }
    EEPtr &operator=( int in )          { return index = in, *this; }
    
    //Iterator functionality.
    bool operator!=( const EEPtr &ptr ) { return index != ptr.index; }
    EERef operator*()                   { return index; }
    
    /** Prefix & Postfix increment/decrement **/
    EEPtr& operator++()                 { return ++index, *this; }
    EEPtr& operator--()                 { return --index, *this; }
    EEPtr operator++ (int)              { return index++; }
    EEPtr operator-- (int)              { return index--; }
    
    int index; //Index of current EEPROM cell.
};

/***
 EEPROMClass class.
 
 This object represents the entire EEPROM space.
 It wraps the functionality of EEPtr and EERef into a basic interface.
 This class is also 100% backwards compatible with earlier Arduino core releases.
 ***/

struct EEPROMClass{
    
    EEPROMClass() {
        SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ); // islemcimizi 80 Mhz'e ayarlıyoruz.
        SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0); // EEPROM activate
        EEPROMInit(); // EEPROM start
    }
    
    //Basic user access methods.
    EERef operator[]( const int idx )    { return idx; }
    uint8_t read( int idx )              { return EERef( idx ); }
    void write( int idx, uint8_t val )   { (EERef( idx )) = val; }
    void update( int idx, uint8_t val )  { EERef( idx ).update( val ); }
    
    //STL and C++11 iteration capability.
    EEPtr begin()                        { return 0x00; }
    EEPtr end()                          { return length(); } //Standards requires this to be the item after the last valid entry. The returned pointer is invalid.
    uint16_t length()                    { return E2END + 1; }
    
    //Functionality to 'get' and 'put' objects to and from EEPROM.
    template< typename T > T &get( int idx, T &t ){
        EEPtr e = idx;
        uint8_t *ptr = (uint8_t*) &t;
        for( int count = sizeof(T) ; count ; --count, ++e )  *ptr++ = *e;
        return t;
    }
    
    template< typename T > const T &put( int idx, const T &t ){
        EEPtr e = idx;
        const uint8_t *ptr = (const uint8_t*) &t;
        for( int count = sizeof(T) ; count ; --count, ++e )  (*e).update( *ptr++ );
        return t;
    }
    void init(void) {
//         Serial.print("\nIn EEPROMClass::init");
        SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
        EEPROMInit();
    }
};

static EEPROMClass EEPROM;
#endif

#endif // big if
