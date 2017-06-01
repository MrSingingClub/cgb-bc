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

#define MAXBUF8LEN (105)
#define NULLBYTE (0)

unsigned char uartSendBuffer[132]; // fifo puffer zum datagramm senden per UART
unsigned char uartRecvBuffer[132]; // fifo puffer für vom UART empfangene Bytes
char packetRecvBuffer[127];  // datagramm puffer für empfangenes datagramm 7bit codieren
char packetSendBuffer[127]; // datagramm puffer für zu versendendes datagramm 7bit codieren
char spiRecvBuffer[MAXBUF8LEN];    // 8-bit Empfangspuffer für AC
char spiSendBuffer[MAXBUF8LEN];    // 8-bit Sendepuffer für AC

ledType ledRx;
ledType ledTx;

uartType uart;

char adr;
char mode;
char dataAvailable;

char spiReadWrite; // 1=read, 0=write
char spiRecvLen;
char spiRecvPtr;
char spiSendPtr;

char packetSendLen;

char packetCtrl;
char packetAddress;
char packetRecvLen;

char packetSendLen;
char packetSendCrc;
char packetSendEnable;

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
    INTCONbits.RBIE = 1;
    IOCB4=1; // ENable RB4 interrupt on change
    
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
    
    TRISBbits.RB2=0; // SPI Data Available
    TRISBbits.RB3=1; // SPI RD/'WR bit 1=AC reads, 0=AC writes
    TRISBbits.RB4=1; // SPI 'CS bit
    
                
    ledInit(&(ledRx),FLASH_COUNT,FLASH_COUNT,0b00000001);
    ledInit(&(ledTx),FLASH_COUNT,FLASH_COUNT,0b00000010);    
}

void InitUart()
{
    uartInit(&(uart),1,9600,64000000,uartSendBuffer,160,uartRecvBuffer,160);
    //uartInitHalfDuplex(&(uart),&LATB,5);

    uartEnableSend(&(uart),1);
    uartEnableRecv(&(uart),1);
}

void InitSPI()
{
    TRISAbits.RA5=1; // SPI SS
    TRISCbits.RC5=0; //SDO
    TRISCbits.RC4=1; //SDI
    TRISCbits.RC3=1; //SCLK
    SSP1STAT=0b01000000;
    SSP1CON1=0b00100100;
    
}
char crcOk(char crc)
{
    return 1;
}

char crcCalc()
{
    return 0;
}

char encodeSendBuffer() // returns number of encoded bytes
{
    
}
char decodeRecvBuffer() // returns number of decoed bytes
{
    
}

void spiInterruptHandler()
{
    if (RBIF) // SPI 'CS Flag
    {    
        if (PORTBbits.RB4==0) // got new CS Low
        {
            spiReadWrite=PORTBbits.RB3;
            if (spiReadWrite) // AC wants to read, write byte to SPI
            {
                SSP1BUF=spiRecvBuffer[spiRecvPtr++]; // put first byte to SPI                
                if (spiRecvPtr==spiRecvLen)
                {
                    LATB2=0;
                }
            }
            else // AC wants to write, prepare buffer
            {
                spiSendPtr=0;
                SSP1BUF=spiSendPtr;
            }
        }
        if (PORTBbits.RB4==1) // CS is over
        {
            if (spiReadWrite) // AC has read
            {
                spiRecvPtr=0;
                LATB2=0;
            }
            else // AC has written, prepare sending via UART
            {
                packetSendLen=encodeSendBuffer();    //packetReady>0 signal main routine to send            
                packetSendCrc=crcCalc();
                packetSendEnable=1;
                spiSendPtr=0;
            }            
        }
        RBIF=0;
    }
    
    if (SSP1IF)
    {
        if (spiReadWrite) //AC reads
        {
            if (spiRecvPtr<spiRecvLen)
            {
                SSP1BUF=spiRecvBuffer[spiRecvPtr++];
            }
            if (spiRecvPtr==spiRecvLen)
            {
                LATB2=0; // no more data avaiable
                spiRecvPtr=0;
                spiRecvLen=0;
            }            
        }
        else // AC writes
        {
            if (spiSendPtr<MAXBUF8LEN)
            {
                spiSendBuffer[spiSendPtr++]=SSP1BUF;
                SSP1BUF=spiSendPtr;
            }            
        }
        SSP1IF=0;
    }
}

void interrupt high_priority IntHighRoutine()
{
    INTCONbits.GIE=0;
 
    uartInterruptHandler(&(uart));
    spiInterruptHandler();
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

void setDataAvailable()
{
    LATB2=1;
}

void main(void)
{     
    char b;
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
            if (packetSendEnable) // check if something to send
            {
                uartSendByte(&uart,CTRL_DATATRANS);
                uartSendByte(&uart,adr); //my address
                uartSendByte(&uart,packetSendLen); // number of 
                uartSendBuf(&uart,packetSendBuffer,packetSendLen);
                uartSendByte(&uart,packetSendCrc);
                mode=STATE_SEND;
                packetSendEnable=0;                   
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
                uartSendByte(&uart,NULLBYTE); // LEN
                uartSendByte(&uart,NULLBYTE); // CRC
                mode=STATE_WAITFORCTRL;
                continue;
            }
            
            if (adr!=packetAddress) // packet not for me
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
                uartSendByte(&uart,NULLBYTE); // LEN
                uartSendByte(&uart,NULLBYTE); // CRC
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
                    uartSendByte(&uart,NULLBYTE); // LEN
                    uartSendByte(&uart,NULLBYTE); // CRC
                }
                // Todo: Konflikt, wenn AC noch nicht gelesen hat
                decodeRecvBuffer();
                setDataAvailable();
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