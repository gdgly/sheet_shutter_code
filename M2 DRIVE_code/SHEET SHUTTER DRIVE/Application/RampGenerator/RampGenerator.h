/********************************************************************************
* FileName: RampGenerator.h
* Description:
* This header file contains the decleration of all the attributes and
* services for RampGenerator.c file. It implements ramp generator for
* speed and current mode of operation
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
#ifndef RAMP_GENERATOR_H
#define RAMP_GENERATOR_H
#include "./Common/Typedefs/Typedefs.h"
#include "./Common/UserDefinition/Userdef.h"

#define SHUTTER_2M_2M   1
#define SHUTTER_4M_4M   2

#define SHUTTER_TYPE    SHUTTER_2M_2M

#define STARTUP_IN_CURRENT_MODE     0

#define SHUTTER_STOPPED 0
#define SHUTTER_MOVING_UP 1
#define SHUTTER_MOVING_DOWN 2

#define RAMP_GENERATOR_TIME_PERIOD      10//50 20160915

#define MOTOR_RATED_TOP_SPEED   2900
#define MOTOR_ZERO_SPEED        0

#if (SHUTTER_TYPE == SHUTTER_2M_2M)
    //#define SHUTTER_LOAD_HOLDING_DUTY        2000
    //#define SHUTTER_LOAD_HOLDING_DUTY        1557   //5%
    #define SHUTTER_LOAD_HOLDING_DUTY        2200   //7%
    //#define SHUTTER_LOAD_HOLDING_DUTY        3200   //10%
    //#define SHUTTER_LOAD_HOLDING_DUTY        4700   //15%
    //#define SHUTTER_LOAD_HOLDING_DUTY        6400   //20%
#elif (SHUTTER_TYPE == SHUTTER_4M_4M)
    #define SHUTTER_LOAD_HOLDING_DUTY        3000
#else
    #error Shutter type not defined
#endif
#define HOLDING_DUTY_INC    40//10//20//200 20160915

#define SHUTTER_SPEED_MIN       500//500//150
#define SHUTTER_SPEED_MIN_STOP  200
#define SHUTTER_SPEED_MAX       3600

// Following variales are used by the 'stopShutter' to stop shutter or move shutter in reverse direction after achieving following mentioned safe speed YG NOV 15
#define GO_UP_MIN_SPEED_BEFORE_APPLYING_BRAKE	300
#define GO_DOWN_MIN_SPEED_BEFORE_APPLYING_BRAKE	300
#define MIN_SPEED_BEFORE_REVERSE_ACTION			300

#define ACCELARATION_TIME       500//2000//500
#define DECELARATION_TIME       600

#define ACC_STEP    (((DWORD)(MOTOR_RATED_TOP_SPEED-MOTOR_ZERO_SPEED)*RAMP_GENERATOR_TIME_PERIOD)/ACCELARATION_TIME)
#define DEC_STEP    (((DWORD)(MOTOR_RATED_TOP_SPEED-MOTOR_ZERO_SPEED)*RAMP_GENERATOR_TIME_PERIOD)/DECELARATION_TIME)

#define RAMP_START_SPEED        0

#define TWO_STEP_RAMP_PROFILE   2
#define NO_SPEED_CHANGE         0

#define SHUTTER_STOP_DISTANCE_CW   50
#define SHUTTER_STOP_DISTANCE_CCW   75

#define lockApply  (PORTCbits.RC8 = 0)
#define lockRelease (PORTCbits.RC8 = 1)

#define enablePWMBuffer     (PORTGbits.RG9 = 0)
#define disablePWMBuffer    (PORTGbits.RG9 = 1)

#define fanON               (PORTCbits.RC13 = 1)
#define fanOFF              (PORTCbits.RC13 = 0)

#if (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER1)
#define emergencySensorSts          (!(PORTAbits.RA1)) //normally open
#define microSwSensorSts            (!(PORTBbits.RB1))
#define AirSwSensorSts              (!(PORTBbits.RB1))
#define wrapSwSensorSts             (!(PORTBbits.RB1))
#define photoElecSensorSts          (!(PORTAbits.RA0)) //normally closed
#define temperatureSensorSts        (!(PORTEbits.RE15))
#define originSensorSts             (PORTDbits.RD8)
#define fourPtSensorSts             (!(PORTCbits.RC10))
#define powerFailSensorSts          (!(PORTBbits.RB7))
#elif (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER2)
#define emergencySensorSts          (!(PORTGbits.RG6))
#define microSwSensorSts            (!(PORTAbits.RA11))
#define AirSwSensorSts              (!(PORTAbits.RA11))
#define wrapSwSensorSts             (!(PORTAbits.RA11))
#define photoElecSensorSts          (!(PORTAbits.RA12))
#define temperatureSensorSts        (!(PORTEbits.RE15))
#define originSensorSts             (PORTDbits.RD8)
#define fourPtSensorSts             (!(PORTCbits.RC10))
#define powerFailSensorSts          (!(PORTBbits.RB7))
#else
#error Motor control hardware type not defined in RampGenerator.h
#endif

//	Macro to select programmable debounce logic.
//	When this macro is enabled programmable debounce is enabled, by which we can set sensor active
//	and sensor inactive debounce by changing the macro value for each sensor.
#define PROGRAMMABLE_DEBOUNCE
#define SAFETY_SENSOR_DEBOUNCE_CNT      5

//PWM coasting time in ms
#define PWM_COASTING_TIME   200      //50ms

/* Enumaration for ramp states */
typedef enum stateNo
{
    STATE_ZERO,
    STATE_ONE,
    STATE_TWO,
    STATE_THREE,
    STATE_FOUR,
    STATE_FIVE,
    STATE_SIX,
    STATE_SEVEN,
    STATE_END
}stateNo_en;

