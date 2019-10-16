/*********************************************************************************
* FileName: debounce.c
* Description:
* This header file contains definations for Key Debounce module mechanism.
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
#include <sysctl.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>
#include "debounce.h"
#include "Drivers/systicktimer.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/


/****************************************************************************
 *  Global variables
****************************************************************************/

_KEYS_STATUS gKeysStatus;
static uint32_t currentTickCount = 0;
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
//void keysPoll(uint8_t *pui8Delta, uint8_t *pui8RawState)
//{
//    uint32_t ui32Delta;
//    uint32_t ui32Data;
//    static uint8_t ui8SwitchClockA = 0;
//    static uint8_t ui8SwitchClockB = 0;
//
//    //
//    // Read the raw state of the push buttons.  Save the raw state
//    // (inverting the bit sense) if the caller supplied storage for the
//    // raw value.
//    //
//    ui32Data = (MAP_GPIOPinRead(BUTTONS_GPIO_BASE, ALL_BUTTONS));
//    if(pui8RawState)
//    {
//        *pui8RawState = (uint8_t)~ui32Data;
//    }
//
//    //
//    // Determine the switches that are at a different state than the debounced
//    // state.
//    //
//    ui32Delta = ui32Data ^ g_ui8ButtonStates;
//
//    //
//    // Increment the clocks by one.
//    //
//    ui8SwitchClockA ^= ui8SwitchClockB;
//    ui8SwitchClockB = ~ui8SwitchClockB;
//
//    //
//    // Reset the clocks corresponding to switches that have not changed state.
//    //
//    ui8SwitchClockA &= ui32Delta;
//    ui8SwitchClockB &= ui32Delta;
//
//    //
//    // Get the new debounced switch state.
//    //
//    g_ui8ButtonStates &= ui8SwitchClockA | ui8SwitchClockB;
//    g_ui8ButtonStates |= (~(ui8SwitchClockA | ui8SwitchClockB)) & ui32Data;
//
//    //
//    // Determine the switches that just changed debounced state.
//    //
//    ui32Delta ^= (ui8SwitchClockA | ui8SwitchClockB);
//
//    //
//    // Store the bit mask for the buttons that have changed for return to
//    // caller.
//    //
//    if(pui8Delta)
//    {
//        *pui8Delta = (uint8_t)ui32Delta;
//    }
//
//    //
//    // Send the debounced buttons states to the keysProcessFlags().
//    // Invert the bit sense so that a '1' indicates the button is pressed,
//    // which is a sensible way to interpret the return value.
//    //
//    keysProcessFlags(~g_ui8ButtonStates, *pui8Delta);
//}

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
void keysProcessFlags_F(uint8_t keyState, uint8_t keyChanged)
{
	uint8_t pressedState_F = keyState & ~(0xEE);

	static uint8_t keyStpPressedFirst;
	/********** General Button Flags *********/

	//AutMan Button > Left Eval
	if(isPowerOfTwo(pressedState_F)) {
		if(BUTTON_PRESSED(PF4_KEY_AUT_MAN, keyState, keyChanged)) {
			if(!gKeysStatus.bits.Key_AutMan_pressed) {
				gKeysStatus.bits.Key_AutMan_pressed = true;
			}
		}
	}

	if(!pressedState_F)

		if( (!gKeysStatus.byte.LB_Pressed) &&
				(BUTTON_RELEASED(PF4_KEY_AUT_MAN, keyState, keyChanged)) ) {
			if(!gKeysStatus.bits.Key_AutMan_released) {
				gKeysStatus.bits.Key_AutMan_released = true;
				gKeysStatus.bits.Key_AutMan_pressed = false;
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
//				if(currentTickCount++ == SYSTICK_3SEC_KEYPRESS)
//					gKeysStatus.bits.Key_3secCls_pressed = true;
//				else
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
//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
