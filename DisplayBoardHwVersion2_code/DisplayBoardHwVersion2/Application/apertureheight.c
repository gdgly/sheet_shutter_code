
/*********************************************************************************
 * FileName: apertureheight.c
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
 *  	0.1D	03/06/2014      	iGATE Offshore team       Initial Creation
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
#include "Middleware/serial.h"
#include "userinterface.h"
#include "intertaskcommunication.h"
#include "communicationmodule.h"
#include "ledhandler.h"
#include "Drivers/systicktimer.h"
#include "parameterlist.h"
#include "errormodule.h"
#include "Middleware/paramdatabase.h"

#define A_CHECK_SHUTTER_STATE						0
#define A_START_APERTURE_HEIGHT						1
#define A_WAIT_FOR_START_APERTURE_HEIGHT_REPLAY		2
#define A_WAIT_FOR_START_APERTURE_HEIGHT            3
#define A_KEY_PRESS_HANDLE							4
#define A_WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE		5
#define A_DISPLAY_INSTALLATION_STATE				6
#define A_GET_CURRENT_COUNT							7
#define A_WAIT_FOR_GET_CURRENT_COUNT_RESPONSE		8
#define A_VALIDATING_APERTURE_HEIGHT				9
#define A_CHECK_FOR_APERTURE_HEIGHT_SUCCESS			10
#define A_APERTURE_HEIGHT_FAILED					11
#define A_WAIT_FOR_COMMIT_APERTURE_HEIGHT			12
#define A_DELAY_3_SECONDS							13
#define A_OUT_APERTURE_HEIGHT						14
#define A_STATE_UNDEFINED                           15

#define SERVICED		0
#define ACTIVATED		1
#define DEACTIVATED		2

extern uint8_t setting_flag;
uint32_t APERT_TIME;
static uint8_t A_lsDelay500msGetCountStart_cyw=0;

uint8_t gApertureheightState = A_CHECK_SHUTTER_STATE;
uint8_t gNextApertureheightState = 0xFF;

extern struct errorDB gsActiveAnomalyList[20];
//uint32_t A_gTickCount3Seconds_cyw=0;

uint8_t A_gui8UpState = 0;
uint8_t A_gui8DownState = 0;
uint8_t A_gui8EnterState = 0;
uint8_t A_gui8OpenState = 0;
uint8_t A_gui8CloseState = 0;
uint8_t A_gui8ModeState=0;

uint32_t wait_for_cleardrivedault_time = 0;

extern uint8_t gValueParamGetStarted ;

uint32_t A_gCurrentEncoderCount = 0;

uint32_t A_gTickCount3Seconds_cyw=0;

extern uint32_t gTickCount3Seconds;
uint32_t A_wait_for_current_time=0;
void guest_reinit(void);
/******************************************************************************
 * FunctionName: apertureheightPaint
 *
 * Function Description:
 * It renders information regarding first stage of installation. This function should
 * be called when switching to installation functional block.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success.
 *
 ********************************************************************************/
uint8_t apertureheightPaint(void)
{
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		//displayText("A100", 2, 0, false, false, false, false);
		//displayText("UPPER LIMIT", 2, 16, false, false, false, false);

		if(gu8_language == Japanese_IDX)
		{
		displayText("タイキチュウ", 2, 0, false, false, false, false, false, false);
		displayText("ハンカイ_セッテイ...", 1, 16, false, false, false, false, true, false);
		}
		else
		{
	    displayText("WAITING FOR", 2, 0, false, false, false, false, false, true);
		displayText("SET APERHEIGHT...", 2, 16, false, false, false, false, false, true);
		}
		//
		// Reset display LEDs state
		//
		gstLEDcontrolRegister.autoManualLED = 0;
		gstLEDcontrolRegister.faultLED = 0;
		gstLEDcontrolRegister.powerLED = 0;

		//
		// Display current encoder count in 3rd line
		//
	//	memset(lBuff, 0, 5);
	//	usnprintf((char*)lBuff, sizeof(lBuff), "%04d", gCurrentEncoderCount);
	//	displayText(lBuff, 2, 32, false, false, false, false);
		setting_flag = 1;

		APERT_TIME = g_ui32TickCount;

		if(gstControlBoardStatus.bits.s3PBS_stoppressd == 1)
		{
			if(gu8_language == Japanese_IDX)
			{
				displayText("テイシ ON", 2, 48, false, false, false, false, false, false);
			}
			else
			{
				displayText("STOP ON", 2, 48, false, false, false, false, false, true);
			}

		}
		else
		{
			displayText("         ", 2, 48, false, false, false, false, false, false);
		}
	//	lsDelay500msGetCountStart_cyw = 1;
		return 0;
}

