/*********************************************************************************
 * FileName: ledhandler.c
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
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "Middleware/paramdatabase.h"
#include "Middleware/serial.h"
#include "userinterface.h"
#include "intertaskcommunication.h"
#include "ledhandler.h"
/****************************************************************************/


/****************************************************************************
 *  Macro definitions:
****************************************************************************/

// LED GPIO Pins
#define POWER_ON_LED		(GPIO_PIN_4)
#define FAULT_LED			(GPIO_PIN_1)
#define AUTO_MANUAL_LED		(GPIO_PIN_0)

/****************************************************************************/


/****************************************************************************
 *  Global variables for this file:
****************************************************************************/

/****************************************************************************/

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
void blinkLED(uint8_t lui8LED, uint8_t lui8LEDStatus, uint8_t lui8PreviousLEDStatus)
{
	static uint32_t lsTickCountsPowerOn = 0;
	static uint32_t lsTickCountsFault = 0;
	static uint32_t lsTickCountsAutoManual = 0;

	static uint8_t lsDelayPowerOn = 0;
	static uint8_t lsDelayFault = 0;
	static uint8_t lsDelayAutoManual = 0;

	static uint32_t lui32PowerOnLEDState = 0;
	static uint32_t lui32FaultLEDState = 0;
	static uint32_t lui32AutoManualLEDState = 0;


	if(lui8PreviousLEDStatus != lui8LEDStatus)
	{
		//
		// Reset led states
		//
		lui32PowerOnLEDState = 0;
		lui32FaultLEDState = 0;
		lui32AutoManualLEDState = 0;
#if 1
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, POWER_ON_LED, lui32PowerOnLEDState);
		ROM_GPIOPinWrite(GPIO_PORTE_BASE, FAULT_LED, lui32FaultLEDState);
		ROM_GPIOPinWrite(GPIO_PORTE_BASE, AUTO_MANUAL_LED, lui32AutoManualLEDState);
#endif

		//
		// Capture time
		//
		if(lui8LED == AUTO_MANUAL_LED)
		{
			lsTickCountsAutoManual = g_ui32TickCount;
		}
		if(lui8LED == FAULT_LED)
		{
			lsTickCountsFault = g_ui32TickCount;
		}
		if(lui8LED == POWER_ON_LED)
		{
			lsTickCountsPowerOn = g_ui32TickCount;
		}

#if 1	// To ensure LED synchronization in case all LEDs have same blink rate
		if( /*(gstLEDcontrolRegister.autoManualLED == gstLEDcontrolRegister.faultLED) &&
			(gstLEDcontrolRegister.faultLED == gstLEDcontrolRegister.powerLED)*/
			(gstLEDcontrolRegister.autoManualLED == gstLEDcontrolRegister.powerLED)
		)
		{
			lsTickCountsAutoManual = g_ui32TickCount;
			lsTickCountsFault = g_ui32TickCount;
			lsTickCountsPowerOn = g_ui32TickCount;
		}
#endif

		switch(lui8LEDStatus)
		{
			case _BLINK_STATUS_50_MSEC:	// 0.05 Seconds
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsDelayAutoManual = 5;
				else if(lui8LED == FAULT_LED)
					lsDelayFault = 5;
				else if(lui8LED == POWER_ON_LED)
					lsDelayPowerOn = 5;

				break;
			}

			case _BLINK_STATUS_100_MSEC:	// 0.1 Seconds
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsDelayAutoManual = 10;
				else if(lui8LED == FAULT_LED)
					lsDelayFault = 10;
				else if(lui8LED == POWER_ON_LED)
					lsDelayPowerOn = 10;

				break;
			}

			case _BLINK_STATUS_150_MSEC: 	// 0.15 Seconds
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsDelayAutoManual = 15;
				else if(lui8LED == FAULT_LED)
					lsDelayFault = 15;
				else if(lui8LED == POWER_ON_LED)
					lsDelayPowerOn = 15;

				break;
			}

			case _BLINK_STATUS_200_MSEC: 	// 0.2 Seconds
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsDelayAutoManual = 20;
				else if(lui8LED == FAULT_LED)
					lsDelayFault = 20;
				else if(lui8LED == POWER_ON_LED)
					lsDelayPowerOn = 20;

				break;
			}

			case _BLINK_STATUS_250_MSEC: 	// 0.25 Seconds
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsDelayAutoManual = 25;
				else if(lui8LED == FAULT_LED)
					lsDelayFault = 25;
				else if(lui8LED == POWER_ON_LED)
					lsDelayPowerOn = 25;

				break;
			}

			case _BLINK_STATUS_300_MSEC: 	// 0.3 Seconds
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsDelayAutoManual = 30;
				else if(lui8LED == FAULT_LED)
					lsDelayFault = 30;
				else if(lui8LED == POWER_ON_LED)
					lsDelayPowerOn = 30;

				break;
			}

			case _BLINK_STATUS_350_MSEC: 	// 0.35 Seconds
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsDelayAutoManual = 35;
				else if(lui8LED == FAULT_LED)
					lsDelayFault = 35;
				else if(lui8LED == POWER_ON_LED)
					lsDelayPowerOn = 35;

				break;
			}

			case _BLINK_STATUS_400_MSEC: 	// 0.4 Seconds
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsDelayAutoManual = 40;
				else if(lui8LED == FAULT_LED)
					lsDelayFault = 40;
				else if(lui8LED == POWER_ON_LED)
					lsDelayPowerOn = 40;

				break;
			}

			case _BLINK_STATUS_450_MSEC: 	// 0.45 Seconds
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsDelayAutoManual = 45;
				else if(lui8LED == FAULT_LED)
					lsDelayFault = 45;
				else if(lui8LED == POWER_ON_LED)
					lsDelayPowerOn = 45;

				break;
			}

			case _BLINK_STATUS_500_MSEC:	// 0.5 Seconds
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsDelayAutoManual = 50;
				else if(lui8LED == FAULT_LED)
					lsDelayFault = 50;
				else if(lui8LED == POWER_ON_LED)
					lsDelayPowerOn = 50;

				break;
			}
		}
	}

	//
	// Power On LED delay generation
	//
	if( (get_timego( lsTickCountsPowerOn) >= lsDelayPowerOn) &&
		(0 != lsDelayPowerOn) &&
		(lui8LED == POWER_ON_LED)
	  )
	{
		//
		// Capture time
		//
		lsTickCountsPowerOn = g_ui32TickCount;

		//
		// Toggle LED
		//
		lui32PowerOnLEDState ^= POWER_ON_LED;
		ROM_GPIOPinWrite(GPIO_PORTF_BASE, POWER_ON_LED, lui32PowerOnLEDState);
	}

	//
	// Fault LED delay generation
	//
	if( (get_timego( lsTickCountsFault) >= lsDelayFault) &&
		(0 != lsDelayFault) &&
		(lui8LED == FAULT_LED)
	  )
	{
		//
		// Capture time
		//
		lsTickCountsFault = g_ui32TickCount;

		//
		// Toggle LED
		//
		lui32FaultLEDState ^= FAULT_LED;
		ROM_GPIOPinWrite(GPIO_PORTE_BASE, FAULT_LED, lui32FaultLEDState);
	}

	//
	// Auto Manual LED delay generation
	//
	if( (get_timego( lsTickCountsAutoManual) >= lsDelayAutoManual) &&
		(0 != lsDelayAutoManual) &&
		(lui8LED == AUTO_MANUAL_LED)
	  )
	{


		//
		// Capture time
		//
		lsTickCountsAutoManual = g_ui32TickCount;

		//
		// Toggle LED
		//
		lui32AutoManualLEDState ^= AUTO_MANUAL_LED;
		ROM_GPIOPinWrite(GPIO_PORTE_BASE, AUTO_MANUAL_LED, lui32AutoManualLEDState);
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
void flashLED(uint8_t lui8LED, uint8_t lui8LEDStatus, uint8_t lui8PreviousLEDStatus)
{
	static uint32_t lsTickCountsPowerOn = 0;
	static uint32_t lsTickCountsFault = 0;
	static uint32_t lsTickCountsAutoManual = 0;

	static uint8_t lsPowerOnFlashCount = 0;
	static uint8_t lsFaultFlashCount = 0;
	static uint8_t lsAutoManualFlashCount = 0;

	if(lui8PreviousLEDStatus != lui8LEDStatus)
	{
		//
		// Capture time
		//
		if(lui8LED == AUTO_MANUAL_LED)
		{
			lsTickCountsAutoManual = g_ui32TickCount;
			ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, 0); //OFF
		}
		else if(lui8LED == FAULT_LED)
		{
			lsTickCountsFault = g_ui32TickCount;
			ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, 0); //OFF
		}
		else if(lui8LED == POWER_ON_LED)
		{
			lsTickCountsPowerOn = g_ui32TickCount;
			ROM_GPIOPinWrite(GPIO_PORTF_BASE, lui8LED, 0); //OFF
		}

		switch(lui8LEDStatus)
		{
			case _FLASH_STATUS_COUNT_1:	// 1 Flash
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsAutoManualFlashCount = 1;
				else if(lui8LED == FAULT_LED)
					lsFaultFlashCount = 1;
				else if(lui8LED == POWER_ON_LED)
					lsPowerOnFlashCount = 1;

				break;
			}

			case _FLASH_STATUS_COUNT_2:	// 2 Flash
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsAutoManualFlashCount = 2;
				else if(lui8LED == FAULT_LED)
					lsFaultFlashCount = 2;
				else if(lui8LED == POWER_ON_LED)
					lsPowerOnFlashCount = 2;

				break;
			}

			case _FLASH_STATUS_COUNT_3: 	// 3 Flash
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsAutoManualFlashCount = 3;
				else if(lui8LED == FAULT_LED)
					lsFaultFlashCount = 3;
				else if(lui8LED == POWER_ON_LED)
					lsPowerOnFlashCount = 3;

				break;
			}

			case _FLASH_STATUS_COUNT_4: 	// 4 Flash
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsAutoManualFlashCount = 4;
				else if(lui8LED == FAULT_LED)
					lsFaultFlashCount = 4;
				else if(lui8LED == POWER_ON_LED)
					lsPowerOnFlashCount = 4;

				break;
			}

			case _FLASH_STATUS_COUNT_5: 	// 5 Flash
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsAutoManualFlashCount = 5;
				else if(lui8LED == FAULT_LED)
					lsFaultFlashCount = 5;
				else if(lui8LED == POWER_ON_LED)
					lsPowerOnFlashCount = 5;

				break;
			}

			case _FLASH_STATUS_COUNT_6: 	// 6 Flash
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsAutoManualFlashCount = 6;
				else if(lui8LED == FAULT_LED)
					lsFaultFlashCount = 6;
				else if(lui8LED == POWER_ON_LED)
					lsPowerOnFlashCount = 6;

				break;
			}

			case _FLASH_STATUS_COUNT_7: 	// 7 Flash
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsAutoManualFlashCount = 7;
				else if(lui8LED == FAULT_LED)
					lsFaultFlashCount = 7;
				else if(lui8LED == POWER_ON_LED)
					lsPowerOnFlashCount = 7;

				break;
			}

			case _FLASH_STATUS_COUNT_8: 	// 8 Flash
			{
				if(lui8LED == AUTO_MANUAL_LED)
					lsAutoManualFlashCount = 8;
				else if(lui8LED == FAULT_LED)
					lsFaultFlashCount = 8;
				else if(lui8LED == POWER_ON_LED)
					lsPowerOnFlashCount = 8;

				break;
			}
		}
	}

	//
	// Power on LED flash generation
	//
	if( (lui8LED == POWER_ON_LED)  && (lsPowerOnFlashCount > 0) )
	{
		static uint8_t lsFlashStatePowerOn = _3_SEC_OFF_1;

		switch(lsFlashStatePowerOn)
		{
			case _3_SEC_OFF_1:
			{
				if( get_timego( lsTickCountsPowerOn) >= 300 )
				{
					ROM_GPIOPinWrite(GPIO_PORTF_BASE, lui8LED, lui8LED); //ON
					lsTickCountsPowerOn = g_ui32TickCount;
					lsFlashStatePowerOn = _500_MSEC_ON_1;
				}
				break;
			}

			case _500_MSEC_ON_1:
			{
				if( get_timego( lsTickCountsPowerOn) >= 50 )
				{
					ROM_GPIOPinWrite(GPIO_PORTF_BASE, lui8LED, 0); //OFF
					lsTickCountsPowerOn = g_ui32TickCount;
					lsFlashStatePowerOn = _500_MSEC_OFF;
				}
				break;
			}

			case _500_MSEC_OFF:
			{
				if( get_timego( lsTickCountsPowerOn) >= 50 )
				{
					ROM_GPIOPinWrite(GPIO_PORTF_BASE, lui8LED, lui8LED); //ON
					lsTickCountsPowerOn = g_ui32TickCount;
					lsFlashStatePowerOn = _500_MSEC_ON_2;
				}
				break;
			}

			case _500_MSEC_ON_2:
			{
				if( get_timego( lsTickCountsPowerOn) >= 50 )
				{
					ROM_GPIOPinWrite(GPIO_PORTF_BASE, lui8LED, 0); //OFF
					lsTickCountsPowerOn = g_ui32TickCount;
					lsFlashStatePowerOn = _3_SEC_OFF_2;
				}
				break;
			}

			case _3_SEC_OFF_2:
			{
				if( get_timego( lsTickCountsPowerOn) >= 300 )
				{
					lsPowerOnFlashCount--;
					if(lsPowerOnFlashCount > 0)
						lsTickCountsPowerOn = g_ui32TickCount;
					lsFlashStatePowerOn = _3_SEC_OFF_1;

				}
				break;
			}

		}
	}

	//
	// Fault LED flash generation
	//
	if( (lui8LED == FAULT_LED) && (lsFaultFlashCount > 0) )
	{
		static uint8_t lsFlashStateFault = _3_SEC_OFF_1;

		switch(lsFlashStateFault)
		{
			case _3_SEC_OFF_1:
			{
				if( get_timego( lsTickCountsFault) >= 300 )
				{
					ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, lui8LED); //ON
					lsTickCountsFault = g_ui32TickCount;
					lsFlashStateFault = _500_MSEC_ON_1;
				}
				break;
			}

			case _500_MSEC_ON_1:
			{
				if( get_timego( lsTickCountsFault) >= 50 )
				{
					ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, 0); //OFF
					lsTickCountsFault = g_ui32TickCount;
					lsFlashStateFault = _500_MSEC_OFF;
				}
				break;
			}

			case _500_MSEC_OFF:
			{
				if( get_timego( lsTickCountsFault) >= 50 )
				{
					ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, lui8LED); //ON
					lsTickCountsFault = g_ui32TickCount;
					lsFlashStateFault = _500_MSEC_ON_2;
				}
				break;
			}

			case _500_MSEC_ON_2:
			{
				if( get_timego(lsTickCountsFault) >= 50 )
				{
					ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, 0); //OFF
					lsTickCountsFault = g_ui32TickCount;
					lsFlashStateFault = _3_SEC_OFF_2;
				}
				break;
			}

			case _3_SEC_OFF_2:
			{
				if( get_timego( lsTickCountsFault) >= 300 )
				{
					lsFaultFlashCount--;
					if(lsFaultFlashCount > 0)
						lsTickCountsFault = g_ui32TickCount;
					lsFlashStateFault = _3_SEC_OFF_1;

				}
				break;
			}

		}
	}

	//
	// Auto Manual LED flash generation
	//
	if( (lui8LED == AUTO_MANUAL_LED) && (lsAutoManualFlashCount > 0) )
	{
		static uint8_t lsFlashStateAutoManual = _3_SEC_OFF_1;

		switch(lsFlashStateAutoManual)
		{
			case _3_SEC_OFF_1:
			{
				if( get_timego( lsTickCountsAutoManual) >= 300 )
				{
					ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, lui8LED); //ON
					lsTickCountsAutoManual = g_ui32TickCount;
					lsFlashStateAutoManual = _500_MSEC_ON_1;
				}
				break;
			}

			case _500_MSEC_ON_1:
			{
				if( get_timego( lsTickCountsAutoManual) >= 50 )
				{
					ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, 0); //OFF
					lsTickCountsAutoManual = g_ui32TickCount;
					lsFlashStateAutoManual = _500_MSEC_OFF;
				}
				break;
			}

			case _500_MSEC_OFF:
			{
				if( get_timego(lsTickCountsAutoManual) >= 50 )
				{
					ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, lui8LED); //ON
					lsTickCountsAutoManual = g_ui32TickCount;
					lsFlashStateAutoManual = _500_MSEC_ON_2;
				}
				break;
			}

			case _500_MSEC_ON_2:
			{
				if( get_timego( lsTickCountsAutoManual) >= 50 )
				{
					ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, 0); //OFF
					lsTickCountsAutoManual = g_ui32TickCount;
					lsFlashStateAutoManual = _3_SEC_OFF_2;
				}
				break;
			}

			case _3_SEC_OFF_2:
			{
				if( get_timego( lsTickCountsAutoManual) >= 300 )
				{
					lsAutoManualFlashCount--;
					if(lsAutoManualFlashCount > 0)
						lsTickCountsAutoManual = g_ui32TickCount;
					lsFlashStateAutoManual = _3_SEC_OFF_1;

				}
				break;
			}

		}
	}
}

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
void checkLEDStatus(uint8_t lui8LED, uint8_t lui8LEDStatus, uint8_t lui8PreviousLEDStatus)
{

	switch(lui8LEDStatus & 0x0F)
	{
		case LED_OFF:	// LED Off
		{
			if(lui8LEDStatus != lui8PreviousLEDStatus)
			{

				if(lui8LED == POWER_ON_LED)
					ROM_GPIOPinWrite(GPIO_PORTF_BASE, lui8LED, 0); //OFF
				else
					ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, 0); //OFF
			}

			break;
		}

		case LED_ON:	// LED On
		{
			if(lui8LEDStatus != lui8PreviousLEDStatus)
			{

				if(lui8LED == POWER_ON_LED)
					ROM_GPIOPinWrite(GPIO_PORTF_BASE, lui8LED, lui8LED); //ON
				else
					ROM_GPIOPinWrite(GPIO_PORTE_BASE, lui8LED, lui8LED); //ON
			}

			break;
		}

		case LED_FLASH:	// LED Flash
		{
			flashLED(lui8LED, lui8LEDStatus, lui8PreviousLEDStatus);
			break;
		}

		case LED_BLINK:	// LED Blink
		{
			blinkLED(lui8LED, lui8LEDStatus, lui8PreviousLEDStatus);
			break;
		}
	}
}


