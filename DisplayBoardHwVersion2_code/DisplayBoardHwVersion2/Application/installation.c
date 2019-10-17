/*********************************************************************************
 * FileName: installation.c
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

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/gpio.h>
#include "inc/hw_types.h" //20170414      201703_No.32
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
#include "logger.h"           //20170414      201703_No.32

/****************************************************************************/

/****************************************************************************
 *  Macros
****************************************************************************/
//#define TEST_MODULE_ON
//#define SKIP_GET_CURRENT_COUNT

//**************************************************************************
// Installation States
//**************************************************************************
#define CHECK_SHUTTER_STATE						0
#define START_INSTALLATION						1
#define WAIT_FOR_START_INSTALLATION_REPLAY		2
#define WAIT_FOR_START_INSTALLATION             3
#define KEY_PRESS_HANDLE						4
#define WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE		5
#define DISPLAY_INSTALLATION_STATE				6
#define GET_CURRENT_COUNT						7
#define WAIT_FOR_GET_CURRENT_COUNT_RESPONSE		8
#define VALIDATING_INSTALLATION					9
#define CHECK_FOR_INSTALLATION_SUCCESS			10
#define INSTALLATION_FAILED						11
#define WAIT_FOR_COMMIT_INSTALLATION			12
#define DELAY_3_SECONDS							13
#define STATE_UNDEFINED							14

/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
//extern uint32_t g_ui32TickCount;
extern uint32_t gTickCount3Seconds;

extern struct errorDB gsActiveAnomalyList[20];

extern uint32_t gShutterType;
extern uint8_t setting_flag;

uint32_t gTickCount3Seconds_cyw=0;

void Set_lcdlightON(void);
void guest_reinit(void);

static uint8_t lsDelay500msGetCountStart_cyw=0;
static unsigned char Flag_startInstallation=0;      //20170628   201703_No.CQ12
uint32_t wait_for_current_time=0;
//**************************************************************************
// Current encoder count to be fetched from drive board
//**************************************************************************
uint32_t gCurrentEncoderCount = 0;

//**************************************************************************
// Current installation state
//**************************************************************************
uint8_t gInstallationState = CHECK_SHUTTER_STATE;

//**************************************************************************
// Next installation state
//**************************************************************************
uint8_t gNextInstallState = 0xFF;

//**************************************************************************
// Keys states used during installation
//**************************************************************************
uint8_t gui8UpState = 0;
uint8_t gui8DownState = 0;
uint8_t gui8EnterState = 0;
uint8_t gui8OpenState = 0;
uint8_t gui8CloseState = 0;


//**************************************************************************
// Change shutter type
//**************************************************************************
uint8_t gucShutterTypeChangedFlag = 0;


uint32_t A100_his_cyw=0;		//20170414      201703_No.32
//static uint8_t AXXX_flag_cyw=0;
 uint32_t A101_his_cyw=0;		//20170414      201703_No.32
//static uint8_t Aover_flag_cyw=0;
uint32_t A102_his_cyw=0;		//20170414      201703_No.32
//static uint8_t A102_flag_cyw;

/****************************************************************************/

/******************************************************************************
 * FunctionName: installationLEDHandler
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns: void
 *
 ********************************************************************************/
