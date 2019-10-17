/*********************************************************************************
* FileName: debounce.c
* Description:
* This header file contains definitions for Key Debounce module mechanism.
* Version: 0.3D
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
 *  	0.3D	09/07/2014									Modifying change for new logic and port pin change
 *  	0.2D	19/06/2014									  (STOP + currentTickcount) issue solved.
 *  	0.1D	07/04/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom_map.h>
#include <driverlib/rom.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>
#include <inc/hw_ints.h>
#include "debounce.h"
#include "Drivers/systicktimer.h"

#include "Application/intertaskcommunication.h"
#include "Application/errormodule.h"
#include "Middleware/serial.h"
#include "Middleware/paramdatabase.h"
#include "Middleware/sensorsdebounce.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/

#define VARIABLE_DEBOUNCE_LOGIC
#define	KEY_PRIORITY_LOGIC_ENABLED

// ****************************************************************
// CHANGE FOLLOWING PARAMETERS TO ADJUST "DEBOUNCE VALUES" ........
// ****************************************************************
// Parameter for Debounce Period, 1msec * value
// RANGE : 1 - 50
#define OPEN_KEY_DEBOUNCE_ACTIVE_EVENT  				20
#define OPEN_KEY_DEBOUNCE_INACTIVE_EVENT 				20

#define CLOSE_KEY_DEBOUNCE_ACTIVE_EVENT  				20
#define CLOSE_KEY_DEBOUNCE_INACTIVE_EVENT 				20

#define STOP_KEY_DEBOUNCE_ACTIVE_EVENT  				20
#define STOP_KEY_DEBOUNCE_INACTIVE_EVENT 				20

#define WIRELESS_OPEN_KEY_DEBOUNCE_ACTIVE_EVENT  		20
#define WIRELESS_OPEN_KEY_DEBOUNCE_INACTIVE_EVENT 		20

#define WIRELESS_CLOSE_KEY_DEBOUNCE_ACTIVE_EVENT  		20
#define WIRELESS_CLOSE_KEY_DEBOUNCE_INACTIVE_EVENT		20

#define WIRELESS_STOP_KEY_DEBOUNCE_ACTIVE_EVENT			20
#define WIRELESS_STOP_KEY_DEBOUNCE_INACTIVE_EVENT		20

// Parameter for Max Temporary Release, 1msec * value
// RANGE : 0 - 3
#define	TEMPORARY_RELEASE_MAX_LIMIT						0


#define DEBOUNCE_COUNTER_BIT_COUNT						10

// *************************************************************************

/****************************************************************************
 *  Constant definitions
****************************************************************************/
#ifdef VARIABLE_DEBOUNCE_LOGIC
const uint8_t cucPressDebounceLimit[32] =
{
OPEN_KEY_DEBOUNCE_ACTIVE_EVENT,
CLOSE_KEY_DEBOUNCE_ACTIVE_EVENT,
STOP_KEY_DEBOUNCE_ACTIVE_EVENT,
WIRELESS_OPEN_KEY_DEBOUNCE_ACTIVE_EVENT,
WIRELESS_CLOSE_KEY_DEBOUNCE_ACTIVE_EVENT,
WIRELESS_STOP_KEY_DEBOUNCE_ACTIVE_EVENT,
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

const uint8_t cucReleaseDebounceLimit[32] =
{
OPEN_KEY_DEBOUNCE_INACTIVE_EVENT,
CLOSE_KEY_DEBOUNCE_INACTIVE_EVENT,
STOP_KEY_DEBOUNCE_INACTIVE_EVENT,
WIRELESS_OPEN_KEY_DEBOUNCE_INACTIVE_EVENT,
WIRELESS_CLOSE_KEY_DEBOUNCE_INACTIVE_EVENT,
WIRELESS_STOP_KEY_DEBOUNCE_INACTIVE_EVENT,
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

const uint8_t cucTempReleaseMaxLimit = TEMPORARY_RELEASE_MAX_LIMIT; // 0 - 3

#endif

// *************************************************************************


/****************************************************************************
 *  Global variables
****************************************************************************/
_KEYS_STATUS gKeysStatus;
uint8_t KEY_STOP_CYW=0;
static uint32_t currentTickCount = 0;
static uint8_t g_ui8ButtonStates = 0x00;

uint32_t get_timego(uint32_t x_data_his);

extern uint16_t gsActiveAnomalylist[20];
extern uint32_t his_rise_time;
extern const _stErrorList gstErrorList [TOTAL_ERRORS];

extern unsigned int suiTimeStampForOnePBS;
/****************************************************************************/

/****************************************************************************
 *  Function prototype for this file:
****************************************************************************/
#ifdef VARIABLE_DEBOUNCE_LOGIC
uint32_t ValidateDebounce(uint32_t luintDelta,uint32_t luintLastState,uint8_t *lpCounter,uint8_t *lpPressedDebounceLimit,uint8_t *lpReleasedDebounceLimit, uint8_t lucIndexSnowModeSensor);

void CheckTempReleaseAndResetClock (uint8_t cucTempReleaseMaxLimit, uint8_t *lpCounter,uint32_t ui32Delta);

#endif


/****************************************************************************
 *  Function definitions for this file:
****************************************************************************/
/******************************************************************************
 * FunctionName: keysPoll
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
 * Function Returns: None
 *
 ********************************************************************************/
void keysPoll(uint8_t *pui8Delta, uint8_t *pui8RawState)
{
    uint32_t ui32Delta;
    uint32_t ui32Data = 0;

#ifndef VARIABLE_DEBOUNCE_LOGIC
    static uint8_t ui8SwitchClockA = 0;
    static uint8_t ui8SwitchClockB = 0;
#endif

#ifdef VARIABLE_DEBOUNCE_LOGIC
    static uint8_t ui8SwitchClock[DEBOUNCE_COUNTER_BIT_COUNT] = {0,0,0,0,0,0,0,0,0,0}; // 0 = MSB ....9 = LSB
#endif

    uint8_t temp;

    //
    // Read the raw state of the push buttons.  Save the raw state
    // (inverting the bit sense) if the caller supplied storage for the
    // raw value.
    //
#ifdef VERSION_1HARDWARE
    ui32Data = 	((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PB7_3PBS_OPEN) >> 7) & KEY_3PBS_OPEN) |
    			((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PB6_3PBS_CLOSE) >> 5) & KEY_3PBS_CLOSE) |
    			(~(MAP_GPIOPinRead(BUTTONS2_GPIO_BASE, PF4_3PBS_STOP) >> 2) & KEY_3PBS_STOP);
#endif	//	VERSION_1HARDWARE

#ifdef VERSION_2HARDWARE

	#ifndef WIRELESS_3PBS_AS_VERSION_3HARDWARE

    ui32Data =	((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE3_3PBS_OPEN) >> 3) & KEY_3PBS_OPEN) 	|     							//LSB
				((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE2_3PBS_CLOSE) >> 1) & KEY_3PBS_CLOSE) 	|
				(~(MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE1_3PBS_STOP) << 1) & KEY_3PBS_STOP) 	|
				((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE_WIRELESS, PB6_3PBS_OPEN_WIRELESS) >> 3) & KEY_3PBS_OPEN_WIRELESS) 		|
    			((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE_WIRELESS, PB7_3PBS_CLOSE_WIRELESS) >> 3) & KEY_3PBS_CLOSE_WIRELESS) 	|
    			((MAP_GPIOPinRead(BUTTONS2_GPIO_BASE_WIRELESS, PF4_3PBS_STOP_WIRELESS) << 1) & KEY_3PBS_STOP_WIRELESS);			//MSB

    /*
    uartSendTxBuffer(UART_debug,"Z",1);
    temp = (uint8_t) ui32Data;
    uartSendTxBuffer(UART_debug,&temp,1);
    */

	#endif	//WIRELESS_3PBS_AS_VERSION_3HARDWARE

	#ifdef WIRELESS_3PBS_AS_VERSION_3HARDWARE

	ui32Data =	((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE3_3PBS_OPEN) >> 3) & KEY_3PBS_OPEN) 	|
				((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE2_3PBS_CLOSE) >> 1) & KEY_3PBS_CLOSE) 	|
				(~(MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE1_3PBS_STOP) << 1) & KEY_3PBS_STOP) 	|
				(~(MAP_GPIOPinRead(BUTTONS1_GPIO_BASE_WIRELESS, PB6_3PBS_OPEN_WIRELESS) >> 3) & KEY_3PBS_OPEN_WIRELESS) 		|
				(~(MAP_GPIOPinRead(BUTTONS1_GPIO_BASE_WIRELESS, PB7_3PBS_CLOSE_WIRELESS) >> 3) & KEY_3PBS_CLOSE_WIRELESS) 	|
				(~(MAP_GPIOPinRead(BUTTONS2_GPIO_BASE_WIRELESS, PF4_3PBS_STOP_WIRELESS) << 1) & KEY_3PBS_STOP_WIRELESS);

	/*
		uartSendTxBuffer(UART_debug,"Z",1);
		temp = (uint8_t) ui32Data;
		uartSendTxBuffer(UART_debug,&temp,1);
	*/

	#endif	//WIRELESS_3PBS_AS_VERSION_3HARDWARE

#endif	//	VERSION_2HARDWARE

    if(pui8RawState)
    {
        *pui8RawState = (uint8_t)~ui32Data;
    }

    //
    // Determine the switches that are at a different state than the debounced
    // state.
    //
    ui32Delta = ui32Data ^ g_ui8ButtonStates;

    //
    // Increment the clocks by one.
    //
#ifndef VARIABLE_DEBOUNCE_LOGIC
    ui8SwitchClockA ^= ui8SwitchClockB;
    ui8SwitchClockB = ~ui8SwitchClockB;
#endif

#ifdef VARIABLE_DEBOUNCE_LOGIC
    // 6 bit counter
    /*
    ui8SwitchClock[0] ^= (ui8SwitchClock[5] & ui8SwitchClock[4] & ui8SwitchClock[3] & ui8SwitchClock[2] & ui8SwitchClock[1]);
    ui8SwitchClock[1] ^= (ui8SwitchClock[5] & ui8SwitchClock[4] & ui8SwitchClock[3] & ui8SwitchClock[2]);
    ui8SwitchClock[2] ^= (ui8SwitchClock[5] & ui8SwitchClock[4] & ui8SwitchClock[3]);
    ui8SwitchClock[3] ^= (ui8SwitchClock[5] & ui8SwitchClock[4]);
    ui8SwitchClock[4] ^= ui8SwitchClock[5];
    ui8SwitchClock[5] = ~ui8SwitchClock[5];
    */

    // 10 bit counter
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
    CheckTempReleaseAndResetClock (cucTempReleaseMaxLimit, (uint8_t *) ui8SwitchClock,ui32Delta);
#endif

#ifndef VARIABLE_DEBOUNCE_LOGIC
    //
    // Get the new debounced switch state.
    //
    g_ui8ButtonStates &= ui8SwitchClockA | ui8SwitchClockB;
    g_ui8ButtonStates |= (~(ui8SwitchClockA | ui8SwitchClockB)) & ui32Data;

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
    ui32Delta =  ValidateDebounce(ui32Delta,g_ui8ButtonStates,(uint8_t *) ui8SwitchClock,(uint8_t *)cucPressDebounceLimit,(uint8_t *)cucReleaseDebounceLimit,0xFF);
    g_ui8ButtonStates = g_ui8ButtonStates & (~ui32Delta);
    g_ui8ButtonStates = g_ui8ButtonStates | (ui32Data & ui32Delta);
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
    // Send the debounced buttons states to the keysProcessFlags().
    // Invert the bit sense so that a '1' indicates the button is pressed,
    // which is a sensible way to interpret the return value.
    //

    keysProcessFlags(~g_ui8ButtonStates, *pui8Delta);

}

#ifdef CTRL_TARGET_BOARD
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

uint8_t flag_loginok = 0;
void GPIOAIntHandler(void)
{
	uint32_t ulStatus;
	ulStatus = GPIOIntStatus(GPIO_PORTA_BASE,true);
	GPIOIntClear(GPIO_PORTA_BASE,ulStatus);
	uint32_t Tp_time_val = 0;
	static uint32_t time1_riseing = 0 ,time2_riseing=0;
	static uint8_t flag_rise = 0;

	if(ulStatus&GPIO_INT_PIN_4)
	{
		his_rise_time = g_ui32TickCount;

		if(flag_rise==0)
		{
			 time1_riseing = g_ui32TickCount;
			 flag_rise = 1;
		}
		else
		{
			time2_riseing = g_ui32TickCount;
			flag_rise = 0;
			if(time2_riseing>time1_riseing)
			{
				Tp_time_val = time2_riseing - time1_riseing;
			}
			else
			{
				Tp_time_val = time2_riseing + 0xffffffff-time1_riseing;
			}
			if(Tp_time_val > 150)
			{
				flag_loginok = 1;
			}
		}

	}

}


