#define _IO_C

#include <xc.h>
#include "typedefs.h"
#include "mcu.h"
#include "io.h"
#include "pars.h"
#include "main.h"
#include "PID.h"
#include "isr.h"
#include "iron.h"
#include "usb/usb.h"
#include "usb/usb_driver.h"
#include "usb/usb_function_hid.h"

#define TXP (*((USBPacket *)USBTxBuffer))
#define RXP (*((USBPacket *)USBRxBuffer))

#define IO_IDLE 0
#define IO_BUSY 1

static UINT8 IO_STATUS;
static unsigned int IO_TICKS;

void ProcessIO();

void IOInit(){
    IO_TICKS=ISRTicks;
    IO_STATUS=IO_BUSY;
    USBDriverInit();
}

void IOTasks(){
    USBDeviceTasks();
    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;
    ProcessIO();   // This is where all the actual bootloader related data transfer/self programming takes place
}

void ProcessIO(){
    UINT8 i;
    if(IO_TICKS!=ADCStep) { //IO_TICKS.ui16!=PIDTicks.ui16){
        if(!HIDTxHandleBusy(USBInHandle)){
            IO_TICKS=ADCStep;
            if(IO_TICKS & 1){
                i=mcuDisableInterrupts();
                TXP.Command=3;
                TXP.LiveData.Ticks=IO_TICKS;
                TXP.LiveData.CTTemp=CTTemp;                                 //
                TXP.LiveData.CTemp=PIDVars[0].CTemp[0];                                //
                TXP.LiveData.ADCTemp=PIDVars[0].ADCTemp[0];                            //
                TXP.LiveData.TAvgF=PIDVars[0].TAvgF[0]>>ADCAVG;                        //
                TXP.LiveData.CHRes=PIDVars[0].HRAvg>>ADCAVG;
                TXP.LiveData.TAvgP=PIDVars[0].TAvgP[0];
                TXP.LiveData.Heater=PHEATER;
                TXP.LiveData.WSDelta[0] = PIDVars[0].WSDelta[0].val + 2048;                //
                TXP.LiveData.WSDelta[1] = PIDVars[0].WSDelta[1].val + 2048;   //
                TXP.LiveData.WSDelta[2] = PIDVars[0].WSDelta[2].val + 2048;   //
                TXP.LiveData.WSDelta[3] = PIDVars[0].WSDelta[3].val + 2048;   //
                TXP.LiveData.WSDelta[4] = PIDVars[0].WSDelta[4].val + 2048;   //
                TXP.LiveData.WSDelta[5] = PIDVars[0].WSDelta[5].val + 2048;   //
                TXP.LiveData.WSDelta[6] = PIDVars[0].WSDelta[6].val + 2048;   //
                TXP.LiveData.WSDelta[7] = PIDVars[0].WSDelta[7].val + 2048;   //
                TXP.LiveData.DestinationReached=PIDVars[0].DestinationReached;         //
                TXP.LiveData.Duty = (UINT16)(PIDVars[0].PIDDutyFull>>8);                       //
                TXP.LiveData.Slope= (UINT16)(PIDVars[0].Slope[0]+2048);
                USBInHandle = HIDTxPacket(HID_EP, (char *)&TXP, 64);
                mcuRestoreInterrupts(i);
            }
        }
    }
    else {
        //if(IO_STATUS==IO_IDLE){
            if(!HIDRxHandleBusy(USBOutHandle)){
                IO_STATUS=IO_BUSY;
          //  }
        //}
        //else{
            switch(RXP.Command){
                case 0x81:
                    mcuJumpToBootLoader();
                    break;
                case 2:
                    BeepTicks=5;
                    if(!HIDTxHandleBusy(USBInHandle)){
                        TXP.Command=2;
                        TXP.QueryDev.PacketDataFieldSize=64;
                        USBInHandle = HIDTxPacket(HID_EP, (char *)&TXP, 64);
                        IO_STATUS=IO_IDLE;
                        BeepTicks=20;
                    }
                    break;
                case 3:
                    //IronPars.PID_KP=RXP.IronPars.PID_KP.i16;
                    //IronPars.PID_KI=RXP.IronPars.PID_KI.i16;
                    //IronPars.PID_DGain=RXP.IronPars.PID_DGain;
                    //IronPars.PID_OVFGain=RXP.IronPars.PID_OVFGain;
                    //IronPars.Gain=RXP.IronPars.Gain.i16;
                    IO_STATUS=IO_IDLE;
                    break;
                case 4:
                    BeepTicks=20;
                    if(!HIDTxHandleBusy(USBInHandle)){
                        TXP.Command=4;
                        //TXP.IronPars.PID_KP.i16=IronPars.PID_KP;
                        //TXP.IronPars.PID_KI.i16=IronPars.PID_KI;
                        //TXP.IronPars.PID_DGain=IronPars.PID_DGain;
                        //TXP.IronPars.PID_OVFGain=IronPars.PID_OVFGain;
                        //TXP.IronPars.Gain.i16=IronPars.Gain;
                        USBInHandle = HIDTxPacket(HID_EP, (char *)&TXP, 64);
                        IO_STATUS=IO_IDLE;
                    }
                    break;
                default:
                    RXP.Command=0;
                    IO_STATUS=IO_IDLE;
                    break;
            }
            USBOutHandle = HIDRxPacket(HID_EP, (char *)&RXP, 64);
        }
    }
}

#undef _IO_C
