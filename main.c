/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  
  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - v3.00
        Device            :  PIC18F25K22
        Driver Version    :  2.00
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.35
        MPLAB             :  MPLAB X 3.20
*/

/*
                         Main application
 */


#define _XTAL_FREQ 16000000 //The speed of your internal(or)external oscillator
#define USE_AND_MASKS

#define LINK_ON_COUNT (100)
#define LINK_OFF_COUNT (10000)
#define FLASH_COUNT (5)
#define UART_COUNT (100)

#include <xc.h>

#include "config_pragma.h"
#include "intosc.h"
#include "uart.h"
#include "eeprom.h"
#include "blink.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define EE_ADR (0)

#define STATE_SEND         (1)
#define STATE_WAITFORCTRL  (2)
#define STATE_WAITFORADDR  (3)
#define STATE_WAITFORLEN   (4)
#define STATE_HEADERCOMPLETE (5)
#define STATE_PASSTHRU     (6)
#define STATE_READPACKET   (7)
#define STATE_WAITFORCRC   (8)

#define CTRL_DATATRANS (0x80)
#define CTRL_DATAACK (0x81)
#define CTRL_GETADDR (0x82)
#define CTRL_PING (0x83)

#define ERR_BUFFULL (0xC0)
#define ERR_PCKINCMPL (0xC1)
#define ERR_CRCFAIL (0xC2)

unsigned char uartSendBuffer[132];
unsigned char uartRecvBuffer[132];
char packetRecvBuffer[132];
char packetSendBuffer[132];
char spiRecvBuffer[106];
char spiSendBuffer[106];

ledType ledRx;
ledType ledTx;

uartType uart;

char adr;
char mode;
char packetReady;
char dataAvailable;

char packetSendLen;

char packetCtrl;
char packetAddress;
char packetRecvLen;

void LoadConfigData(void)
{
    adr=EERead(EE_ADR);
}

void SaveConfigData()
{
    if (adr) EEWrite(EE_ADR,adr);    
}

void InitSystem(void)
{
    // Oszillator konfigurieren
    SetupInternalOscillator(INTOSC16MHZ,INTOSCENPLL);  
    
    // Load config from eeprom
    LoadConfigData();
    
    // Setup interrupts
    INTCON=0;
    INTCON3=0;
    PIE1=0;
    PIE2=0;
    PIE3=0;
    PIE4=0;
    PIE5=0;

    RCONbits.IPEN = 0; //enable priority levels
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    
    // set IO
    TRISAbits.RA0=0;
    TRISAbits.RA1=0;
    TRISAbits.RA2=0;
    TRISAbits.RA3=0;
    TRISAbits.RA4=0;
    TRISAbits.RA5=0;
    ANSELAbits.ANSA0=0;
    ANSELAbits.ANSA1=0;
    ANSELAbits.ANSA2=0;
    ANSELAbits.ANSA3=0;
    ANSELAbits.ANSA5=0;
    
    TRISBbits.RB4=0; // RS485 read/write
    TRISBbits.RB5=0;
                
    ledInit(&(ledRx),FLASH_COUNT,FLASH_COUNT,0b00000001);
    ledInit(&(ledTx),FLASH_COUNT,FLASH_COUNT,0b00000010);    
}

void InitUart()
{
    uartInit(&(uart),1,9600,64000000,uartSendBuffer,160,uartRecvBuffer,160);
    uartInitHalfDuplex(&(uart),&LATB,5);

    uartEnableSend(&(uart),1);
    uartEnableRecv(&(uart),1);
}

void interrupt high_priority IntHighRoutine()
{
    INTCONbits.GIE=0;
 
    uartInterruptHandler(&(uart));

    INTCONbits.GIE=1;
}

void myDelay(long per)
{
    int i;
    while (per--!=0) 
    {
        i=_XTAL_FREQ/4000; // 4 Takte pro Zyklus in millisekunden
        while (i--!=0) {}
    }
}

char crcOk(char crc)
{
    return 1;
}

void decodeRecvBuffer()
{
    
}

