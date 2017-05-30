#ifndef _EEPROM_
#define _EEPROM_

/*
 * File:   eeprom.c
 * Author: dt
 *
 * Created on 14. Mai 2016, 17:51
 */


#include <xc.h>
    //Funktion um Daten ins EEPROM zu schreiben

void EEWrite(unsigned char adresse, unsigned char wert);
unsigned char EERead(unsigned char adresse);
void EEWriteInt(unsigned char adresse,int data);
int EEReadInt(unsigned char adresse);
void EEWriteLong(unsigned char adresse,long data);
long EEReadLong(unsigned char adresse);

#endif