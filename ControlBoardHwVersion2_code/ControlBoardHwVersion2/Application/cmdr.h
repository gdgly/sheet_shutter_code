/*********************************************************************************
* FileName: CMDr.h
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
 *  	0.2D	03/07/2014			iGATE Offshore team			Macros added for Parameter Numbers
 *  	0.1D	21/05/2014			iGATE Offshore team       	Initial Creation
****************************************************************************/


/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define STATUS_FAULT_POLL_TIMEOUT		300		// 500 ms
#define MIN_COMMAND_RESPONSE_LENGTH		9
#define MAX_COMMAND_RESPONSE_LENGTH		9

//#define TxTIME_OUT			100	// 100 mSec
#define COMMAND_RESPONSE_RxTIME_OUT			30	// 30 mSec @ systick incrementing at 1mS

#define RAW		0
#define DIRECT	1


#define INVALID_COMMAND_ID				0x00
#define MAX_CMD_SEND_ATTEMPTS			3

/*********************************************
 * Commands from control board to drive board
**********************************************/

#define RUN_DRIVE						0x01
#define STOP_DRIVE						0x02
#define STOPPING_DRIVE_COMMUNICATION  	0x03
#define START_INSTALLATION_CMD_FROM_CONTROL 				0x04
#define CONFIRM_SUBSTATE_INSTALLATION 	0x05
#define OPEN_SHUTTER					0x06
#define OPEN_SHUTTER_JOG				0x07
#define OPEN_SHUTTER_APERTURE			0x08
#define CLOSE_SHUTTER					0x09
#define CLOSE_SHUTTER_JOG				0x0a
#define CLOSE_SHUTTER_APERTURE			0x0b
#define CLOSE_SHUTTER_IGNORE_SENSOR		0x0c
#define STOP_SHUTTER					0x0d
#define GET_PARAMETER_CMD_FROM_CONTROL	0x0e
#define SET_PARAMETER_CMD_FROM_CONTROL	0x0f
#define FIRMWARE_UPGRADE_CMD_FROM_CONTROL	0x10
#define GET_ERROR_LIST_CMD_FROM_CONTROL	0x11

#define START_POWER_ON_CALIBRATION		0x35	//	Added on 30 Jan 2015 to implement power on calibration control
#define STOP_POWER_ON_CALIBRATION		0x36	//	Added on 30 Jan 2015 to implement power on calibration control
#define RECOVER_ANMOLY                  0X38
#define START_APERTUREHEIGHT_CMD_FROM_CONTROL 				0x39
/*********************************************
 * Drive board parameter numbers
**********************************************/
#define OPERATION_COUNT						600		// 0x0258
#define DRIVE_STATUS						605		// 0x025D
#define	DRIVE_INSTALLATION_STATUS			606		// 0x025E
#define	DRIVE_FAULT_STATUS					607		// 0x025F
#define	DRIVE_COMMUNICATON_FAULT_STATUS		608		// 0x0260
#define	DRIVE_MOTOR_FAULT_STATUS			609		// 0x0261
#define	DRIVE_APPLICATION_FAULT_STATUS		610		// 0x0262
#define	DRIVE_PROCESSOR_FAULT_STATUS		611		// 0x0263


#define SNOW_MODE_PARAMETER					8		// 0x224	//	Added to implement setParameter command for setting snow mode, on 10APR2015