void installationLEDHandler(void)
{
	static bool bFaultOccurred = false;

	//
	// Handle blink rate for A100, A101 and A102 states
	//
	if(
		(   (1 != gstDriveBoardStatus.bits.driveFault) &&
			(1 != gstDriveBoardStatus.bits.driveFaultUnrecoverable) &&
			(1 != gstControlBoardStatus.bits.controlFault) &&
			(1 != gstControlBoardStatus.bits.controlFaultUnrecoverable)&&
			(1 != gstDisplayBoardStatus.bits.displayFault) &&
			(1 != gstDisplayBoardStatus.bits.displayFaultUnrecoverable)
		) //|| ( 0xFF == gstEMtoUM.faultLEDstatus )
	  )
	{
		//
		// Reset LED status to synchronize LEDs blinking since errors are removed.
		//
		if(true == bFaultOccurred)
		{
			gstLEDcontrolRegister.autoManualLED = 0;
			gstLEDcontrolRegister.faultLED = 0;
			gstLEDcontrolRegister.powerLED = 0;

			//
			// Rest fault occurred flag
			//
			bFaultOccurred = false;
		}

		//
		// Set LEDs blink rate as per current installation state
		//
		else if(gstDriveInstallation.bits.installA100 == 1)
		{
			gstLEDcontrolRegister.autoManualLED = _BLINK_STATUS_500_MSEC;
			gstLEDcontrolRegister.faultLED = _BLINK_STATUS_500_MSEC;
			gstLEDcontrolRegister.powerLED = _BLINK_STATUS_500_MSEC;
		}

		else if(gstDriveInstallation.bits.installA101 == 1)
		{
			gstLEDcontrolRegister.autoManualLED = _BLINK_STATUS_250_MSEC;
			gstLEDcontrolRegister.faultLED = _BLINK_STATUS_250_MSEC;
			gstLEDcontrolRegister.powerLED = _BLINK_STATUS_250_MSEC;
		}

		else if(gstDriveInstallation.bits.installA102 == 1)
		{
			gstLEDcontrolRegister.autoManualLED = _BLINK_STATUS_100_MSEC;
			gstLEDcontrolRegister.faultLED = _BLINK_STATUS_100_MSEC;
			gstLEDcontrolRegister.powerLED = _BLINK_STATUS_100_MSEC;
		}

		else if( (gstDriveInstallation.bits.installationValid == 1) ||
				 (gstDriveInstallation.bits.installationSuccess == 1) ||
				 (gstDriveInstallation.bits.installationFailed == 1)
		       )
		{
			//
			// Set LEDs to their default values
			//
			gstLEDcontrolRegister.autoManualLED = LED_OFF;
			gstLEDcontrolRegister.faultLED = LED_OFF;
			gstLEDcontrolRegister.powerLED = LED_ON;
		}
	}

	else if(
			( (1 == gstDriveBoardStatus.bits.driveFault) ||
			 (1 == gstDriveBoardStatus.bits.driveFaultUnrecoverable) ||
			 (1 == gstControlBoardStatus.bits.controlFault) ||
			 (1 == gstControlBoardStatus.bits.controlFaultUnrecoverable) ||
			 (1 == gstDisplayBoardStatus.bits.displayFault) ||
			 (1 == gstDisplayBoardStatus.bits.displayFaultUnrecoverable)
			) //&& ( 0xFF != gstEMtoUM.faultLEDstatus )
		)
	{
		//
		// Update fault LED status
		//
		gstLEDcontrolRegister.faultLED = gstEMtoUM.faultLEDstatus;
//		gstLEDcontrolRegister.autoManualLED = 0;
//		gstLEDcontrolRegister.powerLED = 0;

		//
		// Set fault occurred flag
		//
		bFaultOccurred = true;
	}
}


