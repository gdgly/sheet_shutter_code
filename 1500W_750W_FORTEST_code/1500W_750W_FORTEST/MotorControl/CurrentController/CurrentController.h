/********************************************************************************
* FileName: CurrentController.h
* Description:  
* This header file contains the decleration of all the attributes and 
* services for CurrentController.c file. It implements current mode of
* motor operation
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
#ifndef CURRENT_CONTROLLER_H
#define CURRENT_CONTROLLER_H
#include "./Common/Typedefs/Typedefs.h"

typedef struct CurrControllerFlags{
    unsigned offsetCalulated : 1;	/* Flag needs to be set once offset calculated */
    unsigned unused : 15;  
} currCntrlFlg;

EXTERN currCntrlFlg currControlFlag;

/* This function gives interface of current controller to other modules */
VOID intitCurrentController(VOID);

/* This function calculates ADC offset error */
VOID measureADCOffset(VOID);

/* This function calculates total current */
VOID calculateTotalCurrent(VOID);

VOID filterTotalCurrent(VOID);

//	Added for implementation of power fail functionality on DC Bus for version 4 board- RN- NOV 2015
VOID executePowerFailRoutine(VOID);
    
#endif /* CURRENT_CONTROLLER_H */
