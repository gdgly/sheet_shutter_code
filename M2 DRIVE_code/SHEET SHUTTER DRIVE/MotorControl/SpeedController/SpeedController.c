/*********************************************************************************
* FileName: SpeedController.c
* Description:
* This source file contains the definition of all the functions for SpeedController.
* It implements all the functions required by speed controller.
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
#include "SpeedController.h"
#include "./MotorControl/PIController/pi.h"
#include "./Common/UserDefinition/Userdef.h"
#include "./Application/RampGenerator/RampGenerator.h"
#include "./Common/Extern/Extern.h"
#include "./MotorControl/Algorithm/svm.h"
#include "./Middleware/ParameterDatabase/eeprom.h"
#include "./Common/Delay/Delay.h"
#include "./DMCI/RtdmInterface.h"

//	Macro to enable feature of motor cable fault - Dec 2015
#define ENABLE_MOTOR_CABLE_FAULT

#define PERIOD_FILTER_CONST 2000//2500//2000 /* the smaller the value, the higher the filter
                                    //and the delay introduced */
/* In the sinewave generation algorithm we need an offset to be added to the */
/* pointer when energizing the motor */
//#define PHASE_OFFSET_CW 1800 //measured offset is 100*(360/65536) = 5.4 degrees.
//#define PHASE_OFFSET_CCW 9840 //182 counts for 1 degree

#ifdef USE_PHASE_INC_AND_CORRECTION
// Measures against overcurrent error 20180123 by IME
//#define PHASE_OFFSET_CW 2366//6916//364 //measured offset is 364*(360/65536) = 2 degrees.
//#define PHASE_OFFSET_CW  1092  // 6 degrees.
#define PHASE_OFFSET_CW 2366 //6916 20180125
#define PHASE_OFFSET_CW_START 6916 // 2366 20180125
#define PHASE_OFFSET_CCW 9828//5096//8008//9828 // Fukui result - 54 degree 10192
#define PHASE_OFFSET_CCW_START 6916
#define PHASE_OFFSET_CW_MAX 6916
#define PHASE_OFFSET_CCW_MAX 9828 //5096     //2016/08/17 Down Moving after Over Current by IME
#define PHASE_OFFSET_INC_STEP 10
#define PHASE_OFFSET_DEC_STEP 20
#else
#define PHASE_OFFSET_CW 10000
#define PHASE_OFFSET_CCW 0
#endif

#define SPD_CAL_FOR_PHASEADVANCE    (int)((float)((((float)(measuredSpeed / 60) * NO_POLEPAIRS) * 360) / 1000))
#define MAX_PH_ADV_DEG      1
#define MAX_PH_ADV 		(int)(((float)MAX_PH_ADV_DEG / 360.0) * 65536.0)
UINT16 PhaseAdvance;


/*******************************************************************************/
/* These Phase values represent the base Phase value of the sinewave for each  */
/* one of the sectors (each sector is a translation of the hall effect sensors */
/* reading */
/****************************************************************************/
#define PHASE_ZERO 	0
#define PHASE_ONE	((PHASE_ZERO + 65536/6) % 65536)
#define PHASE_TWO	((PHASE_ONE + 65536/6) % 65536)
#define PHASE_THREE	((PHASE_TWO + 65536/6) % 65536)
#define PHASE_FOUR	((PHASE_THREE + 65536/6) % 65536)
#define PHASE_FIVE	((PHASE_FOUR + 65536/6) % 65536)

#define INVALID     -1
#define CNT_10MS    10      /* Used as a timeout with no hall effect sensors */
                            /* transitions and Forcing steps according to the */
                            /* actual position of the motor */

// 2016/3/3 Motor Stal & PWM Cost
//#define MS_500T 10000//5000//1000//300//500          /* after this time has elapsed, the motor is    */
#define MS_500T 500		// 2017/06/13 by IME
                            /* consider stalled and it's stopped    */

/* PI parameters */
#define P_SPEED_PI_CW 10000//12000
#define I_SPEED_PI_CW 1000//9000
#define P_SPEED_PI_CCW 10000//9000//15000//9000
#define I_SPEED_PI_CCW 500//300//800//1000//2000//6000
#define C_SPEED_PI 0x7FFF
#define MAX_SPEED_PI    31128   //95% of max value ie 32767

/* In the sinewave generation algorithm we need an offset to be added to the */
/* pointer when energizing the motor in CCW. This is done to compensate an   */
/* asymetry of the sinewave */
// Measures against overcurrent error 20180123 by IME
SHORT phaseOffsetCW  = PHASE_OFFSET_CW;
//SHORT phaseOffsetCCW = PHASE_OFFSET_CCW;
//SHORT phaseOffsetCW  = PHASE_OFFSET_CW_START; //20180126
SHORT phaseOffsetCCW = PHASE_OFFSET_CCW_START;

