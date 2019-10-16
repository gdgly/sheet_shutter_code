/*********************************************************************************
* FileName: debounce.c
* Description:
* This header file contains definitions for Key Debounce module mechanism.
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
 *  	0.2D	19/06/2014									  (STOP + currentTickcount) issue solved.
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
#include <sysctl.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>
#include "debounce.h"
#include "Drivers/systicktimer.h"
#include <stdio.h>

#include "Middleware/serial.h"
#include "Application/userinterface.h"
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
#define AUTO_MANUAL_KEY_DEBOUNCE_ACTIVE_EVENT  			20
#define AUTO_MANUAL_KEY_DEBOUNCE_INACTIVE_EVENT 		20

#define OPEN_KEY_DEBOUNCE_ACTIVE_EVENT  				20
#define OPEN_KEY_DEBOUNCE_INACTIVE_EVENT 				20

#define STOP_KEY_DEBOUNCE_ACTIVE_EVENT  				20
#define STOP_KEY_DEBOUNCE_INACTIVE_EVENT 				20

#define CLOSE_KEY_DEBOUNCE_ACTIVE_EVENT  				20
#define CLOSE_KEY_DEBOUNCE_INACTIVE_EVENT 				20

#define UP_KEY_DEBOUNCE_ACTIVE_EVENT  					20
#define UP_KEY_DEBOUNCE_INACTIVE_EVENT 					20

#define DOWN_KEY_DEBOUNCE_ACTIVE_EVENT  				20
#define DOWN_KEY_DEBOUNCE_INACTIVE_EVENT 				20

#define MODE_KEY_DEBOUNCE_ACTIVE_EVENT  				20
#define MODE_KEY_DEBOUNCE_INACTIVE_EVENT 				20

#define ENTER_KEY_DEBOUNCE_ACTIVE_EVENT  				20
#define ENTER_KEY_DEBOUNCE_INACTIVE_EVENT 				20


// Parameter for Max Temporary Release, 1msec * value
// RANGE : 0 - 3
#define	TEMPORARY_RELEASE_MAX_LIMIT						0

// ****************************************************************

/****************************************************************************
 *  function declaration
****************************************************************************/
#ifdef VARIABLE_DEBOUNCE_LOGIC
uint32_t ValidateDebounce(uint32_t luintDelta,uint32_t luintLastState,uint8_t *lpCounter,uint8_t *lpPressedDebounceLimit,uint8_t *lpReleasedDebounceLimit);

void CheckTempReleaseAndResetClock (uint8_t cucTempReleaseMaxLimit, uint8_t *lpCounter,uint32_t ui32Delta);

#endif

void Set_lcdlightON(void);
uint8_t KEY_PRESS_3SEC_ENT_FLAG_CYW=0;
uint8_t KEY_PRESS_3SEC_ENT_FORRESET_CYW=0;
uint8_t KEY_PRESS_3SEC_MODE_FLAG_CYW=0;
uint8_t KEY_PRESS_3SEC_STOP_FLAG_CYW=0;
/****************************************************************************
 *  Constant definitions
****************************************************************************/
#ifdef VARIABLE_DEBOUNCE_LOGIC
const uint8_t cucPressDebounceLimit[32] =
{
AUTO_MANUAL_KEY_DEBOUNCE_ACTIVE_EVENT,
OPEN_KEY_DEBOUNCE_ACTIVE_EVENT,
STOP_KEY_DEBOUNCE_ACTIVE_EVENT,
CLOSE_KEY_DEBOUNCE_ACTIVE_EVENT,
UP_KEY_DEBOUNCE_ACTIVE_EVENT,
DOWN_KEY_DEBOUNCE_ACTIVE_EVENT,
MODE_KEY_DEBOUNCE_ACTIVE_EVENT,
ENTER_KEY_DEBOUNCE_ACTIVE_EVENT,
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
AUTO_MANUAL_KEY_DEBOUNCE_INACTIVE_EVENT,
OPEN_KEY_DEBOUNCE_INACTIVE_EVENT,
STOP_KEY_DEBOUNCE_INACTIVE_EVENT,
CLOSE_KEY_DEBOUNCE_INACTIVE_EVENT,
UP_KEY_DEBOUNCE_INACTIVE_EVENT,
DOWN_KEY_DEBOUNCE_INACTIVE_EVENT,
MODE_KEY_DEBOUNCE_INACTIVE_EVENT,
ENTER_KEY_DEBOUNCE_INACTIVE_EVENT,
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
// ***************************************************************************************************************************************************

#endif

/****************************************************************************
 *  Global variables
****************************************************************************/
//static uint8_t g_ui8ButtonStates = ALL_BUTTONS;
_KEYS_STATUS gKeysStatus;
static uint32_t currentTickCount = 0;
uint32_t currentTickCount_ent_cyw=0;
uint32_t currentTickCount_mode_cyw=0;
uint32_t currentTickCount_stop_cyw=0;
uint32_t historyTickCount_cyw=0;

#ifdef DISP_TARGET_BOARD
static uint8_t g_ui8ButtonStates;
#endif

#ifdef DISP_LAUNCHPAD_BOARD
static uint8_t g_ui8ButtonStates_F = ALL_BUTTONS_F;
static uint8_t g_ui8ButtonStates_D = ALL_BUTTONS_D;
static uint8_t g_ui8ButtonStates_E = ALL_BUTTONS_E;
#endif
/****************************************************************************/


/****************************************************************************
 *  Function definations for this file:
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
#ifdef DISP_TARGET_BOARD

#ifdef VARIABLE_DEBOUNCE_LOGIC
void keysPoll(uint8_t *pui8Delta, uint8_t *pui8RawState)
{
    uint32_t ui32Delta;
    uint32_t ui32Data,ui32Data_cyw;

    /*
    static uint8_t ui8SwitchClockA = 0; //LSB
    static uint8_t ui8SwitchClockB = 0;
    static uint8_t ui8SwitchClockC = 0;
    static uint8_t ui8SwitchClockD = 0;
    static uint8_t ui8SwitchClockE = 0;
    static uint8_t ui8SwitchClockF = 0; //MSB
    */

    static uint8_t ui8SwitchClock[6] = {0,0,0,0,0,0}; // 0 = MSB ....5 = LSB

    //const uint8_t *pucClock[6] = {&ui8SwitchClockF,&ui8SwitchClockE,&ui8SwitchClockD,&ui8SwitchClockC,&ui8SwitchClockB,&ui8SwitchClockA};

    //
    // Read the raw state of the push buttons.  Save the raw state
    // (inverting the bit sense) if the caller supplied storage for the
    // raw value.
    //
    ui32Data = (MAP_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS));

   // ui32Data_cyw =~ui32Data;//  priority  stop>open>close  cear low
  //     if(ui32Data_cyw&0x04)
   //   {
   //   	ui32Data_cyw&=~(0xa);
   //   }
   //   if(ui32Data_cyw&0x02)
   //   {
    // 	ui32Data_cyw&=~(0x8);
   //   }
   //   ui32Data=~ui32Data_cyw;

    //uint8_t ltempdebug = (uint8_t)ui32Data;
    //uartSendTxBuffer(UART_debug,&ltempdebug,1);

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
/*
    ui8SwitchClockF ^= (ui8SwitchClockA & ui8SwitchClockB & ui8SwitchClockC & ui8SwitchClockD & ui8SwitchClockE);
    ui8SwitchClockE ^= (ui8SwitchClockA & ui8SwitchClockB & ui8SwitchClockC & ui8SwitchClockD);
    ui8SwitchClockD ^= (ui8SwitchClockA & ui8SwitchClockB & ui8SwitchClockC);
    ui8SwitchClockC ^= (ui8SwitchClockA & ui8SwitchClockB);
    ui8SwitchClockB ^= ui8SwitchClockA;
    ui8SwitchClockA = ~ui8SwitchClockA;
*/
    ui8SwitchClock[0] ^= (ui8SwitchClock[5] & ui8SwitchClock[4] & ui8SwitchClock[3] & ui8SwitchClock[2] & ui8SwitchClock[1]);
    ui8SwitchClock[1] ^= (ui8SwitchClock[5] & ui8SwitchClock[4] & ui8SwitchClock[3] & ui8SwitchClock[2]);
    ui8SwitchClock[2] ^= (ui8SwitchClock[5] & ui8SwitchClock[4] & ui8SwitchClock[3]);
    ui8SwitchClock[3] ^= (ui8SwitchClock[5] & ui8SwitchClock[4]);
    ui8SwitchClock[4] ^= ui8SwitchClock[5];
    ui8SwitchClock[5] = ~ui8SwitchClock[5];

    //
    // Reset the clocks corresponding to switches that have not changed state.
    //
/*
    ui8SwitchClockA &= ui32Delta;
    ui8SwitchClockB &= ui32Delta;
    ui8SwitchClockC &= ui32Delta;
    ui8SwitchClockD &= ui32Delta;
    ui8SwitchClockE &= ui32Delta;
    ui8SwitchClockF &= ui32Delta;
*/

