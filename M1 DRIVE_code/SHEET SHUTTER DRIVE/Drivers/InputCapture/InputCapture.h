/********************************************************************************
* FileName: InputCapture.h
* Description:  
* This header file contains the decleration of all the attributes and 
* services for InputCapture.c file. It initializes IC1, IC2 and IC3 to
* interface hall sensor
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
#ifndef INPUT_CAPTURE_H
#define INPUT_CAPTURE_H
#include "./Common/Typedefs/Typedefs.h"

/* Initialize input capture to read hall sensor */
VOID initInputCapture(VOID);

#endif /* INPUT_CAPTURE_H */
