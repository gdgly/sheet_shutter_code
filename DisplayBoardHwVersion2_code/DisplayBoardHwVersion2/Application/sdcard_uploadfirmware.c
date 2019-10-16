/*********************************************************************************
 * FileName: sdcard_uploadfirmware.c
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
 *  	0.1D	DD/MM/YYYY      	iGATE Offshore team       Initial Creation
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

/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/

/****************************************************************************/

/******************************************************************************
 * FunctionName: uploadFirmwareRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t uploadFirmwareRunTime(void)
{
	//
	// This function is called periodically
	//

	updateFaultLEDStatus();

	return 0;
}
/******************************************************************************
 * FunctionName: uploadFirmwarePaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t uploadFirmwarePaint(void)
{
	//
	// This is first screen paint function
	//

	return 0;
}

/******************************************************************************
 * FunctionName: uploadFirmwareMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t uploadFirmwareMode(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Mode key press
	//
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
    	gKeysStatus.bits.Key_Mode_pressed = 0;

    	//
    	// Write mode key functionality here
    	//
    	//
    			// Reset Request and response status and acknowledgment status
    			//
    			gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
    			gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
    			gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
    			gstUMtoCMdatabase.commandToControlBoard.val = 0;

    	//    	psActiveMenu = psActiveMenu->psParent;
    	//		psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
    	//		psActiveFunctionalBlock->pfnPaintFirstScreen();
    	//		menuPaintFirstScreen();
    			psActiveFunctionalBlock = &gsMenuFunctionalBlock;
    			psActiveFunctionalBlock->pfnPaintFirstScreen();

    }

	return 0;
}

/******************************************************************************
 * FunctionName: uploadFirmwareEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t uploadFirmwareEnter(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Enter key press
	//
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;

		//
		// Write enter key functionality here
		//
	}

	return 0;
}

/******************************************************************************
 * FunctionName: uploadFirmwareUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t uploadFirmwareUp(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Up key press
	//
	if(gKeysStatus.bits.Key_Up_pressed)
	{
		gKeysStatus.bits.Key_Up_pressed = 0;

		//
		// Write up key functionality here
		//
	}

	return 0;
}

/******************************************************************************
 * FunctionName: uploadFirmwareDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t uploadFirmwareDown(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Down key press
	//
	if(gKeysStatus.bits.Key_Down_pressed)
	{
		gKeysStatus.bits.Key_Down_pressed = 0;

		//
		// Write Down key functionality here
		//
	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsUploadFirmwareFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	uploadFirmwarePaint,
	uploadFirmwareRunTime,
	uploadFirmwareUp,
	uploadFirmwareDown,
	uploadFirmwareMode,
	uploadFirmwareEnter
};
