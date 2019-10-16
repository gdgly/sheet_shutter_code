/*********************************************************************************
* FileName: MCPWM.c
* Description:
* This source file contains the definition of all the functions for MCPWM.
* It initializes all the MCPWM channels required to run the motor.
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
#include "MCPWM.h"
#include "./Common/UserDefinition/Userdef.h"
#include "../Common/Extern/Extern.h"

/******************************************************************************
 * initMCPWM
 *
 * This function initializes MCPWM channels to run the motor.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initMCPWM(VOID)
{
	PHASE1 = (FCY/FPWM - 1);
	PHASE2 = (FCY/FPWM - 1);
	PHASE3 = (FCY/FPWM - 1);
    PTPER = 2*(FCY/FPWM - 1)+1;
    
    IOCON1 = 0xF000;
	IOCON2 = 0xF000;
	IOCON3 = 0xF000;
	
	/* 2 us of dead time */
	DTR1 = 0x0000;	
	DTR2 = 0x0000;	
	DTR3 = 0x0000;
	
    if(PreMotorType == MOTOR_750W)
    {
        ALTDTR1 = 2*ALTDTR_DIV2_750W;       //1us		
        ALTDTR2 = 2*ALTDTR_DIV2_750W;
        ALTDTR3 = 2*ALTDTR_DIV2_750W;
    }
    else if(PreMotorType == MOTOR_1500W)
    {
        ALTDTR1 = 2*ALTDTR_DIV2_1500W;      //5us	
        ALTDTR2 = 2*ALTDTR_DIV2_1500W;
        ALTDTR3 = 2*ALTDTR_DIV2_1500W;
    }
	
	
	PTCON2 = 0x0000;	/* Divide by 1 to generate PWM */  
    #ifdef SVM_INTERRUPT_USE_T7
    {
        PWMCON1 = 0x0205;//0x0604;	/* Enable PWM output pins and configure them as */
        PWMCON2 = 0x0205;//;	/* complementary mode */
        PWMCON3 = 0x0205;//;
    }
    #else 
    {
        PWMCON1 = 0x0605;	/* Enable PWM output pins and configure them as */
        PWMCON2 = 0x0205;//;	/* complementary mode */
        PWMCON3 = 0x0205;//;
    }
    #endif
	
	PDC1 = PHASE1 / 2;	/* Initialize as 0 voltage */
	PDC2 = PHASE2 / 2;	
	PDC3 = PHASE3 / 2;
    PR7 = 14000; //timer 7 to calculate trigger point
    TRIG1 = PHASE1;
    
	/**************************************************/
	//(Disabled on 27 Jan 2015 to disable fault input)
   	//PWMCON1bits.FLTIEN = 1; /* Enable fault interrupt */    
    /*FCLCON1 = 0x002C;
	FCLCON2 = 0x002C;
	FCLCON3 = 0x002C;*/
	/**************************************************/
	
	// For testing only (Added on 27 Jan 2015 to disable fault input)
	// These bits are enabled in main.c just before main while loop
	/**************************************************/
	FCLCON1 = 0x0003;
	FCLCON2 = 0x0003;
	FCLCON3 = 0x0003;
	/**************************************************/
    
    PMD6bits.PWM4MD = 1; //Disable PWM4 
    PMD6bits.PWM5MD = 1; //Disable PWM5 
    PMD6bits.PWM6MD = 1; //Disable PWM6 
	
    IPC23bits.PWM1IP = 6;
	PTCON = 0x0000;		// stop PWM
}
