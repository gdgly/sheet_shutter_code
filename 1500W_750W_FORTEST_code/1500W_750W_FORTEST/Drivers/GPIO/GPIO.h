/********************************************************************************
* FileName: GPIO.h
* Description:  
* This header file contains the decleration of all the attributes and 
* services for GPIO.c file. It initializes all the required port pins.
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
#ifndef GPIO_H
#define GPIO_H
#include "./Common/Typedefs/Typedefs.h"

/* This function initialize all I/O's */
VOID initGPIO(VOID);

    
#endif /* GPIO_H */
