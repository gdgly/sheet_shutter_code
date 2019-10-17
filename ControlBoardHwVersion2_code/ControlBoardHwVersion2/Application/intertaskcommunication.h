/*********************************************************************************
 * FileName: interTaskCommunication.h
 * Description:
 * This source file contains the definition of all the global variables
 * Version: 0.1D
 *
 *
 **********************************************************************************/
#ifndef __INTERTASKCOMMUNICATION_H__
#define __INTERTASKCOMMUNICATION_H__
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
 *  	0.2D	14/07/2014									Drive Status bits sequence change as per Drive Code structure
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Include:
 ****************************************************************************/
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include "Middleware/serial.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/


/****************************************************************************/

/****************************************************************************
 * Local variables for this file
****************************************************************************/

union ui16TempData
{
	uint8_t byte[2];
	struct sixteenBits
	{
		uint16_t val;
	}halfWord;
};

union ui32TempData
{
	uint8_t byte[4];
	struct thirtyTwoBits
	{
		uint32_t val;
	}word;
};
/****************************************************************************/

/****************************************************************************
 *  Structure for other files:
 ****************************************************************************/
typedef struct stMonitorLED
{
	uint8_t monitorLEDstatus;
	/*
	 * for monitor LED on, off, blink rate or flash pattern
				Monitor LED status register
				0bXXXX 0000 �OFF
				0bXXXX 0001 �ON
				0b0001 0010 �1 FLASH
				0b0010 0010 �1 FLASH		//	2flashes   3 secOFF ->0.5secON->0.5secOFF->0.5secON->3secOFF->repeat
				.
				.
				.
				0b1000 0010 �8 FLASH
				0b0001 0011 �BLINK at 0.05 sec
				0b0010 0011 �BLINK at 0.1 sec
				0b0011 0011 �BLINK at 0.15 sec
				0b0100 0011 �BLINK at 0.2 sec
				0b0101 0011 �BLINK at 0.25 sec
				0b0110 0011 �BLINK at 0.3 sec
				0b0111 0011 �BLINK at 0.35 sec
				0b1000 0011 �BLINK at 0.4 sec
				0b1001 0011 �BLINK at 0.45 sec
				0b1010 0011 �BLINK at 0.5 sec
	 */
}_MonitorLED;

typedef struct stElectroMechanicalCounter
{
	uint8_t value;
}_ElectroMechanicalCounter;

enum requestStatus
{
	eINACTIVE,
	eACTIVE
};
enum responseStatus
{
	eNO_STATUS,
	eWAITING,
	eSUCCESS,
	eTIME_OUT,
	eCMD_SEND_FAIL,
	eFAIL
};
enum acknowledgment
{
	eNO_ACK,
	eACK,
	eNACK
};
enum responseAcknowledgement
{
	eNO_StatusAcknowledgement,
	eResponseAcknowledgement_ACK,
	eResponseAcknowledgement_NACK,
	eResponseAcknowledgementProcessed
};

struct stDriveAnomaly
{
	uint16_t anomalyCode;
	uint8_t errorDetails[15];
};

struct stControlAnomaly
{
	time_t timeStamp;
	uint16_t anomalyCode;
	uint8_t errorDetails[15];
};


/*
	Micro-controller error flags
*/
enum controllerErrors
{
	eSystemFault,
	eMemoryMgmtFault,
	eBusFault,
	eUsageFault,
	eWatchdog,
};

