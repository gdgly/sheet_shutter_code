/*********************************************************************************
* FileName: RampGenerator.c
* Description:
* This source file contains the definition of all the functions for RampGenerator.
* It implements all the functions required by ramp generator.
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
 *  09/04/2014            iGate          Initial Creation                                                               
*****************************************************************************/
#include <p33Exxxx.h>
#include "RampGenerator.h"
#include "InchUpDown.h"
#include "JogUpDown.h"
#include "RampUpDown.h"
#include "ApertureUpDown.h"
#include "./Common/UserDefinition/Userdef.h"
#include "./Common/Delay/Delay.h"
#include "./Common/Extern/Extern.h"
#include "./MotorControl/Algorithm/svm.h"
#include "./MotorControl/SpeedController/SpeedController.h"
#include "./MotorControl/CurrentController/CurrentController.h"
#include "./MotorControl/Braking/DCInjection.h"
#include "./DMCI/RTDMUSER.h"
#include "./DMCI/RTDM.h"
#include "./DMCI/RtdmInterface.h"
#include "./Middleware/ParameterDatabase/eeprom.h"
#include "./Application/Application.h"
#include "./Application/CommandHandler.h"
#include "./MotorControl/CurrentController/CurrentLimit.h"

#define CHARGE_BOOTSTRAP_CAP    1

#define TIMER4_PERIOD   10  //10ms
#define RAMP_GENERATOR_PERIOD_CNT   (RAMP_GENERATOR_TIME_PERIOD/TIMER4_PERIOD)

#define SHUTTER_CURRENT_MIN     600     //mA
#define SHUTTER_CURRENT_MAX     17000   //mA

#define SHUTTER_LOAD_HOLDING_TIME   3000
#define SHUTTER_LOAD_HOLDING_TIME_CNT    (SHUTTER_LOAD_HOLDING_TIME/RAMP_GENERATOR_TIME_PERIOD)

#define RAMP_STARTING_CURRENT_MAX   5000       //Limit starting current to 10A
#define RAMP_STARTING_CURRENT_MIN   1000       //Limit starting current to 10A
#define RAMP_STARTING_CURRENT_STEP  500         //Current change step is 200mA
#define RAMP_STARTING_SPEED_MAX     3600        //Limit starting speed to 3600rpm

#define MECHANICAL_LOCK_ACTIVATION_DELAY    100//500//1000    //Time delay required to energize mechanical lock
#define MECHANICAL_LOCK_DEACTIVATION_DELAY  50      //Time delay required to de-energize mechanical lock
#define MECHANICAL_LOCK_ACTIVATION_DELAY_CNT    (MECHANICAL_LOCK_ACTIVATION_DELAY/RAMP_GENERATOR_TIME_PERIOD)
#define MECHANICAL_LOCK_DEACTIVATION_DELAY_CNT  (MECHANICAL_LOCK_DEACTIVATION_DELAY/RAMP_GENERATOR_TIME_PERIOD)

//a = start speed, b = end speed, c = change rate in ms            
#define ACCELARATION_RATE(a,b,c)     __builtin_divud(((DWORD)(b-a)*RAMP_GENERATOR_TIME_PERIOD),c);   
#define DECELARATION_RATE(a,b,c)     __builtin_divud(((DWORD)(a-b)*RAMP_GENERATOR_TIME_PERIOD),c); 

#define FAN_ON_TIME                 600000UL//1000ms*60*10 = 10 minute
#define FAN_ON_TIME_CNT             (FAN_ON_TIME/RAMP_GENERATOR_TIME_PERIOD)

#define SAFETY_SENSOR_JOG_SPEED_PERCENT     50  //when safety sensor is trigger then jog speed will change to
                                                //50% of S1UP speed.

//	Maximum count allowed if shutter is moving in wrong direction
#define	MAX_FALSE_MOVEMENT_COUNT_LIMIT		54  // Number of hall pulses / mechanical revolution  X allowed false revolution 
												// 18 (750W motor) X 3

/* Enumaration for ramp profile */
typedef enum rampProfileNo
{
    RAMP_INCH_UP_PROFILE,
    RAMP_INCH_DN_PROFILE,
    RAMP_JOG_UP_PROFILE,
    RAMP_JOG_DN_PROFILE,
    RAMP_APERTURE_UP_PROFILE,
    RAMP_APERTURE_DN_PROFILE,
    RAMP_GOING_UP_PROFILE,
    RAMP_GOING_DN_PROFILE,
    RAMP_PROFILE_END
}rampProfileNo_en;

//Declare array of sensors
#ifndef PROGRAMMABLE_DEBOUNCE
safetySensors_t sensorList[SAFETY_SENSOR_END] = 
{
    {&readEmergencySensorSts,0,0,0,0,0,0,0,&emergencySensorSwTriggered},         //EMERGENCY_SENSOR
    {&readMicroSwSensorSts,0,0,0,0,0,0,0,&microSwSensorTiggered},      //MICRO_SW_SENSOR
    //{&readAirSwSensorSts,0,0,0,0,0,0,0,&microSwSensorTiggered},        //AIR_SW_SENSOR
    //{&readWrapSwSensorSts,0,0,0,0,0,0,0,&microSwSensorTiggered},       //WRAP_SW_SENSOR
    {&readPhotoElecSensorSts,0,0,0,0,0,0,0,&checkPhotoElecObsLevel},   //PHOTOELECTRIC_SENSOR
    {&readTemperatureSensorSts,0,0,0,0,0,0,0,&tempSensorTriggered},    //TEMPERATURE_SENSOR
    {&readOriginSensorSts,0,0,0,0,0,0,0,&calculateDrift},              //ORIGIN_SENSOR
    {&readFourPtSensorSts,0,0,0,0,0,0,0,&fourPtLimitSwTriggered},      //FOUR_PT_SW_SENSOR
    {&readPowerFailSensorSts,0,0,0,0,0,0,0,&tempSensorTriggered},      //POWER_FAIL_SW_SENSOR    
};
#else
//	CHANGE FOLLOWING PARAMETERS TO ADJUST "SENSOR ACTIVE DEBOUNCE VALUES" (all values in mS)
#define	EMERGENCY_SENSOR_ACTIVE_DEBOUNCE		5
#define	MICRO_SW_SENSOR_ACTIVE_DEBOUNCE			5
//AIR_SW_SENSOR_ACTIVE_DEBOUNCE
//WRAP_SW_SENSOR_ACTIVE_DEBOUNCE
#define	PHOTOELECTRIC_SENSOR_ACTIVE_DEBOUNCE	5
#define	TEMPERATURE_SENSOR_ACTIVE_DEBOUNCE		5
#define	ORIGIN_SENSOR_ACTIVE_DEBOUNCE			1//5	//	Reduced to handle "offset at upper & lower limit"
#define	FOUR_PT_SW_SENSOR_ACTIVE_DEBOUNCE		5
#define	POWER_FAIL_SW_SENSOR_ACTIVE_DEBOUNCE	5
#define	SAFETY_SENSOR_END_ACTIVE_DEBOUNCE		5

//	CHANGE FOLLOWING PARAMETERS TO ADJUST "SENSOR INACTIVE DEBOUNCE VALUES" (all values in mS)
#define	EMERGENCY_SENSOR_INACTIVE_DEBOUNCE		5
#define	MICRO_SW_SENSOR_INACTIVE_DEBOUNCE		5
//AIR_SW_SENSOR_INACTIVE_DEBOUNCE
//WRAP_SW_SENSOR_INACTIVE_DEBOUNCE
#define	PHOTOELECTRIC_SENSOR_INACTIVE_DEBOUNCE	5
#define	TEMPERATURE_SENSOR_INACTIVE_DEBOUNCE	5
#define	ORIGIN_SENSOR_INACTIVE_DEBOUNCE			1//5	//	Reduced to handle "offset at upper & lower limit"
#define	FOUR_PT_SW_SENSOR_INACTIVE_DEBOUNCE		5
#define	POWER_FAIL_SW_SENSOR_INACTIVE_DEBOUNCE	5
#define	SAFETY_SENSOR_END_INACTIVE_DEBOUNCE		5

//	Ingnore false debounce time
#define	FALSE_DEBOUNCE_TIME					5//20	//	Reduced to handle "offset at upper & lower limit"

safetySensors_t sensorList[SAFETY_SENSOR_END] = 
{
    {&readEmergencySensorSts,0,0,0,0,0,0,0,0,0,&emergencySensorSwTriggered},         //EMERGENCY_SENSOR
    {&readMicroSwSensorSts,0,0,0,0,0,0,0,0,0,&microSwSensorTiggered},      //MICRO_SW_SENSOR
    //{&readAirSwSensorSts,0,0,0,0,0,0,0,0,0,&microSwSensorTiggered},        //AIR_SW_SENSOR
    //{&readWrapSwSensorSts,0,0,0,0,0,0,0,0,0,&microSwSensorTiggered},       //WRAP_SW_SENSOR
    {&readPhotoElecSensorSts,0,0,0,0,0,0,0,0,0,&checkPhotoElecObsLevel},   //PHOTOELECTRIC_SENSOR
    {&readTemperatureSensorSts,0,0,0,0,0,0,0,0,0,&tempSensorTriggered},    //TEMPERATURE_SENSOR
    {&readOriginSensorSts,0,0,0,0,0,0,0,0,0,&calculateDrift},              //ORIGIN_SENSOR
    {&readFourPtSensorSts,0,0,0,0,0,0,0,0,0,&fourPtLimitSwTriggered},      //FOUR_PT_SW_SENSOR
    {&readPowerFailSensorSts,0,0,0,0,0,0,0,0,0,&tempSensorTriggered},      //POWER_FAIL_SW_SENSOR    
};

SHORT sensorActiveDebounceValue[SAFETY_SENSOR_END] = 
{
	EMERGENCY_SENSOR_ACTIVE_DEBOUNCE,
	MICRO_SW_SENSOR_ACTIVE_DEBOUNCE,
	//AIR_SW_SENSOR_ACTIVE_DEBOUNCE,
	//WRAP_SW_SENSOR_ACTIVE_DEBOUNCE,
	PHOTOELECTRIC_SENSOR_ACTIVE_DEBOUNCE,
	TEMPERATURE_SENSOR_ACTIVE_DEBOUNCE,
	ORIGIN_SENSOR_ACTIVE_DEBOUNCE,
	FOUR_PT_SW_SENSOR_ACTIVE_DEBOUNCE,
	POWER_FAIL_SW_SENSOR_ACTIVE_DEBOUNCE,
};

SHORT sensorInactiveDebounceValue[SAFETY_SENSOR_END] = 
{
	EMERGENCY_SENSOR_INACTIVE_DEBOUNCE,
	MICRO_SW_SENSOR_INACTIVE_DEBOUNCE,
	//AIR_SW_SENSOR_INACTIVE_DEBOUNCE,
	//WRAP_SW_SENSOR_INACTIVE_DEBOUNCE,
	PHOTOELECTRIC_SENSOR_INACTIVE_DEBOUNCE,
	TEMPERATURE_SENSOR_INACTIVE_DEBOUNCE,
	ORIGIN_SENSOR_INACTIVE_DEBOUNCE,
	FOUR_PT_SW_SENSOR_INACTIVE_DEBOUNCE,
	POWER_FAIL_SW_SENSOR_INACTIVE_DEBOUNCE,
};
#endif	//	PROGRAMMABLE_DEBOUNCE

StatusFlags_t flags;
InputFlags_u inputFlags;

BOOL photElecSensorFault = FALSE;

/* Current mechanical motor direction of rotation Calculated in halls interrupts */
BYTE currentDirection;

/* Required mechanical motor direction of rotation, will have the same sign as the */
/* controlOutput variable from the Speed Controller */
BYTE requiredDirection;
SHORT currentRampProfileNo;
SHORT rampCurrentState;
rampOutputStatus_t rampOutputStatus;
rampStatusFlags_t rampStatusFlags;
rampTripSts_t rampTripSts;
rampStructure_t* currentRampProfilePtr;
rampStructure_t currentRampProfile;
SHORT rampTotalStates;
SHORT rampCurrentStep;
SHORT rampCurrentPosition;
SHORT rampCurrentSpeed;
SHORT rampCurrentTotCurr;
SHORT rampCurrentOpenloopDuty;
SHORT rampDcInjectionOnCounter;
SHORT lockActivationDelayCnt;
SHORT lockDeactivationDelayCnt;

// Acceleration and Decelaration logic updated to compute same based on S1 up and S1 down instead of Rated speed - YG - NOV 15
//SHORT accelaration;
//SHORT decelaration;

SHORT gs16UpAccelaration;
SHORT gs16UpDecelaration;
SHORT gs16DownAccelaration;
SHORT gs16DownDecelaration;

SHORT saveParamToEepromCnt;
BOOL updateSenStsCmd = FALSE;

BOOL shtrMinDistReqFlg = FALSE;
SHORT shtrMinDistVal;
SHORT dcInjDecVal;
SHORT dcInjIncVal;
BOOL pwmCostingReq = FALSE;
DWORD fanOnCnt;

// Added for displaying errors on display screen in case of false movement - RN - NOV 2015
//	Flag to monitor shutter fall
BYTE gucShutterFalseUpMovementCount = 0;
BYTE gucShutterFalseDownMovementCount = 0;

#ifdef DEBUG_SHUTTER_MOVEMENT_IN_WRONG_DIRECTION
//	Debug count to monitor movement of shutter in wrong direction
BYTE gucTempFalseMovementCount = 0;
#endif

VOID stopMotorUseCycle(VOID);


/******************************************************************************
 * initRampGenerator
 *
 *  This function initializes all the variables used by ramp generator.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/  
VOID initRampGenerator(VOID)
{   
    currentDirection = CW;
    requiredDirection = CW; 
    rampCurrentState = RAMP_STATE_END;
    rampStatusFlags.rampDcInjectionOn = 0;
    rampStatusFlags.rampBrakeOn = 1;
    rampStatusFlags.shutterOperationStart = 0;
    rampStatusFlags.shutterOperationComplete = 1;
    rampStatusFlags.rampSpeedControlRequired = 0;
    rampStatusFlags.rampCurrentControlRequired = 0;
    currentRampProfilePtr = NULL;
    rampTotalStates = 0;
    rampCurrentPosition = hallCounts;
    rampCurrentSpeed = 0;
    rampCurrentTotCurr = 0;
    rampCurrentOpenloopDuty = 0;
    rampDcInjectionOnCounter = 0;    
    rampOutputStatus.shutterCurrentPosition = hallCounts;
    rampOutputStatus.shutterMoving = 0;
    rampOutputStatus.shutterMovementDirection = currentDirection;
    currentRampProfileNo = RAMP_PROFILE_END;
    lockActivationDelayCnt = 0;
    lockDeactivationDelayCnt = 0;
    lockApply;
    rampStatusFlags.safetySensorTriggered = 0;
    rampStatusFlags.rampOpenInProgress = 0;
    rampStatusFlags.rampCloseInProgress = 0;
    rampStatusFlags.rampMaintainHoldingDuty = 0;  
    rampStatusFlags.rampDriftCalculated = 0;
    //rampStatusFlags.saveParamToEeprom = FALSE;
    
	// Acceleration and Decelaration logic updated to compute same based on S1 up and S1 down instead of Rated speed - YG - NOV 15
	gs16UpAccelaration = 0;
	gs16UpDecelaration = 0;
	gs16DownAccelaration = 0;
	gs16DownDecelaration = 0;
    
    rampTripSts.rampTripUpStarted = 0;
    rampTripSts.rampTripUpCompleted = 1;
    rampTripSts.rampTripDnStarted = 0;
    rampTripSts.rampTripDnCompleted = 1;
    rampTripSts.rampOrgSenToggled = 0;
}

VOID initProfileData(VOID)
{

	
    //calculate accelaration and decelaration rate common for all profiles

	// Acceleration and Decelaration logic updated to compute same based on S1 up and S1 down instead of Rated speed - YG - NOV 15
/*
    accelaration = ACCELARATION_RATE(RAMP_START_SPEED, \
                                     uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.ratedSpeed_A546, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.accel1Up_A520);
    decelaration = DECELARATION_RATE(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.ratedSpeed_A546, \
                                     RAMP_START_SPEED, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.decel1Up_A521);   
*/

    gs16UpAccelaration = ACCELARATION_RATE(RAMP_START_SPEED, \
                                           uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s1Up_A522, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.accel1Up_A520);
    gs16UpDecelaration = DECELARATION_RATE(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s1Up_A522, \
                                     	   RAMP_START_SPEED, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.decel1Up_A521);   

    gs16DownAccelaration = ACCELARATION_RATE(RAMP_START_SPEED, \
                                             uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s1Down_A528, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.accel1Up_A520);
    gs16DownDecelaration = DECELARATION_RATE(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s1Down_A528, \
                                     	     RAMP_START_SPEED, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.decel1Up_A521);   
    initRampProfileData();
    initJogProfileData();
    initInchProfileData();
    initApertureProfileData();
}