/* Period filter for speed measurement */
DWORD periodFilter;
// 2016/3/3 Motor Stal & PWM Cost
int	cnt_motor_stop = 0;

/* Constants used for properly energizing the motor depending on the rotor's position */
CONST SHORT phaseValues[SECTOR_END] =
{PHASE_ZERO, PHASE_ONE, PHASE_TWO, PHASE_THREE, PHASE_FOUR, PHASE_FIVE};

/* PI configuration structure */
tPIParm speedPIparms;

/* This variable is incremented by the PWM interrupt in order to generate a proper sinewave. Its value */
/* is incremented by a value of PhaseInc, which represents the frequency of the generated sinewave */
WORD phase;
WORD phaseCopy;
SHORT phaseIncFlg;

/* This variable holds the hall sensor input readings */
WORD hallValue;

/* This variables holds present sector value, which is the rotor position */
SHORT sector;

/* This variable holds the last sector value. This is critical to filter slow slew rate on the Hall */
/* effect sensors hardware */
WORD lastSector;

/* This array translates the hall state value read from the digital I/O to the */
/* proper sector.  Hall values of 0 or 7 represent illegal values and therefore */
/* return -1. */

#if (MOTOR_TYPE == PICOMO_MOTOR)
    CHAR sectorTable[] = {INVALID,SECTOR_ZERO,SECTOR_FOUR,SECTOR_FIVE,SECTOR_TWO,SECTOR_ONE,SECTOR_THREE,INVALID};
#elif (MOTOR_TYPE == NEW_MOTOR_1)
    CHAR sectorTable[] = {INVALID,SECTOR_THREE,SECTOR_FIVE,SECTOR_FOUR,SECTOR_ONE,SECTOR_TWO,SECTOR_ZERO,INVALID};
#elif (MOTOR_TYPE == NEW_MOTOR_2)
    CHAR sectorTable[] = {INVALID,SECTOR_ONE,SECTOR_THREE,SECTOR_TWO,SECTOR_FIVE,SECTOR_ZERO,SECTOR_FOUR,INVALID};
#elif (MOTOR_TYPE == NEW_MOTOR_3)
	CHAR sectorTable[] = {INVALID,SECTOR_ZERO,SECTOR_TWO,SECTOR_ONE,SECTOR_FOUR,SECTOR_FIVE,SECTOR_THREE,INVALID};
#else
    #error Motor sectors not defined
#endif

/* Variables containing the Period of half an electrical cycle, which is an */
/* interrupt each edge of one of the hall sensor input */
DWORD period;

/* Used as a temporal variable to perform a fractional divide operation in */
/* assembly */
SHORT measuredSpeed;  /* Actual speed for the PID */
SHORT refSpeed = 200;	    /* Desired speeds for the PID */

/* Output of PID controller, use its sign for required direction */
SHORT controlOutput;
SHORT ctrlOpPercent;

/* Filter used for speed measurement */
WORD periodFilterConstant;
DWORD periodStateVar;

//Observed hall counts for one hall sensor (IC2) is 155, 148, 151
SHORT hallCounts = 0;
SHORT hallCounts_bak = 0x7FFF;

/* Variable used by inbuilt division function */
UINT tmpQu = 0;
UINT tmpRe = 0;

SHORT hall2Triggered;
DWORD totalTimePeriod;

SHORT monitorSectorRoatCnt;
SHORT currentSectorNo;
SHORT nextSectorNo;
SHORT previousSectorNo;
SHORT phaseInc;

BOOL  FLAG_overLoad =FALSE;
WORD  OverLoad_cnt=0;

/* This function is used to measure actual running speed of motor */
VOID measureActualSpeed(VOID);

/* This function is speed PI controller */
VOID speedControl(VOID);

/* This function initializes all the variables used by speed controller */
VOID initSpeedControllerVariables(VOID);

VOID monitorSectorRotation(VOID);