void LED_AOTUMAU_TOGGLE(void)
{
	if(ROM_GPIOPinRead(GPIO_PORTE_BASE, AUTO_MANUAL_LED))
	{
        ROM_GPIOPinWrite(GPIO_PORTE_BASE, AUTO_MANUAL_LED,0);
	}
	else
	{
	    ROM_GPIOPinWrite(GPIO_PORTE_BASE, AUTO_MANUAL_LED,AUTO_MANUAL_LED);
	}
	
}

void LED_AOTUMAU_ON(void)
{
	  ROM_GPIOPinWrite(GPIO_PORTE_BASE, AUTO_MANUAL_LED,AUTO_MANUAL_LED);
}
void LED_AOTUMAU_OFF(void)
{
	  ROM_GPIOPinWrite(GPIO_PORTE_BASE, AUTO_MANUAL_LED,0);
}


/******************************************************************************
 * FunctionName: displayBoardLEDHandler
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
void displayBoardLEDHandler(void)
{
	uint8_t lui8PowerLEDStatus = 0, lui8FaultLEDStatus = 0, lui8AutoManualLEDStatus = 0;
	static uint8_t lui8SavedStatusPowerOnLED = 0, lui8SavedStatusFaultLED = 0, lui8SavedStatusAutoManualLED = 0;

	//
	// Get current LED status
	//
	lui8PowerLEDStatus = gstLEDcontrolRegister.powerLED;
	lui8FaultLEDStatus = gstLEDcontrolRegister.faultLED;
	lui8AutoManualLEDStatus = gstLEDcontrolRegister.autoManualLED;

	//
	// Check current status of LEDs
	//
	checkLEDStatus(POWER_ON_LED, lui8PowerLEDStatus, lui8SavedStatusPowerOnLED);
	checkLEDStatus(FAULT_LED, lui8FaultLEDStatus, lui8SavedStatusFaultLED);
	checkLEDStatus(AUTO_MANUAL_LED, lui8AutoManualLEDStatus, lui8SavedStatusAutoManualLED);

	//
	// Save LEDs status
	//
	lui8SavedStatusPowerOnLED = lui8PowerLEDStatus;
	lui8SavedStatusFaultLED = lui8FaultLEDStatus;
	lui8SavedStatusAutoManualLED = lui8AutoManualLEDStatus;
}

