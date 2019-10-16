/*********************************************************************************
 * FileName: interTaskCommunication.h
 * Description:
 * This source file contains the definition of all the global variables
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
 *  0.2D		14/07/2014									Change in Drive STATUS structure as per Drive Board code
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       Initial Creation
 ****************************************************************************/
#ifndef __INTERTASKCOMM_H__
#define __INTERTASKCOMM_H__
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
typedef struct stLEDcontrolRegister
{
	uint8_t powerLED;		// would be controlled by EM in case of normal and installation operation
	uint8_t autoManualLED;	// would be controlled by UM in case of normal and installation operation
	uint8_t faultLED;		// would be controlled by EM in case of normal, error and installation operation
	uint8_t monitorLED;	// would be controlled by EM in case of normal, error and installation operation
	/*
	 * for monitor LED on, off, blink rate or flash pattern
				Monitor LED status register
				0bXXXX 0000 ?OFF
				0bXXXX 0001 ?ON
				0b0001 0010 ?1 FLASH
				0b0010 0010 ?2 FLASH		//	2flashes   3 secOFF ->0.5secON->0.5secOFF->0.5secON->3secOFF->repeat
				.
				.
				.
				0b1000 0010 ?8 FLASH
				0b0001 0011 ?BLINK at 0.05 sec
				0b0010 0011 ?BLINK at 0.1 sec
				0b0011 0011 ?BLINK at 0.15 sec
				0b0100 0011 ?BLINK at 0.2 sec
				0b0101 0011 ?BLINK at 0.25 sec
				0b0110 0011 ?BLINK at 0.3 sec
				0b0111 0011 ?BLINK at 0.35 sec
				0b1000 0011 ?BLINK at 0.4 sec
				0b1001 0011 ?BLINK at 0.45 sec
				0b1010 0011 ?BLINK at 0.5 sec
	 */
} _LEDcontrolRegister;


enum errorType
{
	eNO_ERROR,
	eRECOVERABLE_ERROR,
	eNON_RECOVERABLE_ERROR
};

/*
	RAM DB for error list
*/
struct errorDB
{
	uint16_t errorCode;
	//unsigned char errordescription[15];
	unsigned char errordescription[30];
	uint8_t errorType;
	// errorType = 0 for no error
	// errorType = 1 for recoverable error
	// errorType = 2 for non-recoverable error
};

typedef enum destinationAddress
{
	eDestControlBoard = 0,
	eDestDriveBoard,
	eDestDisplayBoard
}dest_enum_cyw;
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
	eFAIL,
};
enum acknowledgement
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

struct stControlAnomaly
{
	time_t timeStamp;
	uint16_t anomalyCode;
	uint8_t errorDetails[30];
	uint8_t anomalyLED_blinkRate;
	uint8_t monitorLED_blinkRate;
	uint32_t operationCount;
};

struct stChangeSettingHistory
{
	uint16_t parameterNumber;
	uint32_t oldValue;
	uint32_t newValue;
	time_t timeStamp;
};

/*
	Drive board status and fault registers
*/
typedef union unDriveBoardStatus
{
	uint32_t val;
	struct stDriveBoardStatus
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
		uint32_t unused						: 14;
	} bits;
} _DriveBoardStatus;

typedef union unDriveBoardFault
{
	uint8_t val;
	struct stDriveBoardFault
	{
		uint8_t driveCommunication	: 1;
		uint8_t driveMotor			: 1;
		uint8_t driveApplication	: 1;
		uint8_t driveProcessor		: 1;

		uint8_t unused				: 4;
	} bits;
} _DriveBoardFault;

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
		uint8_t controlBoardBootLoader 		: 1; 	// This bit is one when control board bootloader is running
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
	uint8_t val;
	struct stControlApplicationFault
	{
		uint8_t operationRestrictionTimer	: 1;	// A009 parameter value exceeded
		uint8_t startupSafetySensor			: 1;
		uint8_t unused						: 6;
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
		uint8_t controlBoardCommunicationStatus		:1;
		uint8_t unused						: 3;
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
		uint8_t powerOnIndication	: 1;		//	Added to log system power on event as requested by Bx on 21Apr2015
		uint8_t unused				: 4;
	} bits;
} _DisplayBoardHwFault;