/*
    ui8SwitchClock[0] &= ui32Delta;
    ui8SwitchClock[1] &= ui32Delta;
    ui8SwitchClock[2] &= ui32Delta;
    ui8SwitchClock[3] &= ui32Delta;
    ui8SwitchClock[4] &= ui32Delta;
    ui8SwitchClock[5] &= ui32Delta;
*/

    CheckTempReleaseAndResetClock (cucTempReleaseMaxLimit, (uint8_t *) ui8SwitchClock,ui32Delta);

/*
    //
    // Get the new debounced switch state.
    //
    g_ui8ButtonStates &= ui8SwitchClockA | ui8SwitchClockB;
    g_ui8ButtonStates |= (~(ui8SwitchClockA | ui8SwitchClockB)) & ui32Data;

    //
    // Determine the switches that just changed debounced state.
    //
    ui32Delta ^= (ui8SwitchClockA | ui8SwitchClockB);
*/

    //
    // Get the new debounced switch state.
    // Determine the switches that just changed debounced state.
    //
    ui32Delta =  ValidateDebounce(ui32Delta,g_ui8ButtonStates,(uint8_t *) ui8SwitchClock,(uint8_t *)cucPressDebounceLimit,(uint8_t *)cucReleaseDebounceLimit);
    g_ui8ButtonStates = g_ui8ButtonStates & (~ui32Delta);
    g_ui8ButtonStates = g_ui8ButtonStates | (ui32Data & ui32Delta);


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
#endif

#ifndef VARIABLE_DEBOUNCE_LOGIC
void keysPoll(uint8_t *pui8Delta, uint8_t *pui8RawState)
{
    uint32_t ui32Delta;
    uint32_t ui32Data;
    static uint8_t ui8SwitchClockA = 0;
    static uint8_t ui8SwitchClockB = 0;

    //
    // Read the raw state of the push buttons.  Save the raw state
    // (inverting the bit sense) if the caller supplied storage for the
    // raw value.
    //
    ui32Data = (MAP_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS));
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
    ui8SwitchClockA ^= ui8SwitchClockB;
    ui8SwitchClockB = ~ui8SwitchClockB;

    //
    // Reset the clocks corresponding to switches that have not changed state.
    //
    ui8SwitchClockA &= ui32Delta;
    ui8SwitchClockB &= ui32Delta;

    //
    // Get the new debounced switch state.
    //
    g_ui8ButtonStates &= ui8SwitchClockA | ui8SwitchClockB;
    g_ui8ButtonStates |= (~(ui8SwitchClockA | ui8SwitchClockB)) & ui32Data;

    //
    // Determine the switches that just changed debounced state.
    //
    ui32Delta ^= (ui8SwitchClockA | ui8SwitchClockB);

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
#endif

#endif //-----DISP_TARGET_BOARD

#ifdef DISP_LAUNCHPAD_BOARD
void keysPoll_F(uint8_t *pui8Delta, uint8_t *pui8RawState)
{
    uint32_t ui32Delta;
    uint32_t ui32Data;
    static uint8_t ui8SwitchClockA = 0;
    static uint8_t ui8SwitchClockB = 0;

    //
    // Read the raw state of the push buttons.  Save the raw state
    // (inverting the bit sense) if the caller supplied storage for the
    // raw value.
    //
    ui32Data = (MAP_GPIOPinRead(BUTTONS_GPIO_BASE_F, ALL_BUTTONS_F));
    if(pui8RawState)
    {
        *pui8RawState = (uint8_t)~ui32Data;
    }

    //
    // Determine the switches that are at a different state than the debounced
    // state.
    //
    ui32Delta = ui32Data ^ g_ui8ButtonStates_F;

    //
    // Increment the clocks by one.
    //
    ui8SwitchClockA ^= ui8SwitchClockB;
    ui8SwitchClockB = ~ui8SwitchClockB;

    //
    // Reset the clocks corresponding to switches that have not changed state.
    //
    ui8SwitchClockA &= ui32Delta;
    ui8SwitchClockB &= ui32Delta;

    //
    // Get the new debounced switch state.
    //
    g_ui8ButtonStates_F &= ui8SwitchClockA | ui8SwitchClockB;
    g_ui8ButtonStates_F |= (~(ui8SwitchClockA | ui8SwitchClockB)) & ui32Data;

    //
    // Determine the switches that just changed debounced state.
    //
    ui32Delta ^= (ui8SwitchClockA | ui8SwitchClockB);

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
    keysProcessFlags_F(~g_ui8ButtonStates_F, *pui8Delta);
}

void keysPoll_D(uint8_t *pui8Delta, uint8_t *pui8RawState)
{
    uint32_t ui32Delta;
    uint32_t ui32Data;
    static uint8_t ui8SwitchClockA = 0;
    static uint8_t ui8SwitchClockB = 0;

    //
    // Read the raw state of the push buttons.  Save the raw state
    // (inverting the bit sense) if the caller supplied storage for the
    // raw value.
    //
    ui32Data = (MAP_GPIOPinRead(BUTTONS_GPIO_BASE_D, ALL_BUTTONS_D));
    if(pui8RawState)
    {
        *pui8RawState = (uint8_t)~ui32Data;
    }

    //
    // Determine the switches that are at a different state than the debounced
    // state.
    //
    ui32Delta = ui32Data ^ g_ui8ButtonStates_D;

    //
    // Increment the clocks by one.
    //
    ui8SwitchClockA ^= ui8SwitchClockB;
    ui8SwitchClockB = ~ui8SwitchClockB;

    //
    // Reset the clocks corresponding to switches that have not changed state.
    //
    ui8SwitchClockA &= ui32Delta;
    ui8SwitchClockB &= ui32Delta;

    //
    // Get the new debounced switch state.
    //
    g_ui8ButtonStates_D &= ui8SwitchClockA | ui8SwitchClockB;
    g_ui8ButtonStates_D |= (~(ui8SwitchClockA | ui8SwitchClockB)) & ui32Data;

    //
    // Determine the switches that just changed debounced state.
    //
    ui32Delta ^= (ui8SwitchClockA | ui8SwitchClockB);

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
    keysProcessFlags_D(~g_ui8ButtonStates_D, *pui8Delta);
}

void keysPoll_E(uint8_t *pui8Delta, uint8_t *pui8RawState)
{
    uint32_t ui32Delta;
    uint32_t ui32Data;
    static uint8_t ui8SwitchClockA = 0;
    static uint8_t ui8SwitchClockB = 0;

    //
    // Read the raw state of the push buttons.  Save the raw state
    // (inverting the bit sense) if the caller supplied storage for the
    // raw value.
    //
    ui32Data = (MAP_GPIOPinRead(BUTTONS_GPIO_BASE_E, ALL_BUTTONS_E));
    if(pui8RawState)
    {
        *pui8RawState = (uint8_t)~ui32Data;
    }

    //
    // Determine the switches that are at a different state than the debounced
    // state.
    //
    ui32Delta = ui32Data ^ g_ui8ButtonStates_E;

    //
    // Increment the clocks by one.
    //
    ui8SwitchClockA ^= ui8SwitchClockB;
    ui8SwitchClockB = ~ui8SwitchClockB;

    //
    // Reset the clocks corresponding to switches that have not changed state.
    //
    ui8SwitchClockA &= ui32Delta;
    ui8SwitchClockB &= ui32Delta;

    //
    // Get the new debounced switch state.
    //
    g_ui8ButtonStates_E &= ui8SwitchClockA | ui8SwitchClockB;
    g_ui8ButtonStates_E |= (~(ui8SwitchClockA | ui8SwitchClockB)) & ui32Data;

    //
    // Determine the switches that just changed debounced state.
    //
    ui32Delta ^= (ui8SwitchClockA | ui8SwitchClockB);

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
    keysProcessFlags_E(~g_ui8ButtonStates_E, *pui8Delta);
}
#endif //-------DISP_LAUNCHPAD_BOARD

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
#ifdef DISP_TARGET_BOARD
void initKeys(void)
{
    // Enable the GPIO port D to which the pushbuttons are connected.
    MAP_SysCtlPeripheralEnable(BUTTONS_GPIO_PERIPH); //PortD
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 0x4C4F434B;
	HWREG(GPIO_PORTD_BASE + GPIO_O_CR) = 0x80;
	HWREG(GPIO_PORTD_BASE + GPIO_O_LOCK) = 45;

    // Set each of the button GPIO pins as an input with a pull-up.
    MAP_GPIODirModeSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_DIR_MODE_IN);
    MAP_GPIOPadConfigSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);

    // Initialize the debounced button state with the current state read from
    // the GPIO bank.
    g_ui8ButtonStates = MAP_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);

    g_ui8ButtonStates = 0xFF;

    gKeysStatus.Val = 0;
}
#endif //-----DISP_TARGET_BOARD