/******************************************************************************
 * _T1Interrupt
 *
 * The _T1Interrupt calculates actual speed of motor and runs speed PI controller.
 * It checks for motor stalled and calls force commutation to run motor. If the
 * motor is stalled for more than 1 sec then stop all services to run the motor.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt (void)
{
    SHORT currentSector;
//    PRIVATE WORD cnt10ms = 0;

    IFS0bits.T1IF = 0;

	measureActualSpeed();

// 2016/3/3 Motor Stal & PWM Cost
#ifdef BUG_No88_M2overcurrentfault
     if(cnt_motor_stop>5)
     {
        if (requiredDirection == CW || requiredDirection== CCW)
        {
            currentSector = getCurrentSectorNo();
            calculatePhaseValue(currentSector);
        }
     }
#endif
     if(cnt_motor_stop>10)
     {
         measuredSpeed = 0;
     }


#ifdef ENABLE_MOTOR_CABLE_FAULT
	// **********************************************************************************************************************************************************
	// code to monitor "Motor cable fault" Error and stop the motor
	// "Motor cable fault" needs to declare in following condition
	// If user press down button when shutter is not at lower limit and motor cable has some connection issue, which result shutter to fall down wih high speed

#define		MTR_CABLE_FAULT_TOP_UP_SPEED_VALUE_TO_S1_DOWN	500  // it is an top speed to S1 down speed to declare "motor cable fault" error
#define		MTR_CABLE_FAULT_MONITOR_TIME					500//250         // it is an time period in msec for which "motor cable fault" error needs to monitor,
	// before stoping the motor and declaring the error
	// max limit = 250 msec

	static UINT8 lsCnt1ms = 0,lsCnt1msStarted = 0;

	if (requiredDirection == CCW && currentDirection == CCW) // Monitor error only when required and current direction as "going down"
	{
		if (0 == lsCnt1msStarted ) // timer not started
		{
			// once speed cross the set limit then start timer to monitor the event
			if (
					measuredSpeed > \
					(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s1Down_A528 \
					+ MTR_CABLE_FAULT_TOP_UP_SPEED_VALUE_TO_S1_DOWN)
				)
			{
				lsCnt1msStarted               = 1;  // timer started
			}
		} //if (0 == lsCnt1msStarted) // timer not started
		else if (1 == lsCnt1msStarted) // timer started
		{
			// if goes below the set limit then turn off and reset the timer
			if (measuredSpeed < (uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.s1Down_A528 + MTR_CABLE_FAULT_TOP_UP_SPEED_VALUE_TO_S1_DOWN))
			{
				lsCnt1msStarted = 0;  // timer stopped
				lsCnt1ms = 0;         // reset timer
			}
			else
			{
				lsCnt1ms++; // increment timer

				// monitor timer whether same cross the set limit
				if (lsCnt1ms >= MTR_CABLE_FAULT_MONITOR_TIME)
				{
					uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.motorCableFault = TRUE;
					forceStopShutter(); // stop motor by applying mechanical brake
					// 2016/11/16 When Down , Missing Save Origin Position.
					hallCounts_bak = 0x7FFF;
				}
			}
		} //else if (1 == lsCnt1msStarted) // timer started
	}
	else
	{
		lsCnt1msStarted = 0;  // timer stopped
		lsCnt1ms = 0;         // reset timer
	}

	// **********************************************************************************************************************************************************
#endif	//	ENABLE_MOTOR_CABLE_FAULT

    //if(++cnt10ms >= CNT_10MS)
    //{
    //    cnt10ms = 0;
        if(rampStatusFlags.rampSpeedControlRequired)
            speedControl();
    //}

    monitorSectorRotation();
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt (void)
{
    IFS0bits.T3IF = 0;
    totalTimePeriod += PR3;
}

SHORT getCurrentSectorNo(VOID)
{
    hallValue = HALLA_BIT + (HALLB_BIT << 1) + (HALLC_BIT << 2);
	sector = sectorTable[hallValue];	//Get sector from table

    if(sector == INVALID)
    {
        sector = lastSector;

        //rampCurrentState = RAMP_PWM_COASTING;
        //pwmCostingReq = TRUE;
        //currentLimitClamp = controlOutput;
        //outputDecRate = __builtin_divud(currentLimitClamp, PWM_COASTING_TIME);

        //set hall sensor error flag
        //uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.hallSensor = TRUE;
    }
    //else
    //{
    //    //reset hall sensor error flag
    //    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.hallSensor = FALSE;
    //}

    return(sector);
}

VOID monitorSectorRotation(VOID)
{
    if((!rampStatusFlags.rampDcInjectionOn)&&(!rampStatusFlags.rampBrakeOn))
    {
        //currentSectorNo = sector;
        //check rotation and reset the counter
        //sector number increases in CCW direction
        if(requiredDirection == CW)
        {
            //if we got the expected sector number then reset the count and load next sector number
            if(currentSectorNo == nextSectorNo)
            {
                monitorSectorRoatCnt = 0;
                nextSectorNo = currentSectorNo + 1;
                //if sector reached to end then rotate sector
                if(nextSectorNo > SECTOR_FIVE)
                {
                    nextSectorNo = SECTOR_ZERO;
                }
            }
            else
            {
                monitorSectorRoatCnt++;
            }
        }
        //sector number decreases in CW direction
        else
        {
            if(currentSectorNo == previousSectorNo)
            {
                monitorSectorRoatCnt = 0;
                previousSectorNo = currentSectorNo - 1;
                //if sector reached to end then rotate sector
                if(previousSectorNo < SECTOR_ZERO)
                {
                    previousSectorNo = SECTOR_FIVE;
                }
            }
            else
            {
                monitorSectorRoatCnt++;
            }
        }

        //check if we have crossed the sector rotation time required then immediately stop motor.
        if(monitorSectorRoatCnt > MS_500T)
        {
#ifdef BUG_CQxx_BD_IGBTdamage
            //if emergency switch is triggered the stop shutter immediately
            forceStopShutter();
			// 2016/11/16 When Down , Missing Save Origin Position.
			hallCounts_bak = 0x7FFF;
#else
            rampCurrentState = RAMP_PWM_COASTING;
            pwmCostingReq = TRUE;
            currentLimitClamp = controlOutput;
            outputDecRate = __builtin_divud(currentLimitClamp, PWM_COASTING_TIME);
#endif
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorStall = TRUE;
        }
        else
        {
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorStall = FALSE;
        }
    }
    else
    {
        monitorSectorRoatCnt = 0;
    }
}

/******************************************************************************
 * monitorShutterFalseMovement
 *
 * The monitorShutterFalseMovement updates how many times shutter moved in wrong direction.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
void monitorShutterFalseMovement(void)
{
	static BYTE lsucUp = 0;
	static BYTE lsucDown = 0;
	// Added for displaying errors on display screen in case of false movement - RN - NOV 2015
	if(currentDirection == requiredDirection)
	{
		if(requiredDirection == CCW)
			gucShutterFalseUpMovementCount = 0;
		else
			gucShutterFalseDownMovementCount = 0;
	}
	else
	{
		if(requiredDirection == CCW)
			gucShutterFalseUpMovementCount++;
		else
			gucShutterFalseDownMovementCount++;

		if(gucShutterFalseUpMovementCount > lsucUp)
		{
			lsucUp = gucShutterFalseUpMovementCount;
		}
		if(gucShutterFalseDownMovementCount > lsucDown)
		{
			lsucDown = gucShutterFalseDownMovementCount;
		}
	}
}

/******************************************************************************
 * _IC1Interrupt
 *
 * The _IC1Interrupt calculates the actual mechanical direction of rotation of
 * the motor, and adjust the Phase variable depending on the sector the rotor is in.
 * The sector is validated in order to avoid any spurious interrupt due to a slow
 * slew rate on the halls inputs due to hardware filtering. For Phase adjustment in
 * CCW, an offset is added to compensate non-symetries in the sine table used.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _IC1Interrupt (void)
{
    SHORT currentSector;

    IFS0bits.IC1IF = 0;

    currentSector = getCurrentSectorNo();

    /* This MUST be done for getting around the HW slow rate */
	if (currentSector != lastSector)
	{
        //rampStatusFlags.saveParamToEeprom = TRUE;
        saveParamToEepromCnt = 0;
        currentSectorNo = currentSector;

		/* Motor current direction is computed based on sector */
        #if (MOTOR_TYPE == PICOMO_MOTOR)
            if ((currentSector == SECTOR_FIVE) || (currentSector == SECTOR_TWO))
        #elif (MOTOR_TYPE == NEW_MOTOR_1)
            if ((currentSector == SECTOR_FIVE) || (currentSector == SECTOR_TWO))
        #elif (MOTOR_TYPE == NEW_MOTOR_2)
            if ((currentSector == SECTOR_ZERO) || (currentSector == SECTOR_THREE))
		#elif (MOTOR_TYPE == NEW_MOTOR_3)
			if ((currentSector == SECTOR_FIVE) || (currentSector == SECTOR_TWO))
        #else
            #error IC1 incorrect sector
        #endif
        {
            currentDirection = CW;
            #if (HALL_INC_DIR == CW)
                hallCounts++;
            #else
                hallCounts--;
            #endif
        }
		else
        {
            currentDirection = CCW;
            #if (HALL_INC_DIR == CW)
                hallCounts--;
            #else
                hallCounts++;
            #endif
        }
        calculatePhaseValue(currentSector);
		lastSector = currentSector;

		//20170628
		// Monitoring of shutter false moment should present in all interrupt - YG - NOV 15
		monitorShutterFalseMovement();
	}
}

