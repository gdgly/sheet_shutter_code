/*********************************************************************************
 * FileName: changesettinghistorydump.c
 * Description: Code for displaying 'Dump Change Settings Logs' screen
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
 *  	0.3D	18/08/2014								dumpLogsToSDCard() modified for Empty Logs
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


// Extern Flags for Internal modules
extern _ChgSetHistFlags chgSetHistFlags;

extern uint8_t startTimer;
extern uint32_t tickCount;

static uint32_t dumpresult;
uint8_t sdcardsetting_dump_cyw=0;
uint8_t Disp_sdcardsetting_dump_cyw=0;
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
uint8_t changeSettingHistoryDumpRunTime(void)
{
	//
	// This function is called periodically
	//
	static uint32_t lastTickCount = 0;

	if(chgSetHistFlags.flags.cshistDump)
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
					if(dumpresult == 0) {

						if(gu8_language == Japanese_IDX)
						{
						displayText("セッテイ_リレキ", 2, 0, false, false, false, false, false, false);
						displayText("コピーOK", 2, 16, false, false, false, false, false, false);
						}
						else
						{
					    displayText("LOGS COPIED TO", 2, 0, false, false, false, false, false, true);
						displayText("SD CARD", 2, 16, false, false, false, false,false, true);

						}

					}
					else if(dumpresult == 1){

						if(gu8_language == Japanese_IDX)
						{
						displayText("リレキコピーNG", 2, 0, false, false, false, false, false, false);
						}
						else
						{
					     displayText("FAILED TO COPY", 2, 0, false, false, false, false, false, true);
						 displayText("LOGS", 2, 16, false, false, false, false, false, true);
						}
					}
					else if(dumpresult == 2)
					{

						if(gu8_language == Japanese_IDX)
						{
						displayText("リレキノデータガ", 2, 0, false, false, false, false,false,false);
						displayText("アリマセン", 2, 16, false, false, false, false,false,false);
						}
						else
						{
						displayText("NO LOGS TO COPY", 2, 0, false, false, false, false,false,true);
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
					startTimer = 'N'; //Proceed for timeout scren
					lastTickCount = 0;
					psActiveFunctionalBlock = &gsMenuFunctionalBlock;
					psActiveFunctionalBlock->pfnPaintFirstScreen();
					chgSetHistFlags.flags.cshistDump = 0;
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
uint8_t changeSettingHistoryDumpPaint(void)
{
	//
	// This is first screen paint function
	//

	//
	// Clear Screen.
	//
	chgSetHistFlags.val = 0;
	startTimer = 'N';

	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

	//displayText("DUMP TO SDCARD?", 2, 0, false, false, false, false);
	if(gu8_language == Japanese_IDX)
	{
	displayText("SDカードへコピーシマスカ?", 2, 0, false, false, false, false, false, false);
	}
	else
	{
	displayText("DUMP TO SDCARD?", 2, 0, false, false, false, false,false,true);
	}
	displayText("YES", 2, 16, false, false, false, false,false,false);
	displayText("NO", 2, 32, true, false, false, true,false,false);
	sdcardsetting_dump_cyw = 0;
	Disp_sdcardsetting_dump_cyw =0;
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
uint8_t changeSettingHistoryDumpMode(void)
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
uint8_t changeSettingHistoryDumpEnter(void)
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
		sdcardsetting_dump_cyw  = Disp_sdcardsetting_dump_cyw;
		//
		// Clear Screen.
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		if(sdcardsetting_dump_cyw==1)
		{
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		if(gu8_language == Japanese_IDX)
		{
		displayText("セッテイ_リレキ　コピー", 2, 0, false, false, false, false,false,false);
		displayText("オマチクダサイ...", 2, 16, false, false, false, false, false, false);

		}
		else
		{
		displayText("COPYING LOGS TO SD", 2, 0, false, false, false, false,false,true);
		displayText("CARD", 2, 16, false, false, false, false,false,true);
		displayText("PLEASE WAIT...", 2, 32, false, false, false, false,false,true);
		}
		dumpresult = dumpLogsToSDCard(CHGSETHIST_START_IDX);
		chgSetHistFlags.flags.cshistDump = 1; //May be remove later. Please check for references before removing
		startTimer = 'Y'; //start 3sec timer for screen to stay for 3sec. Yogesh told will get reply within 500ms.
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
uint8_t changeSettingHistoryDumpUp(void)
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
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
		if(gu8_language == Japanese_IDX)
		{
			displayText("SDカードへコピーシマスカ?", 2, 0, false, false, false, false, false, false);
		}
		else
		{
		   displayText("DUMP TO SDCARD?", 2, 0, false, false, false, false,false,true);
		}
		if(Disp_sdcardsetting_dump_cyw == 0)
		{
						Disp_sdcardsetting_dump_cyw = 1;
						displayText("YES", 2, 16, true, false, false, true,false,false);
						displayText("NO", 2, 32, false, false, false, false,false,false);
		}
		else
		{
					    Disp_sdcardsetting_dump_cyw = 0;
						displayText("YES", 2, 16, false, false, false, false,false,false);
						displayText("NO", 2, 32, true, false, false, true,false,false);
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
uint8_t changeSettingHistoryDumpDown(void)
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
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
				GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
				if(gu8_language == Japanese_IDX)
				{
				  displayText("SDカードへコピーシマスカ?", 2, 0, false, false, false, false, false, false);
				}
				else
				{
				 displayText("DUMP TO SDCARD?", 2, 0, false, false, false, false,false,true);
				}
				if(Disp_sdcardsetting_dump_cyw == 0)
				{
								Disp_sdcardsetting_dump_cyw = 1;
								displayText("YES", 2, 16, true, false, false, true,false,false);
								displayText("NO", 2, 32, false, false, false, false,false,false);
				}
				else
				{
							    Disp_sdcardsetting_dump_cyw = 0;
								displayText("YES", 2, 16, false, false, false, false,false,false);
								displayText("NO", 2, 32, true, false, false, true,false,false);
				}
	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsChangeSettingHistoryDumpFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	changeSettingHistoryDumpPaint,
	changeSettingHistoryDumpRunTime,
	changeSettingHistoryDumpUp,
	changeSettingHistoryDumpDown,
	changeSettingHistoryDumpMode,
	changeSettingHistoryDumpEnter
};