#ifdef DISP_LAUNCHPAD_BOARD
void initKeys(void)
{
    //
    // Enable the GPIO port to which the pushbuttons are connected.
    //
    //MAP_SysCtlPeripheralEnable(BUTTONS_GPIO_PERIPH);
	MAP_SysCtlPeripheralEnable(BUTTONS_GPIO_PERIPH_F);
	MAP_SysCtlPeripheralEnable(BUTTONS_GPIO_PERIPH_D);
	MAP_SysCtlPeripheralEnable(BUTTONS_GPIO_PERIPH_E);


	//
		//	Unlock Port F Commit Register
		//
	    HWREG(BUTTONS_GPIO_BASE_F + GPIO_O_LOCK) = 0x4C4F434B;

	    //
	    //	Configure Port F0 as GPIO
	    //
	    HWREG(BUTTONS_GPIO_BASE_F + GPIO_O_CR) = 1;

	    //
	    //	Lock Port F Commit Register
	    //
	    HWREG(BUTTONS_GPIO_BASE_F + GPIO_O_LOCK) = 45;

    //
    // Set each of the button GPIO pins as an input with a pull-up.
    //
//    MAP_GPIODirModeSet(BUTTONS_GPIO_BASE, ALL_BUTTONS, GPIO_DIR_MODE_IN);
//    MAP_GPIOPadConfigSet(BUTTONS_GPIO_BASE, ALL_BUTTONS,
//                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    MAP_GPIODirModeSet(BUTTONS_GPIO_BASE_F, ALL_BUTTONS_F, GPIO_DIR_MODE_IN);
    MAP_GPIOPadConfigSet(BUTTONS_GPIO_BASE_F, ALL_BUTTONS_F,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    MAP_GPIODirModeSet(BUTTONS_GPIO_BASE_D, ALL_BUTTONS_D, GPIO_DIR_MODE_IN);
    MAP_GPIOPadConfigSet(BUTTONS_GPIO_BASE_D, ALL_BUTTONS_D,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    MAP_GPIODirModeSet(BUTTONS_GPIO_BASE_E, ALL_BUTTONS_E, GPIO_DIR_MODE_IN);
    MAP_GPIOPadConfigSet(BUTTONS_GPIO_BASE_E, ALL_BUTTONS_E,
                         GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    //
    // Initialize the debounced button state with the current state read from
    // the GPIO bank.
    //
//  g_ui8ButtonStates = MAP_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS);

    g_ui8ButtonStates_F = MAP_GPIOPinRead(BUTTONS_GPIO_BASE_F, ALL_BUTTONS_F);
    g_ui8ButtonStates_D = MAP_GPIOPinRead(BUTTONS_GPIO_BASE_D, ALL_BUTTONS_D);
    g_ui8ButtonStates_E = MAP_GPIOPinRead(BUTTONS_GPIO_BASE_E, ALL_BUTTONS_E);

    gKeysStatus.Val = 0;
}
#endif //------DISP_LAUNCHPAD_BOARD
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
#ifdef DISP_TARGET_BOARD

#ifndef KEY_PRIORITY_LOGIC_ENABLED

void keysProcessFlags(uint8_t keyState, uint8_t keyChanged)
{
	uint8_t pressedState = keyState & ~(0x00);
	static uint8_t keyStpPressedFirst;
	static uint8_t resetCountflag = 0;

	/********** General Button Flags *********/
	//UP Button > Up Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_UP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Up_pressed) {
				gKeysStatus.bits.Key_Up_pressed = true;
				gKeysStatus.bits.Key_Up_released = false;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_UP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Up_released) {
				gKeysStatus.bits.Key_Up_released = true;
				gKeysStatus.bits.Key_Up_pressed = false;
			}
		}

	//Down Button > Down Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_DOWN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Down_pressed) {
				gKeysStatus.bits.Key_Down_pressed = true;
				gKeysStatus.bits.Key_Down_released = false;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_DOWN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Down_released) {
				gKeysStatus.bits.Key_Down_released = true;
				gKeysStatus.bits.Key_Down_pressed = false;
			}
		}

	//Mode Button > Right Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_MODE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Mode_pressed) {
				gKeysStatus.bits.Key_Mode_pressed = true;
				gKeysStatus.bits.Key_Mode_released = false;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_MODE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Mode_released) {
				gKeysStatus.bits.Key_Mode_released = true;
				gKeysStatus.bits.Key_Mode_pressed = false;
			}
		}

	//Enter Button > Select Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_ENTER, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Enter_pressed) {
				currentTickCount = 0;
					gKeysStatus.bits.Key_Enter_pressed = true;
					gKeysStatus.bits.Key_Enter_released = false;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_ENTER, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Enter_released) {
				gKeysStatus.bits.Key_Enter_released = true;
				gKeysStatus.bits.Key_Enter_pressed = false;
			}
		}

	//AutMan Button > Left Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_AUTMAN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_AutMan_pressed) {
				gKeysStatus.bits.Key_AutMan_pressed = true;
				gKeysStatus.bits.Key_AutMan_released = false;
			}
		}
	}

	if(!pressedState)

		if( (BUTTON_RELEASED(KEY_AUTMAN, keyState, keyChanged) )) {
			if(!gKeysStatus.bits.Key_AutMan_released) {
				gKeysStatus.bits.Key_AutMan_released = true;
				gKeysStatus.bits.Key_AutMan_pressed = false;
			}
		}

	//Open Button > NO BUTTON ON EVAL @LEFT
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_OPEN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Open_pressed) {
				gKeysStatus.bits.Key_Open_pressed = true;
				gKeysStatus.bits.Key_Open_released = false;
				currentTickCount = 0;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_OPEN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Open_released) {
				gKeysStatus.bits.Key_Open_released = true;
				gKeysStatus.bits.Key_Open_pressed = false;
			}
		}

	//Close Button > NO BUTTON ON EVAL @SELECT
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_CLOSE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Close_pressed) {
				currentTickCount = 0;
				gKeysStatus.bits.Key_Close_released = false;
				gKeysStatus.bits.Key_Close_pressed = true;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_CLOSE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Close_released) {
				gKeysStatus.bits.Key_Close_released = true;
				gKeysStatus.bits.Key_Close_pressed = false;
			}
		}

//	Stop Button > NO BUTTON ON EVAL @RIGHT
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_STOP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Stop_pressed) {
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;
				currentTickCount = 0;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_STOP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Stop_released) {
				gKeysStatus.bits.Key_Stop_released = true;
				gKeysStatus.bits.Key_Stop_pressed = false;
				keyStpPressedFirst = 0;
			}
		}



	/********** End of General Button Flags *********/
	//If Key is released reset currentTickCount
	if(gKeysStatus.byte.HB_Released) {
		currentTickCount = 0;
		resetCountflag = 0;
//		gKeysStatus.byte.HB_Released = 0; //Added - 10 Jul14- No Release event usage //Removed
	}

	/********* Extended Button Flags *************/
	if(pressedState) {
		if(pressedState == _ONLY_STOPKEY_PRESSED)//check in upper section only
			if((pressedState == keyChanged))
				keyStpPressedFirst = 1;

		if((KEY_CLOSE == pressedState) &&
				(currentTickCount++ == SYSTICK_3SEC_KEYPRESS))
			gKeysStatus.bits.Key_3secCls_pressed = true;

		//Stop pressed for 3 sec //Added 4 Jun 2014
		if((KEY_STOP == pressedState) &&
				(currentTickCount++ == SYSTICK_3SEC_KEYPRESS)) {
			//Check in application the current mode of menu
			//Inspect for Close pressed flags!
			gKeysStatus.bits.Key_3secStp_pressed = true;
		}

		if((KEY_ENTER == pressedState) &&
				(currentTickCount++ == SYSTICK_3SEC_KEYPRESS))
			gKeysStatus.bits.Key_3secEnter_pressed = true;

		if(!isPowerOfTwo(pressedState)) { //Check if multiple keys are pressed

			if(!resetCountflag) {
				currentTickCount = 0;
				resetCountflag = ~0;
			}
			//Stop + Open pressed for 3 sec
			if(((KEY_OPEN | KEY_STOP) == pressedState) &&
			   keyStpPressedFirst &&
			   currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
//				currentTickCount = 0;
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpOpn_pressed = true;
			}

			//Stop + Close pressed for 3 sec
			if(((KEY_CLOSE | KEY_STOP) == pressedState) &&
			   keyStpPressedFirst &&
			   currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpCls_pressed = true;
			}

			//Open + Stop + Close pressed for 3 sec /*** TO EDIT ** IN APP FIRST PRESS STOP ***/
			if((INSTALLATION_KEYS == pressedState) &&
			   currentTickCount++ == SYSTICK_3SEC_KEYPRESS) {
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys3_3secOpStCl_pressed = true;
			}
		}
	} //-----if(pressedState)
	/********* End of Extended Button Flags *************/
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

	uint8_t pressedState = keyState & ~(0x00);
	static uint8_t keyStpPressedFirst;
	static uint8_t resetCountflag = 0;
	static uint32_t LAST_PRESS_TIME = 0;
	static uint8_t  PRESS_DISABLE_FLAG = 0;

	/********** General Button Flags *********/
	//UP Button > Up Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_UP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Up_pressed) {
				gKeysStatus.bits.Key_Up_pressed = true;
				Set_lcdlightON();
				gKeysStatus.bits.Key_Up_released = false;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_UP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Up_released) {
				gKeysStatus.bits.Key_Up_released = true;
				gKeysStatus.bits.Key_Up_pressed = false;
			}
		}

	//Down Button > Down Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_DOWN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Down_pressed) {
				gKeysStatus.bits.Key_Down_pressed = true;
				Set_lcdlightON();
				gKeysStatus.bits.Key_Down_released = false;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_DOWN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Down_released) {
				gKeysStatus.bits.Key_Down_released = true;
				gKeysStatus.bits.Key_Down_pressed = false;
			}
		}

	//Mode Button > Right Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_MODE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Mode_pressed) {
				currentTickCount_mode_cyw = 0;
				gKeysStatus.bits.Key_Mode_pressed = true;
				Set_lcdlightON();
				gKeysStatus.bits.Key_Mode_released = false;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_MODE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Mode_released) {
				currentTickCount_mode_cyw = 0;
				gKeysStatus.bits.Key_Mode_released = true;
				gKeysStatus.bits.Key_Mode_pressed = false;
			}
		}

	//Enter Button > Select Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_ENTER, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Enter_pressed) {
				currentTickCount = 0;
				    currentTickCount_ent_cyw = 0;
					gKeysStatus.bits.Key_Enter_pressed = true;
					Set_lcdlightON();
					gKeysStatus.bits.Key_Enter_released = false;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_ENTER, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Enter_released) {
				currentTickCount = 0;
			    currentTickCount_ent_cyw = 0;
				gKeysStatus.bits.Key_Enter_released = true;
				gKeysStatus.bits.Key_Enter_pressed = false;
			}
		}

	//AutMan Button > Left Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_AUTMAN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_AutMan_pressed) {
				gKeysStatus.bits.Key_AutMan_pressed = true;
				Set_lcdlightON();
				gKeysStatus.bits.Key_AutMan_released = false;
			}
		}
	}

	if(!pressedState)

		if( (BUTTON_RELEASED(KEY_AUTMAN, keyState, keyChanged) )) {
			if(!gKeysStatus.bits.Key_AutMan_released) {
				gKeysStatus.bits.Key_AutMan_released = true;
				gKeysStatus.bits.Key_AutMan_pressed = false;
			}
		}

