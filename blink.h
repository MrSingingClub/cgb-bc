#ifndef _BLINK_
#define _BLINK_

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H
#endif
#include <xc.h> // include processor files - each processor file is guarded.  

#define LED_STATE_ON 1
#define LED_STATE_OFF 0
#define LED_MODE_MANUAL 0
#define LED_MODE_FLASH 1
#define LED_MODE_BLINK 2

typedef struct
{
    int count;
    char on;
    int onTime;
    int offTime;
    char bitMask;
    char mode;
} ledType;

void ledInit(ledType* led,int on, int off, char bm);
void ledOn(ledType* led);
void ledOff(ledType* led);
void ledBlinkOn(ledType* led);
void ledBlinkOff(ledType* led);
void ledFlash(ledType* led);
void ledTact(ledType* led);

#endif	/* XC_HEADER_TEMPLATE_H */