/******************************************************************************
 * _IC2Interrupt
 *
 * The _IC2Interrupt calculates the actual period between hall effect sensor
 * transitions and it calculates the actual mechanical direction of rotation of
 * the motor, and adjust the Phase variable depending on the sector the rotor is in.
 * The sector is validated in order to avoid any spurious interrupt due to a slow
 * slew rate on the halls inputs due to hardware filtering. For Phase adjustment in
 * CCW, an offset is added to compensate non-symetries in the sine table used.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _IC2Interrupt (void)
{
	SHORT currentSector;

    IFS0bits.IC2IF = 0;

	// 2016/3/3 Motor Stal & PWM Cost
	cnt_motor_stop = 0;

    currentSector = getCurrentSectorNo();

    /* This MUST be done for getting around the HW slow rate */
	if (currentSector != lastSector)
	{
        T3CONbits.TON = 0;
        totalTimePeriod += TMR3;
        TMR3 = 0;
        T3CONbits.TON = 1;
        hall2Triggered = 1;

        //rampStatusFlags.saveParamToEeprom = TRUE;
        saveParamToEepromCnt = 0;

        currentSectorNo = currentSector;

		/* Motor current direction is computed based on sector */
        #if (MOTOR_TYPE == PICOMO_MOTOR)
            if ((currentSector == SECTOR_THREE) || (currentSector == SECTOR_ZERO))
        #elif (MOTOR_TYPE == NEW_MOTOR_1)
            if ((currentSector == SECTOR_ONE) || (currentSector == SECTOR_FOUR))
        #elif (MOTOR_TYPE == NEW_MOTOR_2)
            if ((currentSector == SECTOR_FIVE) || (currentSector == SECTOR_TWO))
		#elif (MOTOR_TYPE == NEW_MOTOR_3)
			if ((currentSector == SECTOR_FOUR) || (currentSector == SECTOR_ONE))
        #else
            #error IC2 incorrect sector
        #endif
            {
                currentDirection = CW;
                #if (HALL_INC_DIR == CW)
                    hallCounts++;
                #else
                    hallCounts--;
                #endif
            }
            else
            {
                currentDirection = CCW;
                #if (HALL_INC_DIR == CW)
                    hallCounts--;
                #else
                    hallCounts++;
                #endif
            }
        calculatePhaseValue(currentSector);
		lastSector = currentSector;

		//20170628
		// Monitoring of shutter false moment should present in all interrupt - YG - NOV 15
		monitorShutterFalseMovement();
	}
}

