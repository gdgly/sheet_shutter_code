/*********************************************************************************
 * FileName: poweron.c
 * Description: This file defines power on state machine for display board.
 * Version: 0.1D
 *
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
 *  	0.1D	20/05/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "grlib/grlib.h"
//#include "Middleware/cfal96x64x16.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "Middleware/paramdatabase.h"
#include "userinterface.h"
#include "intertaskcommunication.h"
#include "communicationmodule.h"
#include "ledhandler.h"
#include "parameterlist.h"
#include "Middleware/serial.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
//#define TEST_COMM_MODULE
#define NORMAL_POWER_ON_SEQUENCE
#define ENABLE_SEND_TIMESTAMP

//#define SKIP_DRIVE_COMMANDS				//	Uncommented to skip Power ON Sequence
//#define TEMPORARY_CODE

#define TOTAL_SHUTTER_TYPES		3

#define HW_VER_GPIO_BASE	GPIO_PORTE_BASE

//**************************************************************************
// Hardware version pins
//**************************************************************************
#define PE2_HW_VER_4	GPIO_PIN_2
#define PE2_HW_VER_3	GPIO_PIN_3
#define PE2_HW_VER_2	GPIO_PIN_4
#define PE2_HW_VER_1	GPIO_PIN_5

#define ALL_HW_VER_PINS		( PE2_HW_VER_4 | PE2_HW_VER_3 | PE2_HW_VER_2 | PE2_HW_VER_1 )

//**************************************************************************
// Power on states
//**************************************************************************
#define GET_SHUTTER_TYPE					0
#define DISPLAY_SHUTTER_TYPE				1
#define DELAY_3_SECONDS						2
#define INSTALL_KEYPRESS_CHECK				3
#define START_INSTALLATION					4
#define WAIT_FOR_START_INSTALLATION			5
#define CONTROL_HARDWARE_VERSION			6
#define CONTROL_FIRMWARE_VERSION			7
#define PAINT_CONTROL_VERSION_INFO			8
#define DRIVE_HARDWARE_VERSION				9
#define DRIVE_FIRMWARE_VERSION				10
#define PAINT_DRIVE_VERSION_INFO			11
#define GET_PARAM							12
#define WAIT_FOR_GET_PARAM_RESPONSE			13
#define DISPLAY_BOARD_VERSION				14
#define SEND_TIMESTAMP						15
#define WAIT_FOR_TIMESTAMP_RESPONSE			16
#define CHECK_POWER_ON_CALLIBRATION			17
#define WAIT_FOR_CALLIBRATION_TO_OVER		18
#define CHECK_INSTALLATION_OR_HOMESCREEN	19
#define DELAY_COMM_FAIL						21
#define SEND_SYS_INIT_CMD					22
#define WAIT_FOR_SYS_INIT_CMD_RESPONSE		23
#define STATE_UNDEFINED						24
#define WAIT_FOR_SHUTTERTYPE                25


#define POWER_ON_SEQ_INTERSCREEN_DELAY		100


/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
extern volatile uint32_t g_ui32TickCount;
extern uint32_t gTickCount3Seconds;

#ifndef SKIP_DRIVE_COMMANDS
//uint8_t gUserModuleState = GET_SHUTTER_TYPE;
uint8_t gUserModuleState = 0xFF;
#endif

#ifdef SKIP_DRIVE_COMMANDS
uint8_t gUserModuleState = CHECK_POWER_ON_CALLIBRATION;
#endif

uint8_t gNextUMState = 0xFF;

uint32_t gControlHardwareVersion = 0;
uint32_t gControlFirmwareVersion = 0;
uint32_t gDriveHardwareVersion = 0;
uint32_t gDriveFirmwareVersion = 0;


//******************************************************************************
// HARDWARE VERSION FOR DISPLAY BOARD WILL READ FROM PORT PINS AND WILL UPDATE IN
// BELOW MENTIONED VARIABLE IN POWER ON SEQUENCE

uint32_t gDisplayHardwareVersion = 0;
//******************************************************************************


//******************************************************************************
// SET GOLLOWING VARIABLE TO SET THE DISPLAY BOARD FIRMWARE VERSION
// Format
// first byte = Not used, 		second byte = Not used,
// third byte = Major Version 	fourth byte = Minor version
//uint32_t gDisplayFirmwareVersion = 0x00000003;
uint32_t gDisplayFirmwareVersion = 0x00000000;

const uint8_t display_fw[][3]={{16,8,15}};
const uint8_t control_fw[][3]={{16,8,15}};
const uint8_t drive_fw[][3]={{16,8,15}};

/*******************************************************
Version 3 details
1)	Logic added to stop displaying error messages for
	some time using variable gucStopErrorsDisplay
2)	PowerFail error type changed to non-recoverable
3)	PowerFail error parameter index corrected
4)	Two new errors 40 and 41 added
5)	Total error count changed from 59 to 61
6)	Drive application error numbers corrected
	(from 34 to 39)
7) 	gucStopErrorsDisplay variable used to stop displaying
	error during parameter reset condition
8)	Updating operation count logic made configurable
	using following macros
	GET_OPERATION_COUNT_UPPER_APERTURE_LIMIT
	GET_OPERATION_COUNT_LOWER_LIMIT
9)	During installation, in case of shutter type change
	an additional message added as "parameter reset" for user
	information
10)	Parameter A500, A501 and A502 range changed for
	tuning purpose
********************************************************/
//******************************************************************************

uint32_t gShutterType = 0;

uint16_t gInputParam = 0;
uint32_t *pReceivedData = NULL;
void Set_lcdlightON(void);
/****************************************************************************/

/******************************************************************************
 * FunctionName: powerOnCalibrationLEDHandler
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns: void
 *
 ********************************************************************************/