VOID initRampProfileData(VOID)
{
    SHORT i;
    
#if (STARTUP_IN_CURRENT_MODE == 1)
    rampFlags_t upBrakeRelease =    {0,1,0,0,0,1,0,1,1,0,0};
#else
    rampFlags_t upBrakeRelease =    {1,0,0,0,0,1,0,1,1,0,0};
#endif
    rampFlags_t upBrakeApply =      {1,0,0,0,1,0,1,0,1,0,0};
    rampFlags_t upRunMode =         {1,0,0,0,0,0,0,0,1,0,0};
    
    rampFlags_t dnBrakeRelease =    {1,0,0,0,0,1,0,1,0,1,0};
    rampFlags_t dnBrakeApply =      {1,0,0,0,1,0,1,0,0,1,0};
    rampFlags_t dnRunMode =         {1,0,0,0,0,0,0,0,0,1,0};
    
    //Initialize required parameters from parameter database
    //Initialize up going profile   
    i = 0;
    rampUpGoingProfile[i].rampGenFlags = upBrakeRelease;
    rampUpGoingProfile[i].startPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
    rampUpGoingProfile[i].endPosition = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos1_A103;
    rampUpGoingProfile[i].startSpeed = RAMP_START_SPEED;
    rampUpGoingProfile[i].endSpeed = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s1Up_A522;
    rampUpGoingProfile[i].speedChangeRate = gs16UpAccelaration;
    rampUpGoingProfile[i].startCurrent = RAMP_STARTING_CURRENT_MIN;
    rampUpGoingProfile[i].endCurrent = RAMP_STARTING_CURRENT_MAX;
    rampUpGoingProfile[i].currentChangeRate = RAMP_STARTING_CURRENT_STEP;
    i++;
    rampUpGoingProfile[i].rampGenFlags = upRunMode;
    rampUpGoingProfile[i].startPosition = rampUpGoingProfile[i-1].startPosition;
    rampUpGoingProfile[i].endPosition = rampUpGoingProfile[i-1].endPosition;
    rampUpGoingProfile[i].startSpeed = rampUpGoingProfile[i-1].endSpeed;
    rampUpGoingProfile[i].endSpeed = rampUpGoingProfile[i-1].endSpeed;
    rampUpGoingProfile[i].speedChangeRate = NO_SPEED_CHANGE;
    i++;
    rampUpGoingProfile[i].rampGenFlags = upRunMode;
    rampUpGoingProfile[i].startPosition = rampUpGoingProfile[i-1].endPosition;
    rampUpGoingProfile[i].endPosition = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos2_A104;
    rampUpGoingProfile[i].startSpeed = rampUpGoingProfile[i-1].endSpeed;
    rampUpGoingProfile[i].endSpeed = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s2Up_A523;
    rampUpGoingProfile[i].speedChangeRate = gs16UpDecelaration;  
    i++;
    rampUpGoingProfile[i].rampGenFlags = upRunMode;
    rampUpGoingProfile[i].startPosition = rampUpGoingProfile[i-1].startPosition;
    rampUpGoingProfile[i].endPosition = rampUpGoingProfile[i-1].endPosition;
    rampUpGoingProfile[i].startSpeed = rampUpGoingProfile[i-1].endSpeed;
    rampUpGoingProfile[i].endSpeed = rampUpGoingProfile[i-1].endSpeed;
    rampUpGoingProfile[i].speedChangeRate = NO_SPEED_CHANGE;
    i++;
    
    if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.upStepCount_A525 == TWO_STEP_RAMP_PROFILE)
    {        
        rampUpGoingProfile[i].rampGenFlags = upBrakeApply; //Value to apply brake
        rampUpGoingProfile[i].startPosition = rampUpGoingProfile[i-1].endPosition;
        rampUpGoingProfile[i].endPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
        rampUpGoingProfile[i].startSpeed = rampUpGoingProfile[i-1].endSpeed;
        rampUpGoingProfile[i].endSpeed = SHUTTER_SPEED_MIN;
        rampUpGoingProfile[i].speedChangeRate = gs16UpDecelaration;  
        rampUpGoingProfile[i].dcInjectionDuty = rampUpGoingProfile[i+2].dcInjectionDuty;
        rampUpGoingProfile[i].dcInjectionTime = rampUpGoingProfile[i+2].dcInjectionTime;
    }
    else
    {
        rampUpGoingProfile[i].rampGenFlags = upRunMode;
        rampUpGoingProfile[i].startPosition = rampUpGoingProfile[i-1].endPosition;
        rampUpGoingProfile[i].endPosition = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos3_A105;
        rampUpGoingProfile[i].startSpeed = rampUpGoingProfile[i-1].endSpeed;
        rampUpGoingProfile[i].endSpeed = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s3Up_A524;
        rampUpGoingProfile[i].speedChangeRate = gs16UpDecelaration; 
        i++;
        rampUpGoingProfile[i].rampGenFlags = upRunMode;
        rampUpGoingProfile[i].startPosition = rampUpGoingProfile[i-1].startPosition;
        rampUpGoingProfile[i].endPosition = rampUpGoingProfile[i-1].endPosition;
        rampUpGoingProfile[i].startSpeed = rampUpGoingProfile[i-1].endSpeed;
        rampUpGoingProfile[i].endSpeed = rampUpGoingProfile[i-1].endSpeed;
        rampUpGoingProfile[i].speedChangeRate = NO_SPEED_CHANGE;
        i++;
        rampUpGoingProfile[i].rampGenFlags = upBrakeApply; //Value to apply brake
        rampUpGoingProfile[i].startPosition = rampUpGoingProfile[i-1].endPosition;
        rampUpGoingProfile[i].endPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
        rampUpGoingProfile[i].startSpeed = rampUpGoingProfile[i-1].endSpeed;
        rampUpGoingProfile[i].endSpeed = SHUTTER_SPEED_MIN;
        rampUpGoingProfile[i].speedChangeRate = gs16UpDecelaration;  
    } 
    
    //Initialize down going profile
    i = 0;
    rampDnGoingProfile[i].rampGenFlags = dnBrakeRelease; //value to remove brake
    rampDnGoingProfile[i].startPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
    rampDnGoingProfile[i].endPosition = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos1_A106;
    rampDnGoingProfile[i].startSpeed = RAMP_START_SPEED;
    rampDnGoingProfile[i].endSpeed = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s1Down_A528;
    rampDnGoingProfile[i].speedChangeRate = gs16DownAccelaration;
    i++;
    rampDnGoingProfile[i].rampGenFlags = dnRunMode;
    rampDnGoingProfile[i].startPosition = rampDnGoingProfile[i-1].startPosition;
    rampDnGoingProfile[i].endPosition = rampDnGoingProfile[i-1].endPosition;
    rampDnGoingProfile[i].startSpeed = rampDnGoingProfile[i-1].endSpeed;
    rampDnGoingProfile[i].endSpeed = rampDnGoingProfile[i-1].endSpeed;
    rampDnGoingProfile[i].speedChangeRate = NO_SPEED_CHANGE;
    i++;
    rampDnGoingProfile[i].rampGenFlags = dnRunMode;
    rampDnGoingProfile[i].startPosition = rampDnGoingProfile[i-1].endPosition;
    rampDnGoingProfile[i].endPosition = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos2_A107;
    rampDnGoingProfile[i].startSpeed = rampDnGoingProfile[i-1].endSpeed;
    rampDnGoingProfile[i].endSpeed = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s2Down_A529;
    rampDnGoingProfile[i].speedChangeRate = gs16DownDecelaration;  
    i++;
    rampDnGoingProfile[i].rampGenFlags = dnRunMode;
    rampDnGoingProfile[i].startPosition = rampDnGoingProfile[i-1].startPosition;
    rampDnGoingProfile[i].endPosition = rampDnGoingProfile[i-1].endPosition;
    rampDnGoingProfile[i].startSpeed = rampDnGoingProfile[i-1].endSpeed;
    rampDnGoingProfile[i].endSpeed = rampDnGoingProfile[i-1].endSpeed;
    rampDnGoingProfile[i].speedChangeRate = NO_SPEED_CHANGE;
    i++;
    
    if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.downStepCount_A531 == TWO_STEP_RAMP_PROFILE)
    {        
        rampDnGoingProfile[i].rampGenFlags = dnBrakeApply; //Value to apply brake
        rampDnGoingProfile[i].startPosition = rampDnGoingProfile[i-1].endPosition;
        rampDnGoingProfile[i].endPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
        rampDnGoingProfile[i].startSpeed = rampDnGoingProfile[i-1].endSpeed;
        rampDnGoingProfile[i].endSpeed = SHUTTER_SPEED_MIN;
        rampDnGoingProfile[i].speedChangeRate = gs16DownDecelaration;  
        rampDnGoingProfile[i].dcInjectionDuty = rampDnGoingProfile[i+2].dcInjectionDuty;
        rampDnGoingProfile[i].dcInjectionTime = rampDnGoingProfile[i+2].dcInjectionTime;
    }
    else
    {
        rampDnGoingProfile[i].rampGenFlags = dnRunMode;
        rampDnGoingProfile[i].startPosition = rampDnGoingProfile[i-1].endPosition;
        rampDnGoingProfile[i].endPosition = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos3_A108;
        rampDnGoingProfile[i].startSpeed = rampDnGoingProfile[i-1].endSpeed;
        rampDnGoingProfile[i].endSpeed = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s3Down_A530;
        rampDnGoingProfile[i].speedChangeRate = gs16DownDecelaration; 
        i++;
        rampDnGoingProfile[i].rampGenFlags = dnRunMode;
        rampDnGoingProfile[i].startPosition = rampDnGoingProfile[i-1].startPosition;
        rampDnGoingProfile[i].endPosition = rampDnGoingProfile[i-1].endPosition;
        rampDnGoingProfile[i].startSpeed = rampDnGoingProfile[i-1].endSpeed;
        rampDnGoingProfile[i].endSpeed = rampDnGoingProfile[i-1].endSpeed;
        rampDnGoingProfile[i].speedChangeRate = NO_SPEED_CHANGE;
        i++;
        rampDnGoingProfile[i].rampGenFlags = dnBrakeApply; //Value to apply brake
        rampDnGoingProfile[i].startPosition = rampDnGoingProfile[i-1].endPosition;
        rampDnGoingProfile[i].endPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
        rampDnGoingProfile[i].startSpeed = rampDnGoingProfile[i-1].endSpeed;
        rampDnGoingProfile[i].endSpeed = SHUTTER_SPEED_MIN;
        rampDnGoingProfile[i].speedChangeRate = gs16DownDecelaration;  
    }     
}

VOID initJogProfileData(VOID)
{
    SHORT i;
    
    rampFlags_t upBrakeRelease =    {1,0,0,0,0,1,0,1,1,0,0};
    rampFlags_t upBrakeApply =      {1,0,0,0,1,0,1,0,1,0,0};
    rampFlags_t upRunMode =         {1,0,0,0,0,0,0,0,1,0,0};
    
    rampFlags_t dnBrakeRelease =    {1,0,0,0,0,1,0,1,0,1,0};
    rampFlags_t dnBrakeApply =      {1,0,0,0,1,0,1,0,0,1,0};
    rampFlags_t dnRunMode =         {1,0,0,0,0,0,0,0,0,1,0};
    
    //Initialize required parameters from parameter database
    //Initialize Jog up going profile
    i = 0;
    rampJogUpProfile[i].rampGenFlags = upBrakeRelease; //value to remove brake
    rampJogUpProfile[i].startPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
    
	//	Default value of riseChangeGearPos3_A105 is 0. This resulted in shutter overshoot at the upper limit
	//	when parameters are reset in case of shutter type change - Dec 2015
    /*if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.upStepCount_A525 == TWO_STEP_RAMP_PROFILE)
        rampJogUpProfile[i].endPosition = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos2_A104;
    else
        rampJogUpProfile[i].endPosition = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos3_A105;*/
	
	rampJogUpProfile[i].endPosition = \
		uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 + 100;
    
    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration ||
       uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration ||
       uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
    {
        rampJogUpProfile[i].startSpeed = RAMP_START_SPEED;    
        rampJogUpProfile[i].endSpeed = __builtin_divud(((DWORD)uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.jogSpeed_A551 \
                                         *uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.ratedSpeed_A546),100);
    }
    else
    {
        rampJogUpProfile[i].startSpeed = RAMP_START_SPEED;    
        rampJogUpProfile[i].endSpeed = __builtin_divud(((DWORD)SAFETY_SENSOR_JOG_SPEED_PERCENT \
                                                        *uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s1Up_A522),100);
    }
    
    
    rampJogUpProfile[i].speedChangeRate = gs16UpAccelaration;
    i++;
    rampJogUpProfile[i].rampGenFlags = upRunMode;
    rampJogUpProfile[i].startPosition = rampJogUpProfile[i-1].startPosition;
    rampJogUpProfile[i].endPosition = rampJogUpProfile[i-1].endPosition;
    rampJogUpProfile[i].startSpeed = rampJogUpProfile[i-1].endSpeed;
    rampJogUpProfile[i].endSpeed = rampJogUpProfile[i-1].endSpeed;
    rampJogUpProfile[i].speedChangeRate = NO_SPEED_CHANGE;
    i++;
    rampJogUpProfile[i].rampGenFlags = upBrakeApply; //value to apply brake
    rampJogUpProfile[i].startPosition = rampJogUpProfile[i-1].endPosition;
    rampJogUpProfile[i].endPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
    rampJogUpProfile[i].startSpeed = rampJogUpProfile[i-1].endSpeed;
    rampJogUpProfile[i].endSpeed = SHUTTER_SPEED_MIN;
    rampJogUpProfile[i].speedChangeRate = gs16UpDecelaration; 

    //Initialize Jog down going profile
    i = 0;
    rampJogDnProfile[i].rampGenFlags = dnBrakeRelease; //value to remove brake
    rampJogDnProfile[i].startPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;

    if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.downStepCount_A531 == TWO_STEP_RAMP_PROFILE)
        rampJogDnProfile[i].endPosition = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos2_A107;
    else
        rampJogDnProfile[i].endPosition = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos3_A108;

    rampJogDnProfile[i].startSpeed = RAMP_START_SPEED;
    rampJogDnProfile[i].endSpeed = rampJogUpProfile[i].endSpeed; 
    rampJogDnProfile[i].speedChangeRate = gs16DownAccelaration;
    i++;
    rampJogDnProfile[i].rampGenFlags = dnRunMode;
    rampJogDnProfile[i].startPosition = rampJogDnProfile[i-1].startPosition;
    rampJogDnProfile[i].endPosition = rampJogDnProfile[i-1].endPosition;
    rampJogDnProfile[i].startSpeed = rampJogDnProfile[i-1].endSpeed;
    rampJogDnProfile[i].endSpeed = rampJogDnProfile[i-1].endSpeed;
    rampJogDnProfile[i].speedChangeRate = NO_SPEED_CHANGE;
    i++;
    rampJogDnProfile[i].rampGenFlags = dnBrakeApply; //value to apply brake
    rampJogDnProfile[i].startPosition = rampJogDnProfile[i-1].endPosition;
    rampJogDnProfile[i].endPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
    rampJogDnProfile[i].startSpeed = rampJogDnProfile[i-1].endSpeed;
    rampJogDnProfile[i].endSpeed = SHUTTER_SPEED_MIN;
    rampJogDnProfile[i].speedChangeRate = gs16DownDecelaration; 
}

VOID initApertureProfileData(VOID)
{
    SHORT i;

    rampFlags_t upBrakeRelease =    {1,0,0,0,0,1,0,1,1,0,0};
    rampFlags_t upBrakeApply =      {1,0,0,0,1,0,1,0,1,0,0};
    rampFlags_t upRunMode =         {1,0,0,0,0,0,0,0,1,0,0};
    
    rampFlags_t dnBrakeRelease =    {1,0,0,0,0,1,0,1,0,1,0};
    rampFlags_t dnBrakeApply =      {1,0,0,0,1,0,1,0,0,1,0};
    rampFlags_t dnRunMode =         {1,0,0,0,0,0,0,0,0,1,0};
    
    //Initialize required parameters from parameter database
    //Initialize Aperture up going profile
    i = 0;
    rampApertureUpProfile[i].rampGenFlags = upBrakeRelease; //value to remove brake
    rampApertureUpProfile[i].startPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
    rampApertureUpProfile[i].endPosition = (uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130 + APERPOS_OFFSET);
    
    rampApertureUpProfile[i].startSpeed = RAMP_START_SPEED;
    rampApertureUpProfile[i].endSpeed = __builtin_divud(((DWORD)uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.jogSpeed_A551 \
                                     *uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.ratedSpeed_A546),100);
    rampApertureUpProfile[i].speedChangeRate = gs16UpAccelaration;
    i++;
    rampApertureUpProfile[i].rampGenFlags = upRunMode;
    rampApertureUpProfile[i].startPosition = rampApertureUpProfile[i-1].startPosition;
    rampApertureUpProfile[i].endPosition = rampApertureUpProfile[i-1].endPosition;
    rampApertureUpProfile[i].startSpeed = rampApertureUpProfile[i-1].endSpeed;
    rampApertureUpProfile[i].endSpeed = rampApertureUpProfile[i-1].endSpeed;
    rampApertureUpProfile[i].speedChangeRate = NO_SPEED_CHANGE;
    i++;
    rampApertureUpProfile[i].rampGenFlags = upBrakeApply; //value to apply brake
    rampApertureUpProfile[i].startPosition = rampApertureUpProfile[i-1].endPosition;
    rampApertureUpProfile[i].endPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130;
    rampApertureUpProfile[i].startSpeed = rampApertureUpProfile[i-1].endSpeed;
    rampApertureUpProfile[i].endSpeed = SHUTTER_SPEED_MIN;
    rampApertureUpProfile[i].speedChangeRate = gs16UpDecelaration;   
    //Initialize Aperture down going profile
    i = 0;
    rampApertureDnProfile[i].rampGenFlags = dnBrakeRelease; //value to remove brake
    rampApertureDnProfile[i].startPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130;
    rampApertureDnProfile[i].endPosition = (uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 - APERPOS_OFFSET);
    rampApertureDnProfile[i].startSpeed = RAMP_START_SPEED;
    rampApertureDnProfile[i].endSpeed = rampApertureUpProfile[i].endSpeed; 
    rampApertureDnProfile[i].speedChangeRate = gs16DownAccelaration;
    i++;
    rampApertureDnProfile[i].rampGenFlags = dnRunMode;
    rampApertureDnProfile[i].startPosition = rampApertureDnProfile[i-1].startPosition;
    rampApertureDnProfile[i].endPosition = rampApertureDnProfile[i-1].endPosition;
    rampApertureDnProfile[i].startSpeed = rampApertureDnProfile[i-1].endSpeed;
    rampApertureDnProfile[i].endSpeed = rampApertureDnProfile[i-1].endSpeed;
    rampApertureDnProfile[i].speedChangeRate = NO_SPEED_CHANGE;
    i++;
    rampApertureDnProfile[i].rampGenFlags = dnBrakeApply; //value to apply brake
    rampApertureDnProfile[i].startPosition = rampApertureDnProfile[i-1].endPosition;
    rampApertureDnProfile[i].endPosition = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
    rampApertureDnProfile[i].startSpeed = rampApertureDnProfile[i-1].endSpeed;
    rampApertureDnProfile[i].endSpeed = SHUTTER_SPEED_MIN;
    rampApertureDnProfile[i].speedChangeRate = gs16DownDecelaration; 
}


