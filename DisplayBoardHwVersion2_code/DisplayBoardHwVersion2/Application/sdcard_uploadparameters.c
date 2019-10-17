/*********************************************************************************
 * FileName: sdcard_uploadparameters.c
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
 *  	0.2D	11/08/2014									gstUMtoCMdatabase command flags set to 0
 *  														UMtoCMDatabase block resets added in NACK, TIMEOUT and SUCCESS case
 *  														SDCard Failure message shown
 *  	0.1D	08/08/2014      	iGATE Offshore team       Initial Creation
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
#include "parameterlist.h"
#include "Middleware/sdcard.h"
#include "Middleware/rtc.h"
#include "Middleware/paramdatabase.h"
/****************************************************************************/

uint32_t strtoint(char a[]);
/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
unsigned char uploadParamFilenames[MAX_FILES_FOR_SDPARAM - 1][MAX_CHARS_IN_LINE];
/****************************************************************************/

// Data Structure for Flags for Internal modules
typedef union
{
	uint8_t val;
	struct
	{
		uint8_t paramULEnterpressed		: 1;
		uint8_t paramDriveupload		: 1;
		uint8_t paramControlupload	 	: 1;
		uint8_t paramDriveuploadDone	: 1;
		uint8_t paramControluploadDone 	: 1;
		uint8_t bit5					: 1;
		uint8_t bit6					: 1;
		uint8_t bit7					: 1;
	} flags;
} _UploadParamFlags;

// Define Flags for Internal modules
_UploadParamFlags UploadParamFlags;

/*static */extern uint8_t gui8DriveParamCount; //Max Drive param count is number_of_drive_parameters
/*static */extern uint8_t gui8ControlParamCount; //Max Control param count is _NO_OF_CTRL_PARAMS
char gcULParamfilename[35];
unsigned char gucParamULFileReadBuff[MAX_CHARS_IN_LINE];

extern struct tm gstcurrentDateTime;

uint8_t sdcardupload_parameter_cyw=0;
uint8_t Disp_sdcardupload_parameter_cyw=0;


extern const uint16_t  PARAM_ALL_SDCARD_CYW[param_all_list_long];
extern const dest_enum_cyw PARAM_ALL_BELONG_CYW[param_all_list_long];