/******************************************************************************
 * FunctionName: installationRunTime
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
uint8_t installationRunTime(void)
{
	static uint8_t lsDelay500msStart = 0;
	static uint8_t lsDelay3secStart = 0;
	//static uint8_t lsDelay3SecInstallFail = 0;
	static uint32_t lsTickCount500Milliseconds = 0;
	static uint8_t lsCommitInstallation = 0;
	static bool bClearScreen = true;
	//static uint8_t lsAnomalyIndex = 0;
	//static uint8_t lsAnomalyCounter = 0;
	//static uint8_t lsDelay500msGetCountStart = 0;
   static uint8_t error_occ=0,error_occ1=0;


	unsigned char lBuff[21];
	Set_lcdlightON();






	switch(gInstallationState)
	{
		case CHECK_SHUTTER_STATE:
		{


			if(gstDriveBoardStatus.bits.driveInstallation == 1)
			{
				//
				// Change state
				//

				gInstallationState = KEY_PRESS_HANDLE;
				//AXXX_flag_cyw = 0;


			}
			else
			{
				//
				// Change state
				//
				gInstallationState = START_INSTALLATION;
				setting_flag = 1;

				Flag_startInstallation=1;  //20170628   201703_No.CQ12

			}

			break;
		}

		case START_INSTALLATION:
		{
			//
			// Initiate Installation command
			//
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				//
				// Initiate START_INSTALLATION command
				//
				gstUMtoCMoperational.commandToControlBoard.bits.startInstallation = 1;

				//
				// Set request status as active
				//
				gstUMtoCMoperational.commandRequestStatus = eACTIVE;


				//
				// Change state
				//
				gInstallationState = WAIT_FOR_START_INSTALLATION_REPLAY;
			}

			break;
		}

		case WAIT_FOR_START_INSTALLATION_REPLAY:
		{
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
					// Clear screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

					if(gu8_language == Japanese_IDX)
					{
					//displayText("リミットセッテイスタート", 2, 0, false, false, false, false,false,false);
						displayText("タイキチュウ", 2, 0, false, false, false, false, false, false);
					    displayText("リミット_セッテイ...", 1, 16, false, false, false, false, true, false);


					}
					else
					{
					//displayText("INSTALLATION", 2, 16, false, false, false, false,false,true);
						displayText("WAITING FOR", 2, 0, false, false, false, false, false, true);
						displayText("INSTALLATION...", 2, 16, false, false, false, false, false, true);
					}
					//displayText("INSTALLATION", 2, 16, false, false, false, false);
					//displayText("リミット_セッテイ_モード", 2, 16, false, false, false, false,false,false);
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
						//displayText("         ", 2, 48, false, false, false, false, false, false);
						GrRectFIllBolymin(0, 127, 48, 63, 0x00, true);   //20161201
					}
					//
					// Capture time
					//
					gTickCount3Seconds = g_ui32TickCount;

					//
					// Change state
					//
					gInstallationState = DELAY_3_SECONDS;
					gNextInstallState = WAIT_FOR_START_INSTALLATION;
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
//					GrRectFIllBolymin(0, 126, 0, 63, true, true);

					//
					// Paint communication failure message
					//
//					displayText("COMMUNICATION", 2, 0, false, false, false, false);
//					displayText("FAILED", 2, 16, false, false, false, false);

					//
					// Reset request and response status
					//
					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
					gstUMtoCMoperational.commandToControlBoard.val = 0;


					//
					// Change state
					//
					gInstallationState = DELAY_3_SECONDS;
					gNextInstallState = START_INSTALLATION;

				}
			}

			break;
		}

		case WAIT_FOR_START_INSTALLATION:
		{
			if(1 == gstDriveBoardStatus.bits.driveInstallation)
			{
				//
				// Change state
				//
				gInstallationState = KEY_PRESS_HANDLE;

				//
				// Clear screen
				//
				//GrRectFIllBolymin(0, 126, 0, 63, true, true);
				GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
			}
			else
			{
				gInstallationState = CHECK_SHUTTER_STATE;//cyw add
			}

			break;
		}

		case KEY_PRESS_HANDLE:
		{
			//
			//	Added check to see whether system is in healthy state. If a fatal error has occurred then
			//	don't process operation keys - Jan 2016
			//
			if(
					(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
					(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)
			)
			{
				//
				// Check whether open key is pressed
				//
				if( (gKeysStatus.bits.Key_Open_pressed) &&
						(0 == gui8CloseState) && (0 == gui8UpState) && (0 == gui8DownState) && (0 == gui8EnterState)
				)
				{
					//
					// Initiate open command
					//
					if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
					{
						gKeysStatus.bits.Key_Open_pressed = 0;
						gui8OpenState = 1;
						gstUMtoCMoperational.commandToControlBoard.bits.openPressed = 1;
						gstUMtoCMoperational.commandRequestStatus = eACTIVE;

						//
						// Change state
						//
						gInstallationState = WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE;
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
						gui8OpenState = 0;
						gstUMtoCMoperational.commandToControlBoard.bits.openReleased = 1;
						gstUMtoCMoperational.commandRequestStatus = eACTIVE;

						//
						// Change state
						//
						gInstallationState = WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE;
					}
				}

				//
				// Check whether close key is pressed
				//
				else if( (gKeysStatus.bits.Key_Close_pressed) &&
						(0 == gui8OpenState) && (0 == gui8UpState) && (0 == gui8DownState) && (0 == gui8EnterState)
				)
				{
					//
					// Initiate close command
					//
					if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
					{
						gKeysStatus.bits.Key_Close_pressed = 0;
						gui8CloseState = 1;
						gstUMtoCMoperational.commandToControlBoard.bits.closePressed = 1;
						gstUMtoCMoperational.commandRequestStatus = eACTIVE;

						//
						// Change state
						//
						gInstallationState = WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE;
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
						gui8CloseState = 0;
						gstUMtoCMoperational.commandToControlBoard.bits.closeReleased = 1;
						gstUMtoCMoperational.commandRequestStatus = eACTIVE;

						//
						// Change state
						//
						gInstallationState = WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE;
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
						gInstallationState = WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE;

					}
				}
				else if(gKeysStatus.bits.Key_Stop_released)
				{

					gKeysStatus.bits.Key_Stop_released = 0;
					/*
				//
				// Initiate stop command
				//
				if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
				{
					gKeysStatus.bits.Key_Stop_released = 0;
					//lsui8StopState = 1;
					gstUMtoCMoperational.commandToControlBoard.bits.stopReleased = 1;
					gstUMtoCMoperational.commandRequestStatus = eACTIVE;
				}
					 */

				}
				//
				// If no key is pressed
				//
				else
				{
					//
					// Change state
					//
					gInstallationState = DISPLAY_INSTALLATION_STATE;

					if( (gstDriveInstallation.bits.installationValid == 1) &&
									(1 == gstDriveBoardStatus.bits.driveInstallation)
								  )
					{
						gInstallationState = VALIDATING_INSTALLATION;
					}


				}
			}/*if((gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
				(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0))*/

			break;
		}

		case WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE:
		{
			//
			// Check for command response and take respective action
			//
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
					gInstallationState = DISPLAY_INSTALLATION_STATE;

					if( (gstDriveInstallation.bits.installationValid == 1) &&
														(1 == gstDriveBoardStatus.bits.driveInstallation)
													  )
										{
											gInstallationState = VALIDATING_INSTALLATION;
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
					gInstallationState = KEY_PRESS_HANDLE;

				}
			}

			break;
		}

		case DISPLAY_INSTALLATION_STATE:
		{
			//
			// Check whether installation parameters A100, A101 or A102 is set
			//
			if(  ( 	(gstDriveInstallation.bits.installA100 == 1) ||
					(gstDriveInstallation.bits.installA101 == 1) ||
					(gstDriveInstallation.bits.installA102 == 1)
				 ) && (1 == gstDriveBoardStatus.bits.driveInstallation)
			  )
			{

				//
				// Display Shutter Type Changes message
				//
				if (gucShutterTypeChangedFlag == 1)
				{
					// Clear line 1 - 3
					//GrRectFIllBolymin(0, 126, 0, 47, true, true);
					GrRectFIllBolymin(0, 127, 0, 47, 0x00, true);


					if(gu8_language == Japanese_IDX)
					{
					displayText("シャッタータイプノ", 2, 0, false, false, false, false,false,false);
					displayText("ヘンコウヲシマス", 2, 16, false, false, false, false,false,false);
					}
					else
					{
					displayText("SHUTTER TYPE CHANGED", 2, 0, false, false, false, false,false,true);
					}

					usnprintf((char*)lBuff, sizeof(lBuff), "TO:%s", cucA537_SH_TYPE_States[gShutterType]);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
					displayText((unsigned char*)lBuff, 2, 16, false, false, false, false, false, false);

					//	During installation, in case of shutter type change an additional
					//	message added as "parameter reset" for user information
					//
					if(gu8_language == Japanese_IDX)
					{
					displayText("パラメータリセット", 2, 32, false, false, false, false, false, false);
					}
					else
					{
					displayText("PARAMETER RESET...", 2, 32, false, false, false, false,false,true);
					}
					//
					// Set flag to indicate 3 second message display delay start
					gucShutterTypeChangedFlag = 2;
					lsDelay500msStart = 1;  // Use this flag with 'gucShutterTypeChangedFlag' to generate delay of 3 second

					//
					// Capture time
					//
					lsTickCount500Milliseconds = g_ui32TickCount;

				}

				//
				// Allow display operation only after every 500 milliseconds.
				//
				if(lsDelay500msStart == 0)
				{

					Flag_startInstallation=0;  //20170628   201703_No.CQ12

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
					if(gstDriveInstallation.bits.installA100 == 1)
					{
						GrRectFIllBolymin(0, 126, 0, 15, 0x00, true);
						displayText("A100", 2, 0, false, false, false, false, false, false);
						GrRectFIllBolymin(0, 126, 16, 31, 0x00, true);
						//
						if(gu8_language == Japanese_IDX)
						{
						displayText("ジョウゲン", 2, 16, false, false, false, false, false, false);
						}
						else
						{
						displayText("UPPER LIMIT", 2, 16, false, false, false, false,false,true);
						}

					}
					else if(gstDriveInstallation.bits.installA101 == 1)
					{
						GrRectFIllBolymin(0, 126, 0, 15, 0x00, true);   //add 20161018
						displayText("A101", 2, 0, false, false, false, false, false, false);
						GrRectFIllBolymin(0, 126, 16, 31, 0x00, true);
						//
						if(gu8_language == Japanese_IDX)
						{
						displayText("カゲン",2, 16, false, false, false, false, false, false);
						}
						else
						{
						displayText("LOWER LIMIT", 2, 16, false, false, false, false,false,true);
						}

					}
					else if(gstDriveInstallation.bits.installA102 == 1)
					{
						GrRectFIllBolymin(0, 126, 0, 15, 0x00, true);   //add 20161018
						displayText("A102", 2, 0, false, false, false, false, false, false);
						GrRectFIllBolymin(0, 126, 16, 31, 0x00, true);
						//
						if(gu8_language == Japanese_IDX)
						{
						displayText("コウデンギリ", 1, 16, false, false, false, false,true,false);
						}
						else
						{
						displayText("PHOTO CUTTOF LIMIT", 2, 16, false, false, false, false,false,true);
						}

					}
                                                ///*****************start 20170616   201703_No.65******************/
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
						//displayText("         ", 2, 48, false, false, false, false, false, false);
						GrRectFIllBolymin(0, 127, 48, 63, 0x00, true);   //20161201
					}
					                         ///*****************end 20170616   201703_No.65******************/
					//
					// Display current encoder count in 3rd line
					//
					memset(lBuff, 0, 21);
					usnprintf((char*)lBuff, sizeof(lBuff), "%05d", gCurrentEncoderCount);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//usnprintf_D05_cyw((char*)lBuff_cyw,sizeof(lBuff_cyw),(char*)lBuff);
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
				gInstallationState = GET_CURRENT_COUNT;

#ifdef SKIP_GET_CURRENT_COUNT
				gInstallationState = VALIDATING_INSTALLATION;
#endif
			}
			else
			{
				//
				// Change state
				//
				gInstallationState = VALIDATING_INSTALLATION;
			}

			if( (get_timego(lsTickCount500Milliseconds) > 50) &&
				(lsDelay500msStart == 1 && gucShutterTypeChangedFlag == 0)
			  )
			{
				//
				// Reset 500 milliseconds delay start flag
				//
				lsDelay500msStart = 0;
			}


			// display SHUTTER TYPE CHANGED message for 4 second - YG - NOV 15
			if( (get_timego(lsTickCount500Milliseconds) > 400) &&
				(lsDelay500msStart == 1 && gucShutterTypeChangedFlag == 2)
			  )
			{
				//
				// Reset 500 milliseconds delay start flag
				//
				lsDelay500msStart = 0;


				gucShutterTypeChangedFlag = 0; // reset flag

				// variable 'gucStopErrorsDisplay' used to inform error module to start displaying the error - YG - NOV 15
				gucStopErrorsDisplay = 0;

				// CLEAR COMPLETE SCREEN - YG - NOV 15
				bClearScreen = true;
			}


			break;
		}

		case GET_CURRENT_COUNT:
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
			if(lsDelay500msGetCountStart_cyw == 1)
			{
				lsDelay500msGetCountStart = 0;
				lsDelay500msGetCountStart_cyw = 0;
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
				(lsDelay500msGetCountStart == 1) &&
				(gucShutterTypeChangedFlag == 0)
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
					gInstallationState = WAIT_FOR_GET_CURRENT_COUNT_RESPONSE;
					wait_for_current_time = g_ui32TickCount;

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
					gInstallationState = VALIDATING_INSTALLATION;
				}
			}
			else
			{
				//
				// Change state
				//
				gInstallationState = VALIDATING_INSTALLATION;
			}

			break;
		}

		case WAIT_FOR_GET_CURRENT_COUNT_RESPONSE:
		{
			//
			// Check GET_PARAMETER response
			//
			if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
			{
				if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS)
				{
					//
					// Save received encoder count value
					//
					gCurrentEncoderCount = gstUMtoCMdatabase.getParameterValue;

					//
					// Display current encoder count in 3rd line
					//
					memset(lBuff, 0, 5);
					usnprintf((char*)lBuff, sizeof(lBuff), "%05d", gCurrentEncoderCount);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//	usnprintf_D05_cyw((char*)lBuff_cyw,sizeof(lBuff_cyw),(char*)lBuff);
					displayText(lBuff, 2, 32, false, false, false, false, false, false);

					//
					// Reset request and response status
					//
					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

					//
					// Change state
					//
					gInstallationState = VALIDATING_INSTALLATION;
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

					//
					// Change state
					//
					gInstallationState = GET_CURRENT_COUNT;
				    lsDelay500msGetCountStart_cyw= 1;
				}
			}
			if(get_timego(wait_for_current_time)>300)
			{
				//
				// Reset request and response status
				//
				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
			    gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

				//
				// Change state
				//
				gInstallationState = GET_CURRENT_COUNT;
				lsDelay500msGetCountStart_cyw= 1;
			}
			break;
		}

		case VALIDATING_INSTALLATION:
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
				(1 == gstDriveBoardStatus.bits.driveInstallation)
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
				gInstallationState = VALIDATING_INSTALLATION;
			}
			else
			{
				//
				// Change state
				//
				gInstallationState = CHECK_FOR_INSTALLATION_SUCCESS;
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
				gInstallationState = CHECK_FOR_INSTALLATION_SUCCESS;

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

			if((lsDelay3secStartValidation == 1)&&(gInstallationState != CHECK_FOR_INSTALLATION_SUCCESS))
			{
			gInstallationState = KEY_PRESS_HANDLE;
			}
			break;
		}

		case CHECK_FOR_INSTALLATION_SUCCESS:
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
				gInstallationState = KEY_PRESS_HANDLE;
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
                        displayText("リミット＿セッテイ", 2, 0, false, false, false, false, false, false);
                        displayText("カンリョウ", 2, 16, false, false, false, false, false, false);
						}
						else
						{
						displayText("INSTALLATION", 2, 0, false, false, false, false,false,true);
						displayText("SUCCESSFUL", 2, 16, false, false, false, false,false,true);
						}                       // setting_flag = 0;

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
#if 0
						//
						// Clear screen
						//
						GrRectFIllBolymin(0, 126, 0, 63, true, true);

						//
						// Display commit installation message
						//
						displayText(" P R E S S   < E N T E R >   T o", 2, 0, false, false, false, false);
						displayText(" C O M M I T   I N S T A L L A T I O N", 2, 16, false, false, false, false);
						displayText(" A N D", 2, 32, false, false, false, false);
						displayText(" S T A R T   O P E R A T I O N", 2, 48, false, false, false, false);