/*
	Drive board status and fault registers
*/
typedef union unDriveStatus
{
	uint32_t val;
	struct stDriveStatus
	{
		uint32_t driveReady					: 1;
		uint32_t drivePowerOnCalibration	: 1;
		uint32_t driveRunTimeCalibration	: 1;
		uint32_t driveInstallation			: 1;

		uint32_t driveFault					: 1;
		uint32_t driveFaultUnrecoverable	: 1;
		uint32_t shutterUpperLimit			: 1;
		uint32_t shutterApertureHeight		: 1;

		uint32_t shutterLowerLimit			: 1;
		uint32_t upDecelStartReached		: 1;	// Added to match updated Drive status bits on 13 Aug 14
		uint32_t ignPhotoElecSensLimRchd	: 1;	// Added to match updated Drive status bits on 13 Aug 14
		uint32_t shutterStopped				: 1;

		uint32_t shutterMovingUp			: 1;
		uint32_t shutterMovingDown			: 1;
		uint32_t shutterMovDwnIgnrSensr		: 1;
		uint32_t originSensorStatus			: 1;

		uint32_t microSwitchSensorStatus	: 1;
		uint32_t peSensorStatus				: 1;
		uint32_t driveBoardBootloader		: 1;
		uint32_t shutterBetweenUplmtAphgt	: 1;	//STT  shutter position between upperlimit and ApertureHeight
		uint32_t shutterBetweenLowlmtAphgt	: 1;	//STT  shutter position between lowerlimit and ApertureHeight
		uint32_t drive_apertureHeight			: 1;
		uint32_t unused						: 10;
	} bits;
} _DriveStatus;


typedef union unDriveFault
{
	uint8_t val;
	struct stDriveFault
	{
		uint8_t driveCommunication	: 1;
		uint8_t driveMotor			: 1;
		uint8_t driveApplication	: 1;
		uint8_t driveProcessor		: 1;

		uint8_t unused				: 1;
	} bits;
} _DriveFault;

typedef union unDriveCommunicationFault
{
	uint8_t val;
	struct stDriveCommunicationFault
	{
		uint8_t crcError			: 1;
		uint8_t uartError			: 1;
		uint8_t commandFrameError	: 1;	// Added to match updated Drive status bits on 13 Aug 14
		uint8_t unused				: 5;
	} bits;
} _DriveCommunicationFault;

typedef union unDriveMotorFault
{
	uint16_t val;
	struct stDriveMotorFault
	{
		uint16_t motorOpenphase			: 1;
//		uint16_t motorOverspeed			: 1;	// Removed to match updated Drive motor fault bits on 13 Aug 14
		uint16_t motorDCbusOverVoltage	: 1;
		uint16_t motorOverCurrent		: 1;
		uint16_t motorExceedingTorque	: 1;

		uint16_t motorPFCshutdown		: 1;
		uint16_t motorStall				: 1;
		uint16_t motorOverheat			: 1;
		//	Added following error bits to match with drive motor fault bits - Dec 2015
		uint16_t motorSusOC				: 1;
		uint16_t motorPWMCosting		: 1;
		uint16_t motorAnyOther			: 1;
		uint16_t motorCableFault		: 1;
		uint16_t unused					: 5;
	} bits;
} _DriveMotorFault;

typedef union unDriveApplicationFault
{
	uint32_t val;
	struct stDriveApplicationFault
	{
		uint32_t peObstacle					: 1;
		uint32_t lowInputVoltage			: 1;
		uint32_t highInputVoltage			: 1;
		uint32_t hallSensor					: 1;

		//	Removed to match updated Drive motor fault bits on 22 Apr 15
		//uint32_t originSensor				: 1;
		uint32_t wraparound					: 1;
		uint32_t microSwitch				: 1;
		uint32_t airSwitch					: 1;

		uint32_t emergencyStop				: 1;

		// Added to match updated Drive motor fault bits on 22 Apr 15
		uint32_t osNotDetectUP				: 1;	// origin sensor not detected while rolling up
		uint32_t osNotDetectDown			: 1;	// origin sensor not detected while rolling down
		uint32_t osDetectOnUp				: 1;	// origin Sensor becomes ON while shutter is between the upper limit and origin sensor level. (expected is OFF)
		uint32_t osDetectOnDown				: 1;	// origin Sensor becomes OFF while shutter is between origin sensor level and lower limit (expected is ON)

		uint32_t osFailValidation			: 1;	// In case of teaching mode, error from limit calculation is more than or equal to 500mm
//		uint32_t installationFailed			: 1;

		uint32_t driveCalibrationFailed		: 1;
		uint32_t microSwitchSensorLimit		: 1;	//	A080
		uint32_t maintenanceCountOverflow	: 1;	//	A025

		uint32_t igbtOverTemperature		: 1;	//	IGBT overtemperature fault
		//	Added following error bits to match with  application fault bits in drive board firmware - Dec 2015
		uint32_t powerFail			 	  	: 1;   //     power fail condition
		uint32_t shutterFalseUpMovementCount 	  : 1;   //     False movement in up direction
		uint32_t shutterFalseDownMovementCount 	  : 1;   //     False movement in down direction

		uint32_t motorCableFault          	: 1;
		uint32_t unused                   	: 11;
	} bits;
} _DriveApplicationFault;

