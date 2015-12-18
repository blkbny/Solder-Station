/* 
 * File:   main.c
 * Author: Sparky
 * Compiler: Microchip XC8 1.12
 *
 * Created on ??????, 2013, ?????? 5, 3:15
 */
#define _MAIN_C

#include "mcu.h"

#include <stddef.h>
#include <GenericTypeDefs.h>
#include <peripheral/int.h>
#include <peripheral/outcompare.h>
#include <peripheral/timer.h>
#include <peripheral/spi.h>
#include <peripheral/i2c.h>
#include <peripheral/cvref.h>
#include <peripheral/cmp.h>
#include <peripheral/adc10.h>
#include <peripheral/ports.h>
#include "typedefs.h"

#include "usb/usb.h"
#include "usb/usb_function_hid.h"

#include "main.h"

#include "disp.h"
#include "isr.h"
#include "iron.h"
#include "PID.h"
#include "menu.h"
#include "io.h"
#include "EEP.h"
#include "OLED.h"
#include "pars.h"


volatile unsigned int   BeepTicks;

volatile unsigned int   MAINS_PER;                  //mains voltage period
volatile unsigned int   T_PER;                      //ISR Timer period for mains frequency/1.2
volatile unsigned int   C_PER;                      //Character display period

volatile mainflags_t mainFlags = {1, 1, 1, 1, 0};

/****** PARAMETERS SAVED IN EEPROM WHEN POWER IS LOST**************************/
volatile unsigned int   TTemp;                      //target temperature/2
volatile pars_t         pars;
/******************************************************************************/

void LoadPars(void)
{
    int i;
    UINT8 b,oldb;

    EEPRead(0, (UINT8 *)&pars, sizeof(pars));
    for(i = 0; i < sizeof(pars); i++){
        if((pars.b[i] < ParDef[i].Min) || (pars.b[i] > ParDef[i].Max)){
            for(i = 0; i < sizeof(pars); i++){
                pars.b[i] = ParDef[i].Default;
            }
            break;
        }
    }

    TTemp = 150;
    oldb=EEPRead(63 + 64, 0, 1);
    for(i = 0; i < 64; i++){
        b = EEPRead(i + 64, 0, 1);
        if((oldb == 0xFF) && (b >= 75) && (b <= 225)){
            TTemp = b;
            break;
        }
        oldb = b;
    }
}


void SavePars(void)
{
    int i;
    UINT8 b, oldb;

    for(i = 0; i < sizeof(pars); i++){
        if(EEPRead(i, 0 ,1) != pars.b[i])EEPWriteImm(i, pars.b[i]);
    }
    oldb = EEPRead(63 + 64, 0, 1);
    for(i = 0; i < 64; i++){
        b = EEPRead(i + 64, 0, 1);
        if((oldb==0xFF) && (b >= 75) && (b <= 225))break;
        oldb = b;
    }
    i &= 63;
    b=EEPRead(i + 64, 0, 1);
    if(b != TTemp){
        EEPWriteImm(i + 64, 0xFF);
        EEPWriteImm(((i + 1) & 63) + 64, TTemp);
    }
}


