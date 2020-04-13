/********************************************************************************
* FileName: Extern.h
* Description:
* This header file declares common variables used by different modules.
*********************************************************************************/

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
 *  09/04/2014            iGate          Initial Creation
*****************************************************************************/
#include "./Common/Typedefs/Typedefs.h"

EXTERN SHORT refSpeed;
EXTERN SHORT measuredSpeed;
EXTERN DWORD period;
EXTERN SHORT controlOutput;
EXTERN SHORT ctrlOpPercent;
EXTERN BYTE currentDirection;
EXTERN BYTE requiredDirection;
EXTERN WORD phase;
EXTERN WORD phaseCopy;
EXTERN SHORT phaseIncFlg;
EXTERN WORD hallValue;
EXTERN SHORT sector;
EXTERN CHAR sectorTable[];
EXTERN CONST SHORT phaseValues[];
EXTERN SHORT phaseOffsetCW;
EXTERN SHORT phaseOffsetCCW;
EXTERN SHORT adcTrigger;
EXTERN SHORT iTotalADCCnt;
EXTERN SHORT iTotalOffset;
EXTERN DWORD iTotalInstFilter;
EXTERN WORD currentAverage;
EXTERN WORD measurediTotal;
EXTERN SHORT hallCounts;
EXTERN SHORT hallCounts_bak;
EXTERN SHORT refiTotalCurrent;
EXTERN DWORD iTotalADCCntAcc;
EXTERN WORD iTotalADCCntAccSmpls;
EXTERN BOOL calcTotalCurrentFlag;
EXTERN SHORT rampCurrentPosition;
EXTERN SHORT rampCurrentSpeed;
EXTERN SHORT rampCurrentState;

EXTERN BOOL emergencySensorTrigrd;
EXTERN BOOL microSwSensorTrigrd;
EXTERN BOOL airSwitchTrigrd;
EXTERN BOOL wrapAroundSensor;
EXTERN BOOL photoElecObsSensTrigrd;
EXTERN BOOL tempSensTrigrd;
EXTERN BOOL fourPtLmtSwtchDetected;
EXTERN BOOL originSensorDetected;
EXTERN BOOL powerFailSensorDetected;
EXTERN BOOL updateSenStsCmd;
EXTERN WORD txResetCount;
EXTERN SHORT currentSectorNo;
EXTERN SHORT phaseInc;
EXTERN SHORT currentLimitClamp;
EXTERN WORD feedbackCurrent;
EXTERN SHORT outputDecRate;
EXTERN BOOL pwmCostingReq;
EXTERN WORD phaseValue;
extern WORD systemTick;
//	Added on 3 Feb 2015 to implement user control on power on calibration
EXTERN BYTE powerOnCalibration;
// Added to overcome installation issue (A100) - RN- NOV 2015
EXTERN BYTE gucInstallationInitiated;
EXTERN SHORT currShutterType;
//	Added this flag to handle issue related to stop action (stopping was not smooth) when stop key is pressed
extern BYTE gui8StopKeyPressed;

//	Shutter false direction movement monitoring flags

#define DEBUG_SHUTTER_MOVEMENT_IN_WRONG_DIRECTION

// Added for displaying errors on display screen in case of false movement - RN - NOV 2015
//	Flag to monitor shutter fall
extern BYTE gucShutterFalseUpMovementCount;
extern BYTE gucShutterFalseDownMovementCount;

//	Added for implementation of power fail functionality on DC Bus for version 4 board- RN- NOV 2015
EXTERN BYTE gucPowerFailFlag;
EXTERN BYTE gucPowerRestoredFlag;
EXTERN BYTE gucInstallationCalledFrom;

#ifdef DEBUG_SHUTTER_MOVEMENT_IN_WRONG_DIRECTION
//	Debug count for false direction movement
extern BYTE gucTempFalseMovementCount;
#endif


#define	LED_RED			PORTAbits.RA7
#define	LED_YELLOW		PORTCbits.RC0

EXTERN UINT16 PhaseAdvance;

EXTERN int cnt_motor_stop;
EXTERN UINT8  ShutterInstallationStep;
EXTERN BOOL ShutterInstallationEnabled;

EXTERN UINT8 Power_ON_igbtOverTemp;
EXTERN UINT16 Time_uart_count; 
EXTERN UINT8 Flag_powerUpCalib_osToggle;

//define IGBT type    
//#define IGBT_LowActive_IR
#define IGBT_HighActive_ROME

//define Correction No.
#define  BUG_No41_microSwSensorTrigrd             //20170606  201703_No.41
#define  BUG_No81_microSwSensor_PowerON          //20170605  201703_No.81
#define  BUG_No82_UartRxTimeOut1S                //20170606  201703_No.82
#define  BUG_No83_igbtOverTemp                   //20170606  201703_No.83
#define  BUG_No76or73_powerUpCalib_osToggle      //20170607  201703_No.76 or 73
#define  BUG_No79_FaultPeObstacle                //20170608  201703_No.79
#define  BUG_No51_SnowA008                       //20170612  201703_No.51
#define  BUG_NoCQ07_Limit_enterCmdRcvd           //20170627  201703_No.CQ07
#define  BUG_No89_PHOTOELECTRIC_SENSOR       //priority5    //20171016  201703_No.89
#define  BUG_No88_M2overcurrentfault         //priority2    //20171029  201703_No.88
#define  BUG_CQxx_BD_IGBTdamage              //priority1    //20171029  BD IGBT damage

