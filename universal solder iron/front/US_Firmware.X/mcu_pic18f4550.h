/* 
 * File:   devpins.h
 * Author: Sparky
 *
 * Created on ?????????, 2013, ??? 16, 18:59
 */

#ifndef DEVPINS_H
#define	DEVPINS_H

#ifdef	__cplusplus
extern "C" {
#endif

#define _XTAL_FREQ 48000000L   //Crystal frequency
#define F_CPU 12000000L        //CPU Frequency

#define B1 PORTAbits.RA5
#define B2 PORTEbits.RE0
#define B3 PORTEbits.RE1
#define SPK LATCbits.LATC2
#define IND1 LATEbits.LATE2
#define IND2 LATCbits.LATC0
#define IND3 LATCbits.LATC1
#define HEATER LATDbits.LATD5
#define NAP PORTBbits.RB5
#define CHSEL LATDbits.LATD7
#define CBAND LATDbits.LATD6
#define IRONID LATAbits.LATA4

#define LEDB LATDbits.LATD4
#define LEDA LATCbits.LATC7
#define LEDC LATDbits.LATD3
#define LEDD LATDbits.LATD0
#define LEDE LATDbits.LATD1
#define LEDF LATCbits.LATC6
#define LEDG LATDbits.LATD2

#define SDA PORTBbits.RB0
#define SDAOUT TRISBbits.TRISB0
#define SCL TRISBbits.TRISB1

#define MAINS CMCONbits.C1OUT
#define COMPREF CVRCONbits.CVR
#define COMPREFBAND CVRCONbits.CVRR
    
//ADC channels
#define ADCH_RT 2
#define ADCH_ID 4
#define ADCH_TEMP 8
#define ADCH_VIN 11

#define _readADC(lch)\
{\
    ADCON0bits.CHS=lch;\
    ADCON0bits.GODONE=1;\
}
#define ADCRESULT ADRES

//I2C devices
#define CPOT 0b01011110
#define GAINPOT 0b01011100
#define OFFADC 0b11000000
#define EEP 0b10100000

#define _resetI2C()\
{\
    LATBbits.LATB0=0;\
    LATBbits.LATB1=0;\
    SCL=1;\
    SDAOUT=1;\
    __delay_us(100);\
    while(SDA==0){\
        SCL=0;\
        __delay_us(100);\
        SCL=1;\
        __delay_us(100);\
    }\
    SDAOUT=0;\
    __delay_us(100);\
    SDAOUT=1;\
    __delay_us(100);\
}

#define _timer_ms(a) ((UINT16)(((float)_XTAL_FREQ*(float)a)/(4.0*8.0*1000.0)))
#define _timer_us(a) ((UINT16)(((float)_XTAL_FREQ*(float)a)/(4.0*8.0*1000000.0)))


#ifdef	__cplusplus
}
#endif

#endif	/* DEVPINS_H */

