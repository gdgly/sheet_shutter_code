/*********************************************************************************
 * FileName: sdcard_downloadparameters.c
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
 *  	0.2D	11/08/2014									Patch for DriveCount and other flags reset if read-only
 *  														UMtoCMDatabase block resets added in NACK, TIMEOUT and SUCCESS case
 *  														In case of NACK and TIMEOUT, DeleteFile functionality added.
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
#include "Middleware/paramdatabase.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "userinterface.h"
#include "intertaskcommunication.h"
#include "parameterlist.h"
#include "Middleware/sdcard.h"
#include "Middleware/rtc.h"
/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
// Data Structure for Flags for Internal modules
typedef union
{
	uint8_t val;
	struct
	{
		//paramDriveupload module flag

		uint8_t paramDLEnterpressed		 : 1;
		uint8_t paramDrivedownload		 : 1;
		uint8_t paramControldownload	 : 1;
		uint8_t paramDrivedownloadDone	 : 1;
		uint8_t paramControldownloadDone : 1;
		uint8_t bit5			: 1;
		uint8_t bit6			: 1;
		uint8_t bit7			: 1;
	} flags;
} _DownloadParamFlags;

// Define Flags for Internal modules
_DownloadParamFlags DownloadParamFlags;

/*static */uint8_t gui8DriveParamCount; //Max Drive param count is number_of_drive_parameters - Also used in Upload file
/*static */uint8_t gui8ControlParamCount; //Max Control param count is _NO_OF_CTRL_PARAMS - Also used in Upload file
/*static */uint8_t gui8ParamCount; //Max ??
char gcDLParamfilename[35];
unsigned char gucParamDLFileWriteBuff[MAX_CHARS_IN_LINE];
struct tm gstcurrentDateTime;

uint8_t sdcarddownload_parameter_cyw=0;
uint8_t Disp_sdcarddownload_parameter_cyw=0;
extern uint16_t gu16_lcdlight;
extern uint8_t gu8_language;
extern unsigned char gu8_gesture_manual;
extern unsigned char menu_gesture_flag_cyw;


const uint16_t  PARAM_ALL_SDCARD_CYW[param_all_list_long]={0,1,2,3,4,5,6,7,9,10,11,16,20,26,27,28,40,41,42,61,62,63,71,72,
		                          103,104,105,106,107,108,110,120,131,
								  200,

								  500,501,504,505,506,507,508,509,510,511,512,513,514,515,516,517,518,519,520,521,522,523,524,525,526,527,528,529,530,531,636,537,538,539,540,541,542,543,544,545,546,547,548,551,
								  600,601,602,605,606,607,608,609,610,611,612,613,614,615,638,
								  801,802,805,806};
