/*********************************************************************************
* FileName: sensorsdebounce.c
* Description:
* This header file contains definitions for Sensor Debounce module mechanism.
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
 *  	0.1D	17/06/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom_map.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/sysctl.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>
#include "sensorsdebounce.h"
#include "Drivers/systicktimer.h"
#include "Middleware/serial.h"
#include "Middleware/system.h"
#include "Drivers/extern.h"

#include "Application/intertaskcommunication.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define VARIABLE_DEBOUNCE_LOGIC

unsigned int suiTimeStampForOnePBS;
// ****************************************************************
// CHANGE FOLLOWING PARAMETERS TO ADJUST "DEBOUNCE VALUES" ........
// ****************************************************************
// Parameter for Debounce Period, 1msec * value
// RANGE : 1 - 50
#define PIN_SAFETY_DEBOUNCE_ACTIVE_EVENT  				20
#define PIN_SAFETY_DEBOUNCE_INACTIVE_EVENT 				20

#define PIN_INTERLOCKIP_DEBOUNCE_ACTIVE_EVENT  			20
#define PIN_INTERLOCKIP_DEBOUNCE_INACTIVE_EVENT 		20

#define PIN_OBSTACLE_DEBOUNCE_ACTIVE_EVENT  			20
#define PIN_OBSTACLE_DEBOUNCE_INACTIVE_EVENT 			20

#define PIN_1PBS_DEBOUNCE_ACTIVE_EVENT  				20
#define PIN_1PBS_DEBOUNCE_INACTIVE_EVENT 				20

#define PIN_WIRELESS_1PBS_DEBOUNCE_ACTIVE_EVENT  		20
#define PIN_WIRELESS_1PBS_DEBOUNCE_INACTIVE_EVENT		20

#define PIN_WIRELESS_MONITOR_DEBOUNCE_ACTIVE_EVENT		20
#define PIN_WIRELESS_MONITOR_DEBOUNCE_INACTIVE_EVENT	20

// Parameter for Max Temporary Release, 1msec * value
// RANGE : 0 - 3
#define	TEMPORARY_RELEASE_MAX_LIMIT						0


#define DEBOUNCE_COUNTER_BIT_COUNT						10

//*************************************************************************

/****************************************************************************
 *  Constant definitions
****************************************************************************/
#ifdef VARIABLE_DEBOUNCE_LOGIC

// Index of the sensor to be consider for snow mode from below constant array 'cucSensorPressDebounceLimit'
#define SENSOR_FOR_SNOW_MODE_INDEX						5

const uint8_t cucSensorPressDebounceLimit[32] =
{
PIN_WIRELESS_MONITOR_DEBOUNCE_ACTIVE_EVENT,
PIN_WIRELESS_1PBS_DEBOUNCE_ACTIVE_EVENT,
PIN_1PBS_DEBOUNCE_ACTIVE_EVENT,
PIN_OBSTACLE_DEBOUNCE_ACTIVE_EVENT,
PIN_INTERLOCKIP_DEBOUNCE_ACTIVE_EVENT,
PIN_SAFETY_DEBOUNCE_ACTIVE_EVENT,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5
};

const uint8_t cucSensorReleaseDebounceLimit[32] =
{
PIN_WIRELESS_MONITOR_DEBOUNCE_INACTIVE_EVENT,
PIN_WIRELESS_1PBS_DEBOUNCE_INACTIVE_EVENT,
PIN_1PBS_DEBOUNCE_INACTIVE_EVENT,
PIN_OBSTACLE_DEBOUNCE_INACTIVE_EVENT,
PIN_INTERLOCKIP_DEBOUNCE_INACTIVE_EVENT,
PIN_SAFETY_DEBOUNCE_INACTIVE_EVENT,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5,
5
};

const uint8_t cucSensorTempReleaseMaxLimit = TEMPORARY_RELEASE_MAX_LIMIT; // 0 - 3

#endif

// *************************************************************************


/****************************************************************************
 *  Global variables
****************************************************************************/
_SENSOR_STATUS gSensorStatus;
static uint8_t g_ui8SensorStates;
/****************************************************************************/

/****************************************************************************
 *  Function prototypes for this file:
****************************************************************************/
void checkInterLockStateAtPowerOn(uint8_t *rawData);
uint32_t get_timego(uint32_t x_data_his);

