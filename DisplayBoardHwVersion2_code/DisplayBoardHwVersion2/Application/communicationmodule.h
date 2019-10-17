/*********************************************************************************
* FileName: communicationmodule.h
* Description:
* This source file contains the prototype definition of all the services of ....
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
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       Initial Creation
****************************************************************************/


/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define STATUS_FAULT_POLL_TIMEOUT					50		// 500mS
#define MIN_COMMAND_RESPONSE_LENGTH					9
#define MAX_COMMAND_RESPONSE_LENGTH					9
#define MAX_CMD_SEND_ATTEMPTS_FOR_CONTROL			3//3    //20160806
#define MAX_CMD_SEND_ATTEMPTS_FOR_DRIVE				1

#define INVALID_COMMAND_ID				0x00

#define DRIVE_BOARD_ADDRESS			0x00
#define CONTROL_BOARD_ADDRESS		0x01
#define DISPLAY_BOARD_ADDRESS		0x02

#define ACK					0x06
#define NACK				0x15

#define DESTINATION_ADDRESS_POSITION		0
#define SOURCE_ADDRESS_POSITION				1
#define FRAME_LENGTH_POSITION			2
#define COMMAND_ID_POSITION				3
#define COMMAND_DATA_POSITION			4

#define RESPONSE_DATA_POSITION			3

#define NUMBER_OF_CRC_BYTES				2

//#define TxTIME_OUT			5		// 50 mSec
#define RxTIME_OUT				100//20//cyw modify 20160801 200ms//15		// 150 mSec @systick = 10mS (and considering 3 command attempts from control to drive
										// each of 25mS response timeout between control and drive)

/*********************************************
 * Commands from display board to control board
**********************************************/

#define AUTO_MAN_SELECT						0x13
#define RUN_CONTROL_BOARD					0x14
#define STOP_CONTROL_BOARD					0x15
#define UP_PRESSED							0x16
#define UP_RELEASED							0x17
#define DOWN_PRESSED						0x18
#define DOWN_RELEASED						0x19
#define OPEN_PRESSED						0x1a
#define OPEN_RELEASED						0x1b
#define CLOSE_PRESSED						0x1c
#define CLOSE_RELEASED						0x1d
#define STOP_PRESSED						0x1e
#define STOP_RELEASED						0x1f
#define ENTER_PRESSED						0x20
#define ENTER_RELEASED						0x21
#define GET_PARAMETER_CMD_FROM_DISPLAY		0x22
#define SET_PARAMETER_CMD_FROM_DISPLAY		0x23
#define SET_TIMESTAMP						0x24
#define FIRMWARE_UPGRADECMD_FROM_DISPLAY	0x25
#define START_INSTALLATION_CMD_FROM_DISPLAY	0x26
#define MONITOR_LED_CONTROL					0x27
#define GET_ERROR_LIST_CMD_FROM_DISPLAY		0x28
#define SYSTEM_INIT_COMPLETE				0x29

//	0x30, 0x31, 0x32 are used for firmware upgrade related commands

#define WIRELESS_MODE_CHANGE_PRESSED		0x33	//	Added on 04 Dec for new requirement from client
#define WIRELESS_MODE_CHANGE_RELEASED		0x34	//	Added on 04 Dec for new requirement from client

//	Added this commands to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
#define SETTINGS_MODE_STATUS				0x37
#define START_APERTUREHEIGHT_CMD_FROM_DISPLAY 0x38
#define ADD_LOGIN                           0X55

#define DRIVE_STATUS_PARAM_NO				605		// 0x025D
#define DRIVE_FAULT_PARAM_NO				607		// 0x025F
#define DRIVE_COMM_FAULT_PARAM_NO			608		// 0x0260
#define DRIVE_MOTOR_FAULT_PARAM_NO			609		// 0x0261
#define DRIVE_APPLN_FAULT_PARAM_NO			610		// 0x0262
#define DRIVE_PROCESSOR_FAULT_PARAM_NO		611		// 0x0263
#define DRIVE_INSTALTION_STATUS_PARAM_NO	606		// 0x025E

#define CONTROL_STATUS_PARAM_NO				612		// 0x0264
#define CONTROL_FAULT_PARAM_NO				638		// 0x027E
#define CONTROL_COMM_FAULT_PARAM_NO			613		// 0x0265
#define CONTROL_APPLN_FAULT_PARAM_NO		614		// 0x0266
#define CONTROL_PROCESSOR_FAULT_PARAM_NO	615		// 0x0267

