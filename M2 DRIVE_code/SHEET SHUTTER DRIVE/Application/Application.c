/*********************************************************************************
* FileName: Application.c
* Description:
* This source file contains the definition of all the functions for the Application for Drive board.
**********************************************************************************/

/****************************************************************************
 * Copyright 2014 Bunka Shutters.
 * This program is the property of the Bunka Shutters
 * Company, Inc.and it shall not be reproduced, distributed or used
 * without permission of an authorized company official.This is an
 * unpublished work subject to Trade Secret and Copyright
 * protection.
*****************************************************************************/

/****************************************************************************
 *  Modification History
 *
 *  Date                  Name          Comments
 *  22/04/2014            iGate          Initial Creation
*****************************************************************************/
#include <p33Exxxx.h>
#include "Application.h"
#include "./Common/UserDefinition/Userdef.h"
#include "CommandHandler.h"
#include "./Application/RampGenerator/RampGenerator.h"
#include "./Middleware/ParameterDatabase/eeprom.h"
#include "./Drivers/Timer/Timer.h"
#include "./Common/Extern/Extern.h"

//#define STRAP1  PORTCbits.RC7
//#define STRAP2  PORTCbits.RC9
//#define STRAP3  PORTDbits.RD5
//#define STRAP4  PORTDbits.RD6

//#define FIRMWARE_VERSION    1001    //MMNN - MM major version, NN minor version
//#define HARDWARE_VERSION    ((STRAP1*1000)+(STRAP2*100)+(STRAP3*10)+STRAP4)

#define UART_ERROR_STATUS_HOLD_COUNT	1000

// UART error monitoring variables
UINT16 errorUARTTimeCount = 0;
WORD systemTick = 0;
//20160806 AOYAGI
//UINT16 TIME_CMD_open_shutter=0;
//UINT16 TIME_CMD_close_shutter=0;
//20160806 AOYAGI
BOOL emergencySensorTrigrd = FALSE;
BOOL microSwSensorTrigrd = FALSE;
BOOL photoElecObsSensTrigrd = FALSE;
BOOL tempSensTrigrd = FALSE;
BOOL airSwitchTrigrd = FALSE;
BOOL wrapAroundSensor = FALSE;
BOOL fourPtLmtSwtchDetected = FALSE;
BOOL originSensorDetected = FALSE;
BOOL powerFailSensorDetected = FALSE;
BOOL OvercurrentfaultTrigrd = FALSE; // indicating PWM fault due to overcurrent

BOOL ShutterInstallationStepNeedSave = TRUE;//AOYAGI 20160815

// Up and Down aperture command received flag added separetly to avoid any chance of micounting of operation count - YG - NOV 15
BOOL bUpApertureCmdRecd = FALSE;
BOOL bDownApertureCmdRecd = FALSE;

BOOL incrementOperationCnt = FALSE;
BOOL sysInitCompleted = FALSE;

SHORT currShutterType = M2_SHUTTER;//BEAD_SHUTTER;

UINT32 InstallCnt = 0;
UINT32 InstallStopCnt = 0;
BOOL StopCnt = TRUE;

UINT8  UpperPostionHigh;
UINT8  UpperPostionLow;
UINT8  LowerPostionHigh;
UINT8  LowerPostionLow;
UINT8  ShutterInstallationStep;

BOOL ShutterInstallationEnabled = FALSE;
//	Added on 3 Feb 2015 to implement user control on power on calibration

//	Global flag to indicate state of power on calibration command
BYTE powerOnCalibration = SERVICED;
// Added to overcome installation issue (A100) - RN- NOV 2015
BYTE gucInstallationInitiated = SERVICED;
//	Global variable to store value of desired power on calibration state
SHORT powerUpCalibrationCurrentState = CALIB_STATE_START;
//	Global variable to store value of desired shutter movement or operation
unsigned char inputFlagsValue = STOP_SHUTTER;

BYTE gucInstallationCalledFrom = 0;
//power up calibration data
#define HALL_COUNT_TO_MM_RATIO      1//2
#define HALL_COUNT(x)   __builtin_divud(x,HALL_COUNT_TO_MM_RATIO)
#define MAX_OS_SEARCH_LENGTH    500
#define OS_VALIDATION_LENGTH    50
#define HALL_COUNTS_FOR_A100    0


#if (SHUTTER_TYPE == SHUTTER_2M_2M)
#if 0
    #define RISE_GEAR_POS1_OFFSET    400
    #define RISE_GEAR_POS2_OFFSET    350
    #define RISE_GEAR_POS3_OFFSET    100

    #define FALL_GEAR_POS1_OFFSET    400
    #define FALL_GEAR_POS2_OFFSET    350
    #define FALL_GEAR_POS3_OFFSET    100
#else
//20160806 AOYAGI
	#define RISE_GEAR_POS1_OFFSET    900//600//850//300//500 20160915   //add 20161018
	#define RISE_GEAR_POS2_OFFSET    400//300//450//200//450 20160915   //add 20161018
  #ifdef BUG_No84_M2speed_Change           //20170614  201703_No.84
	#define RISE_GEAR_POS3_OFFSET    0
  #else
	#define RISE_GEAR_POS3_OFFSET    200//100//200
  #endif

  #ifdef BUG_No92_M2speed_Change_A106
   #define FALL_GEAR_POS1_OFFSET    1200
  #else
	#define FALL_GEAR_POS1_OFFSET    900//600//1150//300//500 20160915  //add 20161018
  #endif
	#define FALL_GEAR_POS2_OFFSET    400//300//450//200//450 20160915   //add 20161018
	#define FALL_GEAR_POS3_OFFSET    200//100//200
//20160806 AOYAGI
#endif

#elif (SHUTTER_TYPE == SHUTTER_4M_4M)
    #define RISE_GEAR_POS1_OFFSET    700
    #define RISE_GEAR_POS2_OFFSET    500
    #define RISE_GEAR_POS3_OFFSET    200

    #define FALL_GEAR_POS1_OFFSET    700
    #define FALL_GEAR_POS2_OFFSET    500
    #define FALL_GEAR_POS3_OFFSET    200

#else
    #error Shutter Knee points not defined
#endif

powerUpCalib_t powerUpCalib;
shutterInstall_t shutterInstall;

UINT16 TIME_CMD_open_shutter=0;
UINT16 TIME_CMD_close_shutter=0;
UINT8  FLAG_CMD_open_shutter=0;
UINT8  CMD_open_shutter=0;
UINT8  FLAG_StartApertureCorrection = 0;   //bug_No.12
UINT8  FLAG_open_shutter_one = 0;
//20170628 by IME
UINT16 TIME_CMD_stop_shutter=100;	//Stop and ignore descent for 500ms. 5ms x 100=500ms

