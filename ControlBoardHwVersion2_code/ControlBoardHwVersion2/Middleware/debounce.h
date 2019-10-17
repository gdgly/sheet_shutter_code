/*********************************************************************************
* FileName: debounce.h
* Description:
* This header file contains interfaces for the Key Debounce module mechanism.
* Version: 0.4D
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
 *  	0.4D	09/07/2014									Modifying change for new logic and port pin change
 *  	0.3D	03/07/2014									 Added 3secStop pressed signal bit
 *  	0.2D	19/06/2014									  Added generic naming for KEYS
 *  	0.1D	31/03/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/
#ifndef __DEBOUNCE_H__
#define __DEBOUNCE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "Drivers/systicktimer.h" //Added for MS_PER_SYSTICK
#include "Middleware/system.h"
#include <inc/hw_memmap.h>
#include <driverlib/rom.h>
/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define CTRL_TARGET_BOARD
//#define CTRL_LAUNCHPAD_BOARD

//#define DEBOUNCE_WITH_PRIORITY_LOGIC	//	Added on 22 Oct to to select between two debounce logics
										//	One with priority among stop-open-close keys
										//	One without key press priorities written by Girish

//#define SYSTICK_3SEC_KEYPRESS ((3*1000)/MS_PER_SYSTICK) //Gets dynamically configured
#define SYSTICK_3SEC_KEYPRESS ((3*1000)/MS_PER_SYSTICK) //Gets dynamically configured

//-----------CTRL_TARGET_BOARD-------------------
#ifdef CTRL_TARGET_BOARD

#define COUNTER_GPIO_PERIPH		SYSCTL_PERIPH_GPIOE
#define COUNTER_GPIO_BASE		GPIO_PORTE_BASE
#define COUNTER_OUT				GPIO_PIN_4	//	electro-mechanical counter
#define COUNTER_OUT_HIGH		0
#define COUNTER_OUT_LOW			COUNTER_OUT

#define INTERLOCK_OUT_GPIO_PERIPH		SYSCTL_PERIPH_GPIOB
#define INTERLOCK_OUT_GPIO_BASE			GPIO_PORTB_BASE
#define INTERLOCK_OUT					GPIO_PIN_3
#define INTERLOCK_OUT_HIGH				0
#define INTERLOCK_OUT_LOW				INTERLOCK_OUT
#define RELAY_CLOSE						(ROM_GPIOPinWrite(INTERLOCK_OUT_GPIO_BASE, INTERLOCK_OUT, INTERLOCK_OUT_HIGH))
#define RELAY_OPEN						(ROM_GPIOPinWrite(INTERLOCK_OUT_GPIO_BASE, INTERLOCK_OUT, INTERLOCK_OUT_LOW))

#define MONITOR_LED_GPIO_PERIPH		SYSCTL_PERIPH_GPIOB
#define MONITOR_LED_GPIO_BASE		GPIO_PORTB_BASE
#define MONITOR_LED					GPIO_PIN_4
#define MONITOR_LED_HIGH			0
#define MONITOR_LED_LOW				MONITOR_LED

#ifdef VERSION_1HARDWARE
#define MULTI_FUNC_OUT_1_GPIO_PERIPH		SYSCTL_PERIPH_GPIOB
#define MULTI_FUNC_OUT_1_GPIO_BASE			GPIO_PORTB_BASE
#define MULTI_FUNC_OUT_1					GPIO_PIN_5
#define MULTI_FUNC_OUT_1_HIGH				0
#define MULTI_FUNC_OUT_1_LOW				MULTI_FUNC_OUT_1
#define ACTIVATE_MULTI_FUNC_OUT_1			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_1_GPIO_BASE,MULTI_FUNC_OUT_1,MULTI_FUNC_OUT_1_HIGH))
#define DEACTIVATE_MULTI_FUNC_OUT_1			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_1_GPIO_BASE,MULTI_FUNC_OUT_1,MULTI_FUNC_OUT_1_LOW))

#define MULTI_FUNC_OUT_2_GPIO_PERIPH		SYSCTL_PERIPH_GPIOD
#define MULTI_FUNC_OUT_2_GPIO_BASE			GPIO_PORTD_BASE
#define MULTI_FUNC_OUT_2					GPIO_PIN_6
#define MULTI_FUNC_OUT_2_HIGH				0
#define MULTI_FUNC_OUT_2_LOW				MULTI_FUNC_OUT_2
#define ACTIVATE_MULTI_FUNC_OUT_2			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_2_GPIO_BASE,MULTI_FUNC_OUT_2,MULTI_FUNC_OUT_2_HIGH))
#define DEACTIVATE_MULTI_FUNC_OUT_2			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_2_GPIO_BASE,MULTI_FUNC_OUT_2,MULTI_FUNC_OUT_2_LOW))
#endif	//	VERSION_1HARDWARE

#ifdef VERSION_2HARDWARE

//	Multi-function out 1 PF3
#define MULTI_FUNC_OUT_1_GPIO_PERIPH		SYSCTL_PERIPH_GPIOF
#define MULTI_FUNC_OUT_1_GPIO_BASE			GPIO_PORTF_BASE
#define MULTI_FUNC_OUT_1					GPIO_PIN_3
#define MULTI_FUNC_OUT_1_HIGH				0
#define MULTI_FUNC_OUT_1_LOW				MULTI_FUNC_OUT_1
#define ACTIVATE_MULTI_FUNC_OUT_1			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_1_GPIO_BASE,MULTI_FUNC_OUT_1,MULTI_FUNC_OUT_1_HIGH))
#define DEACTIVATE_MULTI_FUNC_OUT_1			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_1_GPIO_BASE,MULTI_FUNC_OUT_1,MULTI_FUNC_OUT_1_LOW))

//	Multi-function out 2 PF2
#define MULTI_FUNC_OUT_2_GPIO_PERIPH		SYSCTL_PERIPH_GPIOF
#define MULTI_FUNC_OUT_2_GPIO_BASE			GPIO_PORTF_BASE
#define MULTI_FUNC_OUT_2					GPIO_PIN_2
#define MULTI_FUNC_OUT_2_HIGH				0
#define MULTI_FUNC_OUT_2_LOW				MULTI_FUNC_OUT_2
#define ACTIVATE_MULTI_FUNC_OUT_2			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_2_GPIO_BASE,MULTI_FUNC_OUT_2,MULTI_FUNC_OUT_2_HIGH))
#define DEACTIVATE_MULTI_FUNC_OUT_2			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_2_GPIO_BASE,MULTI_FUNC_OUT_2,MULTI_FUNC_OUT_2_LOW))


//	Multi-function out 3 PF0
#define MULTI_FUNC_OUT_3_GPIO_PERIPH		SYSCTL_PERIPH_GPIOF
#define MULTI_FUNC_OUT_3_GPIO_BASE			GPIO_PORTF_BASE
#define MULTI_FUNC_OUT_3					GPIO_PIN_0
#define MULTI_FUNC_OUT_3_HIGH				0
#define MULTI_FUNC_OUT_3_LOW				MULTI_FUNC_OUT_3
#define ACTIVATE_MULTI_FUNC_OUT_3			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_3_GPIO_BASE,MULTI_FUNC_OUT_3,MULTI_FUNC_OUT_3_HIGH))
#define DEACTIVATE_MULTI_FUNC_OUT_3			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_3_GPIO_BASE,MULTI_FUNC_OUT_3,MULTI_FUNC_OUT_3_LOW))

//	Multi-function out 4 PF1
#define MULTI_FUNC_OUT_4_GPIO_PERIPH		SYSCTL_PERIPH_GPIOF
#define MULTI_FUNC_OUT_4_GPIO_BASE			GPIO_PORTF_BASE
#define MULTI_FUNC_OUT_4					GPIO_PIN_1
#define MULTI_FUNC_OUT_4_HIGH				0
#define MULTI_FUNC_OUT_4_LOW				MULTI_FUNC_OUT_4
#define ACTIVATE_MULTI_FUNC_OUT_4			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_4_GPIO_BASE,MULTI_FUNC_OUT_4,MULTI_FUNC_OUT_4_HIGH))
#define DEACTIVATE_MULTI_FUNC_OUT_4			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_4_GPIO_BASE,MULTI_FUNC_OUT_4,MULTI_FUNC_OUT_4_LOW))

//	Multi-function out 5 PA7
#define MULTI_FUNC_OUT_5_GPIO_PERIPH		SYSCTL_PERIPH_GPIOA
#define MULTI_FUNC_OUT_5_GPIO_BASE			GPIO_PORTA_BASE
#define MULTI_FUNC_OUT_5					GPIO_PIN_7
#define MONIDENGLU_KEY                      GPIO_PIN_5
#define MULTI_FUNC_OUT_5_HIGH				0
#define MULTI_FUNC_OUT_5_LOW				MULTI_FUNC_OUT_5
#define ACTIVATE_MULTI_FUNC_OUT_5			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE,MULTI_FUNC_OUT_5,MULTI_FUNC_OUT_5_HIGH))
#define DEACTIVATE_MULTI_FUNC_OUT_5			(ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE,MULTI_FUNC_OUT_5,MULTI_FUNC_OUT_5_LOW))

#endif	//	VERSION_2HARDWARE


//	Added on 05 Dec 2014 as per new requirement from client
#define WIRELESS_MODE_CHANGE_GPIO_PERIPH		SYSCTL_PERIPH_GPIOA
#define WIRELESS_MODE_CHANGE_GPIO_BASE			GPIO_PORTA_BASE
#define WIRELESS_MODE_CHANGE					GPIO_PIN_5
#define WIRELESS_MODE_CHANGE_HIGH				WIRELESS_MODE_CHANGE
#define WIRELESS_MODE_CHANGE_LOW				0
#define ACTIVATE_WIRELESS_MODE_CHANGE			(ROM_GPIOPinWrite(WIRELESS_MODE_CHANGE_GPIO_BASE,WIRELESS_MODE_CHANGE,WIRELESS_MODE_CHANGE_HIGH))
#define DEACTIVATE_WIRELESS_MODE_CHANGE			(ROM_GPIOPinWrite(WIRELESS_MODE_CHANGE_GPIO_BASE,WIRELESS_MODE_CHANGE,WIRELESS_MODE_CHANGE_LOW))

#define NUM_BUTTONS				3

#ifdef VERSION_1HARDWARE

#define BUTTONS1_GPIO_PERIPH     SYSCTL_PERIPH_GPIOB
#define BUTTONS1_GPIO_BASE       GPIO_PORTB_BASE
#define BUTTONS2_GPIO_PERIPH     SYSCTL_PERIPH_GPIOF
#define BUTTONS2_GPIO_BASE       GPIO_PORTF_BASE

#define PB7_3PBS_OPEN			GPIO_PIN_7
#define PB6_3PBS_CLOSE			GPIO_PIN_6
#define PF4_3PBS_STOP			GPIO_PIN_4

#define KEY_3PBS_OPEN			(1 << 0)
#define KEY_3PBS_CLOSE			(1 << 1)
#define KEY_3PBS_STOP			(1 << 2)

#define _ONLY_STOPKEY_PRESSED	KEY_3PBS_STOP
#define INSTALLATION_KEYS		(KEY_3PBS_OPEN | KEY_3PBS_CLOSE | KEY_3PBS_STOP)

#define ALL_BUTTONS_PB			(PB7_3PBS_OPEN | PB6_3PBS_CLOSE)
#define ALL_BUTTONS_PF			PF4_3PBS_STOP
#endif	//	VERSION_1HARDWARE

#ifdef VERSION_2HARDWARE

    #ifdef Enable_WATCHDOG_CHECK
        #define WATCHDOG_CHECK_GPIO_PERIPH     SYSCTL_PERIPH_GPIOD
        #define WATCHDOG_CHECK_GPIO_BASE       GPIO_PORTD_BASE
        #define WATCHDOG_CHECK_PE			   GPIO_PIN_3

    #endif
/*
 * 		3PBS pins configuration macros start here
 */
