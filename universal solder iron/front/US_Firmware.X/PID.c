#define _PID_C

#include <GenericTypeDefs.h>
#include "PID.h"
#include "isr.h"
#include "iron.h"
#include "ExtFloat.h"

//volatile int LastOn;
//volatile unsigned char OffCnt;
//volatile unsigned char NoHeaterCnt;

void PIDInit(){
    int i;
    for(i = 2; i--;){
        PIDVars[i].Starting = 1;
        PIDVars[i].PIDDuty = 0;
        PIDVars[i].PIDDutyFull = 0;
        PIDVars[i].PWM = 0;
        PIDVars[i].OffDelay = 1600;
    }
};

#define intshr(a,b) ((a < 0) ? (-((-a) >> b)) : (a >> b))

#define assertin(a,b,c) \
    if(b<c){\
        if(a<b)a=b;\
        if(a>c)a=c;\
    }\
    else{\
        if(a<c)a=c;\
        if(a>b)a=b;\
    }

void PID(int PIDStep) {
    int i, w, AVG;
    INT32 dw, pdt;
    t_PIDVars * PV;
    t_IronConfig * IC;


    AVG = ADCAVG;
    PV =(t_PIDVars *)&PIDVars[PIDStep];
    IC =(t_IronConfig *)&IronPars.Config[PIDStep];
    if(IronPars.Config[1].Type){
        AVG--;
    }
    else{
        PV =(t_PIDVars *)&PIDVars[0];
        IC =(t_IronConfig *)&IronPars.Config[0];
    }
    if(PV->LastTTemp!=CTTemp)PV->DestinationReached = 0;
    PV->LastTTemp = CTTemp;

/**** GET ROOM TEMPERATURE **********************************************************/
    if(PIDStep){
        dw = ADCData.VRT >> 2;
        dw *= 147;
        dw >>= 8;
        dw -= 100;
        RTAvg -= intshr(RTAvg, ADCAVG);
        RTAvg += dw;
        CRTemp = intshr(RTAvg, ADCAVG);
    }
/************************************************************************************/

/**** GET HEATER RESISTANCE *********************************************************/
    if(PV->NoHeater) PV->NoHeaterCnt = (1 << (AVG + 1));
    if(PV->NoHeaterCnt) PV->NoHeaterCnt--;
    if(PV->NoHeater){
        PV->HInitData = 1;
        PV->HVAvg = 0;
        PV->HIAvg = 0;
        PV->HRAvg = 0x7FFF;
        PV->HPAvg = 0;
        PV->OffDelay = 1600;
    }
    else{
        if(PV->HInitData) PV->HRAvg = PV->HR << AVG;
        if(PV->HNewData){
            if(PV->HInitData){
                PV->HVAvg = PV->HV << AVG;
                PV->HIAvg = PV->HI << AVG;
                PV->HRAvg = PV->HR << AVG;
                PV->HPAvg = PV->HP << AVG;
                PV->HInitData = 0;
            }
            PV->HVAvg -= PV->HVAvg >> AVG;
            PV->HVAvg += PV->HV;
            PV->HIAvg -= PV->HIAvg >> AVG;
            PV->HIAvg += PV->HI;
            PV->HRAvg -= PV->HRAvg >> AVG;
            PV->HRAvg += PV->HR;
            PV->HPAvg -= PV->HPAvg >> AVG;
            PV->HPAvg += PV->HP;
            PV->HNewData=0;
        }
    }
/************************************************************************************/
    
    PV->ADCTemp[1]=PV->ADCTemp[0];
    PV->ADCTemp[0] = ADCData.VTEMP[1];

/**** WAVE SHAPING *****************************************************/
    PV->WSCorr = 0;
    if(IC->WSLen){
        if(PV->Starting || PV->NoHeaterCnt || PV->NoSensor){
            i=8;
            while(i--){
                PV->WSDelta[i].cnt=0;
                PV->WSDelta[i].val=0;
            }
        }
        else{
            if(PV->DestinationReached == 0){
                i=8;
                while(i--)PV->WSDelta[i].cnt = 0;
            }
            if(ADCData.HeaterOn){
                if(PV->OffCnt > 1){
                    i=((PV->OffCnt > IC->WSLen) ? IC->WSLen : PV->OffCnt);
                    w=PV->WSDelta[0].val;
                    if(i > PV->WSDelta[0].cnt){
                        PV->WSDelta[0].cnt = i;
                        PV->WSDelta[0].val = (PV->ADCTemp[1] - PV->ADCTemp[0]) << 1;
                    }
                    else{
                        if(i == PV->WSDelta[0].cnt){
                            PV->WSDelta[0].val = intshr(PV->WSDelta[0].val, 1);
                            PV->WSDelta[0].val+=(PV->ADCTemp[1] - PV->ADCTemp[0]);
                        }
                    }
                    w -= PV->WSDelta[0].val;
                    for(i = 1; i < IC->WSLen; i++){
                        PV->WSDelta[i].val -= w;
                    }
                }
                if(PV->LastOn<1023){
                    dw = PV->ADCTemp[0] - PV->LastOn;
                    dw <<= 3;
                    dw /= (PV->OffCnt ? PV->OffCnt : 1);
                    PV->Slope[1] = PV->Slope[0];
                    PV->Slope[0] = intshr(PV->Slope[0], 1);
                    PV->Slope[0] += dw;
                }
                PV->LastOn = PV->ADCTemp[0];
                PV->OffCnt = 0;
            }
            else{
                if(PV->OffCnt > 0){
                    if(PV->OffCnt < IC->WSLen){
                        dw = PV->Slope[0];
                        dw *= PV->OffCnt;
                        dw = intshr(dw, 4);
                        if(PV->WSDelta[PV->OffCnt].cnt==0){
                            PV->WSDelta[PV->OffCnt].val = intshr(PV->WSDelta[0].val, 1) - (PV->ADCTemp[0] - PV->LastOn);
                            PV->WSDelta[PV->OffCnt].cnt = 1;
                        }
                        else{
                            PV->WSDelta[PV->OffCnt].val = intshr(PV->WSDelta[PV->OffCnt].val, 1);
                        }
                        PV->WSDelta[PV->OffCnt].val += intshr(PV->WSDelta[0].val, 1) - (PV->ADCTemp[0] - PV->LastOn);
                    }
                }
            }
            for(i=1;i<IC->WSLen;i++){
                assertin(PV->WSDelta[i].val, 0, PV->WSDelta[i-1].val);
            }
            if(PV->OffCnt < IC->WSLen){
                PV->WSCorr = intshr(PV->WSDelta[PV->OffCnt].val, 1);
            }
            else{
                PV->WSCorr += (intshr(PV->WSDelta[IC->WSLen-1].val, 1) * (IC->WSLen-1)) / PV->OffCnt;
            }
            if(PV->OffCnt < 255)PV->OffCnt++;
            //if(ADCData.VTEMP[3] < 0) ADCData.VTEMP[3] = 0;
        }
        //ADCData.VTEMP[2] = ADCData.VTEMP[3];
    }
/************************************************************************************/

/**** GET AND NORMALISE IRON TEMPRATURE *********************************************/
    w = PV->ADCTemp[0] + PV->WSCorr;
    if(w < 0) w = 0;

    //heater resistance compensation for series TC
    //compensation = (HRCompCurrent * 19.6 * Gain * HRAvg)/65536
    //compensation = (((HRCompCurrent * Gain * 20070) / 32767) * HRAvg) / 2048;
    w -= ((((INT32)IC->Gain * (INT32)IC->HRCompCurrent * 20070L) >> 15) * (INT32)(PV->HRAvg >> AVG)) >> 11;


    //Deprecated, will be removed soon
    //dw = w * ((int)IC->SoftGain);
    //dw = intshr(dw, 10) + ((int)IC->SoftOffset);
    //if(IC->Type == 1) dw += CRTemp; //room temperature addition for TC
    //if(dw < 0) dw = 0;
    //PV->CTemp[1] = PV->CTemp[0];
    //PV->CTemp[0] = dw;
/************************************************************************************/
    
    {
        dw = w + IC->Offset;
        //dw *= ((int)IC->SoftGain);
        //dw = intshr(dw, 10) + ((int)IC->SoftOffset);
        if(dw < 0)dw = 0;
        if(dw > 2047)dw = 2047;

/******* INPUT MILLIVOLTS CALCULATION ***********************************************/
        //ADC = Vin * 750 * (IC->Gain / 256) * (1024 / 3000mV)
        //mV = (256 * 3000 * ADC) / (750 * 1024 * IC->Gain)) = ADC / IC->Gain
        ExtFloat x1;
        if(x1.m = ((UINT32)dw) << 20){
            x1.e = 138;
            while (x1.m < 0x80000000){
                x1.m <<=1;
                x1.e--;
            }
        }
        ExtFloatDivByUInt(x1, IC->Gain);

/******* Resistance calculation if resistive sensor *********************************/
        //R=mV/Current=mV / ((1.225V * IC->Current) / (1600 * 256))
        if(IC->Type==2){
            const ExtFloat rcc = {
                0xA72F0539,   //1.6*256/1.225 32 bit mantissa
                127+8        //1.6*256/1.225 exponent
            };
            ExtFloatMul(x1, rcc);
            UINT32 Current;
            if(IC->ChInv){
                Current = IC->CurrentA;         //divide by current A if channel A selected as positive input
                if(!IC->CBandA) x1.e -=4;       //divide by 16 if higher current band on channel A
            }
            else{
                Current = IC->CurrentB;         //divide by current A if channel A selected as positive input
                if(!IC->CBandB) x1.e =-4;       //divide by 16 if higher current band on channel A
            }
            if(Current == 0) Current = 1;
            ExtFloatDivByUInt(x1, Current);
        }
        SFLOAT CX={
            {
                x1.m >> 8,
                x1.e,
                0
            }
        };
        PV->CPolyX = CX.f;

/******* TEMPERATURE POLYNOMIAL CALCULATION *****************************************************/
        //T = C0 + C1 * X + c2 * X^2 + C3 * X^3 + ... + C9 * X^9

        LATBbits.LATB7 = 0;

        int n; //current polynomial power
        ExtFloat xn = x1;
        ExtFloat PSum;
        ExtFloat NSum;

        //Load positive or negatise sum with the first polynomial coefficient depending on it's sign
        float2ExtFloat(PSum, IC->TPoly[0]);
        if(((SFLOAT)IC->TPoly[0]).s){
            NSum = PSum;
            PSum.m = 0;
            PSum.e = 0;
        }

        for(n = 1; n < 10; n++){
            ExtFloat CSum, cn;

            float2ExtFloat(cn, IC->TPoly[n]);


            //get positive or negative sum depending on current coefficient sign
            if(((SFLOAT)IC->TPoly[n]).s){
                CSum = NSum;
            }
            else{
                CSum = PSum;
            }

            ExtFloatMul(cn, xn);

            ExtFloatAdd(CSum, cn);

            //store in positive or negative sum depending on current coefficient
            if(((SFLOAT)IC->TPoly[n]).s){
                NSum = CSum;
            }
            else{
                PSum = CSum;
            }

            //don't calculate next argument power if the end is reached
            if(n >= 9) break;

            //calculate next polynomial argument power
            ExtFloatMul(xn, x1);
        }

        //calculate (PSum - NSum) * 2 in order to get integer temperature * 2
        dw=0;
        if(PSum.e >= NSum.e){
            NSum.m >>= min(PSum.e - NSum.e, 32);
            if(PSum.m > NSum.m){
                if(PSum.e >= 125 + 32){
                    dw=1023;
                }
                else{
                    PSum.m -= NSum.m;
                    dw = ((UINT64)PSum.m << (PSum.e - 125)) >> 32;
                    if(dw > 1023)dw = 1023;
                }
            }
        }

        //Add room temperature if thermocouple
        if(IC->Type == 1) dw += CRTemp;
        if(dw > 1023)dw = 1023;

        PV->CTemp[0] = dw;
        PV->PrbFCTemp1 = dw;

        LATBbits.LATB7 = 1;





/*        LATBbits.LATB7 = 0;
        SFLOAT cf, cff;
        cf.m = x1 >> 8;
        cf.e = x1Exp;
        cf.s = 0;
        cff.f = cf.f;
        float fs1=IC->TPoly[0];
        for(i=1; i<10; i++){
            fs1 += cff.f * IC->TPoly[i];
            if(i < 9){
                cff.f *= cf.f;
            }
        }
        fs1 *= 2;
        //fs1 += CRTemp;
        PV->PrbFCTemp1=fs1;
        LATBbits.LATB7 = 1;*/
    }

    if(PV->Starting || PV->NoHeaterCnt){
        PV->Starting = 0;
        i = (1 << ADCAVG);
        while(i--){
            PV->TBuff[i] = PV->CTemp[0];
            PV->SlopeBuff[i] = PV->CTemp[0];
        }
        PV->TAvgP[0] = PV->CTemp[0];
        PV->TAvgP[1] = PV->CTemp[0];
        PV->TAvg=PV->CTemp[0];
        i = AVG;
        while(i--)PV->TAvg += PV->TAvg;
        PV->TAvgF[0] = PV->TAvg;
        PV->TAvgF[1] = PV->TAvg;
        PV->TSlope = 0;
        PV->TBPos = 0;
        PV->SBPos = 0;
    }

    PV->TAvg -= PV->TBuff[PV->TBPos];
    PV->TAvg += PV->CTemp[0];

    PV->TBuff[PV->TBPos] = PV->CTemp[0];
    PV->TBPos++;
    PV->TBPos &= (1 << AVG) - 1;

    PV->TAvgF[1] = PV->TAvgF[0];
    PV->TAvgF[0] -= PV->TAvgF[0] >> AVG;
    PV->TAvgF[0] += PV->CTemp[0];

    PV->SlopeBuff[PV->SBPos] = PV->TAvg;
    PV->SBPos++;
    PV->SBPos &= (1 << AVG) - 1;

    PV->TSlope -= intshr(PV->TSlope, 2);
    PV->TSlope += PV->SlopeBuff[(PV->SBPos - 1) & ((1 << AVG) - 1)];
    PV->TSlope -= PV->SlopeBuff[PV->SBPos];

    dw = ((INT32)IC->PID_DGain) * intshr(PV->TSlope, 2);
    dw = intshr(dw, AVG + 2);

    PV->TAvgP[1] = PV->TAvgP[0];
    PV->TAvgP[0] = (PV->TAvgF[0] >> AVG);
    PV->TAvgP[0] += dw;
    if(PV->TAvgP[0]<0) PV->TAvgP[0] = 0;

    if(PV->DestinationReached==0){
        w=CTTemp << 2;
        if( ((PV->TAvgP[0] >= w) && (PV->TAvgP[1] < w)) || ((PV->CTemp[0] >= w) && (PV->CTemp[1] < w)) ){
            PV->KeepOff = (IC->WSLen+1); //(1 << AVG) + 1;
            PV->DestinationReached = 1;
        }
    }

    PV->Delta[1] = PV->Delta[0];
    PV->Delta[0] = CTTemp << 2;
    PV->Delta[0] -= PV->TAvgP[0];
    PV->Delta[0] <<= 3;

    pdt = PV->PIDDutyFull; 
    dw = PV->Delta[0] * ((int)(IC->PID_KP + IC->PID_KI));
    pdt += dw;
    dw = PV->Delta[1] * ((int)IC->PID_KP);
    pdt-=dw;

    if(pdt < 0){
        pdt = 0;
    }
    else{
        w = -intshr(PV->Delta[0], 2);
        if(w <= 0){
            if(w < (-200)) pdt = 0xFFFFFF;
        }
        else{
            dw = w * ((int)IC->PID_OVSGain);
            if(dw > 255) dw = 255;
            dw <<= 16;
            dw = 0x00FFFFFF - dw;
            if(pdt > dw) pdt = dw;
        }
        if((IC->Type == 0) || (IC->Type == 255) || PV->NoSensor) pdt = 0;
        if(pdt>0xFFFFFF)pdt=0xFFFFFF;
    }
    PV->PIDDutyFull=pdt; //normalized duty

/**** RECALCULATE DUTY FOR REAL VS RATED POWER*********************************/
    //Duty=Duty * rated power / peak power
    pdt += 0x7FL;
    pdt >>= 8;
    if(PV->HP){
        dw = PV->HPAvg >> AVG;
        pdt *= (INT32)IC->PID_PMax;
        pdt /= dw;
    }
/******************************************************************************/

/**** RECALCULATE DUTY FOR MAX 6.0A RMS CURRENT******************************/
    dw=0xFA00L;
    if(PV->HI){
        dw = PV->HIAvg >> AVG;
        if(dw > 192){
            dw = (INT32)(65535 * 192) / dw;
        }
        else{
            dw=0xFA00;
        }
    }
    if(pdt > dw) pdt = dw;
/******************************************************************************/

    pdt <<= 8;
    if(PV->NoHeater) pdt=0x28F5C; //PWM = once per 2 seconds in order to detect heater resistance on open heater
    if(PV->ShortCircuit) pdt=0x10624; //PWM = once per 5 seconds in order to detect heater resistance on short circuit

    PV->PIDDuty = pdt;

}

#undef _PID_C