/******************************************************************************
 * _IC3Interrupt
 *
 * The _IC3Interrupt calculates the actual mechanical direction of rotation of
 * the motor, and adjust the Phase variable depending on the sector the rotor is in.
 * The sector is validated in order to avoid any spurious interrupt due to a slow
 * slew rate on the halls inputs due to hardware filtering. For Phase adjustment in
 * CCW, an offset is added to compensate non-symetries in the sine table used.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
void __attribute__((interrupt, no_auto_psv)) _IC3Interrupt (void)
{
    SHORT currentSector;

    IFS2bits.IC3IF = 0;

    currentSector = getCurrentSectorNo();

    /* This MUST be done for getting around the HW slow rate */
	if (currentSector != lastSector)
	{
        //rampStatusFlags.saveParamToEeprom = TRUE;
        saveParamToEepromCnt = 0;

        currentSectorNo = currentSector;

		/* Motor current direction is computed based on sector */
        #if (MOTOR_TYPE == PICOMO_MOTOR)
            if ((currentSector == SECTOR_ONE) || (currentSector == SECTOR_FOUR))
        #elif (MOTOR_TYPE == NEW_MOTOR_1)
            if ((currentSector == SECTOR_THREE) || (currentSector == SECTOR_ZERO))
        #elif (MOTOR_TYPE == NEW_MOTOR_2)
            if ((currentSector == SECTOR_ONE) || (currentSector == SECTOR_FOUR))
		#elif (MOTOR_TYPE == NEW_MOTOR_3)
			if ((currentSector == SECTOR_ZERO) || (currentSector == SECTOR_THREE))
        #else
            #error IC3 incorrect sector
        #endif
            {
                currentDirection = CW;
                #if (HALL_INC_DIR == CW)
                    hallCounts++;
                #else
                    hallCounts--;
                #endif
            }
            else
            {
                currentDirection = CCW;
                #if (HALL_INC_DIR == CW)
                    hallCounts--;
                #else
                    hallCounts++;
                #endif
            }
        calculatePhaseValue(currentSector);
		lastSector = currentSector;

		//20170628
		// Monitoring of shutter false moment should present in all interrupt - YG - NOV 15
		monitorShutterFalseMovement();
	}
}