VOID initInchProfileData(VOID)
{
    SHORT i;

    rampFlags_t upBrakeRelease =    {1,0,0,0,0,1,0,1,1,0,0};
    rampFlags_t upBrakeApply =      {1,0,0,0,1,0,1,0,1,0,0};
    rampFlags_t upRunMode =         {1,0,0,0,0,0,0,0,1,0,0};
    
    rampFlags_t dnBrakeRelease =    {1,0,0,0,0,1,0,1,0,1,0};
    rampFlags_t dnBrakeApply =      {1,0,0,0,1,0,1,0,0,1,0};
    rampFlags_t dnRunMode =         {1,0,0,0,0,0,0,0,0,1,0};
   
    //Initialize required parameters from parameter database
    //Initialize Inch up going profile
    i = 0;
    rampInchUpProfile[i].rampGenFlags = upBrakeRelease; //value to remove brake
    rampInchUpProfile[i].startPosition = 32767;     //in inch mode we do not check start and end position
    rampInchUpProfile[i].endPosition = -32768;      //in inch mode we do not check start and end position    
    rampInchUpProfile[i].startSpeed = RAMP_START_SPEED;
    rampInchUpProfile[i].endSpeed = __builtin_divud(((DWORD)uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.inchSpeed_A517 \
                                     *uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.ratedSpeed_A546),100);
    rampInchUpProfile[i].speedChangeRate = gs16UpAccelaration;
    i++;
    rampInchUpProfile[i].rampGenFlags = upRunMode;
    rampInchUpProfile[i].startPosition = rampInchUpProfile[i-1].startPosition;
    rampInchUpProfile[i].endPosition = rampInchUpProfile[i-1].endPosition;
    rampInchUpProfile[i].startSpeed = rampInchUpProfile[i-1].endSpeed;
    rampInchUpProfile[i].endSpeed = rampInchUpProfile[i-1].endSpeed;
    rampInchUpProfile[i].speedChangeRate = NO_SPEED_CHANGE;
    i++;
    rampInchUpProfile[i].rampGenFlags = upBrakeApply; //value to apply brake
    rampInchUpProfile[i].startPosition = rampInchUpProfile[i-1].startPosition;
    rampInchUpProfile[i].endPosition = rampInchUpProfile[i-1].endPosition;
    rampInchUpProfile[i].startSpeed = rampInchUpProfile[i-1].endSpeed;
    rampInchUpProfile[i].endSpeed = SHUTTER_SPEED_MIN;
    rampInchUpProfile[i].speedChangeRate = gs16UpDecelaration;     
    //Initialize Inch down going profile
    i = 0;
    rampInchDnProfile[i].rampGenFlags = dnBrakeRelease; //value to remove brake
    rampInchDnProfile[i].startPosition = -32768;//in inch mode we do not check start and end position
    rampInchDnProfile[i].endPosition = 32767; //in inch mode we do not check start and end position
    rampInchDnProfile[i].startSpeed = RAMP_START_SPEED;
    rampInchDnProfile[i].endSpeed = rampInchUpProfile[i].endSpeed;
    rampInchDnProfile[i].speedChangeRate = gs16DownAccelaration;
    i++;
    rampInchDnProfile[i].rampGenFlags = dnRunMode;
    rampInchDnProfile[i].startPosition = rampInchDnProfile[i-1].startPosition;
    rampInchDnProfile[i].endPosition = rampInchDnProfile[i-1].endPosition;
    rampInchDnProfile[i].startSpeed = rampInchDnProfile[i-1].endSpeed;
    rampInchDnProfile[i].endSpeed = rampInchDnProfile[i-1].endSpeed;
    rampInchDnProfile[i].speedChangeRate = NO_SPEED_CHANGE;
    i++;
    rampInchDnProfile[i].rampGenFlags = dnBrakeApply; //value to apply brake
    rampInchDnProfile[i].startPosition = rampInchDnProfile[i-1].startPosition;
    rampInchDnProfile[i].endPosition = rampInchDnProfile[i-1].endPosition;
    rampInchDnProfile[i].startSpeed = rampInchDnProfile[i-1].endSpeed;
    rampInchDnProfile[i].endSpeed = SHUTTER_SPEED_MIN;
    rampInchDnProfile[i].speedChangeRate = gs16DownDecelaration;   
}

VOID reAdjustRampPositions(VOID)
{
    SHORT cnt = 0;
    
    //if drift was calculated and it is non-zero then readjust ramp positions
    if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637 != 0)
    {
        //Check the direction of movement then adjust ramp profiles
        if(requiredDirection == CW)    
        {
            if(currentRampProfileNo == RAMP_GOING_UP_PROFILE)
            {
                //Readjust up going ramp
                //Read the parameter from which ramp needs to be adjusted
                //if it is two step ramp then adjust positions accordingly
                if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.upStepCount_A525 == TWO_STEP_RAMP_PROFILE)
                {
                    for(cnt = 0; cnt < (RAMP_UP_STATES - 2); cnt++)
                    {
                        //find the parameter which needs to be adjusted
                        if(rampUpGoingProfile[cnt].endPosition < uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128)
                        {                             
                            //Adjust knee points also                            
                            rampUpGoingProfile[cnt].startPosition = rampUpGoingProfile[cnt].startPosition + \
                                uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                            rampUpGoingProfile[cnt].endPosition = rampUpGoingProfile[cnt].endPosition + \
                                uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                        }
                    }
                }
                else    //three step profile
                {
                    for(cnt = 0; cnt < RAMP_UP_STATES; cnt++)
                    {
                        //find the parameter which needs to be adjusted
                        if(rampUpGoingProfile[cnt].endPosition < uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128)
                        {                            
                            rampUpGoingProfile[cnt].startPosition = rampUpGoingProfile[cnt].startPosition + \
                                uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                            rampUpGoingProfile[cnt].endPosition = rampUpGoingProfile[cnt].endPosition + \
                                uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                        }
                    }
                }
            }
            else if(currentRampProfileNo == RAMP_APERTURE_UP_PROFILE)
            {
                //Readjust up going ramp
                //Read the parameter from which ramp needs to be adjusted
                for(cnt = 0; cnt < APER_UP_STATES; cnt++)
                {
                    //find the parameter which needs to be adjusted
                    if(rampApertureUpProfile[cnt].endPosition < uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128)
                    {                        
                        rampApertureUpProfile[cnt].startPosition = rampApertureUpProfile[cnt].startPosition + \
                            uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                        rampApertureUpProfile[cnt].endPosition = rampApertureUpProfile[cnt].endPosition + \
                            uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                    }
                }
            }
            else if(currentRampProfileNo == RAMP_JOG_UP_PROFILE)
            {
                //Readjust up going jog
                //Read the parameter from which jog needs to be adjusted
                for(cnt = 0; cnt < JOG_UP_STATES; cnt++)
                {
                    //find the parameter which needs to be adjusted
                    if(rampJogUpProfile[cnt].endPosition < uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128)
                    {                        
                        rampJogUpProfile[cnt].startPosition = rampJogUpProfile[cnt].startPosition + \
                            uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                        rampJogUpProfile[cnt].endPosition = rampJogUpProfile[cnt].endPosition + \
                            uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                    }
                }
            }
        }
        else
        {
            //Readjust down going ramp
            //Read the parameter from which ramp needs to be adjusted
            if(currentRampProfileNo == RAMP_GOING_DN_PROFILE)
            {
                //Readjust down going ramp
                //Read the parameter from which ramp needs to be adjusted
                //if it is two step ramp then adjust positions accordingly
                if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.downStepCount_A531 == TWO_STEP_RAMP_PROFILE)
                {
                    for(cnt = 0; cnt < (RAMP_DN_STATES - 2); cnt++)
                    {
                        //find the parameter which needs to be adjusted
                        if(rampDnGoingProfile[cnt].endPosition > uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128)
                        {   
                            rampDnGoingProfile[cnt].startPosition = rampDnGoingProfile[cnt].startPosition + \
                                uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                            rampDnGoingProfile[cnt].endPosition = rampDnGoingProfile[cnt].endPosition + \
                                uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;                            
                        }
                    }
                }
                else    //three step profile
                {
                    for(cnt = 0; cnt < RAMP_DN_STATES; cnt++)
                    {
                        //find the parameter which needs to be adjusted
                        if(rampDnGoingProfile[cnt].endPosition > uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128)
                        {
                            rampDnGoingProfile[cnt].startPosition = rampDnGoingProfile[cnt].startPosition + \
                                uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                            rampDnGoingProfile[cnt].endPosition = rampDnGoingProfile[cnt].endPosition + \
                                uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                        }
                    }
                }
            }
            else if(currentRampProfileNo == RAMP_APERTURE_DN_PROFILE)
            {
                //Readjust up going ramp
                //Read the parameter from which ramp needs to be adjusted
                for(cnt = 0; cnt < APER_DN_STATES; cnt++)
                {
                    //find the parameter which needs to be adjusted
                    if(rampApertureDnProfile[cnt].endPosition > uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128)
                    {
                        rampApertureDnProfile[cnt].startPosition = rampApertureDnProfile[cnt].startPosition + \
                            uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                        rampApertureDnProfile[cnt].endPosition = rampApertureDnProfile[cnt].endPosition + \
                            uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                    }
                }
            }
            else if(currentRampProfileNo == RAMP_JOG_DN_PROFILE)
            {
                //Readjust up going jog
                //Read the parameter from which jog needs to be adjusted
                for(cnt = 0; cnt < JOG_DN_STATES; cnt++)
                {
                    //find the parameter which needs to be adjusted
                    if(rampJogDnProfile[cnt].endPosition > uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128)
                    {
                        rampJogDnProfile[cnt].startPosition = rampJogDnProfile[cnt].startPosition + \
                            uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                        rampJogDnProfile[cnt].endPosition = rampJogDnProfile[cnt].endPosition + \
                            uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637;
                    }
                }
            }            
        }
    }
}

VOID pwmBufferControl(SHORT status)
{
    if(status == ENABLE)
    {
        enablePWMBuffer;
    }
    else
    {
        disablePWMBuffer;
    }
}

/******************************************************************************
 * chargeBootstraps
 *
 *  This function charges the bootstrap caps each time the motor is energized for the   
 *  first time after an undetermined amount of time. ChargeBootstraps subroutine turns
 *  ON the lower transistors for 10 ms to ensure voltage on these caps, and then it 
 *  transfers the control of the outputs to the PWM module.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/    
VOID chargeBootstraps(VOID)
{   
#ifdef IGBT_LowActive_IR
    IOCON1 = 0xC780;
	IOCON2 = 0xC780;
	IOCON3 = 0xC780;    
#endif    
#ifdef IGBT_HighActive_ROME
    IOCON1 = 0xC740;   
	IOCON2 = 0xC740;
	IOCON3 = 0xC740;     
#endif   
    PTCONbits.PTEN = 1;
    pwmBufferControl(ENABLE);
	delayMs(CHARGE_BOOTSTRAP_CAP); 
    pwmBufferControl(DISABLE);    
#ifdef IGBT_LowActive_IR
    IOCON1 = 0xF000;
    IOCON2 = 0xF000;
    IOCON3 = 0xF000;    
#endif    
#ifdef IGBT_HighActive_ROME
    IOCON1 = 0xC000;    
    IOCON2 = 0xC000;
    IOCON3 = 0xC000;    
#endif 
    PTCONbits.PTEN = 0;
    
    PDC1 = PHASE1 / 2;	// initialise as 0 volts
	PDC2 = PHASE2 / 2;	
	PDC3 = PHASE3 / 2;
}

/******************************************************************************
 * startMotor
 *
 * This function initializes speed controller and charges bootstrap capacitors.
 * It starts timer1 for speed PI controller, speed measurement and force 
 * commutation. it enables hall feedback and starts MCPWM to run the motor
 * 
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/  
VOID startMotor(VOID)
{
    flags.speedControl = 1;
#if (STARTUP_IN_CURRENT_MODE == 1)
    flags.currentControl = 1;
#else
    flags.currentControl = 0;
#endif
    MotorRunCount = 0;
    //requiredDirection = CCW;
    //currentDirection = CCW;
    MotorDecActive = 0;
    
    initCurrentLimitPI();
    intitSpeedController();     /* Initialize speed controller */
    intitCurrentController();   /* Initialize current controller */
    chargeBootstraps();
    
	TMR1 = 0;			/* Reset timer 1 for speed control */
    TMR2 = 0;			/* Reset timer 2 for current control */
	TMR3 = 0;			/* Reset timer 3 for speed measurement */
    TMR7 = 0;
    TMR8 = 0;
    
	T1CONbits.TON = 1;
    T2CONbits.TON = 1;
	T3CONbits.TON = 1;
    
	IFS0bits.T1IF = 0;		/* Clear timer 1 flag */
    IFS0bits.T2IF = 0;		/* Clear timer 2 flag */
    IFS0bits.T3IF = 0;		/* Clear timer 3 flag */
    IFS3bits.T7IF = 0;   
//    IFS5bits.PWM1IF = 0;
    
	IFS0bits.IC1IF = 0;		/* Clear interrupt flag */
	IFS0bits.IC2IF = 0;	
	IFS2bits.IC3IF = 0;

    
    IEC0bits.T1IE = 1;		/* Enable interrupts for timer 1 */
    IEC0bits.T2IE = 1;		/* Enable interrupts for timer 2 */
    IEC0bits.T3IE = 1;		/* Enable interrupts for timer 3 */
//    IEC3bits.T7IE = 1;
	IEC0bits.IC1IE = 1;		/* Enable interrupts on IC1 */
	IEC0bits.IC2IE = 1;		/* Enable interrupts on IC2 */
	IEC2bits.IC3IE = 1;
//    IEC5bits.PWM1IE = 1;
    #ifdef SVM_INTERRUPT_USE_T7
    {
        IEC3bits.T7IE = 1;
        T7CONbits.TON = 1;
    }
    #else
    {
        IFS5bits.PWM1IF = 0;
        IEC5bits.PWM1IE = 1;
    }
    #endif
    
    PTCONbits.PTEN = 1;	    // start PWM  
//    T7CONbits.TON = 1;
    pwmBufferControl(ENABLE);
    AD1CON1bits.ADON = 1;   //turn ON ADC module 
	flags.motorRunning = 1;	/* Indicate that the motor is running */  
    
    fanOnCnt = 0;           //Reset fan timer count
    flags.exstFanOn = 1; //set fan status flag
    fanON;               //Turn ON heat sink fan
    
}

VOID startMotorCW(VOID)
{
    flags.speedControl = 1;
#if (STARTUP_IN_CURRENT_MODE == 1)
    flags.currentControl = 1;
#else
    flags.currentControl = 0;
#endif
    MotorRunCount = 0;
    requiredDirection = CW;
    currentDirection = CW;
    
    MotorDecActive = 0;
    
    initCurrentLimitPI();
    intitSpeedController();     /* Initialize speed controller */
    intitCurrentController();   /* Initialize current controller */
    chargeBootstraps();
    
	TMR1 = 0;			/* Reset timer 1 for speed control */
    TMR2 = 0;			/* Reset timer 2 for current control */
	TMR3 = 0;			/* Reset timer 3 for speed measurement */
    TMR7 = 0;
    TMR8 = 0;
    
	T1CONbits.TON = 1;
    T2CONbits.TON = 1;
	T3CONbits.TON = 1;
    T8CONbits.TON = 1;
    
	IFS0bits.T1IF = 0;		/* Clear timer 1 flag */
    IFS0bits.T2IF = 0;		/* Clear timer 2 flag */
    IFS0bits.T3IF = 0;		/* Clear timer 3 flag */
    IFS3bits.T7IF = 0;   
//    IFS5bits.PWM1IF = 0;
    
	IFS0bits.IC1IF = 0;		/* Clear interrupt flag */
	IFS0bits.IC2IF = 0;	
	IFS2bits.IC3IF = 0;

    
    IEC0bits.T1IE = 1;		/* Enable interrupts for timer 1 */
    IEC0bits.T2IE = 1;		/* Enable interrupts for timer 2 */
    IEC0bits.T3IE = 1;		/* Enable interrupts for timer 3 */
//    IEC3bits.T7IE = 1;
	IEC0bits.IC1IE = 1;		/* Enable interrupts on IC1 */
	IEC0bits.IC2IE = 1;		/* Enable interrupts on IC2 */
	IEC2bits.IC3IE = 1;
//    IEC5bits.PWM1IE = 1;
    #ifdef SVM_INTERRUPT_USE_T7
    {
        IEC3bits.T7IE = 1;
        T7CONbits.TON = 1;
    }
    #else 
    {
        IFS5bits.PWM1IF = 0;
        IEC5bits.PWM1IE = 1;
    }
    #endif
    
    PTCONbits.PTEN = 1;	    // start PWM  
//    T7CONbits.TON = 1;
    pwmBufferControl(ENABLE);
    AD1CON1bits.ADON = 1;   //turn ON ADC module 
	flags.motorRunning = 1;	/* Indicate that the motor is running */  
    
    fanOnCnt = 0;           //Reset fan timer count
    flags.exstFanOn = 1; //set fan status flag
    fanON;               //Turn ON heat sink fan
    
}

VOID startMotorCCW(VOID)
{
    flags.speedControl = 1;
#if (STARTUP_IN_CURRENT_MODE == 1)
    flags.currentControl = 1;
#else
    flags.currentControl = 0;
#endif
    MotorRunCount = 0;
    requiredDirection = CCW;
    currentDirection = CCW;
    MotorDecActive = 0;
    
    initCurrentLimitPI();
    intitSpeedController();     /* Initialize speed controller */
    intitCurrentController();   /* Initialize current controller */
    chargeBootstraps();
    
	TMR1 = 0;			/* Reset timer 1 for speed control */
    TMR2 = 0;			/* Reset timer 2 for current control */
	TMR3 = 0;			/* Reset timer 3 for speed measurement */
    TMR7 = 0;
    TMR8 = 0;
    
	T1CONbits.TON = 1;
    T2CONbits.TON = 1;
	T3CONbits.TON = 1;
    T8CONbits.TON = 1;
    
	IFS0bits.T1IF = 0;		/* Clear timer 1 flag */
    IFS0bits.T2IF = 0;		/* Clear timer 2 flag */
    IFS0bits.T3IF = 0;		/* Clear timer 3 flag */
    IFS3bits.T7IF = 0;   
//    IFS5bits.PWM1IF = 0;
    
	IFS0bits.IC1IF = 0;		/* Clear interrupt flag */
	IFS0bits.IC2IF = 0;	
	IFS2bits.IC3IF = 0;

    
    IEC0bits.T1IE = 1;		/* Enable interrupts for timer 1 */
    IEC0bits.T2IE = 1;		/* Enable interrupts for timer 2 */
    IEC0bits.T3IE = 1;		/* Enable interrupts for timer 3 */
//    IEC3bits.T7IE = 1;
	IEC0bits.IC1IE = 1;		/* Enable interrupts on IC1 */
	IEC0bits.IC2IE = 1;		/* Enable interrupts on IC2 */
	IEC2bits.IC3IE = 1;
//    IEC5bits.PWM1IE = 1;
    #ifdef SVM_INTERRUPT_USE_T7
    {
        IEC3bits.T7IE = 1;
        T7CONbits.TON = 1;
    }
    #else 
    {
        IFS5bits.PWM1IF = 0;
        IEC5bits.PWM1IE = 1;
    }
    #endif
    
    PTCONbits.PTEN = 1;	    // start PWM  
//    T7CONbits.TON = 1;
    pwmBufferControl(ENABLE);
    AD1CON1bits.ADON = 1;   //turn ON ADC module 
	flags.motorRunning = 1;	/* Indicate that the motor is running */  
    
    fanOnCnt = 0;           //Reset fan timer count
    flags.exstFanOn = 1; //set fan status flag
    fanON;               //Turn ON heat sink fan
    
}

