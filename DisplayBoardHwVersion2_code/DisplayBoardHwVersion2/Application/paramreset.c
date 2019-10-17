/*********************************************************************************
 * FileName: paramreset.c
 * Description: Code for Parameter Reset screen
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
#include "Drivers/systicktimer.h"

#include "Gesture_Sensor/GP2AP054A_cyw.h"//add cyw
/****************************************************************************/

//#define _STR_SHEETPOS_japanese "シート_ポジション?"//SHEET POS PARAMS
#define _STR_SHEETPOS_japanese "リミット_セッテイ"//SHEET POS PARAMS
#define _STR_OPRCOUNT_japanese "ドウサカウント"//OPERATION COUNT
#define _STR_MICSWCOUNT_japanese "マイクロセンサ_カウント"//MICROSWITCH COUNT

#define _STR_SHEETPOS_english "SHEET POS PARAMS?"
#define _STR_OPRCOUNT_english "OPERATION COUNT?"
#define _STR_MICSWCOUNT_english "MICROSWITCH COUNT?"

#define PARAMNO_INITSHEETPOSPARAM 120
#define PARAMNO_OPRCNTRRES 20
#define PARAMNO_MICROSENRES 81

#define SERVICED		0
#define ACTIVATED		1
#define DEACTIVATED		2


extern uint8_t flag_out_setting_cyw;
extern uint8_t KEY_PRESS_3SEC_ENT_FORRESET_CYW;
/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
// Data Structure for Flags for Internal modules
typedef union
{
	uint8_t val;
	struct
	{
		//ParamResetFlags module flag
		uint8_t paramReset		: 1;
		uint8_t bit1			: 1;
		uint8_t bit2			: 1;
		uint8_t bit3			: 1;
		uint8_t bit4			: 1;
		uint8_t bit5			: 1;
		uint8_t bit6			: 1;
		uint8_t bit7			: 1;
	} flags;
} _ParamResetFlags;

