/********************************************************************************
* FileName: ADC.h
* Description:  
* This header file contains the decleration of all the attributes and 
* services for ADC.c file. It initializes all the ADC channels required 
* by application.
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
#ifndef ADC_H
#define ADC_H
#include "./Common/Typedefs/Typedefs.h"

/* Initialization of all ADC channels */
VOID initADC(VOID);
//Added for ADC2- RN- NOV2015
VOID initADC2(VOID);
VOID initADC2For1500W(VOID);

#endif /* ADC_H */
