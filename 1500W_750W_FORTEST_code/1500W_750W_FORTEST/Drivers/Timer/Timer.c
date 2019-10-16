/*********************************************************************************
* FileName: Timer.c
* Description:
* This source file contains the definition of all the functions for Timer.
* It initializes all the timers required in application
**********************************************************************************/

/****************************************************************************
 * Copyright 2014 Bunka Shutters.
 * This program is the property of the Bunka Shutters
 * Company, Inc.and it shall not be reproduced, distributed or used
 * without permission of an authorized company official.This is an
 * unpublished work subject to Trade Secret and Copyright
 * protection.
*****************************************************************************/

/****************************************************************************
 *  Modification History
 *  
 *  Date                  Name          Comments 
 *  09/04/2014            iGate          Initial Creation                                                               
*****************************************************************************/
#include <p33Exxxx.h>
#include <string.h>
#include "Timer.h"
#include "./Common/UserDefinition/Userdef.h"

/******************************************************************************
 * initTMR1
 *
 * This function initializes TIMER1 for speed control and motor stalled protection.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initTMR1(VOID)
{
	T1CON = 0x0030;			/* internal Tcy/256 clock */
	TMR1 = 0;
	PR1 = 273;				/* 1 ms timer */
    IPC0bits.T1IP = 3;
}
/******************************************************************************
 * initTMR2
 *
 * This function initializes TIMER2 for current control of motor.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initTMR2(VOID)
{
	T2CON = 0x0030;			/* internal Tcy/256 clock */
	TMR2 = 0;
	PR2 = 273;				/* 1 ms timer */
    IPC1bits.T2IP = 3;
}
/******************************************************************************
 * initTMR3
 *
 * This function initializes TIMER3 used as a timebase for the two input capture channels.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initTMR3(VOID)
{
#if (PDIV == PDIV_256)
    T3CON = 0x0030; /* internal Tcy/256 clock */
#elif (PDIV == PDIV_64)
    T3CON = 0x0020; /* internal Tcy/64 clock */
#else
    #error Timer3 not configured
#endif    
	TMR3 = 0;
	PR3 = 0xFFFF;
    
    IFS0bits.T3IF = 0;
    IPC2bits.T3IP = 1;
}

VOID initTMR3For1500W(VOID)
{
#if (PDIV == PDIV_256)
    T3CON = 0x0030; /* internal Tcy/256 clock */
#elif (PDIV == PDIV_64)
    T3CON = 0x0020; /* internal Tcy/64 clock */
#else
    #error Timer3 not configured
#endif    
	TMR3 = 0;
	//Changed for ADC2- RN- NOV2015
	PR3 = 2734*2;//0xFFFF;			/* 20ms timer */
    
    IFS0bits.T3IF = 0;
    IPC2bits.T3IP = 1;
}

/******************************************************************************
 * initTMR4
 *
 * This function initializes Timer4 used for Ramp Generator.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initTMR4(VOID)
{
	T4CON = 0x0030;	 /* internal Tcy/256 clock */
	TMR4 = 0;
	PR4 = 2734;		/* 10ms timer */
    IPC6bits.T4IP = 2;
}

/******************************************************************************
 * initTMR5
 *
 * This function initializes Timer5 used to generate trigger for ADC2 sampling
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
//	Added Dec 2015
VOID initTMR5(VOID)
{
	T5CON = 0x0030;	 /* internal Tcy/256 clock */
	TMR5 = 0;
	PR5 = 2734*2;;		/* 20ms timer */
	IFS1bits.T5IF = 0;  /* Clear timer 5 flag */
	IEC1bits.T5IE = 1;	/* Enable interrupts for Timer 5 */
    T5CONbits.TON = 0; 	//stop timer
    IPC7bits.T5IP = 1;
}

/******************************************************************************
 * initTMR6
 *
 * This function initializes Timer6 used to generate system tick
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initTMR6(VOID)
{
	T6CON = 0x0030;	 /* internal Tcy/256 clock */
	TMR6 = 0;
	PR6 = 1367;		/* 5ms timer */
	IFS2bits.T6IF = 0;  /* Clear timer 6 flag */
	IEC2bits.T6IE = 1;	/* Enable interrupts for Timer 6 */
    T6CONbits.TON = 0; //stop timer
    IPC11bits.T6IP = 1;
}

/******************************************************************************
 * initTMR7
 *
 * This function initializes Timer7 used to calculate trigger for current calculation
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initTMR7(VOID)
{
	T7CON = 0x0000;
	TMR7 = 0;
	PR7 = 0; //it is initialized in PWM initialization    
	T7CONbits.TON = 0;
	IFS3bits.T7IF = 0;  /* Clear timer 7 flag */
	IEC3bits.T7IE = 0;	/* Enable interrupts for Timer 7 */
    IPC12bits.T7IP = 6;
}

/******************************************************************************
 * initTMR8
 *
 * This function initializes Timer8 used for debounce module
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initTMR8(VOID)
{
	T8CON = 0x0030;	 /* internal Tcy/256 clock */
	TMR8 = 0;
	PR8 = 27340;				/* 100 ms timer */
	IFS3bits.T8IF = 0;  /* Clear timer 8 flag */
	IEC3bits.T8IE = 1;	/* Enable interrupts for Timer 8 */
    T8CONbits.TON = 0; //stop timer
    IPC12bits.T8IP = 1;
}

VOID initTMR9(VOID)
{
	T9CON = 0x0030;	 /* internal Tcy/256 clock */
	TMR9 = 0;
	PR9 = 27340;				/* 100 ms timer */
	IFS3bits.T9IF = 0;  /* Clear timer 8 flag */
	IEC3bits.T9IE = 1;	/* Enable interrupts for Timer 8 */
    T9CONbits.TON = 0; //stop timer
    IPC13bits.T9IP = 1;
}

VOID initTMR9ForTest(VOID)
{
	T9CON = 0x0030;	 /* internal Tcy/256 clock */
	TMR9 = 0;
	PR9 = 2734;				/* 100 ms timer */
	IFS3bits.T9IF = 0;  /* Clear timer 8 flag */
	IEC3bits.T9IE = 1;	/* Enable interrupts for Timer 8 */
    T9CONbits.TON = 0; //stop timer
    IPC13bits.T9IP = 1;
}


