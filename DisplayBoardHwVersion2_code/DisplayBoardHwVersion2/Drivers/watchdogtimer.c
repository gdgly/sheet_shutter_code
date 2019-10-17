/*********************************************************************************
* FileName: watchdogtimer.c
* Description:
* This source file contains Watchdog module definitions.
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

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <gpio.h>
#include <pin_map.h>
#include <rom_map.h>
#include <rom.h>
#include <sysctl.h>
#include <inc/hw_ints.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <driverlib/watchdog.h>
#include "watchdogtimer.h"
#include "Middleware/uartstdio.h"
#include "Middleware/serial.h"
/****************************************************************************/


/****************************************************************************
 *  Macro definitions:
****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
****************************************************************************/


/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
// Flag to tell the watchdog interrupt handler whether or not to clear the
// interrupt (feed the watchdog).
volatile bool g_bFeedWatchdog = true;

/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
****************************************************************************/

/******************************************************************************
 * FunctionName: WatchdogIntHandler
 *
 * Function Description:
 * Handles the Watchdog timeout interrupt.
 *
 * Function Parameters: None
 *
 * Function Returns: None
 *
 ********************************************************************************/
void WatchdogIntHandler(void)
{
	ROM_WatchdogIntClear(WATCHDOG0_BASE);
	uartSendTxBuffer(UART_debug,"?",3);
	ROM_SysCtlReset();
//	while(1);
}

/******************************************************************************
 * FunctionName: doWatchdogReset
 *
 * Function Description:
 * Do Watchdog Reset if any module requires asserting the controller.
 *
 * Function Parameters: None
 *
 * Function Returns: None
 *
 ********************************************************************************/
void doWatchdogReset(void)
{
	ROM_WatchdogReloadSet(WATCHDOG0_BASE, 0xFFFFFFFF);
	//ROM_WatchdogReloadSet(WATCHDOG0_BASE, ROM_SysCtlClockGet()*5);
}

void doWatchdogReset_powerON(void)      //20170421  201703_No.39
{
	ROM_WatchdogReloadSet(WATCHDOG0_BASE, ROM_SysCtlClockGet());
}
/******************************************************************************
 * FunctionName: initWatchdog
 *
 * Function Description:
 *  Initialization of Watchdog module.
 *
 * Function Parameters: None
 *
 * Function Returns: None
 *
 ********************************************************************************/
void initWatchdog(void)
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);

	// Check to see if the registers are locked, and if so, unlock them.
	if(MAP_WatchdogLockState(WATCHDOG0_BASE) == true)
	{
	MAP_WatchdogUnlock(WATCHDOG0_BASE);
	}

    // Enable the watchdog interrupt.
    ROM_IntEnable(INT_WATCHDOG);

    // Set the period of the watchdog timer.
    ROM_WatchdogReloadSet(WATCHDOG0_BASE, ROM_SysCtlClockGet());

    // Enable reset generation from the watchdog timer.
    ROM_WatchdogResetEnable(WATCHDOG0_BASE);

    // Enable the watchdog timer.
    ROM_WatchdogEnable(WATCHDOG0_BASE);
}