UINT8 Power_ON_igbtOverTemp=0;
UINT16 Time_uart_count=0;
UINT8 Flag_powerUpCalib_osToggle=0;
/******************************************************************************
 * initApplication
 *
 * This function initializes all the variables used by the Application
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initApplication(VOID)
{
    BOOL commonBlockCRCValid, motorBlockCRCValid, applBlockCRCValid = FALSE;

	commonBlockCRCValid = initCommonBlock();
	motorBlockCRCValid = initMotorControlBlock();
	applBlockCRCValid = initApplBlock(); // application block initialization to be last always - if CRC here fails, then call for installation from A100

	// update EEPROM CRC fault
    if(!commonBlockCRCValid || !motorBlockCRCValid || !applBlockCRCValid )
	{
        //reset variable for testing
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.bits.eepromParameterDbCRC = TRUE;
	}
	else
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.bits.eepromParameterDbCRC = FALSE;
	}

//    resetAllParameters();

    currShutterType = M2_SHUTTER;//uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537;

    //Check if previous reset was due to watchdog
    if( RCONbits.WDTO )
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.bits.watchdogTrip = TRUE;
    }
    else
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.bits.watchdogTrip = FALSE;
    }

    hallCounts = uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129;
	rampOutputStatus.shutterCurrentPosition = hallCounts;
	inputFlags.value = STOP_SHUTTER;

    //initialize command handler
	initCommandHandler();
    shutterInstall.enterCmdRcvd = FALSE;

    //Wait for display and control board to get initialized
    //while(!sysInitCompleted)
    //{
    //    commandHandler();
    //}

    updateDriveStatusFlags();
	gucInstallationCalledFrom = 1;
    checkShutterInstallation();
    //If shutter installation is required then do not check shutter position
    //for power On calibration
    if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
    {
        checkShutterPosition();
    }
	//uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = TRUE;

}

/******************************************************************************
 * _T6Interrupt
 *
 * This function is main application handler
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _T6Interrupt (void)
{
    IFS2bits.T6IF = 0;

#ifndef PROGRAMMABLE_DEBOUNCE
    systemTick++;
    //monitor sensor interface
    monitorSafetySensors();
#endif
    //Set fault flags
    updateDriveFaultFlags();
    //set status flag
    updateDriveStatusFlags();
    //Update system counters
    updateSytemCounters();
    //installation
    shutterInstallation();
    //calibration
    powerUpCalibration();
    //20160806 AOYAGI
    if(TIME_CMD_open_shutter)TIME_CMD_open_shutter--;
    if(TIME_CMD_close_shutter)TIME_CMD_close_shutter--;
    //20170628 Stop and ignore descent for 500ms. 5ms x 100=500ms by IME
    if(TIME_CMD_stop_shutter<100)TIME_CMD_stop_shutter++;
}

/******************************************************************************
 * _T8Interrupt
 *
 * This function is debounce tick handler
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _T8Interrupt (void)
{
    IFS3bits.T8IF = 0;
    InstallCnt++;

#ifdef PROGRAMMABLE_DEBOUNCE
    systemTick++;
    //monitor sensor interface
    monitorSafetySensors();
#endif
}

/******************************************************************************
 * getSystemTick
 *
 * This function gets system tick count
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
UINT32 getSystemTick(VOID)
{
	return systemTick;
}

/******************************************************************************
 * application
 *
 * This function implements the task loop for the Application
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID application(VOID)
{
    commandHandler();
}

VOID updateFaultRecoverFlag(VOID)
{
    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.hallSensor)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
    }
    //else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.originSensor)
    //{
    //    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
    //}
    else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.driveCalibrationFailed)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
    }
    //else if((shutterInstall.currentState == INSTALL_STATE_END) &&
    //        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed)
    //{
    //    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
    //}
    else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.bits.eepromProgramming)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
    }
    else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.bits.eepromErase)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
    }
    else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorOverCurrent)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
    }
    //20160806 AOYAGI
//    else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorStall)
//    {
//        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
//    }
    else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorOverheat)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
    }
	else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.igbtOverTemperature)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
    }
	// Added following condition to set the 'driveFaultUnrecoverable' in main status register - YG - 2015
	else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.shutterFalseUpMovementCount)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
    }
	else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.shutterFalseDownMovementCount)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
    }
	// Added following condition to set the 'driveFaultUnrecoverable' in main status register - RN - NOV 2015
	/*else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.powerFail)
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
	}*/
	//	Added motor cable fault to unrecoverable error lists - Dec 2015
	else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.motorCableFault)
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = TRUE;
	}
    else
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFaultUnrecoverable = FALSE;
    }
}
/******************************************************************************
 * updateDriveFaultFlags
 *
 * This function updates drive fault flags, this is mainly required for flag reset operations
 * as the set operations are handled from all over the code - sensor monitors, ramp generator etc
 *
 * PARAMETER REQ:  none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID updateDriveFaultFlags(VOID)
{
	// Reset UART error if a delay has elapsed from when it was detected
	if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveCommunicationFault.bits.uartError)
	{
		errorUARTTimeCount++;
		if(UART_ERROR_STATUS_HOLD_COUNT < errorUARTTimeCount)
		{
			uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveCommunicationFault.bits.uartError = FALSE;
			errorUARTTimeCount = 0;
		}
	}

// 2017/3/8 Measures against mistake maintenanceCountOverflow. by IME
//	if(uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600 >= (uDriveApplBlockEEP.stEEPDriveApplBlock.maintenanceCountLimit_A025*1000))
	if(uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600 >= ((UINT32)uDriveApplBlockEEP.stEEPDriveApplBlock.maintenanceCountLimit_A025*1000))
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.maintenanceCountOverflow = TRUE;
	}
	else
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.maintenanceCountOverflow = FALSE;
	}

	// update communication fault present flag
	if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveCommunicationFault.val)
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveCommunicationFault = TRUE;
	}
	else
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveCommunicationFault = FALSE;
	}

	// update application fault present flag
	if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.val)
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveApplicationFault = TRUE;
	}
	else
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveApplicationFault = FALSE;
	}

	// update motor fault present flag
	if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.val)
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveMotorFault = TRUE;
	}
	else
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveMotorFault = FALSE;
	}

	// update processor fault present flag
	if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.val)
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveProcessorFault = TRUE;
	}
	else
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveProcessorFault = FALSE;
	}

	// update overall drive fault bit
	if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveProcessorFault
		|| uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveMotorFault
		|| uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveApplicationFault
		|| uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveFault.bits.driveCommunicationFault)
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFault = TRUE;
	}
	else
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveFault = FALSE;
	}

    updateFaultRecoverFlag();
}

/******************************************************************************
 * updateDriveStatusFlags
 *
 * This function updates drive status flags
 *
 * PARAMETER REQ:  none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID updateDriveStatusFlags(VOID)
{
    UINT8 status;

    // update shutter current position
	//	Do this only when power fail flag is off. This check is needed as it was observed that during
	//	powerfail condition, while running executePowerFailRoutine() value of A129 was getting changed
	//	resulting in dbCRC error upon next power on. Added - Feb 2016
	if(!gucPowerFailFlag)
	{
	uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129 = rampOutputStatus.shutterCurrentPosition;
	}

    if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.upStepCount_A525 == TWO_STEP_RAMP_PROFILE)
    {
        if(rampOutputStatus.shutterCurrentPosition <=
          uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos2_A104)
        {
            // update drive status position to upper deceleration point reached
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.upDecelStartReached = TRUE;
        }
        else
        {
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.upDecelStartReached = FALSE;
        }
    }
    else
    {
        if(rampOutputStatus.shutterCurrentPosition <=
           uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos3_A105)
        {
            // update drive status position to upper deceleration point reached
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.upDecelStartReached = TRUE;
        }
        else
        {
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.upDecelStartReached = FALSE;
        }
    }

    if(rampOutputStatus.shutterCurrentPosition >= uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102)
    {
        // update drive status position to ignore photo-electric sensor input reached
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.ignPhotoElecSensLimRchd = TRUE;
    }
    else
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.ignPhotoElecSensLimRchd = FALSE;
    }

	// don't update position status if installation is pending or in progress
	if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
	{
		//if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 >= rampOutputStatus.shutterCurrentPosition)
        if(rampOutputStatus.shutterCurrentPosition >= (uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 -
                                                       uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112))
		{
			// 2016/11/16 When Down , Missing Save Origin Position.
//			hallCounts_bak = 0x7FFF;	//20191223 Delete by IME

			// update drive status position to lower limit
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit = TRUE;
		}
        else
        {
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit = FALSE;
        }

		//if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130 >= rampOutputStatus.shutterCurrentPosition)
		if((rampOutputStatus.shutterCurrentPosition <= (uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130 +
                                                       uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112)) &&
            (rampOutputStatus.shutterCurrentPosition >= (uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130 -
                                                       uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112)))//20160806 AOYAGI
        {
			// update drive status position to aperture height
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterApertureHeight = TRUE;
		}
        else
        {
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterApertureHeight = FALSE;
        }

		//if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 <= rampOutputStatus.shutterCurrentPosition)
		if(rampOutputStatus.shutterCurrentPosition <= (uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 +
                                                       uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112))
        {
			hallCounts_bak = 0x7FFF;	//20191223 ADD by IME
			// update drive status position to upper limit
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit = TRUE;
		}
        else
        {
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit = FALSE;
        }
         if((rampOutputStatus.shutterCurrentPosition > (uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 +
            uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112)) && (rampOutputStatus.shutterCurrentPosition <
            (uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130 -
            uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112)))
        {
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterBetweenUplmtAphgt = TRUE;
        }
        else
        {
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterBetweenUplmtAphgt = FALSE;
        }

        if((rampOutputStatus.shutterCurrentPosition < (uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 -
            uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112)) && (rampOutputStatus.shutterCurrentPosition >
            (uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130 +
            uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112)))
        {
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterBetweenLowlmtAphgt = TRUE;
        }
        else
        {
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterBetweenLowlmtAphgt = FALSE;
        }
	}
	else
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit = FALSE;
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterApertureHeight = FALSE;
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit = FALSE;
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.upDecelStartReached = FALSE;
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.ignPhotoElecSensLimRchd = FALSE;
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterBetweenLowlmtAphgt = FALSE;
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterBetweenUplmtAphgt = FALSE;
	}

    //update shutter movement status
    status = getDriveMovement();
    if(status == SHUTTER_STOPPED)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterStopped = TRUE;
    }
    else
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterStopped = FALSE;
    }

    if(status == SHUTTER_MOVING_UP)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterMovingUp = TRUE;
    }
    else
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterMovingUp = FALSE;
    }

    if(status == SHUTTER_MOVING_DOWN)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterMovingDown = TRUE;
    }
    else
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterMovingDown = FALSE;
    }
}

/******************************************************************************
 * updateSytemCounters
 *
 * This function implements system counters - operation count and aperture frequency
 * For both operation count and aperture frequency count, consider that shutter has completely
 * closed (lower limit) and then gone to aperture height or upper limit then we consider
 * count increment as 1.
 * This method also updates drive position status to shutterUpperLimit or shutterApertureHeight
 * or shutterLowerLimit
 *
 * PARAMETER REQ:  none
 *
 * RETURNS: bReply - TRUE for ok, FALSE if values were found to be different and
 * 					the EEPROM values have been updated
 *
 * ERRNO: none
 ********************************************************************************/
