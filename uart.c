#include "uart.h"
#include <xc.h>

void uartInit(uartType* uart, char num, long baud,long osc,unsigned char* sendbuf,unsigned char sendSize,unsigned char* recvbuf,unsigned char recvSize)
{
    uart->number=num;
    char l;
    char h;
    char b16;
    
    switch (num)
    {
        case 1:

        uart->txReg=&TXREG1;
        uart->rxReg=&RCREG1;
        
        ANSELCbits.ANSC6=0;
        ANSELCbits.ANSC7=0;
        TRISCbits.RC6=1;
        TRISCbits.RC7=1;


        TXSTA1bits.BRGH=0;
        TXSTA1bits.SYNC=0;
        TXSTA1bits.TX9=0;
        TXSTA1bits.TXEN=1;
        RCSTA1bits.SPEN=1;
        RCSTA1bits.CREN=1;

        SetBaudRate(uart,baud,osc);

        IPR1bits.TXIP=1; // tx high priority
        IPR1bits.RCIP=1; // rx high prio
        break;

        case 2:
        uart->txReg=&TXREG2;
        uart->rxReg=&RCREG2;
        
        TRISBbits.RB6=1;
        TRISBbits.RB7=1;

        TXSTA2bits.BRGH=0;
        TXSTA2bits.SYNC=0;
        TXSTA2bits.TX9=0;
        TXSTA2bits.TXEN=1;
        RCSTA2bits.SPEN=1;
        RCSTA2bits.CREN=1;

        SetBaudRate(uart,baud,osc);

        IPR3bits.TX2IP=1; // tx high priority
        IPR3bits.RC2IP=1; // rx high prio
        break;
    }

    uart->status=0;

    fifoInit(&uart->sendFifo,sendbuf,sendSize);
    fifoInit(&uart->recvFifo,recvbuf,recvSize);
}

char GetBaudRateConfig(long rate,long osc,char* setBRG16,char* brgh,char* brgl)
{
    int brg=0;
    // Tabelle muss für jede taktfrequenz erstellt werden
    if (osc==64000000)
    {
        *setBRG16=1;
        switch (rate)
        {
            case 300 : brg=13332;break;
            case 1200 : brg=3332;break;
            case 2400 : brg=1666;break;
            case 9600 : brg=416;break;
            case 10417 : brg=383;break;
            case 19200 : brg=207;break;
            case 57600 : brg=68;break;
            case 115200 : brg=34;break;
            default : return 0;
        }        
    }
    else if (osc==1000000)
    {
        *setBRG16=1;
        switch (rate)
        {
            case 300 : brg=207;break;
            case 1200 : brg=51;break;
            case 2400 : brg=25;break;            
            case 10417 : brg=5;break;
            default : return 0;
        }
    }
    else
    {
        return 0;
    }
    *brgh=(char)((brg >> 8) & 0xff);
    *brgl=(char)(brg & 0xff);
    return 1;
}

char SetBaudRate(uartType* uart,long rate, long osc)
{
    char brg16;
    char h;
    char l;
    char o;
    o=GetBaudRateConfig(rate,osc,&brg16,&h,&l);
    if (o==0) return 0;
    
    switch (uart->number)
    {
        case 1 : 
            BAUDCON1bits.BRG16=brg16;
            SPBRGH1=h;
            SPBRG1=l;
            break;
        case 2 :
            BAUDCON2bits.BRG16=brg16;
            SPBRGH2=h;
            SPBRG2=l;
            break;
    }
    return 1;
}

void uartInitHalfDuplex(uartType* uart,volatile unsigned char* port, unsigned char bt)
{
    uart->hdPort=port;
    uart->hdOnMask=1 << bt;
    uart->hdOffMask=~uart->hdOnMask;
    uart->hdFlag=1;
    *(uart->hdPort) &= uart->hdOffMask; // Ausgang auf Lesen setzen
}

