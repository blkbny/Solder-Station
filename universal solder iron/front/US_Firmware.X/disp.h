/* 
 * File:   disp.h
 * Author: Sparky
 *
 * Created on ?????????, 2013, ??? 16, 18:56
 */

#ifndef DISP_H
#define	DISP_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef _DISP_C
//extern const UINT8 s_off[3];
//extern const UINT8 s_on[3];
//extern const UINT8 s_auto[3];
//extern const UINT8 s_Err[3];
//extern const UINT8 s_standby[3];
//extern const UINT8 s_standby_hot[3];
//extern const UINT8 s_HErr[3];
//extern const UINT8 s_HSC[3];
//extern const UINT8 s_SErr[3];
//extern const UINT8 s_NoIron[3];

//extern const UINT8 parText[9][3];

extern volatile UINT8 DISPLAY[3];
extern void ShowChar(UINT8 dch);
extern void DisplayInt(int di);
extern void DisplayData(const UINT8 * dd);
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* DISP_H */