VOID updateSytemCounters(VOID)
{

	//Operation count increment logic when shutter is at lower limit (Reaches lower limit from Upper Limit) is commented - YG - Nov 15
	#if 0
    //if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit && (!rampOutputStatus.shutterMoving))
    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)
    {
        if(incrementOperationCnt)
        {
            incrementOperationCnt = FALSE;
            uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600++;
			//uDriveApplBlockEEP.stEEPDriveApplBlock.maintenanceCountValue_A636++;

            //rampStatusFlags.saveParamToEeprom = TRUE;

            if(bDownApertureCmdRecd)
            {
                bDownApertureCmdRecd = FALSE;
                uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604++;
                 // if auto-correction is allowed for aperture operation
                if(uDriveApplBlockEEP.stEEPDriveApplBlock.autoCorrectionEnabled_A127)
				{
					// if aperture count exceeds the EEPROM parameter -- then trigger a runtime calibration operation,
					// at the end of runtime calibration, the shutter should come to rest at the aperture height instead of upper limit
// 2017/3/8 by IME
//					if(uDriveApplBlockEEP.stEEPDriveApplBlock.correctedFreqAperture_A126 <= uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604)
					if((UINT32)uDriveApplBlockEEP.stEEPDriveApplBlock.correctedFreqAperture_A126 <= uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604)
					{
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.driveCalibrationFailed = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration = TRUE;
                        //set the calibration state to search origin in upward direction
						//	Added on 03 FEB 2015 to implement user control on power up calibration
                        //powerUpCalib.currentState = CALIB_SEARCH_ORG_UP_DIR;
						powerUpCalib.currentState = CALIB_STATE_START;
						powerUpCalibrationCurrentState = CALIB_SEARCH_ORG_UP_DIR;
                        //Set calib target position as upper limit
                        powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
                        //reset calib os toggle status
                        powerUpCalib.osToggle = 0;
                        //Start shutter jog up movement
						//	Added on 03 FEB 2015 to implement user control on power up calibration
                        //inputFlags.value = OPEN_SHUTTER_JOG_50;
						inputFlagsValue = OPEN_SHUTTER_JOG_50;
                        powerOnCalibration = INITIATED;
					}
				}
            }
            updateApplBlockCrc();
        }
    }
    else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)
    {
        incrementOperationCnt = TRUE;
    }
    else if(bUpApertureCmdRecd && uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterApertureHeight)
    {
        incrementOperationCnt = TRUE;
		bUpApertureCmdRecd = TRUE;
    }
	#endif


	//Operation count increment logic when shutter is at upper limit (Reaches Upper limit from Lower Limit) is commented - YG - Nov 15
	#if 0
    if(
		(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit) ||
		(bUpApertureCmdRecd && uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterApertureHeight)
	  )
    {
        if(incrementOperationCnt)
        {
            incrementOperationCnt = FALSE;
            uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600++;
			//uDriveApplBlockEEP.stEEPDriveApplBlock.maintenanceCountValue_A636++;

            //rampStatusFlags.saveParamToEeprom = TRUE;

            if(bUpApertureCmdRecd)
            {
                bUpApertureCmdRecd = FALSE;
                uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604++;
                 // if auto-correction is allowed for aperture operation
                if(uDriveApplBlockEEP.stEEPDriveApplBlock.autoCorrectionEnabled_A127)
				{
					// if aperture count exceeds the EEPROM parameter -- then trigger a runtime calibration operation,
					// at the end of runtime calibration, the shutter should come to rest at the aperture height instead of upper limit
// 2017/3/8 by IME
//					if(uDriveApplBlockEEP.stEEPDriveApplBlock.correctedFreqAperture_A126 <= uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604)
					if((UINT32)uDriveApplBlockEEP.stEEPDriveApplBlock.correctedFreqAperture_A126 <= uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604)
					{
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.driveCalibrationFailed = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration = TRUE;
                        //set the calibration state to search origin in upward direction
						//	Added on 03 FEB 2015 to implement user control on power up calibration
                        //powerUpCalib.currentState = CALIB_SEARCH_ORG_UP_DIR;
						powerUpCalib.currentState = CALIB_STATE_START;
						powerUpCalibrationCurrentState = CALIB_SEARCH_ORG_UP_DIR;
                        //Set calib target position as upper limit
                        powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
                        //reset calib os toggle status
                        powerUpCalib.osToggle = 0;
                        //Start shutter jog up movement
						//	Added on 03 FEB 2015 to implement user control on power up calibration
                        //inputFlags.value = OPEN_SHUTTER_JOG_50;
						inputFlagsValue = OPEN_SHUTTER_JOG_50;
                        powerOnCalibration = INITIATED;
					}
				}
            }
            updateApplBlockCrc();
        }
    }
    else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)
    {
        incrementOperationCnt = TRUE;
    }
	#endif


	//Operation count increment logic when shutter is at upper limit (Reaches Upper limit from any position) is implemneted - YG - Nov 15
	#if 1

	//	Initial value set as TRUE to enable increment of operationCount_A600 when shutter is
	//	powered up when it is at lower limit and then moved to upper limit. - JAN 2016
	static BOOL sShutterAtUpperLimitFlag = TRUE;
	//	Added to handle aperture correction when operation count is incremented at upper limit - Jan 2016
	static BOOL lsbStartApertureCorrection = FALSE;

    if(
		(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit) ||
		(bUpApertureCmdRecd && uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterApertureHeight)
	  )
    {

		// Capture shutter is at limit situation
		if (!sShutterAtUpperLimitFlag && uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)
		{

			sShutterAtUpperLimitFlag = TRUE;

		} // if (!sShutterAtUpperLimitFlag && uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)


        if(incrementOperationCnt)
        {
            incrementOperationCnt = FALSE;
            uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600++;
			//uDriveApplBlockEEP.stEEPDriveApplBlock.maintenanceCountValue_A636++;

            //rampStatusFlags.saveParamToEeprom = TRUE;

            if(bUpApertureCmdRecd)
            {
                bUpApertureCmdRecd = FALSE;
                uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604++;
                 // if auto-correction is allowed for aperture operation
                if(uDriveApplBlockEEP.stEEPDriveApplBlock.autoCorrectionEnabled_A127)
				{
					// if aperture count exceeds the EEPROM parameter -- then trigger a runtime calibration operation,
					// at the end of runtime calibration, the shutter should come to rest at the aperture height instead of upper limit
// 2017/3/8 by IME
//					if((uDriveApplBlockEEP.stEEPDriveApplBlock.correctedFreqAperture_A126 - 1) <= uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604)    //bug_NO.12
					if(((UINT32)uDriveApplBlockEEP.stEEPDriveApplBlock.correctedFreqAperture_A126 - 1) <= uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604)    //bug_NO.12
					{
						//	Added to handle aperture correction when operation count is incremented at upper limit - Jan 2016
						//lsbStartApertureCorrection = TRUE;
                        FLAG_StartApertureCorrection= 1;  //bug_No.12
						//	Removed to handle aperture correction when operation count is incremented at upper limit - Jan 2016
						/*
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.driveCalibrationFailed = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration = TRUE;
                        //set the calibration state to search origin in upward direction
						//	Added on 03 FEB 2015 to implement user control on power up calibration
                        //powerUpCalib.currentState = CALIB_SEARCH_ORG_UP_DIR;
						powerUpCalib.currentState = CALIB_STATE_START;
						powerUpCalibrationCurrentState = CALIB_SEARCH_ORG_UP_DIR;
                        //Set calib target position as upper limit
                        powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
                        //reset calib os toggle status
                        powerUpCalib.osToggle = 0;
                        //Start shutter jog up movement
						//	Added on 03 FEB 2015 to implement user control on power up calibration
                        //inputFlags.value = OPEN_SHUTTER_JOG_50;
						inputFlagsValue = OPEN_SHUTTER_JOG_50;
                        powerOnCalibration = INITIATED;
						*/
					}
				}
            }

			// Operation count update in eeprom on every upper limit is commneted - YG - NOV 15
            //updateApplBlockCrc();
        }
    }
    else if(sShutterAtUpperLimitFlag && !uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)
    {
        incrementOperationCnt = TRUE;
		sShutterAtUpperLimitFlag = FALSE;
    }
	else if(bUpApertureCmdRecd && !uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterApertureHeight)
	//else if(bDownApertureCmdRecd && !uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterApertureHeight) //20161202 add
    {
        incrementOperationCnt = TRUE;
		//bDownApertureCmdRecd = FALSE;	//20161202 add
    }
	//	Added to handle aperture correction when operation count is incremented at upper limit - Jan 2016

	if(lsbStartApertureCorrection \
	   && uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterApertureHeight)	//20161202 add