#define PARAM_SHUTTER_STATUS				200
#define PARAM_SHUTTER_TYPE					537
#define PARAM_CTRL_HARDWARE_VER				451
#define PARAM_CTRL_FIRMWARE_VER				450
#define PARAM_DRV_HARDWARE_VER				550
#define PARAM_DRV_FIRMWARE_VER				549
#define PARAM_CURRENT_VALUE_MONITOR			129

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdint.h>

#include "interTaskCommunication.h"

/****************************************************************************
 *  Global variables:
****************************************************************************/
/*
	communication module inner task communication
*/
typedef struct stCommunicationModuleInnerTaskComm
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	enum destinationAddress destination;

	uint8_t commandType;

	uint8_t uchTxBuffer [TRANSMIT_BUFFER_SIZE];
	uint8_t uchTxBufferLen;

	union unCMDrCommand
	{
		uint32_t val;
		struct stCMDrCommand
		{
			uint32_t autoManualSelect				: 1;
			uint32_t runControlBoard				: 1;
			uint32_t stopControlBoard			  	: 1;
			uint32_t upPressed		 				: 1;
			uint32_t upReleased					 	: 1;
			uint32_t downPressed					: 1;
			uint32_t downReleased					: 1;
			uint32_t openPressed					: 1;
			uint32_t openReleased					: 1;
			uint32_t closePressed					: 1;
			uint32_t closeReleased					: 1;
			uint32_t stopPressed					: 1;
			uint32_t stopReleased					: 1;
			uint32_t enterPressed					: 1;
			uint32_t enterReleased					: 1;
			uint32_t getParameter					: 1;
			uint32_t setParameter					: 1;
			uint32_t setTimeStamp					: 1;
			uint32_t firmwareUpgrade				: 1;
			uint32_t startInstallation				: 1;
			uint32_t systemInitComplete				: 1;		// Added on 26 Aug for display control board sync
			uint32_t monitorLED_control				: 1;
			uint32_t getError						: 1;		//Get error from control board

			uint32_t modePressed					: 1;
			uint32_t modeReleased					: 1;
			uint32_t wirelessModeChangePressed		: 1;	//	Added on 04 Dec as per new requirement from client
			uint32_t wirelessModeChangeReleased		: 1;	//	Added on 04 Dec as per new requirement from client
			//	Added this command to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
			uint32_t settingsModeStatus				: 1;
			uint32_t recover_anomaly                : 1;
			uint32_t startApertureheight			: 1;
			uint32_t unused							: 3;
		} bits;
	}commandToControlBoard;

	uint8_t additionalCommandData;
	// for openShutterJog: = 10 for 10% Jog,  = 50 for 50% jog

	uint16_t parameterNumber;
	// for getParameter and setParameter commands

	uint32_t parameterValue;
	// for setParameter command

	struct stControlAnomaly errorFromControl;
	//Control anomaly

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;

	uint8_t uchRxBuffer [RECEIVE_BUFFER_SIZE];
	uint8_t uchRxBufferLen;

	//
	// Response ack section
	//
	enum responseAcknowledgement commandResponseACK_Status;
} _CommunicationModuleInnerTaskComm;



/******************************************************************************
 * initCommunicationModuleGlobalRegisters
 *
 * Function Description:
 * Initialize global registers being used for CMDr inner task communication.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void initCommunicationModuleGlobalRegisters(void);



/******************************************************************************
 * handleDriveCmdResp
 *
 * Function Description:
 * This function handles sending command to the drive board and receiving
 * command response from the drive board.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/


/******************************************************************************
 * handleControlCmdResp
 *
 * Function Description:
 * This function handles sending command to the control board and receiving
 * command response from the drive board.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void handleControlCmdResp(void);

/******************************************************************************
 * pollDriveControlStatusFault
 *
 * Function Description:
 * This function will Poll the Status Register of the Drive and Control
 * If the Installation and/or Fault bit in Status Register is Set, then function will poll the Sub Installation and Fault Register.
 * Function will use 'handleControlCmdResp' to send physical command and to process the reply
 * Function will store the reply in respective Global Intertask Communication Structure
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void pollDriveControlStatusFault(void);

/******************************************************************************
 * handleCommandFromUM_EM_EH
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 *
 * Function Returns: void
 *
 *
 ********************************************************************************/

void handleCommandFromUM_EM_EH(void);

/******************************************************************************
 * communicationModuleControlBoard
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 *
 * Function Returns: void
 *
 *
 ********************************************************************************/

void communicationModuleControlBoard (void);

/******************************************************************************
 * userModuleToTestCM
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 *
 * Function Returns: void
 *
 *
 ********************************************************************************/

void userModuleToTestCM(void);