typedef enum rampState
{
    RAMP_START,
    RAMP_RESTART,
    RAMP_RUNNING,
    RAMP_STOP,
    RAMP_PWM_COASTING,
    RAMP_STATE_END
}rampState_en;

//List of safety sensors
typedef enum safetySensors
{
    EMERGENCY_SENSOR,
    MICRO_SW_SENSOR,
    //AIR_SW_SENSOR,
    //WRAP_SW_SENSOR,
    PHOTOELECTRIC_SENSOR,
    TEMPERATURE_SENSOR,
    ORIGIN_SENSOR,
    FOUR_PT_SW_SENSOR,
    POWER_FAIL_SW_SENSOR,
    SAFETY_SENSOR_END
}safetySensors_en;

/* flags used for the application */
typedef struct StatusFlags
{
	unsigned motorRunning	:1;  /* This bit is 1 if motor running */
	unsigned speedOpenLoop		:1;  /* This bit is 1 if motor is running in open loop */
    unsigned StartStop      :1;  /* Start/Stop command for motor from DMCI */
    unsigned speedControl   :1;  /* Set to operate in speed mode */
    unsigned currentControl :1;  /* Set to operate in current mode */
    unsigned currOpenLoop	:1;  /* This bit is 1 if motor is running in open loop */
    unsigned exstFanOn      :1;   //indicates fan ON status
	unsigned unused			:9;
}StatusFlags_t;

typedef struct rampFlags
{
	unsigned speedMode			:1; 	/* This bit is 1 if motor is running is speed mode */
	unsigned currentMode		:1; 	/* This bit is 1 if motor is running is Current mode */
	unsigned openloopMode		:1; 	/* This is bit is 1 for open loop operation */
	unsigned startStopMotor		:1; 	/* This bit is to monitor motor ON-Off*/
	unsigned dcInjectionApply	:1;		/* Set to apply DC injection */
	unsigned dcInjectionRelease	:1;		/* Set to release DC injection */
	unsigned brakeApply			:1; 	/* Set to apply Mechanical brake*/
	unsigned brakeRelease		:1; 	/* Set to release Mechanical Brake*/
	unsigned cwDirection		:1; 	/* Set to roate in clockwise direction */
	unsigned ccwDirection		:1;		/* Set to roate in counter-clockwise direction */
	unsigned unused				:6;
}rampFlags_t;

