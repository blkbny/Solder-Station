/* 
 * File:   PIC32MX.h
 * Author: Sparky
 *
 * Created on ???????, 2013, ??? 30, 1:56
 */

#ifndef PIC32MX534F064H_H
#define	PIC32MX534F064H_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include <GenericTypeDefs.h>
#include <peripheral/timer.h>
#include <peripheral/adc10.h>
#include <peripheral/cmp.h>
#include <peripheral/i2c.h>
#include <peripheral/spi.h>
#include <peripheral/dma.h>

#ifdef _PIC32MX534F064H_C
#define P32_EXTERN
#else
#define P32_EXTERN extern
#endif

#define SYS_FREQ                    (80000000UL)
#define CORETIMER_FREQ              (SYS_FREQ/2UL)
#define PER_FREQ                    (SYS_FREQ/2UL)
#define I2C_CLOCK_FREQ              (400000UL)


P32_EXTERN void DelayTicks(UINT32 a);
#define _delay_us(a) DelayTicks(a*(CORETIMER_FREQ/1000000UL))
#define _delay_ms(a) DelayTicks(a*(CORETIMER_FREQ/1000UL))

P32_EXTERN unsigned int mcuSqrt(unsigned int);

//outputs
#define IND1        LATDbits.LATD11
#define IND2        LATCbits.LATC13
#define IND3        LATCbits.LATC14
#define HEATER      LATEbits.LATE6
#define HCH         LATEbits.LATE7
#define CBANDA      LATEbits.LATE2
#define CBANDB      LATFbits.LATF3
#define CHSEL1      LATEbits.LATE5
#define CHSEL2      LATEbits.LATE4
#define CHPOL       LATEbits.LATE3
#define LEDA        LATDbits.LATD7
#define LEDB        LATDbits.LATD6
#define LEDC        LATDbits.LATD4
#define LEDD        LATDbits.LATD1
#define LEDE        LATDbits.LATD2
#define LEDF        LATDbits.LATD5
#define LEDG        LATDbits.LATD3
#define OLED_VCC    LATCbits.LATC14
#define OLED_RES    LATDbits.LATD7
#define OLED_DC     LATDbits.LATD6
#define OLED_CS     LATDbits.LATD4
#define OLED_SCK    LATDbits.LATD1
#define OLED_SDI    LATDbits.LATD2
#define OLED_SDO    LATDbits.LATD3
#define ID_OUT      LATBbits.LATB15

//inputs
#define B1          PORTDbits.RD8
#define B2          PORTDbits.RD9
#define B3          PORTDbits.RD10
#define ID_IN       PORTBbits.RB15
#define NAP         PORTGbits.RG9
#define ID_3S       TRISBbits.TRISB15

#define SPEAKER     OC1CONbits.ON
#define SPKON       if(!SPEAKER)SPEAKER=1
#define SPKOFF      if(SPEAKER)SPEAKER=0

#define SDISDO_IO   {TRISDSET=0b100;TRISDCLR=0b1000;}
#define SDISDO_OI   {TRISDCLR=0b100;TRISDSET=0b1000;}
#define SDISDO_OO   {TRISDCLR=0b1100;}
#define SDI_IN      PORTDbits.RD2
#define SDI_OUT     LATDbits.LATD2
#define SDO_IN      PORTDbits.RD3
#define SDO_OUT     LATDbits.LATD3


//I2C devices
#define CPOT 0b01011110
#define GAINPOT 0b01011100
#define OFFADC 0b11000000
#define EEP 0b10100000

#define SDA PORTGbits.RG7
#define SDAOUT TRISGbits.TRISG7
#define SDALAT LATGbits.LATG7
#define SCL TRISGbits.TRISG8
#define SCLLAT LATGbits.LATG8

#define mcuReset() Reset();

P32_EXTERN void mcuInit1();
P32_EXTERN void mcuInit2();

P32_EXTERN void mcuJumpToBootLoader();

#define mcuDisableInterrupts() INTDisableInterrupts()
#define mcuEnableInterrupts() INTEnableInterrupts()
#define mcuRestoreInterrupts(a) INTRestoreInterrupts(a)

P32_EXTERN void mcuI2CReset();

#define COMPREF CVRCONbits.CVR

#define MAINS CM2CONbits.COUT

