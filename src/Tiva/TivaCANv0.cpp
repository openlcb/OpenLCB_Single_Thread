/*
 * Copyright (C) 2016 by Stuart W Baker
 * CAN library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */


#include "Arduino.h"
#include "TivaCANv0.h"

#include "inc/hw_ints.h"
#include "inc/hw_can.h"
#include "driverlib/can.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

int test_count = 0;

static CANClass *instances[2];

void CANClass::CANIntHandler()
{
    interruptHandled = 1;  // dph
    uint32_t status = ROM_CANIntStatus(Base, CAN_INT_STS_CAUSE);
//while(HWREG(ui32Base + CAN_O_IF1CRQ) & CAN_IF1CRQ_BUSY)
    interruptStatus = status;  // dph
    test_count++;
    
    if (status == CAN_INT_INTID_STATUS)
    {
        status = ROM_CANStatusGet(Base, CAN_STS_CONTROL);
        // some error occured
        if (status & CAN_STATUS_BUS_OFF)
        {
            // bus off error condition
        }
        if (status & CAN_STATUS_EWARN)
        {
            // One of the error counters has exceded a value of 96
            // flush and data in the tx pipeline
            ROM_CANMessageClear(Base, 2);
            TxBuffer.Count = 0;
            TxBuffer.RdIndex = 0;
            TxBuffer.WrIndex = 0;
            TxPending = false;
        }
        if (status & CAN_STATUS_EPASS)
        {
            // In error passive state
        }
        if (status & CAN_STATUS_LEC_STUFF)
        {
            // bit stuffing error occured
        }
        if (status & CAN_STATUS_LEC_FORM)
        {
            // format error occured in the fixed format part of the message
        }
        if (status & CAN_STATUS_LEC_ACK)
        {
            // a transmit message was not acked
        }
        if (status & CAN_STATUS_LEC_CRC)
        {
            // CRC error detected in received message
        }
    }
    else if (status == 1)
    {
        // rx data received
        rxRcv = 1;
        if (RxBuffer.Count < RingBuffer::SIZE)
        {
            // we have space remaining to buffer up this incoming message
            tCANMsgObject can_message;
            can_message.pui8MsgData = RxBuffer.Buf[RxBuffer.WrIndex].buf;
            // Read a message from CAN and clear the interrupt source
            ROM_CANMessageGet(Base, 1, &can_message, 1); // clear interrupt
            
            RxBuffer.Buf[RxBuffer.WrIndex].id = can_message.ui32MsgID;
            RxBuffer.Buf[RxBuffer.WrIndex].rtr = (can_message.ui32Flags &
                                                  MSG_OBJ_REMOTE_FRAME) ? 1 : 0;
            RxBuffer.Buf[RxBuffer.WrIndex].eff = (can_message.ui32Flags &
                                                  MSG_OBJ_EXTENDED_ID) ? 1 : 0;
            RxBuffer.Buf[RxBuffer.WrIndex].err = 0;
            RxBuffer.Buf[RxBuffer.WrIndex].dlc = can_message.ui32MsgLen;
            if (++RxBuffer.WrIndex == RingBuffer::SIZE)
            {
                RxBuffer.WrIndex = 0;
            }
            ++RxBuffer.Count;
        }
        else
        {
            // ran out of space to buffer, flush incoming message
            tCANMsgObject can_message;
            uint8_t data[8];
            can_message.pui8MsgData = data;
            // Read a message from CAN and clear the interrupt source
            ROM_CANMessageGet(Base, 1, &can_message, 1); // clear interrupt
        }
    }
    else if (status == 2)
    {
        // tx complete
        ROM_CANIntClear(Base, 2);
        
        if (TxBuffer.Count)
        {
            // load the next message to transmit
            tCANMsgObject canMessage;
            canMessage.ui32MsgID = TxBuffer.Buf[TxBuffer.RdIndex].id;
            canMessage.ui32MsgIDMask = 0;
            canMessage.ui32Flags = MSG_OBJ_TX_INT_ENABLE;
            if (TxBuffer.Buf[TxBuffer.RdIndex].eff)
            {
                canMessage.ui32Flags |= MSG_OBJ_EXTENDED_ID;
            }
            if (TxBuffer.Buf[TxBuffer.RdIndex].rtr)
            {
                canMessage.ui32Flags |= MSG_OBJ_REMOTE_FRAME;
            }
            canMessage.ui32MsgLen = TxBuffer.Buf[TxBuffer.RdIndex].dlc;
            memcpy(Data, TxBuffer.Buf[TxBuffer.RdIndex].buf, sizeof(Data));
            canMessage.pui8MsgData = Data;
            ROM_CANMessageSet(Base, 2, &canMessage, MSG_OBJ_TYPE_TX);
            if (++TxBuffer.RdIndex == RingBuffer::SIZE)
            {
                TxBuffer.RdIndex = 0;
            }
            --TxBuffer.Count;
        }
        else
        {
            // no more messages pending transmission
            TxPending = false;
        }
    }
    
}

