/* 
 * File:   io.h
 * Author: Sparky
 *
 * Created on ?????, 2013, ??? 28, 23:18
 */

#ifndef IO_H
#define	IO_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <xc.h>
#include "iron.h"

typedef union {
    UINT8 RAWData[64];
    struct  __PACKED {
        UINT8 Command;
        union{
            UINT8 Data[63];
            struct __PACKED {
                unsigned char PacketDataFieldSize;
                unsigned char BytesPerAddress;
            }QueryDev;
            struct __PACKED {
                SUINT16 PID_KP;
                SUINT16 PID_KI;
                UINT8 PID_DGain;
                UINT8 PID_OVFGain;
                SUINT16 Gain;
            }IronPars;
            struct __PACKED {
                UINT16 Ticks;
                UINT8 CTTemp;
                UINT16 CTemp;
                UINT16 ADCTemp;
                UINT16 TAvgF;
                UINT16 CHRes;
                UINT16 TAvgP;
                UINT8 Heater;
                INT16 WSDelta[8] __PACKED;
                UINT8 DestinationReached;
                UINT16 Duty;
                UINT16 Slope;
            }LiveData;
        };
    };
}USBPacket;


#ifndef _IO_C
#define IOC_EXTERN extern
extern volatile USBPacket RXP;
extern volatile USBPacket TXP;

#else
#define IOC_EXTERN
#endif

IOC_EXTERN void IOInit();
IOC_EXTERN void IOTasks();


#undef IOC_EXTERN


#ifdef	__cplusplus
}
#endif

#endif	/* IO_H */

