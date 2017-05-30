/* 
 * File:   fifo.h
 * Author: dt
 *
 * Created on 10. September 2014, 13:55
 */

#ifndef FIFO_H
#define	FIFO_H

#ifdef	__cplusplus
extern "C" {
#endif

#define FIFO_FULL 1
#define FIFO_EMPTY 2

typedef struct
{
    unsigned char* buf;
    unsigned char maxPtr;
    unsigned char status;
    unsigned char putPtr;
    unsigned char getPtr;
} fifoType;

void fifoInit(fifoType* fifo,unsigned char* buf,unsigned char size);
char fifoPut(fifoType* fifobuf,unsigned char put);
char fifoPutStr(fifoType* fifo,unsigned char* str);
char fifoPutStr0(fifoType* fifo,char* str);
char fifoPutBuf(fifoType* fifo,char* buf,char size);
char fifoGet(fifoType* fifoBuf,char* get);
char fifoGetBuf(fifoType* fifoBuf,char* buf,char size);
void fifoWipeBuffer(fifoType* fifiBuf);


#ifdef	__cplusplus
}
#endif

#endif	/* FIFO_H */