#if 0
	//Open Button > NO BUTTON ON EVAL @LEFT
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_OPEN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Open_pressed) {
				gKeysStatus.bits.Key_Open_pressed = true;
				gKeysStatus.bits.Key_Open_released = false;
				currentTickCount = 0;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_OPEN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Open_released) {
				gKeysStatus.bits.Key_Open_released = true;
				gKeysStatus.bits.Key_Open_pressed = false;
			}
		}

	//Close Button > NO BUTTON ON EVAL @SELECT
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_CLOSE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Close_pressed) {
				currentTickCount = 0;
				gKeysStatus.bits.Key_Close_released = false;
				gKeysStatus.bits.Key_Close_pressed = true;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_CLOSE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Close_released) {
				gKeysStatus.bits.Key_Close_released = true;
				gKeysStatus.bits.Key_Close_pressed = false;
			}
		}

//	Stop Button > NO BUTTON ON EVAL @RIGHT
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_STOP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Stop_pressed) {
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;
				currentTickCount = 0;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_STOP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Stop_released) {
				gKeysStatus.bits.Key_Stop_released = true;
				gKeysStatus.bits.Key_Stop_pressed = false;
				keyStpPressedFirst = 0;
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
		if(BUTTON_PRESSED(KEY_OPEN, keyState, keyChanged))
		{
			//if(!gKeysStatus.bits.Key_Open_pressed)
			//{
			//	gKeysStatus.bits.Key_Open_pressed = true;
			//	gKeysStatus.bits.Key_Open_released = false;
			//	currentTickCount = 0;
			//}
			Set_lcdlightON();
			keyPressedEvent |= KEY_OPEN;
		}
	}

	//if(!pressedState)
		if(BUTTON_RELEASED(KEY_OPEN, keyState, keyChanged))
		{
			//if(!gKeysStatus.bits.Key_Open_released)
			//{
			//	gKeysStatus.bits.Key_Open_released = true;
			//	gKeysStatus.bits.Key_Open_pressed = false;
			//}

			keyReleasedEvent |= KEY_OPEN;
		}

	//Close Button > NO BUTTON ON EVAL @SELECT
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_CLOSE, keyState, keyChanged))
		{
			//if(!gKeysStatus.bits.Key_Close_pressed)
			//{
			//	gKeysStatus.bits.Key_Close_released = false;
			//	gKeysStatus.bits.Key_Close_pressed = true;
			//	currentTickCount = 0;
			//}
			Set_lcdlightON();
			keyPressedEvent |= KEY_CLOSE;
		}
	}

	//if(!pressedState)
		if(BUTTON_RELEASED(KEY_CLOSE, keyState, keyChanged))
		{
			//if(!gKeysStatus.bits.Key_Close_released)
			//{
			//	gKeysStatus.bits.Key_Close_released = true;
			//	gKeysStatus.bits.Key_Close_pressed = false;
			//}

			keyReleasedEvent |= KEY_CLOSE;
		}

