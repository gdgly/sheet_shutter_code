/********************************************************************************
* FileName: DCInjection.h
* Description:  
* This header file contains the decleration of all the attributes and 
* services for CurrentController.c file. It implements DC Injection to Hold motor 
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
 *  09/05/2014            iGate          Initial Creation                                                               
*****************************************************************************/
#ifndef DC_INJECTION_H
#define DC_INJECTION_H

#include "./Common/Typedefs/Typedefs.h"

/* This function controls motor hodling at one position */
VOID DCInjectionON(VOID);

/* This function disables the motor hold and runs motor */
VOID DCInjectionOFF(VOID);

#endif /* DC_INJECTION_H */
