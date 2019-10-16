/*********************************************************************************
 * FileName: changesettinghistoryerase.c
 * Description: Code for displaying 'Erase Change Settings Logs' screen
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
 *  	0.2D	03/07/2014								Auto go back and other timers added.
 *  	0.1D	10/06/2014      	iGATE Offshore team       Initial Creation
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
#include "Middleware/paramdatabase.h"
#include "sdcardlogs.h"
#include "logger.h"
/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
// Define Flags for Internal modules
_ChgSetHistFlags chgSetHistFlags;
static uint32_t eraseresult;
extern uint8_t startTimer;
extern uint32_t tickCount;

uint8_t sdcardsetting_erase_cyw=0;
uint8_t Disp_sdcardsetting_erase_cyw=0;
/****************************************************************************/

/******************************************************************************
 * FunctionName: changeSettingHistoryRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t changeSettingHistoryEraseRunTime(void)
{
	//
	// This function is called periodically
	//
	static uint32_t lastTickCount = 0;

	if(chgSetHistFlags.flags.cshistErase)
		if(startTimer == 'Y')
			// Each time the timer tick occurs, process any button events.
			if(g_ui32TickCount != lastTickCount)
			{
				// Remember last tick count
				lastTickCount = g_ui32TickCount;
				if(tickCount++ == _WAITSYSTICK(3))
				{
					startTimer = 'Z'; //Proceed for timeout screen
					tickCount = 0;
					lastTickCount = 0;

					GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);

					if(!eraseresult) {

						if(gu8_language == Japanese_IDX)
						{
						displayText("リレキノリセットヲ", 2, 0, false, false, false, false,false,false);
						displayText("シマス", 2, 16, false, false, false, false, false, false);
						}
						else
						{
						displayText("LOGS ERASED FROM", 2, 0, false, false, false, false,false,true);
						displayText("MEMORY", 2, 16, false, false, false, false, false, true);
						}
					}
					else {

						if(gu8_language == Japanese_IDX)
						{
						displayText("リレキノリセットNG", 2, 0, false, false, false, false,false,false);
						}
						else
						{
					     displayText("FAILED TO ERASE", 2, 0, false, false, false, false,false,true);
						 displayText("LOGS", 2, 16, false, false, false, false, false,true);
						}


					}

				}
				else;
			}
			else;
		else if(startTimer == 'Z')
			if(g_ui32TickCount != lastTickCount)
			{
				// Remember last tick count
				lastTickCount = g_ui32TickCount;
				if(tickCount++ == _WAITSYSTICK(3))
				{
					startTimer = 'N'; //Proceed for timeout screen
					lastTickCount = 0;
					psActiveFunctionalBlock = &gsMenuFunctionalBlock;
					psActiveFunctionalBlock->pfnPaintFirstScreen();
					chgSetHistFlags.flags.cshistErase = 0;
				}
			}
			else;

	updateFaultLEDStatus();

	return 0;
}
/******************************************************************************
 * FunctionName: changeSettingHistoryPaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t changeSettingHistoryErasePaint(void)
{
	//
	// This is first screen paint function
	//
	//
	// Clear Screen.
	//
	chgSetHistFlags.val = 0;
	startTimer = 'N';
	GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);


	if(gu8_language == Japanese_IDX)
	{
	displayText("パラメータセットリレキヲ", 2, 0, false, false, false, false,false,false);
	displayText("リセットシマスカ?", 2, 16, false, false, false, false, false, false);
	}
	else
	{
	displayText("DO YOU WANT TO ERASE", 2, 0, false, false, false, false,false,true);
	displayText("CHANGE SETTING LOGS?", 2, 16, false, false, false, false, false, true);
	//displayText("", 2, 32, false, false, false, false, false, true);
	}

	displayText("YES", 2, 32, false, false, false, false,false,false);
	displayText("NO", 2, 48, true, false, false, true,false,false);
	sdcardsetting_erase_cyw=0;
	Disp_sdcardsetting_erase_cyw=0;
	return 0;
}

/******************************************************************************
 * FunctionName: changeSettingHistoryMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t changeSettingHistoryEraseMode(void)
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

		psActiveFunctionalBlock = &gsMenuFunctionalBlock;
		psActiveFunctionalBlock->pfnPaintFirstScreen();

    }

	return 0;
}

/******************************************************************************
 * FunctionName: changeSettingHistoryEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t changeSettingHistoryEraseEnter(void)
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
		sdcardsetting_erase_cyw  = Disp_sdcardsetting_erase_cyw;

		if(sdcardsetting_erase_cyw==1)
		{
		// Clear Screen.
		GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);

		if(gu8_language == Japanese_IDX)
		{
		displayText("リレキノリセットヲ", 2, 0, false, false, false, false,false,false);
		displayText("シマス", 2, 16, false, false, false, false, false, false);
		displayText("オマチクダサイ", 2, 32, false, false, false, false, false, false);
		}
		else
		{
		displayText("ERASING LOGS FROM", 2, 0, false, false, false, false,false,true);
		displayText("MEMORY", 2, 16, false, false, false, false, false, true);
		displayText("PLEASE WAIT...", 2, 32, false, false, false, false, false, true);
		}

		RESET_CHGSET_PARAM = 1;
		if(writeParameterUpdateInDB(RESETCHGSET_PARAM_IDX, &RESET_CHGSET_PARAM) == _SUCCESS)
		{
			eraseresult = monitorResetLogParam();
			startTimer = 'Y'; //start 3sec timer for screen to stay for 3sec
		}
		else
		{

			if(gu8_language == Japanese_IDX)
			{
			displayText("EEPロムエラー", 2, 48, false, false, false, false, false, false);
			}
			else
			{
			displayText("EEPROM FAILURE", 2, 48, false, false, false, false, false, true);
			}
			startTimer = 'Z';
		}

		chgSetHistFlags.flags.cshistErase = 1; //May be remove later. Please check for references before removing
		tickCount = 0;
		}
		else
		{
			psActiveFunctionalBlock = &gsMenuFunctionalBlock;
			psActiveFunctionalBlock->pfnPaintFirstScreen();
		}
	}

	return 0;
}

/******************************************************************************
 * FunctionName: changeSettingHistoryUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t changeSettingHistoryEraseUp(void)
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
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
		if(gu8_language == Japanese_IDX)
		{
		displayText("パラメータセットリレキヲ", 2, 0, false, false, false, false,false,false);
	    displayText("リセットシマスカ?", 2, 16, false, false, false, false, false, false);
		}
		else
		{
		displayText("DO YOU WANT TO ERASE", 2, 0, false, false, false, false,false,true);
		displayText("CHANGE SETTING LOGS?", 2, 16, false, false, false, false, false, true);
		}
				if(Disp_sdcardsetting_erase_cyw == 0)
				{
								Disp_sdcardsetting_erase_cyw = 1;
								displayText("YES", 2, 32, true, false, false, true,false,false);
								displayText("NO", 2, 48, false, false, false, false,false,false);
				}
				else
				{
							    Disp_sdcardsetting_erase_cyw = 0;
								displayText("YES", 2, 32, false, false, false, false,false,false);
								displayText("NO", 2, 48, true, false, false, true,false,false);
				}

	}

	return 0;
}

/******************************************************************************
 * FunctionName: changeSettingHistoryDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t changeSettingHistoryEraseDown(void)
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
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
		if(gu8_language == Japanese_IDX)
				{
				displayText("パラメータセットリレキヲ", 2, 0, false, false, false, false,false,false);
			    displayText("リセットシマスカ?", 2, 16, false, false, false, false, false, false);
				}
				else
				{
				displayText("DO YOU WANT TO ERASE", 2, 0, false, false, false, false,false,true);
				displayText("CHANGE SETTING LOGS?", 2, 16, false, false, false, false, false, true);
				}
						if(Disp_sdcardsetting_erase_cyw == 0)
						{
										Disp_sdcardsetting_erase_cyw = 1;
										displayText("YES", 2, 32, true, false, false, true,false,false);
										displayText("NO", 2, 48, false, false, false, false,false,false);
						}
						else
						{
									    Disp_sdcardsetting_erase_cyw = 0;
										displayText("YES", 2, 32, false, false, false, false,false,false);
										displayText("NO", 2, 48, true, false, false, true,false,false);
						}
	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsChangeSettingHistoryEraseFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	changeSettingHistoryErasePaint,
	changeSettingHistoryEraseRunTime,
	changeSettingHistoryEraseUp,
	changeSettingHistoryEraseDown,
	changeSettingHistoryEraseMode,
	changeSettingHistoryEraseEnter
};