//	if(lsbStartApertureCorrection \
//	   && uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)
	{
		lsbStartApertureCorrection = FALSE;

		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = FALSE;
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.driveCalibrationFailed = FALSE;
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration = TRUE;
		//set the calibration state to search origin in upward direction
		//	Added on 03 FEB 2015 to implement user control on power up calibration
		//powerUpCalib.currentState = CALIB_SEARCH_ORG_UP_DIR;
		powerUpCalib.currentState = CALIB_STATE_START;
		powerUpCalibrationCurrentState = CALIB_SEARCH_ORG_UP_DIR;
		//Set calib target position as upper limit
		powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
		//reset calib os toggle status
		powerUpCalib.osToggle = 0;
		//Start shutter jog up movement
		//	Added on 03 FEB 2015 to implement user control on power up calibration
		//inputFlags.value = OPEN_SHUTTER_JOG_50;
		inputFlagsValue = OPEN_SHUTTER_JOG_50;
		powerOnCalibration = INITIATED;
    }

	#endif

}

VOID checkShutterPosition(VOID)
{
    //Check shutter current position. if it is above or below upper/lower limit then perform
    //power up calibration
    SHORT positionError;

    //Determine shutter is above or below the origin sensor level accordingly start direction
    //in up or down direction
    if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129 >
       uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128)
    {
        //shutter is below the origin level
        //calculate position error, if it is out of range the trigger calibration.
        positionError = uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129 - \
            uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
        //To avoide negative calculation
        if(positionError < 0)
            positionError = -positionError;
        //if position error is more than overrun prevention then trigger power up calibration
        if(positionError >= uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112)
        {
            //If origin sensor level is not correct then do not start calibration
            if(!sensorList[ORIGIN_SENSOR].sensorCurrSteadyVal)
            {
                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration = TRUE;
                //set the calibration state to search origin in upward direction
				//	Added on 03 FEB 2015 to implement user control on power up calibration
                //powerUpCalib.currentState = CALIB_SEARCH_ORG_UP_DIR;
				powerUpCalib.currentState = CALIB_STATE_START;
				powerUpCalibrationCurrentState = CALIB_SEARCH_ORG_UP_DIR;
                //Set calib target position as upper limit
                powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
                //reset calib os toggle status
                powerUpCalib.osToggle = 0;
                //Start shutter jog up movement
				//	Added on 03 FEB 2015 to implement user control on power up calibration
                //inputFlags.value = OPEN_SHUTTER_JOG_50;
				inputFlagsValue = OPEN_SHUTTER_JOG_50;
            }
            else
            {
                //The system will come here when it has lost its position with respect to
                //therefore search origin sensor for full shutter length
                //reset shutter positions
                //uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129 = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
                //hallCounts = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;

                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration = TRUE;
                //set the calibration state to search origin in downward direction
				//	Added on 03 FEB 2015 to implement user control on power up calibration
                //powerUpCalib.currentState = CALIB_SEARCH_ORG_DN_DIR;
				powerUpCalib.currentState = CALIB_STATE_START;
				powerUpCalibrationCurrentState = CALIB_SEARCH_ORG_DN_DIR;
                //Set calib target position as lower limit
                powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;

                //reset calib os toggle status
                powerUpCalib.osToggle = 0;
                //Start shutter jog down movement
				//	Added on 03 FEB 2015 to implement user control on power up calibration
                //inputFlags.value = CLOSE_SHUTTER_JOG_50;
				inputFlagsValue = CLOSE_SHUTTER_JOG_50;
            }
        }
        else
        {
			//	It was observed that when installation is initiated by control board 3PBS at power on,
			//	driveInstallation and driveReady bits are 1 simultaneously and drive was entering into
			//	installation. This was resulting in false operation.
			//	Hence driveReady bit shall be set only if driveInstallation bit is 0.
			//	Added - Feb 2016
			if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
			{
				uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = TRUE;
			}
            //reset the calibration state machine
            powerUpCalib.currentState = CALIB_STATE_END;
            //reset input flag to ramp generator
            //inputFlags.value = STOP_SHUTTER;
			inputFlagsValue = STOP_SHUTTER;
        }
    }
    else
    {
        //shutter is above the origin level
        //calculate position error, if it is out of range the trigger calibration.
        positionError = uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129 - \
            uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
        //To avoide negative calculation
        if(positionError < 0)
            positionError = -positionError;
        //if position error is more than overrun prevention then trigger power up calibration
        if(positionError >= uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112)
        {
            //If origin sensor level is not correct then do not start calibration
            if(sensorList[ORIGIN_SENSOR].sensorCurrSteadyVal)
            {
                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration = TRUE;
                //set the calibration state to search origin in downward direction
				//	Added on 03 FEB 2015 to implement user control on power up calibration
                //powerUpCalib.currentState = CALIB_SEARCH_ORG_DN_DIR;
				powerUpCalib.currentState = CALIB_STATE_START;
				powerUpCalibrationCurrentState = CALIB_SEARCH_ORG_DN_DIR;
                //Set calib target position as origin sensor limit+500mm
                powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128 \
                    + HALL_COUNT(MAX_OS_SEARCH_LENGTH);
                //reset calib os toggle status
                powerUpCalib.osToggle = 0;
                //Start shutter jog down movement
				//	Added on 03 FEB 2015 to implement user control on power up calibration
                //inputFlags.value = CLOSE_SHUTTER_JOG_50;
				inputFlagsValue = CLOSE_SHUTTER_JOG_50;
            }
            else
            {
                //The system will come here when it has lost its position with respect to
                //therefore search origin sensor for full shutter length
                //reset shutter positions
                //uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129 = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
                //hallCounts = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;

                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration = TRUE;
                //set the calibration state to search origin in upward direction
				//	Added on 03 FEB 2015 to implement user control on power up calibration
                //powerUpCalib.currentState = CALIB_SEARCH_ORG_UP_DIR;
				powerUpCalib.currentState = CALIB_STATE_START;
				powerUpCalibrationCurrentState = CALIB_SEARCH_ORG_UP_DIR;
                //Set calib target position as upper limit
                powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
                //reset calib os toggle status
                powerUpCalib.osToggle = 0;
                //Start shutter jog up movement
				//	Added on 03 FEB 2015 to implement user control on power up calibration
                //inputFlags.value = OPEN_SHUTTER_JOG_50;
				inputFlagsValue = OPEN_SHUTTER_JOG_50;
            }
        }
        else
        {
			//	It was observed that when installation is initiated by control board 3PBS at power on,
			//	driveInstallation and driveReady bits are 1 simultaneously and drive was entering into
			//	installation. This was resulting in false operation.
			//	Hence driveReady bit shall be set only if driveInstallation bit is 0.
			//	Added - Feb 2016
			if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
			{
				uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = TRUE;
			}
            if(powerOnCalibration == TERMINATED)    //20170418  201703_No.22
            {
                        //stop calibration process
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = TRUE;
            }
            //reset the calibration state machine
            powerUpCalib.currentState = CALIB_STATE_END;
            //reset input flag to ramp generator
            //inputFlags.value = STOP_SHUTTER;
			inputFlagsValue = STOP_SHUTTER;
        }
    }
}

