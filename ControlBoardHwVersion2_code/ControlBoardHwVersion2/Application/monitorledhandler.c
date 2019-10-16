/*********************************************************************************
 * FileName: monitorledhandler.c
 * Description:
 * Version: 0.1D
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
 *  	0.1D	05/08/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>
#include "Drivers/systicktimer.h"
#include "Middleware/debounce.h"
#include "Middleware/paramdatabase.h"
#include "intertaskcommunication.h"
#include "monitorledhandler.h"
/****************************************************************************/


/****************************************************************************
 *  Macro definitions:
****************************************************************************/

// LED GPIO Pins
//#define MONITOR_LED	(GPIO_PIN_4)	// on Port B

#define MONITOR_LED_OFF	MONITOR_LED_LOW	// on Port B
#define MONITOR_LED_ON	MONITOR_LED_HIGH

/****************************************************************************/


/****************************************************************************
 *  Global variables for this file:
****************************************************************************/

/****************************************************************************/
uint32_t get_timego(uint32_t x_data_his);
/******************************************************************************
 * FunctionName: blinkLED
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
void blinkLED(uint8_t lui8LEDStatus, uint8_t lui8PreviousLEDStatus)
{
	static uint32_t lsTickCounts = 0;
	static uint32_t lsDelay = 0;
	static uint8_t lui8MonitorLEDState = 0;


	if(lui8PreviousLEDStatus != lui8LEDStatus)
	{
		//
		// Capture time
		//
		lsTickCounts = g_ui32TickCount;

		switch(lui8LEDStatus)
		{
			case _BLINK_STATUS_50_MSEC:	// 0.05 Seconds
			{
				lsDelay = 50;
				break;
			}

			case _BLINK_STATUS_100_MSEC:	// 0.1 Seconds
			{
				lsDelay = 100;
				break;
			}

			case _BLINK_STATUS_150_MSEC: 	// 0.15 Seconds
			{
				lsDelay = 150;
				break;
			}

			case _BLINK_STATUS_200_MSEC: 	// 0.2 Seconds
			{
				lsDelay = 200;
				break;
			}

			case _BLINK_STATUS_250_MSEC: 	// 0.25 Seconds
			{
				lsDelay = 250;
				break;
			}

			case _BLINK_STATUS_300_MSEC: 	// 0.3 Seconds
			{
				lsDelay = 300;
				break;
			}

			case _BLINK_STATUS_350_MSEC: 	// 0.35 Seconds
			{
				lsDelay = 350;
				break;
			}

			case _BLINK_STATUS_400_MSEC: 	// 0.4 Seconds
			{
				lsDelay = 400;
				break;
			}

			case _BLINK_STATUS_450_MSEC: 	// 0.45 Seconds
			{
				lsDelay = 450;
				break;
			}

			case _BLINK_STATUS_500_MSEC:	// 0.5 Seconds
			{
				lsDelay = 500;
				break;
			}
		}
	}

	//
	// Power On LED delay generation
	//
	if( (get_timego(g_ui32TickCount ) >= lsDelay) &&
		(0 != lsDelay)
	  )
	{
		//
		// Capture time
		//
		lsTickCounts = g_ui32TickCount;

		//
		// Toggle LED
		//
		lui8MonitorLEDState ^= MONITOR_LED;
		ROM_GPIOPinWrite(GPIO_PORTB_BASE, MONITOR_LED, lui8MonitorLEDState);
	}

}

/******************************************************************************
 * FunctionName: flashLED
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
void flashLED(uint8_t lui8LEDStatus, uint8_t lui8PreviousLEDStatus)
{
	static uint32_t lsTickCounts = 0;
	static uint8_t lsMonitorLEDFlashCount = 0;
	static uint8_t lsFlashStateMonitorLED = _3_SEC_OFF_HEAD;
	static uint8_t lsCounter = 0;

	if(lui8PreviousLEDStatus != lui8LEDStatus)
	{
		//
		// Capture time
		//
		lsTickCounts = g_ui32TickCount;
		ROM_GPIOPinWrite(GPIO_PORTB_BASE, MONITOR_LED, MONITOR_LED_OFF); //OFF
		lsFlashStateMonitorLED = _3_SEC_OFF_HEAD;

		switch(lui8LEDStatus)
		{
			case _FLASH_STATUS_COUNT_1:	// 1 Flash
			{
				lsMonitorLEDFlashCount = 1;
				break;
			}

			case _FLASH_STATUS_COUNT_2:	// 2 Flash
			{
				lsMonitorLEDFlashCount = 2;
				break;
			}

			case _FLASH_STATUS_COUNT_3: 	// 3 Flash
			{
				lsMonitorLEDFlashCount = 3;
				break;
			}

			case _FLASH_STATUS_COUNT_4: 	// 4 Flash
			{
				lsMonitorLEDFlashCount = 4;
				break;
			}

			case _FLASH_STATUS_COUNT_5: 	// 5 Flash
			{
				lsMonitorLEDFlashCount = 5;
				break;
			}

			case _FLASH_STATUS_COUNT_6: 	// 6 Flash
			{
				lsMonitorLEDFlashCount = 6;
				break;
			}

			case _FLASH_STATUS_COUNT_7: 	// 7 Flash
			{
				lsMonitorLEDFlashCount = 7;
				break;
			}

			case _FLASH_STATUS_COUNT_8: 	// 8 Flash
			{
				lsMonitorLEDFlashCount = 8;
				break;
			}
		}
	}

	//
	// Monitor LED flash generation
	//
	if(lsMonitorLEDFlashCount > 0)
	{
		switch(lsFlashStateMonitorLED)
		{
			case _3_SEC_OFF_HEAD :
			{
				if( get_timego( lsTickCounts) >= 3000 )
				{
					ROM_GPIOPinWrite(GPIO_PORTB_BASE, MONITOR_LED, MONITOR_LED_ON); //ON
					lsTickCounts = g_ui32TickCount;
					lsCounter = ( 2 * lsMonitorLEDFlashCount ) - 1;
					lsFlashStateMonitorLED = _FLASH_LED_500_MSEC;
				}

				break;
			}

			case _FLASH_LED_500_MSEC :
			{
				static uint32_t lsMonitorLEDState = 0;

				if(lsCounter > 0)
				{
					if( get_timego( lsTickCounts) >= 500 )
					{
						lsMonitorLEDState ^= MONITOR_LED;
						ROM_GPIOPinWrite(GPIO_PORTB_BASE, MONITOR_LED, lsMonitorLEDState);
						lsTickCounts = g_ui32TickCount;
						lsCounter--;
					}
				}
				else
				{
					lsMonitorLEDState = 0;
					lsFlashStateMonitorLED = _3_SEC_OFF_TAIL;
#ifndef _6_SEC_OFF
					lsFlashStateMonitorLED = _3_SEC_OFF_HEAD;
#endif
				}

				break;
			}

			case _3_SEC_OFF_TAIL :
			{
				if(get_timego ( lsTickCounts) >= 3000 )
				{
					lsTickCounts = g_ui32TickCount;
					lsFlashStateMonitorLED = _3_SEC_OFF_HEAD;
				}

				break;
			}
		}
	}
}

#if 0
void flashLED(uint8_t lui8LEDStatus, uint8_t lui8PreviousLEDStatus)
{
	static uint32_t lsTickCounts = 0;
	static uint8_t lsMonitorLEDFlashCount = 0;

	if(lui8PreviousLEDStatus != lui8LEDStatus)
	{
		//
		// Capture time
		//
		lsTickCounts = g_ui32TickCount;
		ROM_GPIOPinWrite(GPIO_PORTB_BASE, MONITOR_LED, 0); //OFF

		switch(lui8LEDStatus)
		{
			case _FLASH_STATUS_COUNT_1:	// 1 Flash
			{
				lsMonitorLEDFlashCount = 1;
				break;
			}

			case _FLASH_STATUS_COUNT_2:	// 2 Flash
			{
				lsMonitorLEDFlashCount = 2;
				break;
			}

			case _FLASH_STATUS_COUNT_3: 	// 3 Flash
			{
				lsMonitorLEDFlashCount = 3;
				break;
			}

			case _FLASH_STATUS_COUNT_4: 	// 4 Flash
			{
				lsMonitorLEDFlashCount = 4;
				break;
			}

			case _FLASH_STATUS_COUNT_5: 	// 5 Flash
			{
				lsMonitorLEDFlashCount = 5;
				break;
			}

			case _FLASH_STATUS_COUNT_6: 	// 6 Flash
			{
				lsMonitorLEDFlashCount = 6;
				break;
			}

			case _FLASH_STATUS_COUNT_7: 	// 7 Flash
			{
				lsMonitorLEDFlashCount = 7;
				break;
			}

			case _FLASH_STATUS_COUNT_8: 	// 8 Flash
			{
				lsMonitorLEDFlashCount = 8;
				break;
			}
		}
	}

	//
	// Monitor LED flash generation
	//
	if(lsMonitorLEDFlashCount > 0)
	{
		static uint8_t lsFlashStateMonitorLED = _3_SEC_OFF_1;

		switch(lsFlashStateMonitorLED)
		{
			case _3_SEC_OFF_1:
			{
				if( (g_ui32TickCount - lsTickCounts) >= 3000 )
				{
					ROM_GPIOPinWrite(GPIO_PORTB_BASE, MONITOR_LED, MONITOR_LED_ON); //ON
					lsTickCounts = g_ui32TickCount;
					lsFlashStateMonitorLED = _500_MSEC_ON_1;
				}
				break;
			}

			case _500_MSEC_ON_1:
			{
				if( (g_ui32TickCount - lsTickCounts) >= 500 )
				{
					ROM_GPIOPinWrite(GPIO_PORTB_BASE, MONITOR_LED, MONITOR_LED_OFF); //OFF
					lsTickCounts = g_ui32TickCount;
					lsFlashStateMonitorLED = _500_MSEC_OFF;
				}
				break;
			}

			case _500_MSEC_OFF:
			{
				if( (g_ui32TickCount - lsTickCounts) >= 500 )
				{
					ROM_GPIOPinWrite(GPIO_PORTB_BASE, MONITOR_LED, MONITOR_LED_ON); //ON
					lsTickCounts = g_ui32TickCount;
					lsFlashStateMonitorLED = _500_MSEC_ON_2;
				}
				break;
			}

			case _500_MSEC_ON_2:
			{
				if( (g_ui32TickCount - lsTickCounts) >= 500 )
				{
					ROM_GPIOPinWrite(GPIO_PORTB_BASE, MONITOR_LED, MONITOR_LED_OFF); //OFF
					lsTickCounts = g_ui32TickCount;
					lsFlashStateMonitorLED = _3_SEC_OFF_2;
				}
				break;
			}

			case _3_SEC_OFF_2:
			{
				if( (g_ui32TickCount - lsTickCounts) >= 3000 )
				{
					lsMonitorLEDFlashCount--;
					if(lsMonitorLEDFlashCount > 0)
						lsTickCounts = g_ui32TickCount;
					lsFlashStateMonitorLED = _3_SEC_OFF_1;

				}
				break;
			}

		}
	}
}
#endif

/******************************************************************************
 * FunctionName: checkLEDStatus
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
void checkLEDStatus(uint8_t lui8LEDStatus, uint8_t lui8PreviousLEDStatus)
{
	switch(lui8LEDStatus & 0x0F)
	{
		case LED_OFF:	// LED Off
		{
			ROM_GPIOPinWrite(GPIO_PORTB_BASE, MONITOR_LED, MONITOR_LED_OFF); //OFF
			break;
		}

		case LED_ON:	// LED On
		{
			ROM_GPIOPinWrite(GPIO_PORTB_BASE, MONITOR_LED, MONITOR_LED_ON); //ON
			break;
		}

		case LED_FLASH:	// LED Flash
		{
			flashLED(lui8LEDStatus, lui8PreviousLEDStatus);
			break;
		}

		case LED_BLINK:	// LED Blink
		{
			blinkLED(lui8LEDStatus, lui8PreviousLEDStatus);
			break;
		}
	}
}

/******************************************************************************
 * FunctionName: monitorLEDHandler
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
void monitorLEDHandler(void)
{
	uint8_t lui8MonitorLEDStatus = 0;
	static uint8_t lui8SavedStatusMonitorLED = 0;

	//
	// Get current monitor led status
	//
	lui8MonitorLEDStatus = gstMonitorLEDControlRegister.monitorLEDstatus;

	//
	// Check current monitor led status
	//
	checkLEDStatus(lui8MonitorLEDStatus, lui8SavedStatusMonitorLED);

	//
	// Save Monitor LED status
	//
	lui8SavedStatusMonitorLED = lui8MonitorLEDStatus;
}