#define BUTTONS1_GPIO_PERIPH     SYSCTL_PERIPH_GPIOE
#define BUTTONS1_GPIO_BASE       GPIO_PORTE_BASE

#define PE3_3PBS_OPEN			GPIO_PIN_3
#define PE2_3PBS_CLOSE			GPIO_PIN_2
#define PE1_3PBS_STOP			GPIO_PIN_1

#define KEY_3PBS_OPEN			(1 << 0)
#define KEY_3PBS_CLOSE			(1 << 1)
#define KEY_3PBS_STOP			(1 << 2)

#define _ONLY_STOPKEY_PRESSED	KEY_3PBS_STOP
#define INSTALLATION_KEYS		(KEY_3PBS_OPEN | KEY_3PBS_CLOSE | KEY_3PBS_STOP)

#define ALL_BUTTONS_PE			(PE3_3PBS_OPEN | PE2_3PBS_CLOSE | PE1_3PBS_STOP )

/*
 * 		3PBS pins configuration macros end here
 */

/*
 * 		Wireless 3PBS pins configuration macros start here
 */
#define BUTTONS1_GPIO_PERIPH_WIRELESS     SYSCTL_PERIPH_GPIOB
#define BUTTONS1_GPIO_BASE_WIRELESS       GPIO_PORTB_BASE
#define BUTTONS2_GPIO_PERIPH_WIRELESS     SYSCTL_PERIPH_GPIOF
#define BUTTONS2_GPIO_BASE_WIRELESS       GPIO_PORTF_BASE

