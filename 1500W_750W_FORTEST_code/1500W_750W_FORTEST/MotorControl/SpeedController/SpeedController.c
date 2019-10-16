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
//#define ENABLE_MOTOR_CABLE_FAULT

#define PERIOD_FILTER_CONST 2000//2500//2000 /* the smaller the value, the higher the filter
                                    //and the delay introduced */
/* In the sinewave generation algorithm we need an offset to be added to the */
/* pointer when energizing the motor */
//#define PHASE_OFFSET_CW 1800 //measured offset is 100*(360/65536) = 5.4 degrees.
//#define PHASE_OFFSET_CCW 9840 //182 counts for 1 degree

#ifdef USE_PHASE_INC_AND_CORRECTION
#define PHASE_OFFSET_CW_750W 2002//1274 //measured offset is 1274*(360/65536) = 7 degrees.
#define PHASE_OFFSET_CCW_750W 9828//9646//53 degrees
//#define PHASE_OFFSET_CW_1500W 9828//6916//6006//5460//5096//4004//1820//728//364//1274 //measured offset is 1274*(360/65536) = 7 degrees.
//#define PHASE_OFFSET_CCW_1500W 364 //5096//5460//6006//6370//6916//7280//8372//8736//9464//9828//9100//53 degrees

#define PHASE_OFFSET_CW_def_1500W 6916//2366//6916//364 //measured offset is 364*(360/65536) = 2 degrees.
#define PHASE_OFFSET_CCW_def_1500W 5460//5096//8008//9828 // Fukui result - 54 degree 10192
#define PHASE_OFFSET_CW_MAX_1500W 10556
#define PHASE_OFFSET_CCW_MAX_1500W 364 //5096     //2016/08/17 Down Moving after Over Current by IME
#define PHASE_OFFSET_INC_STEP_1500W 10
#define PHASE_OFFSET_DEC_STEP_1500W 20

#else
#define PHASE_OFFSET_CW 10000 
#define PHASE_OFFSET_CCW 0
#endif
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

#define MS_500T 1000//300//500          /* after this time has elapsed, the motor is    */
                            /* consider stalled and it's stopped    */

/* PI parameters */
//#define P_SPEED_PI_CW 10000//12000
//#define I_SPEED_PI_CW 1000//9000
//#define P_SPEED_PI_CCW 10000//9000//15000//9000
//#define I_SPEED_PI_CCW 500//300//800//1000//2000//6000
#define P_SPEED_PI_CW_750W 10000//6000//7000//10000//13106//20000//15000//5000
#define I_SPEED_PI_CW_750W 700//1500//1800//2000//9830//10000//8000//4000
#define P_SPEED_PI_CCW_750W 10000//22000
#define I_SPEED_PI_CCW_750W 700//100
#define P_SPEED_PI_CW_1500W 10000//6000//7000//10000//13106//20000//15000//5000
#define I_SPEED_PI_CW_1500W 1000//1500//1800//2000//9830//10000//8000//4000
#define P_SPEED_PI_CCW_1500W 10000//22000
#define I_SPEED_PI_CCW_1500W 500//100
#define C_SPEED_PI 0x7FFF 
#define MAX_SPEED_PI    31128   //95% of max value ie 32767

/* In the sinewave generation algorithm we need an offset to be added to the */
/* pointer when energizing the motor in CCW. This is done to compensate an   */
/* asymetry of the sinewave */
SHORT phaseOffsetCW =PHASE_OFFSET_CW_def_1500W;
SHORT phaseOffsetCCW =PHASE_OFFSET_CCW_def_1500W;


#define SET_TARGET_SPEED_750W_CW    1800//1900 //Required final speed
#define SET_TARGET_SPEED_750W_CCW   1800
#define SPEED_INC_STP_750W       100
#define SET_TARGET_SPEED_1500W    2500//1900//1900 //Required final speed
#define SPEED_INC_STP_1500W       50
#define SPD_INC_INTERVAL    100

#define SPD_CAL_FOR_PHASEADVANCE    (int)((float)((((float)(measuredSpeed / 60) * NO_POLEPAIRS_750W) * 360) / 1000))
#define MAX_PH_ADV_DEG      1
#define MAX_PH_ADV 		(int)(((float)MAX_PH_ADV_DEG / 360.0) * 65536.0)
UINT16 PhaseAdvance;


/* Period filter for speed measurement */
DWORD periodFilter;
UINT16	cnt_motor_stop = 0;

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