P32_EXTERN void mcuDCTimerReset();

P32_EXTERN void mcuInitISRTimer();
P32_EXTERN void mcuStopISRTimer();
P32_EXTERN void mcuStartISRTimer(unsigned int per);
#define mcuStartISRTimer_us(us) mcuStartISRTimer((us*(PER_FREQ/1000000))/256);

//DMA
#define mcuVBuffPos() DmaChnGetDstPnt(DMA_CHANNEL0)
#define mcuIBuffPos() DmaChnGetDstPnt(DMA_CHANNEL1)

//ADC channels
#define ADCH_RT ADC_CH0_POS_SAMPLEA_AN14
#define ADCH_ID ADC_CH0_POS_SAMPLEA_AN15
#define ADCH_TEMP ADC_CH0_POS_SAMPLEA_AN5
#define ADCH_VIN ADC_CH0_POS_SAMPLEA_AN3
#define ADCH_VSHUNT ADC_CH0_POS_SAMPLEA_AN4

#define mcuADCRefVdd() AD1CON2bits.VCFG=0b010
#define mcuADCRefVref() AD1CON2bits.VCFG=0b011
P32_EXTERN volatile int mcuADCRES;
P32_EXTERN void mcuADCStop();
P32_EXTERN void mcuADCStartManual();
P32_EXTERN void mcuADCRead(int ADCCH, int num);
P32_EXTERN int mcuADCReadWait(int ADCCH, int num);

#define mcuSPIOpen() SpiChnOpen(SPI_CHANNEL3,SPI_OPEN_MSTEN | SPI_OPEN_MODE8 | SPI_OPEN_CKP_HIGH, 4)
#define mcuSPIClose() SpiChnClose(SPI_CHANNEL3)
#define mcuSPISendByte(b) SpiChnPutC(SPI_CHANNEL3, b)
#define mcuSPISendBytes(b,n) SpiChnPutS(SPI_CHANNEL3,b,n)
#define mcuSPIIsBusy() SpiChnIsBusy(SPI_CHANNEL3)
P32_EXTERN void mcuSPIWait();

#define mcuI2CStart() {int i=INTDisableInterrupts(); I2CStart(I2C4); INTRestoreInterrupts(i); }
#define mcuI2CStop() {int i=INTDisableInterrupts(); I2CStop(I2C4); INTRestoreInterrupts(i); }
#define mcuI2CSendByte(b) {int i=INTDisableInterrupts(); I2CSendByte(I2C4,b); INTRestoreInterrupts(i); }
#define mcuI2CReceiverEnable() I2CReceiverEnable(I2C4, TRUE);
#define mcuI2CReceiverDisable() I2CReceiverEnable(I2C4, FALSE);
#define mcuI2CGetByte() I2CGetByte(I2C4);
#define mcuI2CSendAddrW(b) mcuI2CSendByte(b)
#define mcuI2CSendAddrR(b) mcuI2CSendByte(b | 1)
#define mcuI2CIsACK() I2CByteWasAcknowledged(I2C4)
#define mcuI2CACK() I2CAcknowledgeByte(I2C4, TRUE);
#define mcuI2CWakeUp() INTSetFlag(INT_I2C4)

#define mcuPIDWakeUp() INTSetFlag(INT_OC2)

#define mcuCompDisable() (mCMP2IntEnable(0), MAINS = MAINS, mCMP2ClearIntFlag() ) //, CM2CONCLR=CMP_ENABLE)
#define mcuCompEnableH2L() (MAINS = MAINS, mCMP2ClearIntFlag(), CM2CONCLR = 3 << _CM1CON_EVPOL0_POSITION, CM2CONSET = 2 << _CM1CON_EVPOL0_POSITION, mCMP2IntEnable(1))
#define mcuCompEnableL2H() (MAINS = MAINS, mCMP2ClearIntFlag(), CM2CONCLR = 3 << _CM1CON_EVPOL0_POSITION, CM2CONSET = 1 << _CM1CON_EVPOL0_POSITION, mCMP2IntEnable(1))

#define mcuIDIn()
#define mcuIDOut()

//USB Stack definitions
#define self_power          1
#define USB_BUS_SENSE       1


#undef P32_EXTERN

#ifdef	__cplusplus
}
#endif

#endif	/* PIC32MX_H */

