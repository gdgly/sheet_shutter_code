/********************************************************************************
* FileName: SpeedController.h
* Description:  
* This header file contains the decleration of all the attributes and 
* services for SpeedController.c file. It implements speed mode of
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
#ifndef SPEED_CONTROLLER_H
#define SPEED_CONTROLLER_H
#include <p33Exxxx.h>
#include "./MotorControl/PIController/pi.h"
#include "./Common/Typedefs/Typedefs.h"

#define PHASE_DECREMENT_FLAG    1
#define PHASE_INCREMENT_FLAG    2

#define HALLA_BIT	(PORTAbits.RA8) /* HALLA port pin - RA8 RPI24 */
#define HALLB_BIT	(PORTCbits.RC6) /* HALLB port pin - RC6 RP54 */
#define HALLC_BIT	(PORTFbits.RF0) /* HALLC port pin - RF0 RPI96 */

EXTERN tPIParm speedPIparms;
/* This function initializes all the variables used by speed controller */
VOID intitSpeedController(VOID);

/* This function calculates phase value for given sector */
VOID calculatePhaseValue(WORD sectorNo);

SHORT getCurrentSectorNo(VOID);


#endif /* SPEED_CONTROLLER_H */
