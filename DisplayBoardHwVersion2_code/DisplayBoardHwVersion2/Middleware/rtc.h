/*********************************************************************************
* FileName: rtc.h
* Description: 
* This source file contains the prototype definitions for real time clock
* functionality.
* Version: 0.1D
*
*
**********************************************************************************/

/****************************************************************************
 * Copyright 2014 Bunka Shutters.
 * This program is the property of the Bunka Shutters
 * and it shall not be reproduced, distributed or used
 * without permission of an authorized company official.
 * This is an unpublished work subject to Trade Secret
 * and Copyright protection.
*****************************************************************************/


/****************************************************************************
 *  Modification History
 *
 *  Revision		Date                  Name          			Comments
 *  	0.1D	31/03/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

#ifndef RTC_H
#define RTC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <time.h>

/****************************************************************************
 *  Macro definitions:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global variables:
****************************************************************************/


/****************************************************************************/

/****************************************************************************
 *  Structures:
****************************************************************************/

/****************************************************************************/

/******************************************************************************
 * FunctionName: RTCEnable
 *
 * Function Description: Enables the RTC feature
 *
 * Function Parameters: None
 *
 * Function Returns: None
 *
 ********************************************************************************/
extern void RTCEnable(void);
/********************************************************************************/

/******************************************************************************
 * FunctionName: initRTC
 *
 * Function Description: Initialize RTC
 *
 * Function Parameters: None
 *
 * Function Returns: None
 *
 ********************************************************************************/
extern void initRTC(void);

/******************************************************************************
 * FunctionName: RTCDisable
 *
 * Function Description: Disables the RTC feature
 *
 * Function Parameters: None
 *
 * Function Returns: None
 *
 ********************************************************************************/
extern void RTCDisable(void);
/********************************************************************************/

/******************************************************************************
 * FunctionName: RTCSet
 *
 * Function Description:
 * Sets the value of the real time clock (RTC)
 *
 * Function Parameters:
 * lstDateTime		:	stDateTime type object. Date and time values.
 *
 * Function Returns	:
 * 0	:	Success
 * 1	:	RTC is disabled
 *
 ********************************************************************************/
extern uint8_t RTCSet(struct tm *lDateTime);
/********************************************************************************/

/******************************************************************************
 * FunctionName: RTCGet
 *
 * Function Description:
 * Gets the value of the real time clock (RTC)
 *
 * Function Parameters:
 * lstDateTime	:	stDateTime type object. Date and time values.
 *
 * Function Returns:
 * 0	:	Success
 * 1	:	RTC is disabled
 *
 ********************************************************************************/
extern uint8_t RTCGet(struct tm *lDateTime);
/********************************************************************************/


/********************************************************************************/

#endif /*RTC_H*/
