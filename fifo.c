#include "fifo.h"

void fifoInit(fifoType* fifo,unsigned char* buf,unsigned char size)
{
    fifo->buf=buf;
    fifo->maxPtr=size-1;
    fifo->status=FIFO_EMPTY;
    fifo->getPtr=fifo->maxPtr;
    fifo->putPtr=fifo->maxPtr;
}

char fifoPut(fifoType* fifo,char put)
{
    if ((fifo->status & FIFO_FULL)==FIFO_FULL)
    {
        return 0;
    }

    fifo->buf[fifo->putPtr]=put;
    fifo->status&=~FIFO_EMPTY; // buffer not empty

    fifo->putPtr--;
    if (fifo->putPtr > fifo->maxPtr)
    {
        fifo->putPtr=fifo->maxPtr;
    }

    if (fifo->putPtr==fifo->getPtr)
    {
        fifo->status|=FIFO_FULL;
    }
    return 1;
}

char fifoPutStr(fifoType* fifo,char* str)
{
    while (*str)
    {
        if (fifoPut(fifo,*str)==0)
        {
            return 0;
        }
        str++;
    }
    return 0;
}
char fifoPutStr0(fifoType* fifo,char* str)
{
    do
    {
        if (fifoPut(fifo,*str)==0)
        {
            return 0;
        }
    }
    while (*str++);
    return 0;
}

char fifoPutBuf(fifoType* fifo,char* buf,char size)
{
    while (size)
    {
        if (fifoPut(fifo,*buf)==0)
        {
            return 0;
        }
        buf++;
        size--;
    }
    return 0;
}

char fifoGet(fifoType* fifo,char* get)
{
    if ((fifo->status & FIFO_EMPTY)==FIFO_EMPTY)
    {
        return 0;
    }
    *get=fifo->buf[fifo->getPtr];
    fifo->status&=~FIFO_FULL; // buffer cannot be full
    
    fifo->getPtr--;
    if (fifo->getPtr > fifo->maxPtr)
    {
        fifo->getPtr=fifo->maxPtr;
    }

    if (fifo->getPtr==fifo->putPtr)
    {
        fifo->status|=FIFO_EMPTY;
    }
    return 1;
}

char fifoGetBuf(fifoType* fifo,char* buf,char size)
{
    char elem;
    char copied=0;
    if ((fifo->status & FIFO_EMPTY)==FIFO_EMPTY)
    {
        return 0;
    }
    while (fifoGet(fifo,&elem) && (size-->0))
    {
        *buf++=elem;
        copied++;
    }
    return copied;
}

void fifoWipeBuffer(fifoType* fifoBuf)
{
    fifoBuf->getPtr=fifoBuf->putPtr;
    fifoBuf->status=FIFO_EMPTY;
}

