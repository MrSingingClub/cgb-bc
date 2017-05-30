/* 
 * File:   intosc.h
 * Author: dt
 *
 * Created on 6. September 2014, 15:15
 */

#ifndef INTOSC_H
#define	INTOSC_H

#define INTOSCLF31KHZ  (0b01000)
#define INTOSCMF31KHZ  (0b10000)
#define INTOSCHF31KHZ  (0b11000)
#define INTOSCMF250KHZ (0b01001)
#define INTOSCHF250KHZ (0b00001)
#define INTOSCMF500KHZ (0b01010)
#define INTOSCHF500KHZ (0b10010)
#define INTOSC1MHZ     (0b00011)
#define INTOSC2MHZ     (0b00100)
#define INTOSC4MHZ     (0b00101)
#define INTOSC8MHZ     (0b00110)
#define INTOSC16MHZ    (0b00111)
#define INTOSCENPLL    (0b1)
#define INTOSCDISPLL   (0b0)

#ifdef	__cplusplus
extern "C" {
#endif

    /*
     Benötigt  #pragma config FOSC=INTIO7 oder INTIO67
     */
    void SetupInternalOscillator(unsigned char setBits, unsigned char enpll);


#ifdef	__cplusplus
}
#endif

#endif	/* INTOSC_H */