extern "C" {
    static void CANIntHandler0(void)
    {
        instances[0]->CANIntHandler();
    }
    
    static void CANIntHandler1(void)
    {
        instances[1]->CANIntHandler();
    }
    
} // extern "C"

// CANClass::CANClass()
CANClass::CANClass(uint8_t module, uint32_t baud)
: Base(module == 0 ? CAN0_BASE : CAN1_BASE)
, Interrupt(module == 0 ? INT_CAN0 : INT_CAN1)
, Baud(baud)
, TxPending(false)
{
    TxBuffer.Count = 0;
    TxBuffer.RdIndex = 0;
    TxBuffer.WrIndex = 0;
    RxBuffer.Count = 0;
    RxBuffer.RdIndex = 0;
    RxBuffer.WrIndex = 0;
    
    instances[module] = this;
    
}

// CANClass::begin()
void CANClass::begin()
{
    //Serial.print("\ntivaCANv0::begin");
    CANIntRegister(Base, Base == CAN0_BASE ? CANIntHandler0 : CANIntHandler1);
    
    //  ROM_SysCtlPeripheralEnable(Base);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    ROM_GPIOPinConfigure(GPIO_PE4_CAN0RX);
    ROM_GPIOPinConfigure(GPIO_PE5_CAN0TX);
    ROM_GPIOPinTypeCAN(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);
    
    ROM_CANInit(Base);
#if defined(TARGET_IS_BLIZZARD_RB1)
    //  ROM_CANBitRateSet(Base, SysCtlClockGet(), Baud);
    ROM_CANBitRateSet(Base, ROM_SysCtlClockGet(), Baud);
#else
    ROM_CANBitRateSet(Base, F_CPU, Baud);
#endif
    ROM_CANIntEnable(Base, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);
    //    ROM_CANEnable(Base);
    
    tCANMsgObject canMessage;
    canMessage.ui32MsgID = 0;
    canMessage.ui32MsgIDMask = 0;
    canMessage.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
    canMessage.ui32MsgLen = 8;
    ROM_CANMessageSet(Base, 1, &canMessage, MSG_OBJ_TYPE_RX);  // ??? 1?
    
    ROM_IntEnable(Interrupt);
    ROM_CANEnable(Base);
    //Serial.print("\n TivaCANv0 Begin(), TxPending: ");
    //Serial.print(TxPending);
    //Serial.print("  count: ");
    //Serial.print(TxBuffer.Count);
    //Serial.print("  test_count: ");
    //Serial.println(test_count);

}

// CANClass::end()
void CANClass::end()
{
    ROM_CANDisable(Base);
    ROM_IntDisable(Interrupt);
    ROM_CANMessageClear(Base, 1);
    
    ROM_CANIntDisable(Base, CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS);
    ROM_SysCtlPeripheralDisable(Base);
    CANIntUnregister(Base);
}