//	Stop Button > NO BUTTON ON EVAL @RIGHT
	//if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_STOP, keyState, keyChanged))
		{
			//if(!gKeysStatus.bits.Key_Stop_pressed)
			//{
			//	gKeysStatus.bits.Key_Stop_pressed = true;
			//	gKeysStatus.bits.Key_Stop_released = false;
			//	currentTickCount = 0;
			//}
			Set_lcdlightON();
			keyPressedEvent |= KEY_STOP;
		}
	}

	//if(!pressedState)
		if(BUTTON_RELEASED(KEY_STOP, keyState, keyChanged))
		{
			//if(!gKeysStatus.bits.Key_Stop_released)
			//{
			//	gKeysStatus.bits.Key_Stop_released = true;
			//	gKeysStatus.bits.Key_Stop_pressed = false;
			//	keyStpPressedFirst = 0;
			//}
			currentTickCount_stop_cyw=0;//20160804
			keyReleasedEvent |= KEY_STOP;
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
					(	(keyPressedEvent & KEY_OPEN) && (PriorityLastSentPressKey < Priority_Open_Key)	)||
					(	(keyPressedEvent & KEY_STOP) && (PriorityLastSentPressKey < Priority_Stop_Key)	)
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
				}

			}
			else if (PriorityLastSentPressKey == 2) // Previous key which is not yet released is open
			{
				if(!gKeysStatus.bits.Key_Open_released)
				{
				gKeysStatus.bits.Key_Open_released = true;
				gKeysStatus.bits.Key_Open_pressed = false;
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

				if (keyPressedEvent & KEY_STOP)
				{

					if(!gKeysStatus.bits.Key_Stop_pressed)
					{

						//PRESS_DISABLE_FLAG = 1;
						//LAST_PRESS_TIME=g_ui32TickCount;//20160823
					gKeysStatus.bits.Key_Stop_pressed = true;
						gKeysStatus.bits.Key_Stop_released = false;
						currentTickCount = 0;
						PriorityLastSentPressKey = Priority_Stop_Key;
					}

				}
				else if (keyPressedEvent & KEY_OPEN)
				{

					if(!gKeysStatus.bits.Key_Open_pressed)
					{

						//PRESS_DISABLE_FLAG = 2;
						//LAST_PRESS_TIME=g_ui32TickCount;//20160823
						gKeysStatus.bits.Key_Open_pressed = true;
						gKeysStatus.bits.Key_Open_released = false;
						currentTickCount = 0;
						PriorityLastSentPressKey = Priority_Open_Key;
					}

				}
				else if (keyPressedEvent & KEY_CLOSE)
				{

					if(!gKeysStatus.bits.Key_Close_pressed)
					{

                      // PRESS_DISABLE_FLAG = 3;
						//LAST_PRESS_TIME=g_ui32TickCount;//20160823
                        gKeysStatus.bits.Key_Close_released = false;
                        gKeysStatus.bits.Key_Close_pressed = true;
                        currentTickCount = 0;
                         PriorityLastSentPressKey = Priority_Close_Key;
					}

				}

			} // Check for key press event
			// check for key release event for which press event already sent
			else if (
					((PriorityLastSentPressKey == Priority_Stop_Key) && (keyReleasedEvent & KEY_STOP)) ||
					((PriorityLastSentPressKey == Priority_Open_Key) && (keyReleasedEvent & KEY_OPEN)) ||
					((PriorityLastSentPressKey == Priority_Close_Key) && (keyReleasedEvent & KEY_CLOSE))
					)
			{

				if ((PriorityLastSentPressKey == Priority_Stop_Key) && (keyReleasedEvent & KEY_STOP))
				{

					if(!gKeysStatus.bits.Key_Stop_released)
					{
						gKeysStatus.bits.Key_Stop_released = true;
						gKeysStatus.bits.Key_Stop_pressed = false;
						keyStpPressedFirst = 0;

						keyReleasedEvent &= (~KEY_STOP);
						keyPressedEvent &= (~KEY_STOP);

					}

				}
				else if ((PriorityLastSentPressKey == Priority_Open_Key) && (keyReleasedEvent & KEY_OPEN))
				{

					if(!gKeysStatus.bits.Key_Open_released)
					{
						gKeysStatus.bits.Key_Open_released = true;
						gKeysStatus.bits.Key_Open_pressed = false;

						keyReleasedEvent &= (~KEY_OPEN);
						keyPressedEvent &= (~KEY_OPEN);

					}

				}
				else if ((PriorityLastSentPressKey == Priority_Close_Key) && (keyReleasedEvent & KEY_CLOSE))
				{

					if(!gKeysStatus.bits.Key_Close_released)
					{
						gKeysStatus.bits.Key_Close_released = true;
						gKeysStatus.bits.Key_Close_pressed = false;

						keyReleasedEvent &= (~KEY_CLOSE);
						keyPressedEvent &= (~KEY_CLOSE);

					}

				}

				PriorityLastSentPressKey = 0;

			} // check for key release event for which press event already sent
			// check for key release event which has less priority than the press event already sent( For which released event already sent)
			else if (
					((keyReleasedEvent & KEY_CLOSE) && (PriorityLastSentPressKey > Priority_Close_Key)) ||
					((keyReleasedEvent & KEY_OPEN) && (PriorityLastSentPressKey > Priority_Open_Key))
					)
			{

				if ((keyReleasedEvent & KEY_CLOSE) && (PriorityLastSentPressKey > Priority_Close_Key))
				{

					keyReleasedEvent &= (~KEY_CLOSE);
					keyPressedEvent &= (~KEY_CLOSE);

				}
				else if ((keyReleasedEvent & KEY_OPEN) && (PriorityLastSentPressKey > Priority_Open_Key))
				{

					keyReleasedEvent &= (~KEY_OPEN);
					keyPressedEvent &= (~KEY_OPEN);

				}

			}

		} //else if (FlagToSendKeyEvent == 1)




	/********** End of General Button Flags *********/
	//If Key is released reset currentTickCount
	if(gKeysStatus.byte.HB_Released) {
		currentTickCount = 0;
		resetCountflag = 0;
//		gKeysStatus.byte.HB_Released = 0; //Added - 10 Jul14- No Release event usage //Removed
	}


	/********* Extended Button Flags *************/

    //uartSendTxBuffer(UART_debug,&pressedState,1);

	//pressedState = pressedState & 0x0E;

	if(pressedState)
	{

		//uartSendTxBuffer(UART_debug,&pressedState,1);

		if(pressedState == _ONLY_STOPKEY_PRESSED)//check in upper section only
			if((pressedState == keyChanged))
				keyStpPressedFirst = 1;

		if((KEY_CLOSE == pressedState) && (currentTickCount++ == SYSTICK_3SEC_KEYPRESS))
			gKeysStatus.bits.Key_3secCls_pressed = true;


		if((KEY_STOP == pressedState) && (currentTickCount_stop_cyw++ == SYSTICK_3SEC_KEYPRESS))
		{
			KEY_PRESS_3SEC_STOP_FLAG_CYW = true;
			currentTickCount_stop_cyw = 0;
		}
		//Stop pressed for 3 sec //Added 4 Jun 2014
		if((KEY_STOP == pressedState) && (currentTickCount++ == SYSTICK_3SEC_KEYPRESS))
		{
			//Check in application the current mode of menu
			//Inspect for Close pressed flags!
			gKeysStatus.bits.Key_3secStp_pressed = true;
			//currentTickCount =0;


		}

		if((KEY_ENTER == pressedState) && (currentTickCount++ == SYSTICK_3SEC_KEYPRESS))
		{
			gKeysStatus.bits.Key_3secEnter_pressed = true;

		}

		if((KEY_ENTER == pressedState) && ((currentTickCount_ent_cyw++) >= SYSTICK_3SEC_KEYPRESS_CYW))
		{

			currentTickCount_ent_cyw = 0;
		    KEY_PRESS_3SEC_ENT_FLAG_CYW = true;
		    KEY_PRESS_3SEC_ENT_FORRESET_CYW = true;
		}

		if((KEY_MODE == pressedState) && ((currentTickCount_mode_cyw++) >= SYSTICK_3SEC_KEYPRESS_CYW))
		{

			currentTickCount_mode_cyw = 0;
		    KEY_PRESS_3SEC_MODE_FLAG_CYW = true;
		}
		if(!isPowerOfTwo(pressedState))
		{ //Check if multiple keys are pressed

			if(!resetCountflag)
			{
				currentTickCount = 0;
				resetCountflag = ~0;
			}


			//Stop + Open pressed for 3 sec
			if(((KEY_OPEN | KEY_STOP) == pressedState) &&
			   keyStpPressedFirst &&
			   currentTickCount++ == SYSTICK_3SEC_KEYPRESS)
			{

//				currentTickCount = 0;
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpOpn_pressed = true;
			}

			//Stop + Close pressed for 3 sec
			if(((KEY_CLOSE | KEY_STOP) == pressedState) &&
			   keyStpPressedFirst &&
			   currentTickCount++ == SYSTICK_3SEC_KEYPRESS)
			{

				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpCls_pressed = true;
			}

			//Open + Stop + Close pressed for 3 sec /*** TO EDIT ** IN APP FIRST PRESS STOP ***/
			// At power ON "Key Poll" poll @ 10msec
			if((INSTALLATION_KEYS == pressedState) && currentTickCount++ == (SYSTICK_3SEC_KEYPRESS/1000))
			//if((INSTALLATION_KEYS & pressedState) && currentTickCount++ == (SYSTICK_3SEC_KEYPRESS/1000))
			{
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys3_3secOpStCl_pressed = true;
			}

		}

	} //-----if(pressedState)
	/********* End of Extended Button Flags *************/

	if(get_timego(LAST_PRESS_TIME)>100)//100ms
	{
	switch(PRESS_DISABLE_FLAG)
	{
	case 0:
		break;
	case 1:
		LAST_PRESS_TIME=g_ui32TickCount;
		PRESS_DISABLE_FLAG = 0;
		gKeysStatus.bits.Key_Stop_pressed = true;
		gKeysStatus.bits.Key_Stop_released = false;
		currentTickCount = 0;
	    PriorityLastSentPressKey = Priority_Stop_Key;
		break;
	case 2:
		//LAST_PRESS_TIME=g_ui32TickCount;
		PRESS_DISABLE_FLAG = 0;
		gKeysStatus.bits.Key_Open_pressed = true;
		gKeysStatus.bits.Key_Open_released = false;
		currentTickCount = 0;
		PriorityLastSentPressKey = Priority_Open_Key;
		break;
	case 3:
		LAST_PRESS_TIME =g_ui32TickCount;
		PRESS_DISABLE_FLAG = 0;
		gKeysStatus.bits.Key_Close_released = false;
		gKeysStatus.bits.Key_Close_pressed = true;
		currentTickCount = 0;
		PriorityLastSentPressKey = Priority_Close_Key;
		break;
	default:
		break;
	}
	}
}

#endif //#ifndef KEY_PRIORITY_LOGIC_ENABLED

