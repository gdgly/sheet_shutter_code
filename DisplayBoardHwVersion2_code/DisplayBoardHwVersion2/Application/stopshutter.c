/*********************************************************************************
 * FileName: stopshutter.c
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
 *  	0.1D	09/06/2014      	iGATE Offshore team       Initial Creation
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
#include "Middleware/paramdatabase.h"
/****************************************************************************/

/****************************************************************************
 *  Macros
****************************************************************************/

//**************************************************************************
// Shutter Run States
//**************************************************************************
#define CHECK_ALREADY_STOPPED	0
#define SEND_STOP_COMMAND		1
#define WAIT_FOR_STOP_COMMAND	2
#define SWITCH_TO_HOME_SCREEN	3
/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
uint8_t gStopState = CHECK_ALREADY_STOPPED;

uint8_t shutter_stop_cyw=0;
uint8_t Disp_shutter_stop_cyw=0;
/****************************************************************************/

/******************************************************************************
 * FunctionName: shutterStopHandler
 *
 * Function Description:
 * This function handles stop functional block runtime activities. This function
 * performs below activities.
 * 1. It checks whether shutter system is stopped or not.
 * 2. If not then, initiates command to stop shutter system.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterStopHandler(void)
{
	static uint8_t lsDelay3SecStart = 0;
	static uint32_t lsTickCount3Seconds = 0;

	switch(gStopState)
	{
		case CHECK_ALREADY_STOPPED:
		{
			//
			// Check run and stop flag
			//
			if(gstControlBoardStatus.bits.runStop == 0)
			{
				if(lsDelay3SecStart == 0)
				{
					//
					// Clear screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, true, true);

					//
					// Display message
					//
					if(gu8_language == Japanese_IDX)
					{

					displayText("シャッターシステム", 2, 0, false, false, false, false, false, false);
					displayText("テイシチュウ", 2, 16, false, false, false, false, false, false);
					}
					else
					{
						displayText("SHUTTER SYSTEM", 2, 0, false, false, false, false,false,true);
						displayText("ALREADY STOPPED", 2, 16, false, false, false, false,false,true);
					}
					//
					// Capture time
					//
					lsTickCount3Seconds = g_ui32TickCount;

					//
					// Set delay start flag
					//
					lsDelay3SecStart = 1;
				}
			}
			else
			{
				//
				// Change state
				//
				gStopState = SEND_STOP_COMMAND;
			}

			//
			// Check whether delay is achieved
			//
			if( (get_timego( lsTickCount3Seconds) > 300) &&
				(lsDelay3SecStart == 1)
			  )
			{
				//
				// Set active functional block as home screen functional block.
				//
				psActiveFunctionalBlock = &gsHomeScreenFunctionalBlock;

				//
				// Call first screen paint function
				//
				psActiveFunctionalBlock->pfnPaintFirstScreen();

				//
				// Reset delay start flag
				//
				lsDelay3SecStart = 0;
			}

			break;
		}

		case SEND_STOP_COMMAND:
		{
			//
			// Initiate Run control board command
			//
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				//
				// Set command bit
				//
				gstUMtoCMoperational.commandToControlBoard.bits.stopControlBoard = 1;

				//
				// Set command request status as active
				//
				gstUMtoCMoperational.commandRequestStatus = eACTIVE;

				//
				// Change state
				//
				gStopState = WAIT_FOR_STOP_COMMAND;
			}

			break;
		}

		case WAIT_FOR_STOP_COMMAND:
		{
			//
			// Check Run control board command response status
			//
			if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
			{
				//
				// Check for response success
				//
				if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
				{
					//
					// Reset request and response status
					//
					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;

					//
					// Clear screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, true, true);

					//
					// Display message
					//
					//
					if(gu8_language == Japanese_IDX)
					{
					displayText("シャッターシステム", 2, 0, false, false, false, false, false, false);//change over
					displayText("STOPモード", 2, 16, false, false, false, false, false, false);
					}
					else
					{
						displayText("SHUTTER SYSTEM", 2, 0, false, false, false, false,false,true);
						displayText("STOPPED", 2, 16, false, false, false, false, false, true);
					}
					gstUMtoCMoperational.commandToControlBoard.val = 0;

					//
					// Change state
					//
					gStopState = SWITCH_TO_HOME_SCREEN;

				}
				else if( (gstUMtoCMoperational.commandResponseStatus == eTIME_OUT) ||
						 (gstUMtoCMoperational.commandResponseStatus == eFAIL)
					   )
				{
#if 0
					//
					// Set communication fault flag
					//
					if(gstUMtoCMoperational.commandResponseStatus == eTIME_OUT)
					{
						gstDisplayBoardFault.bits.displayCommunication = 1;
						gstDisplayCommunicationFault.bits.commFailControl = 1;
					}

					if(gstUMtoCMoperational.commandResponseStatus == eFAIL)
					{
						gstDisplayBoardFault.bits.displayCommunication = 1;
						gstDisplayCommunicationFault.bits.crcError = 1;
					}
#endif
					//
					// Clear screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, true, true);

					//
					// Paint communication failure message
					//

					if(gu8_language == Japanese_IDX)
					{
					//
					displayText("シャッターシステム", 2, 0, false, false, false, false, false, false);
					//displayText("STOP PROCESS", 2, 16, false, false, false, false, false, false);
					//displayText("FAILED", 2, 32, false, false, false, false, false, false);
					displayText("STOP NG", 2, 16, false, false, false, false, false, false);
					}
					else
					{
						displayText("SHUTTER SYSTEM", 2, 0, false, false, false, false,false,true);
						displayText("STOP NG", 2, 16, false, false, false, false, false, true);
					}
					gstUMtoCMoperational.commandToControlBoard.val = 0;

					//
					// Reset request and response status
					//
					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;

					//
					// Change state
					//
					gStopState = SWITCH_TO_HOME_SCREEN;

				}
			}

			break;
		}

		case SWITCH_TO_HOME_SCREEN:
		{
			//
			// Start 3 seconds delay
			//
			if(lsDelay3SecStart == 0)
			{
				//
				// Capture time
				//
				lsTickCount3Seconds = g_ui32TickCount;

				//
				// Set delay start flag
				//
				lsDelay3SecStart = 1;
			}

			//
			// Check whether 3 seconds delay is achieved
			//
			if( (get_timego( lsTickCount3Seconds) > 300) &&
				(lsDelay3SecStart == 1)
			  )
			{
				//
				// Set active functional block as home screen functional block.
				//
				psActiveFunctionalBlock = &gsHomeScreenFunctionalBlock;

				//
				// Call first screen paint function
				//
				psActiveFunctionalBlock->pfnPaintFirstScreen();

				//
				// Reset delay start flag
				//
				lsDelay3SecStart = 0;

				//
				// Change state
				//
				gStopState = CHECK_ALREADY_STOPPED;

			}

			break;
		}
	}

	updateFaultLEDStatus();

	return 0;
}

/******************************************************************************
 * FunctionName: shutterStopPaintScreen
 *
 * Function Description:
 * Default function paint first screen activity
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterStopPaintScreen(void)
{
	//
	// Do nothing.
	//
	GrRectFIllBolymin(0, 127, 0, 63, true, true);
	displayText("リミットセッテイヲシマスカ?", 2, 0, false, false, false, false,false,false);
		displayText("YES", 2, 16, false, false, false, false,false,false);
		displayText("NO", 2, 32, true, false, false, true,false,false);
		shutter_stop_cyw = 0;
		Disp_shutter_stop_cyw = 0;
	return 0;
}

/******************************************************************************
 * FunctionName: shutterStopUp
 *
 * Function Description:
 * This is a default function for up key press on Stop screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterStopUp(void)
{
	if(gKeysStatus.bits.Key_Up_pressed)
	{
		gKeysStatus.bits.Key_Up_pressed = 0;
		GrRectFIllBolymin(0, 127, 0, 63, true, true);
					displayText("リミットセッテイヲシマスカ?", 2, 0, false, false, false, false,false,false);
				if(Disp_shutter_stop_cyw == 0)
				{
					Disp_shutter_stop_cyw = 1;
					displayText("YES", 2, 16, true, false, false, true,false,false);
					displayText("NO", 2, 32, false, false, false, false,false,false);
				}
				else
				{
					Disp_shutter_stop_cyw = 0;
					displayText("YES", 2, 16, false, false, false, false,false,false);
					displayText("NO", 2, 32, true, false, false, true,false,false);
				}
	}

	return 0;
}

/******************************************************************************
 * FunctionName: shutterStopDown
 *
 * Function Description:
 * This is a default function for down key press on Stop screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterStopDown(void)
{
	if(gKeysStatus.bits.Key_Down_pressed)
	{
		gKeysStatus.bits.Key_Down_pressed = 0;
		GrRectFIllBolymin(0, 127, 0, 63, true, true);
							displayText("リミットセッテイヲシマスカ?", 2, 0, false, false, false, false,false,false);
						if(Disp_shutter_stop_cyw == 0)
						{
							Disp_shutter_stop_cyw = 1;
							displayText("YES", 2, 16, true, false, false, true,false,false);
							displayText("NO", 2, 32, false, false, false, false,false,false);
						}
						else
						{
							Disp_shutter_stop_cyw = 0;
							displayText("YES", 2, 16, false, false, false, false,false,false);
							displayText("NO", 2, 32, true, false, false, true,false,false);
						}
	}

	return 0;
}

/******************************************************************************
 * FunctionName: shutterStopMode
 *
 * Function Description:
 * This is a default function for mode key press on Stop screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterStopMode(void)
{
	if(gKeysStatus.bits.Key_Mode_pressed)
	{
		gKeysStatus.bits.Key_Mode_pressed = 0;
		psActiveFunctionalBlock = &gsMenuFunctionalBlock;
		psActiveFunctionalBlock->pfnPaintFirstScreen();
	}

	return 0;
}

/******************************************************************************
 * FunctionName: shutterStopEnter
 *
 * Function Description:
 * This is a default function for enter key press on Stop screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterStopEnter(void)
{
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;
		shutter_stop_cyw  =Disp_shutter_stop_cyw;
		if(shutter_stop_cyw == 0)
				{
					psActiveFunctionalBlock = &gsMenuFunctionalBlock;
					psActiveFunctionalBlock->pfnPaintFirstScreen();
				}
	}

	return 0;
}


uint8_t shutterStopTime(void)
{
	if(shutter_stop_cyw == 1)
	{
	shutterStopHandler();
	}
}

/******************************************************************************
 * Define Shutter Run functional block object
*********************************************************************************/
stInternalFunctions gsShutterStopFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	shutterStopPaintScreen,
	shutterStopTime,
	shutterStopUp,
	shutterStopDown,
	shutterStopMode,
	shutterStopEnter
};

