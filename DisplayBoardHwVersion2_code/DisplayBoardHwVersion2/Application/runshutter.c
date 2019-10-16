/*********************************************************************************
 * FileName: runshutter.c
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
#define CHECK_ALREADY_RUNNING	0
#define SEND_RUN_COMMAND		1
#define WAIT_FOR_RUN_COMMAND	2
#define SWITCH_TO_HOME_SCREEN	3
/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
uint8_t gRunState = CHECK_ALREADY_RUNNING;
uint8_t shutter_run_cyw=0;
uint8_t Disp_shutter_run_cyw=0;
/****************************************************************************/

/******************************************************************************
 * FunctionName: shutterRunHandler
 *
 * Function Description:
 * This function handles Run functional block runtime activities. This function
 * performs below activities.
 * 1. It checks whether shutter system is running or not.
 * 2. If not then, initiates command to start shutter system.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterRunHandler(void)
{
	static uint8_t lsDelay3SecStart = 0;
	static uint32_t lsTickCount3Seconds = 0;

	switch(gRunState)
	{
		case CHECK_ALREADY_RUNNING:
		{
			//
			// Check run and stop flag
			//
			if(gstControlBoardStatus.bits.runStop == 1)
			{
				if(lsDelay3SecStart == 0)
				{
					//
					// Clear screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
					//
					// Display message
					//
					//SHUTTER SYSTEM ALREADY RUNNING

					if(gu8_language == Japanese_IDX)
					{
					displayText("シャッターシステム", 2, 0, false, false, false, false, false, false);
					displayText("ドウサチュウ", 2, 16, false, false, false, false, false, false);
					}
					else
					{
					displayText("SHUTTER SYSTEM", 2, 0, false, false, false, false,false,true);
					displayText("ALREADY RUNNING", 2, 16, false, false, false, false,false,true);
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
				gRunState = SEND_RUN_COMMAND;
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

		case SEND_RUN_COMMAND:
		{
			//
			// Initiate Run control board command
			//
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				//
				// Set command bit
				//
				gstUMtoCMoperational.commandToControlBoard.bits.runControlBoard = 1;

				//
				// Set command request status as active
				//
				gstUMtoCMoperational.commandRequestStatus = eACTIVE;

				//
				// Change state
				//
				gRunState = WAIT_FOR_RUN_COMMAND;


			//	lsTickCount3Seconds = g_ui32TickCount;
			}

			break;
		}

		case WAIT_FOR_RUN_COMMAND:
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


					if(gu8_language == Japanese_IDX)
					{
					displayText("シャッターシステム", 2, 0, false, false, false, false, false, false);//change over
					//displayText("ドウサ", 2, 16, false, false, false, false, false, false);
					displayText("RUNモード", 2, 16, false, false, false, false, false, false);
					}
					else
					{
				    displayText("SHUTTER SYSTEM", 2, 0, false, false, false, false,false,true);
					displayText("RUNNING", 2, 16, false, false, false, false,false,true);
					}
					gstUMtoCMoperational.commandToControlBoard.val = 0;

					//
					// Change state
					//
					gRunState = SWITCH_TO_HOME_SCREEN;

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
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
					//
					// Paint communication failure message
					//
					//displayText("SHUTTER SYSTEM", 2, 0, false, false, false, false);
					//displayText("START PROCESS", 2, 16, false, false, false, false);
					//displayText("FAILED", 2, 32, false, false, false, false);

					if(gu8_language == Japanese_IDX)
					{
					displayText("シャッターシステム", 2, 0, false, false, false, false, false, false);
					//displayText("START PROCESS", 2, 16, false, false, false, false);
					displayText("スタート_プロセス", 2, 16,false, false, false, false, false, false);
					displayText("エラー", 2, 32, false, false, false, false, false, false);
					}
					else
					{
					displayText("SHUTTER SYSTEM", 2, 0, false, false, false, false,false,true);
					displayText("START PROCESS", 2, 16, false, false, false, false,false,true);
					displayText("FAILED", 2, 32, false, false, false, false,false,true);
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
					gRunState = SWITCH_TO_HOME_SCREEN;

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
				gRunState = CHECK_ALREADY_RUNNING;

			}

			break;
		}
	}

	updateFaultLEDStatus();

	return 0;
}

/******************************************************************************
 * FunctionName: shutterRunPaintScreen
 *
 * Function Description:
 * Default function paint first screen activity.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterRunPaintScreen(void)
{
	//
	// Do nothing.
	//
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
	if(gu8_language == Japanese_IDX)
	displayText("リミットセッテイヲシマスカ?", 2, 0, false, false, false, false,false,false);
	else
    displayText("DO YOU WANT TO INSTALL?", 2, 0, false, false, false, false,false,true);
	displayText("YES", 2, 16, false, false, false, false,false,false);
	displayText("NO", 2, 32, true, false, false, true,false,false);
	shutter_run_cyw = 0;
	Disp_shutter_run_cyw =0;
	return 0;
}

/******************************************************************************
 * FunctionName: shutterRunUp
 *
 * Function Description:
 * This is a default function for up key press on Run screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterRunUp(void)
{
	if(gKeysStatus.bits.Key_Up_pressed)
	{
		gKeysStatus.bits.Key_Up_pressed = 0;
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
		if(gu8_language == Japanese_IDX)
			displayText("リミットセッテイヲシマスカ?", 2, 0, false, false, false, false,false,false);
			else
		    displayText("DO YOU WANT TO INSTALL?", 2, 0, false, false, false, false,false,true);
		if(Disp_shutter_run_cyw == 0)
		{
			Disp_shutter_run_cyw = 1;
			displayText("YES", 2, 16, true, false, false, true,false,false);
			displayText("NO", 2, 32, false, false, false, false,false,false);
		}
		else
		{
			Disp_shutter_run_cyw = 0;
			displayText("YES", 2, 16, false, false, false, false,false,false);
			displayText("NO", 2, 32, true, false, false, true,false,false);
		}
	}

	return 0;
}

/******************************************************************************
 * FunctionName: shutterRunDown
 *
 * Function Description:
 * This is a default function for down key press on Run screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterRunDown(void)
{
	if(gKeysStatus.bits.Key_Down_pressed)
	{
		gKeysStatus.bits.Key_Down_pressed = 0;
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
		if(gu8_language == Japanese_IDX)
			displayText("リミットセッテイヲシマスカ?", 2, 0, false, false, false, false,false,false);
			else
		    displayText("DO YOU WANT TO INSTALL?", 2, 0, false, false, false, false,false,true);
		if(Disp_shutter_run_cyw == 0)
				{
					Disp_shutter_run_cyw = 1;
					displayText("YES", 2, 16, true, false, false, true,false,false);
					displayText("NO", 2, 32, false, false, false, false,false,false);
				}
				else
				{
					Disp_shutter_run_cyw = 0;
					displayText("YES", 2, 16, false, false, false, false,false,false);
					displayText("NO", 2, 32, true, false, false, true,false,false);
				}
	}

	return 0;
}

/******************************************************************************
 * FunctionName: shutterRunMode
 *
 * Function Description:
 * This is a default function for mode key press on Run screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterRunMode(void)
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
 * FunctionName: shutterRunEnter
 *
 * Function Description:
 * This is a default function for enter key press on Run screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t shutterRunEnter(void)
{
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;
		shutter_run_cyw  = Disp_shutter_run_cyw;
		if(shutter_run_cyw == 0)
		{
			psActiveFunctionalBlock = &gsMenuFunctionalBlock;
			psActiveFunctionalBlock->pfnPaintFirstScreen();
		}
	}

	return 0;
}

uint8_t shutterRunTime(void)
{
	if(shutter_run_cyw == 1)
	{
	shutterRunHandler();
	}
}
/******************************************************************************
 * Define Shutter Run functional block object
*********************************************************************************/
stInternalFunctions gsShutterRunFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	shutterRunPaintScreen,
	shutterRunTime,
	shutterRunUp,
	shutterRunDown,
	shutterRunMode,
	shutterRunEnter
};