void initKeys(void)
{
#ifdef VERSION_1HARDWARE
    //
    // Enable the GPIO port to which the pushbuttons are connected.
    //
	MAP_SysCtlPeripheralEnable(BUTTONS1_GPIO_PERIPH);
	MAP_SysCtlPeripheralEnable(BUTTONS2_GPIO_PERIPH);


    MAP_GPIODirModeSet(BUTTONS1_GPIO_BASE, ALL_BUTTONS_PB, GPIO_DIR_MODE_IN);

    MAP_GPIOPadConfigSet(BUTTONS1_GPIO_BASE, ALL_BUTTONS_PB,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


    MAP_GPIODirModeSet(BUTTONS2_GPIO_BASE, ALL_BUTTONS_PF, GPIO_DIR_MODE_IN);
    MAP_GPIOPadConfigSet(BUTTONS2_GPIO_BASE, ALL_BUTTONS_PF,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    //
    // Initialize the debounced button state with the current state read from
    // the GPIO bank.
    g_ui8ButtonStates = ((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PB7_3PBS_OPEN) >> 7) & KEY_3PBS_OPEN) |
    					((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PB6_3PBS_CLOSE) >> 5) & KEY_3PBS_CLOSE) |
    					(~(MAP_GPIOPinRead(BUTTONS2_GPIO_BASE, PF4_3PBS_STOP) >> 2) & KEY_3PBS_STOP);
#endif	//	VERSION_1HARDWARE

#ifdef VERSION_2HARDWARE

    //
    // Enable the GPIO port to which the pushbuttons are connected.
    //
    MAP_SysCtlPeripheralEnable(BUTTONS1_GPIO_PERIPH);

    MAP_GPIODirModeSet(BUTTONS1_GPIO_BASE, ALL_BUTTONS_PE, GPIO_DIR_MODE_IN);

    MAP_GPIOPadConfigSet(BUTTONS1_GPIO_BASE, ALL_BUTTONS_PE, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    //
    // Enable the GPIO port to which the wireless pushbuttons are connected.
    //
	MAP_SysCtlPeripheralEnable(BUTTONS1_GPIO_PERIPH_WIRELESS);
	MAP_SysCtlPeripheralEnable(BUTTONS2_GPIO_PERIPH_WIRELESS);


    MAP_GPIODirModeSet(BUTTONS1_GPIO_BASE_WIRELESS, ALL_BUTTONS_PB_WIRELESS, GPIO_DIR_MODE_IN);

    MAP_GPIOPadConfigSet(BUTTONS1_GPIO_BASE_WIRELESS, ALL_BUTTONS_PB_WIRELESS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


    MAP_GPIODirModeSet(BUTTONS2_GPIO_BASE_WIRELESS, ALL_BUTTONS_PF_WIRELESS, GPIO_DIR_MODE_IN);
    MAP_GPIOPadConfigSet(BUTTONS2_GPIO_BASE_WIRELESS, ALL_BUTTONS_PF_WIRELESS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    //
    // Initialize the debounced button state with the current state read from
    // the GPIO bank.

	#ifndef WIRELESS_3PBS_AS_VERSION_3HARDWARE

    g_ui8ButtonStates =	((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE3_3PBS_OPEN) >> 3) & KEY_3PBS_OPEN) 	|
    					((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE2_3PBS_CLOSE) >> 1) & KEY_3PBS_CLOSE) 	|
    					(~(MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE1_3PBS_STOP) << 1) & KEY_3PBS_STOP) 	|
    					((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE_WIRELESS, PB6_3PBS_OPEN_WIRELESS) >> 3) & KEY_3PBS_OPEN_WIRELESS)   |
    					((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE_WIRELESS, PB7_3PBS_CLOSE_WIRELESS) >> 3) & KEY_3PBS_CLOSE_WIRELESS) |
    					((MAP_GPIOPinRead(BUTTONS2_GPIO_BASE_WIRELESS, PF4_3PBS_STOP_WIRELESS) << 1) & KEY_3PBS_STOP_WIRELESS);

    // Change carried out on 17 April 2015 by YPG
    // It is observed, if 3PBS switch press before power ON and then regular functionality of keys are not working
    // It is due to the below variable get initialize with the value which is opposite logic
    // For same it is initialized to default values where keys are not pressed
    g_ui8ButtonStates = 0x3F;

	#endif

	#ifdef WIRELESS_3PBS_AS_VERSION_3HARDWARE

	g_ui8ButtonStates =	((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE3_3PBS_OPEN) >> 3) & KEY_3PBS_OPEN) 	|
						((MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE2_3PBS_CLOSE) >> 1) & KEY_3PBS_CLOSE) 	|
						(~(MAP_GPIOPinRead(BUTTONS1_GPIO_BASE, PE1_3PBS_STOP) << 1) & KEY_3PBS_STOP) 	|
						(~(MAP_GPIOPinRead(BUTTONS1_GPIO_BASE_WIRELESS, PB6_3PBS_OPEN_WIRELESS) >> 3) & KEY_3PBS_OPEN_WIRELESS)   |
						(~(MAP_GPIOPinRead(BUTTONS1_GPIO_BASE_WIRELESS, PB7_3PBS_CLOSE_WIRELESS) >> 3) & KEY_3PBS_CLOSE_WIRELESS) |
						(~(MAP_GPIOPinRead(BUTTONS2_GPIO_BASE_WIRELESS, PF4_3PBS_STOP_WIRELESS) << 1) & KEY_3PBS_STOP_WIRELESS);

	// Change carried out on 17 April 2015 by YPG
	// It is observed, if 3PBS switch press before power ON and then regular functionality of keys are not working
	// It is due to the below variable get initialize with the value which is opposite logic
	// For same it is initialized to default values where keys are not pressed
	g_ui8ButtonStates = 0x3F;

	#endif


#endif	//	VERSION_2HARDWARE

    gKeysStatus.Val = 0;


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
       GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);
       GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

       GPIOIntClear(GPIO_PORTA_BASE,GPIO_INT_PIN_4);
       GPIOIntEnable(GPIO_PORTA_BASE,GPIO_INT_PIN_4);
       GPIOIntRegister(GPIO_PORTA_BASE,GPIOAIntHandler);

       GPIOIntTypeSet(GPIO_PORTA_BASE,GPIO_INT_PIN_4,GPIO_BOTH_EDGES);
       IntEnable(INT_GPIOA);

}

/******************************************************************************
 * Function Name: initOutputGPIOs
 *
 * Function Description: This function must be called during application
 * initialization to configure the GPIO pins to which the interlock out,
 * monitor LED, multifunction out 1, multifunction out 2, electro mechanical
 * counter are attached. It enables the GPIO port used for these outputs and
 * configures corresponding GPIO pin as an output.
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void initOutputGPIOs(void)
{
	/*
	Electro-Mechanical counter GPIO configuration
	 */
	// Enable Port E for electro-mechanical counter
	MAP_SysCtlPeripheralEnable(COUNTER_GPIO_PERIPH);

	// PE4 dir out
	MAP_GPIODirModeSet(COUNTER_GPIO_BASE,COUNTER_OUT,GPIO_DIR_MODE_OUT);


	// PE4 pull up configuration
	/*MAP_GPIOPadConfigSet(COUNTER_GPIO_BASE, COUNTER_OUT,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);*/
	MAP_GPIOPadConfigSet(COUNTER_GPIO_BASE, COUNTER_OUT,
				GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);

	ROM_GPIOPinWrite(COUNTER_GPIO_BASE, COUNTER_OUT, COUNTER_OUT_LOW);

	/*
		Interlock output, monitor led, multifunction output 1 GPIO configuration
	*/
	// PORT B pins 3,4,5 are used for intelock, monitor led, multifunction output 1
	MAP_SysCtlPeripheralEnable(MONITOR_LED_GPIO_PERIPH);

	// PB4 dir out for monitor led
	MAP_GPIODirModeSet(MONITOR_LED_GPIO_BASE, MONITOR_LED, GPIO_DIR_MODE_OUT);

	// PB4 pull up configuration
	MAP_GPIOPadConfigSet(MONITOR_LED_GPIO_BASE, MONITOR_LED,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);

	// PB3 dir out for interlock out
	MAP_GPIODirModeSet(INTERLOCK_OUT_GPIO_BASE, INTERLOCK_OUT, GPIO_DIR_MODE_OUT);
	ROM_GPIOPinWrite(INTERLOCK_OUT_GPIO_BASE, INTERLOCK_OUT, INTERLOCK_OUT);

	// PB3 pull up configuration for interlock output
	MAP_GPIOPadConfigSet(INTERLOCK_OUT_GPIO_BASE, INTERLOCK_OUT,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);

	// PB5 dir out for multi function output 1
	MAP_GPIODirModeSet(MULTI_FUNC_OUT_1_GPIO_BASE, MULTI_FUNC_OUT_1, GPIO_DIR_MODE_OUT);
	ROM_GPIOPinWrite(MULTI_FUNC_OUT_1_GPIO_BASE, MULTI_FUNC_OUT_1, MULTI_FUNC_OUT_1);

	// PB5 pull up configuration
	MAP_GPIOPadConfigSet(MULTI_FUNC_OUT_1_GPIO_BASE, MULTI_FUNC_OUT_1,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);


	/*
			Interlock multifunction output 2 GPIO configuration
	 */
	// Enable Port D for multi function output 2
	MAP_SysCtlPeripheralEnable(MULTI_FUNC_OUT_2_GPIO_PERIPH);
	// PD6 dir out for monitor led
	MAP_GPIODirModeSet(MULTI_FUNC_OUT_2_GPIO_BASE,MULTI_FUNC_OUT_2,GPIO_DIR_MODE_OUT);
	ROM_GPIOPinWrite(MULTI_FUNC_OUT_2_GPIO_BASE, MULTI_FUNC_OUT_2, MULTI_FUNC_OUT_2);
	// PB5 pull up configuration
	MAP_GPIOPadConfigSet(MULTI_FUNC_OUT_2_GPIO_BASE, MULTI_FUNC_OUT_2,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);

#ifdef VERSION_2HARDWARE
	//	Added on 05 Dec 2014 as per new requirement from client
	/*
	 * 		WIRELESS MODE CHANGE output GPIO configuration
	 */
	//	Enable Port A for wireless mode change
	MAP_SysCtlPeripheralEnable(WIRELESS_MODE_CHANGE_GPIO_PERIPH);
	//	PA5 dir out for wireless mode change
	MAP_GPIODirModeSet(WIRELESS_MODE_CHANGE_GPIO_BASE,WIRELESS_MODE_CHANGE,GPIO_DIR_MODE_OUT);
	ROM_GPIOPinWrite(WIRELESS_MODE_CHANGE_GPIO_BASE,WIRELESS_MODE_CHANGE,WIRELESS_MODE_CHANGE_LOW);
	//	PA5 pull up configuration
	MAP_GPIOPadConfigSet(WIRELESS_MODE_CHANGE_GPIO_BASE, WIRELESS_MODE_CHANGE,
				GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

	//	Set high by default
	ACTIVATE_WIRELESS_MODE_CHANGE;


	/*
		Interlock multifunction output 3 GPIO configuration
	 */
	// Enable Port F for multi function output 3
	MAP_SysCtlPeripheralEnable(MULTI_FUNC_OUT_3_GPIO_PERIPH);
	// PD6 dir out for monitor led
	MAP_GPIODirModeSet(MULTI_FUNC_OUT_3_GPIO_BASE,MULTI_FUNC_OUT_3,GPIO_DIR_MODE_OUT);
	ROM_GPIOPinWrite(MULTI_FUNC_OUT_3_GPIO_BASE, MULTI_FUNC_OUT_3, MULTI_FUNC_OUT_3);
	// PB5 pull up configuration
	MAP_GPIOPadConfigSet(MULTI_FUNC_OUT_3_GPIO_BASE, MULTI_FUNC_OUT_3,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);

	//	Set high by default
	DEACTIVATE_MULTI_FUNC_OUT_3;
	ACTIVATE_MULTI_FUNC_OUT_3;
	DEACTIVATE_MULTI_FUNC_OUT_3;
	ACTIVATE_MULTI_FUNC_OUT_3;

	/*
		Interlock multifunction output 4 GPIO configuration
	 */
	// Enable Port F for multi function output 4
	MAP_SysCtlPeripheralEnable(MULTI_FUNC_OUT_4_GPIO_PERIPH);
	// PD6 dir out for monitor led
	MAP_GPIODirModeSet(MULTI_FUNC_OUT_4_GPIO_BASE,MULTI_FUNC_OUT_4,GPIO_DIR_MODE_OUT);
	ROM_GPIOPinWrite(MULTI_FUNC_OUT_4_GPIO_BASE, MULTI_FUNC_OUT_4, MULTI_FUNC_OUT_4);
	// PB5 pull up configuration
	MAP_GPIOPadConfigSet(MULTI_FUNC_OUT_4_GPIO_BASE, MULTI_FUNC_OUT_4,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);

	//	Set high by default
	DEACTIVATE_MULTI_FUNC_OUT_4;
	ACTIVATE_MULTI_FUNC_OUT_4;
	DEACTIVATE_MULTI_FUNC_OUT_4;
	ACTIVATE_MULTI_FUNC_OUT_4;

	/*
		Interlock multifunction output 5 GPIO configuration
	 */
	// Enable Port A for multi function output 5
	MAP_SysCtlPeripheralEnable(MULTI_FUNC_OUT_5_GPIO_PERIPH);
	// PD6 dir out for monitor led
	MAP_GPIODirModeSet(MULTI_FUNC_OUT_5_GPIO_BASE,MULTI_FUNC_OUT_5,GPIO_DIR_MODE_OUT);
	MAP_GPIODirModeSet(MULTI_FUNC_OUT_5_GPIO_BASE,MONIDENGLU_KEY,GPIO_DIR_MODE_OUT);

	MAP_GPIOPadConfigSet(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY,
				GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);//GPIO_PIN_TYPE_OD);


	ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, MONIDENGLU_KEY);
	ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MULTI_FUNC_OUT_5, MULTI_FUNC_OUT_5);
	// PB5 pull up configuration
	MAP_GPIOPadConfigSet(MULTI_FUNC_OUT_5_GPIO_BASE, MULTI_FUNC_OUT_5,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);

	//	Set high by default
	DEACTIVATE_MULTI_FUNC_OUT_5;
	ACTIVATE_MULTI_FUNC_OUT_5;
	DEACTIVATE_MULTI_FUNC_OUT_5;
	ACTIVATE_MULTI_FUNC_OUT_5;