void powerOnCalibrationLEDHandler(void)
{
//	static bool bFaultOccurred = false;
	static uint8_t lsFaultOccured = 0xFF;

	if(
		(   (1 != gstDriveBoardStatus.bits.driveFault) &&
			(1 != gstDriveBoardStatus.bits.driveFaultUnrecoverable) &&
			(1 != gstControlBoardStatus.bits.controlFault) &&
			(1 != gstControlBoardStatus.bits.controlFaultUnrecoverable) &&
			(1 != gstDisplayBoardStatus.bits.displayFault) &&
			(1 != gstDisplayBoardStatus.bits.displayFaultUnrecoverable)
		) //|| ( 0xFF == gstEMtoUM.faultLEDstatus )
	  )
	{
		//
		// Reset LED status to synchronize LEDs blinking since errors are removed.
		//
//		if(true == bFaultOccurred)
		if(1 == lsFaultOccured)
		{
			gstLEDcontrolRegister.autoManualLED = 0;
			gstLEDcontrolRegister.faultLED = 0;
			gstLEDcontrolRegister.powerLED = 1;

			//
			// Rest fault occurred flag
			//
//			bFaultOccurred = false;
			lsFaultOccured = 0;
		}
//		else
		else if( (0 == lsFaultOccured) && (1 == gstDriveBoardStatus.bits.drivePowerOnCalibration) )
		{
			gstLEDcontrolRegister.faultLED = _BLINK_STATUS_100_MSEC;
			gstLEDcontrolRegister.autoManualLED = _BLINK_STATUS_100_MSEC;
			gstLEDcontrolRegister.powerLED = _BLINK_STATUS_100_MSEC;

			lsFaultOccured = 0xFF;
		}

	}

	else if(
			( (1 == gstDriveBoardStatus.bits.driveFault) ||
			 (1 == gstDriveBoardStatus.bits.driveFaultUnrecoverable) ||
			 (1 == gstControlBoardStatus.bits.controlFault) ||
			 (1 == gstControlBoardStatus.bits.controlFaultUnrecoverable)||
			 (1 == gstDisplayBoardStatus.bits.displayFault) ||
			 (1 == gstDisplayBoardStatus.bits.displayFaultUnrecoverable)
			) //&& ( 0xFF != gstEMtoUM.faultLEDstatus )
		)
	{
		//
		// Update fault LED status
		//
		gstLEDcontrolRegister.faultLED = gstEMtoUM.faultLEDstatus;

		//
		// Set fault occurred flag
		//
//		bFaultOccurred = true;
		lsFaultOccured = 1;
	}
}


