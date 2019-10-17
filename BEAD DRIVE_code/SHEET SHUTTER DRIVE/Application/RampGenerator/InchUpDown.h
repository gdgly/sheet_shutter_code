/********************************************************************************
* FileName: InchUpDown.h
* Description:  
* This header file declares structures for InchUp an Inch Down Profiles.
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

#define INCH_UP_STATES  3
#define INCH_DN_STATES  3

// 1 hall count = 3mm;

//Position in hall counts
#define INHLOWPOS  10
#define INHHI_POS  678
#define INHPOS_OFFSET  50

//Start and End speed of profile
#define INHSTRTSPD 0
#define INHENDSPD  50

//Start amd End DC injection time
#define INHSTRTDCTIME  100
#define INHENDDCTIME   3000

#define INHSTRTDCCNT  (INHSTRTDCTIME/RAMP_GENERATOR_TIME_PERIOD)
#define INHENDDCTCNT   (INHENDDCTIME/RAMP_GENERATOR_TIME_PERIOD)

#define INH_RATED_SPEED 2500

//Parameters for UP profile
//INH up speed percentage
#define INHUPPERCENT   10

//Speed in rpm
#define INHUPSPD   (((DWORD)INHUPPERCENT*INH_RATED_SPEED)/100)

//Position in hall counts
#define INHUPPOS   (INHHI_POS-INHPOS_OFFSET)

//Parameters for DOWN profile
//INH up speed percentage
#define INHDNPERCENT   10

//Speed in rpm
#define INHDNSPD   (((DWORD)INHDNPERCENT*INH_RATED_SPEED)/100)

//Position in hall counts
#define INHDNPOS   (INHLOWPOS+INHPOS_OFFSET)

rampStructure_t rampInchUpProfile[INCH_UP_STATES] = {
/*Parameters      rampGenFlags       ,startPosition,endPosition, startSpeed, endSpeed,speedChangeRate,startCurrent,endCurrent,currentChangeRate,startOpenloop,endOpenloop,openLoopRate,dcInjectionDuty,dcInjectionTime*/
/* State 0 */{{1,0,0,0,0,1,0,1,1,0,0},	INHLOWPOS,	  INHUPPOS,	 INHSTRTSPD, INHUPSPD,	ACC_STEP,		0,			0,		    0,				    0,			0,		    0,			SHUTTER_LOAD_HOLDING_DUTY,  INHSTRTDCCNT},
/* State 1 */{{1,0,0,0,0,0,0,0,1,0,0},	INHUPPOS,	  INHUPPOS,	 INHUPSPD,	 INHUPSPD,		0,			0,			0,			0,					0,			0,			0,			0,				            0},
/* State 2 */{{1,0,0,0,1,0,1,0,1,0,0},	INHUPPOS,	  INHHI_POS, INHUPSPD,	 INHENDSPD,	DEC_STEP,		0,			0,			0,					0,			0,			0,			SHUTTER_LOAD_HOLDING_DUTY,	 INHENDDCTCNT},
};/* End of INH up profile */

rampStructure_t rampInchDnProfile[INCH_DN_STATES] = {		
/*Parameters       rampGenFlags      ,startPosition,endPosition, startSpeed, endSpeed,speedChangeRate,startCurrent,endCurrent,currentChangeRate,startOpenloop,endOpenloop,openLoopRate,dcInjectionDuty,dcInjectionTime*/
/* State 0 */{{1,0,0,0,0,1,0,1,0,1,0},  INHHI_POS,	 INHDNPOS,	 INHSTRTSPD, INHDNSPD,	ACC_STEP,		0,			0,		    0,				    0,			0,		    0,			SHUTTER_LOAD_HOLDING_DUTY_DOWN_START,	INHSTRTDCCNT},
/* State 1 */{{1,0,0,0,0,0,0,0,0,1,0},	INHDNPOS,	 INHDNPOS,   INHDNSPD,   INHDNSPD,		0,			0,			0,			0,					0,			0,			0,				0,				        0},
/* State 2 */{{1,0,0,0,1,0,1,0,0,1,0},	INHDNPOS,	 INHLOWPOS,  INHDNSPD,   INHENDSPD,	DEC_STEP,		0,			0,			0,					0,			0,			0,		    SHUTTER_LOAD_HOLDING_DUTY,	 INHENDDCTCNT},
};/* End of INH down profile */