#endif
}
#endif //------CTRL_TARGET_BOARD

#ifdef CTRL_LAUNCHPAD_BOARD
void initKeys(void)
{
    //
    // Enable the GPIO port to which the pushbuttons are connected.
    //
	MAP_SysCtlPeripheralEnable(BUTTONS_GPIO_PERIPH);
	//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	//
	//	Unlock Port F Commit Register
	//
    HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = 0x4C4F434B;

    //
    //	Configure Port F0 as GPIO
    //
    HWREG(BUTTONS_GPIO_BASE + GPIO_O_CR) = 1;

    //
    //	Lock Port F Commit Register
    //
    HWREG(BUTTONS_GPIO_BASE + GPIO_O_LOCK) = 45;


    MAP_GPIODirModeSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_DIR_MODE_IN);
    MAP_GPIOPadConfigSet(BUTTONS_GPIO_BASE, ALL_BUTTONS,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    //
    // Initialize the debounced button state with the current state read from
    // the GPIO bank.
    g_ui8ButtonStates = MAP_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);

    gKeysStatus.Val = 0;
}
#endif //----CTRL_LAUNCHPAD_BOARD
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


#ifdef CTRL_LAUNCHPAD_BOARD
void keysProcessFlags(uint8_t keyState, uint8_t keyChanged)
{
	uint8_t pressedState = keyState & ~(0xEE);
	static uint8_t keyStpPressedFirst;

	/********** General Button Flags *********/

	//EVAL board SW1 pressed----
	if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(PF4_SW1, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Open_pressed)
			{
				gKeysStatus.bits.Key_Open_pressed = true;
				currentTickCount = 0;
			}
		}
	}
	//EVAL board SW1 released
	if(!pressedState)
		if(BUTTON_RELEASED(PF4_SW1, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Open_released)
			{
				gKeysStatus.bits.Key_Open_released = true;
				gKeysStatus.bits.Key_Open_pressed = false;
			}
		}

	//EVAL board SW2 pressed----
	if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(PF0_SW2, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = true;
				currentTickCount = 0;
			}
		}
	}
	//EVAL board SW2 released
	if(!pressedState)
		if(BUTTON_RELEASED(PF0_SW2, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_released)
			{
				gKeysStatus.bits.Key_Stop_released = true;
				gKeysStatus.bits.Key_Stop_pressed = false;
			}
		}

	/********** End of General Button Flags *********/

	//If Key is released reset currentTickCount
	if(gKeysStatus.byte.HB_Released)
		currentTickCount = 0;

	/********* Extended Button Flags *************/
	if(pressedState)
	{
		if(pressedState == _ONLY_STOPKEY_PRESSED)
			keyStpPressedFirst = 1;

		if((PF4_SW1 == pressedState) &&
						(currentTickCount++ == SYSTICK_3SEC_KEYPRESS))
					gKeysStatus.bits.Key_3secCls_pressed = true;

		if(!isPowerOfTwo(pressedState))
		{ //Check if multiple keys are pressed

			//SW1 + SW2 pressed for 3 sec
			if(((PF4_SW1 | PF0_SW2) == pressedState) &&
					keyStpPressedFirst &&
					currentTickCount++ == SYSTICK_3SEC_KEYPRESS)
			{
				//				currentTickCount = 0;
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpOpn_pressed = true;
			}
		}
	}
}
#endif	//----CTRL_LAUNCHPAD_BOARD

#ifdef CTRL_TARGET_BOARD

#ifndef KEY_PRIORITY_LOGIC_ENABLED
void keysProcessFlags(uint8_t keyState, uint8_t keyChanged)
{

    //uartSendTxBuffer(UART_debug,&keyState,1);
    //uartSendTxBuffer(UART_debug,&keyChanged,1);

#ifdef VERSION_1HARDWARE
	uint8_t pressedState = keyState & ~(0xF8);
#endif

#ifdef VERSION_2HARDWARE
    uint8_t pressedState = keyState & ~(0xC0);

#endif

	static uint8_t keyStpPressedFirst;
	static uint8_t resetCountflag = 0;

	// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
	static unsigned int suiTimeStmpWirelesOpnReleased,suiTimeStmpWirelesClseReleased,suiTimeStmpWirelesStpReleased,suiTimeStmpWireles1pbsReleased;
	static unsigned char sucWirelesOpnReleasedFlag = 0,sucWirelesClseReleasedFlag = 0,sucWirelesStpReleasedFlag = 0,sucWireles1pbsReleasedFlag = 0;
	#define	KEY_RELEASED_LATCH_TIME	1000


#ifdef VERSION_1HARDWARE
	/********** General Button Flags *********/
	//PE3_3PBS_OPEN
	if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_OPEN, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Open_pressed)
			{
				gKeysStatus.bits.Key_Open_pressed = true;
				gKeysStatus.bits.Key_Open_released = false;

//				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 1;
				gstDriveStatusMenu.bits.Wireless_Close_Status = 1;
				currentTickCount = 0;
			}
		}
	}
	//---PE3_3PBS_OPEN
	if(!pressedState)
		if(BUTTON_RELEASED(KEY_3PBS_OPEN, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Open_released)
			{
				gKeysStatus.bits.Key_Open_released = true;
				gKeysStatus.bits.Key_Open_pressed = false;

//				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 0;
				gstDriveStatusMenu.bits.Wireless_Close_Status = 0;
			}
		}

	//PE1_3PBS_STOP
	if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_STOP, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;

//				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 1;
				gstDriveStatusMenu.bits.Wireless_Stop_Status = 1;
				currentTickCount = 0;
			}
		}
	}
	//---PE1_3PBS_STOP
	if(!pressedState)
		if(BUTTON_RELEASED(KEY_3PBS_STOP, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_released)
			{
				gKeysStatus.bits.Key_Stop_released = true;
				gKeysStatus.bits.Key_Stop_pressed = false;

//				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 0;
				gstDriveStatusMenu.bits.Wireless_Stop_Status = 0;
				keyStpPressedFirst = 0;
			}
		}

	//PE2_3PBS_CLOSE
	if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_CLOSE, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Close_pressed)
			{
				currentTickCount = 0;
				gKeysStatus.bits.Key_Close_pressed = true;
				gKeysStatus.bits.Key_Close_released = false;

//				gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 1;
				gstDriveStatusMenu.bits.Wireless_Open_Status = 1;
			}
		}
	}
	//---PE2_3PBS_CLOSE
	if(!pressedState)
		if(BUTTON_RELEASED(KEY_3PBS_CLOSE, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Close_released)
			{
				gKeysStatus.bits.Key_Close_released = true;
				gKeysStatus.bits.Key_Close_pressed = false;

//				gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 0;
				gstDriveStatusMenu.bits.Wireless_Open_Status = 0;
			}
		}

	/********** End of General Button Flags *********/
	//If Key is released reset currentTickCount
	if(gKeysStatus.byte.HB_Released) {
		currentTickCount = 0;
		resetCountflag = 0;
	}

	/********* Extended Button Flags *************/
	if(pressedState) {
		if(pressedState == _ONLY_STOPKEY_PRESSED)//check in upper section only
			if((pressedState == keyChanged))
				keyStpPressedFirst = 1;

		if((KEY_3PBS_CLOSE == pressedState) &&
				(currentTickCount++ == SYSTICK_3SEC_KEYPRESS))
			gKeysStatus.bits.Key_3secCls_pressed = true;

		//Stop pressed for 3 sec //Added 4 Jun 2014
		if((KEY_3PBS_STOP == pressedState) &&
				(currentTickCount++ == SYSTICK_3SEC_KEYPRESS)) {
			//Check in application the current mode of menu
			//Inspect for Close pressed flags!
			gKeysStatus.bits.Key_3secStp_pressed = true;
		}


		if(!isPowerOfTwo(pressedState)) { //Check if multiple keys are pressed

			if(!resetCountflag) {
				currentTickCount = 0;
				resetCountflag = ~0;
			}

			//Stop + Open pressed for 3 sec
			if(((KEY_3PBS_OPEN | KEY_3PBS_STOP) == pressedState) &&
					keyStpPressedFirst &&
					currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpOpn_pressed = true;
			}

			//Stop + Close pressed for 3 sec
			if(((KEY_3PBS_CLOSE | KEY_3PBS_STOP) == pressedState) &&
					keyStpPressedFirst &&
					currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpCls_pressed = true;
			}

			//Open + Stop + Close pressed for 3 sec
			if((INSTALLATION_KEYS == pressedState) &&
					currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
				//				currentTickCount = 0;
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys3_3secOpStCl_pressed = true;
			}
		}
	}
	/********* End of Extended Button Flags *************/
#endif	//	VERSION_1HARDWARE