VOID powerUpCalibration(VOID)
{
    SHORT positionError;

    if(powerOnCalibration == INITIATED &&
	   (uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration ||
       uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration))
    {
        //read current position
        powerUpCalib.currentPosition = hallCounts;

        //Run power on calibration state machine
        switch(powerUpCalib.currentState)
        {
			case CALIB_STATE_START:
				{
					//	Added on 03 FEB 2015 to implement user control on power up calibration
					powerUpCalib.currentState = powerUpCalibrationCurrentState;
					inputFlags.value = inputFlagsValue;
				}
				break;
            case CALIB_SEARCH_ORG_UP_DIR:
                {
                    //If origin sensor toggle is detected then continue movement for 50mm
                    if(powerUpCalib.osToggle)
                    {
                        //update the target position and state machine
						//20180726 Bug_No94,No95,No97
                        //powerUpCalib.targetPosition = powerUpCalib.currentPosition - HALL_COUNT(OS_VALIDATION_LENGTH);
                        powerUpCalib.targetPosition = OS_VALIDATION_LENGTH;
                        powerUpCalib.currentState = CALIB_MOVE_UP_50MM;
                        //update input to ramp generator
                        inputFlags.value = OPEN_SHUTTER_JOG_50;
						//	Clear osToggle flag.
						//	Inserted to handle OS detection during shutter go down
						powerUpCalib.osToggle = 0;
                    }
                    else
                    {
                        //check if we have reached to upper limit then set calibration failed status
                        //if(powerUpCalib.currentPosition <= powerUpCalib.targetPosition)
                        if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)
                        {
                            //set clibration failed status and stop the shutter
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.driveCalibrationFailed = TRUE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osFailValidation = TRUE;
                            //stop calibration process
                            //reset the calibration state machine
                            powerUpCalib.currentState = CALIB_STATE_END;
                            //reset input flag to ramp generator
                            inputFlags.value = STOP_SHUTTER;
                        }
                    }

                    break;
                }
            case CALIB_SEARCH_ORG_DN_DIR:
                {
                    //If origin sensor toggle is detected then continue movement for 50mm
                    if(powerUpCalib.osToggle)
                    {
                        ////update the target position and state machine
                        //powerUpCalib.targetPosition = powerUpCalib.currentPosition - HALL_COUNT(OS_VALIDATION_LENGTH);
                        //powerUpCalib.currentState = CALIB_MOVE_UP_50MM;
                        ////update input to ramp generator
                        //inputFlags.value = OPEN_SHUTTER_JOG_50;

                        if(powerUpCalib.apertureCalib)
                        {
                            //update the target position and calibration state
                            powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130;
                            //Load the hall count as origin position
                            hallCounts = uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128;
                            powerUpCalib.currentState = CALIB_MOVE_TO_APP_LIMIT;
                            //update input to ramp generator
                            inputFlags.value = CLOSE_SHUTTER_JOG_50;
                        }
                        else
                        {
                            //update the target position and calibration state
                            powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
                            //Load the hall count as origin position
                            hallCounts = uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128;
                            powerUpCalib.currentState = CALIB_MOVE_TO_UP_LIMIT;
                            //update input to ramp generator
                            inputFlags.value = OPEN_SHUTTER_JOG_50;
                        }
						//	Clear osToggle flag.
						//	Inserted to handle OS detection during shutter go down
						powerUpCalib.osToggle = 0;
                    }
                    else
                    {
                        //check if we have reached to lower limit then set calibration failed status
                        //if(powerUpCalib.currentPosition >= powerUpCalib.targetPosition)
                        if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)
                        {
                            //set clibration failed status and stop the shutter
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.driveCalibrationFailed = TRUE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osFailValidation = TRUE;
                            //stop calibration process
                            //reset the calibration state machine
                            powerUpCalib.currentState = CALIB_STATE_END;
                            //reset input flag to ramp generator
                            inputFlags.value = STOP_SHUTTER;
                        }
                    }

                    break;
                }
            case CALIB_MOVE_UP_50MM:
                {
                    if(powerUpCalib.currentPosition <= powerUpCalib.targetPosition)
                    {
                        //update the target position and calibration state
                        powerUpCalib.targetPosition = powerUpCalib.currentPosition + HALL_COUNT(OS_VALIDATION_LENGTH);
                        powerUpCalib.currentState = CALIB_MOVE_DN_50MM;
                        //update input to ramp generator
                        inputFlags.value = CLOSE_SHUTTER_JOG_50;
                    }

                    break;
                }
            case CALIB_MOVE_DN_50MM:
                {
                    //if(powerUpCalib.currentPosition >= powerUpCalib.targetPosition)
					//	Check whether origin sensor is toggeled while shutter is going up
					if(powerUpCalib.osToggle)
                    {
                        if(powerUpCalib.apertureCalib)
                        {
                            //update the target position and calibration state
                            powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130;
                            //Load the hall count as origin position
                            hallCounts = uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128;
                            powerUpCalib.currentState = CALIB_MOVE_TO_APP_LIMIT;
                            //update input to ramp generator
                            inputFlags.value = CLOSE_SHUTTER_JOG_50;
                        }
                        else
                        {
                            //update the target position and calibration state
                            powerUpCalib.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
                            //Load the hall count as origin position
                            hallCounts = uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128;
                            powerUpCalib.currentState = CALIB_MOVE_TO_UP_LIMIT;
                            //update input to ramp generator
                            inputFlags.value = OPEN_SHUTTER_JOG_50;
                        }
						//	Clear osToggle flag.
						//	Inserted to handle OS detection during shutter go down
						powerUpCalib.osToggle = 0;
                    }

                    break;
                }
            case CALIB_MOVE_TO_UP_LIMIT:
            case CALIB_MOVE_TO_DN_LIMIT:
                {
                    //if ramp has reached to final position
                    if(rampCurrentState == RAMP_STATE_END)
                    {
                        positionError = powerUpCalib.currentPosition - powerUpCalib.targetPosition;
                        //to avoide negative calculation
                        if(positionError < 0)
                            positionError = -positionError;

                        //if shutter position is within accepatable range then stop calibration and end process
                        if(positionError <= uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112)
                        {
                            //shutter has reached to final position
                            //Calibration completed sucessful
                            //stop calibration process
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration = FALSE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration = FALSE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = TRUE;
                            //reset the calibration state machine
                            powerUpCalib.currentState = CALIB_STATE_END;
                            //reset input flag to ramp generator
                            inputFlags.value = STOP_SHUTTER;
							powerOnCalibration = SERVICED;
                        }
                        else
                        {
                                //set clibration failed status and stop the shutter
                                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.driveCalibrationFailed = TRUE;
                                //reset the calibration state machine
                                powerUpCalib.currentState = CALIB_STATE_END;
                                //reset input flag to ramp generator
                                inputFlags.value = STOP_SHUTTER;
                        }

                        //increment calibration operation count
                        powerUpCalib.operationCnt++;
                    }

                    break;
                }

            case CALIB_MOVE_TO_APP_LIMIT:
                {
                    if(powerUpCalib.currentPosition >= powerUpCalib.targetPosition)
                    {
                        //stop calibration process
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = TRUE;
                        //reset the calibration state machine
                        powerUpCalib.currentState = CALIB_STATE_END;
                        //reset input flag to ramp generator
                        inputFlags.value = STOP_SHUTTER;
						powerOnCalibration = SERVICED;
                    }
                   break;
                }

            default:
                break;
        }
    }
	//	Added on 03 FEB 2015 to implement user control on power up calibration
    else if(powerOnCalibration == TERMINATED &&
			uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterStopped == TRUE)
	{
		checkShutterPosition();
		powerOnCalibration = SERVICED;
	}
}

