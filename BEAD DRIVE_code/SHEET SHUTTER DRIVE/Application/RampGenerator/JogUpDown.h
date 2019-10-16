/********************************************************************************
* FileName: JogUpDown.h
* Description:  
* This header file declares Structures for Jog-Up and Jog-Down profiles.
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
 *  21/05/2014            iGate          Initial Creation                                                               
*****************************************************************************/
#include "./Common/Typedefs/Typedefs.h"
#include "RampGenerator.h"

#define JOG_UP_STATES  3
#define JOG_DN_STATES  3

//Position in hall counts
#define JOGLOWPOS  10
#define JOGHI_POS  678
#define JOGPOS_OFFSET  100

//Start and End speed of profile
#define JOGSTRTSPD 0
#define JOGENDSPD  50

//Start amd End DC injection time
#define JOGSTRTDCTIME  100//0//1000 //DC injection not required during startup
#define JOGENDDCTIME   3000
#define JOGSTRTDCCNT  (JOGSTRTDCTIME/RAMP_GENERATOR_TIME_PERIOD)
#define JOGENDDCCNT   (JOGENDDCTIME/RAMP_GENERATOR_TIME_PERIOD)

#define JOG_RATED_SPEED 2000

//Parameters for UP profile
//JOG up speed percentage
#define JOGUPPERCENT   50

//Speed in rpm
#define JOGUPSPD   (((DWORD)JOGUPPERCENT*JOG_RATED_SPEED)/100)

//Position in hall counts
#define JOGUPPOS   (JOGHI_POS-JOGPOS_OFFSET)//550

//Parameters for DOWN profile
//JOG up speed percentage
#define JOGDNPERCENT   50

//Speed in rpm
#define JOGDNSPD   (((DWORD)JOGDNPERCENT*JOG_RATED_SPEED)/100)

//Position in hall counts
#define JOGDNPOS   (JOGLOWPOS+JOGPOS_OFFSET)

rampStructure_t rampJogUpProfile[JOG_UP_STATES] = {
/*Parameters      rampGenFlags       ,startPosition,endPosition, startSpeed, endSpeed,speedChangeRate,startCurrent,endCurrent,currentChangeRate,startOpenloop,endOpenloop,openLoopRate,dcInjectionDuty,dcInjectionTime*/
/* State 0 */{{1,0,0,0,0,1,0,1,1,0,0},	JOGLOWPOS,	  JOGUPPOS,	 JOGSTRTSPD, JOGUPSPD,	ACC_STEP,		0,			0,		    0,				    0,			0,		    0,			SHUTTER_LOAD_HOLDING_DUTY,  JOGSTRTDCCNT},
/* State 1 */{{1,0,0,0,0,0,0,0,1,0,0},	JOGUPPOS,	  JOGUPPOS,	 JOGUPSPD,	 JOGUPSPD,		0,			0,			0,			0,					0,			0,			0,			0,				            0},
/* State 2 */{{1,0,0,0,1,0,1,0,1,0,0},	JOGUPPOS,	  JOGHI_POS, JOGUPSPD,	 JOGENDSPD,	DEC_STEP,		0,			0,			0,					0,			0,			0,			SHUTTER_LOAD_HOLDING_DUTY,	 JOGENDDCCNT},
};/* End of Jog up profile */

rampStructure_t rampJogDnProfile[JOG_DN_STATES] = {		
/*Parameters       rampGenFlags      ,startPosition,endPosition, startSpeed, endSpeed,speedChangeRate,startCurrent,endCurrent,currentChangeRate,startOpenloop,endOpenloop,openLoopRate,dcInjectionDuty,dcInjectionTime*/
/* State 0 */{{1,0,0,0,0,1,0,1,0,1,0},  JOGHI_POS,	 JOGDNPOS,	 JOGSTRTSPD, JOGDNSPD,	ACC_STEP,		0,			0,		    0,				    0,			0,		    0,			SHUTTER_LOAD_HOLDING_DUTY,	JOGSTRTDCCNT},
/* State 1 */{{1,0,0,0,0,0,0,0,0,1,0},	JOGDNPOS,	 JOGDNPOS,   JOGDNSPD,   JOGDNSPD,		0,			0,			0,			0,					0,			0,			0,				0,				        0},
/* State 2 */{{1,0,0,0,1,0,1,0,0,1,0},	JOGDNPOS,	 JOGLOWPOS,  JOGDNSPD,   JOGENDSPD,	DEC_STEP,		0,			0,			0,					0,			0,			0,		    SHUTTER_LOAD_HOLDING_DUTY,	 JOGENDDCCNT},
};/* End of Jog down profile */