#ifdef USE_PHASE_INC_AND_CORRECTION
VOID calculatePhaseValue(WORD sectorNo)
{
// Measures against overcurrent error 20180123 by IME
/*
    if(measuredSpeed >= 500)
    {
        if(requiredDirection == CW)
        {
            if(phaseOffsetCW < PHASE_OFFSET_CW_MAX)
                phaseOffsetCW  += PHASE_OFFSET_INC_STEP;
            if(phaseOffsetCW >= PHASE_OFFSET_CW_MAX)
                phaseOffsetCW = PHASE_OFFSET_CW_MAX;
        }
        else if(requiredDirection == CCW)
        {
            if(phaseOffsetCCW > PHASE_OFFSET_CCW_MAX)
                phaseOffsetCCW  -= PHASE_OFFSET_INC_STEP;
            if(phaseOffsetCCW <= PHASE_OFFSET_CCW_MAX)
                phaseOffsetCCW = PHASE_OFFSET_CCW_MAX;
        }
    }
    else
    {
        if(phaseOffsetCW > PHASE_OFFSET_CW)
            phaseOffsetCW -= PHASE_OFFSET_DEC_STEP;
        if(phaseOffsetCW <= PHASE_OFFSET_CW)
            phaseOffsetCW = PHASE_OFFSET_CW;
        if(phaseOffsetCCW < PHASE_OFFSET_CCW)
            phaseOffsetCCW += PHASE_OFFSET_DEC_STEP;
        if(phaseOffsetCCW >= PHASE_OFFSET_CCW)
            phaseOffsetCCW = PHASE_OFFSET_CCW;
    }
*/
	if(requiredDirection == CW)
	{
		if(hallCounts>uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.riseChangeGearPos1_A103)
		{
			// phaseOffsetCW = PHASE_OFFSET_CW_START; //20180126
			   if(phaseOffsetCW < PHASE_OFFSET_CW_MAX)
                phaseOffsetCW  += PHASE_OFFSET_INC_STEP;
               if(phaseOffsetCW >= PHASE_OFFSET_CW_MAX)
                phaseOffsetCW = PHASE_OFFSET_CW_MAX;
			   
		}
		else
		{
			if(phaseOffsetCW > PHASE_OFFSET_CW)
			    phaseOffsetCW -= PHASE_OFFSET_DEC_STEP;
			if(phaseOffsetCW <= PHASE_OFFSET_CW)
			    phaseOffsetCW = PHASE_OFFSET_CW;
		}
	}
	else if(requiredDirection == CCW)
	{
		if(hallCounts<(uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101
			-uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.fallChangeGearPos1_A106))
		{
			phaseOffsetCCW = PHASE_OFFSET_CCW_START;
		}
		else
		{
            if(phaseOffsetCCW < PHASE_OFFSET_CCW)
                phaseOffsetCCW  += PHASE_OFFSET_INC_STEP;
            if(phaseOffsetCCW >= PHASE_OFFSET_CCW)
                phaseOffsetCCW = PHASE_OFFSET_CCW;
		}
	}

    /* Motor commutation is actually based on the required direction, not */
    /* the current dir. This allows driving the motor in four quadrants */
    if(((controlOutput >= 0) && (requiredDirection == CW)) || ((controlOutput < 0) && (requiredDirection == CCW)))
    {
        tmpQu = __builtin_divmodud((DWORD)sectorNo, (WORD)SECTOR_END, &tmpRe);
        //Use phase offset calculation only when shutter moving up and DC injection not applied
//        if((requiredDirection == CW) && (!rampStatusFlags.rampDcInjectionOn))
//            phaseCopy = phase = phaseValues[tmpRe] + (phaseOffsetCW + phaseValue);
//        else
//			phaseCopy = phase = phaseValues[tmpRe] + phaseOffsetCW;
        if((requiredDirection == CW) && (!rampStatusFlags.rampDcInjectionOn))
            phaseCopy = phase = phaseValues[tmpRe] + phaseOffsetCW;
        else
			phaseCopy = phase = phaseValues[tmpRe] + phaseOffsetCW;// + phaseValue;

        phaseIncFlg = PHASE_INCREMENT_FLAG;
    }
    else if(((controlOutput >= 0) && (requiredDirection == CCW)) || ((controlOutput < 0) && (requiredDirection == CW)))
    {
        /* For CCW an offset must be added to compensate difference in */
        /* symmetry of the sine table used for CW and CCW */
        tmpQu = __builtin_divmodud((DWORD)(sectorNo + SECTOR_THREE), (WORD)SECTOR_END, &tmpRe);
        //Use phase offset calculation only when shutter moving up
//        if((requiredDirection == CW) && (!rampStatusFlags.rampDcInjectionOn))
//            phaseCopy = phase = phaseValues[tmpRe]+ (phaseOffsetCCW - phaseValue);
//        else
//            phaseCopy = phase = phaseValues[tmpRe]+ phaseOffsetCCW;
        if((requiredDirection == CW) && (!rampStatusFlags.rampDcInjectionOn))
            phaseCopy = phase = phaseValues[tmpRe]+ (phaseOffsetCCW - phaseValue);
        else
            phaseCopy = phase = phaseValues[tmpRe] + phaseOffsetCCW;// + phaseValue;
        phaseIncFlg = PHASE_DECREMENT_FLAG;
    }
}
#else
VOID calculatePhaseValue(WORD sectorNo)
{
    /* Motor commutation is actually based on the required direction, not */
    /* the current dir. This allows driving the motor in four quadrants */
    if(((controlOutput >= 0) && (requiredDirection == CW)) || ((controlOutput < 0) && (requiredDirection == CCW)))
    {
        tmpQu = __builtin_divmodud((DWORD)sector, (WORD)SECTOR_END, &tmpRe);
        phase = phaseValues[tmpRe] + phaseOffsetCW;
    }
    else if(((controlOutput >= 0) && (requiredDirection == CCW)) || ((controlOutput < 0) && (requiredDirection == CW)))
    {
        /* For CCW an offset must be added to compensate difference in */
        /* symmetry of the sine table used for CW and CCW */
        tmpQu = __builtin_divmodud((DWORD)(sector + SECTOR_THREE), (WORD)SECTOR_END, &tmpRe);
        phaseCopy = phase = phaseValues[tmpRe]+ phaseOffsetCCW;
    }
}
#endif
/******************************************************************************
 * measureActualSpeed
 *
 * The measureActualSpeed calculates actual speed of motor from the hall sensor
 * transistions period. it applies low pass filter to remove noise component.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID measureActualSpeed(VOID)
{
    if(hall2Triggered)
    {
        period = totalTimePeriod;
        totalTimePeriod = 0;
        hall2Triggered = 0;
// 2016/3/3 Motor Stal & PWM Cost
		cnt_motor_stop = 0;
    }
	if (period < MINPERIOD)
		period = MINPERIOD;
	else if (period > MAXPERIOD)
		period = MAXPERIOD;

	periodStateVar+= ((period - periodFilter)*(periodFilterConstant));
	periodFilter = periodStateVar>>15;
	measuredSpeed = __builtin_divud(SPEED_RPM_CALC,periodFilter);
    phaseInc = __builtin_divud(PHASE_INC_CALC,periodFilter);

    register int a_reg asm("A");
    a_reg = __builtin_mpy(MAX_PH_ADV,SPD_CAL_FOR_PHASEADVANCE , 0,0,0,0,0,0);//SPD_CAL_FOR_PHASEADVANCE
    PhaseAdvance = __builtin_sac(a_reg,0);
    if(requiredDirection == CCW)
        PhaseAdvance = -PhaseAdvance;
}

VOID speedControl(VOID)
{
     //***********************20160906_add over load start************************
// Measures against overcurrent error 20180123 by IME
/*    SHORT  refSpeed_80_pct;
    refSpeed_80_pct = refSpeed*5/10;     //50%
    if((measuredSpeed < refSpeed_80_pct)&&(FLAG_overLoad == FALSE))
    {
        OverLoad_cnt++;
        if(OverLoad_cnt>1500) FLAG_overLoad = TRUE;     //1500ms
    }
    else OverLoad_cnt = 0;
    if((flags.motorRunning ==0)||(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)) FLAG_overLoad = FALSE;
    if(FLAG_overLoad == TRUE)refSpeed = refSpeed/2;   //50%
*/
    //**********************20160906_add over load end **************************


    speedPIparms.qInRef = refSpeed;
    speedPIparms.qInMeas = measuredSpeed;