/******************************************************************************
 * stopMotor
 *
 * This function stops timer1 for speed PI controller, speed measurement and force 
 * commutation. it disables hall feedback and stops MCPWM to stop the motor.
 * 
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID stopMotor(VOID)
{
    pwmBufferControl(DISABLE);
    PTCONbits.PTEN = 0; 
    T7CONbits.TON = 0;
//	IEC0bits.T1IE = 0;
    IEC0bits.T2IE = 0;
    IEC0bits.T3IE = 0;
    IEC3bits.T7IE = 0;
    IEC5bits.PWM1IE = 0;
//	T1CONbits.TON = 0;
    T2CONbits.TON = 0;
	T3CONbits.TON = 0;
    T8CONbits.TON = 0;
//	No need to disable hall interrupts
//	Changed to handle "offset at upper & lower limit"
#if 0
	IEC0bits.IC1IE = 0;	
	IEC0bits.IC2IE = 0;
	IEC2bits.IC3IE = 0;	
#endif
    AD1CON1bits.ADON = 0;   //turn OFF ADC module 
	flags.motorRunning = 0;	/* Indicate that the motor has been stopped */
    fanOFF;
    MotorRunCount = 0;
    MotorRunInCycle = 0;
}

VOID stopMotorUseCycle(VOID)
{
    pwmBufferControl(DISABLE);
    PTCONbits.PTEN = 0; 
    T7CONbits.TON = 0;
	IEC0bits.T1IE = 0;
    IEC0bits.T2IE = 0;
    IEC0bits.T3IE = 0;
    IEC3bits.T7IE = 0;
	T1CONbits.TON = 0;
    T2CONbits.TON = 0;
	T3CONbits.TON = 0;
    //T8CONbits.TON = 0;
//	No need to disable hall interrupts
//	Changed to handle "offset at upper & lower limit"
#if 0
	IEC0bits.IC1IE = 0;	
	IEC0bits.IC2IE = 0;
	IEC2bits.IC3IE = 0;	
#endif
    AD1CON1bits.ADON = 0;   //turn OFF ADC module 
	flags.motorRunning = 0;	/* Indicate that the motor has been stopped */
    fanOFF;
}

/******************************************************************************
 * getDriveMovement
 *
 * This function is required to update drive status - Moving up, Moving Down or Stopped 
 * 
 * PARAMETER REQ: none
 *
 * RETURNS: UINT8 - 0 - If Stopped, 1 - If Moving Up, 2 - If Moving down 
 *
 * ERRNO: none
 ********************************************************************************/
UINT8 getDriveMovement()
{
	if(!rampOutputStatus.shutterMoving)
	{
		return SHUTTER_STOPPED; 
	}
	else if(rampOutputStatus.shutterMovementDirection)
	{
		return SHUTTER_MOVING_DOWN; 
	}
	else 
	{
		return SHUTTER_MOVING_UP; 
	}
}

/******************************************************************************
 * startRampGenerator
 *
 * This function starts timer 4 for Ramp generator .
 * 
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID startRampGenerator(VOID)
{
    TMR4 = 0;			    /* Reset timer 4 for ramp generator */
    IFS1bits.T4IF = 0;		/* Clear timer 4 flag */
    IEC1bits.T4IE = 1;		/* Enable interrupts for timer 4 */
    T4CONbits.TON = 1;
}
    
/******************************************************************************
 * stopRampGenerator
 *
 * This function stops timer4 for ramp generator and calls stop motor function.
 * 
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID stopRampGenerator(VOID)
{
    IEC1bits.T4IE = 0;		/* Disable interrupts for timer 4 */
    T4CONbits.TON = 0;
    DISICNT = 0;
}
#ifndef PROGRAMMABLE_DEBOUNCE
VOID monitorSafetySensors(VOID)
{
    //Monitor all sensor modules
    SHORT i;
    for(i = 0; i < SAFETY_SENSOR_END; i++)
    {
        //Read current sensor value
        sensorList[i].sensorCurrVal = sensorList[i].sensorPortPin();
        //if current value is not equal to previous value then set status as toggled and reset the debounce count
        if(sensorList[i].sensorCurrVal != sensorList[i].sensorPrevVal)
        {
            sensorList[i].sensorPrevVal = sensorList[i].sensorCurrVal;
            sensorList[i].sensorDebounceCnt = 0;
            sensorList[i].sensorData = (i == ORIGIN_SENSOR) ? hallCounts : 0;
        }
        else //if steady state is observed for 5ms then treat this as final value
        {
            sensorList[i].sensorDebounceCnt++;
            if(sensorList[i].sensorDebounceCnt > SAFETY_SENSOR_DEBOUNCE_CNT)
            {
                sensorList[i].sensorDebounceCnt = SAFETY_SENSOR_DEBOUNCE_CNT;
                sensorList[i].sensorCurrSteadyVal = sensorList[i].sensorCurrVal;
                if(sensorList[i].sensorCurrSteadyVal != sensorList[i].sensorPrevSteadyVal)
                {
                    sensorList[i].sensorFuncPtr(sensorList[i].sensorCurrSteadyVal);                    
                    sensorList[i].sensorPrevSteadyVal = sensorList[i].sensorCurrSteadyVal;
                }
            }
        }
    }
}
#else
VOID monitorSafetySensors(VOID)
{
    //Monitor all sensor modules
    SHORT i;
    for(i = 0; i < SAFETY_SENSOR_END; i++)
    {
        //Read current sensor value
        sensorList[i].sensorCurrVal = sensorList[i].sensorPortPin();
        //if current value is not equal to previous value check for false debounce
        if(sensorList[i].sensorCurrVal != sensorList[i].sensorPrevVal)
        {
			//	Increment sensor steady state duration count
			sensorList[i].sensorSteadyStateCnt++;
			//	If steady state duration count equals or exceeds false debounce time
			//	 then set status as toggled and reset the debounce count and steady state duration count
			if(sensorList[i].sensorSteadyStateCnt >= FALSE_DEBOUNCE_TIME)
			{
				sensorList[i].sensorSteadyStateCnt = 0;
				sensorList[i].sensorPrevVal = sensorList[i].sensorCurrVal;
				if(sensorList[i].sensorCurrVal == HIGH)
				{
					//	Clear sensor high logic state debouce count
					sensorList[i].sensorHighDebounceCnt = 0;
				}
				else if(sensorList[i].sensorCurrVal == LOW)
				{
					//	Clear sensor low logic state debouce count
					sensorList[i].sensorLowDebounceCnt = 0;
				}
				else
				{
					
				}
				sensorList[i].sensorData = (i == ORIGIN_SENSOR) ? hallCounts : 0;
			}
        }
        else //if steady state is observed for sensorActiveDebounceValue[i] then treat this as final value
        {
			if(sensorList[i].sensorCurrVal == HIGH)
			{
				sensorList[i].sensorHighDebounceCnt++;
				if(sensorList[i].sensorHighDebounceCnt >= sensorActiveDebounceValue[i])
				{
					//sensorList[i].sensorHighDebounceCnt = sensorActiveDebounceValue[i];
					sensorList[i].sensorCurrSteadyVal = sensorList[i].sensorCurrVal;
					if(sensorList[i].sensorCurrSteadyVal != sensorList[i].sensorPrevSteadyVal)
					{
						sensorList[i].sensorFuncPtr(sensorList[i].sensorCurrSteadyVal);                    
						sensorList[i].sensorPrevSteadyVal = sensorList[i].sensorCurrSteadyVal;
					}
				}
			}
			else if(sensorList[i].sensorCurrVal == LOW)
			{
				sensorList[i].sensorLowDebounceCnt++;
				if(sensorList[i].sensorLowDebounceCnt >= sensorInactiveDebounceValue[i])
				{
					//sensorList[i].sensorLowDebounceCnt = sensorInactiveDebounceValue[i];
					sensorList[i].sensorCurrSteadyVal = sensorList[i].sensorCurrVal;
					if(sensorList[i].sensorCurrSteadyVal != sensorList[i].sensorPrevSteadyVal)
					{
						sensorList[i].sensorFuncPtr(sensorList[i].sensorCurrSteadyVal);                    
						sensorList[i].sensorPrevSteadyVal = sensorList[i].sensorCurrSteadyVal;
					}
				}				
			}
        }
    }
}
#endif	//	PROGRAMMABLE_DEBOUNCE

VOID updatePhotoElectricDebounceTime(VOID)
{
#ifdef PROGRAMMABLE_DEBOUNCE
    sensorActiveDebounceValue[PHOTOELECTRIC_SENSOR] = uDriveApplBlockEEP.stEEPDriveApplBlock.snowModePhotoelec_A008;
#endif
}

//Reset sensor status after it get read by control board
VOID resetSensorStatus(VOID)
{    
    if(!photoElecObsSensTrigrd)
    {
       uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.peObstacle = FALSE; 
    }
    if(!microSwSensorTrigrd)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.microSwitch = FALSE;
    }
    if(!tempSensTrigrd)
    {
       uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorOverheat = FALSE;  
    }
    if(!emergencySensorTrigrd)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.emergencyStop = FALSE; 
    }
    //if(!OvercurrentfaultTrigrd)
    //{
    //    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorOverCurrent = FALSE; 
    //}
    
    if(originSensorDetected && uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osDetectOnUp = FALSE;
    }
    
    if(!originSensorDetected && uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osDetectOnDown = FALSE;
    }
    
    updateSenStsCmd = FALSE;
}

#ifndef PROGRAMMABLE_DEBOUNCE
VOID initSensorList(VOID)
{    
    //Initialize all sensor modules
    SHORT i;
    for(i = 0; i < SAFETY_SENSOR_END; i++)
    {
        sensorList[i].sensorCurrVal = sensorList[i].sensorPortPin();
        sensorList[i].sensorPrevVal = sensorList[i].sensorCurrVal;
        sensorList[i].sensorCurrSteadyVal = sensorList[i].sensorCurrVal;
        sensorList[i].sensorPrevSteadyVal = sensorList[i].sensorCurrVal;
        sensorList[i].sensorDebounceCnt = 0;
        sensorList[i].sensorData = (i == ORIGIN_SENSOR)?uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128:0;
    }
    //init sensor current status
    emergencySensorTrigrd = sensorList[EMERGENCY_SENSOR].sensorCurrSteadyVal; 	
    microSwSensorTrigrd = sensorList[MICRO_SW_SENSOR].sensorCurrSteadyVal; 		
    photoElecObsSensTrigrd = sensorList[PHOTOELECTRIC_SENSOR].sensorCurrSteadyVal; 	
    tempSensTrigrd = sensorList[TEMPERATURE_SENSOR].sensorCurrSteadyVal; 			
    originSensorDetected = sensorList[ORIGIN_SENSOR].sensorCurrSteadyVal;  
}
#else
VOID initSensorList(VOID)
{    
    //Initialize all sensor modules
    SHORT i;
    for(i = 0; i < SAFETY_SENSOR_END; i++)
    {
        sensorList[i].sensorCurrVal = sensorList[i].sensorPortPin();
        sensorList[i].sensorPrevVal = sensorList[i].sensorCurrVal;
        sensorList[i].sensorCurrSteadyVal = sensorList[i].sensorCurrVal;
        sensorList[i].sensorPrevSteadyVal = sensorList[i].sensorCurrVal;
        sensorList[i].sensorHighDebounceCnt = 0;
		sensorList[i].sensorLowDebounceCnt = 0;
        sensorList[i].sensorData = (i == ORIGIN_SENSOR)?uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128:0;        
    }
    //init sensor current status
    emergencySensorTrigrd = sensorList[EMERGENCY_SENSOR].sensorCurrSteadyVal; 	
    microSwSensorTrigrd = sensorList[MICRO_SW_SENSOR].sensorCurrSteadyVal; 		
    photoElecObsSensTrigrd = sensorList[PHOTOELECTRIC_SENSOR].sensorCurrSteadyVal; 	
    tempSensTrigrd = sensorList[TEMPERATURE_SENSOR].sensorCurrSteadyVal; 			
    originSensorDetected = sensorList[ORIGIN_SENSOR].sensorCurrSteadyVal;  
}
#endif	//	PROGRAMMABLE_DEBOUNCE

SHORT readEmergencySensorSts(VOID)
{
    return(emergencySensorSts);
}

SHORT readMicroSwSensorSts(VOID)
{
    return(microSwSensorSts);
}

SHORT readAirSwSensorSts(VOID)
{
    return(AirSwSensorSts);
}

SHORT readWrapSwSensorSts(VOID)
{
    return(wrapSwSensorSts);
}

SHORT readPhotoElecSensorSts(VOID)
{
    return(photoElecSensorSts);
}

SHORT readTemperatureSensorSts(VOID)
{
    return(temperatureSensorSts);
}

SHORT readOriginSensorSts(VOID)
{
    return(originSensorSts);
}

SHORT readFourPtSensorSts(VOID)
{
    return(fourPtSensorSts);
}

SHORT readPowerFailSensorSts(VOID)
{
    return(powerFailSensorSts);
}


/******************************************************************************
 * _T4Interrupt
 *
 * The _T4Interrupt generates ramp for shutter operation.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/ 
void __attribute__((interrupt, no_auto_psv)) _T4Interrupt (void)
{    
    static SHORT ms10Cnt = 0;
    
    IFS1bits.T4IF = 0;
    
    //checkParamUpdateToEEP();
	//	Commented if statement to handle "offset at upper & lower limit"
	//	Now executeRampState() will be called every 10mS
	//  Reopen the logic as rising time, falling time operates five time faster than the configured value - YG - NOV 15
    if(++ms10Cnt >= RAMP_GENERATOR_PERIOD_CNT)
    {
        ms10Cnt = 0;
        checkRampCommand();
        executeRampState();  
        updateShutterOutputStatus();

		//	If shutter is moving in wrong direction stop the shutter
		if(
			(gucShutterFalseUpMovementCount >= MAX_FALSE_MOVEMENT_COUNT_LIMIT) ||
			(gucShutterFalseDownMovementCount >= MAX_FALSE_MOVEMENT_COUNT_LIMIT)
		)
		{
			forceStopShutter();
			// Added for displaying errors on display screen in case of false movement - RN - NOV 2015
			if(gucShutterFalseUpMovementCount)
			{
				uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.shutterFalseUpMovementCount = TRUE;
			}
			else
			{
				uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.shutterFalseDownMovementCount = TRUE;
			}
		}
#ifdef DEBUG_SHUTTER_MOVEMENT_IN_WRONG_DIRECTION
		//	Monitor how many times shutter has moved in wrong direction
		if(gucShutterFalseUpMovementCount > gucTempFalseMovementCount)
		{
			gucTempFalseMovementCount = gucShutterFalseUpMovementCount;
		}
		if(gucShutterFalseDownMovementCount > gucTempFalseMovementCount)
		{
			gucTempFalseMovementCount = gucShutterFalseDownMovementCount;
		}

		//	Capture false movement count
		//uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.minDutyCycle_A510 = gucTempFalseMovementCount;

#endif
       
        if(flags.exstFanOn) //if fan is on check timer is expired or not
        {
            if(++fanOnCnt > FAN_ON_TIME_CNT)
            {
                fanOFF;
                flags.exstFanOn = 0;
            }
        }
    }
}

//VOID checkParamUpdateToEEP(VOID)
//{	
//    if((!rampOutputStatus.shutterMoving) && (!rampStatusFlags.rampDcInjectionOn))
//    {
//        //if(rampStatusFlags.saveParamToEeprom)
//        {
//            //Save hall counts after 200ms when motor is stopped.
//            if(saveParamToEepromCnt++ >= 50)
//            {
//                //save current position to eeprom
//                //uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129 = hallCounts;
//                //writeWORD(EEP_CURRENT_VAL_MONITOR, uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129); 				
//                //writeWORD(EEP_ORIGIN_SENS_DRIFT_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637); 
//                //updateCommonBlockCrc(); 
//                
//                //writeDWORD(EEP_OPERATION_COUNT,uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600);
//                //writeWORD(EEP_MAINTENANCE_COUNT_VALUE,uDriveApplBlockEEP.stEEPDriveApplBlock.maintenanceCountValue_A636);
//                //writeDWORD(EEP_APERTURE_HEIGHT_OP_COUNT,uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604);
//                //writeBYTE(EEP_MICRO_SENS_COUNT,uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorCounter_A080);
//                //updateApplBlockCrc();
//                
//                //rampStatusFlags.saveParamToEeprom = FALSE;
//            }
//        }
//    }
//}

/******************************************************************************
 * _INT0Interrupt
 *
 * This function service the input change notification on port pins
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/ 
void __attribute__ ((interrupt, no_auto_psv)) _INT0Interrupt(void)
{
    IFS0bits.INT0IF = 0;
    
    if(readPowerFailSensorSts())
    {
        //pwmBufferControl(DISABLE);
        //lockApply;
        //INTCON2bits.GIE = 0; //Disable all interrupts
        
        forceStopShutter();

        uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129 = hallCounts;        
        //if installation was in progress then reset shutter positions
        if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
        {
            uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 = 0; 
            uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 = 0; 
            writeWORD(EEP_UPPER_STOPPING_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100); 
            writeWORD(EEP_LOWER_STOPPING_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101); 
            writeWORD(EEP_PHOTOELEC_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102); 
            writeWORD(EEP_ORIGIN_SENS_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128); 
        }        
        writeWORD(EEP_CURRENT_VAL_MONITOR, uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129);
        writeWORD(EEP_ORIGIN_SENS_DRIFT_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637); 
        updateCommonBlockCrc(); 
        
        writeDWORD(EEP_OPERATION_COUNT,uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600);
        //writeWORD(EEP_MAINTENANCE_COUNT_VALUE,uDriveApplBlockEEP.stEEPDriveApplBlock.maintenanceCountValue_A636);
        //writeDWORD(EEP_APERTURE_HEIGHT_OP_COUNT,uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604);
        writeDWORD(EEP_APERTURE_HEIGHT_OP_COUNT,uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604);
        writeBYTE(EEP_MICRO_SENS_COUNT,uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorCounter_A080);
        updateApplBlockCrc();
    }
}

//	Added on 20Feb2015 for IGBT over temperature fault
/******************************************************************************
 * _INT1Interrupt
 *
 * This function service the input change notification on port pin RB4
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/ 
void __attribute__ ((interrupt, no_auto_psv)) _INT1Interrupt(void)
{
    IFS1bits.INT1IF = 0;
    //PORTAbits.RA7 = 0;
    /*if(readIgbtOverTemperatureSensorSts())
    {
        PORTAbits.RA7 = 0;
    }
	else
	{
		PORTAbits.RA7 = 1;
	}*/
    
    if(PWMCON1bits.FLTSTAT)
		overcurrentfaultTriggered(TRUE);
#if 1
    else
        igbtOverTempSensorTriggered(TRUE);
#endif
}

