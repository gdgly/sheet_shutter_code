/*********************************************************************************
 * FileName: operationcount.c
 * Description: Code for showing Operation Count
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
 *  	0.1D	26/06/2014      	iGATE Offshore team       Initial Creation
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
//#include "intertaskcommunication.h"
#include "Middleware/paramdatabase.h"
#include "Middleware/sdcard.h"
#include "sdcardlogs.h"
#include "logger.h"
/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
extern uint32_t ui32OperationCount;

/******************************************************************************
 * FunctionName: operationCountRunTime
 *
 * Function Description:
 * This is a default function.
 *
 * Function Parameters: None
 *
 * Function Returns: 0 on success.
 *
 ********************************************************************************/
uint8_t operationCountRunTime(void)
{
	updateFaultLEDStatus();

	return 0;
}
/******************************************************************************
 * FunctionName: operationCountPaint
 *
 * Function Description: Paints current operation count value
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0 on success
 *
 ********************************************************************************/
uint8_t operationCountPaint(void)
{
	uint8_t viewbuff[20],Tp_datalong=0;


	// Clear Screen.
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

	if(gu8_language == Japanese_IDX)
	{
	//
	displayText("ドウサカウント", 2, 0, false, false, false, false, false, false);
	}
	else
	{
	displayText("OPERATION COUNT", 2, 0, false, false, false, false,false,true);
	}

	usnprintf((char *)viewbuff, sizeof(viewbuff), "%u", ui32OperationCount);
	//usnprintf((char*)viewbuff, sizeof(viewbuff), "%07d", ui32OperationCount);
		//memset(lbuff_cyw,0x20,sizeof(lbuff_cyw));
	//	Tp_datalong = data_long_cyw(ui32OperationCount);
	//	usnprintf_nU_cyw((char*)lbuff_cyw,Tp_datalong,(char*)viewbuff);
	//displayText(viewbuff, 2, 16, false, false, false, false);
			displayText(viewbuff, 2, 16, false, false, false, false, false, false);

	return 0;
}

/******************************************************************************
 * FunctionName: operationCountMode
 *
 * Function Description: Move back to status menus
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0 on success
 *
 ********************************************************************************/
uint8_t operationCountMode(void)
{
	//
	// Handle Mode key press
	//
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
    	gKeysStatus.bits.Key_Mode_pressed = 0;

		psActiveFunctionalBlock = &gsMenuFunctionalBlock;
		psActiveFunctionalBlock->pfnPaintFirstScreen();
    }

	return 0;
}

/******************************************************************************
 * FunctionName: operationCountEnter
 *
 * Function Description:
 * This is a default function.
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t operationCountEnter(void)
{
	//
	// Handle Enter key press
	//
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: operationCountUp
 *
 * Function Description:
 * This is a default function.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0 on success.
 *
 ********************************************************************************/
uint8_t operationCountUp(void)
{
	//
	// Handle Up key press
	//
	if(gKeysStatus.bits.Key_Up_pressed)
	{
		gKeysStatus.bits.Key_Up_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: operationCountDown
 *
 * Function Description:
 * This is a default function.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0 on success.
 *
 ********************************************************************************/
uint8_t operationCountDown(void)
{
	//
	// Handle Down key press
	//
	if(gKeysStatus.bits.Key_Down_pressed)
	{
		gKeysStatus.bits.Key_Down_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsOperationCountFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	operationCountPaint,
	operationCountRunTime,
	operationCountUp,
	operationCountDown,
	operationCountMode,
	operationCountEnter
};