#ifdef DEBOUNCE_WITH_PRIORITY_LOGIC     // First attempt for priority logic. Not to be used as it is not considering the key press after some time
#define KEY_PRIORITY_DELAY	5			//	50 mS @10mS systick
void keysProcessFlags(uint8_t keyState, uint8_t keyChanged)
{
	uint8_t pressedState = keyState & ~(0x00);
	static uint8_t keyStpPressedFirst;
	static uint8_t resetCountflag = 0;

	static uint8_t lsui8KeyOpenPressed = 0;
	static uint8_t lsui8KeyClosePressed = 0;
	//	static uint8_t lsui8KeyStopPressed = 0;

	static uint8_t lsui8KeyOpenServiced = 0;
	static uint8_t lsui8KeyCloseServiced = 0;
	static uint8_t lsui8KeyStopServiced = 0;

	static uint8_t lsui8EnterKey3secServiced = 0;

	static uint32_t lsui32KeyCapturedTime = 0;

	/********** General Button Flags *********/
	//UP Button > Up Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_UP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Up_pressed) {
				gKeysStatus.bits.Key_Up_pressed = true;
				gKeysStatus.bits.Key_Up_released = false;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_UP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Up_released) {
				gKeysStatus.bits.Key_Up_released = true;
				gKeysStatus.bits.Key_Up_pressed = false;
			}
		}

	//Down Button > Down Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_DOWN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Down_pressed) {
				gKeysStatus.bits.Key_Down_pressed = true;
				gKeysStatus.bits.Key_Down_released = false;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_DOWN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Down_released) {
				gKeysStatus.bits.Key_Down_released = true;
				gKeysStatus.bits.Key_Down_pressed = false;
			}
		}

	//Mode Button > Right Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_MODE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Mode_pressed) {
				gKeysStatus.bits.Key_Mode_pressed = true;
				gKeysStatus.bits.Key_Mode_released = false;
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_MODE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Mode_released) {
				gKeysStatus.bits.Key_Mode_released = true;
				gKeysStatus.bits.Key_Mode_pressed = false;
			}
		}

	//Enter Button > Select Eval
	if(isPowerOfTwo(pressedState))
	{
		if(BUTTON_PRESSED(KEY_ENTER, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Enter_pressed)
			{
				currentTickCount = 0;
				gKeysStatus.bits.Key_Enter_pressed = true;
				gKeysStatus.bits.Key_Enter_released = false;

				currentTickCount = g_ui32TickCount;

				uartSendTxBuffer(UART_debug,"E",1);
			}
		}
	}

	if(!pressedState)
		if(BUTTON_RELEASED(KEY_ENTER, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Enter_released)
			{
				gKeysStatus.bits.Key_Enter_released = true;
				gKeysStatus.bits.Key_Enter_pressed = false;

				gKeysStatus.bits.Key_3secEnter_pressed = false;

				lsui8EnterKey3secServiced = 0;
			}
		}

	//AutMan Button > Left Eval
	if(isPowerOfTwo(pressedState)) {
		if(BUTTON_PRESSED(KEY_AUTMAN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_AutMan_pressed) {
				gKeysStatus.bits.Key_AutMan_pressed = true;
				gKeysStatus.bits.Key_AutMan_released = false;
			}
		}
	}

	if(!pressedState)

		if( (BUTTON_RELEASED(KEY_AUTMAN, keyState, keyChanged) )) {
			if(!gKeysStatus.bits.Key_AutMan_released) {
				gKeysStatus.bits.Key_AutMan_released = true;
				gKeysStatus.bits.Key_AutMan_pressed = false;
			}
		}

	//PE3_3PBS_OPEN
	if(
			(isPowerOfTwo(pressedState)) ||		//	Only open key is pressed
			(
					((KEY_OPEN | KEY_CLOSE) == pressedState) &&	//	Open + Close keys are pressed
					/*((KEY_OPEN | KEY_STOP) != pressedState) &&
					((KEY_OPEN | KEY_CLOSE | KEY_STOP) != pressedState) &&	//	Stop key is not pressed*/
					(lsui8KeyClosePressed == 0) &&		//	Close key was not pressed within 10ms before open press
					(lsui8KeyCloseServiced == 0)		//	Close key is not already serviced
			)
	)
	{
		if(BUTTON_PRESSED(KEY_OPEN, keyState, keyChanged))
		{
			if(lsui8KeyOpenPressed == 0)
			{
				lsui8KeyOpenPressed = 1;
				lsui32KeyCapturedTime = g_ui32TickCount;
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
			(BUTTON_RELEASED(KEY_OPEN, keyState, keyChanged)) &&
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

			//				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 0;

			uartSendTxBuffer(UART_debug,"o",1);
		}
	}

	//PE1_3PBS_STOP
	if(
			(isPowerOfTwo(pressedState)) ||		//	Only stop key is pressed
			(
					((KEY_OPEN | KEY_STOP) == pressedState) ||	//	Open + Stop keys pressed
					((KEY_CLOSE | KEY_STOP) == pressedState) ||	//	Open + Stop keys pressed
					((KEY_OPEN | KEY_CLOSE | KEY_STOP) == pressedState)	//	Open + Stop + Close keys pressed
			) &&
			(lsui8KeyOpenPressed == 0) &&		//	Open key was not pressed within 10ms before Stop key press
			(lsui8KeyOpenServiced == 0) &&		//	Open key is not already serviced
			(lsui8KeyClosePressed == 0) &&		//	Close key was not pressed within 10ms before Stop key press
			(lsui8KeyCloseServiced == 0)		//	Close key is not already serviced
	)
	{
		if(BUTTON_PRESSED(KEY_STOP, keyState, keyChanged))
		{
			if(!gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;

				//				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 1;

				currentTickCount = g_ui32TickCount;

				lsui8KeyStopServiced = 1;

				uartSendTxBuffer(UART_debug,"S",1);
			}
		}
	}
	//---PE1_3PBS_STOP
	//	if(!pressedState)
	if(
			(BUTTON_RELEASED(KEY_STOP, keyState, keyChanged)) &&
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

			//				gstDriveStatusMenu.bits.Stop_Key_3PBS_Status = 0;
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
		if(BUTTON_PRESSED(KEY_CLOSE, keyState, keyChanged))
		{
			if(lsui8KeyClosePressed == 0)
			{
				lsui8KeyClosePressed = 1;
				lsui32KeyCapturedTime = g_ui32TickCount;
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
			(BUTTON_RELEASED(KEY_CLOSE, keyState, keyChanged)) &&
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

			//				gstDriveStatusMenu.bits.Close_Key_3PBS_Status = 0;

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
						(KEY_OPEN == pressedState) ||
						((KEY_OPEN | KEY_CLOSE) == pressedState)
				) &&
				((KEY_OPEN | KEY_STOP) != pressedState) &&
				((g_ui32TickCount - lsui32KeyCapturedTime) >= KEY_PRIORITY_DELAY) &&
				(gKeysStatus.bits.Key_Open_pressed == false) &&
				(lsui8KeyOpenPressed == 1)
		)
		{	//	Open key was pressed before stop key and stop key was not pressed
			//	within 20mS from open pressed event, declare open key press event
			lsui8KeyOpenPressed = 0;

			gKeysStatus.bits.Key_Open_pressed = true;
			gKeysStatus.bits.Key_Open_released = false;

			//				gstDriveStatusMenu.bits.Open_Key_3PBS_Status = 1;
			currentTickCount = g_ui32TickCount;

			lsui8KeyOpenServiced = 1;

			uartSendTxBuffer(UART_debug,"O",1);
		}
		else if(
				((KEY_OPEN | KEY_STOP) == pressedState) &&
				(lsui8KeyOpenPressed == 1) &&
				((g_ui32TickCount - lsui32KeyCapturedTime) < KEY_PRIORITY_DELAY)
		)
		{	//	Open key was pressed first but within 20mS of open key press event
			//	stop key was also pressed, prioritize stop key and declare stop key
			//	press event
			lsui8KeyOpenPressed = 0;

			if(!gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;

				keyStpPressedFirst = 1;

				lsui8KeyStopServiced = 1;

				currentTickCount = g_ui32TickCount;

				uartSendTxBuffer(UART_debug,"1",1);
			}
		}

		//	Close + Stop key check
		if(
				(KEY_CLOSE == pressedState) &&
				((KEY_CLOSE | KEY_STOP) != pressedState) &&
				((KEY_CLOSE | KEY_OPEN) != pressedState) &&
				((g_ui32TickCount - lsui32KeyCapturedTime) >= KEY_PRIORITY_DELAY) &&
				(gKeysStatus.bits.Key_Close_pressed == false) &&
				(lsui8KeyClosePressed == 1)
		)
		{	//	Close key was pressed before open key or stop key.
			//	Open or stop key or both open and stop keys were pressed after 20mS from
			//	close key press event.
			//	Declare close key pressed event
			lsui8KeyClosePressed = 0;

			currentTickCount = g_ui32TickCount;

			lsui8KeyCloseServiced = 1;

			gKeysStatus.bits.Key_Close_pressed = true;
			gKeysStatus.bits.Key_Close_released = false;

			uartSendTxBuffer(UART_debug,"C",1);
		}
		else if(
				((KEY_CLOSE | KEY_OPEN) == pressedState) &&
				((KEY_CLOSE | KEY_STOP) != pressedState) &&
				(lsui8KeyClosePressed == 1) &&
				((g_ui32TickCount - lsui32KeyCapturedTime) < KEY_PRIORITY_DELAY)
		)
		{	//	Close key was pressed before open key or stop key.
			//	Open key was pressed before 20mS from close key press event.
			//	Declare open key pressed event
			lsui8KeyClosePressed = 0;

			if(!gKeysStatus.bits.Key_Open_pressed)
			{
				gKeysStatus.bits.Key_Open_pressed = true;
				gKeysStatus.bits.Key_Open_released = false;

				currentTickCount = g_ui32TickCount;

				uartSendTxBuffer(UART_debug,"2",1);
			}

		}
		else if(
				(
						((KEY_CLOSE | KEY_STOP) == pressedState) ||
						((KEY_CLOSE | KEY_STOP | KEY_OPEN) == pressedState)
				) &&
				(lsui8KeyClosePressed == 1) &&
				((g_ui32TickCount - lsui32KeyCapturedTime) < KEY_PRIORITY_DELAY)
		)
		{	//	Close key was pressed before open key or stop key.
			//	Stop key was pressed before 20mS from close key press event.
			//	Declare stop key pressed event.
			lsui8KeyClosePressed = 0;

			if(!gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = true;
				gKeysStatus.bits.Key_Stop_released = false;

				keyStpPressedFirst = 1;

				lsui8KeyStopServiced = 1;

				currentTickCount = g_ui32TickCount;

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
						(KEY_CLOSE == pressedState) &&
						(lsui8KeyCloseServiced == 1)
				) &&
				((g_ui32TickCount - currentTickCount) >= SYSTICK_3SEC_KEYPRESS) &&
				(gKeysStatus.bits.Key_3secCls_pressed == false) /*&&
				(gKeysStatus.bits.Key_Close_pressed == true)*/
		)
		{
			gKeysStatus.bits.Key_3secCls_pressed = true;

			uartSendTxBuffer(UART_debug,"X",1);
		}

		//Stop pressed for 3 sec //Added 4 Jun 2014
		if(
				(KEY_STOP == pressedState) &&
				((g_ui32TickCount - currentTickCount) >= SYSTICK_3SEC_KEYPRESS) &&
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
					((KEY_OPEN | KEY_STOP) == pressedState) &&
					(keyStpPressedFirst) &&
					((g_ui32TickCount - currentTickCount) >= SYSTICK_3SEC_KEYPRESS) &&
					(gKeysStatus.bits.Keys2_3secStpOpn_pressed == false)
			)
			{
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys2_3secStpOpn_pressed = true;

				uartSendTxBuffer(UART_debug,"Z",1);
			}

			//Stop + Close pressed for 3 sec
			if(
					((KEY_CLOSE | KEY_STOP) == pressedState) &&
					(keyStpPressedFirst) &&
					((g_ui32TickCount - currentTickCount) >= SYSTICK_3SEC_KEYPRESS) &&
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
					((g_ui32TickCount - currentTickCount) >= SYSTICK_3SEC_KEYPRESS) &&
					(gKeysStatus.bits.Keys3_3secOpStCl_pressed == false)
			)
			{
				//				currentTickCount = 0;
				keyStpPressedFirst = 0;
				gKeysStatus.bits.Keys3_3secOpStCl_pressed = true;

				uartSendTxBuffer(UART_debug,"V",1);
			}
		}

		//Enter pressed for 3 sec //Added 28 Oct 2014 as per client requirement specified by Yogesh during onsite testing
		if(
				(KEY_ENTER == pressedState) && (0 == lsui8EnterKey3secServiced) &&
				((g_ui32TickCount - currentTickCount) >= SYSTICK_3SEC_KEYPRESS) &&
				(gKeysStatus.bits.Key_3secEnter_pressed == false)/* &&
				(gKeysStatus.bits.Key_Enter_pressed == true)*/
		)
		{
			//Check in application the current mode of menu
			//Inspect for Close pressed flags!
			gKeysStatus.bits.Key_3secEnter_pressed = true;
			lsui8EnterKey3secServiced = 1;

			uartSendTxBuffer(UART_debug,"P",1);
		}
	}
	/********* End of Extended Button Flags *************/
}
#endif