VOID calculateDrift(BOOL sts)
{
    originSensorDetected = sts;
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.originSensorStatus = originSensorDetected;
    //If system is in ready state then calculate drift
    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady)
    {
        //If drift was not previously calculated
        if(!rampStatusFlags.rampDriftCalculated)
        {
            //Calculate drift only during Ramp down going profile
			//	Commented RAMP_GOING_UP_PROFILE as drift is to be calculated only while shutter is moving down
			//	Do this only when power fail flag is off. This check is needed as it was observed that during
			//	powerfail condition, while running executePowerFailRoutine() value of A129 was getting changed
			//	resulting in dbCRC error upon next power on. Similarly A637 may change. Added - Feb 2016
            if(/*(currentRampProfileNo == RAMP_GOING_UP_PROFILE) ||*/ 
				(currentRampProfileNo == RAMP_GOING_DN_PROFILE) &&
				(!gucPowerFailFlag)
			)
            {
                //if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady)
                //{
                    //Calculate the drift and set drift calculated status for position calculations.            
                    uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637 = sensorList[ORIGIN_SENSOR].sensorData \
                        - uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128;
                    rampStatusFlags.rampDriftCalculated = 1;
                //}
            }
        }
    }
    
    if(uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604 != 0)
    {
        // Reset aperture frequency count whenever we cross origin sensor 
        uDriveApplBlockEEP.stEEPDriveApplBlock.apertureHeightOperCount_A604 = 0; 
        //rampStatusFlags.saveParamToEeprom = TRUE;
    }
    //update orign sensor toggled status
    rampTripSts.rampOrgSenToggled = 1;
    //update orign sensor toggled status for calibration
    powerUpCalib.osToggle = 1;
    //update orign sensor toggled status for installation
    shutterInstall.osToggle = 1;
    //update status to control board
    updateSenStsCmd = TRUE;
}

VOID checkPhotoElecObsLevel(BOOL sts)
{
    photoElecObsSensTrigrd = sts;
    rampCurrentPosition = hallCounts;
    
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.peSensorStatus = photoElecObsSensTrigrd;
    //photo electic sensor is normally closed, it is triggered when opened
    if(photoElecObsSensTrigrd)
    {
        //If system is in ready state then process photoelectric trigger
        if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady)
        {
            //Sense photo electric only during Ramp down going profile
            if((currentRampProfileNo == RAMP_GOING_DN_PROFILE) || (currentRampProfileNo == RAMP_APERTURE_DN_PROFILE))
            {
                //If shutter is moving in closing direction then only use photo electric trigger
                if(rampOutputStatus.shutterMoving && (requiredDirection == CCW))
                   {
                       //if current shutter position is above ignore PE level then only trigger stop shutter
                       if(rampCurrentPosition < uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102)
                       {
                           rampCurrentState = RAMP_STOP; //Set the current state to ramp stop
                           calcShtrMinDistValue();
						   //	Stop shutter initiated by safety sensor
						   //	Clear flag so as to stop the shutter immediately
						   gui8StopKeyPressed = 0;
                           stopShutter(); //stop shutter immediately
                           photElecSensorFault = TRUE;
                           //set fault status
                           uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.peObstacle = TRUE;
                       }
                   }
            }
        }
    }
    else
    {
        photElecSensorFault = FALSE; //clear the photo electric fault        
        updateSenStsCmd = TRUE;
    }
}

VOID microSwSensorTiggered(BOOL sts)
{
    microSwSensorTrigrd = sts;
    
    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.microSwitchSensorStatus = microSwSensorTrigrd;
    //micro switch is normally open, it is triggered when closed
    if(microSwSensorTrigrd)
    {
        //Sense micro switch only during Ramp up/down going profile
        if((currentRampProfileNo == RAMP_GOING_UP_PROFILE) || (currentRampProfileNo == RAMP_GOING_DN_PROFILE))
        {
            //if micro SW sensor is triggered the stop shutter immediately
            rampCurrentState = RAMP_STOP; //Set the current state to ramp stop
            calcShtrMinDistValue();
			//	Stop shutter initiated by safety sensor
			//	Clear flag so as to stop the shutter immediately
			gui8StopKeyPressed = 0;
            stopShutter(); //stop shutter immediately
            //set mocro switch error flag
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.microSwitch = TRUE;
            
            // Increment micro switch sensor count - A080 
			uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorCounter_A080++; 
            // if count has exceeded limit value A603 
			if(uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorCounter_A080
               > uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorLimValue_A603)
			{
                //once microswitch limit is triggered then set the limit from 10 to 50
                uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorLimValue_A603 = 50;                
                writeBYTE(EEP_MICRO_SENSOR_LIMIT_VAL,uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorLimValue_A603);
                updateApplBlockCrc();
                //set max microSwitchSensorLimit error flag
                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.microSwitchSensorLimit = TRUE;
			}
            else
            {
                //reset max microSwitchSensorLimit error flag
                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.microSwitchSensorLimit = FALSE;                
            }
            //rampStatusFlags.saveParamToEeprom = TRUE;
        }
    }
    else
    {
        updateSenStsCmd = TRUE;
    }
}

VOID tempSensorTriggered(BOOL sts)
{
    tempSensTrigrd = sts;
    //temperature sensor is normally open, it is triggered when closed
    if(tempSensTrigrd)
    {
        //Sense over temperature in all the profiles
        if(currentRampProfileNo != RAMP_PROFILE_END)
        {
            //if temperature sensor is triggered the stop shutter immediately
            rampCurrentState = RAMP_STOP; //Set the current state to ramp stop
			//	Stop shutter initiated by safety sensor
			//	Clear flag so as to stop the shutter immediately
			gui8StopKeyPressed = 0;
            stopShutter(); //stop shutter immediately
            //set over temperature error flag
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorOverheat = TRUE;
        }
    }
    else
    {
        updateSenStsCmd = TRUE;
    }
}

//	Added on 20Feb2015 for IGBT over temperature fault
VOID igbtOverTempSensorTriggered(BOOL sts)
{
	//PORTAbits.RA7 = 0;
	
    //emergency stop sensor is normally open, it is triggered when closed
    if(sts)
    {
        //Sense emergency in all the profiles
        if(currentRampProfileNo != RAMP_PROFILE_END)
        {
            //if emergency switch is triggered the stop shutter immediately
            Motor_ERR_overcurrent_or_igbtOverTemp=1;
            forceStopShutter();
            //set emergency stop error flag
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.igbtOverTemperature = TRUE; 
        }
    }
    //else
    //{
    //    updateSenStsCmd = TRUE;
    //}
	
}

VOID fourPtLimitSwTriggered(BOOL sts)
{
    fourPtLmtSwtchDetected = sts;
    //four point limit switch is normally open, it is triggered when closed
    if(fourPtLmtSwtchDetected)
    {
        //Sense four point limit switch in all the profiles
        if(currentRampProfileNo != RAMP_PROFILE_END)
        {
            //if four point limit switch is triggered the stop shutter immediately
            rampCurrentState = RAMP_STOP; //Set the current state to ramp stop
			//	Stop shutter initiated by safety sensor
			//	Clear flag so as to stop the shutter immediately
			gui8StopKeyPressed = 0;
            stopShutter(); //stop shutter immediately
        }
    }
}

VOID emergencySensorSwTriggered(BOOL sts)
{
	// Disabled emergency stop functionality for version 4 board as emergency stop is not present in it - RN - Nov 15
#if 0
    emergencySensorTrigrd = sts;
    
    //emergency stop sensor is normally open, it is triggered when closed
    if(emergencySensorTrigrd)
    {
        //Sense emergency in all the profiles
        if(currentRampProfileNo != RAMP_PROFILE_END)
        {
            //if emergency switch is triggered then do PWM costing and stop shutter
            rampCurrentState = RAMP_PWM_COASTING;
            pwmCostingReq = TRUE;
            //set the clamp limit for outputs
            currentLimitClamp = controlOutput;
            outputDecRate = __builtin_divud(currentLimitClamp, PWM_COASTING_TIME);
            //set emergency stop error flag
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.emergencyStop = TRUE; 
        }
    }
    else
    {
        updateSenStsCmd = TRUE;
    }
#endif
}

VOID overcurrentfaultTriggered(BOOL sts)
{
    OvercurrentfaultTrigrd = sts;
    //emergency stop sensor is normally open, it is triggered when closed
    if(OvercurrentfaultTrigrd)
    {
        Motor_ERR_overcurrent_or_igbtOverTemp=1;
        //Sense emergency in all the profiles
        if(currentRampProfileNo != RAMP_PROFILE_END)
        {
            //if emergency switch is triggered the stop shutter immediately
            forceStopShutter();
            //set emergency stop error flag
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorOverCurrent = TRUE; 
        }
    }
    //else
    //{
    //    updateSenStsCmd = TRUE;
    //}
}

VOID checkPwmCoastingRequired(VOID)
{
    if(rampCurrentState == RAMP_PWM_COASTING)
    {
        if(pwmCostingReq == TRUE)
        {
            if(currentLimitClamp > 0)
            {
                currentLimitClamp -= outputDecRate;
                if(currentLimitClamp < 0)
                    currentLimitClamp = 0;
            }
            else
            {
                currentLimitClamp = 0;
                pwmCostingReq = FALSE;
                forceStopShutter();
                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorPWMCosting = TRUE;
            }
        }        
    }
}

VOID checkRampTripStatus(VOID)
{
    //SHORT positionError;
    //Monitor ramp trip staus in up going and down going direction
    //Reset the hall counts when shutter has reached to upper limit or lower limit
    
    //Read current position
    rampCurrentPosition = hallCounts;
    
    //Check the ramp trip only during full ramp operation.
    if((currentRampProfileNo == RAMP_GOING_UP_PROFILE) && (!rampTripSts.rampTripUpCompleted))
    {
        //if Up trip completed then reset the hall counts to upper limit
        if(rampTripSts.rampTripUpStarted)
        {
			//	Commented to handle "offset at upper & lower limit"
			//	No need to equate hallCounts to A100
            //hallCounts = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
            rampTripSts.rampTripUpCompleted = 1;
            initProfileData();
            //Reset current profile parameters
            currentRampProfile = *currentRampProfilePtr;
                
            //if origin sensor not detected during trip set the error flag
            if(!rampTripSts.rampOrgSenToggled)
            {
                //set the error flag
                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osNotDetectUP = TRUE;
            }
            else
            {
                //reset the error flag
                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osNotDetectUP = FALSE;
                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osNotDetectDown = FALSE;
                rampTripSts.rampOrgSenToggled = 0;
            }            
        }
    }
    else
    {
        rampTripSts.rampTripUpStarted = 0;
        rampTripSts.rampTripUpCompleted = 1;
    }
        
    if((currentRampProfileNo == RAMP_GOING_DN_PROFILE) && (!rampTripSts.rampTripDnCompleted))
    {
        //if Dn trip completed then reset the hall count to lower limit
        if(rampTripSts.rampTripDnStarted)
        {
			//	Commented to handle "offset at upper & lower limit"
			//	No need to equate hallCounts to A101
            //hallCounts = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
            rampTripSts.rampTripDnCompleted = 1;
            initProfileData();
            //Reset current profile parameters
            currentRampProfile = *currentRampProfilePtr;
                
            //if origin sensor not detected during trip set the error flag
            if(!rampTripSts.rampOrgSenToggled)
            {
                //set the error flag
                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osNotDetectDown = TRUE;
            }
            else
            {
                //reset the error flag
                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osNotDetectUP = FALSE;
                uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osNotDetectDown = FALSE;
                rampTripSts.rampOrgSenToggled = 0;
            }  
        }
    }
    else //Reset the trip status flags
    {
        rampTripSts.rampTripDnStarted = 0;
        rampTripSts.rampTripDnCompleted = 1;
    }
}

VOID checkRampCommand(VOID)
{
    //If energency stop got cleared then clear the flag
    if(rampStatusFlags.safetySensorTriggered)
    {
        rampStatusFlags.safetySensorTriggered = 0;
        rampStatusFlags.rampOpenInProgress = 0;
        rampStatusFlags.rampCloseInProgress = 0;
    }
    
    if(inputFlags.bits.shutterStop)
    {
        if(rampOutputStatus.shutterMoving)
        {
            rampCurrentState = RAMP_STOP;
            rampStatusFlags.rampOpenInProgress = 0;
            rampStatusFlags.rampCloseInProgress = 0;
        }
    }        
    else if(inputFlags.bits.shutterOpen)
    {  
        if(rampStatusFlags.rampCloseInProgress && rampOutputStatus.shutterMoving)
        {
            rampCurrentState = RAMP_STOP;
        }
        else
        {
            if(
				((!rampStatusFlags.rampOpenInProgress) && (!rampOutputStatus.shutterMoving)) ||
				// Added to overcome installation issue (A100) - RN- NOV 2015
				(gucInstallationInitiated && (inputFlags.value == OPEN_SHUTTER_JOG_10))
			)
            {
				gucInstallationInitiated = SERVICED;
                rampCurrentState = RAMP_START;
                rampStatusFlags.rampOpenInProgress = 1;
                rampStatusFlags.rampCloseInProgress = 0;
                rampTripSts.rampTripUpStarted = 1;
                rampTripSts.rampTripUpCompleted = 0;                        
                if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129 <= \
                   uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128)
                {
                    if(!sensorList[ORIGIN_SENSOR].sensorCurrSteadyVal)
                    {
                        rampTripSts.rampOrgSenToggled = 0;
                    }
                    else
                    {
                        rampTripSts.rampOrgSenToggled = 1;
                    }
                }
                else
                {
                    rampTripSts.rampOrgSenToggled = 0;
                }                        
            }
        }
    }        
    else if(inputFlags.bits.shutterClose)
    {
        if(rampStatusFlags.rampOpenInProgress && rampOutputStatus.shutterMoving)
        {
            rampCurrentState = RAMP_STOP;
        }
        else
        {
            if(
				((!rampStatusFlags.rampCloseInProgress) && (!rampOutputStatus.shutterMoving)) ||
				// Added to overcome installation issue (A100) - RN- NOV 2015
				(gucInstallationInitiated && (inputFlags.value == CLOSE_SHUTTER_JOG_10))
			)
            {
				gucInstallationInitiated = SERVICED;
                rampCurrentState = RAMP_START;
                rampStatusFlags.rampCloseInProgress = 1;
                rampStatusFlags.rampOpenInProgress = 0;
                rampTripSts.rampTripDnStarted = 1;
                rampTripSts.rampTripDnCompleted = 0;                        
                if(uDriveCommonBlockEEP.stEEPDriveCommonBlock.currentValueMonitor_A129 >= \
                   uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128)
                {
                    if(sensorList[ORIGIN_SENSOR].sensorCurrSteadyVal)
                    {
                        rampTripSts.rampOrgSenToggled = 0;
                    }
                    else
                    {
                        rampTripSts.rampOrgSenToggled = 1;
                    }
                }
                else
                {
                    rampTripSts.rampOrgSenToggled = 0;
                }                         
            }
        }
    } 
}

VOID updateShutterOutputStatus(VOID)
{
    rampOutputStatus.shutterCurrentPosition = hallCounts;
    if(rampOutputStatus.shutterMoving) //if shutter is moving then update the direction
    {
        rampOutputStatus.shutterMovementDirection = requiredDirection;
    }
    else //if shutter is not moving then ignore direction status bit
    {
        rampOutputStatus.shutterMovementDirection = CW;
    }
}

VOID executeRampState(VOID)
{
    //if drift was calculated then readjust ramp profile
    if(rampStatusFlags.rampDriftCalculated)
    {
        reAdjustRampPositions();
        //reset the flag of drift calculated
        rampStatusFlags.rampDriftCalculated = 0;
        //Reset current profile parameters
        currentRampProfile = *currentRampProfilePtr;
    }
    
    switch(rampCurrentState)
    {
        case RAMP_START:
            {
                //Ramp profile should be initialized only at trip start
                //current profile should not be modified after origin sensor cross
                //and drift calculation.
                if(!rampTripSts.rampOrgSenToggled)
                {
                    initProfileData();
                }
                startShutter();
                break;
            }
            
        case RAMP_RESTART:
            {
                runShutterToReqSpeed();
                break;
            }
            
        case RAMP_RUNNING:
            {
                executeRampProfile();
                break;
            }
            
        case RAMP_STOP:
            {
                stopShutter();
                break;
            }
            
        default:            
            break;        
    }
}

VOID forceStopShutter(VOID)
{
    stopMotor();
    lockApply;
    inputFlags.value = STOP_SHUTTER;
    rampCurrentState = RAMP_STATE_END;
    currentRampProfileNo = RAMP_PROFILE_END;
    rampStatusFlags.rampBrakeOn = 1;
    rampStatusFlags.rampDcInjectionOn = 0;
    rampOutputStatus.shutterMoving = 0;
    rampStatusFlags.rampSpeedControlRequired = 0;
    rampStatusFlags.rampCurrentControlRequired = 0;
    rampStatusFlags.rampOpenInProgress = 0;
    rampStatusFlags.rampCloseInProgress = 0;    
}

VOID startShutter(VOID)
{   
    //Select profile for execution
    rampCurrentStep = 0;
    if(inputFlags.bits.shutterOpen && (inputFlags.bits.jogPercentage == 0))
    {
        if(!inputFlags.bits.aperture)
        {
            //Ramp up profile
            currentRampProfileNo = RAMP_GOING_UP_PROFILE;           
            if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.upStepCount_A525 == TWO_STEP_RAMP_PROFILE)
                rampTotalStates = (RAMP_UP_STATES - 2);
            else
                rampTotalStates = RAMP_UP_STATES;
            currentRampProfilePtr = (rampStructure_t*)rampUpGoingProfile;
            currentRampProfile = *currentRampProfilePtr;
        }
        else
        {
            //Aperture up profile
            currentRampProfileNo = RAMP_APERTURE_UP_PROFILE;
            rampTotalStates = APER_UP_STATES;
            currentRampProfilePtr = (rampStructure_t*)rampApertureUpProfile;
            currentRampProfile = *currentRampProfilePtr; 
        }
    }
    else if(inputFlags.bits.shutterOpen && (inputFlags.bits.jogPercentage == 1))
    {
        //Inch up profile
        currentRampProfileNo = RAMP_INCH_UP_PROFILE;
        rampTotalStates = INCH_UP_STATES;
        currentRampProfilePtr = (rampStructure_t*)rampInchUpProfile;
        currentRampProfile = *currentRampProfilePtr;
    }
    else if(inputFlags.bits.shutterOpen && (inputFlags.bits.jogPercentage == 2))
    {
        //Jog up profile
        currentRampProfileNo = RAMP_JOG_UP_PROFILE;
        rampTotalStates = JOG_UP_STATES;
        currentRampProfilePtr = (rampStructure_t*)rampJogUpProfile;
        currentRampProfile = *currentRampProfilePtr;
    }
    else if(inputFlags.bits.shutterClose && (inputFlags.bits.jogPercentage == 0))
    {
        if(!inputFlags.bits.aperture)
        {
            //Ramp dn profile
            currentRampProfileNo = RAMP_GOING_DN_PROFILE;
            
            if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.downStepCount_A531 == TWO_STEP_RAMP_PROFILE)
                rampTotalStates = (RAMP_DN_STATES - 2);
            else
                rampTotalStates = RAMP_DN_STATES;        
            
            currentRampProfilePtr = (rampStructure_t*)rampDnGoingProfile;
            currentRampProfile = *currentRampProfilePtr;
        }
        else
        {
            //Aperture dn profile
            currentRampProfileNo = RAMP_APERTURE_DN_PROFILE;
            rampTotalStates = APER_DN_STATES;
            currentRampProfilePtr = (rampStructure_t*)rampApertureDnProfile;
            currentRampProfile = *currentRampProfilePtr;
        }
    }
    else if(inputFlags.bits.shutterClose && (inputFlags.bits.jogPercentage == 1))
    {
        //Inch dn profile
        currentRampProfileNo = RAMP_INCH_DN_PROFILE;
        rampTotalStates = INCH_DN_STATES;
        currentRampProfilePtr = (rampStructure_t*)rampInchDnProfile;
        currentRampProfile = *currentRampProfilePtr;
    }
    else if(inputFlags.bits.shutterClose && (inputFlags.bits.jogPercentage == 2))
    {
        //Jog dn profile
        currentRampProfileNo = RAMP_JOG_DN_PROFILE;
        rampTotalStates = JOG_DN_STATES;
        currentRampProfilePtr = (rampStructure_t*)rampJogDnProfile;
        currentRampProfile = *currentRampProfilePtr;
    }
    
    if((currentRampProfileNo == RAMP_GOING_UP_PROFILE) || (currentRampProfileNo == RAMP_GOING_DN_PROFILE))
    {
        //If previously shutter stopped in between then first run the shutter to required speed
        if(rampStatusFlags.shutterOperationStart && (!rampStatusFlags.shutterOperationComplete))
        {
            //change the state to ramp restart
            rampCurrentState = RAMP_RESTART;
        }
        else
        {
            //Change the ramp state to running
            rampCurrentState = RAMP_RUNNING;
            rampStatusFlags.shutterOperationStart = 1; //shutter operation started 
            rampStatusFlags.shutterOperationComplete = 0; //shutter operation completed
        }
    }
    else
    {
        //Change the ramp state to running
        rampCurrentState = RAMP_RUNNING;
        rampStatusFlags.shutterOperationStart = 1; //shutter operation started 
        rampStatusFlags.shutterOperationComplete = 0; //shutter operation completed
    }
    
    lockActivationDelayCnt = 0; //reset lock activation delay
    lockDeactivationDelayCnt = 0; //reset lock de-activation delay
}

