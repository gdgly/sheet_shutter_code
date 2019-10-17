/********************************************************************************
* FileName: ApertureUpDown.h
* Description:  
* This header file declares Structures for Aperture-Up and Aperture-Down profiles.
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

#define APER_UP_STATES  3
#define APER_DN_STATES  3

//Position in hall counts
#define APERLOWPOS  10
#define APERHI_POS  500
#define APERPOS_OFFSET  100

//Start and End speed of profile
#define APERSTRTSPD 0
#define APERENDSPD  50

//Start amd End DC injection time
#define APERSTRTDCTIME  100
#define APERENDDCTIME   3000
#define APERSTRTDCCNT  (APERSTRTDCTIME/RAMP_GENERATOR_TIME_PERIOD)
#define APERENDDCCNT   (APERENDDCTIME/RAMP_GENERATOR_TIME_PERIOD)

#define APER_RATED_SPEED 2000

//Parameters for UP profile
//APER up speed percentage
#define APERUPPERCENT   50

//Speed in rpm
#define APERUPSPD   (((DWORD)APERUPPERCENT*APER_RATED_SPEED)/100)

//Position in hall counts
#define APERUPPOS   (APERHI_POS-APERPOS_OFFSET)

//Parameters for DOWN profile
//APER up speed percentage
#define APERDNPERCENT   50

//Speed in rpm
#define APERDNSPD   (((DWORD)APERDNPERCENT*APER_RATED_SPEED)/100)

//Position in hall counts
#define APERDNPOS   (APERLOWPOS+APERPOS_OFFSET)

rampStructure_t rampApertureUpProfile[APER_UP_STATES] = {
/*Parameters      rampGenFlags       ,startPosition,endPosition, startSpeed, endSpeed,speedChangeRate,startCurrent,endCurrent,currentChangeRate,startOpenloop,endOpenloop,openLoopRate,dcInjectionDuty,dcInjectionTime*/
/* State 0 */{{1,0,0,0,0,1,0,1,1,0,0},	APERLOWPOS,	  APERUPPOS,	 APERSTRTSPD, APERUPSPD,	ACC_STEP,		0,			0,		    0,				    0,			0,		    0,			SHUTTER_LOAD_HOLDING_DUTY,  APERSTRTDCCNT},
/* State 1 */{{1,0,0,0,0,0,0,0,1,0,0},	APERUPPOS,	  APERUPPOS,	 APERUPSPD,	  APERUPSPD,		0,			0,			0,			0,					0,			0,			0,			0,				            0},
/* State 2 */{{1,0,0,0,1,0,1,0,1,0,0},	APERUPPOS,	  APERHI_POS,    APERUPSPD,	  APERENDSPD,	DEC_STEP,		0,			0,			0,					0,			0,			0,			SHUTTER_LOAD_HOLDING_DUTY,	 APERENDDCCNT},
};/* End of Aperture up profile */

rampStructure_t rampApertureDnProfile[APER_DN_STATES] = {		
/*Parameters       rampGenFlags      ,startPosition,endPosition, startSpeed, endSpeed,speedChangeRate,startCurrent,endCurrent,currentChangeRate,startOpenloop,endOpenloop,openLoopRate,dcInjectionDuty,dcInjectionTime*/
/* State 0 */{{1,0,0,0,0,1,0,1,0,1,0},  APERHI_POS,	 APERDNPOS,	  APERSTRTSPD, APERDNSPD,	ACC_STEP,		0,			0,		    0,				    0,			0,		    0,			SHUTTER_LOAD_HOLDING_DUTY_DOWN_START,	APERSTRTDCCNT},
/* State 1 */{{1,0,0,0,0,0,0,0,0,1,0},	APERDNPOS,	 APERDNPOS,   APERDNSPD,   APERDNSPD,		0,			0,			0,			0,					0,			0,			0,				0,				        0},
/* State 2 */{{1,0,0,0,1,0,1,0,0,1,0},	APERDNPOS,	 APERLOWPOS,  APERDNSPD,   APERENDSPD,	DEC_STEP,		0,			0,			0,					0,			0,			0,		    SHUTTER_LOAD_HOLDING_DUTY,	 APERENDDCCNT},
};/* End of Aperture down profile */

