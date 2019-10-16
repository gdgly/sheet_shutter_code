/*********************************************************************************
 * FileName: wireless.c
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
 *  	0.1D	04/12/2014      	iGATE Offshore team       Initial Creation
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

#include "Middleware/serial.h"

/****************************************************************************/

/****************************************************************************
 *  Macros
****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
static uint8_t gsEnterSwitchFlag = 0;
static uint8_t gsStateMachine = 0;
/****************************************************************************/

/******************************************************************************
 * FunctionName: wirelessRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t wirelessRunTime(void)
{
	// Logic to update blinking wirless module status received from control board
	// Commented the logic as it is not in sync with the actual blinking of wireless module status,
	// as the blink rate is 700ms and polling speed of display board is 500ms
#if 0
	static uint32_t lsTickCount500ms = 0;

	if((g_ui32TickCount - lsTickCount500ms) > 50)
	{
		// capture time
		lsTickCount500ms = g_ui32TickCount;

		if(1 == gsEnterSwitchFlag)
		{
			// ON
			displayText("E", 100, 32, false, true, false, false);
		}
		else
		{
			// OFF
			displayText("E", 100, 32, true, true, false, false);
		}

		if(1 == gstControlBoardStatus.bits.wirelessMonitor)
		{
			// ON
			displayText("W", 100, 16, false, true, false, false);
		}
		else
		{
			// OFF
			displayText("W", 100, 16, true, true, false, false);
		}
	}
#endif

	// Logic to update blinking wirless module status independent of actual wireless module status
	// The main reason for this activity is to not put any load on bandwitdth to match wireless module status

	static uint32_t lsTickCount500ms = 0;

	#define BLINK_ON_TIME	50
	#define BLINK_OFF_TIME	20

	static uint32_t lsTickCountTraceWirelesMonitor;
	static uint32_t lsTickCountHandleBlinking;

	if (0 == gsStateMachine)
	{

		if(1 == gstControlBoardStatus.bits.wirelessMonitor)
		{
			// ON
			displayText("W", 100, 16, false, true, false, false, false, true);

			lsTickCountTraceWirelesMonitor = g_ui32TickCount; // g_ui32TickCount increment by 1 after every 10ms

			lsTickCountHandleBlinking = g_ui32TickCount;

			gsStateMachine = 1;
		}

	}
	else if (1 == gsStateMachine)
	{

		if (get_timego( lsTickCountHandleBlinking) > BLINK_ON_TIME )
		{

			// OFF
			displayText("W", 100, 16, true, true, false, false, false, true);

			lsTickCountHandleBlinking = g_ui32TickCount; // g_ui32TickCount increment by 1 after every 10ms

			gsStateMachine = 2;

		}

	}
	else if (2 == gsStateMachine)
	{

		if (get_timego( lsTickCountHandleBlinking) > BLINK_OFF_TIME )
		{

			// ON
			displayText("W", 100, 16, false, true, false, false, false, true);

			lsTickCountHandleBlinking = g_ui32TickCount; // g_ui32TickCount increment by 1 after every 10ms

			gsStateMachine = 1;

		}

	}

	if (0 != gsStateMachine)
	{

		if(1 == gstControlBoardStatus.bits.wirelessMonitor)
		{

			lsTickCountTraceWirelesMonitor = g_ui32TickCount; // g_ui32TickCount increment by 1 after every 10ms

		}

		if (get_timego( lsTickCountTraceWirelesMonitor) > 100 )
		{

			// ON
			displayText("W", 100, 16, true, true, false, false, false, true);

			gsStateMachine = 0;

		}

	}

	if(get_timego( lsTickCount500ms) > 50)
	{
		// capture time
		lsTickCount500ms = g_ui32TickCount;

		if(1 == gsEnterSwitchFlag)
		{
			// ON
			displayText("E", 100, 32, false, true, false, false, false, true);
		}
		else
		{
			// OFF
			displayText("E", 100, 32, true, true, false, false, false, true);
		}

	}

	return 0;
}

/******************************************************************************
 * FunctionName: wirelessPaintScreen
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t wirelessPaintScreen(void)
{
	// Clear screen
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, true, true);

	// Display message
	displayText("WIRELESS", 36, 0, false, false, false, false, false, true);
	displayText("WIRELESS MODULE", 2, 16, false, false, false, false, false, true);
	displayText("W", 100, 16, true, true, false, false, false, true);
	displayText("ENTER SWITCH", 2, 32, false, false, false, false, false, true);
	displayText("E", 100, 32, true, true, false, false, false, true);

	// Reset the state machine variable used to blink the wireless monitor symbol
	gsStateMachine = 0;

	return 0;
}

/******************************************************************************
 * FunctionName: wirelessUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t wirelessUp(void)
{
	if(gKeysStatus.bits.Key_Up_pressed)
	{
		gKeysStatus.bits.Key_Up_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: wirelessDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t wirelessDown(void)
{
	if(gKeysStatus.bits.Key_Down_pressed)
	{
		gKeysStatus.bits.Key_Down_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: wirelessMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t wirelessMode(void)
{
	if(gKeysStatus.bits.Key_Mode_pressed)
	{
		gKeysStatus.bits.Key_Mode_pressed = 0;

		// Reset module flags
		gsEnterSwitchFlag = 0;

		// Go back to menus
		psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
		psActiveFunctionalBlock->pfnPaintFirstScreen();
	}

	return 0;
}

/******************************************************************************
 * FunctionName: wirelessEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t wirelessEnter(void)
{
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;

		if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		{
			gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangePressed = 1;
			gstUMtoCMoperational.commandRequestStatus = eACTIVE;
		}
	}

	if(gKeysStatus.bits.Key_Enter_released)
	{
		gKeysStatus.bits.Key_Enter_released = 0;

		if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		{
			gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangeReleased = 1;
			gstUMtoCMoperational.commandRequestStatus = eACTIVE;
		}
	}

	//
	// Handle command response
	//
	if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
	{
		if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
		{
			if(gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangePressed == 1)
			{
				gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangePressed = 0;

				// Set enter switch flag
				gsEnterSwitchFlag = 1;

				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
			}
			else if(gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangeReleased == 1)
			{
				gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangeReleased = 0;

				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;

				// Reset enter switch flag
				gsEnterSwitchFlag = 0;
			}
		}

		else if( (gstUMtoCMoperational.commandResponseStatus == eTIME_OUT) ||
				 (gstUMtoCMoperational.commandResponseStatus == eFAIL)
				)
		{
			if(gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangePressed == 1)
			{
				gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangePressed = 0;

				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
			}
			else if(gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangeReleased == 1)
			{
				gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangeReleased = 0;

				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
			}
		}
	}

	return 0;
}

/******************************************************************************
 * Define Shutter Run functional block object
*********************************************************************************/
stInternalFunctions gsWirelessFunctionalBlock =
{
	&gsMenuFunctionalBlock,
	0,
	wirelessPaintScreen,
	wirelessRunTime,
	wirelessUp,
	wirelessDown,
	wirelessMode,
	wirelessEnter
};