// Define Flags for Internal modules
_ParamResetFlags ParamResetFlags;
uint8_t startTimer;
uint32_t tickCount;
/****************************************************************************/
uint8_t paramResetPaintprocess(char *paramStr);
uint8_t paramResetEnterprocess(char *paramStr, uint16_t paramno);
/******************************************************************************
 * FunctionName: paramResetRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t paramResetRunTime(char *paramStr, uint16_t paramno)
{
	static uint32_t lastTickCount = 0;
	//Check Status of Param Reset Command here
	//Process result
	uint8_t viewbuff[MAX_CHARS_IN_LINE];
	 uint8_t ccccccc[MAX_CHARS_IN_LINE]={0};
	// uint8_t viewbuff[MAX_CHARS_IN_LINE];
	 //    uint8_t ccccccc[MAX_CHARS_IN_LINE]={0};
	 		// Clear Screen.
	 		//GrRectFIllBolymin(0, 126, 0, 63, true, true);

	if(KEY_PRESS_3SEC_ENT_FORRESET_CYW)
	 {
		KEY_PRESS_3SEC_ENT_FORRESET_CYW = 0;

	     GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

	 	    if(gu8_language == Japanese_IDX)
	 	    {
	 		//displayText("RESETTING", 2, 0, false, false, false, false, false, false);
	 	    displayText("リセット", 2, 16, false, false, false, false, false, false);
	 		usnprintf((char *)viewbuff, sizeof(viewbuff), "%s", paramStr);//参数名字
	 		//memcpy(ccccccc,viewbuff,strlen(viewbuff)-1);
	 		displayText(viewbuff, 2, 0, false, false, false, false, false, false);
	 		//displayText("PLEASE WAIT..", 2, 48, false, false, false, false);
	 		displayText("オマチクダサイ...", 2, 32, false, false, false, false, false, false);
	 	    }
	 	    else
	 	    {displayText("RESETTING", 2, 0, false, false, false, false, false, true);

	 			usnprintf((char *)viewbuff, sizeof(viewbuff), "%s", paramStr);//参数名字
	 			memcpy(ccccccc,viewbuff,strlen(viewbuff)-1);
	 			displayText(ccccccc, 2, 16, false, false, false, false, false, true);
	 			displayText("PLEASE WAIT...", 2, 48, false, false, false, false, false, true);
	 			//displayText("オマチクダサイ..", 2, 48, false, false, false, false, false, false);
	 	   	}

	 		//Process Parameter number
	 		if((paramno == PARAMNO_INITSHEETPOSPARAM) ||  (paramno == PARAMNO_OPRCNTRRES) || (paramno == PARAMNO_MICROSENRES)){
	 			//Set Parameter and send to UART
	 			//update Result and fail reason
	 			if((gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) && (gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS))
	 			{
	 				gstUMtoCMdatabase.commandToControlBoard.val = 0;
	 				gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
	 				gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 1;
	 				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = paramno;
	 				gstUMtoCMdatabase.destination = eDestDriveBoard;
	 				gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 1;
	 				ParamResetFlags.flags.paramReset = 1; //May be remove later. Please check for references before removing
	 				startTimer = 'Y'; //start 3sec timer for screen to stay for 3sec. Yogesh told will get reply within 500ms.
	 				tickCount  = g_ui32TickCount;

	 			}
	 			else
	 			{
	 				displayText("               ", 2, 48, false, false, false, false, false, false);   //20161207
	 				/*if(gu8_language == Japanese_IDX)
	 				displayText("コマンドソ ウシンエラー", 2, 48, false, false, false, false, false, false);
	 				else
	 				displayText("ERR SENDING CMD", 2, 48, false, false, false, false, false, true);*/
	 			}
	 		}

	 }


	if((ParamResetFlags.flags.paramReset) && (gstUMtoCMdatabase.commandRequestStatus == eACTIVE)) /*&& (gstUMtoCMdatabase.acknowledgementReceived == eACK) &&*/
	{
		if((startTimer == 'Y')&&(get_timego(tickCount) >= _WAITSYSTICK(3)))
		{
			       if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS)
					{
						startTimer = 'Z'; //Proceed for timeout screen
						GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);
						tickCount  = g_ui32TickCount;
						if(gu8_language == Japanese_IDX)
						{
						usnprintf((char *)viewbuff, sizeof(viewbuff), "%s", paramStr);//参数名字
						//memcpy(ccccccc,viewbuff,strlen(viewbuff)-1);
						displayText(viewbuff, 2, 0, false, false, false, false,false, false);
						displayText("リセットOK", 2, 16, false, false, false, false, false, false);
						}
						else
						{
						usnprintf((char *)viewbuff, sizeof(viewbuff), "%s", paramStr);//参数名字
						memcpy((uint8_t*)ccccccc,viewbuff,strlen(viewbuff)-1);
						displayText(ccccccc, 2, 0, false, false, false, false,false, true);
						displayText("RESETTING DONE", 2, 16, false, false, false, false,false,true);
						}

						gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
						gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
						gstUMtoCMdatabase.commandToControlBoard.val = 0;

						//
						// Reset operation count value on display board
						//


					}
					//--------
					else if((gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT) ||
							(gstUMtoCMdatabase.commandResponseStatus == eFAIL)) //result unsuccessful
					{
						startTimer = 'Z'; //Proceed for timeout screen
						GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);
						tickCount  = g_ui32TickCount;
						if(gu8_language == Japanese_IDX)
						//
						displayText("リセット_NG", 2, 0, false, false, false, false, false, false);
						else
						displayText("RESET FAILED", 2, 0, false, false, false, false,false,true);

						if(gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT)
						{
							if(gu8_language == Japanese_IDX)
							//
							displayText("タイムアウトシマシタ", 2, 48, false, false, false, false, false, false);
							else
							displayText("TIMEOUT OCCURED", 2, 48, false, false, false, false, false, true);
						}

						else if(gstUMtoCMdatabase.commandResponseStatus == eFAIL)
						{
							if(gu8_language == Japanese_IDX)
							//
							displayText("CRCエラー", 2, 48, false, false, false, false, false, false);
							else
							displayText("CRC ERR OCCURED", 2, 48, false, false, false, false, false, true);
						}
						else
							;

						gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
						gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
						gstUMtoCMdatabase.commandToControlBoard.val = 0;
					}
					//---------

	  }
	}
	if((ParamResetFlags.flags.paramReset) && (startTimer == 'Z'))
	{
		// Each time the timer tick occurs, process any button events.
		//if(g_ui32TickCount != lastTickCount)
		//{
			// Remember last tick count
			//lastTickCount = g_ui32TickCount;

			if(get_timego(tickCount) >= _WAITSYSTICK(3))
			{
				startTimer = ' N'; //Proceed for timeout screen
				lastTickCount = 0;

				ParamResetFlags.flags.paramReset = 0;


				if(paramno == PARAMNO_INITSHEETPOSPARAM)
				{
										//gui8SettingsModeStatus = DEACTIVATED;
					if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
				    {
						gui8SettingsModeStatus = SERVICED;
						gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus = 1;
						gstUMtoCMoperational.additionalCommandData = 0;
						gstUMtoCMoperational.commandRequestStatus = eACTIVE;
						//ui32OperationCount = 0;
					    flag_out_setting_cyw = 1;
					}
					gui8SettingsModeStatus = DEACTIVATED;
					gui8SettingsScreen = DEACTIVATED;
				}

				if(paramno == PARAMNO_OPRCNTRRES)
				{
					ui32OperationCount = 0;
					psActiveFunctionalBlock = &gsMenuFunctionalBlock;
					psActiveFunctionalBlock->pfnPaintFirstScreen();
				}

				if(paramno == PARAMNO_MICROSENRES)
				{
					psActiveFunctionalBlock = &gsMenuFunctionalBlock;
					psActiveFunctionalBlock->pfnPaintFirstScreen();
				}
			}
		//}
	}


	updateFaultLEDStatus();

	return 0;
}