#endif

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
						gInstallationState = WAIT_FOR_COMMIT_INSTALLATION;

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
					gInstallationState = WAIT_FOR_COMMIT_INSTALLATION;
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

                 displayText("リミット＿セッテイ", 2, 0, false, false, false, false, false, false);
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
				gInstallationState = INSTALLATION_FAILED;
				gTickCount3Seconds_cyw = g_ui32TickCount;
			}

			else if(gstDriveBoardStatus.bits.driveReady == 1)
			{
				//
				// Change state
				//
				gInstallationState = WAIT_FOR_COMMIT_INSTALLATION;
			}

			else
			{
				//
				// Change state
				//
				gInstallationState = KEY_PRESS_HANDLE;
			}

			break;
		}

		case INSTALLATION_FAILED:
		{
			if(gstDriveInstallation.bits.installA100 == 1)
			{
				//
				// Clear active error database
				//
				//memset(gsActiveAnomalyList, 0, sizeof(struct errorDB)*10);

				//
				// Change state
				//
				gInstallationState = DISPLAY_INSTALLATION_STATE;

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
					gInstallationState = DISPLAY_INSTALLATION_STATE;

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
			if(get_timego(gTickCount3Seconds_cyw)>300)
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
			gui8UpState = 0;
			gui8DownState = 0;
			gui8EnterState = 0;
			gui8OpenState = 0;
			gui8CloseState = 0;

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





			gInstallationState = CHECK_SHUTTER_STATE;


			//installationPaint();
			//psActiveFunctionalBlock = &gsInstallationFunctionalBlock;
			}
			break;
		}

		case WAIT_FOR_COMMIT_INSTALLATION:
		{
			//
			// Check whether Drive installation flag is reseted and drive ready
			// flag is set.
			//
			if( (gstDriveBoardStatus.bits.driveInstallation == 0) &&
				(gstDriveBoardStatus.bits.driveReady == 1)
			  )
			{
				bClearScreen = true;
				lsCommitInstallation = 0;

				//	Added to handle display screen hang issue at "INSTALLATION SUCCESSFUL" when a key is
				//	pressed during installation success - Dec 2015
				//	Clear key status flags before exiting the installation functional block
				gui8UpState = 0;
				gui8DownState = 0;
				gui8EnterState = 0;
				gui8OpenState = 0;
				gui8CloseState = 0;

				//
				// Clear screen
				//
				//GrRectFIllBolymin(0, 126, 0, 63, true, true);
				GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

				//
				// Set active functional block as home screen functional block
				//
				psActiveFunctionalBlock = &gsHomeScreenFunctionalBlock;

				//
				// Call home screen paint function
				//
				psActiveFunctionalBlock->pfnPaintFirstScreen();

				//
				// Reset installation state
				//
				gInstallationState = CHECK_SHUTTER_STATE;


				guest_reinit();
			}
			else
			{
				//
				// Change state
				//
				gInstallationState = KEY_PRESS_HANDLE;
			}

			break;
		}

		case DELAY_3_SECONDS:
		{
			//
			// Check for 3 seconds delay achieved. If yes, then change state.
			//
			if(get_timego ( gTickCount3Seconds) > 300 )
			{
				gTickCount3Seconds = 0;
				gInstallationState = gNextInstallState;
			}

			break;
		}

		case STATE_UNDEFINED:
		{

			break;
		}

		default:
		{

			break;
		}

	}

	//
	// Handle LEDs states during installation
	//
	installationLEDHandler();

	//
	// Display anomalies on fourth line of display.
	//
	if(  ( 	(gstDriveInstallation.bits.installA100 == 1)  ||
			(gstDriveInstallation.bits.installA101 == 1)  ||
			(gstDriveInstallation.bits.installA102 == 1)  ||
			(gstDriveInstallation.bits.installationValid) ||
			(gstDriveInstallation.bits.installationFailed)
		 ) && (1 == gstDriveBoardStatus.bits.driveInstallation)
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
		gInstallationState = STATE_UNDEFINED;
	}

	return 0;
}
/******************************************************************************
 * FunctionName: installationPaint
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
uint8_t installationPaint(void)
{
	//unsigned char lBuff[5];

	//
	// Clear screen
	//
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

	//displayText("A100", 2, 0, false, false, false, false);
	//displayText("UPPER LIMIT", 2, 16, false, false, false, false);

	if(gu8_language == Japanese_IDX)
	{
	displayText("タイキチュウ", 2, 0, false, false, false, false, false, false);
	displayText("リミット_セッテイ...", 1, 16, false, false, false, false, true, false);
	}
	else
	{
    displayText("WAITING FOR", 2, 0, false, false, false, false, false, true);
	displayText("INSTALLATION...", 2, 16, false, false, false, false, false, true);
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
	lsDelay500msGetCountStart_cyw = 1;


	return 0;
}

/******************************************************************************
 * FunctionName: installationMode
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
unsigned char lucNewShutterType;
uint8_t installationMode(void)
{


	//
	//	Added check to see whether system is in healthy state. If a fatal error has occurred then
	//	don't process operation keys - Jan 2016
	//
	if(
			(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
			(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)
	)
	{
		//
		// Handle Mode key press
		//
		if(gKeysStatus.bits.Key_Mode_pressed)
		{
			gKeysStatus.bits.Key_Mode_pressed = 0;

			//
			// Initiate 'Change Shutter Type  command
			//
			//if((gstDriveInstallation.bits.installA100 == 1) && (gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) && (gucShutterTypeChangedFlag == 0))
			if((gstDriveInstallation.bits.installA100 == 1)||(gstDriveInstallation.bits.installA101 == 1) ||(gstDriveInstallation.bits.installA102 == 1))  //add 20161017
			{

				if(psActiveFunctionalBlock == NULL)
							return 1;




						//gAnomalyIndex = 0;



						//
						// Set the active functional block as the child of currently active
						// functional block.
						//
						psActiveFunctionalBlock =&gsMenuFunctionalBlock;

						//
						// Set active menu as main menu.
						//
						psActiveMenu = &gsMainMenu;

						//
						// Call the first screen paint function of active functional block.
						//
						psActiveFunctionalBlock->pfnPaintFirstScreen();

			}

		}

		//
				// Handle Mode key release
				//
				//	It was observed that stop+open/stop+close was not working during installtion after
				//	initialize parameters or reset installation parameters. It was happening as mode release
				//	flag was not getting cleared. Added - Feb 2016
			    if(gKeysStatus.bits.Key_Mode_released)
			    {
			    	gKeysStatus.bits.Key_Mode_released = 0;
			    }


	}



	return 0;
}

/******************************************************************************
 * FunctionName: installationEnter
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
uint8_t installationEnter(void)
{
	//
	//	Added check to see whether system is in healthy state. If a fatal error has occurred then
	//	don't process operation keys - Jan 2016
	//
	if(
			(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
			(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)
	)
	{
		//
		// Handle Enter key press
		//
		if( (gKeysStatus.bits.Key_Enter_pressed) /*(gKeysStatus.bits.Key_3secEnter_pressed)*/ &&
				(0 == gui8OpenState) && (0 == gui8CloseState) && (0 == gui8UpState) && (0 == gui8DownState)
				&& (Flag_startInstallation==0)  //20170628   201703_No.CQ12
		)
		{
			setting_flag = 1;

			//
			// Initiate ENTER_PRESSED command
			//
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				gKeysStatus.bits.Key_Enter_pressed = 0; //Line uncommneted by YPG on 13 Jan to solve multiple "confirm substate installtion" command on "enter" press
				gui8EnterState = 1;
				gstUMtoCMoperational.commandToControlBoard.bits.enterPressed = 1;
				gstUMtoCMoperational.commandRequestStatus = eACTIVE;
			}
		}
		// Added below code to handle installation stage confirm using only 3PBS (no enter button)
		else if(
				(gKeysStatus.bits.Keys2_3secStpOpn_pressed && gstDriveInstallation.bits.installA100 == 1) ||
				(gKeysStatus.bits.Keys2_3secStpCls_pressed && gstDriveInstallation.bits.installA101 == 1)||
				(gKeysStatus.bits.Key_3secStp_pressed && gstDriveInstallation.bits.installA102 == 1)
		)
		{


			uartSendTxBuffer(UART_debug,"1",1);

			//
			// Initiate stop command
			//
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				gKeysStatus.bits.Keys2_3secStpOpn_pressed = 0;
				gKeysStatus.bits.Keys2_3secStpCls_pressed = 0;
				gKeysStatus.bits.Key_3secStp_pressed = 0;

				gstUMtoCMoperational.commandToControlBoard.bits.enterPressed = 1;
				gstUMtoCMoperational.commandRequestStatus = eACTIVE;

				//
				// Change state
				//
				gInstallationState = WAIT_FOR_KEY_PRESS_HANDLE_RESPONSE;

			}
		}


		if(gKeysStatus.bits.Key_Enter_released)
		{
			gKeysStatus.bits.Key_Enter_released = 0;
			gui8EnterState = 0;

			//
			// Initiate ENTER_PRESSED command
			//
			/*		if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		{
			gstUMtoCMoperational.commandToControlBoard.bits.enterReleased = 1;
			gstUMtoCMoperational.commandRequestStatus = eACTIVE;
		}*/
		}
	}/*if((gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
		(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0))*/

	//
	// Handle command response
	//
	if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
	{
		if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
		{
			if(gstUMtoCMoperational.commandToControlBoard.bits.enterPressed == 1)
			{
				gstUMtoCMoperational.commandToControlBoard.bits.enterPressed = 0;

				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				//20170414      201703_No.32  start
				if(gstDriveInstallation.bits.installA100 == 1)
				{

					gstUMtoLM_write.commandRequestStatus = eACTIVE;
					gstUMtoLM_write.commandResponseStatus = eNO_STATUS;
					gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory = 1;
					gstUMtoLM_write.changeSetting.newValue = gCurrentEncoderCount;
					gstUMtoLM_write.changeSetting.oldValue = A100_his_cyw;
					gstUMtoLM_write.changeSetting.parameterNumber =100;//gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex;
				    gstUMtoLM_write.changeSetting.timeStamp = (HWREG(0x400FC000));
					writeChangeSettingsHistory();
					A100_his_cyw = gCurrentEncoderCount;
				}
				if(gstDriveInstallation.bits.installA101 == 1)
				{

					gstUMtoLM_write.commandRequestStatus = eACTIVE;
					gstUMtoLM_write.commandResponseStatus = eNO_STATUS;
					gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory = 1;
					gstUMtoLM_write.changeSetting.newValue = gCurrentEncoderCount;
					gstUMtoLM_write.changeSetting.oldValue = A101_his_cyw;
					gstUMtoLM_write.changeSetting.parameterNumber =101;//gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex;
				    gstUMtoLM_write.changeSetting.timeStamp = (HWREG(0x400FC000));
					writeChangeSettingsHistory();
					A101_his_cyw = gCurrentEncoderCount;
				}
				if(gstDriveInstallation.bits.installA102 == 1)
				{

					gstUMtoLM_write.commandRequestStatus = eACTIVE;
					gstUMtoLM_write.commandResponseStatus = eNO_STATUS;
					gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory = 1;
					gstUMtoLM_write.changeSetting.newValue = gCurrentEncoderCount;
					gstUMtoLM_write.changeSetting.oldValue = A102_his_cyw;
					gstUMtoLM_write.changeSetting.parameterNumber =102;//gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex;
					gstUMtoLM_write.changeSetting.timeStamp = (HWREG(0x400FC000));
					writeChangeSettingsHistory();
					A102_his_cyw = gCurrentEncoderCount;
				}
				//20170414      201703_No.32 end
			}
			else if(gstUMtoCMoperational.commandToControlBoard.bits.enterReleased == 1)
			{
				gstUMtoCMoperational.commandToControlBoard.bits.enterReleased = 0;

				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
			}
		}

		else if( (gstUMtoCMoperational.commandResponseStatus == eTIME_OUT) ||
				 (gstUMtoCMoperational.commandResponseStatus == eFAIL)
				)
		{

			if(gstUMtoCMoperational.commandToControlBoard.bits.enterPressed == 1)
			{
				gstUMtoCMoperational.commandToControlBoard.bits.enterPressed = 0;

				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
			}
			else if(gstUMtoCMoperational.commandToControlBoard.bits.enterReleased == 1)
			{
				gstUMtoCMoperational.commandToControlBoard.bits.enterReleased = 0;

				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
			}
		}
	}

	return 0;
}