//    if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == BEAD_SHUTTER)
//    {
// Measures against overcurrent error 20180123 by IME
/*
        if(requiredDirection == CW)
        {
            speedPIparms.qOutMax = currentLimitClamp;
            speedPIparms.qOutMin = -(currentLimitClamp);
        }
        else
        {
            speedPIparms.qOutMax = currentLimitClamp;
            speedPIparms.qOutMin = -(currentLimitClamp);
        }
*/
	speedPIparms.qOutMax = 6*measuredSpeed+5000;
	speedPIparms.qOutMin = -(currentLimitClamp);
//    }
//    else if (uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == M1_SHUTTER)
//    {

		//if(requiredDirection == CW)
        //{
        //    speedPIparms.qOutMax = currentLimitClamp;
        //    speedPIparms.qOutMin = -(currentLimitClamp);
        //}

        //speedPIparms.qOutMax = currentLimitClamp;
        //speedPIparms.qOutMin = -(currentLimitClamp);
//    }


	#if 1
	//	Added on 6 Aug 2015 to enable motor rotation in no load condition for bead type shutter (while shutter go down operation)
	if(rampStatusFlags.rampMaintainHoldingDuty == 0 && monitorSectorRoatCnt > 150)
	{
		rampStatusFlags.rampMaintainHoldingDuty = 1;
	}
    #endif


    if(rampStatusFlags.rampMaintainHoldingDuty)
    {
        if(measuredSpeed < SHUTTER_SPEED_MIN_STOP)
        {
            controlOutput += HOLDING_DUTY_INC;
			// Measures against overcurrent error 20180123 by IME
         	if(controlOutput>5000){
				controlOutput=5000;
			}
        }
        else
        {
            rampStatusFlags.rampMaintainHoldingDuty = 0;
            speedPIparms.qInRef = measuredSpeed;
            speedPIparms.qInMeas = measuredSpeed;
            speedPIparms.qdSum = (LONG)controlOutput << 15;
            speedPIparms.qOut = controlOutput;

            calcPiNew(&speedPIparms);
            if(flags.speedControl)
                controlOutput = speedPIparms.qOut;
        }
    }
    else
    {
        calcPiNew(&speedPIparms);

        if(flags.speedControl)
            controlOutput = speedPIparms.qOut;
        //***********************20160906_add over load start************************
            if(FLAG_overLoad == TRUE)
                controlOutput = controlOutput/2;    //50%
        //**********************20160906_add over load end **************************
    }

    //calculate percentage duty
    ctrlOpPercent = __builtin_divud(((unsigned long)controlOutput*100),MAX_SPEED_PI);
}