VOID checkShutterInstallation(VOID)
{
    BOOL triggerInstallation = FALSE;
    //check shutter upper limit, lower limit and ignore PE level.
    //If they are not in proper order then trigger installation
    ShutterInstallationStep = readBYTE(EEP_SHUTTER_INSTALLATION_STEP);
    if(ShutterInstallationStep == 0)
    {
        if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 <=
            uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100)
        {
            triggerInstallation = TRUE;
        }
        else if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102 <=
                uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100)
        {
            triggerInstallation = TRUE;
        }
        else if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102 >=
                uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101)
        {
            triggerInstallation = TRUE;
        }
        else
        {
            triggerInstallation = FALSE;
        }
    }
    else
    {
        triggerInstallation = TRUE;
    }

    if(triggerInstallation)
    {
        startInstallation();
    }
	else if(gucInstallationCalledFrom == 1)
	{
		gucInstallationCalledFrom = 0;
	}
}
VOID startApertureHeight(VOID)
{
    //set drive installation in progress and install A100 position
    //reset drive ready status
    //Set installation in progress status
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveApertureHeight = TRUE;
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation = TRUE;

    //Reset drive installation failed flag
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = FALSE;
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationSuccess = FALSE;
    //set current installation status bit
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA100 = FALSE;
    //Reset ready flag
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = FALSE;
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA130 = TRUE;
    //Set current installation state
    shutterInstall.currentState = INSTALL_A130;
	// Added to overcome installation issue (A100) - RN- NOV 2015
	//gucInstallationInitiated = INITIATED;
    ShutterInstallationEnabled = TRUE;
}
VOID startInstallation(VOID)
{
	if(gucInstallationCalledFrom == 1)
	{
		gucInstallationCalledFrom = 0;
	}
	if(gucInstallationCalledFrom == 2)
	{
		gucInstallationCalledFrom = 0;
	}
    //if power on calibration is true then clear it during installation
    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration == TRUE)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration = FALSE;
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration = FALSE;
    }

    //set drive installation in progress and install A100 position
    //reset drive ready status
    //Set installation in progress status
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation = TRUE;
    //Reset drive installation failed flag
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = FALSE;
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationSuccess = FALSE;
    //set current installation status bit
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA100 = TRUE;
    //Reset ready flag
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = FALSE;
    //Set current installation state
    shutterInstall.currentState = INSTALL_A100;
	// Added to overcome installation issue (A100) - RN- NOV 2015
	gucInstallationInitiated = INITIATED;
    ShutterInstallationEnabled = TRUE;

    writeBYTE(EEP_SHUTTER_INSTALLATION_STEP, INSTALL_A100);    //add 20200303
    ShutterInstallationStep = INSTALL_A100;                    //add 20200303
}