const dest_enum_cyw PARAM_ALL_BELONG_CYW[param_all_list_long]=
                               {eDestControlBoard,//0
								eDestControlBoard,//1
								eDestControlBoard,//2
								eDestControlBoard,//3
								eDestControlBoard,//4
								eDestControlBoard,//5
								eDestControlBoard,//6
								eDestControlBoard,//7
								eDestControlBoard,//9
								eDestControlBoard,//10
								eDestDriveBoard,//11
								eDestControlBoard,//16
								eDestControlBoard,//20
								eDestDisplayBoard,//26
								eDestControlBoard,//27
								eDestControlBoard,//28
								eDestDisplayBoard,//40
								eDestDisplayBoard,//41
								eDestDisplayBoard,//42
								eDestControlBoard,//61
								eDestControlBoard,//62
								eDestControlBoard,//63
								eDestControlBoard,//71
								eDestControlBoard,//72
								eDestDriveBoard,//103
								eDestDriveBoard,//104
								eDestDriveBoard,//105
								eDestDriveBoard,//106
								eDestDriveBoard,//107
								eDestDriveBoard,//108
								eDestDriveBoard,//110
								eDestControlBoard,//120
								eDestControlBoard,//131
								eDestControlBoard,//200

								eDestDriveBoard,//500
								eDestDriveBoard,//501
								eDestDriveBoard,//504
								eDestDriveBoard,//505
								eDestDriveBoard,//506
								eDestDriveBoard,//507
								eDestDriveBoard,//508
								eDestDriveBoard,//509
								eDestDriveBoard,//510
								eDestDriveBoard,//511
								eDestDriveBoard,//512
								eDestDriveBoard,//513
								eDestDriveBoard,//514
								eDestDriveBoard,//515
								eDestDriveBoard,//516
								eDestDriveBoard,//517
								eDestDriveBoard,//518
								eDestDriveBoard,//519
								eDestDriveBoard,//520
								eDestDriveBoard,//521
								eDestDriveBoard,//522
								eDestDriveBoard,//523
								eDestDriveBoard,//524
								eDestDriveBoard,//525
								eDestDriveBoard,//526
								eDestDriveBoard,//527
								eDestDriveBoard,//528
								eDestDriveBoard,//529
								eDestDriveBoard,//530
								eDestDriveBoard,//531
								eDestDriveBoard,//536
								eDestDriveBoard,//537
								eDestDriveBoard,//538
								eDestDriveBoard,//539
								eDestDriveBoard,//540
								eDestDriveBoard,//541
								eDestDriveBoard,//542
								eDestDriveBoard,//543
								eDestDriveBoard,//544
								eDestDriveBoard,//545
								eDestDriveBoard,//546
								eDestDriveBoard,//547
								eDestDriveBoard,//548
								eDestDriveBoard,//551
								eDestControlBoard,//600
								eDestControlBoard,//601
								eDestControlBoard,//602
								eDestControlBoard,//605
								eDestControlBoard,//606
								eDestControlBoard,//607
								eDestControlBoard,//608
								eDestControlBoard,//609
								eDestControlBoard,//610
								eDestControlBoard,//611
								eDestControlBoard,//612
								eDestControlBoard,//613
								eDestControlBoard,//614
								eDestControlBoard,//615
								eDestControlBoard,//638
								eDestControlBoard,//801
								eDestControlBoard,//802
								eDestControlBoard,//805
								eDestControlBoard,//806
                                };


/****************************************************************************/