extern uint16_t gu16_lcdlight;
extern unsigned char menu_gesture_flag_cyw;
/******************************************************************************
 * FunctionName: uploadParametersRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t uploadParametersRunTime(void)
{
	//
	// This function is called periodically
	//
	menuRunTime();

	return 0;
}

/******************************************************************************
 * FunctionName: uploadParametersPaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t uploadDrvParametersPaint(void)
{
	uint8_t i = 0,Tp_max_sdcardlist=0;
	if(gu8_language == Japanese_IDX)
	{
	gSDCardUPParamDRVbrdMenuItems[i].pcText_japanese = "ファイル_センタク:";
	}
	else
	{
	gSDCardUPParamDRVbrdMenuItems[i].pcText_english = "SELECT FILES:      ";
	}
	gSDCardUPParamDRVbrdMenuItems[i].psChildMenu = 0;
	gSDCardUPParamDRVbrdMenuItems[i].childFunctionalBlock = 0;

	Tp_max_sdcardlist = list_SDCard_cyw();

	for(i = 0; i < Tp_max_sdcardlist; i++)
	{
		//if(list_SDCard("drv_", i+1, (char *)uploadParamFilenames[i]) != FR_OK)
		//	break;

//		ustrncpy((char *)gSDCardUPParamDRVbrdMenuItems[i+1].pcText, (const char*)&tempFnamebuff, ustrlen((const char *)tempFnamebuff));
		if(gu8_language == Japanese_IDX)
		{
		gSDCardUPParamDRVbrdMenuItems[i+1].pcText_japanese = uploadParamFilenames[i];
		}
		else
		{
		gSDCardUPParamDRVbrdMenuItems[i+1].pcText_english = uploadParamFilenames[i];
		}
		gSDCardUPParamDRVbrdMenuItems[i+1].psChildMenu = 0;
		gSDCardUPParamDRVbrdMenuItems[i+1].childFunctionalBlock = &gsSDUpParamDRVFunctionalBlock;
	}

	gSDCardUPParamDRVbrdMenu.ui8Items = i+1;
	return 0;
}

uint8_t uploadCtrlParametersPaint(void)
{
	uint8_t i = 0;
	if(gu8_language == Japanese_IDX)
	{
	gSDCardUPParamCTRLbrdMenuItems[i].pcText_japanese = "ファイル_センタク:";
	}
	else
	{
		gSDCardUPParamCTRLbrdMenuItems[i].pcText_english = "SELECT FILES:      ";
	}
	gSDCardUPParamCTRLbrdMenuItems[i].psChildMenu = 0;
	gSDCardUPParamCTRLbrdMenuItems[i].childFunctionalBlock = 0;

	for(i = 0; i < MAX_FILES_FOR_SDPARAM; i++)
	{
		if(list_SDCard("ctl_", i+1, (char *)uploadParamFilenames[i]) != FR_OK)
			break;

//		ustrncpy((char *)gSDCardUPParamDRVbrdMenuItems[i+1].pcText, (const char*)&tempFnamebuff, ustrlen((const char *)tempFnamebuff));
		if(gu8_language == Japanese_IDX)
		{
		gSDCardUPParamCTRLbrdMenuItems[i+1].pcText_japanese = uploadParamFilenames[i];
		}
		else
		{
		gSDCardUPParamCTRLbrdMenuItems[i+1].pcText_english = uploadParamFilenames[i];
		}
		gSDCardUPParamCTRLbrdMenuItems[i+1].psChildMenu = 0;
		gSDCardUPParamCTRLbrdMenuItems[i+1].childFunctionalBlock = &gsSDUpParamCTRLFunctionalBlock;
	}

	gSDCardUPParamCTRLbrdMenu.ui8Items = i+1;
	return 0;
}

/******************************************************************************
 * FunctionName: uploadParametersPaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t SDUpParamDRVPaint(void)
{
	UploadParamFlags.val = 0;
	gui8DriveParamCount = 0;
	gui8ControlParamCount = 0;

	GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);
	//displayText("ARE YOU SURE TO", 2, 0, false, false, false, false,true,false);
	//displayText("UPLOAD PARAMETERS", 2, 16, false, false, false, false,false,true);
	//displayText("FROM SD CARD?", 2, 32, false, false, false, false, false, false);
	if(gu8_language == Japanese_IDX)
	{
	displayText("パラメータヲSDカードニ", 2, 0, false, false, false, false,false,false);
	displayText("コピーシマスカ?", 2, 16, false, false, false, false,false,false);
	}
	else
	{
	displayText("ARE YOU SURE TO", 2, 0, false, false, false, false,false,true);
	displayText("UPLOAD PARAMETERS?", 2, 16, false, false, false, false,false,true);
	}
	//displayText("Y:<ENT>, N:<MOD>", 1, 48, false, false, false, false,true,false);
	displayText("YES", 2, 32, false, false, false, false,false,false);
	    displayText("NO", 2, 48, true, false, false, true,false,false);

		sdcardupload_parameter_cyw=0;
		Disp_sdcardupload_parameter_cyw=0;
	return 0;
}

uint8_t SDUpParamDRVCTRLRunTime(void)
{

	static uint8_t lsDelay3SecStart = 0;
	static uint32_t lsTickCount3Seconds = 0;
	static uint8_t SDULfileLineNo;
	char i,j;

  //  #define diver_list_long   19
   // #define control_list_long  22
  //  const uint8_t diver_list[diver_list_long]  ={29,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,58,59,60};
   // const uint8_t control_list[control_list_long]={17,18,19,20,21,22,23,24,25,26,27,28,33,34,35,36,37,38,39,40,41,57};


	unsigned char readResult;
	char SDULParamNo[5], SDULParamValue[15];

	if(UploadParamFlags.flags.paramULEnterpressed)
	{
		if((gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) && (gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS))
		{
			//Set details for SET PARAMETER here
			if(UploadParamFlags.flags.paramDriveupload)
			{
				if(!gui8DriveParamCount)
				{

					SDULfileLineNo = 3;
					// Fill Details
					gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 1;
					gstUMtoCMdatabase.destination = PARAM_ALL_BELONG_CYW[gui8DriveParamCount];//eDestDriveBoard;
				}

				if(gui8DriveParamCount < param_all_list_long)
				{
					//if(gstUMtoCMdatabase.destination!=eDestDisplayBoard)
					//{

						// Create File and add Header for file
						memset(gucParamULFileReadBuff, 0, sizeof(gucParamULFileReadBuff));
	//					writeResult = write_SDCard(gcULParamfilename, gucParamULFileReadBuff, &retbytes,FILE_OVERWRITE);
						readResult = read_SDCard(gcULParamfilename, (const void *)gucParamULFileReadBuff, SDULfileLineNo++);

						if(readResult)
						{
							UploadParamFlags.val = 0;
							//Reset Added in Auto-GoBack Delay Start section
							gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
							gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
							gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
							gstUMtoCMdatabase.commandToControlBoard.val = 0;

							// Clear Screen
							//GrRectFIllBolymin(0, 126, 0, 63, true, true);
							GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

							// Display SDCARD failed message
							//displayText("SDCARD FAILURE", 2, 0, false, false, false, false, false, false);
							if(gu8_language == Japanese_IDX)
							displayText("SDカードフリョウ", 2, 0, false, false, false, false, false, false);
							else
							displayText("SDCARD FAILURE", 2, 0, false, false, false, false, false, true);

							//
							// Capture Time
							//
							lsTickCount3Seconds = g_ui32TickCount;

							//
							// Set delay start flag
							//
							lsDelay3SecStart = 1;
							//return 0;
						}

						for(i = 0; i < sizeof(SDULParamNo); i++) {
							if(gucParamULFileReadBuff[i] == ',')
								break;
							SDULParamNo[i] = gucParamULFileReadBuff[i];
						}
						SDULParamNo[i] = 0; //Null character

						for(j = 0; j < sizeof(SDULParamValue); j++) {
							SDULParamValue[j] = gucParamULFileReadBuff[i+j+1];
							if(gucParamULFileReadBuff[i+j+1] == 0)
								break;
						}
	//					SDULParamValue[j] = 0; //Null character //Adjusted in above code

						gstUMtoCMdatabase.destination = PARAM_ALL_BELONG_CYW[gui8DriveParamCount];

						if(gstUMtoCMdatabase.destination!=eDestDisplayBoard)
						{
						gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 1;
						gstUMtoCMdatabase.dataToControlBoard.parameterNumber = strtoint(SDULParamNo);//ustrtoul (SDULParamNo, NULL, 10);
						gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
						gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = strtoint(SDULParamValue);
						}
						else
						{
							if(PARAM_ALL_SDCARD_CYW[gui8DriveParamCount]==26)
							{
								gu16_lcdlight =  strtoint(SDULParamValue);
								writeParameterUpdateInDB((PARAM_DISP)gsParamDatabase[Para_LcdBackLight_Index_cyw].paramEEPROMIndex, (uint8_t *)&gu16_lcdlight);

							}
							if(PARAM_ALL_SDCARD_CYW[gui8DriveParamCount]==41)
							{
								menu_gesture_flag_cyw =strtoint(SDULParamValue);
								writeParameterUpdateInDB((PARAM_DISP)gsParamDatabase[Para_Guesture_Index_cyw].paramEEPROMIndex, (uint8_t *)&menu_gesture_flag_cyw);

							}
							if(PARAM_ALL_SDCARD_CYW[gui8DriveParamCount]==42)
							{
								gu8_language = strtoint(SDULParamValue);
								writeParameterUpdateInDB((PARAM_DISP)gsParamDatabase[Para_Languange_Index_cyw].paramEEPROMIndex, (uint8_t *)&gu8_language);

							}
							gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
							gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
							gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
						    gstUMtoCMdatabase.commandToControlBoard.val = 0;
						}



					gui8DriveParamCount++;
				}
				else
				{
					// Also take this decision in ACK SUCCESS section
					gui8DriveParamCount = 0;
					UploadParamFlags.flags.paramDriveupload = 0;
					UploadParamFlags.flags.paramDriveuploadDone = 1;
				}
			}

			else if(UploadParamFlags.flags.paramDriveuploadDone)
			{
				// All Control and Drive parameters are downloaded
				// Initiate Closure
				UploadParamFlags.val = 0;
				//Reset
				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
				gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
				gstUMtoCMdatabase.commandToControlBoard.val = 0;
				if(0 == lsDelay3SecStart)
				{
					//
					// Clear Screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

					//
					// Display failed message on second line
					//
					//displayText("UPLOAD", 2, 0, false, false, false, false, false, false);

					//
					// Paint error reason on fourth line
					//
					//
					if(gu8_language == Japanese_IDX)
					displayText("コピーカンリョウ", 2, 0, false, false, false, false, false, false);
					else
					displayText("SUCCESSFUL", 2, 16, false, false, false, false, false, true);

					//
					// Capture Time
					//
					lsTickCount3Seconds = g_ui32TickCount;

					//
					// Set delay start flag
					//
					lsDelay3SecStart = 1;
				}
			}
		}
		else if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		{
			if(eNO_STATUS == gstUMtoCMdatabase.commandResponseStatus)
			{

			}
			else if(eSUCCESS == gstUMtoCMdatabase.commandResponseStatus)
			{
				if(eACK == gstUMtoCMdatabase.acknowledgementReceived)
				{
					gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
					gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 0;

					if((UploadParamFlags.flags.paramDriveupload) &&
					  (gui8DriveParamCount  >= param_all_list_long))
					{
						gui8DriveParamCount = 0;
						UploadParamFlags.flags.paramDriveupload = 0;
						UploadParamFlags.flags.paramDriveuploadDone = 1;
					}

					//
					// Reset Request and response status and acknowledgment status
					//
					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
					gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
					lsTickCount3Seconds = g_ui32TickCount;
				}
				else if(eNACK == gstUMtoCMdatabase.acknowledgementReceived)
				{
					UploadParamFlags.val = 0;
					//Reset
					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
					gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
					gstUMtoCMdatabase.commandToControlBoard.val = 0;
					//
					// Start delay
					//
					if(0 == lsDelay3SecStart)
					{
						//
						// Clear Screen
						//
						//GrRectFIllBolymin(0, 126, 0, 63, true, true);
						GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

						// Display parameter setting action failed message

						if(gu8_language == Japanese_IDX)
						{
						displayText("カキコミNG", 2, 0, false, false, false, false, false, false);
						}
						else
						{
						displayText("DOWNLOAD", 2, 0, false, false, false, false, false, true);
						displayText("FAILED", 2, 16, false, false, false, false, false, true);
						}

						//displayText("NACK ERROR", 2, 48, false, false, false, false, false, false);
						if(gu8_language == Japanese_IDX)
						displayText("NACKエラー", 2, 48, false, false, false, false, false, false);
						else
					    displayText("NACK ERROR", 2, 48, false, false, false, false, false, true);
						//
						// Capture Time
						//
						lsTickCount3Seconds = g_ui32TickCount;

						//
						// Set delay start flag
						//
						lsDelay3SecStart = 1;
					}
				}
			}
			else if( (gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT) ||
					(gstUMtoCMdatabase.commandResponseStatus == eFAIL)
			)
			{
				UploadParamFlags.val = 0;
				//Reset
				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
				gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
				gstUMtoCMdatabase.commandToControlBoard.val = 0;

				//
				// Start delay
				//
				if(0 == lsDelay3SecStart)
				{
					//
					// Clear Screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

					// Display parameter setting action failed message
				//	displayText("DOWNLOAD", 2, 0, false, false, false, false, false, false);
				//	displayText("FAILED", 2, 16, false, false, false, false, false, false);
					if(gu8_language == Japanese_IDX)
					{
					displayText("カキコミNG", 2, 0, false, false, false, false, false, false);
					}
					else
					{
					displayText("DOWNLOAD", 2, 0, false, false, false, false, false, true);
					displayText("FAILED", 2, 16, false, false, false, false, false, true);
					}

					//
					if(gu8_language == Japanese_IDX)
					displayText("ツウシンエラー", 2, 48, false, false, false, false, false, false);
					else
					displayText("COMM ERROR", 2, 48, false, false, false, false, false, true);
					//
					// Capture Time
					//
					lsTickCount3Seconds = g_ui32TickCount;

					//
					// Set delay start flag
					//
					lsDelay3SecStart = 1;
				}
			}
		}
		else //Check also if all Parameters are fetched //if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		{
			displayText("               ", 2, 48, false, false, false, false, false, false);   //20161207
			/*if(gu8_language == Japanese_IDX)
			displayText("コマンドソ ウシンエラー", 2, 48, false, false, false, false,false,false);
			else
			displayText("ERROR SENDING CMD", 2, 48, false, false, false, false,false,true);*/
		}
	}
	else
		; //Handle else part of (UploadParamFlags.flags.paramDLEnterpressed)


	// Check whether delay is achieved
	if((get_timego( lsTickCount3Seconds) > 300) && (1 == lsDelay3SecStart))
	{
		//
		// Return to parameter list functional block.
		//
		psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
		uploadDrvParametersPaint();
		menuPaintFirstScreen();

		//
		// Reset delay start flag
		//
		lsDelay3SecStart = 0;
	}

	updateFaultLEDStatus();

	return 0;
}