#ifdef VERSION_2HARDWARE

	//uartSendTxBuffer(UART_debug,"H",1);

	/********** General Button Flags *********/

	//PE3_3PBS_OPEN

	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_OPEN, keyState, keyChanged))
		{

			if(!gKeysStatus.bits.Key_Open_pressed)
			{
				gKeysStatus.bits.Key_Open_pressed = true;
				gKeysStatus.bits.Key_Open_released = false;

				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 1;
				currentTickCount = 0;
			}
		}
	}

	//---PE3_3PBS_OPEN
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_OPEN, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Open_released)
			{
				gKeysStatus.bits.Key_Open_released = true;
				gKeysStatus.bits.Key_Open_pressed = false;

				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 0;

			}
		}

	//PE1_3PBS_STOP
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_STOP, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;

				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 1;
				currentTickCount = 0;
			}
		}
	}

	//---PE1_3PBS_STOP
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_STOP, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_released)
			{
				gKeysStatus.bits.Key_Stop_released = true;
				gKeysStatus.bits.Key_Stop_pressed = false;

				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 0;
				keyStpPressedFirst = 0;
			}
		}

	//PE2_3PBS_CLOSE
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_CLOSE, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Close_pressed)
			{
				currentTickCount = 0;
				gKeysStatus.bits.Key_Close_pressed = true;
				gKeysStatus.bits.Key_Close_released = false;

				gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 1;

			}
		}
	}

	//---PE2_3PBS_CLOSE
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_CLOSE, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Close_released)
			{
				gKeysStatus.bits.Key_Close_released = true;
				gKeysStatus.bits.Key_Close_pressed = false;

				gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 0;
			}
		}


	//PE3_3PBS_OPEN_WIRELESS
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_OPEN_WIRELESS, keyState, keyChanged))
		{

			if (gu8_wireless_1pbs_3pbs_o == 1) // Treate the signal as Wireless 3PBS Open
			{
				if(!gKeysStatus.bits.Wireless_Open_pressed)
				{

					gKeysStatus.bits.Wireless_Open_pressed = true;
					gKeysStatus.bits.Wireless_Open_released = false;

					gstDriveStatusMenu.bits.Wireless_Open_Status = 1;

					currentTickCount = 0;

					// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
					sucWirelesOpnReleasedFlag = 0;

				}
			}
			else if (gu8_wireless_1pbs_3pbs_o == 0) // Treate the signal as Wireless 1PBS
			{

				if(!gSensorStatus.bits.Sensor_Wireless_1PBS_active)
				{
					gSensorStatus.bits.Sensor_Wireless_1PBS_active = true;
					gSensorStatus.bits.Sensor_Wireless_1PBS_inactive = false;

					// Set drive status menu parameter
					gstDriveStatusMenu.bits.Wireless_1PBS_Status = 1;

					currentTickCount = 0;

					// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
					sucWireles1pbsReleasedFlag = 0;

				}

			}
		}
	}

	//---PE3_3PBS_OPEN_WIRELESS
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_OPEN_WIRELESS, keyState, keyChanged))
		{

			if (gu8_wireless_1pbs_3pbs_o == 1) // Treate the signal as Wireless 3PBS Open
			{

				if(!gKeysStatus.bits.Wireless_Open_released)
				{

					gKeysStatus.bits.Wireless_Open_released = true;
					gKeysStatus.bits.Wireless_Open_pressed = false;

					// Logic commented to Latch the wireless signals for specific period of time after released 13 Jan 2015
					//gstDriveStatusMenu.bits.Wireless_Open_Status = 0;

					// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
					suiTimeStmpWirelesOpnReleased = g_ui32TickCount;
					sucWirelesOpnReleasedFlag = 1;

				}

			}
			else if (gu8_wireless_1pbs_3pbs_o == 0) // Treate the signal as Wireless 1PBS
			{

				if(!gSensorStatus.bits.Sensor_Wireless_1PBS_inactive)
				{
					gSensorStatus.bits.Sensor_Wireless_1PBS_inactive = true;
					gSensorStatus.bits.Sensor_Wireless_1PBS_active = false;

					// Logic commented to Latch the wireless signals for specific period of time after released 13 Jan 2015
					//gstDriveStatusMenu.bits.Wireless_1PBS_Status = 0;

					// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
					suiTimeStmpWireles1pbsReleased = g_ui32TickCount;
					sucWireles1pbsReleasedFlag = 1;

				}

			}

		}

	// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
	// Logic to check the timeout
	if (1 == sucWirelesOpnReleasedFlag)
	{

		if ((g_ui32TickCount - suiTimeStmpWirelesOpnReleased) > KEY_RELEASED_LATCH_TIME)
		{

			gstDriveStatusMenu.bits.Wireless_Open_Status = 0;
			sucWirelesOpnReleasedFlag = 0;

		}

	}

	if (1 == sucWireles1pbsReleasedFlag)
	{

		if ((g_ui32TickCount - suiTimeStmpWireles1pbsReleased) > KEY_RELEASED_LATCH_TIME)
		{

			gstDriveStatusMenu.bits.Wireless_1PBS_Status = 0;
			sucWireles1pbsReleasedFlag = 0;

		}

	}


	//PE1_3PBS_STOP_WIRELESS
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_STOP_WIRELESS, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Wireless_Stop_pressed)
			{
				gKeysStatus.bits.Wireless_Stop_pressed = true;
				gKeysStatus.bits.Wireless_Stop_released = false;

				gstDriveStatusMenu.bits.Wireless_Stop_Status = 1;
				currentTickCount = 0;
			}
		}
	}

	//---PE1_3PBS_STOP_WIRELESS
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_STOP_WIRELESS, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Wireless_Stop_released)
			{
				gKeysStatus.bits.Wireless_Stop_released = true;
				gKeysStatus.bits.Wireless_Stop_pressed = false;

				// Logic commented to Latch the wireless signals for specific period of time after released 13 Jan 2015
				//gstDriveStatusMenu.bits.Wireless_Stop_Status = 0;

				// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
				suiTimeStmpWirelesStpReleased = g_ui32TickCount;
				sucWirelesStpReleasedFlag = 1;

				keyStpPressedFirst = 0;
			}
		}

	// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
	// Logic to check the timeout
	if (1 == sucWirelesStpReleasedFlag)
	{

		if ((g_ui32TickCount - suiTimeStmpWirelesStpReleased) > KEY_RELEASED_LATCH_TIME)
		{

			gstDriveStatusMenu.bits.Wireless_Stop_Status = 0;
			sucWirelesStpReleasedFlag = 0;

		}

	}

	//PE2_3PBS_CLOSE_WIRELESS
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_CLOSE_WIRELESS, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Wireless_Close_pressed)
			{
				currentTickCount = 0;
				gKeysStatus.bits.Wireless_Close_pressed = true;
				gKeysStatus.bits.Wireless_Close_released = false;

				gstDriveStatusMenu.bits.Wireless_Close_Status = 1;

				uartSendTxBuffer(UART_debug,"C",1);
			}
		}
	}

	//---PE2_3PBS_CLOSE_WIRELESS
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_CLOSE_WIRELESS, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Wireless_Close_released)
			{
				gKeysStatus.bits.Wireless_Close_released = true;
				gKeysStatus.bits.Wireless_Close_pressed = false;

				// Logic commented to Latch the wireless signals for specific period of time after released 13 Jan 2015
				//gstDriveStatusMenu.bits.Wireless_Close_Status = 0;

				// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
				suiTimeStmpWirelesClseReleased = g_ui32TickCount;
				sucWirelesClseReleasedFlag = 1;

				uartSendTxBuffer(UART_debug,"c",1);
			}
		}


	// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
	// Logic to check the timeout
	if (1 == sucWirelesClseReleasedFlag)
	{

		if ((g_ui32TickCount - suiTimeStmpWirelesClseReleased) > KEY_RELEASED_LATCH_TIME)
		{

			gstDriveStatusMenu.bits.Wireless_Close_Status = 0;
			sucWirelesClseReleasedFlag = 0;

		}

	}

	/********** End of General Button Flags *********/
	//If Key is released reset currentTickCount
	if(gKeysStatus.byte.HB_Released) {
		currentTickCount = 0;
		resetCountflag = 0;
	}

	/********* Extended Button Flags *************/
	if(pressedState) {
		if(pressedState == _ONLY_STOPKEY_PRESSED_WIRELESS)//check in upper section only
			if((pressedState == keyChanged))
				keyStpPressedFirst = 1;

		if((KEY_3PBS_CLOSE_WIRELESS == pressedState) &&
				(currentTickCount++ == SYSTICK_3SEC_KEYPRESS))
			gKeysStatus.bits.Key_3secCls_pressed = true;

		//Stop pressed for 3 sec //Added 4 Jun 2014
		if((KEY_3PBS_STOP_WIRELESS == pressedState) &&
				(currentTickCount++ == SYSTICK_3SEC_KEYPRESS)) {
			//Check in application the current mode of menu
			//Inspect for Close pressed flags!
			gKeysStatus.bits.Key_3secStp_pressed = true;
		}


		if(!isPowerOfTwo(pressedState)) { //Check if multiple keys are pressed

			if(!resetCountflag) {
				currentTickCount = 0;
				resetCountflag = ~0;
			}

			//Stop + Open pressed for 3 sec
			if(((KEY_3PBS_OPEN_WIRELESS | KEY_3PBS_STOP_WIRELESS) == pressedState) &&
					keyStpPressedFirst &&
					currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpOpn_pressed = true;
			}

			//Stop + Close pressed for 3 sec
			if(((KEY_3PBS_CLOSE_WIRELESS | KEY_3PBS_STOP_WIRELESS) == pressedState) &&
					keyStpPressedFirst &&
					currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpCls_pressed = true;
			}

			//Open + Stop + Close pressed for 3 sec
			if((INSTALLATION_KEYS_WIRELESS == pressedState) &&
					currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
				//				currentTickCount = 0;
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys3_3secOpStCl_pressed = true;
			}
		}
	}
	/********* End of Extended Button Flags *************/
#endif	//	VERSION_2HARDWARE
}

#endif //#ifndef KEY_PRIORITY_LOGIC_ENABLED



#ifdef KEY_PRIORITY_LOGIC_ENABLED
void keysProcessFlags(uint8_t keyState, uint8_t keyChanged)
{

	// ****************************************************************
	// #define for Key Priority .......................................
	// ****************************************************************
	#define Priority_Close_Key								1
	#define Priority_Open_Key								2
	#define Priority_Stop_Key								3

	static uint8_t keyPressedEvent = 0;
	static uint8_t keyReleasedEvent = 0;
	static uint8_t PriorityLastSentPressKey = 0;	// The variable will reset during power ON and when the key is released
	static uint32_t s_ui32TickCount10ms;
	static uint8_t FlagToSendKeyEvent = 1;  		// When the flag is set, send key press and released event which has highest priority

    //uartSendTxBuffer(UART_debug,&keyState,1);
    //uartSendTxBuffer(UART_debug,&keyChanged,1);

#ifdef VERSION_1HARDWARE
	uint8_t pressedState = keyState & ~(0xF8);
#endif

#ifdef VERSION_2HARDWARE
    uint8_t pressedState = keyState & ~(0xC0);

#endif

	static uint8_t keyStpPressedFirst;
	static uint8_t resetCountflag = 0;

	// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
	static unsigned int suiTimeStmpWirelesOpnReleased,suiTimeStmpWirelesClseReleased,suiTimeStmpWirelesStpReleased,suiTimeStmpWireles1pbsReleased;
	static unsigned char sucWirelesOpnReleasedFlag = 0,sucWirelesClseReleasedFlag = 0,sucWirelesStpReleasedFlag = 0,sucWireles1pbsReleasedFlag = 0;
	#define	KEY_RELEASED_LATCH_TIME	1000


#ifdef VERSION_1HARDWARE
	/********** General Button Flags *********/
	//PE3_3PBS_OPEN
	if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_OPEN, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Open_pressed)
			{
				gKeysStatus.bits.Key_Open_pressed = true;
				gKeysStatus.bits.Key_Open_released = false;

//				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 1;
				gstDriveStatusMenu.bits.Wireless_Close_Status = 1;
				currentTickCount = 0;
			}
		}
	}
	//---PE3_3PBS_OPEN
	if(!pressedState)
		if(BUTTON_RELEASED(KEY_3PBS_OPEN, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Open_released)
			{
				gKeysStatus.bits.Key_Open_released = true;
				gKeysStatus.bits.Key_Open_pressed = false;

//				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 0;
				gstDriveStatusMenu.bits.Wireless_Close_Status = 0;
			}
		}

	//PE1_3PBS_STOP
	if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_STOP, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;

//				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 1;
				gstDriveStatusMenu.bits.Wireless_Stop_Status = 1;
				currentTickCount = 0;
			}
		}
	}
	//---PE1_3PBS_STOP
	if(!pressedState)
		if(BUTTON_RELEASED(KEY_3PBS_STOP, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_released)
			{
				gKeysStatus.bits.Key_Stop_released = true;
				gKeysStatus.bits.Key_Stop_pressed = false;

//				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 0;
				gstDriveStatusMenu.bits.Wireless_Stop_Status = 0;
				keyStpPressedFirst = 0;
			}
		}

	//PE2_3PBS_CLOSE
	if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_CLOSE, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Close_pressed)
			{
				currentTickCount = 0;
				gKeysStatus.bits.Key_Close_pressed = true;
				gKeysStatus.bits.Key_Close_released = false;

//				gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 1;
				gstDriveStatusMenu.bits.Wireless_Open_Status = 1;
			}
		}
	}
	//---PE2_3PBS_CLOSE
	if(!pressedState)
		if(BUTTON_RELEASED(KEY_3PBS_CLOSE, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Close_released)
			{
				gKeysStatus.bits.Key_Close_released = true;
				gKeysStatus.bits.Key_Close_pressed = false;

//				gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 0;
				gstDriveStatusMenu.bits.Wireless_Open_Status = 0;
			}
		}

	/********** End of General Button Flags *********/
	//If Key is released reset currentTickCount
	if(gKeysStatus.byte.HB_Released) {
		currentTickCount = 0;
		resetCountflag = 0;
	}

	/********* Extended Button Flags *************/
	if(pressedState) {
		if(pressedState == _ONLY_STOPKEY_PRESSED)//check in upper section only
			if((pressedState == keyChanged))
				keyStpPressedFirst = 1;

		if((KEY_3PBS_CLOSE == pressedState) &&
				(currentTickCount++ == SYSTICK_3SEC_KEYPRESS))
			gKeysStatus.bits.Key_3secCls_pressed = true;

		//Stop pressed for 3 sec //Added 4 Jun 2014
		if((KEY_3PBS_STOP == pressedState) &&
				(currentTickCount++ == SYSTICK_3SEC_KEYPRESS)) {
			//Check in application the current mode of menu
			//Inspect for Close pressed flags!
			gKeysStatus.bits.Key_3secStp_pressed = true;
		}


		if(!isPowerOfTwo(pressedState)) { //Check if multiple keys are pressed

			if(!resetCountflag) {
				currentTickCount = 0;
				resetCountflag = ~0;
			}

			//Stop + Open pressed for 3 sec
			if(((KEY_3PBS_OPEN | KEY_3PBS_STOP) == pressedState) &&
					keyStpPressedFirst &&
					currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpOpn_pressed = true;
			}

			//Stop + Close pressed for 3 sec
			if(((KEY_3PBS_CLOSE | KEY_3PBS_STOP) == pressedState) &&
					keyStpPressedFirst &&
					currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpCls_pressed = true;
			}

			//Open + Stop + Close pressed for 3 sec
			if((INSTALLATION_KEYS == pressedState) &&
					currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
				//				currentTickCount = 0;
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys3_3secOpStCl_pressed = true;
			}
		}
	}
	/********* End of Extended Button Flags *************/
#endif	//	VERSION_1HARDWARE


#ifdef VERSION_2HARDWARE

	//uartSendTxBuffer(UART_debug,"H",1);

	/********** General Button Flags *********/

