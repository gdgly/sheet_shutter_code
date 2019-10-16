/********************************************************************************
* FileName: RampProfileUpDown.h
* Description:  
* This header file declares structures for Ramp-up and Ramp-down profiles.
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

#define RAMP_UP_STATES  7
#define RAMP_DN_STATES  7

//Position in hall counts
#define RMPLOWPOS  50
#define RMPHI_POS  678

//Start and End speed of profile
#define RMPSTRTSPD 0
#define RMPENDSPD  50

//Start amd End DC injection time
#define RMPSTRTDCTIME  100 //DC injection not required during startup
//	DC injection time is reduced to 100mS (200 - MECHANICAL_LOCK_ACTIVATION_DELAY)
//	as mechanical brake is applied in 10mS(as per datasheet) - Dec 2015
//#define RMPENDDCTIME   3000
#define RMPENDDCTIME   200
#define RMPSTRTDCCNT  (RMPSTRTDCTIME/RAMP_GENERATOR_TIME_PERIOD)
#define RMPENDDCCNT   (RMPENDDCTIME/RAMP_GENERATOR_TIME_PERIOD)

//Parameters for UP profile
//Speed in rpm
#define RMPUPS1SPD   1500
#define RMPUPS2SPD   750
#define RMPUPS3SPD   750

//Position in hall counts
#define RMPUPS1POS   339
#define RMPUPS2POS   550
#define RMPUPS3POS   550

//Parameters for DOWN profile
//Speed in rpm
#define RMPDNS1SPD   1500
#define RMPDNS2SPD   750
#define RMPDNS3SPD   750

//Position in hall counts
#define RMPDNS1POS   339
#define RMPDNS2POS   150
#define RMPDNS3POS   150

rampStructure_t rampUpGoingProfile[RAMP_UP_STATES] = {/* Start of ramp up profile */
/*Parameters       rampGenFlags      ,startPosition,endPosition, startSpeed,  endSpeed,   speedChangeRate,startCurrent,endCurrent,currentChangeRate,startOpenloop,endOpenloop,openLoopRate,dcInjectionDuty,dcInjectionTime*/
/* State 0 */{{1,0,0,0,0,1,0,1,1,0,0},	 RMPLOWPOS,	 RMPUPS1POS, RMPSTRTSPD, RMPUPS1SPD,	ACC_STEP,			0,			0,		    0,				    0,			0,		    0,		SHUTTER_LOAD_HOLDING_DUTY,	RMPSTRTDCCNT},
/* State 1 */{{1,0,0,0,0,0,0,0,1,0,0},	 RMPLOWPOS,	 RMPUPS1POS, RMPUPS1SPD, RMPUPS1SPD,		 0,			    0,			0,			0,					0,			0,			0,				    0,			    0},
/* State 2 */{{1,0,0,0,0,0,0,0,1,0,0},	 RMPUPS1POS, RMPUPS2POS, RMPUPS1SPD, RMPUPS2SPD,	DEC_STEP,			0,			0,			0,					0,			0,			0,				    0,		        0},
/* State 3 */{{1,0,0,0,0,0,0,0,1,0,0},	 RMPUPS1POS, RMPUPS2POS, RMPUPS2SPD, RMPUPS2SPD,		 0,			    0,			0,			0,					0,			0,			0,				    0,			    0},
/* State 4 */{{1,0,0,0,0,0,0,0,1,0,0},	 RMPUPS2POS, RMPUPS3POS, RMPUPS2SPD, RMPUPS3SPD,	DEC_STEP,		    0,			0,			0,					0,			0,			0,				    0,		        0},
/* State 5 */{{1,0,0,0,0,0,0,0,1,0,0},	 RMPUPS2POS, RMPUPS3POS, RMPUPS3SPD, RMPUPS3SPD,		 0,			    0,			0,			0,					0,			0,			0,				    0,			    0},
/* State 6 */{{1,0,0,0,1,0,1,0,1,0,0},	 RMPUPS3POS, RMPHI_POS,	 RMPUPS3SPD, RMPENDSPD,	    DEC_STEP,		    0,			0,			0,					0,			0,			0,	    SHUTTER_LOAD_HOLDING_DUTY,	RMPENDDCCNT },
};/* End of ramp up profile*/

rampStructure_t rampDnGoingProfile[RAMP_DN_STATES] = { /* Start of Ramp down profile */
/*Parameters       rampGenFlags      ,startPosition,endPosition, startSpeed,  endSpeed,   speedChangeRate,startCurrent,endCurrent,currentChangeRate,startOpenloop,endOpenloop,openLoopRate,dcInjectionDuty,dcInjectionTime*/
/* State 0 */{{1,0,0,0,0,1,0,1,0,1,0},	 RMPHI_POS,	 RMPDNS1POS, RMPSTRTSPD, RMPDNS1SPD,	ACC_STEP,			0,			0,		    0,				    0,			0,		    0,		SHUTTER_LOAD_HOLDING_DUTY,	RMPSTRTDCCNT},
/* State 1 */{{1,0,0,0,0,0,0,0,0,1,0},	 RMPHI_POS,	 RMPDNS1POS, RMPDNS1SPD, RMPDNS1SPD,		 0,			    0,			0,			0,					0,			0,			0,				    0,			    0},
/* State 2 */{{1,0,0,0,0,0,0,0,0,1,0},	 RMPDNS1POS, RMPDNS2POS, RMPDNS1SPD, RMPDNS2SPD,	DEC_STEP,			0,			0,			0,					0,			0,			0,				    0,		        0},
/* State 3 */{{1,0,0,0,0,0,0,0,0,1,0},	 RMPDNS2POS, RMPDNS2POS, RMPDNS2SPD, RMPDNS2SPD,		 0,			    0,			0,			0,					0,			0,			0,				    0,			    0},
/* State 4 */{{1,0,0,0,0,0,0,0,0,1,0},	 RMPDNS2POS, RMPDNS3POS, RMPDNS2SPD, RMPDNS3SPD,	DEC_STEP,		    0,			0,			0,					0,			0,			0,				    0,		        0},
/* State 5 */{{1,0,0,0,0,0,0,0,0,1,0},	 RMPDNS3POS, RMPDNS3POS, RMPDNS3SPD, RMPDNS3SPD,		 0,			    0,			0,			0,					0,			0,			0,				    0,			    0},
/* State 6 */{{1,0,0,0,1,0,1,0,0,1,0},	 RMPDNS3POS, RMPLOWPOS,	 RMPDNS3SPD, RMPENDSPD,	    DEC_STEP,		    0,			0,			0,					0,			0,			0,	    SHUTTER_LOAD_HOLDING_DUTY,	RMPENDDCCNT },
};/* End of ramp down profile*/