VOID runShutterToReqSpeed(VOID)
{
    //Read current position
    rampCurrentPosition = hallCounts;
    //Read current speed
    rampCurrentSpeed = refSpeed;
    //Read current totalCurrent
    rampCurrentTotCurr = refiTotalCurrent;

    if(currentRampProfile.rampGenFlags.ccwDirection)
    {
        requiredDirection = CCW;
        
        //if mechanical release required then
        if(currentRampProfile.rampGenFlags.brakeRelease && rampStatusFlags.rampBrakeOn)
        {
            //before releasing mechanical brake apply DC injection to hold the shutter load
            if(!rampStatusFlags.rampDcInjectionOn)
            {
                controlOutput = currentRampProfile.dcInjectionDuty;
                rampStatusFlags.rampDcInjectionOn = 1;
                DCInjectionON();  
                rampDcInjectionOnCounter = 0;
                lockDeactivationDelayCnt = 0; //reset lock de-activation delay
            }
            else
            {                
                if(++lockDeactivationDelayCnt >= MECHANICAL_LOCK_DEACTIVATION_DELAY_CNT)
                {
                    lockRelease;
                    rampStatusFlags.rampBrakeOn = 0; 
                }
                    
                //turn off DC injection after required time
                if(++rampDcInjectionOnCounter >= currentRampProfile.dcInjectionTime)
                {
                    rampStatusFlags.rampDcInjectionOn = 0; 
                    if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == BEAD_SHUTTER)
                        rampStatusFlags.rampMaintainHoldingDuty = 0;
                    else
                        rampStatusFlags.rampMaintainHoldingDuty = 1;
                    
                    refSpeed = SHUTTER_SPEED_MIN;
                    refiTotalCurrent = RAMP_STARTING_CURRENT_MIN;
                    
                    //search for current state in ramp profile
                    while(rampCurrentPosition > currentRampProfile.endPosition)
                    {
                        //Goto next step
                        if(++rampCurrentStep < rampTotalStates)
                        {
                            currentRampProfilePtr++; 
                            currentRampProfile = *currentRampProfilePtr;
                        }
                        else
                        {
                            rampCurrentState = RAMP_STOP;
                            stopShutter();
                            rampStatusFlags.shutterOperationStart = 0; 
                            rampStatusFlags.shutterOperationComplete = 1;
                            break;
                        }
                    }                    
                    
                    #if (STARTUP_IN_CURRENT_MODE == 1)
                    if(currentRampProfile.rampGenFlags.currentMode)
                        rampStatusFlags.rampCurrentControlRequired = 1;
                    else
                        rampStatusFlags.rampSpeedControlRequired = 1;
                    #else
                    rampStatusFlags.rampSpeedControlRequired = 1;
                    #endif
                    startMotor(); //After removing brake start motor
                    rampOutputStatus.shutterMoving = 1;                    
                }
            }
            
        } 
        //if only DC injection is ON then first turn OFF DC injection
        else if(currentRampProfile.rampGenFlags.brakeRelease && rampStatusFlags.rampDcInjectionOn)
        {
            rampStatusFlags.rampDcInjectionOn = 0;             
            if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == BEAD_SHUTTER)
                rampStatusFlags.rampMaintainHoldingDuty = 0;
            else
                rampStatusFlags.rampMaintainHoldingDuty = 1;
            
            refSpeed = SHUTTER_SPEED_MIN;
            refiTotalCurrent = RAMP_STARTING_CURRENT_MIN;
            
            //search for current state in ramp profile
            while(rampCurrentPosition > currentRampProfile.endPosition)
            {
                //Goto next step
                if(++rampCurrentStep < rampTotalStates)
                {
                    currentRampProfilePtr++; 
                    currentRampProfile = *currentRampProfilePtr;
                }
                else
                {
                    rampCurrentState = RAMP_STOP;
                    stopShutter();
                    rampStatusFlags.shutterOperationStart = 0; 
                    rampStatusFlags.shutterOperationComplete = 1;
                    break;
                }
            } 
            
            #if (STARTUP_IN_CURRENT_MODE == 1)
            if(currentRampProfile.rampGenFlags.currentMode)
                rampStatusFlags.rampCurrentControlRequired = 1;
            else
                rampStatusFlags.rampSpeedControlRequired = 1;
            #else
            rampStatusFlags.rampSpeedControlRequired = 1;
            #endif
            startMotor(); //After removing brake start motor
            rampOutputStatus.shutterMoving = 1;
        }            
        //if shutter is moving in up direction then start shutter in current mode
        else if(rampCurrentPosition < currentRampProfile.endPosition)
        {
            //If speed mode is required at startup then start in speed mode
            if(currentRampProfile.rampGenFlags.speedMode)
            {
                //if target speed is achived then change the state to running
                if(rampCurrentSpeed < currentRampProfile.endSpeed)
                {
                    //if required state is found then start current mode
                    if(rampCurrentSpeed < RAMP_STARTING_SPEED_MAX)
                    {
                        //Set the current reference
                        refiTotalCurrent = 0;
                        //Disable current loop
                        rampStatusFlags.rampCurrentControlRequired = 0;                    
                        //set the current reference
                        refSpeed += gs16DownDecelaration;
                        //check boundary of max speed
                        if(refSpeed > RAMP_STARTING_SPEED_MAX)
                        {
                            refSpeed = RAMP_STARTING_SPEED_MAX;
                        }
                        //enable speed control
                        rampStatusFlags.rampSpeedControlRequired = 1;                    
                    }
                }
                else
                {
                    //Change the ramp state to running
                    rampCurrentState = RAMP_RUNNING;
                    rampStatusFlags.shutterOperationStart = 1;      //shutter operation started 
                    rampStatusFlags.shutterOperationComplete = 0;   //shutter operation completed
                }
            }
            //if current mode is required at startup then start in current mode
            else if(currentRampProfile.rampGenFlags.currentMode)
            {
                if(rampCurrentTotCurr < currentRampProfile.endCurrent)
                {
                    //Before increasing current check if system already achived required speed
                    if(measuredSpeed < currentRampProfile.endSpeed)
                    {
                        //set the speed reference
                        refSpeed = currentRampProfile.endSpeed;
                        //disable speed control
                        rampStatusFlags.rampSpeedControlRequired = 0;                            
                        //set the current reference
                        refiTotalCurrent += currentRampProfile.currentChangeRate;                            
                        //Limit refcurrent
                        if(refiTotalCurrent > currentRampProfile.endCurrent)
                        {
                            refiTotalCurrent = currentRampProfile.endCurrent;
                        } 
                        //Enable current control
                        rampStatusFlags.rampCurrentControlRequired = 1;
                    }
                    else
                    {
                        //Change the ramp state to running
                        rampCurrentState = RAMP_RUNNING;
                        rampStatusFlags.shutterOperationStart = 1;      //shutter operation started 
                        rampStatusFlags.shutterOperationComplete = 0;   //shutter operation completed
                    }
                }
                else
                {
                    //Change the ramp state to running
                    rampCurrentState = RAMP_RUNNING;
                    rampStatusFlags.shutterOperationStart = 1;      //shutter operation started 
                    rampStatusFlags.shutterOperationComplete = 0;   //shutter operation completed
                }
            } 
        }
        else
        {
            //Goto next step
            if(++rampCurrentStep < rampTotalStates)
            {
                currentRampProfilePtr++; 
                currentRampProfile = *currentRampProfilePtr;
            }
            else
            {
                rampCurrentState = RAMP_STOP;
                stopShutter();
                rampStatusFlags.shutterOperationStart = 0; 
                rampStatusFlags.shutterOperationComplete = 1;
            }
        }        
    }
    else if(currentRampProfile.rampGenFlags.cwDirection)
    {
        requiredDirection = CW;
        
        //if mechanical release required then
        if(currentRampProfile.rampGenFlags.brakeRelease && rampStatusFlags.rampBrakeOn)
        {
            //before releasing mechanical brake apply DC injection to hold the shutter load
            if(!rampStatusFlags.rampDcInjectionOn)
            {
                controlOutput = currentRampProfile.dcInjectionDuty;
                rampStatusFlags.rampDcInjectionOn = 1;
                DCInjectionON();                
                rampDcInjectionOnCounter = 0;
                lockDeactivationDelayCnt = 0;       //reset lock de-activation delay
            }
            else
            {                
                if(++lockDeactivationDelayCnt >= MECHANICAL_LOCK_DEACTIVATION_DELAY_CNT)
                {
                    lockRelease;
                    rampStatusFlags.rampBrakeOn = 0; 
                }
                
                //turn off DC injection after required time
                if(++rampDcInjectionOnCounter >= currentRampProfile.dcInjectionTime)
                {
                    rampStatusFlags.rampDcInjectionOn = 0;  
                    rampStatusFlags.rampMaintainHoldingDuty = 1;
                    refSpeed = SHUTTER_SPEED_MIN;
                    refiTotalCurrent = RAMP_STARTING_CURRENT_MIN;
                    
                    //search for current state in ramp profile
                    while(rampCurrentPosition < currentRampProfile.endPosition)
                    {
                        //Goto next step
                        if(++rampCurrentStep < rampTotalStates)
                        {
                            currentRampProfilePtr++; 
                            currentRampProfile = *currentRampProfilePtr;
                        }
                        else
                        {
                            rampCurrentState = RAMP_STOP;
                            stopShutter();
                            rampStatusFlags.shutterOperationStart = 0; 
                            rampStatusFlags.shutterOperationComplete = 1;
                            break;
                        }
                    } 
                    
                    #if (STARTUP_IN_CURRENT_MODE == 1)
                    if(currentRampProfile.rampGenFlags.currentMode)
                        rampStatusFlags.rampCurrentControlRequired = 1;
                    else
                        rampStatusFlags.rampSpeedControlRequired = 1;
                    #else
                    rampStatusFlags.rampSpeedControlRequired = 1;
                    #endif
                    startMotor();   //After removing brake start motor
                    rampOutputStatus.shutterMoving = 1;
                }
            }            
        }  
        //if only DC injection is ON then first turn OFF DC injection
        else if(currentRampProfile.rampGenFlags.brakeRelease && rampStatusFlags.rampDcInjectionOn)
        {
            rampStatusFlags.rampDcInjectionOn = 0;  
            rampStatusFlags.rampMaintainHoldingDuty = 1;
            refSpeed = SHUTTER_SPEED_MIN;
            refiTotalCurrent = RAMP_STARTING_CURRENT_MIN;
            
            //search for current state in ramp profile
            while(rampCurrentPosition < currentRampProfile.endPosition)
            {
                //Goto next step
                if(++rampCurrentStep < rampTotalStates)
                {
                    currentRampProfilePtr++; 
                    currentRampProfile = *currentRampProfilePtr;
                }
                else
                {
                    rampCurrentState = RAMP_STOP;
                    stopShutter();
                    rampStatusFlags.shutterOperationStart = 0; 
                    rampStatusFlags.shutterOperationComplete = 1;
                    break;
                }
            } 
            
            #if (STARTUP_IN_CURRENT_MODE == 1)
            if(currentRampProfile.rampGenFlags.currentMode)
                rampStatusFlags.rampCurrentControlRequired = 1;
            else
                rampStatusFlags.rampSpeedControlRequired = 1;
            #else
            rampStatusFlags.rampSpeedControlRequired = 1;
            #endif
            startMotor();   //After removing brake start motor
            rampOutputStatus.shutterMoving = 1;
        }            
        //if shutter is moving in dn direction then start shutter in speed mode
        else if(rampCurrentPosition >= currentRampProfile.endPosition)
        {
            //If speed mode is required at startup then start in speed mode
            if(currentRampProfile.rampGenFlags.speedMode)
            {
                //if target speed is achived then change the state to running
                if(rampCurrentSpeed < currentRampProfile.endSpeed)
                {                
                    //if required state is found then start current mode
                    if(rampCurrentSpeed < RAMP_STARTING_SPEED_MAX)
                    {
                        //Set the current reference
                        refiTotalCurrent = 0;
                        //Disable current loop
                        rampStatusFlags.rampCurrentControlRequired = 0;                    
                        //set the speed reference
                        refSpeed += gs16UpDecelaration;
                        //check boundary of max speed
                        if(refSpeed > RAMP_STARTING_SPEED_MAX)
                        {
                            refSpeed = RAMP_STARTING_SPEED_MAX;
                        }
                        //enable speed control
                        rampStatusFlags.rampSpeedControlRequired = 1;
                    }
                }
                else
                {
                    //Change the ramp state to running
                    rampCurrentState = RAMP_RUNNING;
                    rampStatusFlags.shutterOperationStart = 1; //shutter operation started 
                    rampStatusFlags.shutterOperationComplete = 0; //shutter operation completed
                }
            }
            //if current mode is required at startup then start in current mode
            else if(currentRampProfile.rampGenFlags.currentMode)
            {
                if(rampCurrentTotCurr < currentRampProfile.endCurrent)
                {
                    //Before increasing current check if system already achived required speed
                    if(measuredSpeed < currentRampProfile.endSpeed)
                    {
                        //set the speed reference
                        refSpeed = currentRampProfile.endSpeed;
                        //disable speed control
                        rampStatusFlags.rampSpeedControlRequired = 0;                            
                        //set the current reference
                        refiTotalCurrent += currentRampProfile.currentChangeRate;                            
                        //Limit refcurrent
                        if(refiTotalCurrent > currentRampProfile.endCurrent)
                        {
                            refiTotalCurrent = currentRampProfile.endCurrent;
                        } 
                        //Enable current control
                        rampStatusFlags.rampCurrentControlRequired = 1;
                    }
                    else
                    {
                        //Change the ramp state to running
                        rampCurrentState = RAMP_RUNNING;
                        rampStatusFlags.shutterOperationStart = 1; //shutter operation started 
                        rampStatusFlags.shutterOperationComplete = 0; //shutter operation completed
                    }
                }
                else
                {
                    //Change the ramp state to running
                    rampCurrentState = RAMP_RUNNING;
                    rampStatusFlags.shutterOperationStart = 1; //shutter operation started 
                    rampStatusFlags.shutterOperationComplete = 0; //shutter operation completed
                }
            } 
        }
        else
        {
            //Goto next step
            if(++rampCurrentStep < rampTotalStates)
            {
                currentRampProfilePtr++; 
                currentRampProfile = *currentRampProfilePtr;
            }
            else
            {
                rampCurrentState = RAMP_STOP;
                stopShutter();
                rampStatusFlags.shutterOperationStart = 0; 
                rampStatusFlags.shutterOperationComplete = 1;
            }
        }
    }    
}

VOID calcShtrMinDistValue(VOID)
{
    //Read current position
    rampCurrentPosition = hallCounts;

    if(requiredDirection == CW)
    {
        shtrMinDistVal = rampCurrentPosition - uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shtrRevOperMinLimit_A110;
        //Limit the min travel distance to upper limit
        if(shtrMinDistVal < uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100)
        {
            shtrMinDistVal = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
        }
    }
    else
    {
        shtrMinDistVal = rampCurrentPosition + uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shtrRevOperMinLimit_A110;
        //Limit the min travel distance to lower limit
        if(shtrMinDistVal > uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101)
        {
            shtrMinDistVal = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
        }
    }
    shtrMinDistReqFlg = TRUE;    
}

VOID calcShtrStopDistValue(VOID)
{
    //Read current position
    rampCurrentPosition = hallCounts;
    
    if(requiredDirection == CW)
    {
        shtrMinDistVal = rampCurrentPosition - SHUTTER_STOP_DISTANCE_CW;
        //Limit the min travel distance to upper limit
        if(shtrMinDistVal < uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100)
        {
            shtrMinDistVal = uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100;
        }
    }
    else
    {
        shtrMinDistVal = rampCurrentPosition + SHUTTER_STOP_DISTANCE_CCW;
        //Limit the min travel distance to lower limit
        if(shtrMinDistVal > uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101)
        {
            shtrMinDistVal = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101;
        }
    }
    shtrMinDistReqFlg = TRUE;    
}