VOID shutterInstallation(VOID)
{
    SHORT positionError;

    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation && ShutterInstallationEnabled)
    {
        if((ShutterInstallationStepNeedSave)&&(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveApertureHeight==0))   //bug_NO.43
        {
            ShutterInstallationStep = readBYTE(EEP_SHUTTER_INSTALLATION_STEP);
            switch(ShutterInstallationStep)
            {
                case INSTALL_A100:
                {
                    shutterInstall.currentState = INSTALL_A100;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA100 = TRUE;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA101 = FALSE;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA102 = FALSE;
                    break;
                }
                case INSTALL_A101:
                {
                    shutterInstall.currentState = INSTALL_A101;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA100 = FALSE;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA101 = TRUE;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA102 = FALSE;
                    break;
                }
                case INSTALL_A102:
                {
                    shutterInstall.currentState = INSTALL_A102;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA100 = FALSE;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA101 = FALSE;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA102 = TRUE;
                    break;
                }
                default:
                {
                    shutterInstall.currentState = INSTALL_A100;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA100 = TRUE;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA101 = FALSE;
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA102 = FALSE;
                    break;
                }
            }
        }
        //read current position
        shutterInstall.currentPosition = hallCounts;

        //Run installation state machine
        switch(shutterInstall.currentState)
        {
            case INSTALL_A100:
                {
                    //If enter buttun is pressed then set hall counts to zero and reset A100 position
                    if(shutterInstall.enterCmdRcvd)
                    {
                            hallCounts = HALL_COUNTS_FOR_A100;
                            uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 = hallCounts;
                             writeWORD(EEP_UPPER_STOPPING_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100);
                            updateCommonBlockCrc();
                            ShutterInstallationStepNeedSave = FALSE;
                            writeBYTE(EEP_SHUTTER_INSTALLATION_STEP, INSTALL_A101);
                            shutterInstall.enterCmdRcvd = FALSE;
                            //clear previous installation status bit
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA100 = FALSE;
                            //set current installation status bit
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA101 = TRUE;
                            //Set current installation state
                            shutterInstall.currentState = INSTALL_A101;
                    }

                    break;
                }
            case INSTALL_A101:
                {
                    //If enter button is pressed then set A101 position equal to current hall counts
                    if(shutterInstall.enterCmdRcvd)
                    {
                        uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 = shutterInstall.currentPosition;
                        writeWORD(EEP_LOWER_STOPPING_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101);
                        updateCommonBlockCrc();
                        writeBYTE(EEP_SHUTTER_INSTALLATION_STEP, INSTALL_A102);
                        ShutterInstallationStepNeedSave = FALSE;
                        shutterInstall.enterCmdRcvd = FALSE;
                        //clear previous installation status bit
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA101 = FALSE;
                        //set current installation status bit
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA102 = TRUE;
                        //Set current installation state
                        shutterInstall.currentState = INSTALL_A102;
                    }

                    break;
                }
            case INSTALL_A102:
                {
                    //If enter button is pressed then set A102 position equal to current hall counts
                    if(shutterInstall.enterCmdRcvd)
                    {
                        uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102 = shutterInstall.currentPosition;
                        writeWORD(EEP_PHOTOELEC_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102);
                        updateCommonBlockCrc();
//                        writeBYTE(EEP_SHUTTER_INSTALLATION_STEP, INSTALL_A102);
                        ShutterInstallationStepNeedSave = FALSE;
                        shutterInstall.enterCmdRcvd = FALSE;
                        //clear previous installation status bit
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA102 = FALSE;
                        //set current installation status bit
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationValidation = TRUE;
                        //Set current installation state
                        shutterInstall.currentState = INSTALL_VERIFY;
                    }

                    break;
                }
            case INSTALL_VERIFY:
                {
                    //Verify above positions, If they are not in proper order then abort installation process
                    //and declare installation failed
                    //check photo electric sensor, if it is triggered then declare installation failed
                    UpperPostionLow = readBYTE(EEP_UPPER_STOPPING_POS);
                    UpperPostionHigh = readBYTE(EEP_UPPER_STOPPING_POS + 1);
                    uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 = (UpperPostionHigh << 8) | UpperPostionLow;
                    LowerPostionLow = readBYTE(EEP_LOWER_STOPPING_POS);
                    LowerPostionHigh = readBYTE(EEP_LOWER_STOPPING_POS + 1);
                    uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 = (LowerPostionHigh << 8) | LowerPostionLow;
                    if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 <=
                       uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100)
                    {
						//clear previous installation status bit
						//Added to handle display screen hang at "VALIDATING INSTALLATION" issue during installation fail - Dec 2015
						uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationValidation = FALSE;
                        //set drive installation failed and abort installation process
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = TRUE;
                        //reset the installation state machine
                        shutterInstall.currentState = INSTALL_STATE_END;
                        ShutterInstallationStepNeedSave = FALSE;
                        //reset input flag to ramp generator
                        inputFlags.value = STOP_SHUTTER;
                    }
                    else if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102 <=
                            uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100)
                    {
						//clear previous installation status bit
						//Added to handle display screen hang at "VALIDATING INSTALLATION" issue during installation fail - Dec 2015
						uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationValidation = FALSE;
                        //set drive installation failed and abort installation process
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = TRUE;
                        //reset the installation state machine
                        shutterInstall.currentState = INSTALL_STATE_END;
                        ShutterInstallationStepNeedSave = FALSE;
                        //reset input flag to ramp generator
                        inputFlags.value = STOP_SHUTTER;
                    }
                    else if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102 >=
                            uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101)
                    {
						//clear previous installation status bit
						//Added to handle display screen hang at "VALIDATING INSTALLATION" issue during installation fail - Dec 2015
						uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationValidation = FALSE;
                        //set drive installation failed and abort installation process
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = TRUE;
                        //reset the installation state machine
                        shutterInstall.currentState = INSTALL_STATE_END;
                        ShutterInstallationStepNeedSave = FALSE;
                        //reset input flag to ramp generator
                        inputFlags.value = STOP_SHUTTER;
                    }
					// PE Sensor validation removed during installation validation - YG - Nov 2015
					/*
                    else if(photoElecObsSensTrigrd)
                    {
                        //Set photo electric fault
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.peObstacle = TRUE;
                        //set drive installation failed
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = TRUE;
                    }
					*/
                    else
                    {
                        //Check if system has recovered from photoelectric fault
                        if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.peObstacle)
                        {
                            //Clear photo electric fault
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.peObstacle = FALSE;
                            //Clear drive installation failed
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = FALSE;
                        }
                        //verification sucessful, now perform validation
                        shutterInstall.currentState = INSTALL_SEARCH_ORG;
                        ShutterInstallationStepNeedSave = FALSE;
                        //Set installation target position as upper limit
                        shutterInstall.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
                        //reset installation os toggle status
                        shutterInstall.osToggle = 0;
                        //Start shutter jog up movement
                        inputFlags.value = OPEN_SHUTTER_JOG_50;
                        inputFlags_Installation.value = inputFlags.value;
                    }

                    break;
                }
            case INSTALL_SEARCH_ORG:
                {
                    //Search origin sensor in upward direction. if origin sensor is not detected and we reached to
                    //upper limit then abort installation process and declare installation failed and origin
                    //sensor not detected
                    if(shutterInstall.osToggle)
                    {
						//	Clear osToggle flag.
						//	Inserted to handle OS detection during shutter go down
						shutterInstall.osToggle = 0;
                        ShutterInstallationStepNeedSave = FALSE;
                        //update the target position and state machine
						//20180726 Bug_No94,No95,No97
                        //shutterInstall.targetPosition = shutterInstall.currentPosition - HALL_COUNT(OS_VALIDATION_LENGTH);
                        shutterInstall.targetPosition = OS_VALIDATION_LENGTH;
                        shutterInstall.currentState = INSTALL_MOVE_UP_50MM;
                        //update input to ramp generator
                        inputFlags.value = OPEN_SHUTTER_JOG_50;
                        inputFlags_Installation.value = inputFlags.value;
                    }
                    else
                    {
                        //check if we have reached to upper limit then set installation failed status
                        //20180726 Bug_No94,No95,No97
                        //if(shutterInstall.currentPosition <= shutterInstall.targetPosition)
                        if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)
                        {
                            //set drive installation failed and abort installation process
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = TRUE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osFailValidation = TRUE;
                            //reset the installation state machine
                            ShutterInstallationStepNeedSave = FALSE;
                            shutterInstall.currentState = INSTALL_STATE_END;

                            //reset input flag to ramp generator
                            inputFlags.value = STOP_SHUTTER;
                            inputFlags_Installation.value = inputFlags.value;
                        }
                    }

                    break;
                }
            case INSTALL_MOVE_UP_50MM:
                {
                    //Move the shutter to 50mm above origin sensor
                    if(shutterInstall.currentPosition <= shutterInstall.targetPosition)
                    {
                        //update the target position and installation state
                        shutterInstall.targetPosition = shutterInstall.currentPosition + HALL_COUNT(OS_VALIDATION_LENGTH);
                        ShutterInstallationStepNeedSave = FALSE;
                        shutterInstall.currentState = INSTALL_MOVE_DN_50MM;
                        //update input to ramp generator
                        inputFlags.value = CLOSE_SHUTTER_JOG_10;
                        inputFlags_Installation.value = inputFlags.value;
                    }

                    break;
                }
            case INSTALL_MOVE_DN_50MM:
                {
                    //Move shutter to 50mm down from current position. Set this current position as origin sensor
                    //level position A128
                    //if(shutterInstall.currentPosition >= shutterInstall.targetPosition)
					//	Check whether origin sensor is toggeled while shutter is going up
					if(shutterInstall.osToggle)
                    {
						//	Clear osToggle flag.
						shutterInstall.osToggle = 0;
                        //update the target position and installation state
                        //shutterInstall.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
                        //Load the current position in A128
                        uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128 = shutterInstall.currentPosition;
                        //update the target position and installation state
                        shutterInstall.targetPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128 \
                            - HALL_COUNT(MAX_OS_SEARCH_LENGTH);
                        ShutterInstallationStepNeedSave = FALSE;
                        shutterInstall.currentState = INSTALL_MOVE_TO_UP_LIMIT;
                        //update input to ramp generator
                        inputFlags.value = OPEN_SHUTTER_JOG_50;
                        inputFlags_Installation.value = inputFlags.value;
                    }

                    break;
                }
            case INSTALL_MOVE_TO_UP_LIMIT:
