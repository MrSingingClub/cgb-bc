

#include "intosc.h"
#include <xc.h>

void SetupInternalOscillator(unsigned char setBits ,unsigned char enpll)
{
    OSCCONbits.IRCF     = setBits & 7; // maskiere untere 3 bits
    OSCCONbits.SCS      = 0b00;
    OSCTUNEbits.PLLEN   = enpll; // Takt mal 4

    OSCCON2bits.MFIOSEL = (setBits & 8) >> 3;  // maskiere bit4 als 0/1
    OSCTUNEbits.INTSRC  = (setBits & 16) >> 4; // maksiere bit 5 als 0/1
}