/****************************************************************************
 *  Function prototype from other file:
****************************************************************************/
#ifdef VARIABLE_DEBOUNCE_LOGIC
extern uint32_t ValidateDebounce(uint32_t luintDelta,uint32_t luintLastState,uint8_t *lpCounter,uint8_t *lpPressedDebounceLimit,uint8_t *lpReleasedDebounceLimit, uint8_t lucIndexSnowModeSensor);

extern void CheckTempReleaseAndResetClock (uint8_t cucSensorTempReleaseMaxLimit, uint8_t *lpCounter,uint32_t ui32Delta);

#endif

/****************************************************************************
 *  Function definitions for this file:
****************************************************************************/
extern uint8_t isPowerOfTwo(uint8_t x);


/******************************************************************************
 * FunctionName: sensorsPoll
 *
 * Function Description:
 * (This function taken from buttons.c file available in Tiva Drivers folder)
 * Polls the current state of the buttons and determines which have changed.

//!
//! This function should be called periodically by the application to poll the
//! pushbuttons.  It determines both the current debounced state of the buttons
//! and also which buttons have changed state since the last time the function
//! was called.
//!
//! In order for button debouncing to work properly, this function should be
//! caled at a regular interval, even if the state of the buttons is not needed
//! that often.
//!
//! If button debouncing is not required, the the caller can pass a pointer
//! for the \e pui8RawState parameter in order to get the raw state of the
//! buttons.  The value returned in \e pui8RawState will be a bit mask where
//! a 1 indicates the buttons is pressed.
 *
 * Function Parameters:
 * pui8Delta:			points to a character that will be written to indicate
 *						which button states changed since the last time this
 *						function was called. This value is derived from the
 *						debounced state of the buttons.
 * pui8RawState: 		points to a location where the raw button state will
 * 						be stored.
 *
 * Function Returns:
 * Returns the current debounced state of the buttons where a 1 in the button
 * ID's position indicates that the button is pressed and a 0 indicates that
 * it is released.
 *
 ********************************************************************************/