/******************************************************************************
 * FunctionName: powerOnRunTime
 *
 * Function Description:
 * This function is a state machine to monitor shutter system initialization process and
 * displays associated screens and messages. This function performs below mentioned jobs-
 * 1. Display shutter system model and type at power on.
 * 2. Retrieve and paint Display, control and drive board's firmware and hardware version
 * 3. Send time stamp to control board.
 * 4. Monitor power on calibration state.
 * 5. Direct control to either Home screen or installation as per the current state for drive.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t powerOnRunTime()
{
	//char lBuff[20];
	char lBuff[30]={0};

	uBoardVersion luBoardVer;
	Set_lcdlightON();
#ifdef	TEST_COMM_MODULE
	gUserModuleState = CHECK_INSTALLATION_OR_HOMESCREEN;
#endif

	switch(gUserModuleState)
	{

	case 0xFF :
	{
		//
		// Clear screen
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		//displayText(" B X   S H U T T E R S", 30, 0, false, false, false, false);
		displayText("BX SHUTTERS", 18, 0, false, false, false, false,true,false);
		GrLineDrawHorizontalBolymin(0, 126, 14, false);	// line draw function

		gUserModuleState = GET_SHUTTER_TYPE;

		break;
	}

	case GET_SHUTTER_TYPE :
	{
		//
		// Start GET_PARAMETER command for input parameter as Shutter type
		//
		if((gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) && (gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS))
		{

			gstUMtoCMdatabase.dataToControlBoard.parameterNumber =PARAM_SHUTTER_TYPE;
			gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 1;
			gstUMtoCMdatabase.destination = eDestDriveBoard;
			gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
			gUserModuleState = WAIT_FOR_SHUTTERTYPE;
		}
		//gInputParam = PARAM_SHUTTER_TYPE;		//	parameter number for Shutter_Type

		//pReceivedData = &gShutterType;
		//gUserModuleState = GET_PARAM;
		//gNextUMState = DISPLAY_SHUTTER_TYPE;

		//	Added on 07 Jul 14 to incorporate destination ID for getParam command
		//gstUMtoCMdatabase.destination = eDestDriveBoard;

		break;
	}
	case WAIT_FOR_SHUTTERTYPE:
		if(eACTIVE == gstUMtoCMdatabase.commandRequestStatus)
		{
			if(eSUCCESS == gstUMtoCMdatabase.commandResponseStatus)
			{
				if(eACK == gstUMtoCMdatabase.acknowledgementReceived)
				{

					if(gstUMtoCMdatabase.getParameterValue < TOTAL_SHUTTER_TYPES)
					gShutterType =  gstUMtoCMdatabase.getParameterValue;
					//
				    // Reset command bit
					//
					gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 0;

					//
					// Reset Request and response status and acknowledgment status
					//
					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
					gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;

					gUserModuleState = DISPLAY_SHUTTER_TYPE;
				}
			}
		}

	break;
	case DISPLAY_SHUTTER_TYPE:
	{
		//
		// Clear screen
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		//
		// Display first power on screen screen with shutter type received
		//
		//displayText(" B X   S H U T T E R S", 30, 0, false, false, false, false);
		displayText("BX SHUTTERS", 18, 0, false, false, false, false,true,false);
		GrLineDrawHorizontalBolymin(0, 126, 14, false);	// line draw function
		displayText("MODEL: DAIMAJIN", 2, 16, false, false, false, false, false, false);

		//
		// Display shutter type information on second line
		//
		if(gShutterType < TOTAL_SHUTTER_TYPES)
		{
			usnprintf(lBuff, sizeof(lBuff), "   %s", cucA537_SH_TYPE_States[gShutterType]);//OK

			displayText((unsigned char*)lBuff, 2, 32, false, false, false, false, false, false);
		}

		//
		// Change current and next state
		//
		gUserModuleState = DELAY_3_SECONDS;
		gNextUMState = INSTALL_KEYPRESS_CHECK;

		//
		// Capture time
		//
		gTickCount3Seconds = g_ui32TickCount;

		break;
	}

	case DELAY_3_SECONDS :
	{
		//
		// Check for 3 seconds delay achieved. If yes, then change state.
		//
		if(get_timego ( gTickCount3Seconds) >= POWER_ON_SEQ_INTERSCREEN_DELAY )
		//if(g_ui32TickCount == 300 + gTickCount3Seconds)
		{
			gTickCount3Seconds = 0;
			gUserModuleState = gNextUMState;
		}

		break;
	}

	case INSTALL_KEYPRESS_CHECK :
	{
		//
		// Check whether installation command is initiated using open + close + stop key press.
		//
		if(gKeysStatus.bits.Keys3_3secOpStCl_pressed == 1)
		{
			gKeysStatus.bits.Keys3_3secOpStCl_pressed = 0;

			//
			// Change state
			//
			gUserModuleState = START_INSTALLATION;

			break;
		}
		else
		{
			//
			// Change state
			//
			gUserModuleState = CONTROL_HARDWARE_VERSION;

			// Added on 30 April to remove power on sequence
			// gUserModuleState = SEND_TIMESTAMP;

			break;
		}
	}

	case START_INSTALLATION	:
	{
		//
		// Clear screen
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		//
		// Display installation initiated message
		//
		if(gu8_language == Japanese_IDX)
		{

		displayText("リミットセッテイヲ", 2, 0, false, false, false, false, false, false);
		displayText("スタートシマス......", 2, 16, false, false, false, false, false, false);
		}
		else
		{
		displayText("INSTALLATION", 2, 0, false, false, false, false,false,true);
				//displayText("INSTALLATION", 2, 0, false, false, false, false, false, false);
		displayText("INITIATED......", 2, 16, false, false, false, false, false, true);
		}
		//
		// Initiate Installation command and change state
		//
		if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		{
			gstUMtoCMoperational.commandToControlBoard.bits.startInstallation = 1;
			gstUMtoCMoperational.commandRequestStatus = eACTIVE;
			gUserModuleState = WAIT_FOR_START_INSTALLATION;

		}

		break;
	}

	case WAIT_FOR_START_INSTALLATION :
	{
		//
		// Check for installation command response and take respective action
		//
		if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
		{
			if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
			{
				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				//gstUMtoCMdatabase.commandToControlBoard.val = 0;
				gstUMtoCMoperational.commandToControlBoard.val = 0;

				//
				// Change current and next state
				//
				gUserModuleState = DELAY_3_SECONDS;
				gNextUMState = CONTROL_HARDWARE_VERSION;

				// Added on 30 April to remove power on sequence
				// gUserModuleState = SEND_TIMESTAMP;

				//
				// Capture Time
				//
				gTickCount3Seconds = g_ui32TickCount;
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
				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				gstUMtoCMdatabase.commandToControlBoard.val = 0;

//				//
//				// Clear first 3 lines on screen
//				//
//				GrRectFIllBolymin(0, 126, 0, 47, true, true);
//
//				//
//				// Display first power on screen screen with shutter type received
//				//
//				displayText(" B X   S H U T T E R S", 30, 0, false, false, false, false);
//				GrLineDrawHorizontalBolymin(0, 126, 14, false);	// line draw function

				//
				// Paint communication failure message
				//
//				displayText("COMMUNICATION", 2, 0, false, false, false, false);
//				displayText("FAILED", 2, 16, false, false, false, false);

				//
				// Change states
				//
				gUserModuleState = DELAY_3_SECONDS;
				gNextUMState = START_INSTALLATION;

				//
				// Capture Time
				//
				gTickCount3Seconds = g_ui32TickCount;
			}
		}

		break;
	}

	case CONTROL_HARDWARE_VERSION	:
	{
		//
		// Start GET_PARAMETER command for input parameter as Control hardware version
		//
		gInputParam = PARAM_CTRL_HARDWARE_VER;		//	parameter number for Control hardware version
		pReceivedData = &gControlHardwareVersion;
		gUserModuleState = GET_PARAM;
		gNextUMState = CONTROL_FIRMWARE_VERSION;

		//	Added on 07 Jul 14 to incorporate destination ID for getParam command
		gstUMtoCMdatabase.destination = eDestControlBoard;

		break;
	}

	case CONTROL_FIRMWARE_VERSION	:
	{
		//
		// Start GET_PARAMETER command for input parameter as Control firmware version
		//
		gInputParam = PARAM_CTRL_FIRMWARE_VER;		//	parameter number for Control firmware version
		pReceivedData = &gControlFirmwareVersion;
		gUserModuleState = GET_PARAM;
		gNextUMState = PAINT_CONTROL_VERSION_INFO;

		//	Added on 07 Jul 14 to incorporate destination ID for getParam command
		gstUMtoCMdatabase.destination = eDestControlBoard;

		break;
	}

	case PAINT_CONTROL_VERSION_INFO :
	{
		//
		// Clear first 3 lines on screen
		//
		//GrRectFIllBolymin(0, 126, 0, 47, true, true);
		GrRectFIllBolymin(0, 127, 0, 47, 0x00, true);

		//
		// Paint screen title
		//
		if(gu8_language == Japanese_IDX)
		displayText("Cユニット", 25, 0, false, false, false, false, false, false);
		else
		displayText("CONTROL BOARD", 25, 0, false, false, false, false,false,true);

		GrLineDrawHorizontalBolymin(0, 126, 14, false);	// line draw function

		//
		// Display control board hardware version
		//
		memset(&luBoardVer, 0, sizeof(uBoardVersion));
		luBoardVer.ui32VersionWord = gControlHardwareVersion;
		if(gu8_language == Japanese_IDX)
		{
		usnprintf(lBuff, sizeof(lBuff), "HWバージョン:%u", luBoardVer.ui8VersionBytes[0] );
		displayText((unsigned char*)lBuff, 2, 16, false, false, false, false, false, false);
		}
		else
		{
		usnprintf(lBuff, sizeof(lBuff), "HW VERSION: %u", gControlHardwareVersion);
		displayText((unsigned char*)lBuff, 2, 16, false, false, false, false, false, true);
		}


		//
		// Display control board firmware version
		//
		memset(&luBoardVer, 0, sizeof(uBoardVersion));
		luBoardVer.ui32VersionWord = gControlFirmwareVersion;
		//usnprintf(lBuff, sizeof(lBuff), "FW VERSION: %u", gControlFirmwareVersion);
		if(gu8_language == Japanese_IDX)
		{
		usnprintf(lBuff, sizeof(lBuff), "FWバージョン:%u%u%u.%u", control_fw[luBoardVer.ui8VersionBytes[1]][0],control_fw[luBoardVer.ui8VersionBytes[1]][1],control_fw[luBoardVer.ui8VersionBytes[1]][2],luBoardVer.ui8VersionBytes[0] );
		displayText((unsigned char*)lBuff, 2, 32, false, false, false, false, false, false);
		}
		else
		{
		usnprintf(lBuff, sizeof(lBuff), "FW VERSION: %u%u%u.%u", control_fw[luBoardVer.ui8VersionBytes[1]][0],control_fw[luBoardVer.ui8VersionBytes[1]][1],control_fw[luBoardVer.ui8VersionBytes[1]][2],luBoardVer.ui8VersionBytes[0] );
		displayText((unsigned char*)lBuff, 2, 32, false, false, false, false, false, true);
		}
	//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
		//memcpy(lBuff_cyw,lBuff,sizeof(lBuff));
	//	usnprintf_UU_cyw(lBuff_cyw,2,lBuff);
		//displayText((unsigned char*)lBuff, 2, 32, false, false, false, false);


		//
		// Change states
		//
		gUserModuleState = DELAY_3_SECONDS;
		gNextUMState = DISPLAY_BOARD_VERSION;

		//
		// Capture time for delay generation.
		//
		gTickCount3Seconds = g_ui32TickCount;

		break;
	}

	case DRIVE_HARDWARE_VERSION :
	{
		//
		// Start GET_PARAMETER command for input parameter as Drive Hardware version
		//
		gInputParam = PARAM_DRV_HARDWARE_VER;		//	parameter number for Drive Hardware version
		pReceivedData = &gDriveHardwareVersion;
		gUserModuleState = GET_PARAM;
		gNextUMState = DRIVE_FIRMWARE_VERSION;

		//	Added on 07 Jul 14 to incorporate destination ID for getParam command
		gstUMtoCMdatabase.destination = eDestDriveBoard;

		break;
	}

	case DRIVE_FIRMWARE_VERSION :
	{
		//
		// Start GET_PARAMETER command for input parameter as Drive firmware version
		//
		gInputParam = PARAM_DRV_FIRMWARE_VER;		//	parameter number for Drive firmware version
		pReceivedData = &gDriveFirmwareVersion;
		gUserModuleState = GET_PARAM;
		gNextUMState = PAINT_DRIVE_VERSION_INFO;

		//	Added on 07 Jul 14 to incorporate destination ID for getParam command
		gstUMtoCMdatabase.destination = eDestDriveBoard;

		break;
	}

	case PAINT_DRIVE_VERSION_INFO :
	{
		//
		// Clear first 3 lines on screen
		//
		//GrRectFIllBolymin(0, 126, 0, 47, true, true);
		GrRectFIllBolymin(0, 127, 0, 47, 0x00, true);

		//
		// Paint title
		//
		if(gu8_language == Japanese_IDX)
		displayText("Mユニット", 25, 0, false, false, false, false, false, false);
		else
		displayText("DRIVE BOARD", 31, 0, false, false, false, false,false,true);
		GrLineDrawHorizontalBolymin(0, 126, 14, false);	// line draw function

		//
		// Display drive board firmware and hardware version
		//
//		usnprintf(lBuff, sizeof(lBuff), "HW VERSION: %u", gDriveHardwareVersion);
//		displayText((unsigned char*)lBuff, 2, 16, false, false, false, false);
//		usnprintf(lBuff, sizeof(lBuff), "FW VERSION: %u", gDriveFirmwareVersion);
//		displayText((unsigned char*)lBuff, 2, 32, false, false, false, false);

		//
		// Display control board hardware version
		//
		memset(&luBoardVer, 0, sizeof(uBoardVersion));
		luBoardVer.ui32VersionWord = gDriveHardwareVersion;
		if(gu8_language == Japanese_IDX)
		{
		usnprintf(lBuff, sizeof(lBuff), "HWバージョン:%u", luBoardVer.ui8VersionBytes[0] );
		displayText((unsigned char*)lBuff, 2, 16, false, false, false, false, false, false);
		}
		else
		{
		usnprintf(lBuff, sizeof(lBuff), "HW VERSION: %u", luBoardVer.ui8VersionBytes[0] );
		displayText((unsigned char*)lBuff, 2, 16, false, false, false, false, false, true);
		}


		//
		// Display control board firmware version
		//
		memset(&luBoardVer, 0, sizeof(uBoardVersion));
		luBoardVer.ui32VersionWord = gDriveFirmwareVersion;
		if(gu8_language == Japanese_IDX)
		{
		usnprintf(lBuff, sizeof(lBuff), "FWバージョン:%u%u%u.%u", drive_fw[luBoardVer.ui8VersionBytes[1]][0], drive_fw[luBoardVer.ui8VersionBytes[1]][1], drive_fw[luBoardVer.ui8VersionBytes[1]][2],luBoardVer.ui8VersionBytes[0] );
		displayText((unsigned char*)lBuff, 2, 32, false, false, false, false, false, false);
		}
		else
		{
		usnprintf(lBuff, sizeof(lBuff), "FW VERSION: %u%u%u.%u", drive_fw[luBoardVer.ui8VersionBytes[1]][0], drive_fw[luBoardVer.ui8VersionBytes[1]][1], drive_fw[luBoardVer.ui8VersionBytes[1]][2],luBoardVer.ui8VersionBytes[0] );
		displayText((unsigned char*)lBuff, 2, 32, false, false, false, false, false, true);
		}
		//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//memcpy(lBuff_cyw,lBuff,sizeof(lBuff));
	//	usnprintf_UU_cyw(lBuff_cyw,5,lBuff);
		//displayText((unsigned char*)lBuff, 2, 32, false, false, false, false);

		//
		// Change states
		//
		gUserModuleState = DELAY_3_SECONDS;

#ifdef NORMAL_POWER_ON_SEQUENCE
		gNextUMState = CHECK_POWER_ON_CALLIBRATION;
#endif

#ifdef ENABLE_SEND_TIMESTAMP
		gNextUMState = SEND_TIMESTAMP;
#endif
		//
		// Capture time for delay generation
		//
		gTickCount3Seconds = g_ui32TickCount;

		break;
	}

	case GET_PARAM :
	{
		//
		// Initiate GET_PARAMETER command
		//
		if(gstUMtoCMdatabase.commandRequestStatus == eINACTIVE)
		{
			//
			// Select GET_PARAMETER command to initiate
			//
			gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 1;

			//
			// Supply input parameter number
			//
			gstUMtoCMdatabase.dataToControlBoard.parameterNumber = gInputParam;

			//
			// Set command request status active
			//
			gstUMtoCMdatabase.commandRequestStatus = eACTIVE;

			//
			// Change state
			//
			gUserModuleState = WAIT_FOR_GET_PARAM_RESPONSE;
		}

		break;
	}

	case WAIT_FOR_GET_PARAM_RESPONSE :
	{
		//
		// Check for response success or failure.
		//
		if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		{
			//
			// get received parameter value on success.
			//
			if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS)
			{
				*pReceivedData = gstUMtoCMdatabase.getParameterValue;
				pReceivedData = NULL;

				//
				// Reset request and response status
				//
				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
				gstUMtoCMdatabase.commandToControlBoard.val = 0;

				//
				// Change state
				//
				gUserModuleState = gNextUMState;
			}
			else if( (gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT) ||
					(gstUMtoCMdatabase.commandResponseStatus == eFAIL)
			)
			{
#if 0
				//
				// Set communication fault flag
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
				// Clear first 3 lines on screen
				//
				//GrRectFIllBolymin(0, 126, 0, 47, true, true);
				GrRectFIllBolymin(0, 127, 0, 47, 0x00, true);

				//
				// Display first power on screen screen with shutter type received
				//
				//displayText(" B X   S H U T T E R S", 30, 0, false, false, false, false);
				displayText("BX SHUTTERS", 18, 0, false, false, false, false, true, false);
				GrLineDrawHorizontalBolymin(0, 126, 14, false);	// line draw function

				//
				// Paint communication failure message
				//

//				displayText("COMMUNICATION", 2, 0, false, false, false, false);
//				displayText("FAILED", 2, 16, false, false, false, false);
//				displayText("PARAM NOT FOUND", 2, 48, false, false, false, false);

				//
				// Capture time
				//
				 gTickCount3Seconds = g_ui32TickCount;

				//
				// Change states
				//
				gUserModuleState = DELAY_COMM_FAIL;

				//
				// Reset request and response status
				//
				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
				gstUMtoCMdatabase.commandToControlBoard.val = 0;

			}
		}

		break;
	}

	case DISPLAY_BOARD_VERSION :
	{
		//
		// Get display board hardware and firmware version
		//

		//
		// Clear first 3 lines on screen
		//
		//GrRectFIllBolymin(0, 126, 0, 47, true, true);
		GrRectFIllBolymin(0, 127, 0, 47, 0x00, true);

		//
		// Paint Title
		//
		if(gu8_language == Japanese_IDX)
		displayText("Dユニット", 25, 0, false, false, false, false, false, false);
		else
		displayText("DISPLAY BOARD", 25, 0, false, false, false, false,false,true);

		GrLineDrawHorizontalBolymin(0, 126, 14, false);	// line draw function

		//
		// Paint display board version information on screen
		//
//		usnprintf(lBuff, sizeof(lBuff), "HW VERSION: %u", gu32_disp_hwver);
//		displayText((unsigned char*)lBuff, 2, 16, false, false, false, false);
//		usnprintf(lBuff, sizeof(lBuff), "FW VERSION: %u", gu32_disp_fwver);
//		displayText((unsigned char*)lBuff, 2, 32, false, false, false, false);

		gu32_disp_hwver = (0x0F)&(MAP_GPIOPinRead(HW_VER_GPIO_BASE, ALL_HW_VER_PINS));
		gDisplayHardwareVersion = gu32_disp_hwver;

		//
		// Display control board hardware version
		//
		memset(&luBoardVer, 0, sizeof(uBoardVersion));
		luBoardVer.ui32VersionWord = gu32_disp_hwver;
		if(gu8_language == Japanese_IDX)
		{
		usnprintf(lBuff, sizeof(lBuff), "HWバージョン:%u", luBoardVer.ui8VersionBytes[0] );
		displayText((unsigned char*)lBuff, 2, 16, false, false, false, false, false, false);
		}
		else
		{
		usnprintf(lBuff, sizeof(lBuff), "HW VERSION: %u", luBoardVer.ui8VersionBytes[0] );
		displayText((unsigned char*)lBuff, 2, 16, false, false, false, false, false, true);
		}


		//
		// Display control board firmware version
		//
		//gu32_disp_fwver = gu32_disp_fwver_DEF;
		memset(&luBoardVer, 0, sizeof(uBoardVersion));
		luBoardVer.ui32VersionWord = gu32_disp_fwver;
		//usnprintf(lBuff, sizeof(lBuff), "FW VERSION: 1.%u", luBoardVer.ui8VersionBytes[0] );
		if(gu8_language == Japanese_IDX)
		{
		usnprintf(lBuff, sizeof(lBuff), "FWバージョン:%u%u%u.%u",display_fw[((gDisplayFirmwareVersion >> 8) & 0x000000FF)][0],display_fw[((gDisplayFirmwareVersion >> 8) & 0x000000FF)][1],display_fw[((gDisplayFirmwareVersion >> 8) & 0x000000FF)][2], (gDisplayFirmwareVersion & 0x000000FF) );
		displayText((unsigned char*)lBuff, 2, 32, false, false, false, false, false, false);
		}
		else
		{
		usnprintf(lBuff, sizeof(lBuff), "FW VERSION: %u%u%u.%u",display_fw[((gDisplayFirmwareVersion >> 8) & 0x000000FF)][0],display_fw[((gDisplayFirmwareVersion >> 8) & 0x000000FF)][1],display_fw[((gDisplayFirmwareVersion >> 8) & 0x000000FF)][2], (gDisplayFirmwareVersion & 0x000000FF) );
		displayText((unsigned char*)lBuff, 2, 32, false, false, false, false, false, true);
		}
	//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//memcpy(lBuff_cyw,lBuff,sizeof(lBuff));
	//	usnprintf_UU_cyw(lBuff_cyw,2,lBuff);
				//displayText((unsigned char*)lBuff, 2, 32, false, false, false, false);

		//displayText((unsigned char*)lBuff, 2, 32, false, false, false, false);


		//
		// Change states
		//
		gUserModuleState = DELAY_3_SECONDS;
		gNextUMState = DRIVE_HARDWARE_VERSION;

		//
		// Capture time for delay generation
		//
		gTickCount3Seconds = g_ui32TickCount;

		break;
	}

	case SEND_TIMESTAMP	:
	{
		if(gstUMtoCMdatabase.commandRequestStatus == eINACTIVE)
		{
			//
			// Initiate SET_TIMESTAMP command
			//
			gstUMtoCMdatabase.commandToControlBoard.bits.setTimeStamp = 1;

			//
			// Set request status as active
			//
			gstUMtoCMdatabase.commandRequestStatus = eACTIVE;

			//
			// supply current time stamp
			//
			gstUMtoCMdatabase.dataToControlBoard.commandData.timeStamp = (HWREG(0x400FC000));

			//
			// Change state
			//
			gUserModuleState = WAIT_FOR_TIMESTAMP_RESPONSE;
		}

		break;
	}

	case WAIT_FOR_TIMESTAMP_RESPONSE :
	{
		if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		{
			//
			// Check for response success
			//
			if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS)
			{
				//
				// Reset request and response status
				//
				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

				//
				// Change state
				//
#if 1	// disabled as new state added for system initialization command
				gUserModuleState = CHECK_POWER_ON_CALLIBRATION;
#endif

#if 0	// move to the state for sending system init command
				gUserModuleState = SEND_SYS_INIT_CMD;
#endif
			}
			else if( (gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT) ||
					(gstUMtoCMdatabase.commandResponseStatus == eFAIL)
			)
			{
#if 0
				//
				// Set communication fault flag
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
				// Clear first 3 lines on screen
				//
				//GrRectFIllBolymin(0, 126, 0, 47, true, true);
				GrRectFIllBolymin(0, 127, 0, 47, 0x00, true);

				//
				// Display first power on screen screen with shutter type received
				//
				//displayText(" B X   S H U T T E R S", 30, 0, false, false, false, false);
				displayText("BX SHUTTERS", 18, 0, false, false, false, false, true, false);
				GrLineDrawHorizontalBolymin(0, 126, 14, false);	// line draw function

				//
				// Paint communication failure message
				//
//				displayText("COMMUNICATION", 2, 0, false, false, false, false);
//				displayText("FAILED", 2, 16, false, false, false, false);
//				displayText("TIMESTAMP NOT SET", 2, 48, false, false, false, false);

				//
				// Change states
				//
				gUserModuleState = DELAY_3_SECONDS;
				gNextUMState = SEND_TIMESTAMP;

				//
				// Capture time for delay generation
				//
				gTickCount3Seconds = g_ui32TickCount;
			}
		}

		break;
	}

	case SEND_SYS_INIT_CMD:
	{
		if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		{
			gstUMtoCMoperational.commandToControlBoard.bits.systemInitComplete = 1;
			gstUMtoCMoperational.commandRequestStatus = eACTIVE;

			gUserModuleState = WAIT_FOR_SYS_INIT_CMD_RESPONSE;
			//Set_lcdlightON();
		}

		break;
	}

	case WAIT_FOR_SYS_INIT_CMD_RESPONSE:
	{
		if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
		{
			if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
			{
				gstUMtoCMoperational.commandToControlBoard.val = 0;

				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;

				//
				// Change state
				//
				gUserModuleState = CHECK_POWER_ON_CALLIBRATION;

				// Added on 28 April to check Run mode or Installtion mode
				gUserModuleState = CHECK_INSTALLATION_OR_HOMESCREEN;

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
				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;

				//
				// Clear first 3 lines on screen
				//
				//GrRectFIllBolymin(0, 126, 0, 47, true, true);
				GrRectFIllBolymin(0, 127, 0, 47, 0x00, true);
				//
				// Display first power on screen screen with shutter type received
				//
				//displayText(" B X   S H U T T E R S", 30, 0, false, false, false, false);
				displayText("BX SHUTTERS", 18, 0, false, false, false, false,true,false);
				GrLineDrawHorizontalBolymin(0, 126, 14, false);	// line draw function

				//
				// Change states
				//
				gUserModuleState = DELAY_3_SECONDS;
				gNextUMState = SEND_SYS_INIT_CMD;

				//
				// Capture Time
				//
				gTickCount3Seconds = g_ui32TickCount;
			}
		}

		break;
	}

	case CHECK_POWER_ON_CALLIBRATION :
	{

		//
		// Check whether drive power on calibration flag is set
		//
		if(gstDriveBoardStatus.bits.drivePowerOnCalibration == 1)
		{

			//
			// Clear screen
			//
			//GrRectFIllBolymin(0, 126, 0, 63, true, true);
			GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
			//
			// Display shutter system initializing message
			//
			//displayText(" B X   S H U T T E R S", 30, 0, false, false, false, false);
			displayText("BX SHUTTERS", 18, 0, false, false, false, false,true,false);
			if(gu8_language == Japanese_IDX)
		//
			displayText("ティーチングモード", 2, 16, false, false, false, false, false, false);
			else
			displayText("TEACHING MODE", 2, 16, false, false, false, false, false, true);

			//
			// Change states
			//
			gUserModuleState = DELAY_3_SECONDS;
			gNextUMState = WAIT_FOR_CALLIBRATION_TO_OVER;

			//
			// Blink LEDs with 100 msec blink rate
			//
			gstLEDcontrolRegister.faultLED = _BLINK_STATUS_100_MSEC;
			gstLEDcontrolRegister.autoManualLED = _BLINK_STATUS_100_MSEC;
			gstLEDcontrolRegister.powerLED = _BLINK_STATUS_100_MSEC;

		}
		else
		{

			//
			// Clear screen
			//
			//GrRectFIllBolymin(0, 126, 0, 63, true, true);
			GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
			//
			// Display shutter system initializing message
			//
			//
			if(gu8_language == Japanese_IDX)
			displayText("シャッターシステム", 2, 0, false, false, false, false, false, false);
			else
			displayText("SHUTTER SYSTEM", 2, 0, false, false, false, false,false,true);

			//displayText("INITIALIAZING...", 2, 16, false, false, false, false);
			displayText("INITIALIAZING...", 1, 16, false, false, false, false,true ,false);
			//
			// change states
			//
			gUserModuleState = DELAY_3_SECONDS;
			gNextUMState = CHECK_INSTALLATION_OR_HOMESCREEN;

			// Added on 28 April to send System Init Command to control
			// move to the state for sending system init command
			gNextUMState = SEND_SYS_INIT_CMD;

		}

		//
		// Capture time for delay generation
		//
		gTickCount3Seconds = g_ui32TickCount;

		break;
	}

	case WAIT_FOR_CALLIBRATION_TO_OVER :
	{

		//
		// Handle operation key press events
		//
		operationKeysHandler();

		//
		// Check for drive calibration flag reset
		//
		if(gstDriveBoardStatus.bits.drivePowerOnCalibration == 0)
		{
			//
			// Set LEDs to default value
			//
			gstLEDcontrolRegister.faultLED = LED_OFF;
			gstLEDcontrolRegister.autoManualLED = LED_OFF;
			gstLEDcontrolRegister.powerLED = LED_ON;

			//
			// Change states
			//
			gUserModuleState = DELAY_3_SECONDS;
			gNextUMState = CHECK_INSTALLATION_OR_HOMESCREEN;

			// Added on 28 April to send System Init Command to control
			// move to the state for sending system init command
			gNextUMState = SEND_SYS_INIT_CMD;


		}
		/*else
		{

//			if( (1 == gstDriveBoardStatus.bits.driveFault) ||
//				(1 == gstDriveBoardStatus.bits.driveFaultUnrecoverable) ||
//				(1 == gstControlBoardStatus.bits.controlFault) ||
//				(1 == gstControlBoardStatus.bits.controlFaultUnrecoverable)
//			  )
//			{
//				gstLEDcontrolRegister.faultLED = gstEMtoUM.faultLEDstatus;
//			}
//			else if( (1 != gstDriveBoardStatus.bits.driveFault) &&
//				(1 != gstDriveBoardStatus.bits.driveFaultUnrecoverable) &&
//				(1 != gstControlBoardStatus.bits.controlFault) &&
//				(1 != gstControlBoardStatus.bits.controlFaultUnrecoverable)
//			)
//			{
//				gstLEDcontrolRegister.faultLED = _BLINK_STATUS_100_MSEC;
//			}

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
				gUserModuleState = STATE_UNDEFINED;
			}


		}*/

		// Logic to come out of teaching mode during power ON after staring the installation
		if (
				(gstUMtoCMoperational.commandToControlBoard.bits.startInstallation == 1) &&
				(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
		   )
		{

			if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
			{

				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				gstUMtoCMoperational.commandToControlBoard.val = 0;

			}

		}

		break;
	}

