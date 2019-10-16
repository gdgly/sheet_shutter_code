/*********************************************************************************
* FileName: xyz.h
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
/*********************************************
 * Commands from drive board to control board
**********************************************/

#define RESTART_DRIVE_COMMUNICATION		0x12
#define MIN_COMMAND_LENGTH		6
#define MAX_COMMAND_LENGTH		12

//#define TxTIME_OUT			100	// 100 mSec
#define COMMAND_RxTIME_OUT			25	// 25 mSec @ systick incrementing at 1mS
//#define ONE_BYTE_RECEIVE_TIME	2	// 2ms

/*********************************************
 * Commands from display board to control board
**********************************************/

#define AUTO_MAN_SELECT					0x13
#define RUN_CONTROL_BOARD				0x14
#define STOP_CONTROL_BOARD				0x15
#define UP_PRESSED						0x16
#define UP_RELEASED						0x17
#define DOWN_PRESSED					0x18
#define DOWN_RELEASED					0x19
#define OPEN_PRESSED					0x1a
#define OPEN_RELEASED					0x1b
#define CLOSE_PRESSED					0x1c
#define CLOSE_RELEASED					0x1d
#define STOP_PRESSED					0x1e
#define STOP_RELEASED					0x1f
#define ENTER_PRESSED					0x20
#define ENTER_RELEASED					0x21
#define GET_PARAMETER_CMD_FROM_DISPLAY	0x22
#define SET_PARAMETER_CMD_FROM_DISPLAY	0x23
#define SET_TIMESTAMP					0x24
#define FIRMWARE_UPGRADECMD_FROM_DISPLAY	0x25
#define START_INSTALLATION_CMD_FROM_DISPLAY				0x26
#define MONITOR_LED_CONTROL				0x27
#define GET_ERROR_LIST_CMD_FROM_DISPLAY	0x28
#define SYSTEM_INIT_COMPLETE				0x29

//	0x30, 0x31, 0x32 are used for firmware upgrade related commands

#define WIRELESS_MODE_CHANGE_PRESSED		0x33	//	Added on 04 Dec for new requirement from client
#define WIRELESS_MODE_CHANGE_RELEASED		0x34	//	Added on 04 Dec for new requirement from client

//	0x35, 0x36 are used as START_POWER_ON_CALIBRATION and STOP_POWER_ON_CALIBRATION

//	Added this commands to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
#define SETTINGS_MODE_STATUS				0x37
#define ADD_LOGIN                           0X55
#define NEW_LOGIN                           0X56

#define DRIVE_STATUS_PARAM_NO	605
#define DRIVE_FAULT_PARAM_NO	607
#define DRIVE_COMM_FAULT_PARAM_NO	608
#define DRIVE_MOTOR_FAULT_PARAM_NO	609
#define DRIVE_APPLN_FAULT_PARAM_NO	610
#define DRIVE_PROCESSOR_FAULT_PARAM_NO	611
#define DRIVE_INSTALTION_STATUS_PARAM_NO	606

#define CONTROL_STATUS_PARAM_NO	612
#define CONTROL_FAULT_PARAM_NO	638
#define CONTROL_COMM_FAULT_PARAM_NO	612
#define CONTROL_APPLN_FAULT_PARAM_NO	614
#define CONTROL_PROCESSOR_FAULT_PARAM_NO	615

#define CONTROL_BOARD_FIRMWARE_VERSION		450

/****************************************************************************/


/****************************************************************************
 *  Global variables:
****************************************************************************/


/****************************************************************************/


/******************************************************************************
 * communicationModuleDisplayToTestCMDr
 *
 * Function Description:
 * This function is temporarily created to test CMDr. It clears the CMdi to CMDr
 * command being activated in main.c file for functional testing.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void communicationModuleDisplayToTestCMDr(void);

/******************************************************************************
 * communicationModuleDisplay
 *
 * Function Description:
 * This function receives commands from the Display board and sends response
 * to the display
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void communicationModuleDisplay(void);