typedef union unDisplayApplicationFault
{
	uint8_t val;
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
	Communication between UM and CM (operational commands)
*/
typedef struct stUMtoCMoperational
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	union unUMCM
	{
		uint32_t val;
		struct stUMCM
		{
			uint32_t autoManSelect				: 1;
			uint32_t runControlBoard			: 1;
			uint32_t stopControlBoard			: 1;
			uint32_t upPressed					: 1;
			uint32_t upReleased					: 1;
			uint32_t downPressed				: 1;
			uint32_t downReleased				: 1;
			uint32_t openPressed				: 1;
			uint32_t openReleased				: 1;
			uint32_t closePressed				: 1;
			uint32_t closeReleased				: 1;
			uint32_t stopPressed				: 1;
			uint32_t stopReleased				: 1;
			uint32_t enterPressed				: 1;
			uint32_t enterReleased				: 1;
			uint32_t modePressed				: 1;
			uint32_t modeReleased				: 1;
			uint32_t startInstallation			: 1;
			uint32_t systemInitComplete			: 1;	//	Added on 26 Aug for display control board sync
			uint32_t wirelessModeChangePressed	: 1;	//	Added on 04 Dec as per new requirement from client
			uint32_t wirelessModeChangeReleased	: 1;	//	Added on 04 Dec as per new requirement from client
			//	Added this command to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
			uint32_t settingsModeStatus			: 1;
			uint32_t unused						: 10;
		} bits;
	} commandToControlBoard;

	//	Added this member to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
	uint8_t additionalCommandData;
	// for settingsModeStatus command : additionalCommandData	= 1 : settings mode active
	//															= 0 : settings mode inactive

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
	enum acknowledgement acknowledgementReceived;
} _UMtoCMoperational;

/*
	Communication between UM and CM (DB related commands)
*/
typedef struct stUMtoCMdatabase
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	enum destinationAddress destination;

	union unCommandUMCM
	{
		uint16_t val;
		struct cstCommandUMCM
		{
//			uint16_t getErrorList		: 1;
			uint16_t setTimeStamp		: 1;
			uint16_t getParameter		: 1;
			uint16_t setParameter		: 1;
			uint16_t firmwareUpgrade	: 1;
			uint16_t unused				: 11;
		} bits;
	} commandToControlBoard;

	struct cstCommandDataUMCM
	{
		union
		{
			time_t timeStamp;
			uint32_t setParameterValue;
		}commandData;
		uint16_t parameterNumber;	// for getParameter and setParameter commands
		uint16_t unused;
	}dataToControlBoard;

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
	enum acknowledgement acknowledgementReceived;

	struct stControlAnomaly errorFromControl;
	uint32_t getParameterValue;
} _UMtoCMdatabase;

/*
	Communication between UM and LM
*/
typedef struct stUMtoLM_write
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	union unCommandUMLM
	{
		uint8_t val;
		struct stCommandUMLM
		{
			uint8_t changeSettingHistory	: 1;
			uint8_t unused					: 7;
		} bits;
	} commandToLMwrite;

	struct stChangeSettingHistory changeSetting;
	uint8_t changeSetting_index;

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
} _UMtoLM_write;

/*
	Communication between EM and UM
*/
typedef struct stEMtoUM
{
	uint8_t faultLEDstatus;
	/*
	 * for fault LED on, off, blink rate or flash pattern
		Monitor LED status register
		0bXXXX 0000 ?OFF
		0bXXXX 0001 ?ON
		0b0001 0010 ?1 FLASH
		0b0010 0010 ?2 FLASH		//	2flashes   3 secOFF ->0.5secON->0.5secOFF->0.5secON->3secOFF->repeat
		.
		.
		.
		0b1000 0010 ?8 FLASH
		0b0001 0011 ?BLINK at 0.05 sec
		0b0010 0011 ?BLINK at 0.1 sec
		0b0011 0011 ?BLINK at 0.15 sec
		0b0100 0011 ?BLINK at 0.2 sec
		0b0101 0011 ?BLINK at 0.25 sec
		0b0110 0011 ?BLINK at 0.3 sec
		0b0111 0011 ?BLINK at 0.35 sec
		0b1000 0011 ?BLINK at 0.4 sec
		0b1001 0011 ?BLINK at 0.45 sec
		0b1010 0011 ?BLINK at 0.5 sec
	 */
}_EMtoUM;

/*
	Communication between UM and EM
*/
typedef struct stUMtoEM
{
	//
	// command section
	//

//	enum requestStatus commandRequestStatus;

	union unCommandUMEM
	{
		uint8_t val;
		struct stCommandUMEM
		{
			uint8_t anomalyHistoryAccessed		: 1;
			uint8_t unused						: 7;
		} bits;
	} commandToEM;

	//
	// Response Section
	//
//	enum responseStatus commandResponseStatus;
} _UMtoEM;

/*
	Communication between EM and CM (get error list from controller)
*/
typedef struct stEMtoCM_errorList
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	union unCommandEMtoCM
	{
		uint8_t val;
		struct stCommandEMtoCM
		{
			uint8_t getError		: 1;
			uint8_t unused			: 7;
		} bits;
	} commandToCM;

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;

	struct stControlAnomaly errorFromControl;

	//
	//	Response Acknowledgment section
	//
	enum responseAcknowledgement commandResponseACK_Status;
} _EMtoCM_errorList;

/*
	Communication between EM and LM
*/
typedef struct stEMtoLM
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;
	struct stControlAnomaly errorToLM;

	uint8_t anomaly_index;
	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;

	//
	// Response Acknowledgment section
	//
	enum responseAcknowledgement commandResponseACK;
} _EMtoLM;

