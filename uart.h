/* 
 * File:   uart.h
 * Author: dt
 *
 * Created on 10. September 2014, 22:36
 */

#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "fifo.h"

#define UART_SENDBUFFULL   1
#define UART_SENDBUFEMPTY  2
#define UART_RECVBUFFULL   4
#define UART_RECVBUFEMPTY  8
#define UART_FRAMEERROR   16
#define UART_OVRRUNERROR  32


typedef struct
{
    fifoType  sendFifo;
    fifoType  recvFifo;
    char      SendEnabled;
    char      RecvEnabled;
    volatile unsigned char* txReg;
    volatile unsigned char* rxReg;
    char      status;
    unsigned char      number;
    char      hdFlag; // half duplex enable;
    volatile unsigned char* hdPort; // half duplex digital output port
    char      hdOnMask;
    char      hdOffMask;
} uartType;

void uartInit(uartType* uart, char num, long baud,long osc,unsigned char* sendbuf,unsigned char sendSize,unsigned char* recvbuf,unsigned char recvSize);
char SetBaudRate(uartType* uart,long rate, long osc);

void uartInitHalfDuplex(uartType* uart,volatile unsigned char* port, unsigned char bt);
void uartEnableSend(uartType* uart,char onoff);
void uartEnableRecv(uartType* uart,char onoff);
void uartEnableHalfDuplex(uartType* uart, unsigned char onoff);
char uartSendByte(uartType* uart,unsigned char snd);
char uartSendString(uartType* uart,unsigned char* str);
char uartSendString0(uartType* uart,unsigned char* str);
char uartSendBuf(uartType* uart,unsigned char* str,unsigned char size);
char uartRecvByte(uartType* uart,unsigned char* data);
char uartRecvBuf(uartType* uart,char* data,char size);
void uartInterruptHandler(uartType* uart);
void uartInterruptHandler(uartType* uart);
void uartTransmitAll(uartType* uart);
char uartStatus(uartType* uart);
char uartIsTransmitting(uartType* uart);
char uartSendBufferFull(uartType* uart);
char uartRecvBufferFull(uartType* uart);




#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