#if 0
	//PE3_3PBS_OPEN

	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_OPEN, keyState, keyChanged))
		{

			if(!gKeysStatus.bits.Key_Open_pressed)
			{
				gKeysStatus.bits.Key_Open_pressed = true;
				gKeysStatus.bits.Key_Open_released = false;

				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 1;
				currentTickCount = 0;
			}
		}
	}

	//---PE3_3PBS_OPEN
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_OPEN, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Open_released)
			{
				gKeysStatus.bits.Key_Open_released = true;
				gKeysStatus.bits.Key_Open_pressed = false;

				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 0;

			}
		}

	//PE1_3PBS_STOP
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_STOP, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;

				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 1;
				currentTickCount = 0;
			}
		}
	}

	//---PE1_3PBS_STOP
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_STOP, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_released)
			{
				gKeysStatus.bits.Key_Stop_released = true;
				gKeysStatus.bits.Key_Stop_pressed = false;

				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 0;
				keyStpPressedFirst = 0;
			}
		}

	//PE2_3PBS_CLOSE
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_CLOSE, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Close_pressed)
			{
				currentTickCount = 0;
				gKeysStatus.bits.Key_Close_pressed = true;
				gKeysStatus.bits.Key_Close_released = false;

				gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 1;

			}
		}
	}

	//---PE2_3PBS_CLOSE
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_CLOSE, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Close_released)
			{
				gKeysStatus.bits.Key_Close_released = true;
				gKeysStatus.bits.Key_Close_pressed = false;

				gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 0;
			}
		}
#endif


		//*****************************************************************************************
		//Key Priority Logic:
		//Capture key Press Event, Key Released Event for Open, Close, Stop in local Static variable
		//*****************************************************************************************

		//Open Button > NO BUTTON ON EVAL @LEFT
		//if(isPowerOfTwo(pressedState))
		{
			if(BUTTON_PRESSED(KEY_3PBS_OPEN, keyState, keyChanged))
			{
				//if(!gKeysStatus.bits.Key_Open_pressed)
				//{
				//	gKeysStatus.bits.Key_Open_pressed = true;
				//	gKeysStatus.bits.Key_Open_released = false;
				//	currentTickCount = 0;
				//}

				keyPressedEvent |= KEY_3PBS_OPEN;
			}
		}

		//if(!pressedState)
			if(BUTTON_RELEASED(KEY_3PBS_OPEN, keyState, keyChanged))
			{
				//if(!gKeysStatus.bits.Key_Open_released)
				//{
				//	gKeysStatus.bits.Key_Open_released = true;
				//	gKeysStatus.bits.Key_Open_pressed = false;
				//}

				keyReleasedEvent |= KEY_3PBS_OPEN;
			}

		//Close Button > NO BUTTON ON EVAL @SELECT
		//if(isPowerOfTwo(pressedState))
		{
			if(BUTTON_PRESSED(KEY_3PBS_CLOSE, keyState, keyChanged))
			{
				//if(!gKeysStatus.bits.Key_Close_pressed)
				//{
				//	gKeysStatus.bits.Key_Close_released = false;
				//	gKeysStatus.bits.Key_Close_pressed = true;
				//	currentTickCount = 0;
				//}

				keyPressedEvent |= KEY_3PBS_CLOSE;
			}
		}

		//if(!pressedState)
			if(BUTTON_RELEASED(KEY_3PBS_CLOSE, keyState, keyChanged))
			{
				//if(!gKeysStatus.bits.Key_Close_released)
				//{
				//	gKeysStatus.bits.Key_Close_released = true;
				//	gKeysStatus.bits.Key_Close_pressed = false;
				//}

				keyReleasedEvent |= KEY_3PBS_CLOSE;
			}

	//	Stop Button > NO BUTTON ON EVAL @RIGHT
		//if(isPowerOfTwo(pressedState))
		{
			if(BUTTON_PRESSED(KEY_3PBS_STOP, keyState, keyChanged))
			{
				//if(!gKeysStatus.bits.Key_Stop_pressed)
				//{
				//	gKeysStatus.bits.Key_Stop_pressed = true;
				//	gKeysStatus.bits.Key_Stop_released = false;
				//	currentTickCount = 0;
				//}

				keyPressedEvent |= KEY_3PBS_STOP;
				gstControlBoardStatus.bits.s3PBS_stoppress = 1;//for item bug_No.106 20160909
			}
		}

		//if(!pressedState)
			if(BUTTON_RELEASED(KEY_3PBS_STOP, keyState, keyChanged))
			{
				//if(!gKeysStatus.bits.Key_Stop_released)
				//{
				//	gKeysStatus.bits.Key_Stop_released = true;
				//	gKeysStatus.bits.Key_Stop_pressed = false;
				//	keyStpPressedFirst = 0;
				//}

				keyReleasedEvent |= KEY_3PBS_STOP;
				gstControlBoardStatus.bits.s3PBS_stoppress = 0;//for item bug_No.106 20160909
			}


			//*****************************************************************************************
			//Key Priority Logic:
			//Check for any new key press which has highest priority than the earlier key pressed which is not yet released
			//Send released command for earlier key pressed which is not yet released
			//Start the timer to send newly active operation key with high priority after timeout
			//*****************************************************************************************
				if (
						(PriorityLastSentPressKey != 0) &&
						(
							(	(keyPressedEvent & KEY_3PBS_OPEN) && (PriorityLastSentPressKey < Priority_Open_Key)	)||
							(	(keyPressedEvent & KEY_3PBS_STOP) && (PriorityLastSentPressKey < Priority_Stop_Key)	)
						)
				   )
				{

					//Send released command for earlier key pressed which is not yet released
					if (PriorityLastSentPressKey == 1) 		// Previous key which is not yet released is Close
					{
						if(!gKeysStatus.bits.Key_Close_released)
						{
						gKeysStatus.bits.Key_Close_released = true;
						gKeysStatus.bits.Key_Close_pressed = false;

						gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 1;
						}

					}
					else if (PriorityLastSentPressKey == 2) // Previous key which is not yet released is open
					{
						if(!gKeysStatus.bits.Key_Open_released)
						{
						gKeysStatus.bits.Key_Open_released = true;
						gKeysStatus.bits.Key_Open_pressed = false;

						gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 1;
						}

					}

					// reset the the variable which hold the priority
					PriorityLastSentPressKey = 0;

					//Start Timer to send next active highest priority operation command
					s_ui32TickCount10ms = g_ui32TickCount;

					//Flag to send highest priority key press and released = reset
					FlagToSendKeyEvent = 0;

				}
				//*****************************************************************************************
				//Key Priority Logic:
				//Check for timeout to get complete and enable flag to send the key events with highest priority
				//*****************************************************************************************
				else if ( (FlagToSendKeyEvent == 0) && (get_timego( s_ui32TickCount10ms) > 10 /*100msec as ticker is of 10msec*/) )
				{

					FlagToSendKeyEvent = 1;

				}
				//*****************************************************************************************
				//Key Priority Logic:
				//Send key event which has highest priority
				//*****************************************************************************************
				else if (FlagToSendKeyEvent == 1)
				{

					// Check for key press event
					if (PriorityLastSentPressKey == 0)
					{

						if (keyPressedEvent & KEY_3PBS_STOP)
						{

							if(!gKeysStatus.bits.Key_Stop_pressed)
							{
								KEY_STOP_CYW =1;
								gKeysStatus.bits.Key_Stop_pressed = true;
								gKeysStatus.bits.Key_Stop_released = false;
								currentTickCount = 0;
								gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 1;

								PriorityLastSentPressKey = Priority_Stop_Key;

							}

						}
						else if (keyPressedEvent & KEY_3PBS_OPEN)
						{

							if(!gKeysStatus.bits.Key_Open_pressed)
							{
								gKeysStatus.bits.Key_Open_pressed = true;
								gKeysStatus.bits.Key_Open_released = false;
								currentTickCount = 0;
								gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 1;

								PriorityLastSentPressKey = Priority_Open_Key;

							}

						}
						else if (keyPressedEvent & KEY_3PBS_CLOSE)
						{

							if(!gKeysStatus.bits.Key_Close_pressed)
							{
								gKeysStatus.bits.Key_Close_released = false;
								gKeysStatus.bits.Key_Close_pressed = true;
								currentTickCount = 0;
								gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 1;

								PriorityLastSentPressKey = Priority_Close_Key;

							}

						}

					} // Check for key press event
					// check for key release event for which press event already sent
					else if (
							((PriorityLastSentPressKey == Priority_Stop_Key) && (keyReleasedEvent & KEY_3PBS_STOP)) ||
							((PriorityLastSentPressKey == Priority_Open_Key) && (keyReleasedEvent & KEY_3PBS_OPEN)) ||
							((PriorityLastSentPressKey == Priority_Close_Key) && (keyReleasedEvent & KEY_3PBS_CLOSE))
							)
					{

						if ((PriorityLastSentPressKey == Priority_Stop_Key) && (keyReleasedEvent & KEY_3PBS_STOP))
						{

							if(!gKeysStatus.bits.Key_Stop_released)
							{
								gKeysStatus.bits.Key_Stop_released = true;
								gKeysStatus.bits.Key_Stop_pressed = false;
								keyStpPressedFirst = 0;
								gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 0;

								keyReleasedEvent &= (~KEY_3PBS_STOP);
								keyPressedEvent &= (~KEY_3PBS_STOP);

							}

						}
						else if ((PriorityLastSentPressKey == Priority_Open_Key) && (keyReleasedEvent & KEY_3PBS_OPEN))
						{

							if(!gKeysStatus.bits.Key_Open_released)
							{
								gKeysStatus.bits.Key_Open_released = true;
								gKeysStatus.bits.Key_Open_pressed = false;
								gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 0;

								keyReleasedEvent &= (~KEY_3PBS_OPEN);
								keyPressedEvent &= (~KEY_3PBS_OPEN);

							}

						}
						else if ((PriorityLastSentPressKey == Priority_Close_Key) && (keyReleasedEvent & KEY_3PBS_CLOSE))
						{

							if(!gKeysStatus.bits.Key_Close_released)
							{
								gKeysStatus.bits.Key_Close_released = true;
								gKeysStatus.bits.Key_Close_pressed = false;
								gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 0;

								keyReleasedEvent &= (~KEY_3PBS_CLOSE);
								keyPressedEvent &= (~KEY_3PBS_CLOSE);

							}

						}

						PriorityLastSentPressKey = 0;

					} // check for key release event for which press event already sent
					// check for key release event which has less priority than the press event already sent( For which released event already sent)
					else if (
							((keyReleasedEvent & KEY_3PBS_CLOSE) && (PriorityLastSentPressKey > Priority_Close_Key)) ||
							((keyReleasedEvent & KEY_3PBS_OPEN) && (PriorityLastSentPressKey > Priority_Open_Key))
							)
					{

						if ((keyReleasedEvent & KEY_3PBS_CLOSE) && (PriorityLastSentPressKey > Priority_Close_Key))
						{

							keyReleasedEvent &= (~KEY_3PBS_CLOSE);
							keyPressedEvent &= (~KEY_3PBS_CLOSE);

						}
						else if ((keyReleasedEvent & KEY_3PBS_OPEN) && (PriorityLastSentPressKey > Priority_Open_Key))
						{

							keyReleasedEvent &= (~KEY_3PBS_OPEN);
							keyPressedEvent &= (~KEY_3PBS_OPEN);

						}

					}

				} //else if (FlagToSendKeyEvent == 1)


	//PE3_3PBS_OPEN_WIRELESS
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_OPEN_WIRELESS, keyState, keyChanged))
		{

			if (gu8_wireless_1pbs_3pbs_o == 1) // Treate the signal as Wireless 3PBS Open
			{
				if(!gKeysStatus.bits.Wireless_Open_pressed)
				{

					gKeysStatus.bits.Wireless_Open_pressed = true;
					gKeysStatus.bits.Wireless_Open_released = false;

					gstDriveStatusMenu.bits.Wireless_Open_Status = 1;

					currentTickCount = 0;

					// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
					sucWirelesOpnReleasedFlag = 0;

				}
			}
			else if (gu8_wireless_1pbs_3pbs_o == 0) // Treate the signal as Wireless 1PBS
			{

				if(!gSensorStatus.bits.Sensor_Wireless_1PBS_active)
				{
					gSensorStatus.bits.Sensor_Wireless_1PBS_active = true;
					gSensorStatus.bits.Sensor_Wireless_1PBS_inactive = false;

					// Set drive status menu parameter
					gstDriveStatusMenu.bits.Wireless_1PBS_Status = 1;

					currentTickCount = 0;

					// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
					sucWireles1pbsReleasedFlag = 0;

				}

			}
		}
	}

	//---PE3_3PBS_OPEN_WIRELESS
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_OPEN_WIRELESS, keyState, keyChanged))
		{

			if (gu8_wireless_1pbs_3pbs_o == 1) // Treate the signal as Wireless 3PBS Open
			{

				if(!gKeysStatus.bits.Wireless_Open_released)
				{

					gKeysStatus.bits.Wireless_Open_released = true;
					gKeysStatus.bits.Wireless_Open_pressed = false;

					// Logic commented to Latch the wireless signals for specific period of time after released 13 Jan 2015
					//gstDriveStatusMenu.bits.Wireless_Open_Status = 0;

					// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
					suiTimeStmpWirelesOpnReleased = g_ui32TickCount;
					sucWirelesOpnReleasedFlag = 1;

					suiTimeStampForOnePBS = g_ui32TickCount; //20161201

				}

			}
			else if (gu8_wireless_1pbs_3pbs_o == 0) // Treate the signal as Wireless 1PBS
			{

				if(!gSensorStatus.bits.Sensor_Wireless_1PBS_inactive)
				{
					gSensorStatus.bits.Sensor_Wireless_1PBS_inactive = true;
					gSensorStatus.bits.Sensor_Wireless_1PBS_active = false;

					// Logic commented to Latch the wireless signals for specific period of time after released 13 Jan 2015
					//gstDriveStatusMenu.bits.Wireless_1PBS_Status = 0;

					// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
					suiTimeStmpWireles1pbsReleased = g_ui32TickCount;
					sucWireles1pbsReleasedFlag = 1;

					suiTimeStampForOnePBS = g_ui32TickCount; //20161202

				}

			}

		}

	// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
	// Logic to check the timeout
	if (1 == sucWirelesOpnReleasedFlag)
	{

		if (get_timego( suiTimeStmpWirelesOpnReleased) > KEY_RELEASED_LATCH_TIME)
		{

			gstDriveStatusMenu.bits.Wireless_Open_Status = 0;
			sucWirelesOpnReleasedFlag = 0;

		}

	}

	if (1 == sucWireles1pbsReleasedFlag)
	{

		if (get_timego( suiTimeStmpWireles1pbsReleased) > KEY_RELEASED_LATCH_TIME)
		{

			gstDriveStatusMenu.bits.Wireless_1PBS_Status = 0;
			sucWireles1pbsReleasedFlag = 0;

		}

	}


	//PE1_3PBS_STOP_WIRELESS
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_STOP_WIRELESS, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Wireless_Stop_pressed)
			{
				gKeysStatus.bits.Wireless_Stop_pressed = true;
				gKeysStatus.bits.Wireless_Stop_released = false;

				gstDriveStatusMenu.bits.Wireless_Stop_Status = 1;
				currentTickCount = 0;
			}
		}
	}

	//---PE1_3PBS_STOP_WIRELESS
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_STOP_WIRELESS, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Wireless_Stop_released)
			{
				gKeysStatus.bits.Wireless_Stop_released = true;
				gKeysStatus.bits.Wireless_Stop_pressed = false;

				// Logic commented to Latch the wireless signals for specific period of time after released 13 Jan 2015
				//gstDriveStatusMenu.bits.Wireless_Stop_Status = 0;

				// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
				suiTimeStmpWirelesStpReleased = g_ui32TickCount;
				sucWirelesStpReleasedFlag = 1;

				keyStpPressedFirst = 0;

				suiTimeStampForOnePBS = g_ui32TickCount; //20161202
			}
		}

	// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
	// Logic to check the timeout
	if (1 == sucWirelesStpReleasedFlag)
	{

		if (get_timego( suiTimeStmpWirelesStpReleased) > KEY_RELEASED_LATCH_TIME)
		{

			gstDriveStatusMenu.bits.Wireless_Stop_Status = 0;
			sucWirelesStpReleasedFlag = 0;

		}

	}

	//PE2_3PBS_CLOSE_WIRELESS
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_3PBS_CLOSE_WIRELESS, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Wireless_Close_pressed)
			{
				currentTickCount = 0;
				gKeysStatus.bits.Wireless_Close_pressed = true;
				gKeysStatus.bits.Wireless_Close_released = false;

				gstDriveStatusMenu.bits.Wireless_Close_Status = 1;

				uartSendTxBuffer(UART_debug,"C",1);
			}
		}
	}

	//---PE2_3PBS_CLOSE_WIRELESS
	//if(!pressedState)

		if(BUTTON_RELEASED(KEY_3PBS_CLOSE_WIRELESS, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Wireless_Close_released)
			{
				gKeysStatus.bits.Wireless_Close_released = true;
				gKeysStatus.bits.Wireless_Close_pressed = false;

				// Logic commented to Latch the wireless signals for specific period of time after released 13 Jan 2015
				//gstDriveStatusMenu.bits.Wireless_Close_Status = 0;

				// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
				suiTimeStmpWirelesClseReleased = g_ui32TickCount;
				sucWirelesClseReleasedFlag = 1;

				uartSendTxBuffer(UART_debug,"c",1);
			}
		}


	// Logic added to Latch the wireless signals for specific period of time after released 13 Jan 2015
	// Logic to check the timeout
	if (1 == sucWirelesClseReleasedFlag)
	{

		if (get_timego( suiTimeStmpWirelesClseReleased) > KEY_RELEASED_LATCH_TIME)
		{

			gstDriveStatusMenu.bits.Wireless_Close_Status = 0;
			sucWirelesClseReleasedFlag = 0;

		}

	}

	/********** End of General Button Flags *********/
	//If Key is released reset currentTickCount
	if(gKeysStatus.byte.HB_Released)
	{
		currentTickCount = 0;
		resetCountflag = 0;
	}

	/********* Extended Button Flags *************/
	if(pressedState)
	{
		if(pressedState == _ONLY_STOPKEY_PRESSED)//check in upper section only
		{

			if((pressedState == keyChanged))
			{
				keyStpPressedFirst = 1;
			}

		}

		if((KEY_3PBS_CLOSE == pressedState) && (currentTickCount++ == SYSTICK_3SEC_KEYPRESS))
		{
			gKeysStatus.bits.Key_3secCls_pressed = true;
		}

		//Stop pressed for 3 sec //Added 4 Jun 2014
		if((KEY_3PBS_STOP == pressedState) && (currentTickCount++ == SYSTICK_3SEC_KEYPRESS))
		{
			//Check in application the current mode of menu
			//Inspect for Close pressed flags!
			gKeysStatus.bits.Key_3secStp_pressed = true;
		}


		if(!isPowerOfTwo(pressedState)) //Check if multiple keys are pressed
		{

			if(!resetCountflag)
			{
				currentTickCount = 0;
				resetCountflag = ~0;
			}

			//Stop + Open pressed for 3 sec
			if(((KEY_3PBS_OPEN | KEY_3PBS_STOP) == pressedState) &&
			   keyStpPressedFirst &&
			   currentTickCount++ == SYSTICK_3SEC_KEYPRESS
			  )
			{
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpOpn_pressed = true;
			}

			//Stop + Close pressed for 3 sec
			if(((KEY_3PBS_CLOSE | KEY_3PBS_STOP) == pressedState) &&
				keyStpPressedFirst &&
				currentTickCount++ == SYSTICK_3SEC_KEYPRESS
			  )
			{
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpCls_pressed = true;
			}

			//Open + Stop + Close pressed for 3 sec
			if((INSTALLATION_KEYS == pressedState) && currentTickCount++ == SYSTICK_3SEC_KEYPRESS)
			{
				//				currentTickCount = 0;
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys3_3secOpStCl_pressed = true;
			}

		}

	}

