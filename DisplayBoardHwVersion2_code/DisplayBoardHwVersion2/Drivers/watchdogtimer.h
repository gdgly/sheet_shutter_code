/*********************************************************************************
* FileName: watchdogtimer.h
* Description:
* This header file contains the prototypes for Watchdog module.
* Watchdog Base0 is used with Interrupt
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
 *  	0.1D	07/04/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

#ifndef __WATCHDOGTIMER_H__
#define __WATCHDOGTIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 *  Macro definitions:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global variables:
****************************************************************************/
extern volatile bool g_bFeedWatchdog;
/****************************************************************************/


/******************************************************************************
 * Function Prototypes
 ********************************************************************************/
void initWatchdog(void);
void WatchdogIntHandler(void);
void doWatchdogReset(void);
/********************************************************************************/

#endif /*__WATCHDOGTIMER_H__*/