typedef struct rampStructure
{
	rampFlags_t rampGenFlags;
	SHORT startPosition;	/* Start position of Ramp profile*/
	SHORT endPosition;		/* End potion of running state*/
	SHORT startSpeed;		/* start speed of the state*/
	SHORT endSpeed;			/* End spped of the State*/
	SHORT speedChangeRate;	/* Speed change rate to achieve given acc or decc*/
	SHORT startCurrent;		/* Start Current of the State in current mode*/
	SHORT endCurrent;		/* End Current of the State in current mode*/
	SHORT currentChangeRate;/* Current change rate to achieve given Acc or decc*/
	SHORT startOpenloop;	/* Set starting open loop duty*/
	SHORT endOpenloop;		/* Set open loop duty to achieve*/
	SHORT openLoopRate;		/* Set open loop duty rate */
	SHORT dcInjectionDuty;	/* Set duty for PDC register for DC injection Holding*/
	SHORT dcInjectionTime;	/* Set duration for DC injection Holding*/
}rampStructure_t;

typedef struct _rampOutputStatus
{
    UINT8 shutterMovementDirection; //Indicates shutter movement direction 0 = CW, 1 = CCW
    UINT8 shutterMoving;              //Indicates shutter is moving or stopped
    INT16 shutterCurrentPosition;   //Indicates current shutter position in Hall counts unit
}rampOutputStatus_t;


/* flags used as the interface between Logic Solver and Ramp Generator blocks */
typedef union _InputFlags
{
	unsigned char value;
	struct{
	unsigned char shutterOpen	:1;	/* TRUE indicates Open command from Control board */
	unsigned char shutterClose	:1;	/* TRUE indicates Close command from Control board */
	unsigned char shutterStop	:1;	/* TRUE indicates Stop command from Control board */
    unsigned char jogPercentage :2; /*  0 - No Jog, use normal speed profile
                                                1 - 10% Jog
                                                2 - 50% Jog */
	unsigned char aperture		:1;	/* TRUE indicates Aperture active - open/ close only upto Aperture read from EEPROM */
	}bits;
}InputFlags_u;

typedef struct _rampStatusFlags
{
	unsigned rampGenRunning	:1;
    unsigned rampDcInjectionOn :1;
    unsigned rampBrakeOn :1;
    unsigned shutterOperationStart :1;
    unsigned shutterOperationComplete :1;
    unsigned rampSpeedControlRequired:1;
    unsigned rampCurrentControlRequired:1;
    unsigned rampLockRelDmci:1;
    unsigned rampLockAppDmci:1;
    unsigned safetySensorTriggered:1;
    unsigned rampOpenInProgress:1;
    unsigned rampCloseInProgress:1;
    unsigned rampMaintainHoldingDuty:1;
    unsigned rampDriftCalculated:1;
    unsigned mechLockRelFlag:1;
	unsigned saveParamToEeprom:1;
}rampStatusFlags_t;

typedef struct _rampTripSts
{
	unsigned rampTripUpStarted	    :1;
    unsigned rampTripUpCompleted	:1;
    unsigned rampTripDnStarted	    :1;
    unsigned rampTripDnCompleted	:1;
    unsigned rampOrgSenToggled      :1;
	unsigned unused			        :11;
}rampTripSts_t;