//	static uint8_t Tp_count_anomaly_cyw=0,Tp_count_anomaly_cyw1;
//		for(Tp_count_anomaly_cyw = 0;Tp_count_anomaly_cyw <20;Tp_count_anomaly_cyw++)
//		{
//		    if(gsActiveAnomalylist[Tp_count_anomaly_cyw]!=0)
//		    {
//		    	for(Tp_count_anomaly_cyw1 = 0;Tp_count_anomaly_cyw1<TOTAL_ERRORS;Tp_count_anomaly_cyw1++)
//		    	{
//		    		if(gsActiveAnomalylist[Tp_count_anomaly_cyw]==gstErrorList[Tp_count_anomaly_cyw1].errorCode)
//		    		{
//		    			if(gstErrorList[Tp_count_anomaly_cyw1].stop_shutter_move_comm==NONE_OPEN_CLOSE)
//		    			{
//		    				gKeysStatus.bits.Wireless_Open_pressed = false;
//		    			    gKeysStatus.bits.Wireless_Close_pressed = false;
//		    				gSensorStatus.bits.Sensor_Wireless_1PBS_active = false;
//		    				gSensorStatus.bits.Sensor_1PBS_active = false;
//		    				gKeysStatus.bits.Key_Open_pressed =  false;
//		    			    gKeysStatus.bits.Key_Close_pressed =  false;
//		    			}
//		    			if(gstErrorList[Tp_count_anomaly_cyw1].stop_shutter_move_comm==OPEN_ONLY)
//		    			{
//		    				  gKeysStatus.bits.Wireless_Close_pressed = false;
//		    				  gKeysStatus.bits.Key_Close_pressed =  false;
//		    			}
//		    		}
//
//		    	}
//
//
//		    }
//
//		}
	/********* End of Extended Button Flags *************/
#endif	//	VERSION_2HARDWARE
}

#endif //#ifdef KEY_PRIORITY_LOGIC_ENABLED



#ifdef DEBOUNCE_WITH_PRIORITY_LOGIC // First attempt for priority logic. Not to be used as it is not considering the key press after some time

#define DEBOUNCE_DEBUG_COMMENTS

#define KEY_PRIORITY_DELAY	5	//	50 mS @10mS systick