/******************************************************************************
 * FunctionName: installationUp
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
uint8_t installationUp(void)
{
	//
	//	Added check to see whether system is in healthy state. If a fatal error has occurred then
	//	don't process operation keys - Jan 2016
	//
	if(
			(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
			(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)
	)
	{
		//
		// Handle Up key press
		//
		if( (gKeysStatus.bits.Key_Up_pressed) &&
				(0 == gui8OpenState) && (0 == gui8CloseState) && (0 == gui8DownState) && (0 == gui8EnterState)
		)
		{
			//
			// Initiate UP_PRESSED command
			//
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				gKeysStatus.bits.Key_Up_pressed = 0;

				gui8UpState = 1;
				gstUMtoCMoperational.commandToControlBoard.bits.upPressed = 1;
				gstUMtoCMoperational.commandRequestStatus = eACTIVE;
			}
		}

		else if(gKeysStatus.bits.Key_Up_released)
		{
			//
			// Initiate UP_PRESSED command
			//
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				gKeysStatus.bits.Key_Up_released = 0;

				gui8UpState = 0;
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
 * FunctionName: installationDown
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
uint8_t installationDown(void)
{
	//
	//	Added check to see whether system is in healthy state. If a fatal error has occurred then
	//	don't process operation keys - Jan 2016
	//
	if(
			(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
			(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)
	)
	{
		//
		// Handle Down key press
		//
		if( (gKeysStatus.bits.Key_Down_pressed) &&
				(0 == gui8OpenState) && (0 == gui8CloseState) && (0 == gui8UpState) && (0 == gui8EnterState)
		)
		{
			//
			// Initiate DOWN_PRESSED command
			//
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				gKeysStatus.bits.Key_Down_pressed = 0;
				gui8DownState = 1;
				gstUMtoCMoperational.commandToControlBoard.bits.downPressed = 1;
				gstUMtoCMoperational.commandRequestStatus = eACTIVE;
			}
		}

		else if(gKeysStatus.bits.Key_Down_released)
		{
			//
			// Initiate DOWN_PRESSED command
			//
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				gKeysStatus.bits.Key_Down_released = 0;
				gui8DownState = 0;
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
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsInstallationFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	installationPaint,
	installationRunTime,
	installationUp,
	installationDown,
	installationMode,
	installationEnter
};