void main(void){
    int i;

    mcuInit1();

    i=20;
    OLED_CS=1;
    OLED_DC=1;
    OLED_RES=0;
    while(i){
        i--;
        SDISDO_IO;
        SDO_OUT=1;
        _delay_ms(1);
        if(SDI_IN!=1)break;
        SDO_OUT=0;
        _delay_ms(1);
        if(SDI_IN!=0)break;
        SDISDO_OI;
        SDI_OUT=1;
        _delay_ms(1);
        if(SDO_IN!=1)break;
        SDI_OUT=0;
        _delay_ms(1);
        if(SDO_IN!=0)break;
    }
    SDI_OUT=0;
    SDO_OUT=0;
    SDISDO_OO;
    OLED_RES=1;
    if(i)mainFlags.OLED=0;

    mcuInit2();

    if(mainFlags.OLED){
        mcuSPIOpen();
        OLEDInit();
    }

    SPEAKER=0;

    OpenTimer1(T1_ON | T1_IDLE_STOP | T1_TMWDIS_OFF | T1_GATE_OFF | T1_PS_1_256 | T1_SYNC_EXT_OFF | T1_SOURCE_INT,0xFFFF);
    ConfigIntTimer1(T1_INT_OFF | T1_INT_PRIOR_7 | T1_INT_SUB_PRIOR_3);
    _delay_ms(100);

    mainFlags.ACPower = 0;
    MAINS_PER = 0;
    mcuDCTimerReset();
    while(MAINS && (mT1GetIntFlag() == 0));
    _delay_us(100);
    while((!MAINS) && (mT1GetIntFlag()==0));
    mcuDCTimerReset();
    for(i=0;((i<8) && (mT1GetIntFlag()==0));i++){
        _delay_us(100);
        while(MAINS && (mT1GetIntFlag()==0)){};
        _delay_us(100);
        while((!MAINS) && (mT1GetIntFlag()==0)){};
        MAINS_PER=ReadTimer1();
    }
    if(mT1GetIntFlag()){
        MAINS_PER=(PER_FREQ/256)/110;        //55Hz
        T_PER=MAINS_PER;
    }
    else{
        MAINS_PER>>=3;
        T_PER=MAINS_PER+((PER_FREQ/256)/1000);
        mainFlags.ACPower=1;
    }
    C_PER=(MAINS_PER-(PER_FREQ/256)/500)>>2;
    CloseTimer1();
    OpenTimer1(T1_ON | T1_IDLE_STOP | T1_TMWDIS_OFF | T1_GATE_OFF | T1_PS_1_256 | T1_SYNC_EXT_OFF | T1_SOURCE_INT, T_PER);

    SPEAKER=0;
    IronInit();
    ISRInit();
    PIDInit();
    IOInit();
    MenuInit();

    if(mainFlags.ACPower){
        while(MAINS);
        while(!MAINS);
    }
    mcuDCTimerReset();

    CMP2ConfigInt(CMP_INT_PRIOR_7 | CMP_INT_SUB_PRI_3 | CMP_INT_ENABLE);
    ConfigIntADC10(ADC_INT_PRI_7 | ADC_INT_SUB_PRI_3 | ADC_INT_ON);
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_7 | T1_INT_SUB_PRIOR_3);

    INTClearFlag(INT_I2C4B);
    INTClearFlag(INT_I2C4M);
    INTClearFlag(INT_I2C4S);
    INTClearFlag(INT_I2C4);
    INTSetVectorPriority(INT_I2C_4_VECTOR, INT_PRIORITY_LEVEL_6);
    INTSetVectorSubPriority(INT_I2C_4_VECTOR, INT_SUB_PRIORITY_LEVEL_3);
    INTEnable(INT_I2C4B,INT_ENABLED);
    INTEnable(INT_I2C4M,INT_ENABLED);
    INTEnable(INT_I2C4S,INT_ENABLED);
    INTEnable(INT_I2C4,INT_ENABLED);

    INTClearFlag(INT_OC2);
    INTSetVectorPriority(_OUTPUT_COMPARE_2_VECTOR, INT_PRIORITY_LEVEL_5);
    INTSetVectorSubPriority(_OUTPUT_COMPARE_2_VECTOR, INT_SUB_PRIORITY_LEVEL_3);
    INTEnable(INT_OC2,INT_ENABLED);

    mcuInitISRTimer();
    INTEnableSystemMultiVectoredInt();

    CBANDA=1;
    CBANDB=1;
    CHSEL1=0;
    CHSEL2=1;
    CHPOL=0;
    I2CData.CurrentA.ui16=128;
    I2CData.CurrentB.ui16=128;
    I2CData.Gain.ui16=128;
    I2CData.Offset.ui16=128;
    I2CAddCommands(I2C_SET_CPOT | I2C_SET_GAINPOT | I2C_SET_OFFSET);

    LoadPars();

    ISRStop();

    mainFlags.PowerLost = 0;

    ISRStart();

    while(1){
        if(mainFlags.PowerLost){
            SavePars();
            MenuTasks();
            _delay_ms(1000);
            mcuReset();
            while(1);
        }
        MenuTasks();
        IOTasks();
        IronTasks();
    }
}

#undef _MAIN_C