void keysProcessFlags(uint8_t keyState, uint8_t keyChanged)
{
	uint8_t pressedState = keyState & ~(0xF8);
	static uint8_t keyStpPressedFirst;
	static uint8_t resetCountflag = 0;


	static uint8_t lsui8KeyOpenPressed = 0;
	static uint8_t lsui8KeyClosePressed = 0;
//	static uint8_t lsui8KeyStopPressed = 0;

	static uint8_t lsui8KeyOpenServiced = 0;
	static uint8_t lsui8KeyCloseServiced = 0;
	static uint8_t lsui8KeyStopServiced = 0;


	static uint32_t lsui32KeyCapturedTime = 0;

	/********** General Button Flags *********/
	//PE3_3PBS_OPEN
	if(
			(isPowerOfTwo(pressedState)) ||		//	Only open key is pressed
			(
					((KEY_3PBS_OPEN | KEY_3PBS_CLOSE) == pressedState) &&	//	Open + Close keys are pressed
					/*((KEY_3PBS_OPEN | KEY_3PBS_STOP) != pressedState) &&
					((KEY_3PBS_OPEN | KEY_3PBS_CLOSE | KEY_3PBS_STOP) != pressedState) &&	//	Stop key is not pressed*/
					(lsui8KeyClosePressed == 0) &&		//	Close key was not pressed within 10ms before open press
					(lsui8KeyCloseServiced == 0)		//	Close key is not already serviced
			)
	)
	{
		if(BUTTON_PRESSED(KEY_3PBS_OPEN, keyState, keyChanged))
		{
			if(lsui8KeyOpenPressed == 0)
			{
//				uartSendTxBuffer(UART_debug,"!",1);
				lsui8KeyOpenPressed = 1;
				lsui32KeyCapturedTime = g_ui32TickCount10ms;
			}

/*			if(!gKeysStatus.bits.Key_Open_pressed)
			{
				gKeysStatus.bits.Key_Open_pressed = true;
				gKeysStatus.bits.Key_Open_released = false;

				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 1;
				currentTickCount = g_ui32TickCount10ms;

				uartSendTxBuffer(UART_debug,"O",1);
			}*/
		}
	}
	//---PE3_3PBS_OPEN
//	if(!pressedState)
		if(
				(BUTTON_RELEASED(KEY_3PBS_OPEN, keyState, keyChanged)) &&
				(lsui8KeyOpenServiced == 1)
		)
		{
			if(!gKeysStatus.bits.Key_Open_released)
			{
				gKeysStatus.bits.Key_Open_released = true;
				gKeysStatus.bits.Key_Open_pressed = false;

				gKeysStatus.bits.Keys2_3secStpOpn_pressed = false;
				gKeysStatus.bits.Keys3_3secOpStCl_pressed = false;

				lsui8KeyOpenServiced = 0;

				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 0;
#ifdef DEBOUNCE_DEBUG_COMMENTS
				uartSendTxBuffer(UART_debug,"o",1);
#endif
			}
		}

	//PE1_3PBS_STOP
	if(
			(isPowerOfTwo(pressedState)) ||		//	Only stop key is pressed
			(
					((KEY_3PBS_OPEN | KEY_3PBS_STOP) == pressedState) ||	//	Open + Stop keys pressed
					((KEY_3PBS_CLOSE | KEY_3PBS_STOP) == pressedState) ||	//	Open + Stop keys pressed
					((KEY_3PBS_OPEN | KEY_3PBS_CLOSE | KEY_3PBS_STOP) == pressedState)	//	Open + Stop + Close keys pressed
			) &&
			(lsui8KeyOpenPressed == 0) &&		//	Open key was not pressed within 10ms before Stop key press
			(lsui8KeyOpenServiced == 0) &&		//	Open key is not already serviced
			(lsui8KeyClosePressed == 0) &&		//	Close key was not pressed within 10ms before Stop key press
			(lsui8KeyCloseServiced == 0)		//	Close key is not already serviced
	)
	{
		if(BUTTON_PRESSED(KEY_3PBS_STOP, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;

				lsui8KeyStopServiced = 1;

				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 1;

				currentTickCount = g_ui32TickCount10ms;

				uartSendTxBuffer(UART_debug,"S",1);
			}
		}
	}
	//---PE1_3PBS_STOP
//	if(!pressedState)
		if(
				(BUTTON_RELEASED(KEY_3PBS_STOP, keyState, keyChanged)) &&
				(lsui8KeyStopServiced == 1)
		)
		{

			if(!gKeysStatus.bits.Key_Stop_released)
			{
				gKeysStatus.bits.Key_Stop_released = true;
				gKeysStatus.bits.Key_Stop_pressed = false;

				gKeysStatus.bits.Key_3secStp_pressed = false;

				gKeysStatus.bits.Keys2_3secStpOpn_pressed = false;
				gKeysStatus.bits.Keys2_3secStpCls_pressed = false;

				gKeysStatus.bits.Keys3_3secOpStCl_pressed = false;

				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 0;
				keyStpPressedFirst = 0;

				lsui8KeyStopServiced = 0;

				uartSendTxBuffer(UART_debug,"s",1);
			}
		}

	//PE2_3PBS_CLOSE
	if(
			(isPowerOfTwo(pressedState))	//	Only close key is pressed
	)
	{
		if(BUTTON_PRESSED(KEY_3PBS_CLOSE, keyState, keyChanged))
		{
			if(lsui8KeyClosePressed == 0)
			{
				lsui8KeyClosePressed = 1;
				lsui32KeyCapturedTime = g_ui32TickCount10ms;
			}

			/*if(!gKeysStatus.bits.Key_Close_pressed)
			{
				currentTickCount = g_ui32TickCount10ms;

				gKeysStatus.bits.Key_Close_pressed = true;
				gKeysStatus.bits.Key_Close_released = false;

				gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 1;

				uartSendTxBuffer(UART_debug,"C",1);
			}*/
		}
	}
	//---PE2_3PBS_CLOSE
//	if(!pressedState)
		if(
				(BUTTON_RELEASED(KEY_3PBS_CLOSE, keyState, keyChanged)) &&
				(lsui8KeyCloseServiced == 1)
		)
		{
			if(!gKeysStatus.bits.Key_Close_released)
			{
				gKeysStatus.bits.Key_Close_released = true;
				gKeysStatus.bits.Key_Close_pressed = false;

				gKeysStatus.bits.Key_3secCls_pressed = false;
				gKeysStatus.bits.Keys3_3secOpStCl_pressed = false;
				gKeysStatus.bits.Keys2_3secStpCls_pressed = false;

				lsui8KeyCloseServiced = 0;

				gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 0;

				uartSendTxBuffer(UART_debug,"c",1);
			}
		}

	/********** End of General Button Flags *********/
	//If Key is released reset currentTickCount
	if(gKeysStatus.byte.HB_Released)
	{
//		currentTickCount = 0;
		resetCountflag = 0;
	}



	/********* Priority Button Flags *************/

	if(pressedState)
	{
		//	Open + Stop key check
		if(
				(
						(KEY_3PBS_OPEN == pressedState) ||					//	Only open key is pressed
						((KEY_3PBS_OPEN | KEY_3PBS_CLOSE) == pressedState)	//	Open + Close keys pressed
				) &&
				((KEY_3PBS_OPEN | KEY_3PBS_STOP) != pressedState) &&
				((g_ui32TickCount10ms - lsui32KeyCapturedTime) >= KEY_PRIORITY_DELAY) &&
				(gKeysStatus.bits.Key_Open_pressed == false) &&
				(lsui8KeyOpenPressed == 1)
		)
		{	//	Open key was pressed before stop key and stop key was not pressed
			//	within 20mS from open pressed event, declare open key press event
			lsui8KeyOpenPressed = 0;

			gKeysStatus.bits.Key_Open_pressed = true;
			gKeysStatus.bits.Key_Open_released = false;

			gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 1;
			currentTickCount = g_ui32TickCount10ms;

			lsui8KeyOpenServiced = 1;

			uartSendTxBuffer(UART_debug,"O",1);
		}
		else if(
				((KEY_3PBS_OPEN | KEY_3PBS_STOP) == pressedState) &&
				(lsui8KeyOpenPressed == 1) &&
				((g_ui32TickCount10ms - lsui32KeyCapturedTime) < KEY_PRIORITY_DELAY)
		)
		{	//	Open key was pressed first but within 20mS of open key press event
			//	stop key was also pressed, prioritize stop key and declare stop key
			//	press event
			lsui8KeyOpenPressed = 0;

			if(!gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;

				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 1;

				keyStpPressedFirst = 1;

				lsui8KeyStopServiced = 1;

				currentTickCount = g_ui32TickCount10ms;

				uartSendTxBuffer(UART_debug,"1",1);
			}
		}

		//	Close + Stop key check
		if(
				(KEY_3PBS_CLOSE == pressedState) &&
				((KEY_3PBS_CLOSE | KEY_3PBS_STOP) != pressedState) &&
				((KEY_3PBS_CLOSE | KEY_3PBS_OPEN) != pressedState) &&
				((g_ui32TickCount10ms - lsui32KeyCapturedTime) >= KEY_PRIORITY_DELAY) &&
				(gKeysStatus.bits.Key_Close_pressed == false) &&
				(lsui8KeyClosePressed == 1)
		)
		{	//	Close key was pressed before open key or stop key.
			//	Open or stop key or both open and stop keys were pressed after 20mS from
			//	close key press event.
			//	Declare close key pressed event
			lsui8KeyClosePressed = 0;

			currentTickCount = g_ui32TickCount10ms;

			lsui8KeyCloseServiced = 1;

			gKeysStatus.bits.Key_Close_pressed = true;
			gKeysStatus.bits.Key_Close_released = false;

			gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 1;

			uartSendTxBuffer(UART_debug,"C",1);
		}
		else if(
				((KEY_3PBS_CLOSE | KEY_3PBS_OPEN) == pressedState) &&
				((KEY_3PBS_CLOSE | KEY_3PBS_STOP) != pressedState) &&
				(lsui8KeyClosePressed == 1) &&
				((g_ui32TickCount10ms - lsui32KeyCapturedTime) < KEY_PRIORITY_DELAY)
		)
		{	//	Close key was pressed before open key or stop key.
			//	Open key was pressed before 20mS from close key press event.
			//	Declare open key pressed event
			lsui8KeyClosePressed = 0;

			if(!gKeysStatus.bits.Key_Open_pressed)
			{
				gKeysStatus.bits.Key_Open_pressed = true;
				gKeysStatus.bits.Key_Open_released = false;

				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 1;
				currentTickCount = g_ui32TickCount10ms;

				lsui8KeyOpenServiced = 1;

				uartSendTxBuffer(UART_debug,"2",1);
			}

		}
		else if(
				(
						((KEY_3PBS_CLOSE | KEY_3PBS_STOP) == pressedState) ||
						((KEY_3PBS_CLOSE | KEY_3PBS_STOP | KEY_3PBS_OPEN) == pressedState)
				) &&
				(lsui8KeyClosePressed == 1) &&
				((g_ui32TickCount10ms - lsui32KeyCapturedTime) < KEY_PRIORITY_DELAY)
		)
		{	//	Close key was pressed before open key or stop key.
			//	Stop key was pressed before 20mS from close key press event.
			//	Declare stop key pressed event.
			lsui8KeyClosePressed = 0;

			if(!gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;

				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 1;

				keyStpPressedFirst = 1;

				lsui8KeyStopServiced = 1;

				currentTickCount = g_ui32TickCount10ms;

				uartSendTxBuffer(UART_debug,"3",1);
			}
		}
	}

	/********* Priority Button Flags End here *************/



	/********* Extended Button Flags *************/
	if(pressedState)
	{
		if(pressedState == _ONLY_STOPKEY_PRESSED)//check in upper section only
			if((pressedState == keyChanged))
				keyStpPressedFirst = 1;

		if(
				(
						(KEY_3PBS_CLOSE == pressedState) &&
						(lsui8KeyCloseServiced == 1)
				) &&
				((g_ui32TickCount10ms - currentTickCount) >= SYSTICK_3SEC_KEYPRESS) &&
				(gKeysStatus.bits.Key_3secCls_pressed == false) /*&&
				(gKeysStatus.bits.Key_Close_pressed == true)*/
		)
		{
			gKeysStatus.bits.Key_3secCls_pressed = true;

			uartSendTxBuffer(UART_debug,"X",1);
		}

		//Stop pressed for 3 sec //Added 4 Jun 2014
		if(
				(KEY_3PBS_STOP == pressedState) &&
				((g_ui32TickCount10ms - currentTickCount) >= SYSTICK_3SEC_KEYPRESS) &&
				(gKeysStatus.bits.Key_3secStp_pressed == false) /*&&
				(gKeysStatus.bits.Key_Stop_pressed == true)*/
		)
		{
			//Check in application the current mode of menu
			//Inspect for Close pressed flags!
			gKeysStatus.bits.Key_3secStp_pressed = true;

			uartSendTxBuffer(UART_debug,"Y",1);
		}


		if(!isPowerOfTwo(pressedState)) { //Check if multiple keys are pressed

			if(!resetCountflag) {
//				currentTickCount = g_ui32TickCount10ms;
				resetCountflag = ~0;
			}

			//Stop + Open pressed for 3 sec
			if(
					((KEY_3PBS_OPEN | KEY_3PBS_STOP) == pressedState) &&
					(keyStpPressedFirst) &&
					((g_ui32TickCount10ms - currentTickCount) >= SYSTICK_3SEC_KEYPRESS) &&
					(gKeysStatus.bits.Keys2_3secStpOpn_pressed == false)
			)
			{
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpOpn_pressed = true;

				uartSendTxBuffer(UART_debug,"Z",1);
			}

			//Stop + Close pressed for 3 sec
			if(
					((KEY_3PBS_CLOSE | KEY_3PBS_STOP) == pressedState) &&
					(keyStpPressedFirst) &&
					((g_ui32TickCount10ms - currentTickCount) >= SYSTICK_3SEC_KEYPRESS) &&
					(gKeysStatus.bits.Keys2_3secStpCls_pressed == false)
			)
			{
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpCls_pressed = true;

				uartSendTxBuffer(UART_debug,"W",1);
			}

			//Open + Stop + Close pressed for 3 sec
			if(
					(INSTALLATION_KEYS == pressedState) &&
					((g_ui32TickCount10ms - currentTickCount) >= SYSTICK_3SEC_KEYPRESS) &&
					(gKeysStatus.bits.Keys3_3secOpStCl_pressed == false)
			)
			{
				//				currentTickCount = 0;
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys3_3secOpStCl_pressed = true;

				uartSendTxBuffer(UART_debug,"V",1);
			}
		}
	}
	/********* End of Extended Button Flags *************/
}
#endif
#endif //-----CTRL_TARGET_BOARD


/******************************************************************************
 * Function Name: isPowerOfTwo
 *
 * Function Description: This function checks whether byte is a power of 2
 *
 * Function Parameters: x - byte to detect
 *
 * Function Returns: 1 if input is power of two else returns 0
 *
 ********************************************************************************/
uint8_t isPowerOfTwo(uint8_t x)
{

	uint8_t temp = x - 1;
	temp &= x;
	temp = !temp;
	if(temp && x)
		return 1;
	else
		return 0;


/*
	uint8_t i,j = 0x01,k = 0;

	for (i = 0;i < 8;i++)
	{

		if (x & j)
		{
			k++;
		}

		j = j << 1;

	}

	if (k == 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
*/

}
//*****************************************************************************


/******************************************************************************
 * Function Name: 			ValidateDebounce
 *
 * Function Description: 	Call function to validate the debounce occurrence count for pressed or released events
 * 							with configured individual debounce period
 * 							Function will also apply Snow Mode logic to only one signal defined by input parameter 'lucIndexSnowModeSensor'
 *
 * Function Parameters:		lucIndexSnowModeSensor
 * 							0 - 7 : signal defined by index to be consider for snow mode
 * 							0xFF  : do not consider
 *
 * Function Returns: 		Function will return the status which will indicate the signal which fulfill the debounce limit
 *
 ********************************************************************************/
