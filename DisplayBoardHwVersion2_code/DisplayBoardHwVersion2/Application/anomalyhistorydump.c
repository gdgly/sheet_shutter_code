/*********************************************************************************
 * FileName: anomalyhistorydump.c
 * Description: Code for displaying 'Dump Anomaly Logs' screen
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
 *  	0.3D	18/08/2014								Handled 'dumpresult' for Empty logs
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


/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
extern uint8_t startTimer;
extern uint32_t tickCount;

static uint32_t dumpresult;

_AnomHistFlags AnomHistFlags;

uint8_t sdcardanomaly_dump_cyw=0;
uint8_t Disp_sdcardanomaly_dump_cyw=0;
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
uint8_t anomalyHistoryDumpRunTime(void)
{
	//
	// This function is called periodically
	//
	static uint32_t lastTickCount = 0;
	if(AnomHistFlags.flags.anomhistDump)
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
						displayText("エラー_リレキ", 2, 0, false, false, false, false, false, false);
						displayText("コピーOK", 2, 16, false, false, false, false, false, false);
						}
						else
						{
					      displayText("LOGS COPIED TO", 2, 0, false, false, false, false, false,true);
						  displayText("SD CARD", 2, 16, false, false, false, false, false,true);
						}
					}
					else if(dumpresult == 1){

						if(gu8_language == Japanese_IDX)
						{
						displayText("リレキコピーNG", 2, 0, false, false, false, false, false, false);
						}
						else
						{
						displayText("FAILED TO COPY", 2, 0, false, false, false, false, false,true);
						displayText("LOGS", 2, 16, false, false, false, false, false,true);
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
						 displayText("NO LOGS TO COPY", 2, 0, false, false, false, false, false,true);
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
					AnomHistFlags.flags.anomhistDump = 0;
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
uint8_t anomalyHistoryDumpPaint(void)
{
	//
	// This is first screen paint function
	//

	// Clear Screen.
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
	AnomHistFlags.val = 0;
	startTimer = 'N';
	if(gu8_language == Japanese_IDX)
	{
	displayText("SDカードへコピーシマスカ?", 2, 0, false, false, false, false, false, false);
	}
	else
	{
	displayText("DUMP TO SDCARD?", 2, 0, false, false, false, false, false,true);
	}
	displayText("YES", 2, 16, false, false, false, false,false,false);
	displayText("NO", 2, 32, true, false, false, true,false,false);
	sdcardanomaly_dump_cyw = 0;
	Disp_sdcardanomaly_dump_cyw =0;
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
uint8_t anomalyHistoryDumpMode(void)
{
	//
	// This function is called periodically
	//

	// Handle Mode key press
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
uint8_t anomalyHistoryDumpEnter(void)
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
		sdcardanomaly_dump_cyw  = Disp_sdcardanomaly_dump_cyw;
		if(sdcardanomaly_dump_cyw == 1)
		{
		// Clear Screen.
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		if(gu8_language == Japanese_IDX)
		{
		displayText("エラー_リレキ　コピー", 2, 0, false, false, false, false, false,false);
		displayText("オマチクダサイ...", 2, 16, false, false, false, false, false, false);

		}
		else
		{
		displayText("COPYING LOGS TO SD", 2, 0, false, false, false, false, false,true);
		displayText("CARD", 2, 16, false, false, false, false, false,true);
		displayText("PLEASE WAIT...", 2, 32, false, false, false, false, false,true);
		}
		dumpresult = dumpLogsToSDCard(ANOMHIST_START_IDX);
		AnomHistFlags.flags.anomhistDump = 1;
		startTimer = 'Y'; //start 3sec timer for screen to stay for 3sec
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
uint8_t anomalyHistoryDumpUp(void)
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
				displayText("SDカードへコピーシマスカ?", 2, 0, false, false, false, false, false, false);
		       }
		       else
		       {
		    	displayText("DUMP TO SDCARD?", 2, 0, false, false, false, false, false,true);
		       }
				if(Disp_sdcardanomaly_dump_cyw == 0)
				{
					Disp_sdcardanomaly_dump_cyw = 1;
								displayText("YES", 2, 16, true, false, false, true,false,false);
								displayText("NO", 2, 32, false, false, false, false,false,false);
				}
				else
				{
					Disp_sdcardanomaly_dump_cyw = 0;
								displayText("YES", 2, 16, false, false, false, false,false,false);
								displayText("NO", 2, 32, true, false, false, true,false,false);
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
uint8_t anomalyHistoryDumpDown(void)
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
			displayText("SDカードへコピーシマスカ?", 2, 0, false, false, false, false, false, false);
		}
		else
	   {
			displayText("DUMP TO SDCARD?", 2, 0, false, false, false, false, false,true);
	   }
	   if(Disp_sdcardanomaly_dump_cyw == 0)
		{
			Disp_sdcardanomaly_dump_cyw = 1;
			displayText("YES", 2, 16, true, false, false, true,false,false);
			displayText("NO", 2, 32, false, false, false, false,false,false);
		}
		else
		{
			Disp_sdcardanomaly_dump_cyw = 0;
			displayText("YES", 2, 16, false, false, false, false,false,false);
			displayText("NO", 2, 32, true, false, false, true,false,false);
		}
	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsAnomalyHistoryDumpFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	anomalyHistoryDumpPaint,
	anomalyHistoryDumpRunTime,
	anomalyHistoryDumpUp,
	anomalyHistoryDumpDown,
	anomalyHistoryDumpMode,
	anomalyHistoryDumpEnter
};