#define PB7_3PBS_CLOSE_WIRELESS			GPIO_PIN_7
#define PB6_3PBS_OPEN_WIRELESS			GPIO_PIN_6
#define PF4_3PBS_STOP_WIRELESS			GPIO_PIN_4

#define KEY_3PBS_OPEN_WIRELESS			(1 << 3)
#define KEY_3PBS_CLOSE_WIRELESS			(1 << 4)
#define KEY_3PBS_STOP_WIRELESS			(1 << 5)

#define _ONLY_STOPKEY_PRESSED_WIRELESS	KEY_3PBS_STOP_WIRELESS
#define INSTALLATION_KEYS_WIRELESS		(KEY_3PBS_OPEN_WIRELESS | KEY_3PBS_CLOSE_WIRELESS | KEY_3PBS_STOP_WIRELESS)

#define ALL_BUTTONS_PB_WIRELESS			(PB7_3PBS_CLOSE_WIRELESS | PB6_3PBS_OPEN_WIRELESS)
#define ALL_BUTTONS_PF_WIRELESS			PF4_3PBS_STOP_WIRELESS
/*
 * 		Wireless 3PBS pins configuration macros end here
 */


#endif	//	VERSION_2HARDWARE

#endif	//	CTRL_TARGET_BOARD
//-----------CTRL_TARGET_BOARD-------------------