typedef union unDriveProcessorFault
{
	uint16_t val;
	struct stDriveProcessorFault
	{
		uint16_t flashImageCRC			: 1;
		uint16_t eepromParameterDbCRC	: 1;
//		uint16_t ramIntegrity			: 1;	// Removed to match updated Drive processor fault bits on 13 Aug 14
//		uint16_t processor				: 1;	// Removed to match updated Drive processor fault bits on 13 Aug 14
		uint16_t watchdog				: 1;
		uint16_t eepromProgramming		: 1;

		uint16_t eepromErase			: 1;
		uint16_t unsued					: 11;
	} bits;
} _DriveProcessorFault;

typedef union unDriveInstallation
{
	uint16_t val;
	struct stDriveInstallation
	{
		uint16_t installA100			: 1;
		uint16_t installA101			: 1;
		uint16_t installA102			: 1;
		uint16_t installationValid		: 1;

		uint16_t installationSuccess	: 1;
		uint16_t installationFailed		: 1;
		uint16_t unused					: 10;
	} bits;
} _DriveInstallation;

/*
	Control board status and fault registers
*/
typedef union unControlBoardStatus
{
	uint8_t val;
	struct stControlBoardStatus
	{
		uint8_t runStop						: 1;	// Run - 1, Stop - 0
		uint8_t autoManual					: 1;	// Auto - 1, Manual - 0
		uint8_t controlFault				: 1;
		uint8_t controlFaultUnrecoverable	: 1;

		uint8_t driveCommunicationStatus	: 1;
		uint8_t controlBoardBootLoader		: 1;	//	This bit is one when control board bootloader is running
		uint8_t wirelessMonitor				: 1;	//	Added on 04 Dec 2014
		uint8_t unused						: 1;
	} bits;
} _ControlBoardStatus;

typedef union unControlBoardFault
{
	uint8_t val;
	struct stControlBoardFault
	{
		uint8_t controlCommunication	: 1;
		uint8_t	controlApplication		: 1;
		uint8_t controlProcessor		: 1;
		uint8_t unused					: 5;
	} bits;
} _ControlBoardFault;

typedef union unControlCommunicationFault
{
	uint8_t val;
	struct stControlCommunicationFault
	{
		uint8_t crcErrorDrive	: 1;	// Renamed on 14 Aug 2014 as two more crc errors related to display and relay are added
		uint8_t commFailDrive	: 1;
		uint8_t commFailRelay	: 1;
		uint8_t uartErrorDrive	: 1;

		uint8_t crcErrorDisplay	: 1;	// Added on 14 Aug 2014
		uint8_t crcErrorRelay	: 1;	// Added on 14 Aug 2014
		uint8_t uartErrorDisplay: 1;
		uint8_t uartErrorRelay	: 1;
	} bits;
} _ControlCommunicationFault;

typedef union unControlApplicationFault
{
	uint32_t val;
	struct stControlApplicationFault
	{
		uint8_t operationRestrictionTimer	: 1;	// A009 parameter value exceeded
		uint8_t startupSafetySensor			: 1;
		uint8_t ObstacleSensor              : 1;
		uint8_t unused						: 5;
	} bits;
} _ControlApplicationFault;

typedef union unControlProcessorFault
{
	uint16_t val;
	struct stControlProcessorFault
	{
		uint16_t eepromParameterDbCRC	: 1;
		uint16_t ramIntegrity			: 1;
		uint16_t eepromProgramming		: 1;
		uint16_t eepromErase			: 1;

		uint16_t flashImageCRC			: 1;
		uint16_t processor				: 1;
		uint16_t watchdog				: 1;
		uint16_t eepromRead				: 1;

		uint16_t unsued					: 8;
	} bits;
} _ControlProcessorFault;