/******************************************************************************
 * FunctionName: apertureheightRunTime
 *
 * Function Description:
 * Handles installation runtime activities. This function handles below operations
 * 1. Initiate installation command.
 * 2. Handles operational key press events during installation process.
 * 3. Display information related to current state of installation.
 * 4. Initiate command to fetch current encoder count from control board.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t apertureheightRunTime(void)
{
	static uint8_t lsDelay500msStart = 0;
	static bool bClearScreen = true;
	static uint32_t lsTickCount500Milliseconds = 0;
	unsigned char lBuff[21];
	  static uint8_t error_occ=0,error_occ1=0;
	  static uint8_t lsCommitInstallation = 0;
	  static uint8_t lsDelay3secStart = 0;

	switch(gApertureheightState)
	{
	case A_CHECK_SHUTTER_STATE:

		if(gstDriveBoardStatus.bits.driveApertureheight == 1)
		{
			gApertureheightState = A_KEY_PRESS_HANDLE;
		}
		else
		{
			//
			// Change state
			//
			gApertureheightState = A_START_APERTURE_HEIGHT;
			setting_flag = 1;
		}
		break;
	case A_START_APERTURE_HEIGHT:
		//
		// Initiate Installation command
		//
		if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		{
		//
		// Initiate START_INSTALLATION command
		//
		gstUMtoCMoperational.commandToControlBoard.bits.startapertureheight = 1;

		//
		// Set request status as active
		//
		gstUMtoCMoperational.commandRequestStatus = eACTIVE;


		//
		// Change state
		//
		gApertureheightState = A_WAIT_FOR_START_APERTURE_HEIGHT_REPLAY;
		}
		break;
	case A_WAIT_FOR_START_APERTURE_HEIGHT_REPLAY:
		//
		// Check Installation command response status
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
		gstUMtoCMoperational.commandToControlBoard.val = 0;



		//
	    // Capture time
		//
		gTickCount3Seconds = g_ui32TickCount;

		//
	    // Change state
		//
		gApertureheightState = A_DELAY_3_SECONDS;
		gNextApertureheightState = A_WAIT_FOR_START_APERTURE_HEIGHT;
		}
		else if( (gstUMtoCMoperational.commandResponseStatus == eTIME_OUT) ||
			 (gstUMtoCMoperational.commandResponseStatus == eFAIL))
		{


			//
			// Reset request and response status
			//
			gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
			gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
			gstUMtoCMoperational.commandToControlBoard.val = 0;


			//
			// Change state
			//
			gApertureheightState = A_DELAY_3_SECONDS;
			gNextApertureheightState = A_START_APERTURE_HEIGHT;

			//
				    // Capture time
					//
				//	gTickCount3Seconds = g_ui32TickCount;

					//
				    // Change state
					//
					//gApertureheightState = A_DELAY_3_SECONDS;
					//gNextApertureheightState = A_WAIT_FOR_START_APERTURE_HEIGHT;
		}
		}
		break;
	 case A_WAIT_FOR_START_APERTURE_HEIGHT:
	    if(1 == gstDriveBoardStatus.bits.driveApertureheight)
	    {
	    		//
	    		// Change state
	    		//
	    		gApertureheightState = A_KEY_PRESS_HANDLE;

	    		//
	    		// Clear screen
	    		//
	    		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
	    }
	    else
	    {
	    	    gApertureheightState = A_CHECK_SHUTTER_STATE;//cyw add
	    }

	    break;
	 case A_KEY_PRESS_HANDLE:
		 if(
		 					(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
		 					(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)
		 	)
		 	{
		 		//
		 		// Check whether open key is pressed
		 		//
		 				if( (gKeysStatus.bits.Key_Open_pressed) &&
		 						(0 == A_gui8CloseState) && (0 == A_gui8UpState) && (0 == A_gui8DownState) && (0 == A_gui8EnterState)
		 				)
		 				{
		 					//
		 					// Initiate open command
		 					//
		 					if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		 					{
		 						gKeysStatus.bits.Key_Open_pressed = 0;
		 						A_gui8OpenState = 1;
		 						gstUMtoCMoperational.commandToControlBoard.bits.openPressed = 1;
		 						gstUMtoCMoperational.commandRequestStatus = eACTIVE;

		 						//
		 						// Change state
		 						//
		 						gApertureheightState = A_WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE;
		 					}
		 				}

		 				else if(gKeysStatus.bits.Key_Open_released)
		 				{
		 					//
		 					// Initiate open command
		 					//
		 					if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		 					{
		 						gKeysStatus.bits.Key_Open_released = 0;
		 						A_gui8OpenState = 0;
		 						gstUMtoCMoperational.commandToControlBoard.bits.openReleased = 1;
		 						gstUMtoCMoperational.commandRequestStatus = eACTIVE;

		 						//
		 						// Change state
		 						//
		 						gApertureheightState = A_WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE;
		 					}
		 				}

		 				//
		 				// Check whether close key is pressed
		 				//
		 				else if( (gKeysStatus.bits.Key_Close_pressed) &&
		 						(0 == A_gui8OpenState) && (0 == A_gui8UpState) && (0 == A_gui8DownState) && (0 == A_gui8EnterState)
		 				)
		 				{
		 					//
		 					// Initiate close command
		 					//
		 					if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		 					{
		 						gKeysStatus.bits.Key_Close_pressed = 0;
		 						A_gui8CloseState = 1;
		 						gstUMtoCMoperational.commandToControlBoard.bits.closePressed = 1;
		 						gstUMtoCMoperational.commandRequestStatus = eACTIVE;

		 						//
		 						// Change state
		 						//
		 						gApertureheightState = A_WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE;
		 					}
		 				}

		 				else if(gKeysStatus.bits.Key_Close_released)
		 				{
		 					//
		 					// Initiate close command
		 					//
		 					if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		 					{
		 						gKeysStatus.bits.Key_Close_released = 0;
		 						A_gui8CloseState = 0;
		 						gstUMtoCMoperational.commandToControlBoard.bits.closeReleased = 1;
		 						gstUMtoCMoperational.commandRequestStatus = eACTIVE;

		 						//
		 						// Change state
		 						//
		 						gApertureheightState = A_WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE;
		 					}
		 				}

		 				else if(gKeysStatus.bits.Key_Stop_pressed)
		 				{
		 					//
		 					// Initiate stop command
		 					//
		 					if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		 					{
		 						gKeysStatus.bits.Key_Stop_pressed = 0;
		 						//lsui8StopState = 1;
		 						gstUMtoCMoperational.commandToControlBoard.bits.stopPressed = 1;
		 						gstUMtoCMoperational.commandRequestStatus = eACTIVE;

		 						//
		 						// Change state
		 						//
		 						gApertureheightState = A_WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE;

		 					}
		 				}
		 				else if(gKeysStatus.bits.Key_Stop_released)
		 				{

		 					gKeysStatus.bits.Key_Stop_released = 0;


		 				}
		 				//
		 				// If no key is pressed
		 				//
		 				else
		 				{
		 					//
		 					// Change state
		 					//
		 					gApertureheightState = A_DISPLAY_INSTALLATION_STATE;

		 					if( (gstDriveInstallation.bits.installationValid == 1) &&
		 									(1 == gstDriveBoardStatus.bits.driveApertureheight)
		 								  )
		 					{
		 						gApertureheightState = A_VALIDATING_APERTURE_HEIGHT;
		 					}


		 				}
		 			}/*if((gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
		 				(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0))*/


		   break;
	 case A_WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE:
		 if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
		 			{
		 				if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
		 				{
		 					//
		 					// Reset request and response status
		 					//


		 					if(gstUMtoCMoperational.commandToControlBoard.bits.openPressed == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.openPressed = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;

		 					}
		 					else if(gstUMtoCMoperational.commandToControlBoard.bits.openReleased == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.openReleased = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
		 					}
		 					else if(gstUMtoCMoperational.commandToControlBoard.bits.closePressed == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.closePressed = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
		 					}
		 					else if(gstUMtoCMoperational.commandToControlBoard.bits.closeReleased == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.closeReleased = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
		 					}
		 					else if(gstUMtoCMoperational.commandToControlBoard.bits.stopPressed == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.stopPressed = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
		 					}
		 					else if(gstUMtoCMoperational.commandToControlBoard.bits.stopReleased == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.stopReleased = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
		 					}

		 					//
		 					// Change state
		 					//
		 					gApertureheightState = A_DISPLAY_INSTALLATION_STATE;

		 					if( (gstDriveInstallation.bits.installationValid == 1) &&
		 						(1 == gstDriveBoardStatus.bits.driveApertureheight)
		 					 )
		 					{
		 					 gApertureheightState = A_VALIDATING_APERTURE_HEIGHT;
		 					}

		 					//
		 					// Clear screen
		 					//
		 					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		 				}

		 				else if( (gstUMtoCMoperational.commandResponseStatus == eTIME_OUT) ||
		 						 (gstUMtoCMoperational.commandResponseStatus == eFAIL)
		 					   )
		 				{
		 					//
		 					// Reset request and response status
		 					//


		 					if(gstUMtoCMoperational.commandToControlBoard.bits.openPressed == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.openPressed = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
		 					}
		 					else if(gstUMtoCMoperational.commandToControlBoard.bits.openReleased == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.openReleased = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
		 					}
		 					else if(gstUMtoCMoperational.commandToControlBoard.bits.closePressed == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.closePressed = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
		 					}
		 					else if(gstUMtoCMoperational.commandToControlBoard.bits.closeReleased == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.closeReleased = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
		 					}
		 					else if(gstUMtoCMoperational.commandToControlBoard.bits.stopPressed == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.stopPressed = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
		 					}
		 					else if(gstUMtoCMoperational.commandToControlBoard.bits.stopReleased == 1)
		 					{
		 						gstUMtoCMoperational.commandToControlBoard.bits.stopReleased = 0;

		 						gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		 						gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
		 					}

		 					//
		 					// Change state
		 					//
		 					gApertureheightState = A_KEY_PRESS_HANDLE;

		 				}
		 			}


		 break;
	 case A_DISPLAY_INSTALLATION_STATE:
		 if(1 == gstDriveBoardStatus.bits.driveApertureheight)
		 	{



		 		//
		 		// Allow display operation only after every 500 milliseconds.
		 		//
		 		if(lsDelay500msStart == 0)
		 		{

		 					//
		 					// Clear screen only once
		 					//
		 					if(bClearScreen == true)
		 					{
		 						//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		 						GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
		 						bClearScreen = false;
		 					}

		 					//
		 					// Display texts as per currently set installation parameter
		 					//

		 					GrRectFIllBolymin(0, 126, 0, 15, 0x00, true);
		 					displayText("A130", 2, 0, false, false, false, false, false, false);
		 					GrRectFIllBolymin(0, 126, 16, 31, 0x00, true);
		 					if(gu8_language == Japanese_IDX)
		 					{
		 					displayText("ハンカイ_セッテイ", 2, 16, false, false, false, false, false, false);
		 					}
		 					else
		 					{
		 					displayText("APERTURE HEIGHT", 2, 16, false, false, false, false,false,true);
		 					}

		 					//
		 					// Display current encoder count in 3rd line
		 					//
		 					memset(lBuff, 0, 21);
		 					usnprintf((char*)lBuff, sizeof(lBuff), "%05d", A_gCurrentEncoderCount);

		 					displayText(lBuff, 2, 32, false, false, false, false, false, false);

		 					//
		 					// Clear active error database
		 					//
		 					//memset(gsActiveAnomalyList, 0, sizeof(struct errorDB)*10);

		 					//
		 					// Capture time
		 					//
		 					lsTickCount500Milliseconds = g_ui32TickCount;

		 					//
		 					// Set 500 milliseconds delay start flag
		 					//
		 					lsDelay500msStart = 1;
		 				}

		 				//
		 				// Change state
		 				//
		 				gApertureheightState = A_GET_CURRENT_COUNT;

		 			}
		 			else
		 			{
		 				//
		 				// Change state
		 				//
		 				gApertureheightState = A_VALIDATING_APERTURE_HEIGHT;
		 			}




		 			// display SHUTTER TYPE CHANGED message for 4 second - YG - NOV 15
		 			if( (get_timego(lsTickCount500Milliseconds) > 400) &&
		 				(lsDelay500msStart == 1 )
		 			  )
		 			{
		 				//
		 				// Reset 500 milliseconds delay start flag
		 				//
		 				lsDelay500msStart = 0;


		 				// variable 'gucStopErrorsDisplay' used to inform error module to start displaying the error - YG - NOV 15
		 				gucStopErrorsDisplay = 0;

		 				// CLEAR COMPLETE SCREEN - YG - NOV 15
		 				bClearScreen = true;
		 			}



		 break;
	 case A_GET_CURRENT_COUNT:
	 {
			static uint8_t lsDelay500msGetCountStart = 0;
					static uint32_t lsTickCount500msGetCount = 0;

					//
					// Start 500 milliseconds delay
					//
					if(error_occ == 1)
					{
							error_occ = 0;
							lsDelay500msGetCountStart = 0;
					}
					if(A_lsDelay500msGetCountStart_cyw == 1)
					{
						lsDelay500msGetCountStart = 0;
						A_lsDelay500msGetCountStart_cyw = 0;
					}
					if(lsDelay500msGetCountStart == 0)
					{
						//
						// Capture time
						//
						lsTickCount500msGetCount = g_ui32TickCount;

						//
						// Set 500 milliseconds delay start flag
						//
						lsDelay500msGetCountStart = 1;
					}



					//
					// Check whether 500 milliseconds delay is achieved
					//
					if( (get_timego(lsTickCount500msGetCount) > 50) &&
						(lsDelay500msGetCountStart == 1)

					  )
					{
						if(gstUMtoCMdatabase.commandRequestStatus == eINACTIVE)
						{
							//
							// Initiate GET_PARAMETER command for parameter number A129
							// i.e. Current Value Monitor
							//
							gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 1;
							gstUMtoCMdatabase.dataToControlBoard.parameterNumber = PARAM_CURRENT_VALUE_MONITOR;
							gstUMtoCMdatabase.destination = eDestDriveBoard;

							//
							// Set command request status as active
							//
							gstUMtoCMdatabase.commandRequestStatus = eACTIVE;

							//
							// Change state
							//
							gApertureheightState = A_WAIT_FOR_GET_CURRENT_COUNT_RESPONSE;
							A_wait_for_current_time = g_ui32TickCount;

							//
							// Reset 500 milliseconds delay start flag
							//
							lsDelay500msGetCountStart = 0;
						}
						else
						{
							//
							// Change state
							//
							gApertureheightState = A_VALIDATING_APERTURE_HEIGHT;
						}
					}
					else
					{
						//
						// Change state
						//
						gApertureheightState = A_VALIDATING_APERTURE_HEIGHT;
					}

	 }
	     break;
	 case A_WAIT_FOR_GET_CURRENT_COUNT_RESPONSE:
		 if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		 			{
		 				if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS)
		 				{
		 					//
		 					// Save received encoder count value
		 					//
		 					A_gCurrentEncoderCount = gstUMtoCMdatabase.getParameterValue;

		 					//
		 					// Display current encoder count in 3rd line
		 					//
		 					memset(lBuff, 0, 5);
		 					usnprintf((char*)lBuff, sizeof(lBuff), "%05d", A_gCurrentEncoderCount);
		 				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
		 				//	usnprintf_D05_cyw((char*)lBuff_cyw,sizeof(lBuff_cyw),(char*)lBuff);
		 					displayText(lBuff, 2, 32, false, false, false, false, false, false);

		 					//
		 					// Reset request and response status
		 					//
		 					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
		 					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
		 					gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 0;
		 					//
		 					// Change state
		 					//
		 					gApertureheightState = A_VALIDATING_APERTURE_HEIGHT;
		 				}

		 				else if( (gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT) ||
		 						 (gstUMtoCMdatabase.commandResponseStatus == eFAIL)
		 					   )
		 				{
		 					//
		 					// Reset request and response status
		 					//
		 					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
		 					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
		 					gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 0;
		 					//
		 					// Change state
		 					//
		 					gApertureheightState = A_GET_CURRENT_COUNT;
		 				    A_lsDelay500msGetCountStart_cyw= 1;
		 				}
		 			}
		 			if(get_timego(A_wait_for_current_time)>300)
		 			{
		 				//
		 				// Reset request and response status
		 				//
		 				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
		 			    gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
		 			    gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 0;
		 				//
		 				// Change state
		 				//
		 			   gApertureheightState = A_GET_CURRENT_COUNT;
		 				A_lsDelay500msGetCountStart_cyw= 1;
		 			}
		 break;
	 case A_VALIDATING_APERTURE_HEIGHT:
	 {
		 static uint8_t lsDelay3secStartValidation = 0;



		 			if(error_occ1 ==1)
		 			{
		 				error_occ1 = 0;
		 				lsDelay3secStartValidation = 0;
		 			}

		 			//
		 			// Check whether installation validation is going on
		 			//
		 			if( (gstDriveInstallation.bits.installationValid == 1) &&
		 				(1 == gstDriveBoardStatus.bits.driveApertureheight)
		 			  )
		 			{
		 				if(lsDelay3secStartValidation == 0)
		 				{
		 					//
		 					// Clear screen
		 					//
		 					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		 					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		 					//
		 					// Display Validating installation message
		 					//
		 					if(gu8_language == Japanese_IDX)
		 					{
		 					displayText("ゲンテンホセイチュウ", 2, 0, false, false, false, false, false, false);
		 					}
		 					else
		 					{
		 				    displayText("VALIDATING", 2, 0, false, false, false, false, false, true);
		 					displayText("INSTALLATION...", 2, 16, false, false, false, false,false,true);
		 					}
		                      //displayText("VALIDATING", 2, 0, false, false, false, false, false, false);
		 					//displayText("INSTALLATION...", 2, 16, false, false, false, false);
		 					//displayText("リミット_セッテイ_モード", 2, 16, false, false, false, false, false, false);
		 					//
		 					// Set LEDs to default value
		 					//
		 					//gstLEDcontrolRegister.faultLED = LED_OFF;
		 					//gstLEDcontrolRegister.autoManualLED = LED_OFF;
		 					//gstLEDcontrolRegister.powerLED = LED_ON;

		 					//
		 					// Capture time
		 					//
		 					gTickCount3Seconds = g_ui32TickCount;

		 					//
		 					// Start 3 seconds delay
		 					//
		 					lsDelay3secStartValidation = 1;
		 				}

		 			}
		 			else if((lsDelay3secStartValidation == 1))
		 			{
		 				//
		 				// Remain in current state
		 				//
		 				//gInstallationState = KEY_PRESS_HANDLE;
		 				gApertureheightState = A_VALIDATING_APERTURE_HEIGHT;
		 			}
		 			else
		 			{
		 				//
		 				// Change state
		 				//
		 				gApertureheightState = A_CHECK_FOR_APERTURE_HEIGHT_SUCCESS;
		 			}

		 			//
		 			// Check whether 3 seconds delay is achieved and
		 			// validation flag is reseted
		 			//
		 			if( (get_timego(gTickCount3Seconds) > 300) &&
		 				(gstDriveInstallation.bits.installationValid == 0) &&
		 				(gstDriveInstallation.val != 0) &&
		 				(lsDelay3secStartValidation == 1)
		 			  )
		 			{
		 				//
		 				// Change state
		 				//
		 				gApertureheightState = A_CHECK_FOR_APERTURE_HEIGHT_SUCCESS;

		 				//
		 				// Reset 3 seconds delay start flag
		 				//
		 				lsDelay3secStartValidation = 0;

		 				//
		 				// Clear screen
		 				//
		 				//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		 				GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
		 			}

		 			if((lsDelay3secStartValidation == 1)&&(gApertureheightState != A_CHECK_FOR_APERTURE_HEIGHT_SUCCESS))
		 			{
		 				gApertureheightState = A_KEY_PRESS_HANDLE;
		 			}
	 }
		 break;

		case A_CHECK_FOR_APERTURE_HEIGHT_SUCCESS:
			{
				//
				// Check for installation success
				//
				if( (gstDriveInstallation.bits.installA100 == 1) && (1 == gstDriveBoardStatus.bits.driveInstallation) )
				{
					//
					// Clear active error database
					//
					//memset(gsActiveAnomalyList, 0, sizeof(struct errorDB)*10);

					//
					// Change state
					//
					//gInstallationState = DISPLAY_INSTALLATION_STATE;
					gApertureheightState = A_KEY_PRESS_HANDLE;
				}

				else if(
						(gstDriveInstallation.bits.installationSuccess == 1) &&
						//	Added check of driveReady to handle display screen hang issue at "INSTALLATION SUCCESSFUL"
						//	when a key is pressed during installation success - Dec 2015
						( (1 == gstDriveBoardStatus.bits.driveInstallation) || (1 == gstDriveBoardStatus.bits.driveReady) )
				)
				{
					//
					// Check whether commit installation is going on.
					// If not, than display installation successful message.
					// If yes, than wait till committing installation
					//
					if(lsCommitInstallation == 0)
					{
						if(lsDelay3secStart == 0)
						{
							//
							// Clear screen
							//
							//GrRectFIllBolymin(0, 126, 0, 63, true, true);
							GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

							//
							// Display installation successful message
							//

							if(gu8_language == Japanese_IDX)
							{
	                        displayText("ハンカイ_セッテイ", 2, 0, false, false, false, false, false, false);
	                        displayText("カンリョウ", 2, 16, false, false, false, false, false, false);
							}
							else
							{
							displayText("INSTALLATION", 2, 0, false, false, false, false,false,true);
							displayText("SUCCESSFUL", 2, 16, false, false, false, false,false,true);
							}
	                       // setting_flag = 0;

							//
							// Capture time
							//
							gTickCount3Seconds = g_ui32TickCount;

							//
							// Start 3 seconds delay
							//
							lsDelay3secStart = 1;
						}

						//
						// Check whether 3 seconds delay is achieved
						//
						if( (get_timego( gTickCount3Seconds) > 400) &&//old is 300  cyw modify 400 successful screen not out
							(lsDelay3secStart == 1) &&
							//	Added check of commandRequestStatus to handle display screen hang issue at "INSTALLATION SUCCESSFUL"
							//	when a key is pressed during installation success - Dec 2015
							(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
						  )
						{

	#if 1
							// Initiate enter command here to commit installation.
							if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.enterPressed = 1;
								gstUMtoCMoperational.commandRequestStatus = eACTIVE;
							}
	#endif

							//
							// Change state
							//
							gApertureheightState = A_WAIT_FOR_COMMIT_APERTURE_HEIGHT;

							//
							// Reset 3 seconds delay start flag and set commit installation flag
							//
							lsDelay3secStart = 0;
							lsCommitInstallation = 1;
						}
					}
					else
					{
						//
						// Change state
						//
						gApertureheightState = A_WAIT_FOR_COMMIT_APERTURE_HEIGHT;
					}
				}

				else if( (gstDriveInstallation.bits.installationFailed == 1) && (1 == gstDriveBoardStatus.bits.driveInstallation) )
				{
					//
					// Clear screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

					//
					// Display Installation failed message
					//
					if(gu8_language == Japanese_IDX)
					{

	                 displayText("ハンカイ_セッテイ", 2, 0, false, false, false, false, false, false);
				     displayText("エラー", 2, 16, false, false, false, false, false, false);
					}
					else
					{
						displayText("INSTALLATION", 2, 0, false, false, false, false,false,true);
						displayText("FAILED", 2, 16, false, false, false, false,false,true);
					}
					//setting_flag = 0;
					//
					// Change state
					//
					gApertureheightState = A_APERTURE_HEIGHT_FAILED;
					A_gTickCount3Seconds_cyw = g_ui32TickCount;
				}

				else if(gstDriveBoardStatus.bits.driveReady == 1)
				{
					//
					// Change state
					//
					gApertureheightState = A_WAIT_FOR_COMMIT_APERTURE_HEIGHT;
				}

				else
				{
					//
					// Change state
					//
					gApertureheightState = A_KEY_PRESS_HANDLE;
				}

				break;
			}
	case A_APERTURE_HEIGHT_FAILED:
		if(gstDriveInstallation.bits.installA100 == 1)
					{
						//
						// Clear active error database
						//
						//memset(gsActiveAnomalyList, 0, sizeof(struct errorDB)*10);

						//
						// Change state
						//
						gApertureheightState = A_DISPLAY_INSTALLATION_STATE;

						//
						// Clear screen
						//
						//GrRectFIllBolymin(0, 126, 0, 63, true, true);
						GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
					}
					else
					{

						//
						// Check whether fault flags are reseted
						//
						if( (gstDriveBoardStatus.bits.driveFault == 0) &&
							(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
							//(gstDriveInstallation.bits.installationFailed == 0) &&
							(gstDriveInstallation.bits.installationFailed == 0) &&
							(gstDriveInstallation.val != 0)
						  )
						{
							//
							// Clear active error database
							//
							memset(gsActiveAnomalyList, 0, sizeof(struct errorDB)*20);

							//
							// Change state
							//
							gApertureheightState = A_DISPLAY_INSTALLATION_STATE;

							//
							// Clear screen
							//
							//GrRectFIllBolymin(0, 126, 0, 63, true, true);
							GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

							//
							// Reset commit installation flag
							//
							lsCommitInstallation = 0;
						}
						//}
					}

					//gstDriveBoardStatus.bits.driveInstallation =0;

					//
					// Capture time
					//
				   // gTickCount3Seconds = g_ui32TickCount;


				    //
				    //change state
				    //
					//gInstallationState = DELAY_3_SECONDS;
					//gNextInstallState = CHECK_SHUTTER_STATE;
					//gInstallationState = CHECK_SHUTTER_STATE;
					if(get_timego(A_gTickCount3Seconds_cyw)>300)
					{
									//
									// Clear screen
									//
									//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

									//
									// Set active functional block as home screen functional block
									//

					lsDelay500msStart=0;
					error_occ =1;
					error_occ1 = 1;
					gstDriveInstallation.bits.installA100 = 1;
					A_gui8UpState = 0;
					A_gui8DownState = 0;
					A_gui8EnterState = 0;
					A_gui8OpenState = 0;
					A_gui8CloseState = 0;

									//
									// Reset installation state
									//
					lsDelay500msStart = 0;
				    lsDelay3secStart = 0;
						//static uint8_t lsDelay3SecInstallFail = 0;
				    lsTickCount500Milliseconds = 0;
				    lsCommitInstallation = 0;

					gstDriveInstallation.bits.installationFailed =0;
					gstDriveInstallation.bits.installA100=1;
					gstDriveInstallation.bits.installA101=0;
					gstDriveInstallation.bits.installA102=0;
					gstDriveInstallation.bits.installationValid=0;
		            gstDriveInstallation.bits.installationSuccess=0;
					gstDriveBoardStatus.bits.driveInstallation = 0;





					gApertureheightState = A_CHECK_SHUTTER_STATE;


					//installationPaint();
					//psActiveFunctionalBlock = &gsInstallationFunctionalBlock;
					}

		break;
	case A_WAIT_FOR_COMMIT_APERTURE_HEIGHT:
			{
				//
				// Check whether Drive installation flag is reseted and drive ready
				// flag is set.
				//
				if( (gstDriveBoardStatus.bits.driveApertureheight == 0) &&
					(gstDriveBoardStatus.bits.driveReady == 1)
				  )
				{
					if(eINACTIVE == gstUMtoCMdatabase.commandRequestStatus)
					{

								gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 1;
								gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 130;
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = A_gCurrentEncoderCount;
								gstUMtoCMdatabase.destination = eDestDriveBoard;

								//
								// Set command request status as active
								//
								gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
					}

					if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
					{
						if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS)
						{


							gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
							gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
							gstUMtoCMdatabase.commandToControlBoard.bits.setParameter =0;
							//
							// Clear screen
							//
							//GrRectFIllBolymin(0, 126, 0, 63, true, true);
							GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

							if(gu8_language == Japanese_IDX)
							{
							    displayText("ハンカイ_セッテイ", 2, 0, false, false, false, false, false, false);
							    displayText("カンリョウ", 2, 16, false, false, false, false, false, false);
							}
							else
							{
								displayText("APERTURE HEIGHT", 2, 0, false, false, false, false,false,true);
								displayText("SUCCESSFUL", 2, 16, false, false, false, false,false,true);
							}

							gTickCount3Seconds = g_ui32TickCount;

							gApertureheightState = A_DELAY_3_SECONDS;
							gNextApertureheightState = A_OUT_APERTURE_HEIGHT;

						}
						else if( (gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT) ||
								(gstUMtoCMdatabase.commandResponseStatus == eFAIL)
								)
						{
							gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
							gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
							gstUMtoCMdatabase.commandToControlBoard.bits.setParameter =0;
							//
							// Clear screen
							//
							//GrRectFIllBolymin(0, 126, 0, 63, true, true);
							GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

							//
							// Display Installation failed message
							//
							if(gu8_language == Japanese_IDX)
							{

									displayText("ハンカイ_セッテイ", 2, 0, false, false, false, false, false, false);
									displayText("エラー", 2, 16, false, false, false, false, false, false);
							}
							else
							{
									displayText("APERTURE HEIGHT", 2, 0, false, false, false, false,false,true);
									displayText("FAILED", 2, 16, false, false, false, false,false,true);
							}

							gTickCount3Seconds = g_ui32TickCount;
							gApertureheightState = A_DELAY_3_SECONDS;
							gNextApertureheightState = A_OUT_APERTURE_HEIGHT;
						}

					}
				}
				else
				{
					//
					// Change state
					//
					//gApertureheightState = A_KEY_PRESS_HANDLE;
					if(get_timego(wait_for_cleardrivedault_time)>100)
					{
						//
						// Clear screen
						//
						//GrRectFIllBolymin(0, 126, 0, 63, true, true);
						GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

						//
						// Display Installation failed message
						//
						if(gu8_language == Japanese_IDX)
						{

									displayText("ハンカイ_セッテイ", 2, 0, false, false, false, false, false, false);
									displayText("エラー", 2, 16, false, false, false, false, false, false);
						}
						else
						{
									displayText("APERTURE HEIGHT", 2, 0, false, false, false, false,false,true);
									displayText("FAILED", 2, 16, false, false, false, false,false,true);
						}

									gTickCount3Seconds = g_ui32TickCount;
									gApertureheightState = A_DELAY_3_SECONDS;
									gNextApertureheightState = A_OUT_APERTURE_HEIGHT;
					}


				}

		}
				break;

	case A_DELAY_3_SECONDS:
			{
				//
				// Check for 3 seconds delay achieved. If yes, then change state.
				//
				if(get_timego ( gTickCount3Seconds) > 300 )
				{
					gTickCount3Seconds = 0;
					gApertureheightState = gNextApertureheightState;
				}

				break;
			}
	case A_OUT_APERTURE_HEIGHT:
			{
				bClearScreen = true;
				lsCommitInstallation = 0;

				//	Added to handle display screen hang issue at "INSTALLATION SUCCESSFUL" when a key is
				//	pressed during installation success - Dec 2015
				//	Clear key status flags before exiting the installation functional block
				A_gui8UpState = 0;
				A_gui8DownState = 0;
				A_gui8EnterState = 0;
				A_gui8OpenState = 0;
				A_gui8CloseState = 0;

				//
				// Clear screen
				//
				//GrRectFIllBolymin(0, 126, 0, 63, true, true);
				GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

				//
				// Set active functional block as home screen functional block
				//


				if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
				  {
				    	gui8SettingsModeStatus = DEACTIVATED;
				    	gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus = 1;
				    	gstUMtoCMoperational.additionalCommandData = 0;
				    	gstUMtoCMoperational.commandRequestStatus = eACTIVE;
				    	//flag_out_setting_cyw = 2;
				    }
				  gui8SettingsModeStatus = DEACTIVATED;
				  gui8SettingsScreen = DEACTIVATED;

				if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
				{
				  	if((gstUMtoCMoperational.commandResponseStatus == eSUCCESS)||(gstUMtoCMoperational.commandResponseStatus==eTIME_OUT)||(gstUMtoCMoperational.commandResponseStatus==eFAIL))
				  {

				  		gValueParamGetStarted = 0;
				  		gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				  		gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				  		gstUMtoCMoperational.commandToControlBoard.val = 0;

				  			psActiveFunctionalBlock = &gsHomeScreenFunctionalBlock;

				//
				// Call home screen paint function
				//
				  			psActiveFunctionalBlock->pfnPaintFirstScreen();

				//
				// Reset installation state
				//
				  			gApertureheightState = A_CHECK_SHUTTER_STATE;


				  			guest_reinit();
				  }
				}
				break;
			}
	case A_STATE_UNDEFINED:
		{
		    break;
		}
	default:break;

	}
	//installationLEDHandler();

	//
	// Display anomalies on fourth line of display.
	//
	if(  ( 	(gstDriveInstallation.bits.installA130 == 1)  ||
			(gstDriveInstallation.bits.installationValid) ||
			(gstDriveInstallation.bits.installationFailed)
		 ) && (1 == gstDriveBoardStatus.bits.driveApertureheight)
	  )
	{
		displayAnomalies();
	}

	//
	// Check whether any fatal error has occurred.
	//
	if(	(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 1) ||
	    (gstControlBoardStatus.bits.controlFaultUnrecoverable == 1) ||
	    (gstDisplayBoardStatus.bits.displayFaultUnrecoverable == 1)
	  )
	{
		//
		// Change state
		//
		gApertureheightState = A_STATE_UNDEFINED;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: apertureheightUp
 *
 * Function Description:
 * Handles Up key press and release events during installation.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success.
 *
 ********************************************************************************/
uint8_t apertureheightUp(void)
{
	if(
				(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
				(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)
		)
		{
			//
			// Handle Up key press
			//
			if( (gKeysStatus.bits.Key_Up_pressed) &&
					(0 == A_gui8OpenState) && (0 == A_gui8CloseState) && (0 == A_gui8DownState) && (0 == A_gui8EnterState)
			)
			{
				//
				// Initiate UP_PRESSED command
				//
				if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
				{
					gKeysStatus.bits.Key_Up_pressed = 0;

					A_gui8UpState = 1;
					gstUMtoCMoperational.commandToControlBoard.bits.upPressed = 1;
					gstUMtoCMoperational.commandRequestStatus = eACTIVE;
				}
			}

			if(gKeysStatus.bits.Key_Up_released)
			{
				//
				// Initiate UP_PRESSED command
				//
				if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
				{
					gKeysStatus.bits.Key_Up_released = 0;

					A_gui8UpState = 0;
					gstUMtoCMoperational.commandToControlBoard.bits.upReleased = 1;
					gstUMtoCMoperational.commandRequestStatus = eACTIVE;
				}
			}
		}

		//
		// Handle command response
		//
		if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
		{
			if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
			{
				if(gstUMtoCMoperational.commandToControlBoard.bits.upPressed == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.upPressed = 0;

					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				}
				else if(gstUMtoCMoperational.commandToControlBoard.bits.upReleased == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.upReleased = 0;

					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				}
			}

			else if( (gstUMtoCMoperational.commandResponseStatus == eTIME_OUT) ||
					 (gstUMtoCMoperational.commandResponseStatus == eFAIL)
					)
			{
				if(gstUMtoCMoperational.commandToControlBoard.bits.upPressed == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.upPressed = 0;

					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				}
				else if(gstUMtoCMoperational.commandToControlBoard.bits.upReleased == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.upReleased = 0;

					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				}
			}
		}

		return 0;
}


/******************************************************************************
 * FunctionName: apertureheightDown
 *
 * Function Description:
 * Handles Down key press and release events during installation.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success.
 *
 ********************************************************************************/
uint8_t apertureheightDown(void)
{
	if(
				(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
				(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)
		)
		{
			//
			// Handle Down key press
			//
			if( (gKeysStatus.bits.Key_Down_pressed) &&
					(0 == A_gui8OpenState) && (0 == A_gui8CloseState) && (0 == A_gui8UpState) && (0 == A_gui8EnterState)
			)
			{
				//
				// Initiate DOWN_PRESSED command
				//
				if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
				{
					gKeysStatus.bits.Key_Down_pressed = 0;
					A_gui8DownState = 1;
					gstUMtoCMoperational.commandToControlBoard.bits.downPressed = 1;
					gstUMtoCMoperational.commandRequestStatus = eACTIVE;
				}
			}

			if(gKeysStatus.bits.Key_Down_released)
			{
				//
				// Initiate DOWN_PRESSED command
				//
				if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
				{
					gKeysStatus.bits.Key_Down_released = 0;
					A_gui8DownState = 0;
					gstUMtoCMoperational.commandToControlBoard.bits.downReleased = 1;
					gstUMtoCMoperational.commandRequestStatus = eACTIVE;
				}
			}
		}

		//
		// Handle command response
		//
		if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
		{
			if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
			{


				if(gstUMtoCMoperational.commandToControlBoard.bits.downPressed == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.downPressed = 0;

					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				}
				else if(gstUMtoCMoperational.commandToControlBoard.bits.downReleased == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.downReleased = 0;

					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				}
			}

			else if( (gstUMtoCMoperational.commandResponseStatus == eTIME_OUT) ||
					 (gstUMtoCMoperational.commandResponseStatus == eFAIL)
					)
			{

				if(gstUMtoCMoperational.commandToControlBoard.bits.downPressed == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.downPressed = 0;

					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				}
				else if(gstUMtoCMoperational.commandToControlBoard.bits.downReleased == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.downReleased = 0;

					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				}


			}
		}

		return 0;
}


/******************************************************************************
 * FunctionName: apertureheightMode
 *
 * Function Description:
 * Handles mode key press event during installation.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success.
 *
 ********************************************************************************/

uint8_t apertureheightMode(void)
{
	if(gKeysStatus.bits.Key_Mode_pressed)
	{
				gKeysStatus.bits.Key_Mode_pressed = 0;
				//psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
				//psActiveFunctionalBlock->pfnPaintFirstScreen();
				//psActiveFunctionalBlock = &gsHomeScreenFunctionalBlock;
				//psActiveFunctionalBlock->pfnPaintFirstScreen();
				setting_flag = 1;

				if(gstDriveBoardStatus.bits.driveApertureheight == 1)
				{
								if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
								{
									 //Line uncommneted by YPG on 13 Jan to solve multiple "confirm substate installtion" command on "enter" press
									A_gui8ModeState = 1;
									gstUMtoCMoperational.commandToControlBoard.bits.enterPressed = 1;
									gstUMtoCMoperational.commandRequestStatus = eACTIVE;

									//gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 1;
									//gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 130;
									//gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = A_gCurrentEncoderCount;
									//gstUMtoCMdatabase.destination = eDestDriveBoard;

									//gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
								}
				}
				else
				{
					psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
					psActiveFunctionalBlock->pfnPaintFirstScreen();
				}


	}

	if(A_gui8ModeState == 1)
	{
			if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
			{
				if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
				{
					A_gui8ModeState = 0;
					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;

					if(gstUMtoCMoperational.commandToControlBoard.bits.enterPressed == 1)
					{
						gstUMtoCMoperational.commandToControlBoard.bits.enterPressed = 0;





						//
						// Display installation successful message
						//



						gApertureheightState = A_OUT_APERTURE_HEIGHT;

						wait_for_cleardrivedault_time = g_ui32TickCount;

					}
					else if(gstUMtoCMoperational.commandToControlBoard.bits.enterReleased == 1)
					{
						gstUMtoCMoperational.commandToControlBoard.bits.enterReleased = 0;

					//	gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					//	gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
					}
				}

				else if( (gstUMtoCMoperational.commandResponseStatus == eTIME_OUT) ||
						 (gstUMtoCMoperational.commandResponseStatus == eFAIL)
						)
				{

					A_gui8ModeState = 0;
					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;


					if(gstUMtoCMoperational.commandToControlBoard.bits.enterPressed == 1)
					{
						gstUMtoCMoperational.commandToControlBoard.bits.enterPressed = 0;

						//
						// Clear screen
						//
					   //GrRectFIllBolymin(0, 126, 0, 63, true, true);
						GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

						//
						// Display Installation failed message
						//
						if(gu8_language == Japanese_IDX)
						{

							 displayText("ハンカイ_セッテイ", 2, 0, false, false, false, false, false, false);
							 displayText("エラー", 2, 16, false, false, false, false, false, false);
						}
						else
						{
							displayText("APERTURE HEIGHT", 2, 0, false, false, false, false,false,true);
							displayText("FAILED", 2, 16, false, false, false, false,false,true);
						}
						gTickCount3Seconds = g_ui32TickCount;

						gApertureheightState = A_OUT_APERTURE_HEIGHT;
						//gNextApertureheightState = A_OUT_APERTURE_HEIGHT;

					}
					else if(gstUMtoCMoperational.commandToControlBoard.bits.enterReleased == 1)
					{
						gstUMtoCMoperational.commandToControlBoard.bits.enterReleased = 0;

						//gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
						//gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
					}
				}
			}

		}
					return 0;
}


/******************************************************************************
 * FunctionName: apertureheightEnter
 *
 * Function Description:
 * Handles Enter key press and release events during installation.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success.
 *
 ********************************************************************************/
uint8_t apertureheightEnter(void)
{
	if(
				(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
				(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)
		)
		{
			//
			// Handle Enter key press
			//
			if( (gKeysStatus.bits.Key_Enter_pressed) /*(gKeysStatus.bits.Key_3secEnter_pressed)*/ &&
					(0 == A_gui8OpenState) && (0 == A_gui8CloseState) && (0 == A_gui8UpState) && (0 == A_gui8DownState)

			)
			{
				gKeysStatus.bits.Key_Enter_pressed = 0;

				setting_flag = 1;

				//
				// Initiate ENTER_PRESSED command
				//
				if(gstDriveBoardStatus.bits.driveApertureheight == 1)
				{
				if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
				{
					 //Line uncommneted by YPG on 13 Jan to solve multiple "confirm substate installtion" command on "enter" press
					A_gui8EnterState = 1;
					gstUMtoCMoperational.commandToControlBoard.bits.enterPressed = 1;
					gstUMtoCMoperational.commandRequestStatus = eACTIVE;

					//gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 1;
					//gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 130;
					//gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = A_gCurrentEncoderCount;
					//gstUMtoCMdatabase.destination = eDestDriveBoard;

					//gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
				}
				}
			}


			//if(gKeysStatus.bits.Key_Enter_released)
			//{
			//	gKeysStatus.bits.Key_Enter_released = 0;
			//	A_gui8EnterState = 0;


			//}
		}/*if((gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
			(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0))*/

		//
		// Handle command response
		//
	if(A_gui8EnterState == 1)
	{
		if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
		{
			if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
			{
				A_gui8EnterState = 0;
				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;

				if(gstUMtoCMoperational.commandToControlBoard.bits.enterPressed == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.enterPressed = 0;





					//
					// Display installation successful message
					//



					gApertureheightState = A_WAIT_FOR_COMMIT_APERTURE_HEIGHT;

					wait_for_cleardrivedault_time = g_ui32TickCount;

				}
				else if(gstUMtoCMoperational.commandToControlBoard.bits.enterReleased == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.enterReleased = 0;

				//	gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				//	gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				}
			}

			else if( (gstUMtoCMoperational.commandResponseStatus == eTIME_OUT) ||
					 (gstUMtoCMoperational.commandResponseStatus == eFAIL)
					)
			{

				A_gui8EnterState = 0;
				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;


				if(gstUMtoCMoperational.commandToControlBoard.bits.enterPressed == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.enterPressed = 0;

					//
					// Clear screen
					//
				   //GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

					//
					// Display Installation failed message
					//
					if(gu8_language == Japanese_IDX)
					{

						 displayText("ハンカイ_セッテイ", 2, 0, false, false, false, false, false, false);
						 displayText("エラー", 2, 16, false, false, false, false, false, false);
					}
					else
					{
						displayText("APERTURE HEIGHT", 2, 0, false, false, false, false,false,true);
						displayText("FAILED", 2, 16, false, false, false, false,false,true);
					}
					gTickCount3Seconds = g_ui32TickCount;

					gApertureheightState = A_DELAY_3_SECONDS;
					gNextApertureheightState = A_OUT_APERTURE_HEIGHT;

				}
				else if(gstUMtoCMoperational.commandToControlBoard.bits.enterReleased == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.enterReleased = 0;

					//gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					//gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				}
			}
		}

	}
		return 0;

}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsApertureheightFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	apertureheightPaint,
	apertureheightRunTime,
	apertureheightUp,
	apertureheightDown,
	apertureheightMode,
	apertureheightEnter
};