#endif //-----DISP_TARGET_BOARD

#ifdef DISP_LAUNCHPAD_BOARD
void keysProcessFlags_F(uint8_t keyState, uint8_t keyChanged)
{
	uint8_t pressedState_F = keyState & ~(0xEE);

	static uint8_t keyStpPressedFirst;
	/********** General Button Flags *********/

	//AutMan Button > Left Eval
	if(isPowerOfTwo(pressedState_F)) {
		if(BUTTON_PRESSED(PF4_KEY_AUT_MAN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Open_pressed) {
				gKeysStatus.bits.Key_Open_pressed = true;
			}
		}
	}

	if(!pressedState_F)

		if( /*(!gKeysStatus.byte.LB_Pressed) &&*/
				(BUTTON_RELEASED(PF4_KEY_AUT_MAN, keyState, keyChanged)) ) {
			if(!gKeysStatus.bits.Key_Open_released) {
				gKeysStatus.bits.Key_Open_released = true;
				gKeysStatus.bits.Key_Open_pressed = false;
			}
		}

	//Stop Button > NO BUTTON ON EVAL @RIGHT
	if(isPowerOfTwo(pressedState_F)) {
		if(BUTTON_PRESSED(PF0_KEY_STOP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Stop_pressed) {
				gKeysStatus.bits.Key_Stop_pressed = true;
				currentTickCount = 0;
			}
		}
	}

	if(!pressedState_F)
		if(BUTTON_RELEASED(PF0_KEY_STOP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Stop_released) {
				gKeysStatus.bits.Key_Stop_released = true;
				gKeysStatus.bits.Key_Stop_pressed = false;
				keyStpPressedFirst = 0;
			}
		}

	/********** End of General Button Flags *********/
	//If Key is released reset currentTickCount
	if(gKeysStatus.byte.HB_Released)
		currentTickCount = 0;

}

void keysProcessFlags_D(uint8_t keyState, uint8_t keyChanged)
{
	uint8_t pressedState_D = keyState & ~(0xF1);

	static uint8_t keyStpPressedFirst;
	/********** General Button Flags *********/

	//UP Button > Up Eval
	if(isPowerOfTwo(pressedState_D)) {
		if(BUTTON_PRESSED(PD2_KEY_DOWN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Down_pressed) {
				currentTickCount = 0;
				gKeysStatus.bits.Key_Down_pressed = true;
			}
		}
	}

	if(!pressedState_D)
		if(BUTTON_RELEASED(PD2_KEY_DOWN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Down_released) {
				gKeysStatus.bits.Key_Down_released = true;
				gKeysStatus.bits.Key_Down_pressed = false;
			}
		}

	//Down Button > Down Eval
	if(isPowerOfTwo(pressedState_D)) {
		if(BUTTON_PRESSED(PD1_KEY_UP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Up_pressed) {
				gKeysStatus.bits.Key_Up_pressed = true;
			}
		}
	}

	if(!pressedState_D)
		if(BUTTON_RELEASED(PD1_KEY_UP, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Up_released) {
				gKeysStatus.bits.Key_Up_released = true;
				gKeysStatus.bits.Key_Up_pressed = false;
			}
		}

	//Mode Button > Right Eval
	if(isPowerOfTwo(pressedState_D)) {
		if(BUTTON_PRESSED(PD3_KEY_MODE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Mode_pressed) {
				gKeysStatus.bits.Key_Mode_pressed = true;
			}
		}
	}

	if(!pressedState_D)
		if(BUTTON_RELEASED(PD3_KEY_MODE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Mode_released) {
				gKeysStatus.bits.Key_Mode_released = true;
				gKeysStatus.bits.Key_Mode_pressed = false;
			}
		}

	/********** End of General Button Flags *********/
	//If Key is released reset currentTickCount
	if(gKeysStatus.byte.HB_Released)
		currentTickCount = 0;

}

void keysProcessFlags_E(uint8_t keyState, uint8_t keyChanged)
{
	uint8_t pressedState_E = keyState & ~(0xF1);

	static uint8_t keyStpPressedFirst;
	/********** General Button Flags *********/


	//Enter Button > Select Eval
	if(isPowerOfTwo(pressedState_E)) {
		if(BUTTON_PRESSED(PE1_KEY_ENTER, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Enter_pressed) {
				currentTickCount = 0;
				gKeysStatus.bits.Key_Enter_pressed = true;
			}
		}
	}

	if(!pressedState_E)
		if(BUTTON_RELEASED(PE1_KEY_ENTER, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Enter_released) {
				gKeysStatus.bits.Key_Enter_released = true;
				gKeysStatus.bits.Key_Enter_pressed = false;
			}
		}

	//Open Button > NO BUTTON ON EVAL @LEFT
//	if(isPowerOfTwo(pressedState_E)) {
//		if(BUTTON_PRESSED(PE2_KEY_OPEN, keyState, keyChanged)) {
//			if(!gKeysStatus.bits.Key_Open_pressed) {
//				gKeysStatus.bits.Key_Open_pressed = true;
//				currentTickCount = 0;
//			}
//		}
//	}
//
//	if(!pressedState_E)
//		if(BUTTON_RELEASED(PE2_KEY_OPEN, keyState, keyChanged)) {
//			if(!gKeysStatus.bits.Key_Open_released) {
//				gKeysStatus.bits.Key_Open_released = true;
//				gKeysStatus.bits.Key_Open_pressed = false;
//			}
//		}

	//UP Button > Up Eval
	if(isPowerOfTwo(pressedState_E)) {
		if(BUTTON_PRESSED(PE2_KEY_OPEN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Up_pressed) {
				currentTickCount = 0;
				gKeysStatus.bits.Key_Up_pressed = true;
			}
		}
	}

	if(!pressedState_E)
		if(BUTTON_RELEASED(PE2_KEY_OPEN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Up_released) {
				gKeysStatus.bits.Key_Up_released = true;
				gKeysStatus.bits.Key_Up_pressed = false;
			}
		}

	//Close Button > NO BUTTON ON EVAL @SELECT
	if(isPowerOfTwo(pressedState_E)) {
		if(BUTTON_PRESSED(PE3_KEY_CLOSE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Close_pressed) {
				currentTickCount = 0;
				gKeysStatus.bits.Key_Close_pressed = true;

			}
		}
	}

	if(!pressedState_E)
		if(BUTTON_RELEASED(PE3_KEY_CLOSE, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_Close_released) {
				gKeysStatus.bits.Key_Close_released = true;
				gKeysStatus.bits.Key_Close_pressed = false;
			}
		}

	/********** End of General Button Flags *********/
	//If Key is released reset currentTickCount
	if(gKeysStatus.byte.HB_Released)
		currentTickCount = 0;
}
#endif //-----DISP_LAUNCHPAD_BOARD
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
}