//#if (MOTOR_TYPE == PICOMO_MOTOR)
//    CHAR sectorTable[] = {INVALID,SECTOR_ZERO,SECTOR_FOUR,SECTOR_FIVE,SECTOR_TWO,SECTOR_ONE,SECTOR_THREE,INVALID};
//#elif (MOTOR_TYPE == NEW_MOTOR_1)
//    CHAR sectorTable[] = {INVALID,SECTOR_THREE,SECTOR_FIVE,SECTOR_FOUR,SECTOR_ONE,SECTOR_TWO,SECTOR_ZERO,INVALID};
//#elif (MOTOR_TYPE == NEW_MOTOR_2)
CHAR sectorTable[8];
//#else
//    #error Motor sectors not defined
//#endif
  
/* Variables containing the Period of half an electrical cycle, which is an */
/* interrupt each edge of one of the hall sensor input */
DWORD period;
DWORD phaseIncPerSec = 0;

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

//SHORT speedError;

//Observed hall counts for one hall sensor (IC2) is 155, 148, 151
SHORT hallCounts = 0;

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

WORD MotorRunCount = 0;
WORD MotorCycleCount = 0;
BYTE MotorStartComplete = 0;
BYTE MotorDecActive = 0;


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
//    PRIVATE WORD cnt10ms = 0;
    
    IFS0bits.T1IF = 0;
    
    static WORD cnt1000ms = 0;
    //if(!MotorStartComplete)
    //{
        if(++cnt1000ms >= SPD_INC_INTERVAL)
        {
            cnt1000ms = 0;
            if(MotorDecActive == 0)
            {
                if(PreMotorType == MOTOR_750W)
                {
                    if(requiredDirection == CW)
                    {
                        if(refSpeed < SET_TARGET_SPEED_750W_CW)
                        {            
                            refSpeed += SPEED_INC_STP_750W;
                            if(refSpeed >= SET_TARGET_SPEED_750W_CW)
                            {
                                refSpeed = SET_TARGET_SPEED_750W_CW;
                                //MotorStartComplete = 1;
                            }
                        }
                    }
                    else
                    {
                        if(refSpeed < SET_TARGET_SPEED_750W_CCW)
                        {            
                            refSpeed += SPEED_INC_STP_750W;
                            if(refSpeed >= SET_TARGET_SPEED_750W_CCW)
                            {
                                refSpeed = SET_TARGET_SPEED_750W_CCW;
                                //MotorStartComplete = 1;
                            }
                        }
                    }
                }
                else if(PreMotorType == MOTOR_1500W)
                {
                    if(refSpeed < SET_TARGET_SPEED_1500W)
                    {            
                        refSpeed += SPEED_INC_STP_1500W;
                        if(refSpeed >= SET_TARGET_SPEED_1500W)
                        {
                            refSpeed = SET_TARGET_SPEED_1500W;
                        //MotorStartComplete = 1;
                        }
                    }
                }
            }
            else
            {
                if(refSpeed > 350)
                {            
                    refSpeed -= 200;
                    if(refSpeed <= 50)
                        refSpeed = 20;
                }  
            }
        }         
	measureActualSpeed();
    
    if(++cnt_motor_stop>CNT_10MS*4)
	{
		measuredSpeed = 0;
		cnt_motor_stop = CNT_10MS*4;
	}
    
    speedControl(); 
    
    monitorSectorRotation();
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt (void)
{
    IFS0bits.T3IF = 0;
    totalTimePeriod += PR3;
}

SHORT getCurrentSectorNo(VOID)
{
    if(PreMotorType == MOTOR_750W)
    {
        sectorTable[0] = INVALID;
        sectorTable[1] = SECTOR_ONE;
        sectorTable[2] = SECTOR_THREE;
        sectorTable[3] = SECTOR_TWO;
        sectorTable[4] = SECTOR_FIVE;
        sectorTable[5] = SECTOR_ZERO;
        sectorTable[6] = SECTOR_FOUR;
        sectorTable[7] = INVALID;
        //sectorTable[8] = {INVALID,SECTOR_ONE,SECTOR_THREE,SECTOR_TWO,SECTOR_FIVE,SECTOR_ZERO,SECTOR_FOUR,INVALID};
    }
    else if(PreMotorType == MOTOR_1500W)
    {
        sectorTable[0] = INVALID;
        sectorTable[1] = SECTOR_ZERO;
        sectorTable[2] = SECTOR_TWO;
        sectorTable[3] = SECTOR_ONE;
        sectorTable[4] = SECTOR_FOUR;
        sectorTable[5] = SECTOR_FIVE;
        sectorTable[6] = SECTOR_THREE;
        sectorTable[7] = INVALID;
//        sectorTable[8] = {INVALID,SECTOR_ZERO,SECTOR_TWO,SECTOR_ONE,SECTOR_FOUR,SECTOR_FIVE,SECTOR_THREE,INVALID};
    }
    hallValue = HALLA_BIT + (HALLB_BIT << 1) + (HALLC_BIT << 2);    
	sector = sectorTable[hallValue];	//Get sector from table
   
    if(sector == INVALID)
    {
        sector = lastSector;       
    }  
    return(sector);
}