uint32_t ValidateDebounce(uint32_t luintDelta,uint32_t luintLastState,uint8_t *lpCounter,uint8_t *lpPressedDebounceLimit,uint8_t *lpReleasedDebounceLimit, uint8_t lucIndexSnowModeSensor)
{

	uint32_t luiScanBits;
	uint8_t lucLoop,lucLoop1;
	uint32_t luiRetDelta;
	uint16_t luiCounter;
	uint16_t luiSnowModeTiming;

	luiRetDelta = 0;
	luiScanBits = 1;


	//if(gSensorStatus.bits.Sensor_Safety_active == false)
	//{
	//		luiCounter = 0;
	//}
	// Scan 'luintDelta' for variable debounce time elapsed for press and release events
	for (lucLoop = 0 ; lucLoop < 8; lucLoop++)
	{

		if (luintDelta & luiScanBits)		   // check for change in last state and current state
		{
			luiCounter = 0;

			for (lucLoop1 = 0 ; lucLoop1 < DEBOUNCE_COUNTER_BIT_COUNT; lucLoop1++) // scan specific bits of counter to get the current value (counter is distributed across 6 variables)
			{

				luiCounter = luiCounter << 1;

				if ((*(lpCounter + lucLoop1)) & luiScanBits) // lpCounter will point to MSB to LSB
				{

					luiCounter = luiCounter | 0x01;

				}

			} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)

			// calculate sensor scanning time in case of snow mode
			if (gu8_snow_mode != 0 && lucIndexSnowModeSensor != 0xFF && lucLoop == lucIndexSnowModeSensor)
			{
				if (gu8_snow_mode == 1)
				{
					luiSnowModeTiming =250;// 600;//250; // 0.25 sec    20160919 bug_No
				}
				else
				{
					luiSnowModeTiming = 500;//2000;//500; //0.5 sec     20160919 bug_No
				}

			}

			if (luintLastState & luiScanBits) // Validate key press debounce
			{

				// Handle Snow mode scanning logic
				if (gu8_snow_mode != 0 && lucIndexSnowModeSensor != 0xFF && lucLoop == lucIndexSnowModeSensor)
				{

					if (luiCounter == luiSnowModeTiming)
					{

						luiRetDelta = luiRetDelta | luiScanBits;

					}

				}
				else
				{

					if (luiCounter == *(lpPressedDebounceLimit + lucLoop))
					{

						luiRetDelta = luiRetDelta | luiScanBits;

					}

				}

			}
			else // Validate key release debounce
			{

				// Handle Snow mode scanning logic
				if (gu8_snow_mode != 0 && lucIndexSnowModeSensor != 0xFF && lucLoop == lucIndexSnowModeSensor)
				{

					if (luiCounter == luiSnowModeTiming)
					{

						luiRetDelta = luiRetDelta | luiScanBits;

					}

				}
				else
				{

					if (luiCounter == *(lpReleasedDebounceLimit + lucLoop))
					{

						luiRetDelta = luiRetDelta | luiScanBits;

					}

				}

			}

		} //if (luintDelta & luiScanBits)

		luiScanBits = luiScanBits << 1;

	} //for (lucLoop = 0 ; lucLoop < 32; lucLoop++)

	// Reset the debounce counter for signals whose debounce time is elapsed
	luiScanBits = 1;

	for (lucLoop = 0 ; lucLoop < 8; lucLoop++)
	{

		if (luiRetDelta & luiScanBits)
		{

			// Reset
			for (lucLoop1 = 0 ; lucLoop1 < DEBOUNCE_COUNTER_BIT_COUNT; lucLoop1++)
			{

				*(lpCounter + lucLoop1) = *(lpCounter + lucLoop1) & (~luiScanBits);

			} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)

		}

	luiScanBits = luiScanBits << 1;

	} //for (lucLoop = 0 ; lucLoop < 32; lucLoop++)


return luiRetDelta;

} //ValidateDebounce


/******************************************************************************
 * Function Name: 			CheckTempReleaseAndResetClock
 *
 * Function Description: 	The function is used to reset the counter which is used to count debounce period when there is no change in last state and current state of input like sensors/keys
 * 							The function also support feature to support temporary release before reseting the counter
 *
 * Function Parameters:
 *
 * Function Returns: 		void
 *
 ********************************************************************************/
void CheckTempReleaseAndResetClock (uint8_t cucTempReleaseMaxLimit, uint8_t *lpCounter,uint32_t ui32Delta)
{
	uint32_t luiScanBits;
	uint8_t lucScanBits1;
	uint8_t lucLoop,lucLoop1;
	uint16_t luiCounter;

	static uint8_t ui8TempReleaseCounter[3] = {0,0,0}; // 0 = MSB ....2 = LSB



	if (cucTempReleaseMaxLimit == 0)
	{
		*lpCounter = *lpCounter & ui32Delta;
		*(lpCounter + 1) = *(lpCounter + 1) & ui32Delta;
		*(lpCounter + 2) = *(lpCounter + 2) & ui32Delta;
		*(lpCounter + 3) = *(lpCounter + 3) & ui32Delta;
		*(lpCounter + 4) = *(lpCounter + 4) & ui32Delta;
		*(lpCounter + 5) = *(lpCounter + 5) & ui32Delta;
		*(lpCounter + 6) = *(lpCounter + 6) & ui32Delta;
		*(lpCounter + 7) = *(lpCounter + 7) & ui32Delta;
		*(lpCounter + 8) = *(lpCounter + 8) & ui32Delta;
		*(lpCounter + 9) = *(lpCounter + 9) & ui32Delta;
	}
	else
	{

		luiScanBits = 1;

		for (lucLoop = 0 ; lucLoop < 8; lucLoop++) // scan counter alloctaed to every input
		{

				luiCounter = 0;

				for (lucLoop1 = 0 ; lucLoop1 < DEBOUNCE_COUNTER_BIT_COUNT; lucLoop1++) // scan specific bits of counter to get the current value (counter is distributed across 6 variables)
				{

					luiCounter = luiCounter << 1;

					if ((*(lpCounter + lucLoop1)) & luiScanBits) // lpCounter will point to MSB to LSB
					{

						luiCounter = luiCounter | 0x01;

					}

				} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)

				// Check for No press condition (lpCounter == 1, ui32Delta == 0) and reset the lpCounter, 'Local Temp release Counter'
				if ((luiCounter == 1) && ((ui32Delta & luiScanBits) == 0))
				{
					*lpCounter = *lpCounter & (~luiScanBits);
					*(lpCounter + 1) = *(lpCounter + 1) & (~luiScanBits);
					*(lpCounter + 2) = *(lpCounter + 2) & (~luiScanBits);
					*(lpCounter + 3) = *(lpCounter + 3) & (~luiScanBits);
					*(lpCounter + 4) = *(lpCounter + 4) & (~luiScanBits);
					*(lpCounter + 5) = *(lpCounter + 5) & (~luiScanBits);
					*(lpCounter + 6) = *(lpCounter + 6) & (~luiScanBits);
					*(lpCounter + 7) = *(lpCounter + 7) & (~luiScanBits);
					*(lpCounter + 8) = *(lpCounter + 8) & (~luiScanBits);
					*(lpCounter + 9) = *(lpCounter + 9) & (~luiScanBits);

					ui8TempReleaseCounter[0] &= (~luiScanBits);
					ui8TempReleaseCounter[1] &= (~luiScanBits);
					ui8TempReleaseCounter[2] &= (~luiScanBits);
				}
				// Check for Temp Released recovered within allowed limit (lpCounter > 0,ui32Delta == 1) and reset 'Local Temp release Counter'
				else if ((luiCounter > 1) && (ui32Delta & luiScanBits))
				{

					ui8TempReleaseCounter[0] &= (~luiScanBits);
					ui8TempReleaseCounter[1] &= (~luiScanBits);
					ui8TempReleaseCounter[2] &= (~luiScanBits);

				}
				// Check for Temp release condition (lpCounter > 1, ui32Delta == 0), decrement 'lpCounter', increment 'Local Temp release Counter'
				// If 'Local Temp release Counter' reches to 'cucTempReleaseMaxLimit', then reset 'lpCounter', 'Local Temp release Counter'
				else if ((luiCounter > 1) && ((ui32Delta & luiScanBits) == 0))
				{

					//decrement 'lpCounter'...................................................................

						// Decrement the local counter value
						luiCounter = luiCounter - 1;

						// Reset the 'lpCounter'
						*lpCounter = *lpCounter & (~luiScanBits);	//0 = MSB ....5 = LSB
						*(lpCounter + 1) = *(lpCounter + 1) & (~luiScanBits);
						*(lpCounter + 2) = *(lpCounter + 2) & (~luiScanBits);
						*(lpCounter + 3) = *(lpCounter + 3) & (~luiScanBits);
						*(lpCounter + 4) = *(lpCounter + 4) & (~luiScanBits);
						*(lpCounter + 5) = *(lpCounter + 5) & (~luiScanBits);
						*(lpCounter + 6) = *(lpCounter + 6) & (~luiScanBits);
						*(lpCounter + 7) = *(lpCounter + 7) & (~luiScanBits);
						*(lpCounter + 8) = *(lpCounter + 8) & (~luiScanBits);
						*(lpCounter + 9) = *(lpCounter + 9) & (~luiScanBits);

						// set the new value in 'lpCounter'
						lucScanBits1 = 0x20;	//0b00100000;
						for (lucLoop1 = 0 ; lucLoop1 < DEBOUNCE_COUNTER_BIT_COUNT; lucLoop1++)
						{

							if (luiCounter & lucScanBits1)
							{

								*(lpCounter + lucLoop1) = *(lpCounter + lucLoop1) | luiScanBits;

							}

							lucScanBits1 = lucScanBits1 >> 1;

						} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)

					//increment 'Local Temp release Counter' .................................................
						// compute curent value of 'Local Temp release Counter'
						luiCounter = 0;

						for (lucLoop1 = 0 ; lucLoop1 < 3; lucLoop1++) // scan specific bits of temp release counter to get the current value (counter is distributed across 3 variables)
						{

							luiCounter = luiCounter << 1;

							if ((*(ui8TempReleaseCounter + lucLoop1)) & luiScanBits) // lpCounter will point to MSB to LSB
							{

								luiCounter = luiCounter | 0x01;

							}

						} //for (lucLoop1 = 0 ; lucLoop1 < DEBOUNCE_COUNTER_BIT_COUNT; lucLoop1++)

						luiCounter = luiCounter + 1;

						// Reset the 'Local Temp release Counter'
						*ui8TempReleaseCounter = *ui8TempReleaseCounter & (~luiScanBits);	//0 = MSB ....5 = LSB
						*(ui8TempReleaseCounter + 1) = *(ui8TempReleaseCounter + 1) & (~luiScanBits);
						*(ui8TempReleaseCounter + 2) = *(ui8TempReleaseCounter + 2) & (~luiScanBits);

						// set the new value in 'Local Temp release Counter'
						lucScanBits1 = 0x04;	//0b00000100;
						for (lucLoop1 = 0 ; lucLoop1 < 3; lucLoop1++)
						{

							if (luiCounter & lucScanBits1)
							{

								*(ui8TempReleaseCounter + lucLoop1) = *(ui8TempReleaseCounter + lucLoop1) | luiScanBits;

							}

							lucScanBits1 = lucScanBits1 >> 1;

						} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)


				    // If 'Local Temp release Counter' reches to 'cucTempReleaseMaxLimit', then reset 'lpCounter', 'Local Temp release Counter'

				    	// compute curent value of 'Local Temp release Counter'
						luiCounter = 0;

						for (lucLoop1 = 0 ; lucLoop1 < 3; lucLoop1++) // scan specific bits of temp release counter to get the current value (counter is distributed across 3 variables)
						{

							luiCounter = luiCounter << 1;

							if ((*(ui8TempReleaseCounter + lucLoop1)) & luiScanBits) // lpCounter will point to MSB to LSB
							{

								luiCounter = luiCounter | 0x01;

							}

						} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)


					if (luiCounter > cucTempReleaseMaxLimit)
					{

						// Reset the 'lpCounter'
						*lpCounter = *lpCounter & (~luiScanBits);	//0 = MSB ....5 = LSB
						*(lpCounter + 1) = *(lpCounter + 1) & (~luiScanBits);
						*(lpCounter + 2) = *(lpCounter + 2) & (~luiScanBits);
						*(lpCounter + 3) = *(lpCounter + 3) & (~luiScanBits);
						*(lpCounter + 4) = *(lpCounter + 4) & (~luiScanBits);
						*(lpCounter + 5) = *(lpCounter + 5) & (~luiScanBits);
						*(lpCounter + 6) = *(lpCounter + 6) & (~luiScanBits);
						*(lpCounter + 7) = *(lpCounter + 7) & (~luiScanBits);
						*(lpCounter + 8) = *(lpCounter + 8) & (~luiScanBits);
						*(lpCounter + 9) = *(lpCounter + 9) & (~luiScanBits);

						// Reset the 'Local Temp release Counter'
						ui8TempReleaseCounter[0] &= (~luiScanBits);
						ui8TempReleaseCounter[1] &= (~luiScanBits);
						ui8TempReleaseCounter[2] &= (~luiScanBits);

					} //if (luiCounter > cucTempReleaseMaxLimit)

				} //else if ((luiCounter > 1) && ((ui32Delta & luiScanBits) == 0))

			luiScanBits = luiScanBits << 1;

		} //for (lucLoop = 0 ; lucLoop < 32; lucLoop++)

	} // else of if (cucTempReleaseMaxLimit == 0)

} //void CheckTempReleaseAndResetClock (uint8_t cucTempReleaseMaxLimit, uint8_t *lpCounter,uint32_t ui32Delta)

//*****************************************************************************

//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