/****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/

#include "interTaskCommunication.h"

/****************************************************************************/
/* Indexes of Drive EEPROM parameters */
typedef enum _DriveEEPROMData
{
	A100_UPPER_STOPPING_POS = 0,						// 0
	A101_LOWER_STOPPING_POS,							// 1
	A102_PHOTO_ELEC_POS,								// 2
	A128_ORIGIN_SENS_POS,								// 3
	A637_ORIGIN_SENS_DRIFT,								// 4
	A129_CURRENT_VAL_MONITOR,							// 5
	A130_APERTURE_HEIGHT_POS,							// 6
	A131_APERTURE_MODE_ENABLE, // common block end		// 7
	A008_SNOW_MODE_PHOTOELEC,							// 8
	A021_INITIAL_VAL_SETTING,							// 9
	A025_MAINTENANCE_COUNT_LIMIT,						// 10
	A636_MAINTENANCE_COUNT_VALUE,						// 11
	A080_MICRO_SENS_COUNT,								// 12
	A081_MICRO_SENS_COUNT_RESET,						// 13
	A112_OVERRUN_PROTECT,							// 14
	A120_RESET_TO_DEFAULT,						// 15
	A125_POWERUP_CALIB,						// 16
	A126_CORRECTED_FREQ_APERTURE,						// 17
	A127_AUTO_CORRECT_ENABLED,						// 18
	A549_DRIVE_FW_VER,						// 19
	A550_DRIVE_HW_VER,						// 20
	A600_OPERATION_COUNT,						// 21
	A603_MICRO_SENS_LIM_VALUE,						// 22
	A604_APERTURE_HEIGHT_OPER_COUNT, // appl block end						// 23
	A605_DRIVE_BOARD_STATUS,						// 24
	A606_DRIVE_INSTALLATION,						// 25
	A607_DRIVE_FAULT,						// 26
	A608_DRIVE_BOARD_COMM_FAULT,						// 27
	A609_DRIVE_MOTOR_FAULT,						// 28
	A610_DRIVE_APPL_FAULT,						// 29
	A611_DRIVE_PROCESSOR_FAULT,						// 30
	A616_ANOMALY_HISTORY_1,						// 31
	A617_ANOMALY_HISTORY_2,						// 32
	A618_ANOMALY_HISTORY_3,						// 33
	A619_ANOMALY_HISTORY_4,						// 34
	A620_ANOMALY_HISTORY_5,						// 35
	A621_ANOMALY_HISTORY_6,						// 36
	A622_ANOMALY_HISTORY_7,						// 37
	A623_ANOMALY_HISTORY_8,						// 38
	A624_ANOMALY_HISTORY_9,						// 39
	A625_ANOMALY_HISTORY_10, // appl status block end						// 40
	A011_DECEL_BY_PHOTO_ELEC_BLOCKING_LIM,						// 41
	A011_WAIT_FOR_STOPPAGE,						// 42
	A103_RISE_CHANGE_GEAR_POS1,						// 43
	A104_RISE_CHANGE_GEAR_POS2,						// 44
	A105_RISE_CHANGE_GEAR_POS3,						// 45
	A106_FALL_CHANGE_GEAR_POS1,						// 46
	A107_FALL_CHANGE_GEAR_POS2,						// 47
	A108_FALL_CHANGE_GEAR_POS3,						// 48
	A110_SHUTTER_REVERSE_OP_MIN_LIM,						// 49
	A500_PWM_FREQ_MOTOR_CTRL,						// 50
	A501_STARTUP_DUTY_CYCLE,						// 51
	A504_MAX_STARTUP_TIME_LIM,						// 52
	A505_STARTUP_SECTOR_CONST,						// 53
	A506_timer2_pre_scalar,						// 54
	A507_timer2_to_rpm,						// 55
	A508_timer2_min,						// 56
	A509_timer2_max,						// 57
	A510_min_duty_cycle,						// 58
	A511_break_enabled,						// 59
	A512_speed_PI_prop_gain,						// 60
	A513_speed_PI_integral_gain,						// 61
	A514_current_PI_prop_gain,						// 62
	A515_current_PI_integral_gain,						// 63
	A516_output_freq,						// 64
	A517_inch_speed,						// 65
	A518_drive_status,						// 66
	A519_current_error,						// 67
	A520_acceleration1_up,						// 68
	A521_deceleration1_up,						// 69
	A522_s1_up,						// 70
	A523_s2_up,						// 71
	A524_s3_up,						// 72
	A525_up_step_count,						// 73
	A526_acceleration1_down,						// 74
	A527_deceleration1_down,						// 75
	A528_s1_down,						// 76
	A529_s2_down,						// 77
	A530_s3_down,						// 78
	A531_down_step_count,						// 79
	A536_shutter_length,						// 80
	A537_shutter_type,						// 81
	A538_OV_limit,						// 82
	A539_OI_limit,						// 83
	A540_OS_limit,						// 84
	A541_OF_limit,						// 85
	A542_thermal_protection,						// 86
	A543_torque_const,						// 87
	A544_backEMF_const,						// 88
	A545_speed_const,						// 89
	A546_rated_speed,						// 90
	A547_rated_current,						// 91
	A548_pole_pairs, // motor block end						// 92
	number_of_drive_parameters						// 93
}enDriveEEPROMData;
/****************************************************************************
 *  Global variables
****************************************************************************/
/*
	CMDr inner task communication
*/
typedef struct stCMDrInnerTaskComm
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	uint8_t commandType;

	uint8_t uchTxBuffer [TRANSMIT_BUFFER_SIZE];
	uint8_t uchTxBufferLen;

	union unCMDrCommand
	{
		uint32_t val;
		struct stCMDrCommand
		{
			uint32_t runDrive						: 1;
			uint32_t stopDrive						: 1;
			uint32_t stoppingDriveCommunication  	: 1;
			uint32_t startInstallation 				: 1;
			uint32_t confirmSubstateInstallation 	: 1;
			uint32_t openShutter					: 1;
			uint32_t openShutterJog					: 1;
			uint32_t openShutterApperture			: 1;
			uint32_t closeShutter					: 1;
			uint32_t closeShutterJog				: 1;
			uint32_t closeShutterApperture			: 1;
			uint32_t closeShutterIgnoreSensor		: 1;
			uint32_t stopShutter					: 1;
			uint32_t getParameter					: 1;
			uint32_t setParameter					: 1;
			uint32_t getError						: 1;		//Get error from drive board
			uint32_t startPowerOnCalibration		: 1;
			uint32_t stopPowerOnCalibration			: 1;
			uint32_t recoveranmoly                  : 1;
			uint32_t start_apertureHeight 			: 1;
			uint32_t unused							: 13;
		} bits;
	}commandToDriveBoard;

	uint8_t additionalCommandData;
	// for openShutterJog: = 10 for 10% Jog,  = 50 for 50% jog

	uint16_t parameterNumber;
	// for getParameter and setParameter commands

	uint32_t parameterValue;
	// for setParameter command

	struct stDriveAnomaly errorFromDrive;
	// Drive anomaly

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
} _CMDrInnerTaskComm;


/****************************************************************************/


/******************************************************************************
 * initCMDrGlobalRegisters
 *
 * Function Description:
 * Initialize global registers being used for CMDr inner task communication.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void initCMDrGlobalRegisters(void);


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
void handleControlCmdResp(void);

/******************************************************************************
 * pollDriveStatusFault
 *
 * Function Description:
 * This function will Poll the Status Register of the Drive
 * If the Installation and/or Fault bit in Status Register is Set, then function will poll the Sub Installation and Fault Register.
 * Function will use 'handleDriveCmdResp' to send physical command and to process the reply
 * Function will store the reply in respective Global Intertask Communication Structure
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void pollDriveStatusFault(void);

/******************************************************************************
 * handleCommandFromLS_EM_CMDi
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
void handleCommandFromLS_EM_CMDi(void);

/******************************************************************************
 * communicationModuleDrive
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
void communicationModuleDrive (void);