void main(void)
{     
    char b;
    char newByte;
    char nullByte=0;
    char pckRecvPtr=0;
    adr=-1;
    
    InitSystem();
    
    ledOn(&ledRx);
    ledOn(&ledTx);
        
    InitUart(); 
    myDelay(500);
    
    ledOff(&ledRx);
    ledOff(&ledTx);
    
    mode=STATE_WAITFORCTRL;
    packetAddress=-1;  
    packetRecvLen=-1;
    
    while (1)
    {
        if (mode==STATE_SEND)
        {
            if (uart.status & UART_SENDBUFEMPTY)
            {
                mode=STATE_WAITFORCTRL;
            }
            else
            {
                ledFlash(&ledTx);
                continue;
            }
        }
        
        if (mode==STATE_WAITFORCTRL)
        {
            if (packetReady) // check if something to send
            {
                uartSendBuf(&uart,packetRecvBuffer,packetReady);
                mode=STATE_SEND;
                packetReady=0;                   
                continue;
            }      
        }  
        
        if (!uartRecvByte(&uart,b)) // check, if no new byte was received continue
            continue;
        
        ledFlash(&ledRx); // show recv
        
        if (b & 128) // CTRL byte?
        {
            // if (mode!=STATE_WAITFORCTRL) IncompletePacket!
            packetCtrl=b;
            mode=STATE_WAITFORADDR;                  
            packetAddress=-1;  
            packetRecvLen=-1;
            continue;
        }                           
        
        if (mode==STATE_WAITFORADDR)
        {
            packetAddress=b;
            mode==STATE_WAITFORLEN;
            continue;
        }
        if (mode==STATE_WAITFORLEN)
        {
            packetRecvLen=b;            
            mode=STATE_HEADERCOMPLETE;                        
            continue;
        }
        if (mode==STATE_HEADERCOMPLETE)
        {
            if (packetCtrl==CTRL_GETADDR)
            {
                adr=packetAddress+1;                
                uartSendByte(&uart,packetCtrl);
                uartSendByte(&uart,adr);
                uartSendByte(&uart,nullByte); // LEN
                uartSendByte(&uart,nullByte); // CRC
                mode=STATE_WAITFORCTRL;
                continue;
            }
            
            if (adr!=packetAddress) // packet for me
            {                
                uartSendByte(&uart,packetCtrl);
                uartSendByte(&uart,packetAddress);
                uartSendByte(&uart,packetRecvLen); // LEN
                uartSendByte(&uart,b); // CRC
                mode=STATE_PASSTHRU;
                continue;
            }
            // from here on the packet is for me
            if (packetCtrl==CTRL_PING)
            {
                uartSendByte(&uart,packetCtrl);
                uartSendByte(&uart,adr);
                uartSendByte(&uart,nullByte); // LEN
                uartSendByte(&uart,nullByte); // CRC
                mode=STATE_WAITFORCTRL;
                continue;
            }
            if (packetCtrl==CTRL_DATATRANS || packetCtrl==CTRL_DATAACK)
            {
                mode=STATE_READPACKET;
                pckRecvPtr=0;
                continue;
            }            
        } // if header complete
        
        if (mode==STATE_PASSTHRU)
        {
            uartSendByte(&uart,b);
        }
        if (mode==STATE_READPACKET)
        {
            packetRecvBuffer[pckRecvPtr++]=b;
            if (pckRecvPtr==packetRecvLen)
                mode=STATE_WAITFORCRC;            
            continue;
        }
        if (mode==STATE_WAITFORCRC)
        {
            if(crcOk(b))
            {
                if (packetCtrl==CTRL_DATAACK)
                {
                    uartSendByte(&uart,packetCtrl);
                    uartSendByte(&uart,adr);
                    uartSendByte(&uart,nullByte); // LEN
                    uartSendByte(&uart,nullByte); // CRC
                }
                decodeRecvBuffer();
                dataAvailable=1;
            }
            mode=STATE_WAITFORCTRL;
            continue;                    
        }

        ledTact(&ledRx);
        ledTact(&ledTx);
    }
}
/**
 End of File
*/