/*
	Display board status and fault registers
*/
typedef union unDisplayBoardStatus
{
	uint8_t val;
	struct stDisplayBoardStatus
	{
		uint8_t runStop						: 1;	// Run - 1, Stop - 0
		uint8_t autoManual					: 1;	// Auto - 1, Manual - 0
		uint8_t displayFault				: 1;
		uint8_t displayFaultUnrecoverable	: 1;

		uint8_t unused						: 4;
	} bits;
} _DisplayBoardStatus;

typedef union unDisplayBoardFault
{
	uint8_t val;
	struct stDisplayBoardFault
	{
		uint8_t displayCommunication	: 1;
		uint8_t displayBoard			: 1;
		uint8_t	displayApplication		: 1;
		uint8_t displayProcessor		: 1;

		uint8_t unused					: 4;
	} bits;
} _DisplayBoardFault;

typedef union unDisplayCommunicationFault
{
	uint8_t val;
	struct stDisplayCommunicationFault
	{
		uint8_t crcError		: 1;
		uint8_t commFailControl	: 1;
		uint8_t commFailDrive	: 1;
		uint8_t uartError		: 1;

		uint8_t unused			: 4;
	} bits;
} _DisplayCommunicationFault;

typedef union unDisplayBoardHwFault
{
	uint8_t val;
	struct stDisplayBoardHwFault
	{
		uint8_t sdCardDetection		: 1;
		uint8_t sdWriteProtect		: 1;
		uint8_t commFailOLED		: 1;
		uint8_t unused				: 5;
	} bits;
} _DisplayBoardHwFault;

typedef union unDisplayApplicationFault
{
	uint32_t val;
	struct stDisplayApplicationFault
	{
		uint8_t sdRead	: 1;
		uint8_t sdWrite	: 1;
		uint8_t unused	: 6;
	} bits;
} _DisplayApplicationFault;

typedef union unDisplayProcessorFault
{
	uint16_t val;
	struct stDisplayProcessorFault
	{
		uint16_t eepromParameterDbCRC	: 1;
		uint16_t ramIntegrity			: 1;
		uint16_t eepromProgramming		: 1;
		uint16_t eepromErase			: 1;

		uint16_t flashImageCRC			: 1;
		uint16_t processor				: 1;
		uint16_t watchdog				: 1;
		uint16_t eepromRead				: 1;

		uint16_t unsued					: 8;
	} bits;
} _DisplayProcessorFault;


/*
	Communication between CMDi and CMDr
*/
typedef struct stCMDitoCMDr
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;
	uint8_t transmitCommandPacket[TRANSMIT_BUFFER_SIZE];
	uint8_t transmitCommandPacketLen;

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;


	uint8_t receiveCommandPacket[RECEIVE_BUFFER_SIZE];
	uint8_t receiveCommandPacketLen;
} _CMDitoCMDr;

/*
	Communication between EM and DH
*/
typedef struct stEMtoDH
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;
	struct stControlAnomaly errorToDH;
	uint8_t controlAnomalyIndex;

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
} _EMtoDH;


/*
	Commands from drive board to control board
*/
typedef struct stCMDrtoLS
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	union unCMDrtoLS
	{
		uint8_t val;
		struct stCMDrLS
		{
			uint8_t restartDriveCommunication	: 1;
			uint8_t unsued						: 7;
		} bits;
	}commandFromDriveBoard;

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
	enum acknowledgment acknowledgementReceived;
} _CMDrtoLS;