#ifdef	NORMAL_POWER_ON_SEQUENCE

	case CHECK_INSTALLATION_OR_HOMESCREEN :
	{

#ifdef TEMPORARY_CODE
		gstDriveBoardStatus.bits.driveReady = 1;
#endif
		//
		// Check whether shutter system is under installation
		//
		if(gstDriveBoardStatus.bits.driveInstallation == 1)
		{
			//
			// Set active functional block as installation functional block
			//
			psActiveFunctionalBlock = &gsInstallationFunctionalBlock;

			//
			// Paint active functional block first screen.
			//
			psActiveFunctionalBlock->pfnPaintFirstScreen();

			//
			// Reset states to initial value.
			//
			gUserModuleState = GET_SHUTTER_TYPE;

#ifdef SKIP_DRIVE_COMMANDS
			gUserModuleState = CHECK_POWER_ON_CALLIBRATION;
#endif
			gNextUMState = 0xFF;
		}
		else if(gstDriveBoardStatus.bits.driveReady == 1)
		{
			//
			// Set active functional block as home screen functional block.
			//
			psActiveFunctionalBlock = &gsHomeScreenFunctionalBlock;

			//
			// Paint active functional block first screen.
			//
			psActiveFunctionalBlock->pfnPaintFirstScreen();

			//
			// Reset states to initial value.
			//
			gUserModuleState = GET_SHUTTER_TYPE;

#ifdef SKIP_DRIVE_COMMANDS
			gUserModuleState = CHECK_POWER_ON_CALLIBRATION;
#endif

			gNextUMState = 0xFF;
		}
		else
		{
			gUserModuleState = CHECK_INSTALLATION_OR_HOMESCREEN;
			gNextUMState = 0xFF;
		}

		//			//
		//			// Paint active functional block first screen.
		//			//
		//			psActiveFunctionalBlock->pfnPaintFirstScreen();

		//			//
		//			// Reset states to initial value.
		//			//
		//			gUserModuleState = GET_SHUTTER_TYPE;
		//			gNextUMState = 0xFF;
		//	Added on 07 Jul 14 to incorporate destination ID for getParam command
		gstUMtoCMdatabase.destination = eDestControlBoard;
		gstUMtoCMdatabase.commandToControlBoard.val = 0;

		break;
	}