/*
	Communication between EM and CM (Monitor LED control)
*/
typedef struct stEMtoCM_monitorLED
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	union unEMCM
	{
		uint8_t val;
		struct stEMCM
		{
			uint8_t monitorLEDControl		: 1;
			uint8_t unused					: 7;
		} bits;
	} commandToControlBoard;
	/*
		Monitor LED status register
	*/
	uint32_t additionalCommandData;
	/*
	 * for monitor LED on, off, blink rate or flash pattern
			Monitor LED status register
			0bXXXX 0000 ?OFF
			0bXXXX 0001 ?ON
			0b0001 0010 ?1 FLASH
			0b0010 0010 ?1 FLASH		//	2flashes   3 secOFF ->0.5secON->0.5secOFF->0.5secON->3secOFF->repeat
			.
			.
			.
			0b1000 0010 ?8 FLASH
			0b0001 0011 ?BLINK at 0.05 sec
			0b0010 0011 ?BLINK at 0.1 sec
			0b0011 0011 ?BLINK at 0.15 sec
			0b0100 0011 ?BLINK at 0.2 sec
			0b0101 0011 ?BLINK at 0.25 sec
			0b0110 0011 ?BLINK at 0.3 sec
			0b0111 0011 ?BLINK at 0.35 sec
			0b1000 0011 ?BLINK at 0.4 sec
			0b1001 0011 ?BLINK at 0.45 sec
			0b1010 0011 ?BLINK at 0.5 sec
	 */

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
	enum acknowledgement acknowledgementReceived;
} _EMtoCM_monitorLED;

/*
	Communication between UM and LM
*/
typedef struct stUMtoLM_read
{
	//
	// command section
	//

	enum requestStatus commandRequestStatus;

	union unUMLM
	{
		uint8_t val;
		struct stUMLM
		{
			uint8_t readChangeSettingsHistory	: 1;
			uint8_t readAnomalyHistory			: 1;
			uint8_t unused						: 6;
		} bits;
	} commandToLMread;

	uint8_t historyOrAnomalyIndex;
	// for readChangeSettingsHistory : = 1 to 10
	// for readAnomalyHistory : = 1 to 10

	//
	// Response Section
	//
	enum responseStatus commandResponseStatus;
	enum acknowledgement acknowledgementReceived;

	struct stChangeSettingHistory changeSettingHistory;
	struct stControlAnomaly anomalyHistory;
} _UMtoLM_read;


/****************************************************************************/

/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/
/*
	Drive board status and fault registers declaration
*/
extern _DriveBoardStatus		gstDriveBoardStatus;
extern _DriveBoardFault			gstDriveBoardFault;
extern _DriveCommunicationFault	gstDriveCommunicationFault;
extern _DriveMotorFault			gstDriveMotorFault;
extern _DriveApplicationFault	gstDriveApplicationFault;
extern _DriveProcessorFault		gstDriveProcessorfault;
extern _DriveInstallation		gstDriveInstallation;

/*
	Control board status and fault registers declaration
*/
extern _ControlBoardStatus			gstControlBoardStatus;
extern _ControlBoardFault			gstControlBoardFault;
extern _ControlCommunicationFault	gstControlCommunicationFault;
extern _ControlApplicationFault	gstControlApplicationFault;
extern _ControlProcessorFault		gstControlProcessorFault;

/*
	Display board status and fault registers
*/
extern _DisplayBoardStatus			gstDisplayBoardStatus;
extern _DisplayBoardFault			gstDisplayBoardFault;
extern _DisplayCommunicationFault	gstDisplayCommunicationFault;
extern _DisplayBoardHwFault			gstDisplayBoardHwFault;
extern _DisplayApplicationFault		gstDisplayApplicationFault;
extern _DisplayProcessorFault		gstDisplayProcessorFault;

/*
	Inter-module communication
*/
extern _LEDcontrolRegister		gstLEDcontrolRegister;

extern _UMtoCMoperational		gstUMtoCMoperational;
extern _UMtoCMdatabase			gstUMtoCMdatabase;
extern _UMtoLM_write			gstUMtoLM_write;
extern _EMtoUM					gstEMtoUM;
extern _UMtoEM					gstUMtoEM;
extern _EMtoCM_errorList		gstEMtoCM_errorList;
extern _EMtoLM					gstEMtoLM;
extern _EMtoCM_monitorLED		gstEMtoCM_monitorLED;
extern _UMtoLM_read				gstUMtoLM_read;


/*
 * Communication error indicator for error module.
 * Added on 15 Sep 2014 to avoid communication error logging during power fail.
*/

extern uint8_t gucDItoCT_CommError;
extern uint8_t gucCTtoDR_CommError;

extern uint8_t gui8_AnomalyHistory_ParamIdx;
extern uint8_t gui8_ChgSettHistory_ParamIdx;

//	Added this variable to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
extern uint8_t gui8SettingsModeStatus;
extern uint8_t gui8SettingsScreen;

/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/



/****************************************************************************/

/****************************************************************************
 *  Function prototype declarations:
 ****************************************************************************/
void initDispBoardGlobalRegisters(void);

#endif /**** __INTERTASKCOMM_H__ ***/