VOID monitorSectorRotation(VOID)
{
    if((!rampStatusFlags.rampDcInjectionOn)&&(!rampStatusFlags.rampBrakeOn))
    {

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
	//	Added to handle "offset at upper & lower limit"
    
    IFS0bits.IC1IF = 0;
    
    currentSector = getCurrentSectorNo();
    
    /* This MUST be done for getting around the HW slow rate */
	//	Added check for invalid sector to handle "offset at upper & lower limit"
	if ((currentSector != lastSector) && (sector != INVALID))	
	{        
        //rampStatusFlags.saveParamToEeprom = TRUE;
        saveParamToEepromCnt = 0;        
        //currentSectorNo = currentSector;
        
//		/* Motor current direction is computed based on sector */        
//        #if (MOTOR_TYPE == PICOMO_MOTOR)
//            if ((currentSector == SECTOR_FIVE) || (currentSector == SECTOR_TWO))
//        #elif (MOTOR_TYPE == NEW_MOTOR_1)
//            if ((currentSector == SECTOR_FIVE) || (currentSector == SECTOR_TWO))
//        #elif (MOTOR_TYPE == NEW_MOTOR_2)
//            if ((currentSector == SECTOR_ZERO) || (currentSector == SECTOR_THREE))
//        #else
//            #error IC1 incorrect sector
//        #endif  
        currentSectorNo = currentSector;
        if(PreMotorType == MOTOR_750W)
        {
            if ((currentSector == SECTOR_ZERO) || (currentSector == SECTOR_THREE))       
            {
                 #if (HALL_INC_DIR == CW)
                    currentDirection = CCW;
                    hallCounts++;
                #else
                    currentDirection = CW;
                    hallCounts--;
                #endif   
            }
            else
            {
                #if (HALL_INC_DIR == CW)
                    currentDirection = CW;
                    hallCounts--;
                #else
                    currentDirection = CCW;
                    hallCounts++;
                #endif
            }
            calculatePhaseValue(currentSector);
            //PORTAbits.RA7 = 0;
            lastSector = currentSector; /* Update last sector */
        }
        else if(PreMotorType == MOTOR_750W)
        {
            currentSectorNo = currentSector;
            if ((currentSector == SECTOR_FIVE) || (currentSector == SECTOR_TWO))
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
        }
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
    
	
	//	Added to handle "offset at upper & lower limit"	
    
    IFS0bits.IC2IF = 0;
    
    currentSector = getCurrentSectorNo();
    
    /* This MUST be done for getting around the HW slow rate */
	//	Added check for invalid sector to handle "offset at upper & lower limit"
	if ((currentSector != lastSector) && (sector != INVALID))
	{  
        //rampStatusFlags.saveParamToEeprom = TRUE;
        saveParamToEepromCnt = 0;
        
        T3CONbits.TON = 0;
        totalTimePeriod += TMR3;
		TMR3 = 0;
		T3CONbits.TON = 1;
		hall2Triggered = 1;
        
        currentSectorNo = currentSector;
        
		/* Motor current direction is computed based on sector */        
//        #if (MOTOR_TYPE == PICOMO_MOTOR)
//            if ((currentSector == SECTOR_THREE) || (currentSector == SECTOR_ZERO))
//        #elif (MOTOR_TYPE == NEW_MOTOR_1)
//            if ((currentSector == SECTOR_ONE) || (currentSector == SECTOR_FOUR))
//        #elif (MOTOR_TYPE == NEW_MOTOR_2)
        if(PreMotorType == MOTOR_750W)
        {
            if ((currentSector == SECTOR_FIVE) || (currentSector == SECTOR_TWO))       
            {
                 #if (HALL_INC_DIR == CW)
                    currentDirection = CCW;
                    hallCounts++;
                #else
                    currentDirection = CW;
                    hallCounts--;
                #endif   
            }
            else
            {
                #if (HALL_INC_DIR == CW)
                    currentDirection = CW;
                    hallCounts--;
                #else
                    currentDirection = CCW;
                    hallCounts++;
                #endif
            }
            calculatePhaseValue(currentSector);
            //PORTAbits.RA7 = 0;
            lastSector = currentSector; /* Update last sector */
        }
        else if(PreMotorType == MOTOR_1500W)
        {
            currentSectorNo = currentSector;
            
			if ((currentSector == SECTOR_FOUR) || (currentSector == SECTOR_ONE))
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
        }
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
	
	//	Added to handle "offset at upper & lower limit"	
    
    IFS2bits.IC3IF = 0;
    
    currentSector = getCurrentSectorNo();    
  
    /* This MUST be done for getting around the HW slow rate */
	//	Added check for invalid sector to handle "offset at upper & lower limit"
	if ((currentSector != lastSector) && (sector != INVALID))
	{        
        //rampStatusFlags.saveParamToEeprom = TRUE;
        saveParamToEepromCnt = 0;
        
        currentSectorNo = currentSector;
        if(PreMotorType == MOTOR_750W)
        {
            if ((currentSector == SECTOR_ONE) || (currentSector == SECTOR_FOUR))       
            {
                 #if (HALL_INC_DIR == CW)
                    currentDirection = CCW;
                    hallCounts++;
                #else
                    currentDirection = CW;
                    hallCounts--;
                #endif   
            }
            else
            {
                #if (HALL_INC_DIR == CW)
                    currentDirection = CW;
                    hallCounts--;
                #else
                    currentDirection = CCW;
                    hallCounts++;
                #endif
            }
            calculatePhaseValue(currentSector);
            //PORTAbits.RA7 = 0;
            lastSector = currentSector; /* Update last sector */
        }
        else if(PreMotorType == MOTOR_1500W)
        {
            currentSectorNo = currentSector;

			if ((currentSector == SECTOR_ZERO) || (currentSector == SECTOR_THREE))
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
        }
	}
}

#ifdef USE_PHASE_INC_AND_CORRECTION
VOID calculatePhaseValue(WORD sectorNo)
{   
	if(controlOutput < 0)
	{
		LED_RED = 0;
	}
    
    if(PreMotorType == MOTOR_750W)
    {
        phaseOffsetCW = PHASE_OFFSET_CW_750W;
        phaseOffsetCCW = PHASE_OFFSET_CCW_750W;
    }
    else if(PreMotorType == MOTOR_1500W)
    {
//        phaseOffsetCW = PHASE_OFFSET_CW_1500W;
//        phaseOffsetCCW = PHASE_OFFSET_CCW_1500W;
            if(measuredSpeed >= 500)
            {    
                if(requiredDirection == CW)
                {
                    if(phaseOffsetCW < PHASE_OFFSET_CW_MAX_1500W)
                        phaseOffsetCW  += PHASE_OFFSET_INC_STEP_1500W;
                    if(phaseOffsetCW >= PHASE_OFFSET_CW_MAX_1500W)
                        phaseOffsetCW = PHASE_OFFSET_CW_MAX_1500W;  
                }
                else if(requiredDirection == CCW)
                {
                    if(phaseOffsetCCW > PHASE_OFFSET_CCW_MAX_1500W)
                        phaseOffsetCCW  -= PHASE_OFFSET_INC_STEP_1500W;
                    if(phaseOffsetCCW <= PHASE_OFFSET_CCW_MAX_1500W)
                        phaseOffsetCCW = PHASE_OFFSET_CCW_MAX_1500W;
                }
            }
            else
            {
                if(phaseOffsetCW > PHASE_OFFSET_CW_def_1500W)
                    phaseOffsetCW -= PHASE_OFFSET_DEC_STEP_1500W;
                if(phaseOffsetCW <= PHASE_OFFSET_CW_def_1500W)
                    phaseOffsetCW = PHASE_OFFSET_CW_def_1500W;
                if(phaseOffsetCCW < PHASE_OFFSET_CCW_def_1500W)
                    phaseOffsetCCW += PHASE_OFFSET_DEC_STEP_1500W;
                if(phaseOffsetCCW >= PHASE_OFFSET_CCW_def_1500W)
                    phaseOffsetCCW = PHASE_OFFSET_CCW_def_1500W;
            }        
    }
    #if 1        
    /* Motor commutation is actually based on the required direction, not */
    /* the current dir. This allows driving the motor in four quadrants */    
    if(((controlOutput >= 0) && (requiredDirection == CW)) || ((controlOutput < 0) && (requiredDirection == CCW)))
    {
        tmpQu = __builtin_divmodud((DWORD)sectorNo, (WORD)SECTOR_END, &tmpRe);
        //Use phase offset calculation only when shutter moving up and DC injection not applied
        if((requiredDirection == CW) && (!rampStatusFlags.rampDcInjectionOn))
            phaseCopy = phase = phaseValues[tmpRe] + phaseOffsetCW;
        else
			phaseCopy = phase = phaseValues[tmpRe] + phaseOffsetCW;// + phaseValue;
//        if((requiredDirection == CW) && (!rampStatusFlags.rampDcInjectionOn))
//            phaseCopy = phase = phaseValues[tmpRe] + (phaseOffsetCW + phaseValue);
//        else
//			phaseCopy = phase = phaseValues[tmpRe] + phaseOffsetCW;
        
        phaseIncFlg = PHASE_INCREMENT_FLAG;
    }
    else if(((controlOutput >= 0) && (requiredDirection == CCW)) || ((controlOutput < 0) && (requiredDirection == CW)))
    {
        /* For CCW an offset must be added to compensate difference in */
        /* symmetry of the sine table used for CW and CCW */
        tmpQu = __builtin_divmodud((DWORD)(sectorNo + SECTOR_THREE), (WORD)SECTOR_END, &tmpRe);
        //Use phase offset calculation only when shutter moving up
        if((requiredDirection == CW) && (!rampStatusFlags.rampDcInjectionOn))
            phaseCopy = phase = phaseValues[tmpRe]+ (phaseOffsetCCW - phaseValue);
        else
            phaseCopy = phase = phaseValues[tmpRe] + phaseOffsetCCW;// + phaseValue;
//        if((requiredDirection == CW) && (!rampStatusFlags.rampDcInjectionOn))
//            phaseCopy = phase = phaseValues[tmpRe]+ (phaseOffsetCCW - phaseValue);
//        else
//            phaseCopy = phase = phaseValues[tmpRe]+ phaseOffsetCCW;
        
        
        phaseIncFlg = PHASE_DECREMENT_FLAG;
    }
    #else
    if (requiredDirection == CW)
	{
        phaseCopy = phase = phaseValues[sectorNo] + 2002;
        phaseIncFlg = PHASE_INCREMENT_FLAG;
	}
	else
	{
        // For CCW an offset must be added to compensate difference in 
        // symmetry of the sine table used for CW and CCW
		phaseCopy = phase = phaseValues[(sectorNo + 3) % 6] + 9828; //+ phaseValue;//+ PhaseOffset;
        phaseIncFlg = PHASE_DECREMENT_FLAG;
	}
    #endif
    
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
        cnt_motor_stop = 0;
    }
    if(PreMotorType == MOTOR_750W)
    {
        if (period < MINPERIOD_750W)  
            period = MINPERIOD_750W;
        else if (period > MAXPERIOD_750W)
            period = MAXPERIOD_750W;
    }
    else if(PreMotorType == MOTOR_1500W)
    {
        if (period < MINPERIOD_1500W)  
            period = MINPERIOD_1500W;
        else if (period > MAXPERIOD_1500W)
            period = MAXPERIOD_1500W;
    }
    
	periodStateVar+= ((period - periodFilter)*(periodFilterConstant));
	periodFilter = periodStateVar>>15;
    if(PreMotorType == MOTOR_750W)
        measuredSpeed = __builtin_divud(SPEED_RPM_CALC_750W,periodFilter);
    else if(PreMotorType == MOTOR_1500W)
        measuredSpeed = __builtin_divud(SPEED_RPM_CALC_1500W,periodFilter);
    phaseInc = __builtin_divud(PHASE_INC_CALC,periodFilter);
//    phaseIncPerSec = __builtin_muluu(measuredSpeed,655);
//    phaseInc = __builtin_divud(phaseIncPerSec,1000);
    
    
    register int a_reg asm("A");
    a_reg = __builtin_mpy(MAX_PH_ADV,SPD_CAL_FOR_PHASEADVANCE , 0,0,0,0,0,0);//SPD_CAL_FOR_PHASEADVANCE
    PhaseAdvance = __builtin_sac(a_reg,0);
    if(requiredDirection == CCW)
        PhaseAdvance = -PhaseAdvance;
        
}

VOID speedControl(VOID) 
{   
    speedPIparms.qInRef = refSpeed;
    speedPIparms.qInMeas = measuredSpeed;
    
    if(PreMotorType == MOTOR_1500W)
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
    else if(PreMotorType == MOTOR_750W)
    {
        speedPIparms.qOutMax = currentLimitClamp;
        speedPIparms.qOutMin = -(currentLimitClamp);  
    }
        calcPiNew(&speedPIparms);
        
        if(flags.speedControl)
            controlOutput = speedPIparms.qOut;
    
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
VOID intitSpeedController(VOID)
{ 
    
    hall2Triggered = 0;
    totalTimePeriod = 0;
    measuredSpeed = 0; 
    refSpeed = 200;
    
    PhaseAdvance = 0;
    
    if(PreMotorType == MOTOR_750W)
    {
        controlOutput = 0;
        period = MAXPERIOD_750W;
        periodFilter = MAXPERIOD_750W;
        periodFilterConstant = PERIOD_FILTER_CONST;
        periodStateVar = ((DWORD)MAXPERIOD_750W << 15);
        phaseInc = __builtin_divud(PHASE_INC_CALC, periodFilter);
    }
    else if(PreMotorType == MOTOR_1500W)
    {
        controlOutput = 0;
        period = MAXPERIOD_1500W;
        periodFilter = MAXPERIOD_1500W;
        periodFilterConstant = PERIOD_FILTER_CONST;
        periodStateVar = ((DWORD)MAXPERIOD_1500W << 15);
        phaseInc = __builtin_divud(PHASE_INC_CALC, periodFilter);
    }
    
    //speedError = 0;
    tmpQu = 0;
    tmpRe = 0;
    
    if(PreMotorType == MOTOR_750W)
    {
        sectorTable[0] = INVALID;
        sectorTable[1] = SECTOR_ONE;
        sectorTable[2] = SECTOR_THREE;
        sectorTable[3] = SECTOR_TWO;
        sectorTable[4] = SECTOR_FIVE;
        sectorTable[5] = SECTOR_ZERO;
        sectorTable[6] = SECTOR_FOUR;
        sectorTable[7] = INVALID;
//        sectorTable[8] = {INVALID,SECTOR_ONE,SECTOR_THREE,SECTOR_TWO,SECTOR_FIVE,SECTOR_ZERO,SECTOR_FOUR,INVALID};
    }
    else if(PreMotorType == MOTOR_1500W)
    {
        sectorTable[0] = INVALID;
        sectorTable[1] = SECTOR_ZERO;
        sectorTable[2] = SECTOR_TWO;
        sectorTable[3] = SECTOR_ONE;
        sectorTable[4] = SECTOR_FOUR;
        sectorTable[5] = SECTOR_FIVE;
        sectorTable[6] = SECTOR_THREE;
        sectorTable[7] = INVALID;
//        sectorTable[8] = {INVALID,SECTOR_ZERO,SECTOR_TWO,SECTOR_ONE,SECTOR_FOUR,SECTOR_FIVE,SECTOR_THREE,INVALID};
    }

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
    if(PreMotorType == MOTOR_750W)
    {
        if(requiredDirection == CW)
        {
//            initPiNew(&speedPIparms,P_SPEED_PI_CW_750W,I_SPEED_PI_CW_750W,C_SPEED_PI,currentLimitClamp,-(currentLimitClamp),0);
            initPiNew(&speedPIparms,P_SPEED_PI_CW_750W,I_SPEED_PI_CW_750W,C_SPEED_PI,5000,-5000,0);
        }
        else
        {
            initPiNew(&speedPIparms,P_SPEED_PI_CCW_750W,I_SPEED_PI_CCW_750W,C_SPEED_PI,5000,-5000,0);
        }
    }
    else if(PreMotorType == MOTOR_1500W)
    {
        if(requiredDirection == CW)
        {
            initPiNew(&speedPIparms,P_SPEED_PI_CW_1500W,I_SPEED_PI_CW_1500W,C_SPEED_PI,15000,-15000,0);
        }
        else
        {
            initPiNew(&speedPIparms,P_SPEED_PI_CCW_1500W,I_SPEED_PI_CCW_1500W,C_SPEED_PI,15000,-15000,0);
        }
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

/*VOID intitSpeedController(VOID)
{
    initSpeedControllerVariables();    
}*/
