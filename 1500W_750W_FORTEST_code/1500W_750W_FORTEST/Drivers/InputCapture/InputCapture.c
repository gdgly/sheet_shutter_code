/*********************************************************************************
* FileName: InputCapture.c
* Description:
* This source file contains the definition of all the functions for input capture.
* It initializes IC1, IC2 and IC3 to interface hall sensor.
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
#include "InputCapture.h"

//hall counts 0x177, 0x1A1, 0x16B, 0x165
//hall counts 375, 417, 363, 357 
//average hall counts is 365 for one hall sensor, for 3 hall sensor 1095

//on actual setup 708, 678, 693


/******************************************************************************
 * initInputCapture
 *
 * This function initializes IC1, IC2 and IC3 to interface hall sensor.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initInputCapture(VOID)
{
	/* Hall A -> IC1. Hall A is only used for commutation. */
	/* Hall B -> IC2. Hall B is used for Speed measurement and commutation. */
	/* Hall C -> IC3. Hall C is only used for commutation. */	
	/* Input capture every edge with interrupts and TMR3 */
	IC1CON1 = 0x0001;
	IC1CON2 = 0x0000;
    IPC0bits.IC1IP = 6;
	IFS0bits.IC1IF = 0;
	IC2CON1 = 0x0001;
    IC2CON2 = 0x0000;
    IPC1bits.IC2IP = 6;
	IFS0bits.IC2IF = 0;
	IC3CON1 = 0x0001;
	IC3CON2 = 0x0000;
    IPC9bits.IC3IP = 6;
	IFS2bits.IC3IF = 0;    
}