void sensorsPoll(uint8_t *pui8Delta, uint8_t *pui8RawState)
{
    uint32_t ui32Delta;
    uint32_t ui32Data;

#ifndef VARIABLE_DEBOUNCE_LOGIC
    static uint8_t ui8SwitchClockA = 0;
    static uint8_t ui8SwitchClockB = 0;
#endif

#ifdef VARIABLE_DEBOUNCE_LOGIC
    static uint8_t ui8SwitchClock[DEBOUNCE_COUNTER_BIT_COUNT] = {0,0,0,0,0,0,0,0,0,0}; // 0 = MSB ....9 = LSB
#endif

    //uint8_t temp;

#ifdef VERSION_1HARDWARE
    //
    // Read the raw state of the push buttons.  Save the raw state
    // (inverting the bit sense) if the caller supplied storage for the
    // raw value.
    //
    ui32Data = ((MAP_GPIOPinRead(GPIOD_BASE, PIN_SAFETY_PD2) << 3) & PIN_SAFETY) |
    		((MAP_GPIOPinRead(GPIOE_BASE, PIN_INTERLOCKIP_PE3) << 1) & PIN_INTERLOCKIP) |
    		((MAP_GPIOPinRead(GPIOD_BASE, PIN_OBSTACLE_PD1) << 2) & PIN_OBSTACLE) |
    		((MAP_GPIOPinRead(GPIOD_BASE, PIN_1PBS_PD3) >> 1) & PIN_1PBS) |
    		((MAP_GPIOPinRead(GPIOD_BASE, PIN_MULTIFUNCIP2_PD0) << 1) & PIN_MULTIFUNCIP2) |
    		((MAP_GPIOPinRead(GPIOE_BASE, PIN_MULTIFUNCIP1_PE5) >> 5) & PIN_MULTIFUNCIP1);
#endif

#ifdef VERSION_2HARDWARE
    //
    // Read the raw state of the push buttons.  Save the raw state
    // (inverting the bit sense) if the caller supplied storage for the
    // raw value.
    //
    ui32Data = 	((MAP_GPIOPinRead(GPIOD_BASE, PIN_SAFETY_PD2/*PIN_SAFETY_PD1*/) << 3/*4*/) & PIN_SAFETY) |						//MSB
				((MAP_GPIOPinRead(GPIOC_BASE, PIN_INTERLOCKIP_PC7) >> 3) & PIN_INTERLOCKIP) |
				((MAP_GPIOPinRead(GPIOD_BASE, PIN_OBSTACLE_PD1/*PIN_OBSTACLE_PD2*/) << 2/*1*/) & PIN_OBSTACLE) |
				((MAP_GPIOPinRead(GPIOB_BASE, PIN_1PBS_PB5) >> 3) & PIN_1PBS) |
	   	   	   	((MAP_GPIOPinRead(GPIOA_BASE, PIN_WIRELESS_1PBS_PA3) >> 2) & PIN_WIRELESS_1PBS) |
	   	   	   	((MAP_GPIOPinRead(GPIOA_BASE, PIN_WIRELESS_MONITOR_PA4) >> 4) & PIN_WIRELESS_MONITOR);	//LSB
/*
    uartSendTxBuffer(UART_debug,"B",1);
    temp = (uint8_t) ui32Data;
    uartSendTxBuffer(UART_debug,&temp,1);
*/

#endif


    if(((ui32Data&PIN_OBSTACLE)==0)&&
        			//(

      									//gstDriveStatus.bits.shutterLowerLimit == 0		 &&
       								//	((gstDriveStatus.bits.shutterMovingDown == 1||gstDriveStatus.bits.shutterUpperLimit==1) ||
       								//		(gstDriveStatus.bits.shutterStopped && gstDriveStatus.bits.shutterLowerLimit == 0 &&
       								//			gstDriveStatus.bits.shutterUpperLimit == 0 ))
        							//)&&
        			 (gstControlBoardStatus.bits.autoManual == 1)
        		   )
    	{
    		   gstControlApplicationFault.bits.ObstacleSensor = 1;
    		   gstControlBoardFault.bits.controlApplication = 1;
    		 		gstControlBoardStatus.bits.controlFault = 1;
    		}
    		else
    		{
    			gstControlApplicationFault.bits.ObstacleSensor = 0;
    			if(gstControlApplicationFault.val == 0)
    					{
    						gstControlBoardFault.bits.controlApplication = 0;
    					}
    					if(gstControlBoardFault.val == 0)
    					{
    						gstControlBoardStatus.bits.controlFault = 0;
    					}
    		}

    /*    union ui32TempData lun32Data;
    uint8_t lui8Pending = 0;
    lun32Data.word.val = ui32Data;

    uartSendTxBuffer(UART_debug,lun32Data.byte,4);
    do
    {
    	uartCheckFreeTxBuffer(UART_debug,&lui8Pending);
    }while(lui8Pending);*/

    if(pui8RawState)
    {
        *pui8RawState = (uint8_t)~ui32Data;
    }

    //
    //	Check whether Interlock input signal is active after power on
    //
    //checkInterLockStateAtPowerOn(pui8RawState);

    //
    // Determine the switches that are at a different state than the debounced
    // state.
    //
    ui32Delta = ui32Data ^ g_ui8SensorStates;

    //
    // Increment the clocks by one.
    //
#ifndef VARIABLE_DEBOUNCE_LOGIC
    ui8SwitchClockA ^= ui8SwitchClockB;
    ui8SwitchClockB = ~ui8SwitchClockB;
#endif

#ifdef VARIABLE_DEBOUNCE_LOGIC
    ui8SwitchClock[0] ^= (ui8SwitchClock[9] & ui8SwitchClock[8] & ui8SwitchClock[7] & ui8SwitchClock[6] & ui8SwitchClock[5] & ui8SwitchClock[4] & ui8SwitchClock[3] & ui8SwitchClock[2] & ui8SwitchClock[1]);
    ui8SwitchClock[1] ^= (ui8SwitchClock[9] & ui8SwitchClock[8] & ui8SwitchClock[7] & ui8SwitchClock[6] & ui8SwitchClock[5] & ui8SwitchClock[4] & ui8SwitchClock[3] & ui8SwitchClock[2]);
    ui8SwitchClock[2] ^= (ui8SwitchClock[9] & ui8SwitchClock[8] & ui8SwitchClock[7] & ui8SwitchClock[6] & ui8SwitchClock[5] & ui8SwitchClock[4] & ui8SwitchClock[3]);
    ui8SwitchClock[3] ^= (ui8SwitchClock[9] & ui8SwitchClock[8] & ui8SwitchClock[7] & ui8SwitchClock[6] & ui8SwitchClock[5] & ui8SwitchClock[4]);
    ui8SwitchClock[4] ^= (ui8SwitchClock[9] & ui8SwitchClock[8] & ui8SwitchClock[7] & ui8SwitchClock[6] & ui8SwitchClock[5]);
    ui8SwitchClock[5] ^= (ui8SwitchClock[9] & ui8SwitchClock[8] & ui8SwitchClock[7] & ui8SwitchClock[6]);
    ui8SwitchClock[6] ^= (ui8SwitchClock[9] & ui8SwitchClock[8] & ui8SwitchClock[7]);
    ui8SwitchClock[7] ^= (ui8SwitchClock[9] & ui8SwitchClock[8]);
    ui8SwitchClock[8] ^= ui8SwitchClock[9];
    ui8SwitchClock[9] = ~ui8SwitchClock[9];
#endif


    //
    // Reset the clocks corresponding to switches that have not changed state.
    //
#ifndef VARIABLE_DEBOUNCE_LOGIC
    ui8SwitchClockA &= ui32Delta;
    ui8SwitchClockB &= ui32Delta;
#endif

#ifdef VARIABLE_DEBOUNCE_LOGIC
    CheckTempReleaseAndResetClock (cucSensorTempReleaseMaxLimit, (uint8_t *) ui8SwitchClock,ui32Delta);
#endif

#ifndef VARIABLE_DEBOUNCE_LOGIC
    //
    // Get the new debounced switch state.
    //
    g_ui8SensorStates &= ui8SwitchClockA | ui8SwitchClockB;
    g_ui8SensorStates |= (~(ui8SwitchClockA | ui8SwitchClockB)) & ui32Data;

    //
    // Determine the switches that just changed debounced state.
    //
    ui32Delta ^= (ui8SwitchClockA | ui8SwitchClockB);
#endif

#ifdef VARIABLE_DEBOUNCE_LOGIC
    //
    // Get the new debounced switch state.
    // Determine the switches that just changed debounced state.
    //
    ui32Delta =  ValidateDebounce(ui32Delta,g_ui8SensorStates,(uint8_t *) ui8SwitchClock,(uint8_t *)cucSensorPressDebounceLimit,(uint8_t *)cucSensorReleaseDebounceLimit,SENSOR_FOR_SNOW_MODE_INDEX);
    g_ui8SensorStates = g_ui8SensorStates & (~ui32Delta);
    g_ui8SensorStates = g_ui8SensorStates | (ui32Data & ui32Delta);
#endif

    //
    // Store the bit mask for the buttons that have changed for return to
    // caller.
    //
    if(pui8Delta)
    {
        *pui8Delta = (uint8_t)ui32Delta;
    }

    //
    // Send the debounced buttons states to the caller.  Invert the bit
    // sense so that a '1' indicates the button is pressed, which is a
    // sensible way to interpret the return value.
    //
    sensorsProcessFlags(~g_ui8SensorStates, *pui8Delta);

#if 0
    //	Added on 05 Dec 2014 as per new requirement from client
    uint8_t lui8Wireless1PBS = 0;
    uint8_t lui8WirelessMonitor = 0;
    uint8_t lui8InterlockInput = 0;

    lui8Wireless1PBS = MAP_GPIOPinRead(GPIOA_BASE, PIN_WIRELESS_1PBS_PA3);
    lui8WirelessMonitor = MAP_GPIOPinRead(GPIOA_BASE, PIN_WIRELESS_MONITOR_PA4);
    lui8InterlockInput = MAP_GPIOPinRead(GPIOC_BASE, PIN_INTERLOCKIP_PC7);

    if(lui8Wireless1PBS)
    {
    	gstDriveStatusMenu.bits.Wireless_1PBS_Status = 1;
    }
    else
    {
    	gstDriveStatusMenu.bits.Wireless_1PBS_Status = 0;
    }

    if(lui8WirelessMonitor)
    {
    	gstControlBoardStatus.bits.wirelessMonitor = 1;
    }
    else
    {
    	gstControlBoardStatus.bits.wirelessMonitor = 0;
    }

	if(lui8InterlockInput)
	{
		gstDriveStatusMenu.bits.Interlock_Input_Status = 1;
	}
	else
	{
		gstDriveStatusMenu.bits.Interlock_Input_Status = 0;
	}

#endif

}

