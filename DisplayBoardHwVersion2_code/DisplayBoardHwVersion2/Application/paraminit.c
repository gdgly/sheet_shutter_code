/*********************************************************************************
 * FileName: paraminit.c
 * Description: Code for Parameter Initialization screen
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
 *  	0.1D	20/06/2014      	iGATE Offshore team       Initial Creation
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
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "userinterface.h"
#include "intertaskcommunication.h"
#include "Middleware/paramdatabase.h"
#include "sdcardlogs.h"
#include "logger.h"
#include "Middleware/sdcard.h"

#define SERVICED		0
#define ACTIVATED		1
#define DEACTIVATED		2


extern uint8_t flag_out_setting_cyw;
extern uint8_t gui8SettingsModeStatus;


void Para_On_Display_Board_init_cyw(void);
extern uint8_t KEY_PRESS_3SEC_ENT_FORRESET_CYW;
/****************************************************************************/

#define PARAMNO_INITVAL 21
/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/
// Data Structure for Flags for Internal modules
typedef union {
	uint8_t val;
	struct {
		//ParamInitFlags module flag
		uint8_t paramInit :1;
		uint8_t bit1 :1;
		uint8_t bit2 :1;
		uint8_t bit3 :1;
		uint8_t bit4 :1;
		uint8_t bit5 :1;
		uint8_t bit6 :1;
		uint8_t bit7 :1;
	} flags;
} _ParamInitFlags;

// Define Flags for Internal modules
_ParamInitFlags ParamInitFlags;
extern uint8_t startTimer;
extern uint32_t tickCount;

#if 0
/******************************************************************************
 * FunctionName: paramInitRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t paramInitRunTime(void)
{
	//
	// This function is called periodically
	//

	//--------------------------------
	static uint32_t lastTickCount = 0;
	//Check Status of Param Reset Command here
	//Process result

	if((ParamInitFlags.flags.paramInit) && (gstUMtoCMdatabase.commandRequestStatus == eACTIVE)) /*&& (gstUMtoCMdatabase.acknowledgementReceived == eACK) &&*/
	{
		if(startTimer == 'Y')
		{
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

					GrRectFIllBolymin(0, 126, 0, 63, true, true);
					if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS)
					{
						displayText("INITIALIZATION", 2, 0, false, false, false, false);
						displayText("SUCCESSFUL", 2, 16, false, false, false, false);

						gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
						gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
					}
					//--------
					else if((gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT) ||
							(gstUMtoCMdatabase.commandResponseStatus == eFAIL))//result unsuccessful
					{
						displayText("INITIALIZATION", 2, 0, false, false, false, false);
						displayText("FAILED", 2, 16, false, false, false, false);

						gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
						gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

						if(gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT)
						{
							gstDisplayBoardFault.bits.displayCommunication = 1;
							gstDisplayCommunicationFault.bits.commFailControl = 1;
						}

						else if(gstUMtoCMdatabase.commandResponseStatus == eFAIL)
						{
							gstDisplayBoardFault.bits.displayCommunication = 1;
							gstDisplayCommunicationFault.bits.crcError = 1;
						}
						else
						;
					}
					//---------

				} //----if(tickCount3Secs++ == _WAITSYSTICK_3SEC)
			} //----if(g_ui32TickCount != lastTickCount)
		} //----if(startTimer == 'Y')
	}
	else if((ParamInitFlags.flags.paramInit) && (startTimer == 'Z'))
	{
		// Each time the timer tick occurs, process any button events.
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
				ParamInitFlags.flags.paramInit = 0;
			}
		}
	}
	//---------------------------------
	//-----------------------------------

	updateFaultLEDStatus();

	return 0;
}
#endif

