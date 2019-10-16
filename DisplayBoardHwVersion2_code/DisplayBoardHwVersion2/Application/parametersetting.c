/*********************************************************************************
 * FileName: parametersetting.c
 * Description:
 * Version: 0.1D
 *
 **********************************************************************************/

/****************************************************************************
 * Copyright 2014 Bunka Shutters.
 * This program is the property of the Bunka Shutters
 * and it shall not be reproduced, distributed or used
 * without permission of an authorized company official.
 * This is an unpublished work subject to Trade Secret
 * and Copyright protection.
 *****************************************************************************/


/****************************************************************************
 *  Modification History
 *
 *  Revision		Date                  Name          			Comments
 *  	0.1D	11/06/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/gpio.h>
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "grlib/grlib.h"
//#include "Middleware/cfal96x64x16.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "userinterface.h"
#include "intertaskcommunication.h"
#include "parameterlist.h"
/****************************************************************************/

/****************************************************************************
 *  Macros
****************************************************************************/

//***************************************************************************
// Parameter setting focus indices
//****************************************************************************/
#define PARAM_ONES			0
#define PARAM_TENS			1
#define PARAM_HUNDREDS		2
#define PARAM_THOUSANDS		3
/****************************************************************************/

/****************************************************************************
 *  Enumerations
****************************************************************************/

/****************************************************************************/

/****************************************************************************
 *  Structures
****************************************************************************/

/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
unsigned char paramA005Values[3][20] =
{
	"ENABLE IN AUTO",
	"ENABLE IN MANUAL",
	"ENABLE AUTO MANUAL"
};
/****************************************************************************/