// the current 'stopShutter' function, in case 'shtrMinDistReqFlg' flag is set then it first reduced the speed the speed and then wait for shutter to move specific distance
// if while reducing the speed, if 'control output' becomes negative and motor moves in reverse direction before travelling the distance required by 'shtrMinDistReqFlg' flag
// then motor contineously keep on running in reverse direction
// The function is commneted and new function is developed which will first move the distance defined by the 'shtrMinDistReqFlg' flag and then speed will reduce - YG - NOV 15
#if 0
VOID stopShutter(VOID)
{
	// Logic related to increasing I gain while stoping the shutter is commneted - YG - Nov 15
	#if 0
    static SHORT I_gainForStop = 0;		//	current gain during stop operation
	static SHORT lsucCaptureI = 0;		//	variable to restore original current gain after stop
	#endif
	
    BOOL applyBrake = FALSE;  
    rampCurrentPosition = hallCounts;
    rampCurrentSpeed = refSpeed;   
    
    //if current speed is greater than 200 rpm then apply descelaration
    //then apply DC injection, then mechanical brake     
    if(
		(!gui8StopKeyPressed && (rampCurrentSpeed > SHUTTER_SPEED_MIN)) ||		//	Safety sensor is triggered
		(
			(gui8StopKeyPressed) &&		//	Stop key is pressed
			(
				(rampCurrentSpeed > SHUTTER_SPEED_MIN) || 
				( 
					((measuredSpeed > GO_UP_MIN_SPEED_BEFORE_APPLYING_BRAKE) && (requiredDirection == CW)) ||		//	measured speed is greater than 300 while going up 
					((measuredSpeed > GO_DOWN_MIN_SPEED_BEFORE_APPLYING_BRAKE) && (requiredDirection == CCW))		//	measured speed is greater than 450 while going down
				)
			) 
		)
	)
    {
		// Logic related to increasing I gain while stoping the shutter is commneted - YG - Nov 15
		#if 0
		if (lsucCaptureI == 0)
		{
			//	update current gain value for stop operation
			if(requiredDirection == CW)
			{
				lsucCaptureI = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.speed_PI_KI_A513 * 10;
			}
			else if(requiredDirection == CCW)
			{
				lsucCaptureI = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.current_PI_KI_A515 * 10;
			}
			I_gainForStop = lsucCaptureI;
		}
		#endif
		
        refiTotalCurrent = 0;
        rampStatusFlags.rampCurrentControlRequired = 0;        
        refSpeed -= decelaration;
        if(refSpeed < SHUTTER_SPEED_MIN)
        {
            refSpeed = SHUTTER_SPEED_MIN;
        }
        
        rampStatusFlags.rampSpeedControlRequired = 1;
		// Logic related to increasing I gain while stoping the shutter is commneted - YG - Nov 15
		#if 0
		if(lsucCaptureI)
		{
			//	dynamically increment i gain to speed up stop action
			I_gainForStop = I_gainForStop + 200;//20;
			if (I_gainForStop >= 20000)
			{
				I_gainForStop = 20000;
			}
			speedPIparms.qKi = I_gainForStop;
		}
		#endif
		
    }
    //Check if minimum travel before reverse is required
    else if(shtrMinDistReqFlg)
    {
		// Logic related to increasing I gain while stoping the shutter is commneted - YG - Nov 15
		#if 0
		//	restore current gain value
		I_gainForStop = lsucCaptureI;
		speedPIparms.qKi = I_gainForStop;
		lsucCaptureI = 0;	
		#endif
		
        if((requiredDirection == CW) && (rampCurrentPosition > shtrMinDistVal))
        {
            rampStatusFlags.rampSpeedControlRequired = 1;
        }
        else if((requiredDirection == CCW) && (rampCurrentPosition < shtrMinDistVal))
        {
            rampStatusFlags.rampSpeedControlRequired = 1;
        }
        else
        {
            applyBrake = TRUE;
            shtrMinDistReqFlg = FALSE;
        }
    }
    else
    {

		// Logic related to increasing I gain while stoping the shutter is commneted - YG - Nov 15
		#if 0
		//	restore current gain value		
		I_gainForStop = lsucCaptureI;
		speedPIparms.qKi = I_gainForStop;
		lsucCaptureI = 0;
		#endif
		
        applyBrake = TRUE;
    }
        
    if(applyBrake == TRUE)
    {
        rampStatusFlags.rampSpeedControlRequired = 0;
        rampStatusFlags.rampCurrentControlRequired = 0;
        
        if(!rampStatusFlags.rampDcInjectionOn)
        {
            controlOutput = SHUTTER_LOAD_HOLDING_DUTY;
            rampStatusFlags.rampDcInjectionOn = 1;
            DCInjectionON();            
            rampOutputStatus.shutterMoving = 0; //indicate shutter stopped
            rampDcInjectionOnCounter = 0;
            lockActivationDelayCnt = 0; //reset lock activation delay
            //calculate decrement value for dc injection
            dcInjDecVal = __builtin_divud(SHUTTER_LOAD_HOLDING_DUTY, (SHUTTER_LOAD_HOLDING_TIME_CNT - MECHANICAL_LOCK_ACTIVATION_DELAY_CNT));
        }
        else
        {
            if(++lockActivationDelayCnt >= MECHANICAL_LOCK_ACTIVATION_DELAY_CNT)
            {
                //Turn ON mechanical brake
                lockApply;
                rampStatusFlags.rampBrakeOn = 1;                
            }

            //If mechanical brake is ON decrement DC injection duty.
            if(rampStatusFlags.rampBrakeOn)
            {
                controlOutput -= dcInjDecVal;
                if(controlOutput < 0)
                    controlOutput = 0;
            }
            //Change the state after dc injection apply timer overflow
            if(++rampDcInjectionOnCounter >= SHUTTER_LOAD_HOLDING_TIME_CNT)
            {
                DCInjectionOFF();
                rampStatusFlags.rampDcInjectionOn = 0;
                //Update current state of ramp state machine to final state
                rampCurrentState = RAMP_STATE_END;
                currentRampProfileNo = RAMP_PROFILE_END;
                //Call stop motor to stop all the interrupt
                stopMotor();
				//	Shutter is stopped, clear flag
				gui8StopKeyPressed = 0;
            }
            
        }
        //check the trip status
        //if shutter has reached to upper or lower limit then immediately stop the shutter
        if((rampCurrentPosition < uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100) 
           || (rampCurrentPosition > uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101))
        {
			//	No need to call this function from here as it may override drift calculation logic
            //checkRampTripStatus();
        }
        
    }
}
#endif




// the old 'stopShutter' function, in case 'shtrMinDistReqFlg' flag is set then it first reduced the speed the speed and then wait for shutter to move specific distance
// if while reducing the speed, if 'control output' becomes negative and motor moves in reverse direction before travelling the distance required by 'shtrMinDistReqFlg' flag
// then motor contineously keep on running in reverse direction
// The new function is developed which will first move the distance defined by the 'shtrMinDistReqFlg' flag and then speed will reduce - YG - NOV 15
#if 1
VOID stopShutter(VOID)
{
	//  Logic related to increasing I gain while stoping the shutter is tuned to optimize value - YG - Nov 15
	#if 1
    static SHORT I_gainForStop = 0;		//	current gain during stop operation
	static SHORT lsucCaptureI = 0;		//	variable to restore original current gain after stop

	UINT16 lu16KiIncrementMaxValue;
	#endif
	
    BOOL applyBrake = FALSE;  
    rampCurrentPosition = hallCounts;
    rampCurrentSpeed = refSpeed; 


  	//Check if minimum travel before reverse is required
    if(shtrMinDistReqFlg)
    {
		// Logic related to increasing I gain while stoping the shutter is commneted - YG - Nov 15
		
        if((requiredDirection == CW) && (rampCurrentPosition > shtrMinDistVal))
        {
            rampStatusFlags.rampSpeedControlRequired = 1;
        }
        else if((requiredDirection == CCW) && (rampCurrentPosition < shtrMinDistVal))
        {
            rampStatusFlags.rampSpeedControlRequired = 1;
        }
        else
        {
            shtrMinDistReqFlg = FALSE;
        }
    }  
    //if current speed is greater than 200 rpm then apply descelaration
    //then apply DC injection, then mechanical brake     
    else if(
				(
					// Logic added for safety sensor and Up/ Down button press when shutter is moving, to go in respective apposite direction after achieving safe speed - YG NOV 15
					//	Safety sensor is triggered
				    (
						!gui8StopKeyPressed && 
						(
							(rampCurrentSpeed > SHUTTER_SPEED_MIN) ||
							(measuredSpeed > MIN_SPEED_BEFORE_REVERSE_ACTION)
						)
					) ||		
					//	Stop key is pressed
					(
						(gui8StopKeyPressed) &&		
						(
							(rampCurrentSpeed > SHUTTER_SPEED_MIN) || 
							( 
								((measuredSpeed > GO_UP_MIN_SPEED_BEFORE_APPLYING_BRAKE) && (requiredDirection == CW)) ||		//	measured speed is greater than 300 while going up 
								((measuredSpeed > GO_DOWN_MIN_SPEED_BEFORE_APPLYING_BRAKE) && (requiredDirection == CCW))		//	measured speed is greater than 450 while going down
							)
						) 
					)
				) &&
				// Check shutter current position with referance upper and lower limit and depending on the direction of movement
				(

					(requiredDirection == CW /* UP Direction*/    && rampCurrentPosition > uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100) ||
					(requiredDirection == CCW /* DOWN Direction*/ && rampCurrentPosition < uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101)

				)
          )
    {
		//  Logic related to increasing I gain while stoping the shutter is tuned to optimize value - YG - Nov 15
		#if 1
		if (lsucCaptureI == 0)
		{
			//	update current gain value for stop operation
			if(requiredDirection == CW)
			{
				lsucCaptureI = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.speed_PI_KI_A513 * 10;
			}
			else if(requiredDirection == CCW)
			{
				lsucCaptureI = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.current_PI_KI_A515 * 10;
			}
			I_gainForStop = lsucCaptureI;
		}
		#endif
		
        refiTotalCurrent = 0;
        rampStatusFlags.rampCurrentControlRequired = 0;        
        refSpeed -= gs16UpDecelaration;
        if(refSpeed < SHUTTER_SPEED_MIN)
        {
            refSpeed = SHUTTER_SPEED_MIN;
        }
        
        rampStatusFlags.rampSpeedControlRequired = 1;
		// Logic related to increasing I gain while stoping the shutter is tuned to optimize value - YG - Nov 15
		#if 1
		if(lsucCaptureI)
		{
		lu16KiIncrementMaxValue = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.maxStartupTimeLim_A504;

		if (lu16KiIncrementMaxValue == 0)
		{

		lu16KiIncrementMaxValue = 1000;

		}
		else
		{

		lu16KiIncrementMaxValue = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.maxStartupTimeLim_A504;

		}
			//	dynamically increment i gain to speed up stop action
			I_gainForStop = I_gainForStop + 200;
			if (I_gainForStop >= lu16KiIncrementMaxValue)
			{
				I_gainForStop = lu16KiIncrementMaxValue;
			}
			speedPIparms.qKi = I_gainForStop;
		}
		#endif
		
    }
    else
    {

		//  Logic related to increasing I gain while stoping the shutter is tuned to optimize value - YG - Nov 15
		#if 1
		//	restore current gain value		
		I_gainForStop = lsucCaptureI;
		speedPIparms.qKi = I_gainForStop;
		lsucCaptureI = 0;
		#endif
		
        applyBrake = TRUE;
    }
        
    if(applyBrake == TRUE)
    {
        rampStatusFlags.rampSpeedControlRequired = 0;
        rampStatusFlags.rampCurrentControlRequired = 0;
        
        if(!rampStatusFlags.rampDcInjectionOn)
        {
            controlOutput = SHUTTER_LOAD_HOLDING_DUTY;
            rampStatusFlags.rampDcInjectionOn = 1;
            DCInjectionON();            
            rampOutputStatus.shutterMoving = 0; //indicate shutter stopped
            rampDcInjectionOnCounter = 0;
            lockActivationDelayCnt = 0; //reset lock activation delay
            //calculate decrement value for dc injection
            dcInjDecVal = __builtin_divud(SHUTTER_LOAD_HOLDING_DUTY, (SHUTTER_LOAD_HOLDING_TIME_CNT - MECHANICAL_LOCK_ACTIVATION_DELAY_CNT));
        }
        else
        {
            if(++lockActivationDelayCnt >= MECHANICAL_LOCK_ACTIVATION_DELAY_CNT)
            {
                //Turn ON mechanical brake
                lockApply;
                rampStatusFlags.rampBrakeOn = 1;                
            }

            //If mechanical brake is ON decrement DC injection duty.
            if(rampStatusFlags.rampBrakeOn)
            {
                controlOutput -= dcInjDecVal;
                if(controlOutput < 0)
                    controlOutput = 0;
            }
            //Change the state after dc injection apply timer overflow
            if(++rampDcInjectionOnCounter >= SHUTTER_LOAD_HOLDING_TIME_CNT)
            {
                DCInjectionOFF();
                rampStatusFlags.rampDcInjectionOn = 0;
                //Update current state of ramp state machine to final state
                rampCurrentState = RAMP_STATE_END;
                currentRampProfileNo = RAMP_PROFILE_END;
                //Call stop motor to stop all the interrupt
                stopMotor();
				//	Shutter is stopped, clear flag
				gui8StopKeyPressed = 0;
            }
            
        }
        //check the trip status
        //if shutter has reached to upper or lower limit then immediately stop the shutter
        if((rampCurrentPosition < uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100) 
           || (rampCurrentPosition > uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101))
        {
			//	No need to call this function from here as it may override drift calculation logic
            //checkRampTripStatus();
        }
        
    }
}
#endif