/******************************************************************************
 * FunctionName: paramInitRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t paramInitRunTime(void) {
	//
	// This function is called periodically
	//
	static uint8_t lsucDriveParamResetStart = 0;
		static uint8_t lsui8Delay3SecStart = 0;
		static uint8_t Sec2Start_cyw=0;
		static uint32_t lsui32TickCount3Seconds = 0;

	#ifdef DISP_TARGET_BOARD
		if (KEY_PRESS_3SEC_ENT_FORRESET_CYW) {
			KEY_PRESS_3SEC_ENT_FORRESET_CYW = 0;
	#endif
			lsui32TickCount3Seconds = g_ui32TickCount;
			Sec2Start_cyw =1;
			// Clear Screen.
			//GrRectFIllBolymin(0, 126, 0, 63, true, true);
			GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

			if(gu8_language == Japanese_IDX)
			{
			displayText("オールリセット", 2, 0, false, false, false, false,false,false);
			displayText("リセット", 2, 16, false, false, false, false, false, false);
			//displayText("ヨミコミシテイマス", 2, 0, false, false, false, false,false,false);
			//displayText("PROGRESS", 2, 16, false, false, false, false, false, false);
			displayText("オマチクダサイ...", 2, 32, false, false, false, false, false, false);
			}
			else
			{
			displayText("INITIALIZATION IN", 2, 0, false, false, false, false,false,true);
			displayText("PROGRESS", 2, 16, false, false, false, false, false, true);
			displayText("PLEASE WAIT...", 2, 48, false, false, false, false, false, true);
			}
			if ((gstUMtoCMdatabase.commandRequestStatus == eINACTIVE)
					&& (gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS)) {
				gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
				gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 1;
				gstUMtoCMdatabase.dataToControlBoard.parameterNumber =
				PARAMNO_INITVAL;
				gstUMtoCMdatabase.destination = eDestControlBoard;
				gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue =
						1;
			} else {
				displayText("               ", 2, 48, false, false, false, false, false, false);   //20161207
				/*if(gu8_language == Japanese_IDX)
				//
				displayText("コマンドソ ウシンエラー", 2, 48, false, false, false, false,false,false);
				else
				displayText("ERROR SENDING CMD", 2, 48, false, false, false, false,false,true);*/
			}
		}

		if((Sec2Start_cyw == 1)&&(get_timego( lsui32TickCount3Seconds) >= 300))
		{
		if (gstUMtoCMdatabase.commandRequestStatus == eACTIVE) {
			if (gstUMtoCMdatabase.commandResponseStatus == eSUCCESS) {
				gstUMtoCMdatabase.commandToControlBoard.val = 0;

				if (1 == lsucDriveParamResetStart) {
					GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);

					if(gu8_language == Japanese_IDX)
					{
					displayText("オールリセット", 2, 0, false, false, false, false, false, false);
					displayText("リセット_OK", 2, 16, false, false, false, false, false, false);
					}
					else
					{
				    displayText("INITIALIZATION", 2, 0, false, false, false, false, false, true);
					displayText("SUCCESSFUL", 2, 16, false, false, false, false, false, true);
					}
					//
					// Start 3 seconds delay
					// Capture time
					//
					lsui8Delay3SecStart = 1;
					Sec2Start_cyw = 0;
					lsui32TickCount3Seconds = g_ui32TickCount;
				}
				//
				// Reset request and response status (control board parameters reset completed)
				//
				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

				if ((gstUMtoCMdatabase.commandRequestStatus == eINACTIVE)
						&& (gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS)
						&& (0 == lsucDriveParamResetStart)) {
					lsucDriveParamResetStart = 1;
					gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
					gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 1;
					gstUMtoCMdatabase.dataToControlBoard.parameterNumber =
					PARAMNO_INITVAL;
					gstUMtoCMdatabase.destination = eDestDriveBoard;
					gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue =
							1;
				}

			} else if ((gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT)
					|| (gstUMtoCMdatabase.commandResponseStatus == eFAIL)) {
	#if 0
				//
				// Set communication fault flags
				//
				if(gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT)
				{
					gstDisplayBoardFault.bits.displayCommunication = 1;
					gstDisplayCommunicationFault.bits.commFailControl = 1;
				}

				if(gstUMtoCMdatabase.commandResponseStatus == eFAIL)
				{
					gstDisplayBoardFault.bits.displayCommunication = 1;
					gstDisplayCommunicationFault.bits.crcError = 1;
				}
	#endif
				//
				// Reset request and response status
				//
				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

				gstUMtoCMdatabase.commandToControlBoard.val = 0;

				//
				// Display initialization failed message
				//
				GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);

				if(gu8_language == Japanese_IDX)
				{

				displayText("オールリセット", 2, 0, false, false, false, false, false, false);
				displayText("リセット_NG", 2, 16, false, false, false, false, false, false);
				}
				else
				{
				displayText("INITIALIZATION", 2, 0, false, false, false, false, false, true);
				displayText("FAILED", 2, 16, false, false, false, false, false, true);
				}
				//
				// Start 3 seconds delay
				// Capture time
				//
				lsui8Delay3SecStart = 1;
				Sec2Start_cyw = 0;
				lsui32TickCount3Seconds = g_ui32TickCount;
			}

		}
		}
		if ((get_timego( lsui32TickCount3Seconds) >= 300)
				&& (1 == lsui8Delay3SecStart)) {

			Para_On_Display_Board_init_cyw();

			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			 {
					gui8SettingsModeStatus = SERVICED;
					gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus = 1;
					gstUMtoCMoperational.additionalCommandData = 0;
					gstUMtoCMoperational.commandRequestStatus = eACTIVE;
					flag_out_setting_cyw = 1;
			}
			gui8SettingsModeStatus = DEACTIVATED;
			gui8SettingsScreen = DEACTIVATED;


			lsucDriveParamResetStart = 0;
			lsui8Delay3SecStart = 0;
			//psActiveFunctionalBlock = &gsMenuFunctionalBlock;
			//psActiveFunctionalBlock->pfnPaintFirstScreen();
		}

	//	gKeysStatus.bits.Key_Enter_pressed = 0;

		return 0;
	updateFaultLEDStatus();

	return 0;
}
/******************************************************************************
 * FunctionName: paramInitPaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t paramInitPaint(void) {
	// Clear Screen.
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
	KEY_PRESS_3SEC_ENT_FORRESET_CYW =0;
	if(gu8_language == Japanese_IDX)
	{
	//displayText("INIT SHUTTER & DRIVE", 2, 0, false, false, false, false,false,true);
	//displayText("PARAMS TO DEFAULT?", 2, 16, false, false, false, false,false,true);
	displayText("オールリセット", 2, 0, false, false, false, false,false,false);
	displayText("リセットシマスカ?", 2, 16, false, false, false, false,false,false);
	displayText("YES:<ENTER 3sec>", 1, 32, false, false, false, false,true,false);
	displayText("NO:<MODE>", 2, 48, false, false, false, false, false, false);
	}
	else
	{
	displayText("INIT SHUTTER & DRIVE", 2, 0, false, false, false, false,false,true);
	displayText("PARAMS TO DEFAULT?", 2, 16, false, false, false, false,false,true);
	displayText("YES:<ENTER 3sec>", 1, 32, false, false, false, false,false,true);
	displayText("NO:<MODE>", 2, 48, false, false, false, false, false, true);
	}
	return 0;
}

/******************************************************************************
 * FunctionName: paramInitMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t paramInitMode(void) {
	//
	// This function is called periodically
	//

	//
	// Handle Mode key press
	//
	if (gKeysStatus.bits.Key_Mode_pressed) {
		gKeysStatus.bits.Key_Mode_pressed = 0;

		psActiveFunctionalBlock = &gsMenuFunctionalBlock;
		psActiveFunctionalBlock->pfnPaintFirstScreen();
	}

	return 0;
}

/******************************************************************************
 * FunctionName: paramInitEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t paramInitEnter(void) {
	if(gKeysStatus.bits.Key_Enter_pressed == 1)
	{
	gKeysStatus.bits.Key_Enter_pressed = 0;

	}

	return 0;
}

/******************************************************************************
 * FunctionName: paramInitUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t paramInitUp(void) {
	//
	// This function is called periodically
	//

	//
	// Handle Up key press
	//
	if (gKeysStatus.bits.Key_Up_pressed) {
		gKeysStatus.bits.Key_Up_pressed = 0;

	}

	return 0;
}

/******************************************************************************
 * FunctionName: paramInitDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t paramInitDown(void) {
	//
	// This function is called periodically
	//

	//
	// Handle Down key press
	//
	if (gKeysStatus.bits.Key_Down_pressed) {
		gKeysStatus.bits.Key_Down_pressed = 0;

	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
 *********************************************************************************/
stInternalFunctions gsParamInitFunctionalBlock =
{
		0,
		&gsMenuFunctionalBlock,
		paramInitPaint,
		paramInitRunTime,
		paramInitUp,
		paramInitDown,
		paramInitMode,
		paramInitEnter
};