/*
	Communication between LS and CMDr
*/
typedef struct stLStoCMDr
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	union unLSCMDr
	{
		uint32_t val;
		struct stLSCMDr
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
			uint32_t getOperationCount				: 1;
			uint32_t startPowerOnCalibration		: 1;
			uint32_t stopPowerOnCalibration			: 1;
			uint32_t setParameter					: 1;	//	Added to implement setParameter command for setting snow mode, on 10APR2015
			uint32_t start_apertureHeight 			: 1;
			uint32_t unused							: 14;
		} bits;
	}commandToDriveBoard;

	uint8_t additionalCommandData;
	// for openShutterJog: = 10 for 10% Jog,  = 50 for 50% jog

	//	Added to implement setParameter command for setting snow mode, on 10APR2015
	struct cstCommandDataLSCMDr
	{
		union
		{
			uint32_t setParameterValue;
		}commandData;
		uint16_t parameterNumber;	// for setParameter command
		uint16_t unused;
	}dataToDriveBoard;

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
	enum acknowledgment acknowledgementReceived;
	uint32_t getParameterValue;

} _LStoCMDr;

/*
	Communication between EM and CMDr
*/
typedef struct stEMtoCMDr
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	union unEMCMDr
	{
		uint8_t val;
		struct stEMCMDr
		{
			uint8_t getError		: 1;		//Get error from drive board
			uint8_t unsued			: 7;
		} bits;
	} commandToDriveBoard;
	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
	struct stDriveAnomaly errorFromDrive;

	//
	// Response Acknowledgment section
	//
	enum responseAcknowledgement commandResponseACK_Status;
	//enum acknowledgement commandResponseACK;
} _EMtoCMDr;


/*
	Communication between CMDi and LS
*/
typedef struct stCMDitoLS
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	union unCMDiLS
	{
		uint32_t val;
		struct stCMDiLS
		{
			uint32_t autoManSel			: 1;
			uint32_t runControlBoard	: 1;
			uint32_t stopControlBoard	: 1;
			uint32_t upPressed			: 1;
			uint32_t upReleased			: 1;
			uint32_t downPressed		: 1;
			uint32_t downReleased		: 1;
			uint32_t openPressed		: 1;
			uint32_t openReleased		: 1;
			uint32_t closePressed		: 1;
			uint32_t closeReleased		: 1;
			uint32_t stopPressed		: 1;
			uint32_t stopReleased		: 1;
			uint32_t enterPressed		: 1;
			uint32_t enterReleased		: 1;
			uint32_t startInstallation	: 1;
			uint32_t wirelessModeChangePressed	: 1;	//	Added on 04 Dec for new client requirement
			uint32_t wirelessModeChangeReleased	: 1;	//	Added on 04 Dec for new client requirement

			//	Added this commands to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
			uint32_t settingsModeStatus	: 1;
			uint32_t start_apertureHeight	: 1;
			uint32_t unused				: 12;
		} bits;
	} commandDisplayBoardLS;

	//	Added this field to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
	uint8_t additionalCommandData;

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
	enum acknowledgment acknowledgementReceived;
} _CMDitoLS;

/*
	Communication between CMDi and DH
*/
typedef struct stCMDitoDH
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	union unCommandCMDiDH
	{
		uint16_t val;
		struct stCommandCMDiDH
		{
			uint16_t getErrorList		: 1;
			uint16_t setTimeStamp		: 1;
			uint16_t getParameter		: 1;
			uint16_t setParameter		: 1;
			uint16_t firmwareUpgrade	: 1;
			uint16_t recoveranomaly     : 1;
			uint16_t unused				: 11;
		} bits;
	} commandDisplayBoardDH;

	struct cstCommandDataCMDiDH
	{
		union
		{
			time_t timeStamp;
			uint32_t setParameterValue;
		}commandData;
		uint16_t parameterNumber;	// for getParameter and setParameter commands
	}commandDataCMDiDH;

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
	enum acknowledgment acknowledgementReceived;

	struct stControlAnomaly errorFromControl;
	uint32_t getParameterValue;

	//
	// Response Acknowledgment section
	//
	enum responseAcknowledgement commandResponseACK_Status;
	//enum acknowledgment commandResponseACK;
} _CMDitoDH;