//            case INSTALL_MOVE_TO_DN_LIMIT:
                {
                    //move the shutter 500mm up to detect upper limit. if upper limit is not detected within this
                    //range then set calibration faliled error flag
                    if(shutterInstall.currentPosition < shutterInstall.targetPosition)
                    {
                        //clear previous installation status bit
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationValidation = FALSE;
                        //set drive installation failed and abort installation process
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = TRUE;
                        //reset the installation state machine
                        ShutterInstallationStepNeedSave = FALSE;
                        shutterInstall.currentState = INSTALL_STATE_END;
                        //reset input flag to ramp generator
                        inputFlags.value = STOP_SHUTTER;
                        inputFlags_Installation.value = inputFlags.value;
                    }

                    //if ramp has reached to final position
                    if(rampCurrentState == RAMP_STATE_END)
                    {
                        positionError = shutterInstall.currentPosition - uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
                        //to avoide negative calculation
                        if(positionError < 0)
                            positionError = -positionError;

                        //if shutter position is within accepatable range then stop calibration and end process
                        /*if(positionError <= uDriveApplBlockEEP.stEEPDriveApplBlock.overrunProtection_A112)*/
						//limit fail 2016/08/04 by IME
						//if(positionError <= 10)
						if(positionError <= 100)
                        {
                            //shutter has reached to final position
                            //clear previous installation status bit
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationValidation = FALSE;
                            //set current installation status bit
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = FALSE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationSuccess = TRUE;

                            shutterInstall.currentState = INSTALL_SUCCESSFUL;
                            ShutterInstallationStepNeedSave = FALSE;
                            //reset input flag to ramp generator
                            inputFlags.value = STOP_SHUTTER;
                            inputFlags_Installation.value = inputFlags.value;
                        }
                        else
                        {
                            //clear previous installation status bit
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationValidation = FALSE;
                            //set drive installation failed and abort installation process
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = TRUE;
                            //reset the installation state machine
                            shutterInstall.currentState = INSTALL_STATE_END;
                            ShutterInstallationStepNeedSave = FALSE;
                            //reset input flag to ramp generator
                            inputFlags.value = STOP_SHUTTER;
                            inputFlags_Installation.value = inputFlags.value;
                        }
                    }

                    break;
                }

            case INSTALL_SUCCESSFUL:
                {
                    //If enter button is pressed then set installation sucessful
                    if(shutterInstall.enterCmdRcvd)
                    {
                        //Installation completed sucessful
                        //stop installation process
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = TRUE;
                        //reset the calibration state machine
                        shutterInstall.currentState = INSTALL_COMPLETE;

                        if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.upStepCount_A525 == TWO_STEP_RAMP_PROFILE)
                        {
                            uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos1_A103 \
                                = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 + RISE_GEAR_POS2_OFFSET;
                            uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos2_A104 \
                                = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 + RISE_GEAR_POS3_OFFSET;

                            uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos1_A106 \
                            = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 - FALL_GEAR_POS2_OFFSET;
                            uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos2_A107 \
                            = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 - FALL_GEAR_POS3_OFFSET;

                            writeWORD(EEP_RISE_CHANGE_GEAR_POS1, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos1_A103);
                            writeWORD(EEP_RISE_CHANGE_GEAR_POS2, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos2_A104);
                            writeWORD(EEP_FALL_CHANGE_GEAR_POS1, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos1_A106);
                            writeWORD(EEP_FALL_CHANGE_GEAR_POS2, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos2_A107);
                            updateMotorBlockCrc();
                        }
                        else
                        {
                            uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos1_A103 \
                                = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 + RISE_GEAR_POS1_OFFSET;
                            uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos2_A104 \
                                = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 + RISE_GEAR_POS2_OFFSET;
                            uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos3_A105 \
                                = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 + RISE_GEAR_POS3_OFFSET;

                            uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos1_A106 \
                            = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 - FALL_GEAR_POS1_OFFSET;
                            uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos2_A107 \
                            = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 - FALL_GEAR_POS2_OFFSET;
                            uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos3_A108 \
                            = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 - FALL_GEAR_POS3_OFFSET;

                            writeWORD(EEP_RISE_CHANGE_GEAR_POS1, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos1_A103);
                            writeWORD(EEP_RISE_CHANGE_GEAR_POS2, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos2_A104);
                            writeWORD(EEP_RISE_CHANGE_GEAR_POS3, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos3_A105);
                            writeWORD(EEP_FALL_CHANGE_GEAR_POS1, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos1_A106);
                            writeWORD(EEP_FALL_CHANGE_GEAR_POS2, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos2_A107);
                            writeWORD(EEP_FALL_CHANGE_GEAR_POS3, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos3_A108);
                            updateMotorBlockCrc();
                        }
                        //After sucessfull installation update shutter positions to EEPROM
//                        writeWORD(EEP_UPPER_STOPPING_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100);
//                        writeWORD(EEP_LOWER_STOPPING_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101);
//                        writeWORD(EEP_PHOTOELEC_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102);
                        writeWORD(EEP_ORIGIN_SENS_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128);
                        //update CRC
                        updateCommonBlockCrc();
                        shutterInstall.enterCmdRcvd = FALSE;
                        //increment installation operation count
                        shutterInstall.operationCnt++;
                        ShutterInstallationStepNeedSave = FALSE;
                        writeBYTE(EEP_SHUTTER_INSTALLATION_STEP, 0);
                    }

                    break;
                }
            case INSTALL_STATE_END:
            {
                if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed)
                {
//                    delayMs(8888);
                    if(StopCnt)
                    {
                        InstallStopCnt = InstallCnt;
                        StopCnt = FALSE;
                    }
                    else
                    {
                        if((InstallCnt - InstallStopCnt) >= 3000)
                        {
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = FALSE;
//                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation = FALSE;
                            shutterInstall.currentState = INSTALL_A100;
                            writeBYTE(EEP_SHUTTER_INSTALLATION_STEP, 0);
                            ShutterInstallationStepNeedSave = TRUE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA100 = TRUE;
                            StopCnt = TRUE;
                        }
                    }

//                    InstallCnt++;
//                    if(InstallCnt >= 188888)
//                    {
//                        InstallCnt = 0;
//                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationFailed = FALSE;
//                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation = FALSE;
//                        shutterInstall.currentState = INSTALL_A100;
//                    }
//                    else
//                    {
//                        shutterInstall.currentState = INSTALL_STATE_END;
//                    }
                }
                break;
            }

            case INSTALL_A130:
                if(shutterInstall.enterCmdRcvd)
                {
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveApertureHeight = FALSE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady = TRUE;
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA130 = FALSE;

                        shutterInstall.enterCmdRcvd = FALSE;
                        //reset the calibration state machine
                        shutterInstall.currentState = INSTALL_COMPLETE;
                        ShutterInstallationEnabled = FALSE;
                }
                break;
            default:
                break;
        }
    }
}