// CANClass::available()
int CANClass::available()
{
    //Serial.print("\nIn tivaCANv0::available(), count = "); Serial.print(RxBuffer.Count);
    //Serial.print("\nrxRcv = "); Serial.print(rxRcv);
    //Serial.print("\ninterruptHandled = "); Serial.print(interruptHandled);
    //Serial.print("\ninterruptStatus = "); Serial.print(interruptStatus,HEX);
    return RxBuffer.Count;
}

////// Returns true if transmit buffer is free
bool CANClass::tx_idle()
{
    //Serial.print("\ntivaCANv0::tx_idle");
    //Serial.print("\n TxPending = ");
    //Serial.print(TxPending,BIN);
    //return !TxPending;
    //   return (TxBuffer.Count<2);
    if(CANStatusGet(CAN0_BASE, CAN_STS_TXREQUEST)) return false;
    return true;
}


// CANClass::write()
int CANClass::write(const CAN_message_t msg[], size_t count)
{
    //Serial.print("\ntivaCANv0::write");
   int result = 0;
    unsigned long startTime = millis();
    //Serial.println("\nwrite called");
    while (count)
    {
        
        //Serial.print("write loop, TxPending: ");
        // Serial.print(TxPending);
        //Serial.print("  count: ");
        //Serial.print(TxBuffer.Count);
        //Serial.print("  test_count: ");
        //Serial.println(test_count);
        
        ROM_IntDisable(Interrupt);
        if (!TxPending)
        {
            TxPending = true;
            tCANMsgObject canMessage;
            canMessage.ui32MsgID = msg->id;
            canMessage.ui32MsgIDMask = 0;
            canMessage.ui32Flags = MSG_OBJ_TX_INT_ENABLE;
            if (msg->eff)
            {
                canMessage.ui32Flags |= MSG_OBJ_EXTENDED_ID;
            }
            if (msg->rtr)
            {
                canMessage.ui32Flags |= MSG_OBJ_REMOTE_FRAME;
            }
            canMessage.ui32MsgLen = msg->dlc;
            memcpy(Data, msg->buf, sizeof(Data));
            canMessage.pui8MsgData = Data;
            ROM_CANMessageSet(Base, 2, &canMessage, MSG_OBJ_TYPE_TX);
            --count;
            ++msg;
            ++result;
        }
        else if (TxBuffer.Count < RingBuffer::SIZE)
        {
            memcpy(&TxBuffer.Buf[TxBuffer.WrIndex], msg, sizeof(CAN_message_t));
            if (++TxBuffer.WrIndex == RingBuffer::SIZE)
            {
                TxBuffer.WrIndex = 0;
            }
            ++TxBuffer.Count;
            --count;
            ++msg;
            ++result;
        }
        ROM_IntEnable(Interrupt);
        
        if (msg->timeout <= (millis() - startTime))
        {
            break;
        }
    }
    
    return result;
}

// CANClass::read()
int CANClass::read(CAN_message_t msg[], size_t count)
{
    //Serial.print("\ntivaCANv0::read");
    int result = 0;
    unsigned long startTime = millis();
    
    while (count)
    {
        ROM_IntDisable(Interrupt);
        if (RxBuffer.Count)
        {
            memcpy(msg, &RxBuffer.Buf[RxBuffer.RdIndex], sizeof(CAN_message_t));
            //Serial.print("CAN.read rcvd msg. ID=");
            //Serial.println(msg->id,HEX);
            if (++RxBuffer.RdIndex == RingBuffer::SIZE)
            {
                RxBuffer.RdIndex = 0;
            }
            --RxBuffer.Count;
            --count;
            ++msg;
            ++result;
        }
        ROM_IntEnable(Interrupt);
        
        if (msg->timeout <= (millis() - startTime))
        {
            break;
        }
    }
    return result;
}