/*
	Communication between CMDi and MLH
*/
typedef struct stCMDitoMLH
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	union unCMDiMLH
	{
		uint8_t val;
		struct stCMDiMLH
		{
			uint8_t monitorLEDControl	: 1;
			uint8_t unused				: 7;
		} bits;
	} commandDisplayBoardMLH;

	/*
		Monitor LED status register
	*/
	uint8_t additionalCommandData;
	/*
	 * for monitor LED on, off, blink rate or flash pattern
		Monitor LED status register
		0bXXXX 0000 �OFF
		0bXXXX 0001 �ON
		0b0001 0010 �1 FLASH
		0b0010 0010 �1 FLASH		//	2flashes   3 secOFF ->0.5secON->0.5secOFF->0.5secON->3secOFF->repeat
		.
		.
		.
		0b1000 0010 �8 FLASH
		0b0001 0011 �BLINK at 0.05 sec
		0b0010 0011 �BLINK at 0.1 sec
		0b0011 0011 �BLINK at 0.15 sec
		0b0100 0011 �BLINK at 0.2 sec
		0b0101 0011 �BLINK at 0.25 sec
		0b0110 0011 �BLINK at 0.3 sec
		0b0111 0011 �BLINK at 0.35 sec
		0b1000 0011 �BLINK at 0.4 sec
		0b1001 0011 �BLINK at 0.45 sec
		0b1010 0011 �BLINK at 0.5 sec
	 */

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
	enum acknowledgment acknowledgmentReceived;
} _CMDitoMLH;

#ifdef VERSION_1HARDWARE
typedef union unDriveStatusMenu
{
	uint32_t val;
	struct stDriveStatusMenu
	{
		uint32_t Wireless_1PBS_Status						: 1;
		uint32_t Wireless_Stop_Status						: 1;
		uint32_t Wireless_Close_Status						: 1;
		uint32_t Wireless_Open_Status						: 1;
		uint32_t Upper_Deceleration_Strt_Pt_Reach_Status	: 1;
		uint32_t Lower_Limit_Reached_Status					: 1;
		uint32_t Upper_Limit_Reached_Status					: 1;
		uint32_t Multi_Fun_Input2_Status					: 1;
		uint32_t Multi_Fun_Input1_Status					: 1;
		uint32_t Emergency_Stop_Input_Status				: 1;
		uint32_t Close_Key_3PBS_Status						: 1;
		uint32_t Stop_Key_3PBS_Status						: 1;
		uint32_t Open_Key_3PBS_Status						: 1;
		uint32_t One_PBS_Status								: 1;
		uint32_t Interlock_Input_Status						: 1;
		uint32_t Startup_Status								: 1;
		uint32_t Origin_Status								: 1;
		uint32_t Mocro_Switch_Status						: 1;
		uint32_t PE_Sensor_Status							: 1;
		uint32_t Ignore_PE_Limit_Reached_Status				: 1;
		uint32_t Motor_Thermal_Input_Status					: 1;
		uint32_t Manual_Mode_Status							: 1;
		uint32_t Auto_Mode_Status							: 1;
		uint32_t unused										: 9;
	} bits;
} _DriveStatusMenu;
#endif

#ifndef VERSION_1HARDWARE
typedef union unDriveStatusMenu
{
	uint32_t val;
	struct stDriveStatusMenu
	{
		uint32_t Wireless_1PBS_Status						: 1;
		uint32_t Wireless_Stop_Status						: 1;
		uint32_t Wireless_Close_Status						: 1;
		uint32_t Wireless_Open_Status						: 1;
		uint32_t Upper_Deceleration_Strt_Pt_Reach_Status	: 1;
		uint32_t Lower_Limit_Reached_Status					: 1;
		uint32_t Upper_Limit_Reached_Status					: 1;
		uint32_t unused_1									: 1;
		uint32_t Startup_Safety_Status						: 1;
		uint32_t Emergency_Stop_Input_Status				: 1;
		uint32_t Close_Key_3PBS_Status						: 1;
		uint32_t Stop_Key_3PBS_Status						: 1;
		uint32_t Open_Key_3PBS_Status						: 1;
		uint32_t One_PBS_Status								: 1;
		uint32_t Interlock_Input_Status						: 1;
		uint32_t Startup_Status								: 1;
		uint32_t Origin_Status								: 1;
		uint32_t Mocro_Switch_Status						: 1;
		uint32_t PE_Sensor_Status							: 1;
		uint32_t Ignore_PE_Limit_Reached_Status				: 1;
		uint32_t Motor_Thermal_Input_Status					: 1;
		uint32_t Manual_Mode_Status							: 1;
		uint32_t Auto_Mode_Status							: 1;
		uint32_t unused										: 9;
	} bits;
} _DriveStatusMenu;
#endif