/******************************************************************************
 * initSpeedControllerVariables
 *
 * The initSpeedControllerVariables inititializes all the variables used by
 * speed control mode.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initSpeedControllerVariables(VOID)
{
    SHORT P_gain;
    SHORT I_gain;

	SHORT lshcurrentLimitClampNew;

    hall2Triggered = 0;
    totalTimePeriod = 0;
    measuredSpeed = 0;
//    if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == BEAD_SHUTTER)
//    {
//        if(requiredDirection == CW)
//        {
//            controlOutput = SHUTTER_LOAD_HOLDING_DUTY;
//        }
//        else
//        {
//            controlOutput = 0;
//        }
//    }
//    else if (uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == M2_SHUTTER)
        controlOutput = SHUTTER_LOAD_HOLDING_DUTY;

    period = MAXPERIOD;
    periodFilter = MAXPERIOD;
    periodFilterConstant = PERIOD_FILTER_CONST;
    periodStateVar = ((DWORD)MAXPERIOD << 15);
    phaseInc = __builtin_divud(PHASE_INC_CALC, periodFilter);

    hallValue = HALLA_BIT + (HALLB_BIT << 1) + (HALLC_BIT << 2);
	sector = sectorTable[hallValue];	//Get sector from table
    currentSectorNo = lastSector = sector;
    calculatePhaseValue(currentSectorNo);

    nextSectorNo = currentSectorNo + 1;
    //if sector reached to end then rotate sector
    if(nextSectorNo > SECTOR_FIVE)
    {
        nextSectorNo = SECTOR_ZERO;
    }
    previousSectorNo = currentSectorNo - 1;
    //if sector reached to end then rotate sector
    if(previousSectorNo < SECTOR_ZERO)
    {
        previousSectorNo = SECTOR_FIVE;
    }
    monitorSectorRoatCnt = 0;
    saveParamToEepromCnt = 0;

    if(requiredDirection == CW)
    {
        P_gain = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.speed_PI_KP_A512 * 100;
        I_gain = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.speed_PI_KI_A513 * 10;

		if (uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.PWMFreqMotorCtrl_A500 == 0)
		{
			lshcurrentLimitClampNew = 15000;
		}
		else
		{
			lshcurrentLimitClampNew = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.PWMFreqMotorCtrl_A500;
		}

        //initPiNew(&speedPIparms,P_SPEED_PI_CW,I_SPEED_PI_CW,C_SPEED_PI,currentLimitClamp,-(currentLimitClamp),0);
        //initPiNew(&speedPIparms,P_gain,I_gain,C_SPEED_PI,currentLimitClamp,-(currentLimitClamp),0);
		initPiNew(&speedPIparms,P_gain,I_gain,C_SPEED_PI,lshcurrentLimitClampNew,-(lshcurrentLimitClampNew),0);
    }
    else
    {
        P_gain = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.current_PI_KP_A514 * 100;
        I_gain = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.current_PI_KI_A515 * 10;

//        if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == BEAD_SHUTTER)
//        {
//            //initPiNew(&speedPIparms,P_SPEED_PI_CCW,I_SPEED_PI_CCW,C_SPEED_PI,5000,-(currentLimitClamp),0);
//            initPiNew(&speedPIparms,P_gain,I_gain,C_SPEED_PI,5000,-(currentLimitClamp),0);
//        }
//        else if (uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == M2_SHUTTER)
//        {
            //initPiNew(&speedPIparms,P_SPEED_PI_CCW,I_SPEED_PI_CCW,C_SPEED_PI,5000,-(currentLimitClamp),0);
            //initPiNew(&speedPIparms,P_gain,I_gain,C_SPEED_PI,currentLimitClamp,-(currentLimitClamp),0);


			 if (uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.startupDutyCycle_A501 == 0)
			 {
			 lshcurrentLimitClampNew = 15000;
			 }
			 else
			 {
			 lshcurrentLimitClampNew = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.startupDutyCycle_A501;
			 }

			 // initPiNew(&speedPIparms,P_gain,I_gain,C_SPEED_PI,5000,-(currentLimitClamp),0);
			 initPiNew(&speedPIparms,P_gain,I_gain,C_SPEED_PI,lshcurrentLimitClampNew,-(lshcurrentLimitClampNew),0);
//        }
    }
}

/******************************************************************************
 * intitSpeedController
 *
 * The intitSpeedController gives interface of speed controller to other modules.
 * speed control mode.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID intitSpeedController(VOID)
{
    initSpeedControllerVariables();
}
