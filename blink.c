/*
 * File:   blink.c
 * Author: dt
 *
 * Created on 10. August 2016, 18:52
 */


#include <xc.h>
#include "blink.h"

void ledInit(ledType* led,int on, int off, char bm)
{
    led->onTime=on;
    led->offTime=off;
    led->bitMask=bm;
    led->on=0;
    led->count=0;
}

void ledOn(ledType* led)
{
    LATA|=led->bitMask;
    led->on=1;
    led->mode=LED_MODE_MANUAL;
}

void ledOff(ledType* led)
{
    LATA&=~led->bitMask;
    led->on=0;
    led->mode=LED_MODE_MANUAL;
}

void ledBlinkOn(ledType* led)
{
    led->mode=LED_MODE_BLINK;
    led->count=0;
}
void ledBlinkOff(ledType* led)
{
    led->mode=LED_MODE_MANUAL;
    led->count=0;
}

void ledFlash(ledType* led)
{
    led->mode=LED_MODE_FLASH;
    led->count=0;
}

void ledTact(ledType* led)
{    
    if (led->count>0) led->count--;
    if (led->count==0)
    {
        if (led->on)
        {
            LATA&=~led->bitMask; // clear bit;
            led->count=led->offTime;
            led->on=0;
        }
        else
        {            
            if (led->mode==LED_MODE_FLASH || led->mode==LED_MODE_BLINK) LATA|=led->bitMask; // set bit
            led->count=led->onTime;
            led->on=1;
            if (led->mode==LED_MODE_FLASH) led->mode=LED_MODE_MANUAL;
        }
    }    
}