/******************************************************************************
 * Function Name: checkInterLockStateAtPowerOn
 *
 * Function Description:
 *
 * Function Parameters: uint8_t *rawSensorState
 * 						Raw sensor state
 *
 * Function Returns: void
 *
 ********************************************************************************/
uint32_t get_timego10ms(uint32_t x_data_his);
void checkInterLockStateAtPowerOn(uint8_t *rawSensorState)
{
    //
    //	Check whether Interlock input signal is active after power on
    //
    static uint8_t lui8PowerOn = 0;

    static uint32_t lui32CaptrueTime = 0;

    if(
    		((*rawSensorState) & PIN_INTERLOCKIP) &&
    		(lui8PowerOn == 0)
    )
    {
    	lui8PowerOn = 1;
    	lui32CaptrueTime = g_ui32TickCount10ms;
    }

    if(
    		((*rawSensorState) & PIN_INTERLOCKIP) &&
    		(get_timego10ms( lui32CaptrueTime) >= 3) &&
    		(lui8PowerOn == 1)
    )
    {
    	lui8PowerOn = 2;
    	gSensorStatus.bits.Sensor_InterlockIP_active = true;

    	//uartSendTxBuffer(UART_debug,"A",1);
    }
}

/******************************************************************************
 * Function Name: initKeys
 *
 * Function Description: This function must be called during application
 * initialization to configure the GPIO pins to which the pushbuttons are
 * attached.  It enables the port used by the buttons and configures each
 * button GPIO as an input.
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void initSensors(void)
{
    //
    // Enable the GPIO port to which the Sensors are connected.
    //

	MAP_SysCtlPeripheralEnable(GPIOD_PERIPH);
	MAP_SysCtlPeripheralEnable(GPIOE_PERIPH);

	//Added for Debug. Remove it later
	/*MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_3);
	ROM_GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_3, 0);*/