#ifndef PROGRAMMABLE_DEBOUNCE
typedef struct _safetySensors
{
    SHORT (*sensorPortPin)(VOID);
	unsigned sensorCurrVal	            :1;
    unsigned sensorPrevVal	            :1;
    unsigned sensorCurrSteadyVal	    :1;
    unsigned sensorPrevSteadyVal    	:1;
	unsigned unused			            :12;
    SHORT sensorDebounceCnt;
    SHORT sensorData;
    VOID (*sensorFuncPtr)(BOOL);
}safetySensors_t;
#else
#define	LOW		0
#define HIGH	1
typedef struct _safetySensors
{
    SHORT (*sensorPortPin)(VOID);
	unsigned sensorCurrVal	            :1;
    unsigned sensorPrevVal	            :1;
    unsigned sensorCurrSteadyVal	    :1;
    unsigned sensorPrevSteadyVal    	:1;
	unsigned unused			            :12;
    SHORT sensorHighDebounceCnt;
	SHORT sensorLowDebounceCnt;
	SHORT sensorSteadyStateCnt;
    SHORT sensorData;
    VOID (*sensorFuncPtr)(BOOL);
}safetySensors_t;
#endif	//	PROGRAMMABLE_DEBOUNCE

EXTERN InputFlags_u inputFlags;     /* Application Input flags */
EXTERN InputFlags_u inputFlags_Installation;
EXTERN StatusFlags_t flags;         /* Application status flag */
EXTERN rampStatusFlags_t rampStatusFlags;
EXTERN rampOutputStatus_t rampOutputStatus;
EXTERN rampTripSts_t rampTripSts;
EXTERN safetySensors_t sensorList[];
EXTERN SHORT saveParamToEepromCnt;

/* This function initializes all variables required by ramp generator */
VOID initRampGenerator(VOID);

/* This function charges bootstrap capacitor for quick start */
VOID chargeBootstraps(VOID);

/* This function starts all services required to run motor */
VOID startMotor(VOID);

/* This function stops all services required to run motor */
VOID stopMotor(VOID);

/* This function generates ramp for shutter operation */
VOID runProfileRamp(VOID);

/* This function starts ramp generator */
VOID startRampGenerator(VOID);

/* This function stops ramp generator */
VOID stopRampGenerator(VOID);

/* This function is required to update drive status - Moving up, Moving Down, Stopped */
UINT8 getDriveMovement(VOID);

VOID calculateDrift(BOOL);

VOID checkRampCommand(VOID);
VOID startShutter(VOID);
VOID stopShutter(VOID);
VOID executeRampProfile(VOID);
VOID checkRampTripStatus(VOID);

VOID executeRampState(VOID);
VOID brakingRequired(VOID);
VOID runShutterToReqSpeed(VOID);
VOID forceStopShutter(VOID);
VOID updateShutterOutputStatus(VOID);
VOID initRampProfileData(VOID);
VOID initJogProfileData(VOID);
VOID initInchProfileData(VOID);
VOID initApertureProfileData(VOID);
VOID reAdjustRampPositions(VOID);
VOID initProfileData(VOID);
VOID checkPhotoElecObsLevel(BOOL);
VOID microSwSensorTiggered(BOOL);
VOID tempSensorTriggered(BOOL);
//	Added on 20Feb2015 for IGBT over temperature fault
VOID igbtOverTempSensorTriggered(BOOL sts);
VOID fourPtLimitSwTriggered(BOOL);
VOID emergencySensorSwTriggered(BOOL);
VOID pwmBufferControl(SHORT);
VOID overcurrentfaultTriggered(BOOL);
//VOID checkParamUpdateToEEP(VOID);
VOID calcShtrMinDistValue(VOID);
VOID calcShtrStopDistValue(VOID);

SHORT readEmergencySensorSts(VOID);
SHORT readMicroSwSensorSts(VOID);
SHORT readAirSwSensorSts(VOID);
SHORT readWrapSwSensorSts(VOID);
SHORT readPhotoElecSensorSts(VOID);
SHORT readTemperatureSensorSts(VOID);
SHORT readOriginSensorSts(VOID);
SHORT readFourPtSensorSts(VOID);
SHORT readPowerFailSensorSts(VOID);
VOID initSensorList(VOID);
VOID monitorSafetySensors(VOID);
VOID resetSensorStatus(VOID);

VOID checkPwmCoastingRequired(VOID);
VOID updatePhotoElectricDebounceTime(VOID);

#endif /* RAMP_GENERATOR_H */
