/*********************************************************************************
* FileName: rtc.c
* Description:
* This source file contains the definitions for RTC functionality.
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

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "inc/hw_types.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/hibernate.h"
#include "utils/ustdlib.h"
#include "rtc.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/

/****************************************************************************
 *  The following are defines for the Hibernation module register addresses
 *  for RTC functionality.
****************************************************************************/

#define HIB_CTL         	0x400FC010  // Hibernation Control
#define HIB_RTCC        	0x400FC000  // Hibernation RTC Counter
#define HIB_RTCM0       	0x400FC004  // Hibernation RTC Match 0
#define HIB_RTCLD       	0x400FC00C  // Hibernation RTC Load
#define HIB_RTCT        	0x400FC024  // Hibernation RTC Trim
#define HIB_RTCSS       	0x400FC028  // Hibernation RTC Sub Seconds
#define HIB_CTL_CLK32EN     0x00000040  // Clocking Enable

/****************************************************************************
 * The following are defines for the bit fields in the HIB_CTL register.
*****************************************************************************/

#define HIB_CTL_WRC     	0x80000000  // Write Complete/Capable
#define HIB_CTL_RTCEN   	0x00000001  // RTC Timer Enable
/****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
****************************************************************************/
uint8_t RTCEnabled = 0;
/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
****************************************************************************/

/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
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
void RTCEnable(void)
{
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
	//
	// Turn on the clock enable bit.
	//
	HWREG(HIB_CTL) |= HIB_CTL_CLK32EN;

	//
	// Wait for write completion
	//
	while(!(HWREG(HIB_CTL) & HIB_CTL_WRC))
	{
	}

	//
	// Turn on the RTC enable bit.
	//
	HWREG(HIB_CTL) |= HIB_CTL_RTCEN;

	//
	// Wait for write completion
	//
	while(!(HWREG(HIB_CTL) & HIB_CTL_WRC))
	{
	}

	//
	// Set RTCEnabled flag
	//
	RTCEnabled = 1;
}
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
void initRTC(void)
{
	 //
	// Enable access to  the hibernate peripheral.  If the hibernate peripheral
	// was already running then this will have no effect.
	//
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);

    //
    // Enable the Hibernate module to run.
    //
    HibernateEnableExpClk(SysCtlClockGet());

    //
    // The hibernate peripheral trim register must be set per silicon
    // erratum 2.1
    //
    HibernateRTCTrimSet(0x7FFF);

    //
    // Enable RTC
    //
    RTCEnable();
}


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
void RTCDisable(void)
{
	//
	// Turn off the RTC enable bit.
	//
	HWREG(HIB_CTL) &= ~HIB_CTL_RTCEN;

	//
	// Wait for write completion
	//
	while(!(HWREG(HIB_CTL) & HIB_CTL_WRC))
	{
	}

	//
	// clear RTCEnabled flag
	//
	RTCEnabled = 0;
}
/********************************************************************************/

/******************************************************************************
 * FunctionName: RTCGet
 *
 * Function Description:
 * Gets the value of the real time clock (RTC)
 *
 * Function Parameters:
 * lDateTime	:	pointer to tm type object. Date and time values.
 *
 * Function Returns:
 * 0	:	Success
 * 1	:	RTC is disabled
 *
 ********************************************************************************/
uint8_t RTCGet(struct tm *lDateTime)
{
	uint32_t countRTC;

	//
	// Check whether RTC is enabled or not.
	//
	if(0 == RTCEnabled)
		return 1;

	//
	// Get the value of the RTC counter register
	//
	countRTC = (HWREG(HIB_RTCC));

	//
	// Convert RTC counter value to date and time format
	//
	ulocaltime((time_t)countRTC, lDateTime);

	return 0;
}
/********************************************************************************/

/******************************************************************************
 * FunctionName: RTCSet
 *
 * Function Description:
 * Sets the value of the real time clock (RTC)
 *
 * Function Parameters:
 * lDateTime		: pointer to tm type object. Date and time values.
 *
 * Function Returns	:
 * 0	:	Success
 * 1	:	RTC is disabled
 *
 ********************************************************************************/
uint8_t RTCSet(struct tm *lDateTime)
{
	uint32_t valRTC;

	//
	// Check whether RTC is enabled or not.
	//
	if(0 == RTCEnabled)
		return 1;

	//
	// Convert date and time value to integer value
	//
	valRTC = umktime(lDateTime);

	//
	// Write above obtained integer value to RTC load register.
	//
	HWREG(HIB_RTCLD) = valRTC;

	return 0;
}
/********************************************************************************/