void uartEnableSend(uartType* uart,char onoff)
{
    switch (uart->number)
    {
        case 1: PIE1bits.TX1IE=onoff & 1;break;
        case 2: PIE3bits.TX2IE=onoff & 1;break;
    }

}
void uartEnableRecv(uartType* uart,char onoff)
{
    switch (uart->number)
    {
        case 1: PIE1bits.RC1IE=onoff & 1;break;
        case 2: PIE3bits.RC2IE=onoff & 1;break;
    }
}
void uartEnableHalfDuplex(uartType* uart,unsigned char onoff)
{
    uart->hdFlag=onoff;
}
char uartSendByte(uartType* uart,unsigned char snd)
{
    unsigned char res;
    uartEnableSend(uart,0);
    res=fifoPut(&uart->sendFifo,snd);
    uartEnableSend(uart,1);

    return res;
}
char uartSendString(uartType* uart,unsigned char* str)
{
    unsigned char res;
    uartEnableSend(uart,0);
    res=fifoPutStr(&uart->sendFifo,str);
    uartEnableSend(uart,1);
    return res;
}
char uartSendString0(uartType* uart,unsigned char* str)
{
    unsigned char res;
    uartEnableSend(uart,0);
    res=fifoPutStr0(&uart->sendFifo,str);
    uartEnableSend(uart,1);
    return res;
}
char uartSendBuf(uartType* uart,char* str,char size)
{
    unsigned char res;
    uartEnableSend(uart,0);
    res=fifoPutBuf(&uart->sendFifo,str,size);
    uartEnableSend(uart,1);
    return res;
}

char uartRecvByte(uartType* uart,unsigned char* data)
{
    return fifoGet(&uart->recvFifo,data);
}

char uartRecvBuf(uartType* uart,char* data,char size)
{
    return fifoGetBuf(&uart->recvFifo,data,size);
}



void uartInterruptHandler(uartType* uart)
{
    char iTxFlag;
    char iTxEnable;
    char iRxFlag;
    char iRxEnable;
    char fErr;
    char oErr;
    volatile unsigned char* rcsta;
    volatile unsigned char* txsta;

    char data;

    switch (uart->number)
    {
        case 1:
            iTxFlag=PIR1bits.TX1IF;
            iTxEnable=PIE1bits.TX1IE;
            iRxFlag=PIR1bits.RC1IF;
            iRxEnable=PIE1bits.RCIE;
            fErr=RCSTA1bits.FERR;
            oErr=RCSTA1bits.OERR;
            rcsta=&RCSTA1;
            txsta=&TXSTA1;
            break;
        case 2:
            iTxFlag=PIR3bits.TX2IF;
            iTxEnable=PIE3bits.TX2IE;
            iRxFlag=PIR3bits.RC2IF;
            iRxEnable=PIE3bits.RC2IE;
            fErr=RCSTA2bits.FERR;
            oErr=RCSTA2bits.OERR;
            rcsta=&RCSTA2;
            txsta=&TXSTA2;
            break;
    }
    if (iTxFlag ) // Transmission register TSR is empty
    {
        if ( fifoGet(&(uart->sendFifo),&data) ) // has data to send
        {
            // half duplex mode needs send flag output
            if (uart->hdFlag )
            {
                *(uart->hdPort) |= uart->hdOnMask;
            }
            *uart->txReg=data;
        }
        else // no more data to transmit
        {
            // hafl duplex mode: switch to receive
            if (uart->hdFlag)
            {
                while ((*txsta & 2)==0) // wait until TMRT bit is high
                {}
                *(uart->hdPort) &= uart->hdOffMask;
            }
            uartEnableSend(uart,0);
        }
    }

    while (iRxEnable & iRxFlag)
    {
        if (fErr)
        {
            uart->status|=UART_FRAMEERROR;
            break;
        }
        else
        {
            if (oErr)
            {
                uart->status|=UART_OVRRUNERROR;
                *rcsta&=~0x10;
                *rcsta|=0x10;
                break;
            }
            else
            {
                data=*uart->rxReg;
                fifoPut(&uart->recvFifo,data);
            }
        }
        
        switch (uart->number)
        {
            case 1:
                iRxFlag=PIR1bits.RC1IF;
                break;
            case 2:
                iRxFlag=PIR3bits.RC2IF;
                break;
        }
    }
}

void uartTransmitAll(uartType* uart)
{
    unsigned char data;

    while (fifoGet(&uart->sendFifo,&data))
    {
        if (uart->number==1)
        {
            while (!PIR1bits.TX1IF)
            {}
            *uart->txReg=data;
        }
        if (uart->number==2)
        {
            while (!PIR3bits.TX2IF)
            {}
            *uart->txReg=data;
        }
    }
}

char uartStatus(uartType* uart)
{
    char res;
    res=uart->status | uart->sendFifo.status | (uart->recvFifo.status << 2 );
    return res;
}

char uartSendBufferFull(uartType* uart)
{
    return (uart->sendFifo.status & FIFO_FULL);
}
char uartRecvBufferFull(uartType* uart)
{
    return (uart->recvFifo.status & FIFO_FULL);
}

char uartIsTransmitting(uartType* uart)
{
    return ( (*(uart->hdPort) & uart->hdOnMask) );
}