uint8_t SDUpParamDRVMode(void)
{
	//
	// Handle Mode key press
	//
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
    	gKeysStatus.bits.Key_Mode_pressed = 0;

		//
		// Reset Request and response status and acknowledgment status
		//
		gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
		gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
		gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
		gstUMtoCMdatabase.commandToControlBoard.val = 0;

//    	psActiveMenu = psActiveMenu->psParent;
		psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
//		psActiveFunctionalBlock->pfnPaintFirstScreen();
		menuPaintFirstScreen();
    }

	return 0;
}

uint8_t SDUpParamDRVEnter(void)
{
	//
	// Handle Enter key press
	//
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;
		sdcardupload_parameter_cyw  = Disp_sdcardupload_parameter_cyw;

		if(sdcardupload_parameter_cyw==1)

		{

		GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);



		if(gu8_language == Japanese_IDX)
		{
			displayText("SDカードへコピー", 2, 0, false, false, false, false, false, false);
		displayText("カンリョウ", 2, 16, false, false, false, false,false,false);
		displayText("オマチクダサイ...", 2, 48, false, false, false, false, false, false);
		}
		else
		{
		displayText("UPLOADING", 2, 0, false, false, false, false, false, true);
		displayText("DRIVE PARAMETERS", 2, 16, false, false, false, false,false,true);
		displayText("FROM SD CARD", 2, 32, false, false, false, false, false, true);
		displayText("PLEASE WAIT...", 2, 48, false, false, false, false, false, true);
		}
		if((gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) && (gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS) && !UploadParamFlags.flags.paramULEnterpressed)
		{
			UploadParamFlags.flags.paramULEnterpressed = 1; //May be remove later. Please check for references before removing
			UploadParamFlags.flags.paramDriveupload = 1; //First get Drive parameters
			gstUMtoCMdatabase.commandToControlBoard.val = 0;
//			CALCULATE (uiFocusIndex - 1)
//			gcULParamfilename
//			gSDCardUPParamDRVbrdMenuItems[i+1].pcText = uploadParamFilenames[i];
			usnprintf((char *)gcULParamfilename, sizeof(gcULParamfilename), "%s\0", uploadParamFilenames[gSDCardUPParamDRVbrdMenu.ui8FocusIndex - 1]);

		}
		else
		{
			displayText("               ", 2, 48, false, false, false, false, false, false);   //20161207
			/*if(gu8_language == Japanese_IDX)
			displayText("コマンドソ ウシンエラー", 2, 48, false, false, false, false,false,false);
			else
			displayText("ERROR SENDING CMD", 2, 48, false, false, false, false,false,true);*/
		}
		}
		else
		{
			psActiveFunctionalBlock = &gsMenuFunctionalBlock;
			psActiveFunctionalBlock->pfnPaintFirstScreen();
		}
	}

	//CALCULATE (uiFocusIndex - 1)
	return 0;
}