#endif

#ifdef TEST_COMM_MODULE

	case CHECK_INSTALLATION_OR_HOMESCREEN :
	{
		//
		// Check whether shutter system is under installation
		//
		/*if(gstDriveBoardStatus.bits.driveInstallation == 1)
			{
				//
				// Set active functional block as installation functional block
				//
				psActiveFunctionalBlock = &gsInstallationFunctionalBlock;
			}
			else
			{
				//
				// Set active functional block as home screen functional block.
				//
				psActiveFunctionalBlock = &gsHomeScreenFunctionalBlock;
			}*/


		//psActiveFunctionalBlock = &gsHomeScreenFunctionalBlock;
		psActiveFunctionalBlock = &gsInstallationFunctionalBlock;
		gstDriveBoardStatus.bits.driveInstallation = 1;
		//
		// Paint active functional block first screen.
		//
		psActiveFunctionalBlock->pfnPaintFirstScreen();

		//
		// Reset states to initial value.
		//
		gUserModuleState = GET_SHUTTER_TYPE;
		gNextUMState = 0xFF;

		break;
	}
#endif

	case DELAY_COMM_FAIL:
	{
		if( get_timego( gTickCount3Seconds) >= 300 )
		{
			gUserModuleState = GET_PARAM;
		}
		break;
	}

	//}


	}

	//
	// Display anomalies
	//
	displayAnomalies();

	//
	// Go to undefined state if fatal error occurs
	//
	if(	(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 1) ||
	    (gstControlBoardStatus.bits.controlFaultUnrecoverable == 1) ||
	    (gstDisplayBoardStatus.bits.displayFaultUnrecoverable == 1)
	  )
	{
		//
		// Change state
		//
		gUserModuleState = STATE_UNDEFINED;
	}

	powerOnCalibrationLEDHandler();

	return 0;
}

