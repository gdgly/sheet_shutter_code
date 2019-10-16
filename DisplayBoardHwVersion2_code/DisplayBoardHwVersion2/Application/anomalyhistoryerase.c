/*********************************************************************************
 * FileName: anomalyhistoryerase.c
 * Description: Code for displaying 'Erase Anomaly Logs' screen
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
extern uint8_t startTimer;
extern uint32_t tickCount;

static uint32_t eraseresult;


uint8_t sdcardanomaly_erase_cyw=0;
uint8_t Disp_sdcardanomaly_erase_cyw=0;
//extern _AnomHistFlags AnomHistFlags;
/****************************************************************************/

/******************************************************************************
 * FunctionName: anomalyHistoryRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t anomalyHistoryEraseRunTime(void)
{
	//
	// This function is called periodically
	//
	static uint32_t lastTickCount = 0;
	if(AnomHistFlags.flags.anomhistErase)
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
						displayText("MEMORY", 2, 16, false, false, false, false,false,true);
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
					AnomHistFlags.flags.anomhistErase = 0;
				}
			}
			else;

	updateFaultLEDStatus();

	return 0;
}
/******************************************************************************
 * FunctionName: anomalyHistoryPaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t anomalyHistoryErasePaint(void)
{
	//
	// This is first screen paint function
	//

	//
	// Clear Screen.
	//
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
	AnomHistFlags.val = 0;
	startTimer = 'N';

	if(gu8_language == Japanese_IDX)
	{
	displayText("エラーリレキヲ", 2, 0, false, false, false, false, false,false);
	displayText("リセットシマスカ?", 2, 16, false, false, false, false, false, false);
	}
	else
	{
	displayText("DO YOU WANT TO ERASE", 2, 0, false, false, false, false, false,true);
	displayText("ANOMALY HISTORY?", 2, 16, false, false, false, false, false,true);//ANOMALY HISTORY
	}
	displayText("YES", 2, 32, false, false, false, false,false,false);
	displayText("NO", 2, 48, true, false, false, true,false,false);

	 sdcardanomaly_erase_cyw=0;
	 Disp_sdcardanomaly_erase_cyw=0;
	return 0;
}

/******************************************************************************
 * FunctionName: anomalyHistoryMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t anomalyHistoryEraseMode(void)
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
 * FunctionName: anomalyHistoryEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t anomalyHistoryEraseEnter(void)
{
	//
	// Handle Enter key press
	//
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;
		sdcardanomaly_erase_cyw  = Disp_sdcardanomaly_erase_cyw;
	   if(sdcardanomaly_erase_cyw == 1)
	   {
		//
		// Clear Screen.
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		if(gu8_language == Japanese_IDX)
		{
		displayText("リレキノリセットヲ", 2, 0, false, false, false, false, false,false);
		displayText("シマス", 2, 16, false, false, false, false, false, false);
		displayText("オマチクダサイ...", 2, 32, false, false, false, false, false, false);
		}
		else
		{
	    displayText("ERASING LOGS FROM", 2, 0, false, false, false, false, false,true);
		displayText("MEMORY", 2, 16, false, false, false, false,  false,true);
		displayText("PLEASE WAIT...", 2, 32, false, false, false, false, false,true);
		}
		RESET_ANOM_PARAM = 1;

		if(writeParameterUpdateInDB(RESETANOM_PARAM_IDX, &RESET_ANOM_PARAM) == _SUCCESS)
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
			displayText("EEPROM FAILURE", 2, 48, false, false, false, false, false,true);
			}
			startTimer = 'Z';
		}

		AnomHistFlags.flags.anomhistErase = 1;
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
 * FunctionName: anomalyHistoryUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t anomalyHistoryEraseUp(void)
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
				displayText("エラーリレキヲ", 2, 0, false, false, false, false, false,false);
				displayText("リセットシマスカ?", 2, 16, false, false, false, false, false, false);
				}
				else
				{
				displayText("DO YOU WANT TO ERASE", 2, 0, false, false, false, false, false,true);
				displayText("ANOMALY HISTORY?", 2, 16, false, false, false, false, false,true);//ANOMALY HISTORY
				}
						if(Disp_sdcardanomaly_erase_cyw == 0)
						{
							Disp_sdcardanomaly_erase_cyw = 1;
										displayText("YES", 2, 32, true, false, false, true,false,false);
										displayText("NO", 2, 48, false, false, false, false,false,false);
						}
						else
						{
							Disp_sdcardanomaly_erase_cyw = 0;
										displayText("YES", 2, 32, false, false, false, false,false,false);
										displayText("NO", 2, 48, true, false, false, true,false,false);
						}

	}

	return 0;
}

/******************************************************************************
 * FunctionName: anomalyHistoryDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t anomalyHistoryEraseDown(void)
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
			displayText("エラーリレキヲ", 2, 0, false, false, false, false, false,false);
			displayText("リセットシマスカ?", 2, 16, false, false, false, false, false, false);
			}
			else
			{
			displayText("DO YOU WANT TO ERASE", 2, 0, false, false, false, false, false,true);
			displayText("ANOMALY HISTORY?", 2, 16, false, false, false, false, false,true);//ANOMALY HISTORY
			}
								if(Disp_sdcardanomaly_erase_cyw == 0)
								{
									Disp_sdcardanomaly_erase_cyw = 1;
												displayText("YES", 2, 32, true, false, false, true,false,false);
												displayText("NO", 2, 48, false, false, false, false,false,false);
								}
								else
								{
									Disp_sdcardanomaly_erase_cyw = 0;
												displayText("YES", 2, 32, false, false, false, false,false,false);
												displayText("NO", 2, 48, true, false, false, true,false,false);
								}
	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsAnomalyHistoryEraseFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	anomalyHistoryErasePaint,
	anomalyHistoryEraseRunTime,
	anomalyHistoryEraseUp,
	anomalyHistoryEraseDown,
	anomalyHistoryEraseMode,
	anomalyHistoryEraseEnter
};