uint8_t SDUpParamCTRLEnter(void)
{
	//
	// Handle Enter key press
	//
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;
		sdcardupload_parameter_cyw  = Disp_sdcardupload_parameter_cyw;
		if(sdcardupload_parameter_cyw==1)

		{
		GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);


		if(gu8_language == Japanese_IDX)
		{
		displayText("SDカードへコピー", 2, 0, false, false, false, false, false, false);
		displayText("カンリョウ", 2, 16, false, false, false, false, false, false);
		//
		displayText("オマチクダサイ...", 2, 48, false, false, false, false, false, false);
		}
		else
		{
		displayText("UPLOADING CTRL", 2, 0, false, false, false, false, false, true);
		displayText("PARAMETERS", 2, 16, false, false, false, false, false, true);
		displayText("FROM SD CARD", 2, 32, false, false, false, false, false, true);
		displayText("PLEASE WAIT...", 2, 48, false, false, false, false,false,true);
		}
		if((gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) && (gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS) && !UploadParamFlags.flags.paramULEnterpressed)
		{
			UploadParamFlags.flags.paramULEnterpressed = 1; //May be remove later. Please check for references before removing
			UploadParamFlags.flags.paramControlupload = 1; //Set Control parameters
			gstUMtoCMdatabase.commandToControlBoard.val = 0;
//			CALCULATE (uiFocusIndex - 1)
//			gcULParamfilename
//			gSDCardUPParamDRVbrdMenuItems[i+1].pcText = uploadParamFilenames[i];
			usnprintf((char *)gcULParamfilename, sizeof(gcULParamfilename), "%s\0", uploadParamFilenames[gSDCardUPParamCTRLbrdMenu.ui8FocusIndex - 1]);

		}
		else
		{
			displayText("               ", 2, 48, false, false, false, false, false, false);   //20161207
			/*if(gu8_language == Japanese_IDX)
			displayText("コマンドソ ウシンエラー", 2, 48, false, false, false, false, false, false);
			else
			displayText("ERROR SENDING CMD", 2, 48, false, false, false, false, false, true);*/
		}
		}
		else
		{
			psActiveFunctionalBlock = &gsMenuFunctionalBlock;
		    psActiveFunctionalBlock->pfnPaintFirstScreen();
		}
	}

	//CALCULATE (uiFocusIndex - 1)
	return 0;
}

