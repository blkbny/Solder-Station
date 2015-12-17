/*
 * SolderStation-c-code1.0.c
 *
 * Created: 12/15/2015 2:21:16 PM
 * Author : Dperreault
 */ 

#include <avr/io.h>


int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
    }
}

void init_hardwr(void)
{
	//set both PA0 and PA1 as outputs for DC and CS to control LCD
	DDRA = (0<<DDRA0 | 0<<DDRA1) //init pins PA0-PA1 as outputs
	
	//set PC1, PB1, and PB2 as SPI data com
	//init SPI
	
	//set PA3 as ADC0 for reading analog temp from iron
	//init ADC0
	
	//set PA4 as ADC1 for reading analog from potent. to set temp
	//init ADC1
	
	//set PA6 as PWM for Iron temp control
	//init PWM 
	
}