/******************************************************************************
 * FunctionName: downloadParametersRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t downloadParametersRunTime(void)
{
	static uint8_t lsDelay3SecStart = 0;
	static uint32_t lsTickCount3Seconds = 0;
    //#define diver_list_long   19
    //#define control_list_long  22
    //const uint8_t diver_list[diver_list_long]  ={29,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,58,59,60};
    //const uint8_t control_list[control_list_long]={17,18,19,20,21,22,23,24,25,26,27,28,33,34,35,36,37,38,39,40,41,57};
	unsigned char Tp_gucParamDLFileWriteBuff[50]={0};

	unsigned int retbytes;
	unsigned char writeResult;

	if(DownloadParamFlags.flags.paramDLEnterpressed)
	{
		if((gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) && (gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS))
		{
			//Set details for GET PARAMETER here
			if(DownloadParamFlags.flags.paramDrivedownload/* && !DownloadParamFlags.flags.paramControldownload*/)
			{
				if(!gui8DriveParamCount)
				{
					// Generate filename
					RTCGet(&gstcurrentDateTime);
					memset(gcDLParamfilename, 0, sizeof(gcDLParamfilename));
					ustrncpy(gcDLParamfilename, "Par_\0", 9);
					usnprintf(gcDLParamfilename, sizeof(gcDLParamfilename), "%s%04u%02u%02u%02u%02u.txt\0", gcDLParamfilename,gstcurrentDateTime.tm_year + 1900, gstcurrentDateTime.tm_mon + 1, gstcurrentDateTime.tm_mday,  gstcurrentDateTime.tm_hour, gstcurrentDateTime.tm_min);

					// Create File and add Header for file
					memset(gucParamDLFileWriteBuff, 0, sizeof(gucParamDLFileWriteBuff));
					usnprintf((char *)Tp_gucParamDLFileWriteBuff, sizeof(Tp_gucParamDLFileWriteBuff), "*ALL PARAMS - %04u-%02u-%02u %02u:%02u:%02u\r\n\r\n",gstcurrentDateTime.tm_year + 1900,  gstcurrentDateTime.tm_mon + 1, gstcurrentDateTime.tm_mday,   gstcurrentDateTime.tm_hour, gstcurrentDateTime.tm_min, gstcurrentDateTime.tm_sec);
					writeResult = write_SDCard(gcDLParamfilename, Tp_gucParamDLFileWriteBuff, &retbytes,FILE_OVERWRITE);

					if(writeResult)
					{
						DownloadParamFlags.val = 0;
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
						{
						displayText("SD�J�[�h�t�����E", 2, 0, false, false, false, false, false, false);
						}
						else
						{
						displayText("SDCARD FAILURE", 2, 0, false, false, false, false, false, true);
						}
						return 0;
					}

					// Fill Details
					gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 1;

				}//!gui8DriveParamCount

				if(gui8DriveParamCount<param_all_list_long)
				{
					if(PARAM_ALL_BELONG_CYW[gui8DriveParamCount]==eDestDisplayBoard)
					{
					gstUMtoCMdatabase.dataToControlBoard.parameterNumber=PARAM_ALL_SDCARD_CYW[gui8DriveParamCount];
					if(PARAM_ALL_SDCARD_CYW[gui8DriveParamCount]==26)
						gstUMtoCMdatabase.getParameterValue = gu16_lcdlight;
					if(PARAM_ALL_SDCARD_CYW[gui8DriveParamCount]==40)
						gstUMtoCMdatabase.getParameterValue = gu8_gesture_manual;	
					if(PARAM_ALL_SDCARD_CYW[gui8DriveParamCount]==41)
						gstUMtoCMdatabase.getParameterValue = menu_gesture_flag_cyw;
					if(PARAM_ALL_SDCARD_CYW[gui8DriveParamCount]==42)
						gstUMtoCMdatabase.getParameterValue = gu8_language;
					gui8DriveParamCount++;
					goto savesdcard;
					}
					else
					{
					gstUMtoCMdatabase.destination = PARAM_ALL_BELONG_CYW[gui8DriveParamCount];
					gstUMtoCMdatabase.dataToControlBoard.parameterNumber = PARAM_ALL_SDCARD_CYW[gui8DriveParamCount];
					gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
					gstUMtoCMdatabase.getParameterValue = 0;
					gui8DriveParamCount++;
					}


				}
				else
				{
					// Also take this decision in SUCCESS section
					gui8DriveParamCount = 0;
					DownloadParamFlags.flags.paramDrivedownload = 0;
					DownloadParamFlags.flags.paramDrivedownloadDone = 1;


				}
			}//DownloadParamFlags.flags.paramDrivedownload
			else if(DownloadParamFlags.flags.paramDrivedownloadDone)
			{
				// All Control and Drive parameters are downloaded
				// Initiate Closure
				DownloadParamFlags.val = 0;
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
					//displayText("DOWNLOAD", 2, 16, false, false, false, false, false, false);

					//
					// Paint error reason on fourth line
					//
					//displayText("SUCCESSFUL", 2, 48, false, false, false, false, false, false);
					if(gu8_language == Japanese_IDX)
					{
					displayText("SD�J�[�h�@�R�s�[", 2, 0, false, false, false, false, false, false);
					displayText("OK", 2, 16, false, false, false, false, false, false);
					}
					else
					{
					displayText("DOWNLOAD", 2, 16, false, false, false, false, false, true);
					displayText("SUCCESSFUL", 2, 48, false, false, false, false, false, true);
					}
					//
					// Capture Time
					//
					lsTickCount3Seconds = g_ui32TickCount;

					//
					// Set delay start flag
					//
					lsDelay3SecStart = 1;
				}
			}//(DownloadParamFlags.flags.paramControldownloadDone && DownloadParamFlags.flags.paramDrivedownloadDone)
		}//(gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) && (gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS)
		else if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		{
			if(eNO_STATUS == gstUMtoCMdatabase.commandResponseStatus)
			{

			}
			else if(eSUCCESS == gstUMtoCMdatabase.commandResponseStatus)
			{
				if(eACK == gstUMtoCMdatabase.acknowledgementReceived)
				{
savesdcard:				memset(gucParamDLFileWriteBuff, 0, sizeof(gucParamDLFileWriteBuff));
					usnprintf((char *)gucParamDLFileWriteBuff, sizeof(gucParamDLFileWriteBuff), "%u,%u\r\n", gstUMtoCMdatabase.dataToControlBoard.parameterNumber, gstUMtoCMdatabase.getParameterValue);
					writeResult = write_SDCard(gcDLParamfilename, gucParamDLFileWriteBuff, &retbytes,FILE_APPEND_WRITE);

					if(writeResult)
					{
						DownloadParamFlags.val = 0;
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
						displayText("SD�J�[�h�t�����E", 2, 0, false, false, false, false, false, false);
						else
						displayText("SDCARD FAILURE", 2, 0, false, false, false, false, false, true);
						return 0;
					}

					if((DownloadParamFlags.flags.paramDrivedownload) &&
					  (gui8DriveParamCount >= param_all_list_long))
					{
						gui8DriveParamCount = 0;
						DownloadParamFlags.flags.paramDrivedownload = 0;
						DownloadParamFlags.flags.paramDrivedownloadDone = 1;

					}

					//
					// Reset Request and response status and acknowledgment status
					//
					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
					gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
				}
				else if(eNACK == gstUMtoCMdatabase.acknowledgementReceived)
				{
					//
					// Reset command bit
					//
					gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 0;

					DownloadParamFlags.val = 0;
					//Reset Added in Auto-GoBack Delay Start section
					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
					gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
					gstUMtoCMdatabase.commandToControlBoard.val = 0;

					//Delete file
					delete_SDCard(gcDLParamfilename);

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
						//displayText("DOWNLOAD", 2, 0, false, false, false, false, false, false);
						//displayText("FAILED", 2, 16, false, false, false, false, false, false);
						if(gu8_language == Japanese_IDX)
						{
						displayText("�J�L�R�~NG", 2, 0, false, false, false, false, false, false);
						}
						else
						{
						displayText("DOWNLOAD", 2, 0, false, false, false, false, false, true);
						displayText("FAILED", 2, 16, false, false, false, false, false, true);
						}
						//if(DownloadParamFlags.flags.paramDrivedownload)
						//{
						//	if(gu8_language == Japanese_IDX)
						//	{
						//	    displayText((unsigned char *)gsParamDatabase[diver_list[gui8DriveParamCount]].paramName_japanese, 2, 32, false, false, false, false, false, false);
						//	}
						//	else
						//	{
						//		displayText((unsigned char *)gsParamDatabase[diver_list[gui8DriveParamCount]].paramName_english, 2, 32, false, false, false, false, false, false);
						//	}
						//}

						//displayText("NACK ERROR", 2, 48, false, false, false, false, false, false);
						if(gu8_language == Japanese_IDX)
						displayText("NACK�G���[", 2, 48, false, false, false, false, false, false);
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

				DownloadParamFlags.val = 0;
				//Reset Added in Auto-GoBack Delay Start section
				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
				gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
				gstUMtoCMdatabase.commandToControlBoard.val = 0;

				//Delete file
				delete_SDCard(gcDLParamfilename);

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
					//displayText("DOWNLOAD", 2, 0, false, false, false, false, false, false);
					//displayText("FAILED", 2, 16, false, false, false, false, false, false);
					if(gu8_language == Japanese_IDX)
					{
					displayText("�J�L�R�~NG", 2, 0, false, false, false, false, false, false);
					}
					else
					{
					displayText("DOWNLOAD", 2, 0, false, false, false, false, false, true);
					displayText("FAILED", 2, 16, false, false, false, false, false, true);
					}

					//if(DownloadParamFlags.flags.paramDrivedownload)
					//{
					//	if(gu8_language == Japanese_IDX)
					//	displayText((unsigned char *)gsParamDatabase[diver_list[gui8DriveParamCount]].paramName_japanese, 2, 32, false, false, false, false, false, false);
					//	else
					//	displayText((unsigned char *)gsParamDatabase[diver_list[gui8DriveParamCount]].paramName_english, 2, 32, false, false, false, false, false, false);

					//}
					//else if(DownloadParamFlags.flags.paramControldownload)
					//{
					//	if(gu8_language == Japanese_IDX)
					//	displayText((unsigned char *)gsParamDatabase[control_list[gui8ControlParamCount]].paramName_japanese, 2, 32, false, false, false, false, false, false);
					//	else
					//	displayText((unsigned char *)gsParamDatabase[control_list[gui8ControlParamCount]].paramName_english, 2, 32, false, false, false, false, false, false);

					//}

					if(gu8_language == Japanese_IDX)
					{
					//displayText("COMM ERROR", 2, 48, false, false, false, false, false, false);
					displayText("�c�E�V���G���[", 2, 48, false, false, false, false, false, false);
					}
					else
					{
					displayText("COMM ERROR", 2, 48, false, false, false, false, false, false);
					}
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
		}//(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		else //Check also if all Parameters are fetched //if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		{
			displayText("               ", 2, 48, false, false, false, false, false, false);   //20161207
			/*if(gu8_language == Japanese_IDX)
			//
			displayText("�R�}���h�\ �E�V���G���[", 2, 48, false, false, false, false, false, false);
			else
			displayText("ERR SENDING CMD", 2, 48, false, false, false, false, false, true);*/

			//Take necessary actions here
		}
	}//DownloadParamFlags.flags.paramDLEnterpressed
	else
		; //Handle else part of (DownloadParamFlags.flags.paramDLEnterpressed)


	// Check whether delay is achieved
	if((get_timego( lsTickCount3Seconds) > 300) && (1 == lsDelay3SecStart))
	{
		//
		// Return to parameter list functional block.
		//
		psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
		psActiveFunctionalBlock->pfnPaintFirstScreen();

		//
		// Reset delay start flag
		//
		lsDelay3SecStart = 0;
	}

	updateFaultLEDStatus();

	return 0;
}
/******************************************************************************
 * FunctionName: downloadParametersPaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t downloadParametersPaint(void)
{
	//
	// This is first screen paint function
	//
	DownloadParamFlags.val = 0;
	gui8DriveParamCount = 0;
	gui8ControlParamCount = 0;

	GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);
	//displayText("DO YOU WANT TO", 2, 0, false, false, false, false, false, false);
	//displayText("DOWNLOAD SHUTTER AND", 2, 16, false, false, false, false,false,true);
	//displayText("DRIVE PARAMETERS?", 2, 32, false, false, false, false,false,true);
	if(gu8_language == Japanese_IDX)
	{
	displayText("SD�J�[�h", 2, 0, false, false, false, false, false, false);
	displayText("�p�����[�^�Z�[�u�V�}�X�J?", 2, 16, false, false, false, false,false,false);
	}
	else
	{
	displayText("DO YOU WANT TO", 2, 0, false, false, false, false, false, true);
	displayText("DOWNLOAD PARAMETERS?", 2, 16, false, false, false, false,false,true);
			//displayText("DRIVE PARAMETERS?", 2, 32, false, false, false, false,false,true);
	}
	//displayText("Y:<ENT>, N:<MOD>", 1, 48, false, false, false, false,true, false);
	displayText("YES", 2, 32, false, false, false, false,false,false);
    displayText("NO", 2, 48, true, false, false, true,false,false);

	sdcarddownload_parameter_cyw=0;
	Disp_sdcarddownload_parameter_cyw=0;
	return 0;
}

/******************************************************************************
 * FunctionName: downloadParametersMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t downloadParametersMode(void)
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

		//
		// Reset Request and response status and acknowledgment status
		//
		gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
		gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
		gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
		gstUMtoCMdatabase.commandToControlBoard.val = 0;
    	//
    	// Write mode key functionality here
    	//
		psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
		psActiveFunctionalBlock->pfnPaintFirstScreen();
    }

	return 0;
}

/******************************************************************************
 * FunctionName: downloadParametersEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t downloadParametersEnter(void)
{
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;


		sdcarddownload_parameter_cyw  = Disp_sdcarddownload_parameter_cyw;

		if(sdcarddownload_parameter_cyw==1)

		{
		//
		// Write enter key functionality here
		//
		GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);

		//displayText("DOWNLOADING SHUTTER", 2, 0, false, false, false, false,false,true);
		//displayText("AND DRIVE PARAMETERS", 2, 16, false, false, false, false,false,true);
		//displayText("TO SD CARD", 2, 32, false, false, false, false, false, false);
		if(gu8_language == Japanese_IDX)
		{
		displayText("SD�J�[�h�փp�����[�^�m", 2, 0, false, false, false, false,false,false);
		displayText("�R�s�[���V�}�X", 2, 16, false, false, false, false,false,false);
		displayText("�I�}�`�N�_�T�C...", 2, 48, false, false, false, false, false, false);
		}
		else
		{
		displayText("DOWNLOADING SHUTTER", 2, 0, false, false, false, false,false,true);
		displayText("AND DRIVE PARAMETERS", 2, 16, false, false, false, false,false,true);
		displayText("TO SD CARD", 2, 32, false, false, false, false, false, true);
		}
		if((gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) && (gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS) && !DownloadParamFlags.flags.paramDLEnterpressed)
		{

			DownloadParamFlags.flags.paramDLEnterpressed = 1; //May be remove later. Please check for references before removing
			DownloadParamFlags.flags.paramDrivedownload = 1; //First get Drive parameters


		}
		else
		{
			displayText("               ", 2, 48, false, false, false, false, false, false);   //20161207
			/*if(gu8_language == Japanese_IDX)
			displayText("�R�}���h�\ �E�V���G���[", 2, 48, false, false, false, false,false,false);
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

	return 0;
}

/******************************************************************************
 * FunctionName: downloadParametersUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t downloadParametersUp(void)
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
			displayText("SD�J�[�h", 2, 0, false, false, false, false, false, false);
			displayText("�p�����[�^�Z�[�u�V�}�X�J?", 2, 16, false, false, false, false,false,false);
			}
			else
			{
			displayText("DO YOU WANT TO", 2, 0, false, false, false, false, false, true);
			displayText("DOWNLOAD PARAMETERS?", 2, 16, false, false, false, false,false,true);
					//displayText("DRIVE PARAMETERS?", 2, 32, false, false, false, false,false,true);
			}
			if(Disp_sdcarddownload_parameter_cyw == 0)
						{
							          Disp_sdcarddownload_parameter_cyw = 1;
										displayText("YES", 2, 32, true, false, false, true,false,false);
										displayText("NO", 2, 48, false, false, false, false,false,false);
						}
						else
						{
							            Disp_sdcarddownload_parameter_cyw = 0;
										displayText("YES", 2, 32, false, false, false, false,false,false);
										displayText("NO", 2, 48, true, false, false, true,false,false);
						}
		//
		// Write up key functionality here
		//
	}

	return 0;
}

/******************************************************************************
 * FunctionName: downloadParametersDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t downloadParametersDown(void)
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
			displayText("SD�J�[�h", 2, 0, false, false, false, false, false, false);
			displayText("�p�����[�^�Z�[�u�V�}�X�J?", 2, 16, false, false, false, false,false,false);
			}
			else
			{
			displayText("DO YOU WANT TO", 2, 0, false, false, false, false, false, true);
			displayText("DOWNLOAD PARAMETERS?", 2, 16, false, false, false, false,false,true);
					//displayText("DRIVE PARAMETERS?", 2, 32, false, false, false, false,false,true);
			}
								if(Disp_sdcarddownload_parameter_cyw == 0)
								{
									          Disp_sdcarddownload_parameter_cyw = 1;
												displayText("YES", 2, 32, true, false, false, true,false,false);
												displayText("NO", 2, 48, false, false, false, false,false,false);
								}
								else
								{
									            Disp_sdcarddownload_parameter_cyw = 0;
												displayText("YES", 2, 32, false, false, false, false,false,false);
												displayText("NO", 2, 48, true, false, false, true,false,false);
								}
		//
		// Write Down key functionality here
		//
	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsDownloadParametersFunctionalBlock =
{
	&gsMenuFunctionalBlock,
	0,
	downloadParametersPaint,
	downloadParametersRunTime,
	downloadParametersUp,
	downloadParametersDown,
	downloadParametersMode,
	downloadParametersEnter
};