//    MAP_GPIODirModeSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_DIR_MODE_IN);
//    MAP_GPIOPadConfigSet(BUTTONS_GPIO_BASE, ALL_BUTTONS,
//                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	HWREG(GPIOD_BASE + GPIO_O_LOCK) = 0x4C4F434B;
	HWREG(GPIOD_BASE + GPIO_O_CR) = 0x80;
	HWREG(GPIOD_BASE + GPIO_O_LOCK) = 45;
	ROM_GPIOPinTypeGPIOInput(GPIOD_BASE, ALL_SENSORS_PD);

#ifdef VERSION_1HARDWARE
	ROM_GPIOPinTypeGPIOInput(GPIOE_BASE, ALL_SENSORS_PE);
#endif

#ifdef VERSION_2HARDWARE
	//	Added on 05 Dec 2014 as per new requirement from client
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	ROM_GPIOPinTypeGPIOInput(GPIOA_BASE,ALL_SENSORS_PA);

	//	PA3 pull up configuration
	MAP_GPIOPadConfigSet(GPIOA_BASE, PIN_WIRELESS_1PBS_PA3,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
	MAP_GPIOPadConfigSet(GPIOA_BASE, PIN_WIRELESS_MONITOR_PA4,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	ROM_GPIOPinTypeGPIOInput(GPIOC_BASE,PIN_INTERLOCKIP_PC7);

	//	PA3 pull up configuration
	MAP_GPIOPadConfigSet(GPIOC_BASE, PIN_INTERLOCKIP_PC7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


	// 1PBS CONFIGURATION...................................................................................
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	ROM_GPIOPinTypeGPIOInput(GPIOA_BASE/*GPIOB_BASE*/,PIN_1PBS_PB5);
	MAP_GPIOPadConfigSet(GPIOB_BASE, PIN_1PBS_PB5,GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


#endif

#ifdef VERSION_1HARDWARE
    //
    // Initialize the debounced button state with the current state read from
    // the GPIO bank.
	//	Commented on 05 Dec 2014 to incorporate new requirement from client and added the code below.
    /*g_ui8SensorStates = ((MAP_GPIOPinRead(GPIOD_BASE, PIN_SAFETY_PD2) << 3) & PIN_SAFETY) |
    		((MAP_GPIOPinRead(GPIOE_BASE, PIN_INTERLOCKIP_PE3) << 1) & PIN_INTERLOCKIP) |
    		((MAP_GPIOPinRead(GPIOD_BASE, PIN_OBSTACLE_PD1) << 2) & PIN_OBSTACLE) |
    		((MAP_GPIOPinRead(GPIOD_BASE, PIN_1PBS_PD3) >> 1) & PIN_1PBS) |
    		((MAP_GPIOPinRead(GPIOD_BASE, PIN_MULTIFUNCIP2_PD0) << 1) & PIN_MULTIFUNCIP2) |
    		((MAP_GPIOPinRead(GPIOE_BASE, PIN_MULTIFUNCIP1_PE5) >> 5) & PIN_MULTIFUNCIP1);*/
#endif

#ifdef VERSION_2HARDWARE
	//	Added on 05 Dec 2014 as per new requirement from client
	g_ui8SensorStates = ((MAP_GPIOPinRead(GPIOD_BASE, PIN_SAFETY_PD2/*PIN_SAFETY_PD1*/) << 3/*4*/) & PIN_SAFETY) |
						((MAP_GPIOPinRead(GPIOC_BASE, PIN_INTERLOCKIP_PC7) >> 3) & PIN_INTERLOCKIP) |
						((MAP_GPIOPinRead(GPIOD_BASE, PIN_OBSTACLE_PD1/*PIN_OBSTACLE_PD2*/) << 2/*1*/) & PIN_OBSTACLE) |
						(~(MAP_GPIOPinRead(GPIOB_BASE, PIN_1PBS_PB5) >> 3) & PIN_1PBS) |
 		   	   	   	   	((MAP_GPIOPinRead(GPIOA_BASE, PIN_WIRELESS_1PBS_PA3) >> 2) & PIN_WIRELESS_1PBS) |
 		   	   	   	   	((MAP_GPIOPinRead(GPIOA_BASE, PIN_WIRELESS_MONITOR_PA4) >> 4) & PIN_WIRELESS_MONITOR);
#endif

	//	Do not consider current level of inter-lock and startup-safety signals at power-on.
	//	This is to make sure that if these signals are active during/before power-on then system shall process them.
	//	Added on Jan 2016.
	//	Obstacle sensor and 1PBS shall be processed if they are active during/before power on - added on Feb 2016
	g_ui8SensorStates |= 0x3C;

    gSensorStatus.Val = 0;
}

/******************************************************************************
 * Function Name: keysProcessFlags
 *
 * Function Description: This function must be called during application
 * initialization to configure the GPIO pins to which the pushbuttons are
 * attached.  It enables the port used by the buttons and configures each
 * button GPIO as an input.
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void sensorsProcessFlags(uint8_t sensorState, uint8_t sensorChanged)
{

#ifdef VERSION_1HARDWARE
	uint8_t activeState = sensorState & ~(0xC0);
#endif

	static unsigned int suiTimeStmpWirelesMonReleased;
	static unsigned char sucWirelesMonReleasedFlag = 0;
	#define	WIRELESS_MONITOR_LATCH_TIME			  1000

	/********** General Sensor Flags *********/

//5	//Safety Sensor active
//	if(isPowerOfTwo(activeState))
	/*{
		if(SENSOR_ACTIVE(PIN_SAFETY, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_Safety_active)
			{
				gSensorStatus.bits.Sensor_Safety_active = true;
				gSensorStatus.bits.Sensor_Safety_inactive = false;

				// Set drive status menu parameter
				gstDriveStatusMenu.bits.Startup_Safety_Status = 1;

			}
		}
	}
	//---Safety Sensor inactive
//	if(!activeState)
		if(SENSOR_INACTIVE(PIN_SAFETY, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_Safety_inactive)
			{
				gSensorStatus.bits.Sensor_Safety_inactive = true;
				gSensorStatus.bits.Sensor_Safety_active = false;

				// Reset drive status menu parameter
				gstDriveStatusMenu.bits.Startup_Safety_Status = 0;

			}
		}*/

//4	//Interlock Input Sensor active
//	if(isPowerOfTwo(activeState))
	{
		if(SENSOR_ACTIVE(PIN_INTERLOCKIP, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_InterlockIP_active)
			{
				gSensorStatus.bits.Sensor_InterlockIP_active = true;
				gSensorStatus.bits.Sensor_InterlockIP_inactive = false;

				// Set drive status menu parameter
				gstDriveStatusMenu.bits.Interlock_Input_Status = 1;

				uartSendTxBuffer(UART_debug,"+",1);
			}
		}
	}
	//---Interlock Input Sensor inactive
//	if(!activeState)
		if(SENSOR_INACTIVE(PIN_INTERLOCKIP, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_InterlockIP_inactive)
			{
				gSensorStatus.bits.Sensor_InterlockIP_inactive = true;
				gSensorStatus.bits.Sensor_InterlockIP_active = false;

				// Set drive status menu parameter
				gstDriveStatusMenu.bits.Interlock_Input_Status = 0;

				uartSendTxBuffer(UART_debug,"/",1);

			}
		}

//3	//Obstacle Sensor active
//	if(isPowerOfTwo(activeState))
	{
		if(SENSOR_ACTIVE(PIN_OBSTACLE, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_Obstacle_active)
			{
				suiTimeStampForOnePBS = g_ui32TickCount;  //bug_No.88
				gSensorStatus.bits.Sensor_Obstacle_active = true;
				gSensorStatus.bits.Sensor_Obstacle_inactive = false;

				// Set drive status menu parameter
				gstDriveStatusMenu.bits.Startup_Status = 1;

			}
		}
	}
	//---Obstacle Sensor inactive
//	if(!activeState)
		if(SENSOR_INACTIVE(PIN_OBSTACLE, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_Obstacle_inactive)
			{
				gSensorStatus.bits.Sensor_Obstacle_inactive = true;
				gSensorStatus.bits.Sensor_Obstacle_active = false;

				// Set drive status menu parameter
				gstDriveStatusMenu.bits.Startup_Status = 0;

			}
		}

//2	//1PBS Sensor active
//	if(isPowerOfTwo(activeState))
	{
		if(SENSOR_ACTIVE(PIN_1PBS, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_1PBS_active)
			{
				gSensorStatus.bits.Sensor_1PBS_active = true;
				gSensorStatus.bits.Sensor_1PBS_inactive = false;
				suiTimeStampForOnePBS = g_ui32TickCount;
				if(OnePBSEnableStatus == false)
				{
					OnePBSEnabled = true;
					OnePBSEnableStatus = true;
				}

				gstDriveStatusMenu.bits.One_PBS_Status = 1;
			}
		}
	}
	//---1PBS Sensor inactive
//	if(!activeState)
		if(SENSOR_INACTIVE(PIN_1PBS, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_1PBS_inactive)
			{
				gSensorStatus.bits.Sensor_1PBS_inactive = true;
				gSensorStatus.bits.Sensor_1PBS_active = false;

				gstDriveStatusMenu.bits.One_PBS_Status = 0;
			}
		}
#ifdef VERSION_1HARDWARE
//1	//MultiFucn IP2 Sensor active
//	if(isPowerOfTwo(activeState))
	{
		if(SENSOR_ACTIVE(PIN_MULTIFUNCIP2, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_MultifuncIP2_active)
			{
				gSensorStatus.bits.Sensor_MultifuncIP2_active = true;
				gSensorStatus.bits.Sensor_MultifuncIP2_inactive = false;

				// Set drive status menu parameter
				gstDriveStatusMenu.bits.Multi_Fun_Input2_Status = 1;

			}
		}
	}
	//---MultiFucn IP2 Sensor inactive
//	if(!activeState)
		if(SENSOR_INACTIVE(PIN_MULTIFUNCIP2, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_MultifuncIP2_inactive)
			{
				gSensorStatus.bits.Sensor_MultifuncIP2_inactive = true;
				gSensorStatus.bits.Sensor_MultifuncIP2_active = false;

				// Set drive status menu parameter
				gstDriveStatusMenu.bits.Multi_Fun_Input2_Status = 0;

			}
		}

//0	//MultiFucn IP1 Sensor active-----------
//	if(isPowerOfTwo(activeState))
	{
		if(SENSOR_ACTIVE(PIN_MULTIFUNCIP1, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_MultifuncIP1_active)
			{
				gSensorStatus.bits.Sensor_MultifuncIP1_active = true;
				gSensorStatus.bits.Sensor_MultifuncIP1_inactive = false;

				// Set drive status menu parameter
				gstDriveStatusMenu.bits.Multi_Fun_Input1_Status = 1;

			}
		}
	}
	//---MultiFucn IP1 Sensor inactive-----------
//	if(!activeState)
		if(SENSOR_INACTIVE(PIN_MULTIFUNCIP1, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_MultifuncIP1_inactive)
			{
				gSensorStatus.bits.Sensor_MultifuncIP1_inactive = true;
				gSensorStatus.bits.Sensor_MultifuncIP1_active = false;

				// Set drive status menu parameter
				gstDriveStatusMenu.bits.Multi_Fun_Input1_Status = 0;

			}
		}
#endif //	VERSION_1HARDWARE

#if 0
		//---Wireless 1PBS Sensor active-----------
		//	if(isPowerOfTwo(activeState))
		{
			if(SENSOR_ACTIVE(PIN_WIRELESS_1PBS, sensorState, sensorChanged))
			{
				if(!gSensorStatus.bits.Sensor_Wireless_1PBS_active)
				{
					gSensorStatus.bits.Sensor_Wireless_1PBS_active = true;
					gSensorStatus.bits.Sensor_Wireless_1PBS_inactive = false;

					// Set drive status menu parameter
					gstDriveStatusMenu.bits.Wireless_1PBS_Status = 1;

				}
			}
		}

		//---Wireless 1PBS Sensor inactive-----------
		//	if(!activeState)
		if(SENSOR_INACTIVE(PIN_WIRELESS_1PBS, sensorState, sensorChanged))
		{
			if(!gSensorStatus.bits.Sensor_Wireless_1PBS_inactive)
			{
				gSensorStatus.bits.Sensor_Wireless_1PBS_inactive = true;
				gSensorStatus.bits.Sensor_Wireless_1PBS_active = false;

				// Set drive status menu parameter
				gstDriveStatusMenu.bits.Wireless_1PBS_Status = 0;

			}
		}
#endif

		//---Wireless 1PBS Sensor active-----------
		//	if(isPowerOfTwo(activeState))
		{
			if(SENSOR_ACTIVE(PIN_WIRELESS_MONITOR, sensorState, sensorChanged))
			{
				//if(!gSensorStatus.bits.Sensor_Wireless_1PBS_active)
				{
					// Set control status
					gstControlBoardStatus.bits.wirelessMonitor = 1;

					sucWirelesMonReleasedFlag = 0;

				}
			}
		}
		//---Wireless 1PBS Sensor inactive-----------
		//	if(!activeState)
		if(SENSOR_INACTIVE(PIN_WIRELESS_MONITOR, sensorState, sensorChanged))
		{
			//if(!gSensorStatus.bits.Sensor_Wireless_1PBS_inactive)
			{

				// Reset control status
				//gstControlBoardStatus.bits.wirelessMonitor = 0;

				sucWirelesMonReleasedFlag = 1;
				suiTimeStmpWirelesMonReleased = g_ui32TickCount;

			}
		}

		if (1 == sucWirelesMonReleasedFlag)
		{

			if (get_timego(suiTimeStmpWirelesMonReleased) > WIRELESS_MONITOR_LATCH_TIME)
			{

				gstControlBoardStatus.bits.wirelessMonitor = 0;
				sucWirelesMonReleasedFlag = 0;

			}

		}

	/********** End of General Button Flags *********/

//	//If Key is released reset currentTickCount
//	if(gSensorStatus.byte.HB_Released)
//		currentTickCount = 0;

}

///******************************************************************************
// * Function Name: isPowerOfTwo
// *
// * Function Description: This function checks whether byte is a power of 2
// *
// * Function Parameters: x - byte to detect
// *
// * Function Returns: 1 if input is power of two else returns 0
// *
// ********************************************************************************/
//uint8_t isPowerOfTwo(uint8_t x)
//{
//	uint8_t temp = x - 1;
//	temp &= x;
//	temp = !temp;
//	if(temp && x)
//		return 1;
//	else
//		return 0;
//}

