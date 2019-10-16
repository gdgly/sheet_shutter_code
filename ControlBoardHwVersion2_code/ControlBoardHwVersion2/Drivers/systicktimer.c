/*********************************************************************************
* FileName: systick.c
* Description:
* This source file contains SysTick module definitions
* Version: 0.2D
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
 *  	0.2D	16/05/2014			iGATE Offshore team		  1ms SysTick
 *  	0.1D	07/04/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/systick.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include "systicktimer.h"
/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
volatile uint32_t g_ui32TickCount, g_ui32TickCount10ms;
uint32_t g_ui32TickCount1sec = 0;
uint32_t ui32SysClock;
volatile uint32_t g_ui32LastTick = 0;

/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
****************************************************************************/

/******************************************************************************
 * FunctionName: SysTickIntHandler
 *
 * Function Description:
 * Handles the SysTick timeout interrupt.
 *
 * Function Parameters: None
 *
 * Function Returns: None
 *
 ********************************************************************************/
void SysTickIntHandler(void)
{
	g_ui32TickCount++;

	static uint8_t tcount;

	if(tcount++ > 10) {
		tcount = 0;
		// Tick Count - Used in Debounce routine
		g_ui32TickCount10ms++;
	}
}

/******************************************************************************
 * FunctionName: GetTickms
 *
 * Function Description:
 *  This function returns the number of ticks since the last time this function
 *  was called.
 *
 * Function Parameters: None
 *
 * Function Returns: the number of ticks
 *
 ********************************************************************************/
uint32_t GetTickms(void)
{
    uint32_t ui32RetVal, ui32Saved;

    ui32RetVal = g_ui32TickCount;
    ui32Saved = ui32RetVal;

    if(ui32Saved > g_ui32LastTick)
    {
        ui32RetVal = ui32Saved - g_ui32LastTick;
    }
    else
    {
        ui32RetVal = g_ui32LastTick - ui32Saved;
    }

    //
    // This could miss a few milliseconds but the timings here are on a
    // much larger scale.
    //
    g_ui32LastTick = ui32Saved;

    //
    // Return the number of milliseconds since the last time this was called.
    //
    return(ui32RetVal * MS_PER_SYSTICK);
}

/******************************************************************************
 * FunctionName: Init_SysTick
 *
 * Function Description:
 *  Initialization of SysTick module for 1ms INT.
 *
 * Function Parameters: None
 *
 * Function Returns: None
 *
 ********************************************************************************/
void Init_SysTick(void)
{
	ui32SysClock = MAP_SysCtlClockGet();
    // Configure SysTick to periodically interrupt.
    g_ui32TickCount = 0;
    g_ui32TickCount10ms = 0;
    MAP_SysTickPeriodSet(ui32SysClock / CLOCK_RATE);
    MAP_SysTickIntEnable();
    MAP_SysTickEnable();
}

