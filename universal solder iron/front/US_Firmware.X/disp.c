#define _DISP_C
#include <xc.h>
#include <GenericTypeDefs.h>
#include "mcu.h"
#include "main.h"
#include "disp.h"
#include "OLED.h"
#undef _DISP_C

const UINT8 numbers[10] =
{
    252,
    160,
    62,
    186,
    226,
    218,
    222,
    176,
    254,
    250
};

//const UINT8 s_off[3]            = {252, 86, 86};    //"off"
//const UINT8 s_on[3]             = {252, 134, 0};    //"on"
//const UINT8 s_auto[3]           = {246, 236, 78};   //"Aut"
//const UINT8 s_Err[3]            = {94, 6, 6};       //"Err"
//const UINT8 s_standby[3]        = {2, 2, 2};        //"---"
//const UINT8 s_standby_hot[3]    = {230, 252, 78};   //"HOt"
//const UINT8 s_HErr[3]           = {230, 2, 86};     //"H-F"
//const UINT8 s_HSC[3]            = {230,218, 92};     //"S-C"
//const UINT8 s_SErr[3]           = {218, 2, 86};     //"S-F"
//const UINT8 s_NoIron[3]         = {134, 134, 134};  //"nnn"

volatile UINT8 DISPLAY[3] = {0, 0, 0};             //Display buffer

void ShowChar(UINT8 dch){
    UINT8_VAL lc;
    if(!mainFlags.OLED){
        IND1=1;
        IND2=1;
        IND3=1;
        if(dch!=0){
            lc.Val=DISPLAY[dch-1];
            LEDG=lc.bits.b1;
            LEDE=lc.bits.b2;
            LEDD=lc.bits.b3;
            LEDA=lc.bits.b4;
            LEDB=lc.bits.b5;
            LEDF=lc.bits.b6;
            LEDC=lc.bits.b7;
            IND1=!(dch==1);
            IND2=!(dch==2);
            IND3=!(dch==3);
        }
        else{
            LEDG=0;
            LEDE=0;
            LEDD=0;
            LEDA=0;
            LEDB=0;
            LEDF=0;
            LEDC=0;
        }
    }
}

void DisplayInt(int di){
    UINT8 i;
    UINT16 ldi;
    ldi = di;
    if(di < 0)ldi =- di;
    for(i = 0; i < 3; i++){
        ldi <<= 1;
        if((ldi & 0xF00) >= 0x500)ldi += 0x300;
    }
    ldi >>= 3;
    for(i = 0; i < 4; i++){
        if((ldi & 0xF0) >= 0x50)ldi += 0x30;
        if((ldi & 0xF00) >= 0x500)ldi += 0x300;
        ldi <<= 1;
    }
    ldi >>= 4;
    for(i = 0; i < 3; i++){
        DISPLAY[2 - i] = numbers[ldi & 0x0F];
        ldi >>= 4;
    }
    for(i = 0; i < 2; i++){
        if(DISPLAY[i] != numbers[0])break;
        DISPLAY[i] = 0;
    }
    if(di < 0)DISPLAY[0] = 2;
}

void DisplayData(const UINT8 * dd)
{
    DISPLAY[0] = dd[0];
    DISPLAY[1] = dd[1];
    DISPLAY[2] = dd[2];
}