/******************************************************************************
 * FunctionName: powerOnEnter
 *
 * Function Description:
 * This is a default function for enter key press during power on.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t powerOnEnter(void)
{
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		//gKeysStatus.bits.Key_Enter_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: powerOnUp
 *
 * Function Description:
 * This is a default function for up key press during power on.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t powerOnUp(void)
{
	if(gKeysStatus.bits.Key_Up_pressed)
	{
		gKeysStatus.bits.Key_Up_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: powerOnDown
 *
 * Function Description:
 * This is a default function for down key press during power on.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t powerOnDown(void)
{
	if(gKeysStatus.bits.Key_Down_pressed)
	{
		gKeysStatus.bits.Key_Down_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: powerOnMode
 *
 * Function Description:
 * This is a default function for mode key press during power on.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t powerOnMode(void)
{
	// Logic to come out of teaching mode during power ON after staring the installation
	if((gstDriveBoardStatus.bits.drivePowerOnCalibration == 1) && (gKeysStatus.bits.Key_Mode_pressed))
	{

		if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		{
			gstUMtoCMoperational.commandToControlBoard.bits.startInstallation = 1;
			gstUMtoCMoperational.commandRequestStatus = eACTIVE;
		}

	}
	else if(gKeysStatus.bits.Key_Mode_pressed)
	{
		gKeysStatus.bits.Key_Mode_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: powerOnPaint
 *
 * Function Description:
 * This is a default function. It does nothing.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t powerOnPaint(void)
{
	// Do nothing.
	return 0;
}

stInternalFunctions gsPowerOnFunctionalBlock =
{
	0,
	0,
	powerOnPaint,
	powerOnRunTime,
	powerOnUp,
	powerOnDown,
	powerOnMode,
	powerOnEnter
};