uint8_t paramResetA120RunTime(void)
{
	if(gu8_language == Japanese_IDX)
	paramResetRunTime(_STR_SHEETPOS_japanese,PARAMNO_INITSHEETPOSPARAM);
	else
	paramResetRunTime(_STR_SHEETPOS_english,PARAMNO_INITSHEETPOSPARAM);

	return 0;
}
uint8_t paramResetA020RunTime(void)
{
	if(gu8_language == Japanese_IDX)
	paramResetRunTime(_STR_OPRCOUNT_japanese,PARAMNO_OPRCNTRRES);
	else
	paramResetRunTime(_STR_OPRCOUNT_english,PARAMNO_OPRCNTRRES);

	return 0;
}
uint8_t paramResetA081RunTime(void)
{
	if(gu8_language == Japanese_IDX)
	paramResetRunTime(_STR_MICSWCOUNT_japanese,PARAMNO_MICROSENRES);
	else
	paramResetRunTime(_STR_MICSWCOUNT_english,PARAMNO_MICROSENRES);

	return 0;
}
/******************************************************************************
 * FunctionName: paramResetPaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t paramResetA120Paint(void)
{
	if(gu8_language == Japanese_IDX)
	paramResetPaintprocess(_STR_SHEETPOS_japanese);
	else
	paramResetPaintprocess(_STR_SHEETPOS_english);

	gKeysStatus.bits.Key_3secEnter_pressed = 0;
	KEY_PRESS_3SEC_ENT_FORRESET_CYW =0;
	return 0;
}

uint8_t paramResetA020Paint(void)
{
	if(gu8_language == Japanese_IDX)
	paramResetPaintprocess(_STR_OPRCOUNT_japanese);
	else
	paramResetPaintprocess(_STR_OPRCOUNT_english);

	gKeysStatus.bits.Key_3secEnter_pressed = 0;
	KEY_PRESS_3SEC_ENT_FORRESET_CYW =0;
	return 0;
}

uint8_t paramResetA081Paint(void)
{
	if(gu8_language == Japanese_IDX)
	paramResetPaintprocess(_STR_MICSWCOUNT_japanese);
	else
	paramResetPaintprocess(_STR_MICSWCOUNT_english);

	gKeysStatus.bits.Key_3secEnter_pressed = 0;
	KEY_PRESS_3SEC_ENT_FORRESET_CYW =0;
	return 0;
}
/******************************************************************************
 * FunctionName: paramResetMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t paramResetMode(void)
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
 * FunctionName: paramResetEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t paramResetA120Enter(void)
{

	if(gKeysStatus.bits.Key_Enter_pressed == 0)
    {
	gKeysStatus.bits.Key_Enter_pressed = 0;
    }
	return 0;
}

uint8_t paramResetA020Enter(void)
{



	if(gKeysStatus.bits.Key_Enter_pressed == 0)
	    {
		gKeysStatus.bits.Key_Enter_pressed = 0;
	    }
	return 0;
}

uint8_t paramResetA081Enter(void)
{
	if(gKeysStatus.bits.Key_Enter_pressed == 0)
	    {
		gKeysStatus.bits.Key_Enter_pressed = 0;
	    }
	return 0;
}

/******************************************************************************
 * FunctionName: paramResetUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t paramResetUp(void)
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

	}

	return 0;
}

/******************************************************************************
 * FunctionName: paramResetDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t paramResetDown(void)
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

	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsParamResetA120FunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	paramResetA120Paint,
	paramResetA120RunTime,
	paramResetUp,
	paramResetDown,
	paramResetMode,
	paramResetA120Enter
};

stInternalFunctions gsParamResetA020FunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	paramResetA020Paint,
	paramResetA020RunTime,
	paramResetUp,
	paramResetDown,
	paramResetMode,
	paramResetA020Enter
};

stInternalFunctions gsParamResetA081FunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	paramResetA081Paint,
	paramResetA081RunTime,
	paramResetUp,
	paramResetDown,
	paramResetMode,
	paramResetA081Enter
};

uint8_t paramResetPaintprocess(char *paramStr)
{
	uint8_t viewbuff[MAX_CHARS_IN_LINE];
	uint8_t viewbuff_JAP[24];

	//
	// This is first screen paint function
	//

	// Clear Screen.
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
	if(gu8_language == Japanese_IDX)
	{
	//
	//usnprintf((char *)viewbuff, sizeof(viewbuff), "%s", paramStr);//参数名字
	//displayText(viewbuff, 2, 0, false, false, false, false, false, false);
	usnprintf((char *)viewbuff_JAP, sizeof(viewbuff_JAP), "%s", paramStr);//参数名字       //20170613  201703_No.54
	displayText(viewbuff_JAP, 2, 0, false, false, false, false, false, false);      //20170613  201703_No.54
	usnprintf((char *)viewbuff, sizeof(viewbuff), "リセットシマスカ?");
	displayText(viewbuff, 2, 16, false, false, false, false, false, false);

	}
	else
	{
	usnprintf((char *)viewbuff, sizeof(viewbuff), "DO YOU WANT TO RESET");
	displayText(viewbuff, 2, 0, false, false, false, false, false, false);
	usnprintf((char *)viewbuff, sizeof(viewbuff), "%s", paramStr);//参数名字
	displayText(viewbuff, 2, 16, false, false, false, false, false, true);
	}


	if(gu8_language == Japanese_IDX)
	{
	displayText("YES:<ENTER 3sec>", 1, 32, false, false, false, false,true,false);
	displayText("NO:<MODE>", 2, 48, false, false, false, false, false, false);
	}
	else
	{
	displayText("YES:<ENTER 3sec>", 1, 32, false, false, false, false,false,true);
	displayText("NO:<MODE>", 2, 48, false, false, false, false, false, true);
	}

	return 0;


}

uint8_t paramResetEnterprocess(char *paramStr, uint16_t paramno)
{




	return 0;
}