VOID executeRampProfile(VOID)
{    
    rampCurrentPosition = hallCounts;
    rampCurrentSpeed = refSpeed;
    rampCurrentTotCurr = refiTotalCurrent;
    rampCurrentOpenloopDuty = controlOutput;

    if(currentRampProfile.rampGenFlags.ccwDirection)
    {           
        requiredDirection = CCW;
        
        //if mechanical release required then
        if(currentRampProfile.rampGenFlags.brakeRelease && rampStatusFlags.rampBrakeOn)
        {
            //before releasing mechanical brake apply DC injection to hold the shutter load
            if(!rampStatusFlags.rampDcInjectionOn)
            {
                controlOutput = currentRampProfile.dcInjectionDuty;
                rampStatusFlags.rampDcInjectionOn = 1;
                DCInjectionON();                
                rampDcInjectionOnCounter = 0;
                lockDeactivationDelayCnt = 0;
            }
            else
            {                
                if(++lockDeactivationDelayCnt >= MECHANICAL_LOCK_DEACTIVATION_DELAY_CNT)
                {
                    lockRelease;
                    rampStatusFlags.rampBrakeOn = 0; 
                }                
               
                //turn off DC injection after required time
                if(++rampDcInjectionOnCounter >= currentRampProfile.dcInjectionTime)
                {
                    rampStatusFlags.rampDcInjectionOn = 0; 
                    if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == BEAD_SHUTTER)
                        rampStatusFlags.rampMaintainHoldingDuty = 0;
                    else
                        rampStatusFlags.rampMaintainHoldingDuty = 1;
                    refSpeed = SHUTTER_SPEED_MIN;
                    refiTotalCurrent = RAMP_STARTING_CURRENT_MIN;
                    #if (STARTUP_IN_CURRENT_MODE == 1)
                    if(currentRampProfile.rampGenFlags.currentMode)
                        rampStatusFlags.rampCurrentControlRequired = 1;
                    else
                        rampStatusFlags.rampSpeedControlRequired = 1;
                    #else
                    rampStatusFlags.rampSpeedControlRequired = 1;
                    #endif
                    startMotor();   //After removing brake start motor
                    rampOutputStatus.shutterMoving = 1;                    
                }
            }            
        } 
        //if only DC injection is ON then first turn OFF DC injection
        else if(currentRampProfile.rampGenFlags.brakeRelease && rampStatusFlags.rampDcInjectionOn)
        {
            rampStatusFlags.rampDcInjectionOn = 0; 
            if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == BEAD_SHUTTER)
                rampStatusFlags.rampMaintainHoldingDuty = 0;
            else
                rampStatusFlags.rampMaintainHoldingDuty = 1;
            refSpeed = SHUTTER_SPEED_MIN;
            refiTotalCurrent = RAMP_STARTING_CURRENT_MIN;
            #if (STARTUP_IN_CURRENT_MODE == 1)
            if(currentRampProfile.rampGenFlags.currentMode)
                rampStatusFlags.rampCurrentControlRequired = 1;
            else
                rampStatusFlags.rampSpeedControlRequired = 1;
            #else
            rampStatusFlags.rampSpeedControlRequired = 1;
            #endif
            startMotor();   //After removing brake start motor
            rampOutputStatus.shutterMoving = 1;
        }
        else if(currentRampProfile.rampGenFlags.speedMode)
        {
#if 0
			SHORT lshSpeedChangeRate;
#endif
            //if current position is within the required limit then execute the step else goto next step
            if(rampCurrentPosition < currentRampProfile.endPosition)
            {
                if(currentRampProfile.startSpeed > currentRampProfile.endSpeed)
                {
                    //descelaration
                    if(rampCurrentSpeed > currentRampProfile.endSpeed)
                    {
#if 0
    					lshSpeedChangeRate = DECELARATION_RATE(currentRampProfile.startSpeed, currentRampProfile.endSpeed,uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.decel1Up_A521);
#endif
                        //Set the current refernce
                        refiTotalCurrent = currentRampProfile.endCurrent;
                        //Disable current loop
                        rampStatusFlags.rampCurrentControlRequired = 0;
                        //set the speed reference
                        refSpeed -= currentRampProfile.speedChangeRate;
#if 0
						refSpeed -= lshSpeedChangeRate;
#endif

                        //enable speed control
                        //Limit refspeed
                        if(refSpeed < currentRampProfile.endSpeed)
                        {
                            refSpeed = currentRampProfile.endSpeed;
                        }                        
                        rampStatusFlags.rampSpeedControlRequired = 1;
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.speedChangeRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                        else
                        {
                            //Set the current refernce
                            refiTotalCurrent = currentRampProfile.endCurrent;
                            //Disable current loop
                            rampStatusFlags.rampCurrentControlRequired = 0;
                            //set the speed reference
                            refSpeed = currentRampProfile.endSpeed;
                            //enable speed control
                            rampStatusFlags.rampSpeedControlRequired = 1;
                        }
                    }
                }
                else  //  else of if(currentRampProfile.startSpeed > currentRampProfile.endSpeed)
                {
                    if(rampCurrentSpeed < currentRampProfile.endSpeed)
                    {
#if 0
						lshSpeedChangeRate = ACCELARATION_RATE(currentRampProfile.startSpeed, currentRampProfile.endSpeed, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.accel1Up_A520);
#endif
                        //Set the current refernce
                        refiTotalCurrent = currentRampProfile.endCurrent;
                        //Disable current loop
                        rampStatusFlags.rampCurrentControlRequired = 0;                        
                        //set the speed reference
                        
						refSpeed += currentRampProfile.speedChangeRate;
#if 0
						refSpeed += lshSpeedChangeRate;
#endif
						
                        //Limit refspeed
                        if(refSpeed > currentRampProfile.endSpeed)
                        {
                            refSpeed = currentRampProfile.endSpeed;
                        }                        
                        //enable speed control
                        rampStatusFlags.rampSpeedControlRequired = 1;
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.speedChangeRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                        else
                        {
                            //Set the current refernce
                            refiTotalCurrent = currentRampProfile.endCurrent;
                            //Disable current loop
                            rampStatusFlags.rampCurrentControlRequired = 0;
                            //set the speed reference
                            refSpeed = currentRampProfile.endSpeed;
                            //enable speed control
                            rampStatusFlags.rampSpeedControlRequired = 1;
                        }
                    }
                }
            }
			// Added to overcome installation issue (A100) - RN- NOV 2015
            else if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
            {
                //Goto next step
                if(++rampCurrentStep < rampTotalStates)
                {
                    currentRampProfilePtr++; 
                    currentRampProfile = *currentRampProfilePtr;
                }
                else
                {
                    brakingRequired();
                }
            }
        }        
        //If it is current mode
        else if(currentRampProfile.rampGenFlags.currentMode)
        {
            //if current position is within the required limit then execute the step else goto next step
            if(rampCurrentPosition < currentRampProfile.endPosition)
            {
                if(currentRampProfile.startCurrent > currentRampProfile.endCurrent)
                {
                    //descelaration
                    if(rampCurrentTotCurr > currentRampProfile.endCurrent)
                    {
                        //Before increasing current check if system already achived required speed
                        if(measuredSpeed > currentRampProfile.endSpeed)
                        {
                            //set the speed reference
                            refSpeed = currentRampProfile.endSpeed;
                            //disable speed control
                            rampStatusFlags.rampSpeedControlRequired = 0;                            
                            //set the current reference
                            refiTotalCurrent -= currentRampProfile.currentChangeRate;                            
                            //Limit refcurrent
                            if(refiTotalCurrent < currentRampProfile.endCurrent)
                            {
                                refiTotalCurrent = currentRampProfile.endCurrent;
                            }                              
                            //Enable current control
                            rampStatusFlags.rampCurrentControlRequired = 1;
                        }
                        else
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.currentChangeRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                        else
                        {
                            //set the current reference
                            refiTotalCurrent = currentRampProfile.endCurrent;
                            //Enable current control
                            rampStatusFlags.rampCurrentControlRequired = 1;
                        }
                    }
                }
                else
                {
                    //if((rampCurrentTotCurr >= currentRampProfile.startCurrent) && (rampCurrentTotCurr < currentRampProfile.endCurrent))
                    if(rampCurrentTotCurr < currentRampProfile.endCurrent)
                    {
                        //Before increasing current check if system already achived required speed
                        //if(rampCurrentSpeed < currentRampProfile.endSpeed)
                        if(measuredSpeed < currentRampProfile.endSpeed)
                        {
                            //set the speed reference
                            refSpeed = currentRampProfile.endSpeed;
                            //disable speed control
                            rampStatusFlags.rampSpeedControlRequired = 0;                            
                            //set the current reference
                            refiTotalCurrent += currentRampProfile.currentChangeRate;                            
                            //Limit refcurrent
                            if(refiTotalCurrent > currentRampProfile.endCurrent)
                            {
                                refiTotalCurrent = currentRampProfile.endCurrent;
                            }                              
                            //Enable current control
                            rampStatusFlags.rampCurrentControlRequired = 1;
                        }
                        else
                        {
                            //goto to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.currentChangeRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                        else
                        {
                            //set the current reference
                            refiTotalCurrent = currentRampProfile.endCurrent;
                            //Enable current control
                            rampStatusFlags.rampCurrentControlRequired = 1;
                        }
                    }
                }
            }
            else
            {
                //Goto next step
                if(++rampCurrentStep < rampTotalStates)
                {
                    currentRampProfilePtr++; 
                    currentRampProfile = *currentRampProfilePtr;
                }
                else
                {
                    brakingRequired();
                }
            }
        }
        //if it is open loop mode 
        else if(currentRampProfile.rampGenFlags.openloopMode)
        {
            //if current position is within the required limit then execute the step else goto next step
            if(rampCurrentPosition < currentRampProfile.endPosition)
            {
                if(currentRampProfile.startOpenloop > currentRampProfile.endOpenloop)
                {
                    //descelaration
                    if((rampCurrentOpenloopDuty <= currentRampProfile.startOpenloop) && (rampCurrentOpenloopDuty > currentRampProfile.endOpenloop))
                    {
                        //set the speed reference
                        controlOutput -= currentRampProfile.openLoopRate;
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.openLoopRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                    }
                }
                else
                {
                    if((rampCurrentOpenloopDuty >= currentRampProfile.startOpenloop) && (rampCurrentOpenloopDuty < currentRampProfile.endOpenloop))
                    {
                        //set the speed reference
                        controlOutput += currentRampProfile.openLoopRate;
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.openLoopRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                    }
                }
            }
            else
            {
                //Goto next step
                if(++rampCurrentStep < rampTotalStates)
                {
                    currentRampProfilePtr++; 
                    currentRampProfile = *currentRampProfilePtr;
                }
                else
                {
                    brakingRequired();
                }
            }
        }
    }
    else if(currentRampProfile.rampGenFlags.cwDirection) 
    {		
        requiredDirection = CW;
            
        //if mechanical release required then
        if(currentRampProfile.rampGenFlags.brakeRelease && rampStatusFlags.rampBrakeOn)
        {
            //before releasing mechanical brake apply DC injection to hold the shutter load
            if(!rampStatusFlags.rampDcInjectionOn)
            {
                controlOutput = currentRampProfile.dcInjectionDuty;
                rampStatusFlags.rampDcInjectionOn = 1;
                DCInjectionON();                
                rampDcInjectionOnCounter = 0;
                lockDeactivationDelayCnt = 0;   //reset lock de-activation delay
            }
            else
            {               
                if(++lockDeactivationDelayCnt >= MECHANICAL_LOCK_DEACTIVATION_DELAY_CNT)
                {
                    lockRelease;
                    rampStatusFlags.rampBrakeOn = 0; 
                }
                
                //turn off DC injection after required time
                if(++rampDcInjectionOnCounter >= currentRampProfile.dcInjectionTime)
                {
                    rampStatusFlags.rampDcInjectionOn = 0;  
                    rampStatusFlags.rampMaintainHoldingDuty = 1;
                    refSpeed = SHUTTER_SPEED_MIN;
                    refiTotalCurrent = RAMP_STARTING_CURRENT_MIN;
                    #if (STARTUP_IN_CURRENT_MODE == 1)
                    if(currentRampProfile.rampGenFlags.currentMode)
                        rampStatusFlags.rampCurrentControlRequired = 1;
                    else
                        rampStatusFlags.rampSpeedControlRequired = 1;
                    #else
                    rampStatusFlags.rampSpeedControlRequired = 1;
                    #endif
                    startMotor();   //After removing brake start motor
                    rampOutputStatus.shutterMoving = 1;
                }
            }            
        }  
        //if only DC injection is ON then first turn OFF DC injection
        else if(currentRampProfile.rampGenFlags.brakeRelease && rampStatusFlags.rampDcInjectionOn)
        {
            rampStatusFlags.rampDcInjectionOn = 0;  
            rampStatusFlags.rampMaintainHoldingDuty = 1;
            refSpeed = SHUTTER_SPEED_MIN;
            refiTotalCurrent = RAMP_STARTING_CURRENT_MIN;
            #if (STARTUP_IN_CURRENT_MODE == 1)
            if(currentRampProfile.rampGenFlags.currentMode)
                rampStatusFlags.rampCurrentControlRequired = 1;
            else
                rampStatusFlags.rampSpeedControlRequired = 1;
            #else
            rampStatusFlags.rampSpeedControlRequired = 1;
            #endif
            startMotor();       //After removing brake start motor
            rampOutputStatus.shutterMoving = 1;
        }
        else if(currentRampProfile.rampGenFlags.speedMode)
        {
#if 0
			SHORT lshSpeedChangeRate;
#endif

            //if current position is within the required limit then execute the step else goto next step
            if(rampCurrentPosition >= currentRampProfile.endPosition)
            {
                if(currentRampProfile.startSpeed > currentRampProfile.endSpeed)                
                {
                    //descelaration
                    if(rampCurrentSpeed > currentRampProfile.endSpeed)
                    {
#if 0
						lshSpeedChangeRate = DECELARATION_RATE(currentRampProfile.startSpeed,currentRampProfile.endSpeed,uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.decel1Up_A521);
#endif

                        //Set the current refernce
                        refiTotalCurrent = currentRampProfile.endCurrent;
                        //Disable current loop
                        rampStatusFlags.rampCurrentControlRequired = 0;                        
                        //set the speed reference
                        
						refSpeed -= currentRampProfile.speedChangeRate;
#if 0
						refSpeed -= lshSpeedChangeRate;
#endif

                        //Limit refspeed
                        if(refSpeed < currentRampProfile.endSpeed)
                        {
                            refSpeed = currentRampProfile.endSpeed;
                        }                        
                        //enable speed control
                        rampStatusFlags.rampSpeedControlRequired = 1;
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.speedChangeRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                        else
                        {
                            //Set the current refernce
                            refiTotalCurrent = currentRampProfile.endCurrent;
                            //Disable current loop
                            rampStatusFlags.rampCurrentControlRequired = 0;
                            //set the speed reference
                            refSpeed = currentRampProfile.endSpeed;
                            //enable speed control
                            rampStatusFlags.rampSpeedControlRequired = 1;
                        }
                    }
                }
                else  // else of if(currentRampProfile.startSpeed > currentRampProfile.endSpeed)               
                {
                    if(rampCurrentSpeed < currentRampProfile.endSpeed)
                    {
#if 0
						lshSpeedChangeRate = ACCELARATION_RATE(currentRampProfile.startSpeed, currentRampProfile.endSpeed, uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.accel1Up_A520);
#endif
                        //Set the current refernce
                        refiTotalCurrent = currentRampProfile.endCurrent;
                        //Disable current loop
                        rampStatusFlags.rampCurrentControlRequired = 0;                        
                        //set the speed reference
                        
						refSpeed += currentRampProfile.speedChangeRate;
#if 0
						refSpeed += lshSpeedChangeRate;
#endif

                        //Limit refspeed
                        if(refSpeed > currentRampProfile.endSpeed)
                        {
                            refSpeed = currentRampProfile.endSpeed;
                        }                        
                        //enable speed control
                        rampStatusFlags.rampSpeedControlRequired = 1;
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.speedChangeRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                        else
                        {
                            //Set the current refernce
                            refiTotalCurrent = currentRampProfile.endCurrent;
                            //Disable current loop
                            rampStatusFlags.rampCurrentControlRequired = 0;
                            //set the speed reference
                            refSpeed = currentRampProfile.endSpeed;
                            //enable speed control
                            rampStatusFlags.rampSpeedControlRequired = 1;
                        }
                    }
                }
            }
			// Added to overcome installation issue (A100) - RN- NOV 2015
            else if(
					(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation) ||
					(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationValidation)
				)
            {
                //Goto next step
                if(++rampCurrentStep < rampTotalStates)
                {
                    currentRampProfilePtr++; 
                    currentRampProfile = *currentRampProfilePtr;
                }
                else
                {
                    brakingRequired();
                }
            }
        }        
        //If it is current mode
        else if(currentRampProfile.rampGenFlags.currentMode)
        {
            //if current position is within the required limit then execute the step else goto next step
            if(rampCurrentPosition >= currentRampProfile.endPosition)
            {
                if(currentRampProfile.startCurrent > currentRampProfile.endCurrent)
                {
                    //descelaration
                    if(rampCurrentTotCurr > currentRampProfile.endCurrent)
                    {
                        //Before increasing current check if system already achived required speed
                        if(measuredSpeed > currentRampProfile.endSpeed)
                        {
                            //set the speed reference
                            refSpeed = currentRampProfile.endSpeed;
                            //disable speed control
                            rampStatusFlags.rampSpeedControlRequired = 0;                            
                            //set the current reference
                            refiTotalCurrent -= currentRampProfile.currentChangeRate;                            
                            //Limit refcurrent
                            if(refiTotalCurrent < currentRampProfile.endCurrent)
                            {
                                refiTotalCurrent = currentRampProfile.endCurrent;
                            } 
                            //Enable current control
                            rampStatusFlags.rampCurrentControlRequired = 1;
                        }
                        else
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.currentChangeRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                        else
                        {
                            //set the current reference
                            refiTotalCurrent = currentRampProfile.endCurrent;
                            //Enable current control
                            rampStatusFlags.rampCurrentControlRequired = 1;
                        }
                    }
                }
                else
                {
                    if(rampCurrentTotCurr < currentRampProfile.endCurrent)
                    {
                        //Before increasing current check if system already achived required speed
                        if(measuredSpeed < currentRampProfile.endSpeed)
                        {
                            //set the speed reference
                            refSpeed = currentRampProfile.endSpeed;
                            //disable speed control
                            rampStatusFlags.rampSpeedControlRequired = 0;                            
                            //set the current reference
                            refiTotalCurrent += currentRampProfile.currentChangeRate;                            
                            //Limit refcurrent
                            if(refiTotalCurrent > currentRampProfile.endCurrent)
                            {
                                refiTotalCurrent = currentRampProfile.endCurrent;
                            } 
                            //Enable current control
                            rampStatusFlags.rampCurrentControlRequired = 1;
                        }
                        else
                        {
                            //got0 to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.currentChangeRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                        else
                        {
                            //set the current reference
                            refiTotalCurrent = currentRampProfile.endCurrent;
                            //Enable current control
                            rampStatusFlags.rampCurrentControlRequired = 1;
                        }
                    }
                }
            }
            else
            {
                //Goto next step
                if(++rampCurrentStep < rampTotalStates)
                {
                    currentRampProfilePtr++; 
                    currentRampProfile = *currentRampProfilePtr;
                }
                else
                {
                    brakingRequired();
                }
            }
        }
        //if it is open loop mode 
        else if(currentRampProfile.rampGenFlags.openloopMode)
        {
            //if current position is within the required limit then execute the step else goto next step
            if(rampCurrentPosition >= currentRampProfile.endPosition)
            {
                if(currentRampProfile.startOpenloop > currentRampProfile.endOpenloop)
                {
                    //descelaration
                    if((rampCurrentOpenloopDuty <= currentRampProfile.startOpenloop) && (rampCurrentOpenloopDuty > currentRampProfile.endOpenloop))
                    {
                        //set the speed reference
                        controlOutput -= currentRampProfile.openLoopRate;
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.openLoopRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                    }
                }
                else
                {
                    if((rampCurrentOpenloopDuty >= currentRampProfile.startOpenloop) && (rampCurrentOpenloopDuty < currentRampProfile.endOpenloop))
                    {
                        //set the speed reference
                        controlOutput += currentRampProfile.openLoopRate;
                    }
                    else
                    {
                        //if DC injection is required at the end of state then apply DC injection
                        if(currentRampProfile.rampGenFlags.dcInjectionApply)
                        {
                            brakingRequired();
                        }
                        //if we are in constant speed state then speed change rate will be zero
                        //in that case state change will be controlled by target position
                        else if(currentRampProfile.openLoopRate != 0)
                        {
                            //got to next step
                            if(++rampCurrentStep < rampTotalStates)
                            {
                                currentRampProfilePtr++;
                                currentRampProfile = *currentRampProfilePtr;
                            }
                            else
                            {
                                brakingRequired();
                            }
                        }
                    }
                }
            }
            else
            {
                //Goto next step
                if(++rampCurrentStep < rampTotalStates)
                {
                    currentRampProfilePtr++; 
                    currentRampProfile = *currentRampProfilePtr;
                }
                else
                {
                    brakingRequired();
                }
            }
        }

    }
    
}


VOID brakingRequired(VOID)
{
    BOOL applyBrake = FALSE;
    
    rampCurrentSpeed = refSpeed;
    rampCurrentPosition = hallCounts;
    
    if(requiredDirection == CW)
    {
        if(rampCurrentPosition <= currentRampProfile.endPosition)
        {
            applyBrake = TRUE;
        }
    }
    else
    {
         if(rampCurrentPosition >= currentRampProfile.endPosition)
         {
             applyBrake = TRUE;
         }
		 //	Commented to handle "offset at upper & lower limit"
		 //	Shutter used to stop above lower limit because of this.
#if 0         
         if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)
         {
             applyBrake = TRUE;
         }
#endif
    }

    if(applyBrake)
    {
        rampStatusFlags.rampSpeedControlRequired = 0;
        rampStatusFlags.rampCurrentControlRequired = 0;
        
        if(!rampStatusFlags.rampDcInjectionOn)
        {
            controlOutput = currentRampProfile.dcInjectionDuty;//comment
            rampStatusFlags.rampDcInjectionOn = 1;
            DCInjectionON();            //comment
            rampOutputStatus.shutterMoving = 0; //indicate shutter stopped
            rampDcInjectionOnCounter = 0;
            lockActivationDelayCnt = 0; //reset lock activation delay
            rampStatusFlags.shutterOperationStart = 0; 
            rampStatusFlags.shutterOperationComplete = 1;
            //calculate decrement value for dc injection
            dcInjDecVal = __builtin_divud(currentRampProfile.dcInjectionDuty, (currentRampProfile.dcInjectionTime - MECHANICAL_LOCK_ACTIVATION_DELAY_CNT));//comment
        }
        else
        {
            if(++lockActivationDelayCnt >= MECHANICAL_LOCK_ACTIVATION_DELAY_CNT)
            {
                //Turn ON mechanical brake
                lockApply;
                rampStatusFlags.rampBrakeOn = 1;
            }
            
            //If mechanical brake is ON decrement DC injection duty.
            if(rampStatusFlags.rampBrakeOn)
            {
                controlOutput -= dcInjDecVal;
                if(controlOutput < 0)
                    controlOutput = 0;
            }
            
            //Change the state after dc injection apply timer overflow
            if(++rampDcInjectionOnCounter >= currentRampProfile.dcInjectionTime)
            {
                DCInjectionOFF();
                rampStatusFlags.rampDcInjectionOn = 0;
                stopMotor();
                
                if(++rampCurrentStep < rampTotalStates)
                {
                    currentRampProfilePtr++;
                    currentRampProfile = *currentRampProfilePtr;
                }
                else
                {
                    rampCurrentState = RAMP_STATE_END;
                    currentRampProfileNo = RAMP_PROFILE_END;  
                }
            }
        }
        //check the trip status
        checkRampTripStatus();
    }
}

VOID runTestCode(VOID)
{
            if(readOriginSensorSts() == 0)
            {
                flags.StartStop = 1;
            }
            else
            {
                if(refSpeed <= 350)
                    flags.StartStop = 0;
            } 
            
            if ((flags.StartStop == 1) && (!flags.motorRunning))//&& (!sustainedOcFlg))
            {
                //startMotor();	
                //PORTDbits.RD6 = 1;
                //flags.motorRunning = 1;
                lockRelease;
                delayMs(100); 
                startMotor();
            }
            else if ((flags.StartStop == 0) && (flags.motorRunning))// && (!sustainedOcFlg))
            {
                stopMotor();
                //PORTDbits.RD6 = 0;
                //flags.motorRunning = 0;
                delayMs(100);
                lockApply;
            }

}