uint8_t SDUpParamDRVUp(void)
{
	//
	// Handle Enter key press
	//
	if(gKeysStatus.bits.Key_Up_pressed)
	{
		gKeysStatus.bits.Key_Up_pressed = 0;
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
		if(gu8_language == Japanese_IDX)
			{
			displayText("パラメータヲSDカードニ", 2, 0, false, false, false, false,false,false);
			displayText("コピーシマスカ?", 2, 16, false, false, false, false,false,false);
			}
			else
			{
			displayText("ARE YOU SURE TO", 2, 0, false, false, false, false,false,true);
			displayText("UPLOAD PARAMETERS?", 2, 16, false, false, false, false,false,true);
			}
								if(Disp_sdcardupload_parameter_cyw == 0)
								{
									          Disp_sdcardupload_parameter_cyw = 1;
												displayText("YES", 2, 32, true, false, false, true,false,false);
												displayText("NO", 2, 48, false, false, false, false,false,false);
								}
								else
								{
									            Disp_sdcardupload_parameter_cyw = 0;
												displayText("YES", 2, 32, false, false, false, false,false,false);
												displayText("NO", 2, 48, true, false, false, true,false,false);
								}
	}

	return 0;
}

uint8_t SDUpParamDRVDown(void)
{
	//
	// Handle Enter key press
	//
	if(gKeysStatus.bits.Key_Down_pressed)
	{
		gKeysStatus.bits.Key_Down_pressed = 0;
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
		if(gu8_language == Japanese_IDX)
			{
			displayText("パラメータヲSDカードニ", 2, 0, false, false, false, false,false,false);
			displayText("コピーシマスカ?", 2, 16, false, false, false, false,false,false);
			}
			else
			{
			displayText("ARE YOU SURE TO", 2, 0, false, false, false, false,false,true);
			displayText("UPLOAD PARAMETERS?", 2, 16, false, false, false, false,false,true);
			}
										if(Disp_sdcardupload_parameter_cyw == 0)
										{
											          Disp_sdcardupload_parameter_cyw = 1;
														displayText("YES", 2, 32, true, false, false, true,false,false);
														displayText("NO", 2, 48, false, false, false, false,false,false);
										}
										else
										{
											            Disp_sdcardupload_parameter_cyw = 0;
														displayText("YES", 2, 32, false, false, false, false,false,false);
														displayText("NO", 2, 48, true, false, false, true,false,false);
										}
	}

	return 0;
}
/******************************************************************************
 * FunctionName: uploadParametersMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t uploadParametersMode(void)
{
	//
	// This function is called periodically
	//
//	menuMode();
	//
	// Handle Mode key press
	//
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
    	gKeysStatus.bits.Key_Mode_pressed = 0;

    	psActiveMenu = psActiveMenu->psParent;
		psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
		psActiveFunctionalBlock->pfnPaintFirstScreen();
    }

	return 0;
}

/******************************************************************************
 * FunctionName: uploadParametersEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t uploadParametersEnter(void)
{
	//
	// This function is called periodically
	//
	menuEnter();
	//
	// Handle Enter key press
	//
//	if(gKeysStatus.bits.Key_Enter_pressed)
//	{
//		gKeysStatus.bits.Key_Enter_pressed = 0;
//
//	}

	return 0;
}

/******************************************************************************
 * FunctionName: uploadParametersUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t uploadParametersUp(void)
{
	//
	// This function is called periodically
	//
	menuUp();
	//
	// Handle Up key press
	//
//	if(gKeysStatus.bits.Key_Up_pressed)
//	{
//		gKeysStatus.bits.Key_Up_pressed = 0;
//
//	}

	return 0;
}

/******************************************************************************
 * FunctionName: uploadParametersDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t uploadParametersDown(void)
{
	//
	// This function is called periodically
	//

	menuDown();
	//
	// Handle Down key press
	//
//	if(gKeysStatus.bits.Key_Down_pressed)
//	{
//		gKeysStatus.bits.Key_Down_pressed = 0;
//
//	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsUploadDrvParametersFunctionalBlock =
{
	&gsMenuFunctionalBlock,
	0,
	uploadDrvParametersPaint,
	uploadParametersRunTime,
	uploadParametersUp,
	uploadParametersDown,
	uploadParametersMode,
	uploadParametersEnter
};

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsUploadCtrlParametersFunctionalBlock =
{
	&gsMenuFunctionalBlock,
	0,
	uploadCtrlParametersPaint,
	uploadParametersRunTime,
	uploadParametersUp,
	uploadParametersDown,
	uploadParametersMode,
	uploadParametersEnter
};

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsSDUpParamDRVFunctionalBlock =
{
	&gsUploadDrvParametersFunctionalBlock,
	//&gsMenuFunctionalBlock,
	0,
	SDUpParamDRVPaint,
	SDUpParamDRVCTRLRunTime,
	SDUpParamDRVUp,
	SDUpParamDRVDown,
	SDUpParamDRVMode,
	SDUpParamDRVEnter
};

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsSDUpParamCTRLFunctionalBlock =
{
	&gsUploadCtrlParametersFunctionalBlock,
	0,
	SDUpParamDRVPaint,
	SDUpParamDRVCTRLRunTime,
	SDUpParamDRVUp,
	SDUpParamDRVDown,
	SDUpParamDRVMode,
	SDUpParamCTRLEnter
};

uint32_t strtoint(char a[])
{
  uint32_t c, offset = 0, n = 0;

  for (c = offset; a[c] != '\0'; c++) {
    n = n * 10 + a[c] - '0';
  }

  return n;
}
