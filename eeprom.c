/*
 * File:   eeprom.c
 * Author: dt
 *
 * Created on 14. Mai 2016, 17:51
 */


#include <xc.h>
    //Funktion um Daten ins EEPROM zu schreiben

void EEWrite(unsigned char adresse, unsigned char wert){

    char orgGIE;
    char err;
    orgGIE=INTCONbits.GIE;
    
    EEADR=adresse;                                      //Adresse nach Adressregister
    EEDATA=wert;                                        //Daten ins Datenregister
    EECON1bits.EEPGD=0;                                 // Data EEprom Zugriff
    EECON1bits.CFGS=0; // not config registers, but address EEPROM
    EECON1bits.WREN=1;                                  //Schreibzugriff
    INTCONbits.GIE=0;

    while(EECON1bits.WR);                               //Auf Schreibende warten    
    EECON2=0x55;                                        //Sicherheitscode (Datenblatt!!!)
    EECON2=0xAA;                                        //Sicherheitscode (Datenblatt!!!)
    EECON1bits.WR=1;                                    //schreiben
    while(EECON1bits.WR);                               //Auf Schreibende warten
    err=EECON1bits.WRERR;               
    EECON1bits.WREN=0;                                  //Schreiben deaktivieren
    PIR2bits.EEIF=0;                                    //Write-Interrupt EEPROM löschen
    INTCONbits.GIE=orgGIE;
    

}

//Funktion um Daten aus dem EEPROM zu lesen

unsigned char EERead(unsigned char adresse){

    char orgGIE;
    char data;
    orgGIE=INTCONbits.GIE;
    
    INTCONbits.GIE=0;
    EECON1bits.CFGS=0;
    EECON1bits.EEPGD=0;                                 //EEprom Zugriff
    EEADR=adresse;                                      //Adresse setzen
    EECON1bits.RD=1;                                    //Lesezugriff
    INTCONbits.GIE=orgGIE;
    
    return EEDATA;
    
}

void EEWriteInt(unsigned char adresse,int data)
{
    EEWrite(adresse,(char)(data & 0xFF));
    EEWrite(adresse+1,(char)((data >> 8) & 0xFF));    
}

int EEReadInt(unsigned char adresse)
{
    char l;
    char h;
    l=EERead(adresse);
    h=EERead(adresse+1);
    return (int)((h << 8 ) | l);
}

void EEWriteLong(unsigned char adresse,long data)
{
    char chk;
    EEWrite(adresse,data & 0xFF);
    data>>=8;
    EEWrite(adresse+1,data &0xFF);
    data>>=8;
    EEWrite(adresse+2,data &0xFF);
    data>>=8;
    EEWrite(adresse+3,data &0xFF);
}

long EEReadLong(unsigned char adresse)
{
    char ll;
    char l;
    char h;
    char hh;
    long d;
    ll=EERead(adresse);
    l=EERead(adresse+1);
    h=EERead(adresse+2);
    hh=EERead(adresse+3);
    d=hh;
    d=d << 8 | h;
    d=d << 8 | l;
    d=d << 8 | ll;
    return d;
    
}