/******************************************************************************
 * Function Name: 			ValidateDebounce
 *
 * Function Description: 	Call function to validate the debounce occurrence count for pressed or released events with configured individual debounce period
 *
 * Function Parameters:
 *
 * Function Returns: 		Function will return the status which will indicate the signal which fulfill the debounce limit
 *
 ********************************************************************************/
uint32_t ValidateDebounce(uint32_t luintDelta,uint32_t luintLastState,uint8_t *lpCounter,uint8_t *lpPressedDebounceLimit,uint8_t *lpReleasedDebounceLimit)
{

	uint32_t luiScanBits;
	uint8_t lucLoop,lucLoop1;
	uint32_t luiRetDelta;
	uint8_t lucCounter;

	luiRetDelta = 0;
	luiScanBits = 1;

	// Scan 'luintDelta' for variable debounce time elapsed for press and release events
	for (lucLoop = 0 ; lucLoop < 8; lucLoop++)
	{

		if (luintDelta & luiScanBits)		   // check for change in last state and current state
		{
			lucCounter = 0;

			for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++) // scan specific bits of counter to get the current value (counter is distributed across 6 variables)
			{

				lucCounter = lucCounter << 1;

				if ((*(lpCounter + lucLoop1)) & luiScanBits) // lpCounter will point to MSB to LSB
				{

					lucCounter = lucCounter | 0x01;

				}

			} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)


			if (luintLastState & luiScanBits) // Validate key press debounce
			{

				if (lucCounter == *(lpPressedDebounceLimit + lucLoop))
				{

					luiRetDelta = luiRetDelta | luiScanBits;

				}

			}
			else // Validate key release debounce
			{

				if (lucCounter == *(lpReleasedDebounceLimit + lucLoop))
				{

					luiRetDelta = luiRetDelta | luiScanBits;

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
			for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)
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
	uint8_t lucCounter;

	static uint8_t ui8TempReleaseCounter[3] = {0,0,0}; // 0 = MSB ....2 = LSB

	if (cucTempReleaseMaxLimit == 0)
	{
		*lpCounter = *lpCounter & ui32Delta;
		*(lpCounter + 1) = *(lpCounter + 1) & ui32Delta;
		*(lpCounter + 2) = *(lpCounter + 2) & ui32Delta;
		*(lpCounter + 3) = *(lpCounter + 3) & ui32Delta;
		*(lpCounter + 4) = *(lpCounter + 4) & ui32Delta;
		*(lpCounter + 5) = *(lpCounter + 5) & ui32Delta;
	}
	else
	{

		luiScanBits = 1;

		for (lucLoop = 0 ; lucLoop < 8; lucLoop++) // scan counter alloctaed to every input
		{

				lucCounter = 0;

				for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++) // scan specific bits of counter to get the current value (counter is distributed across 6 variables)
				{

					lucCounter = lucCounter << 1;

					if ((*(lpCounter + lucLoop1)) & luiScanBits) // lpCounter will point to MSB to LSB
					{

						lucCounter = lucCounter | 0x01;

					}

				} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)

				// Check for No press condition (lpCounter == 1, ui32Delta == 0) and reset the lpCounter, 'Local Temp release Counter'
				if ((lucCounter == 1) && ((ui32Delta & luiScanBits) == 0))
				{
					*lpCounter = *lpCounter & (~luiScanBits);
					*(lpCounter + 1) = *(lpCounter + 1) & (~luiScanBits);
					*(lpCounter + 2) = *(lpCounter + 2) & (~luiScanBits);
					*(lpCounter + 3) = *(lpCounter + 3) & (~luiScanBits);
					*(lpCounter + 4) = *(lpCounter + 4) & (~luiScanBits);
					*(lpCounter + 5) = *(lpCounter + 5) & (~luiScanBits);

					ui8TempReleaseCounter[0] &= (~luiScanBits);
					ui8TempReleaseCounter[1] &= (~luiScanBits);
					ui8TempReleaseCounter[2] &= (~luiScanBits);
				}
				// Check for Temp Released recovered within allowed limit (lpCounter > 0,ui32Delta == 1) and reset 'Local Temp release Counter'
				else if ((lucCounter > 1) && (ui32Delta & luiScanBits))
				{

					ui8TempReleaseCounter[0] &= (~luiScanBits);
					ui8TempReleaseCounter[1] &= (~luiScanBits);
					ui8TempReleaseCounter[2] &= (~luiScanBits);

				}
				// Check for Temp release condition (lpCounter > 1, ui32Delta == 0), decrement 'lpCounter', increment 'Local Temp release Counter'
				// If 'Local Temp release Counter' reches to 'cucTempReleaseMaxLimit', then reset 'lpCounter', 'Local Temp release Counter'
				else if ((lucCounter > 1) && ((ui32Delta & luiScanBits) == 0))
				{

					//decrement 'lpCounter'...................................................................

						// Decrement the local counter value
						lucCounter = lucCounter - 1;

						// Reset the 'lpCounter'
						*lpCounter = *lpCounter & (~luiScanBits);	//0 = MSB ....5 = LSB
						*(lpCounter + 1) = *(lpCounter + 1) & (~luiScanBits);
						*(lpCounter + 2) = *(lpCounter + 2) & (~luiScanBits);
						*(lpCounter + 3) = *(lpCounter + 3) & (~luiScanBits);
						*(lpCounter + 4) = *(lpCounter + 4) & (~luiScanBits);
						*(lpCounter + 5) = *(lpCounter + 5) & (~luiScanBits);

						// set the new value in 'lpCounter'
						lucScanBits1 = 0x20;//0b00100000;
						for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)
						{

							if (lucCounter & lucScanBits1)
							{

								*(lpCounter + lucLoop1) = *(lpCounter + lucLoop1) | luiScanBits;

							}

							lucScanBits1 = lucScanBits1 >> 1;

						} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)

					//increment 'Local Temp release Counter' .................................................
						// compute curent value of 'Local Temp release Counter'
						lucCounter = 0;

						for (lucLoop1 = 0 ; lucLoop1 < 3; lucLoop1++) // scan specific bits of temp release counter to get the current value (counter is distributed across 3 variables)
						{

							lucCounter = lucCounter << 1;

							if ((*(ui8TempReleaseCounter + lucLoop1)) & luiScanBits) // lpCounter will point to MSB to LSB
							{

								lucCounter = lucCounter | 0x01;

							}

						} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)

						lucCounter = lucCounter + 1;

						// Reset the 'Local Temp release Counter'
						*ui8TempReleaseCounter = *ui8TempReleaseCounter & (~luiScanBits);	//0 = MSB ....5 = LSB
						*(ui8TempReleaseCounter + 1) = *(ui8TempReleaseCounter + 1) & (~luiScanBits);
						*(ui8TempReleaseCounter + 2) = *(ui8TempReleaseCounter + 2) & (~luiScanBits);

						// set the new value in 'Local Temp release Counter'
						lucScanBits1 = 0x04;//0b00000100;
						for (lucLoop1 = 0 ; lucLoop1 < 3; lucLoop1++)
						{

							if (lucCounter & lucScanBits1)
							{

								*(ui8TempReleaseCounter + lucLoop1) = *(ui8TempReleaseCounter + lucLoop1) | luiScanBits;

							}

							lucScanBits1 = lucScanBits1 >> 1;

						} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)


				    // If 'Local Temp release Counter' reches to 'cucTempReleaseMaxLimit', then reset 'lpCounter', 'Local Temp release Counter'

				    	// compute curent value of 'Local Temp release Counter'
						lucCounter = 0;

						for (lucLoop1 = 0 ; lucLoop1 < 3; lucLoop1++) // scan specific bits of temp release counter to get the current value (counter is distributed across 3 variables)
						{

							lucCounter = lucCounter << 1;

							if ((*(ui8TempReleaseCounter + lucLoop1)) & luiScanBits) // lpCounter will point to MSB to LSB
							{

								lucCounter = lucCounter | 0x01;

							}

						} //for (lucLoop1 = 0 ; lucLoop1 < 6; lucLoop1++)


					if (lucCounter > cucTempReleaseMaxLimit)
					{

						// Reset the 'lpCounter'
						*lpCounter = *lpCounter & (~luiScanBits);	//0 = MSB ....5 = LSB
						*(lpCounter + 1) = *(lpCounter + 1) & (~luiScanBits);
						*(lpCounter + 2) = *(lpCounter + 2) & (~luiScanBits);
						*(lpCounter + 3) = *(lpCounter + 3) & (~luiScanBits);
						*(lpCounter + 4) = *(lpCounter + 4) & (~luiScanBits);
						*(lpCounter + 5) = *(lpCounter + 5) & (~luiScanBits);

						// Reset the 'Local Temp release Counter'
						ui8TempReleaseCounter[0] &= (~luiScanBits);
						ui8TempReleaseCounter[1] &= (~luiScanBits);
						ui8TempReleaseCounter[2] &= (~luiScanBits);

					} //if (lucCounter > cucTempReleaseMaxLimit)

				} //else if ((lucCounter > 1) && ((ui32Delta & luiScanBits) == 0))

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