//----------CTRL_LAUNCHPAD_BOARD------------------
#ifdef CTRL_LAUNCHPAD_BOARD
#define BUTTONS_GPIO_PERIPH     SYSCTL_PERIPH_GPIOF
#define BUTTONS_GPIO_BASE       GPIO_PORTF_BASE

#define NUM_BUTTONS				2

#define PF4_SW1			GPIO_PIN_4 //OPEN button
#define PF0_SW2			GPIO_PIN_0 //STOP button


#define _ONLY_STOPKEY_PRESSED	PF0_SW2

#define ALL_BUTTONS             (PF0_SW2 | PF4_SW1)
#define INSTALLATION_KEYS		(PF0_SW2 | PF4_SW1)
#endif
//----------CTRL_LAUNCHPAD_BOARD------------------

#define BUTTON_PRESSED(button, buttons, changed)                              \
        (((button) & (changed)) && ((button) & (buttons)))

#define BUTTON_RELEASED(button, buttons, changed)                             \
        (((button) & (changed)) && !((button) & (buttons)))

/****************************************************************************/


/****************************************************************************
 *  Global variables:
****************************************************************************/
typedef union {
	uint32_t Val; 	//4bytes
    uint16_t w[2];
    uint8_t  v[4];

    struct
    {
    	uint16_t LW;
    	uint16_t HW;
    } word;

    struct
    {
    	uint8_t LB_Pressed;
    	uint8_t HB_Released;
    	uint8_t UB_ExtendedFlags;
    	uint8_t MB;
    } byte;

	struct {
    	uint32_t Key_Open_pressed :1;	//0
    	uint32_t Key_Stop_pressed :1;	//1
    	uint32_t Key_Close_pressed:1;	//2
    	uint32_t Wireless_Open_pressed :1;	//3
    	uint32_t Wireless_Stop_pressed :1;	//4
    	uint32_t Wireless_Close_pressed:1;	//5
    	uint32_t bit6			  :1;					//6
    	uint32_t bit7			  :1;					//7

    	uint32_t Key_Open_released:1;	//8
    	uint32_t Key_Stop_released:1;	//9
    	uint32_t Key_Close_released:1;	//10
    	uint32_t Wireless_Open_released:1;	//11
    	uint32_t Wireless_Stop_released :1;	//12
    	uint32_t Wireless_Close_released:1;	//13
    	uint32_t bit11			  :1;					//14
    	uint32_t bit12			  :1;					//15

    	uint32_t Keys2_3secStpCls_pressed:1;			//16
    	uint32_t Keys2_3secStpOpn_pressed:1;			//17
    	uint32_t Keys3_3secOpStCl_pressed:1;			//18 - Any key combination
    	uint32_t Key_3secCls_pressed	 :1;			//19 - Used only for debug/TO BE USED ENTER
    	uint32_t Key_3secStp_pressed	 :1;			//20 - Added on 3 Jul 14
    	uint32_t bit21			  :1;					//21
    	uint32_t bit22			  :1;					//22
    	uint32_t bit23			  :1;					//23

    	uint32_t bit24			  :1;					//24
    	uint32_t bit25			  :1;					//25
    	uint32_t bit26			  :1;					//26
    	uint32_t bit27			  :1;					//27
    	uint32_t bit28			  :1;					//28
    	uint32_t bit29			  :1;					//29
    	uint32_t bit30			  :1;					//30
    	uint32_t bit31			  :1;					//31
	} bits;

} _KEYS_STATUS;

extern _KEYS_STATUS gKeysStatus;


//*****************************************************************************
// Function prototypes.
//*****************************************************************************
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
void initKeys(void);

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
void initOutputGPIOs(void);

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
 * Function Returns:
 * Returns the current debounced state of the buttons where a 1 in the button
 * ID's position indicates that the button is pressed and a 0 indicates that
 * it is released.
 *
 ********************************************************************************/
void keysPoll(uint8_t *pui8Delta, uint8_t *pui8RawState);

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
void keysProcessFlags(uint8_t keyState, uint8_t keyChanged);

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
uint8_t isPowerOfTwo(uint8_t x);

/********************************************************************************/

#endif /*__DEBOUNCE_H__*/
