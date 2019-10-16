/********************************************************************************
* FileName: MCPWM.h
* Description:  
* This header file contains the decleration of all the attributes and 
* services for MCPWM.c file. It initializes all the MCPWM channels required 
* to run the motor.
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
#ifndef MCPWM_H
#define MCPWM_H
#include "./Common/Typedefs/Typedefs.h"

/* This function initializes PWM at 20kHz, Center aligned, Complementary mode with 2 us of deadtime */
VOID initMCPWM(VOID);

#endif /* MCPWM_H */
