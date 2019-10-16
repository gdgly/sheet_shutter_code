/********************************************************************************
* FileName: Timer.h
* Description:  
* This header file contains the decleration of all the attributes and 
* services for Timer.c file. It initializes all the timers required in application
*********************************************************************************/

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
#ifndef TIMER_H
#define TIMER_H
#include "./Common/Typedefs/Typedefs.h"

/* TIMER1 used for speed control and motor stalled protection */
VOID initTMR1(VOID);

/* TIMER2 used for current control of motor */
VOID initTMR2(VOID);

/* TIMER3 used as a timebase for the two input capture channels */
VOID initTMR3(VOID);

VOID initTMR3For1500W(VOID);

/* TIMER4 used by Ramp Generator */
VOID initTMR4(VOID);

/* TIMER5 used by ADC2 for DC bus voltage monitoring */
//	Added Dec 2015
VOID initTMR5(VOID);

/* This function initializes Timer6 used to generate system tick */
VOID initTMR6(VOID); 

/* This function initializes Timer7 used to generate system tick */
VOID initTMR7(VOID); 

/* This function initializes Timer8 used to generate system tick */
VOID initTMR8(VOID); 

VOID initTMR9(VOID);

VOID initTMR9ForTest(VOID);

#endif /* TIMER_H */