/****************************************************************************/

/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/

/*
	Monitor LED control register declaration
*/
extern _MonitorLED gstMonitorLEDControlRegister;

extern _ElectroMechanicalCounter gstElectroMechanicalCounter;

/*
	Drive Status Menu
*/
extern _DriveStatusMenu	gstDriveStatusMenu;

/*
	Drive board status and fault registers declaration
*/
extern _DriveStatus				gstDriveStatus;
extern _DriveFault					gstDriveBoardFault;
extern _DriveCommunicationFault	gstDriveCommunicationFault;
extern _DriveMotorFault			gstDriveMotorFault;
extern _DriveApplicationFault		gstDriveApplicationFault;
extern _DriveProcessorFault		gstDriveProcessorfault;
extern _DriveInstallation			gstDriveInstallation;

/*
	Control board status and fault registers declaration
*/
extern _ControlBoardStatus			gstControlBoardStatus;
extern _ControlBoardFault			gstControlBoardFault;
extern _ControlCommunicationFault	gstControlCommunicationFault;
extern _ControlApplicationFault	gstControlApplicationFault;
extern _ControlProcessorFault		gstControlProcessorFault;

#ifdef unused	// Display board registers not needed for control board
/*
	Display board status and fault registers
*/
extern _DisplayBoardStatus			gstDisplayBoardStatus;
extern _DisplayBoardFault			gstDisplayBoardFault;
extern _DisplayCommunicationFault	gstDisplayCommunicationFault;
extern _DisplayBoardHwFault		gstDisplayBoardHwFault;
extern _DisplayApplicationFault	gstDisplayApplicationFault;
extern _DisplayProcessorFault		gstDisplayProcessorFault;

#endif
/*
	Inter-module communication
*/
extern _CMDitoCMDr		gstCMDitoCMDr;
extern _EMtoDH			gstEMtoDH;
extern _CMDrtoLS		gstCMDrtoLS;
extern _LStoCMDr		gstLStoCMDr;
extern _EMtoCMDr		gstEMtoCMDr;
extern _CMDitoLS		gstCMDitoLS;
extern _CMDitoDH		gstCMDitoDH;
extern _CMDitoMLH		gstCMDitoMLH;

// Following flag is used to change system state from run to stop.
// CMDi will set this flag  when setParameter command is received from display.
// Logic solver will change the system state from run to stop and it
// won't send any operation command to drive when this flag is set.

extern uint8_t gucSetParameterCommandFlag;

//
//	Added on 17 Nov 2014 to implement drive board firmware upgrade functionality.
//
//	Following flag is used to indicate that drive firmware upgrade was initiated by
//	display board and drive board has responded to the request with ACK.

extern uint8_t gui8DriveFirwareUpgradeInitiated;

//
//	Added this flag to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
//
extern uint8_t guiSettingsModeStatus;
/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/



/****************************************************************************/


/****************************************************************************
 *  Function Prototype:
 ****************************************************************************/

void initControlBoardGlobalRegisters(void);

// *********************************************************************************************
// Variable for sub-state 'Handle Counter' of 'Logic_Solver_Drive_Run'
// *********************************************************************************************
enum HandleCounter {

	HandleCounterInitGetCounter = 0,
	HandleCounterGetCounterWaitingReply,
	HandleCounterWaitLowerLimitGetCounter,
	HandleCounterLowerLimitGetCounterWaitingReply,
	HandleCounterWaitNonLowerLimit

};
extern enum HandleCounter geHandleCounter;

extern uint32_t gulCounterValue;

// *********************************************************************************************


// *********************************************************************************************
// Variable to indicate "System Init Complete" command received from Display board
// *********************************************************************************************

extern uint8_t gucSystemInitComplete;

// *********************************************************************************************



#endif /* __INTERTASKCOMMUNICATION_H__ */
