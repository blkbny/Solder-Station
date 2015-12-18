/* 
 * File:   PID.h
 * Author: Sparky
 *
 * Created on ??????????, 2013, ?????? 21, 2:41
 */

#ifndef PID_H
#define	PID_H
#include <GenericTypeDefs.h>
#include "typedefs.h"
#include "ExtFloat.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define ADCAVG 3
    
#ifndef _PID_C
#define PID_H_EXTERN extern
#else
#define PID_H_EXTERN
#endif

    typedef struct {
        int cnt;
        int val;
    }t_WSDelta;

    typedef struct {
        UINT32 PWM;
        UINT32 PIDDuty;
        UINT32 PIDDutyFull;
        
        int Starting;
        int LastTTemp;
        int DestinationReached;
        int KeepOff;

        int NoSensor;
        int NoHeater;
        int NoHeaterCnt;
        int ShortCircuit;

        UINT32 OffDelay; //Delay between comparator threshold and zero cross point when heater is on, microseconds

        int ADCTemp[4];

        t_WSDelta WSDelta[8];
        int LastOn;
        int OnCnt;
        int OffCnt;
        int WSCorr;

        int CTemp[2];
        int PrbFCTemp;
        int PrbFCTemp1;
        SFLOAT FCTemp;
        int CHRes;

        int TAvg;
        int TAvgF[2];
        int TAvgP[2];

        int TBuff[(1<<ADCAVG)];
        unsigned int TBPos;
        INT32 TSlope;
        INT32 SlopeBuff[(1<<ADCAVG)];
        unsigned int SBPos;

        INT32 Slope[2];

        int Delta[2];

        int HInitData;          //1 when heater data is to be initialized, 0 otherwise
        int HNewData;           //1 when new data is written to HR,HP and HI, 0 otherwise
        int HR;                 //last heater resistance /10
        int HRAvg;              //averaged heater resistance /10
        int HP;                 //last heater power
        int HPAvg;              //averaged heater power
        int HV;                 //last heater voltage x12.19 ((1/28/3)*1024)
        int HVAvg;              //averaged heater voltage x12.19
        int HI;                 //last heater current x42.55
        int HIAvg;              //averaged heater current x42.55
        float CPolyX;           //current temperature polynomial argument (millivolts for TC, resistance(ohms) for resistive)
    } t_PIDVars;


PID_H_EXTERN volatile unsigned int PIDTicks;
PID_H_EXTERN volatile int CRTemp;
PID_H_EXTERN volatile int RTAvg;
PID_H_EXTERN volatile int CTTemp;

PID_H_EXTERN volatile t_PIDVars PIDVars[2];
    
PID_H_EXTERN void PIDInit();

PID_H_EXTERN void PID(int step);

#undef PID_H_EXTERN

#ifdef	__cplusplus
}
#endif

#endif	/* PID_H */

