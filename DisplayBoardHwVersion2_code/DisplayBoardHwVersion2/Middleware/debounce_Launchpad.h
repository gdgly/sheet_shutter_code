/*********************************************************************************
* FileName: debounce.h
* Description:
* This header file contains interfaces for the Key Debounce module mechanism.
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
#ifndef __DEBOUNCE_H__
#define __DEBOUNCE_H__

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define SYSTICK_3SEC_KEYPRESS 300 //Configured for 10ms SysTick Interrupt

//#define BUTTONS_GPIO_PERIPH     SYSCTL_PERIPH_GPIOM
//#define BUTTONS_GPIO_BASE       GPIO_PORTM_BASE

#define BUTTONS_GPIO_PERIPH_F     SYSCTL_PERIPH_GPIOF
#define BUTTONS_GPIO_BASE_F       GPIO_PORTF_BASE

#define BUTTONS_GPIO_PERIPH_D     SYSCTL_PERIPH_GPIOD
#define BUTTONS_GPIO_BASE_D       GPIO_PORTD_BASE

#define BUTTONS_GPIO_PERIPH_E     SYSCTL_PERIPH_GPIOE
#define BUTTONS_GPIO_BASE_E       GPIO_PORTE_BASE

#define NUM_BUTTONS             5


#define _ONLY_STOPKEY_PRESSED	PF0_KEY_STOP



#define BUTTON_PRESSED(button, buttons, changed)                              \
        (((button) & (changed)) && ((button) & (buttons)))

#define BUTTON_RELEASED(button, buttons, changed)                             \
        (((button) & (changed)) && !((button) & (buttons)))

//#define PM0_KEY_UP               GPIO_PIN_0
//#define PM1_KEY_DOWN             GPIO_PIN_1
//#define PF5_KEY_OPEN             GPIO_PIN_2
//#define PM3_KEY_MODE            GPIO_PIN_3
//#define PM4_KEY_ENTER           GPIO_PIN_4

#define PF4_KEY_AUT_MAN		GPIO_PIN_4
#define PF0_KEY_STOP		GPIO_PIN_0

#define PD1_KEY_UP			GPIO_PIN_1
#define PD2_KEY_DOWN		GPIO_PIN_2
#define PD3_KEY_MODE		GPIO_PIN_3

#define PE1_KEY_ENTER		GPIO_PIN_1
#define PE2_KEY_OPEN		GPIO_PIN_2
#define PE3_KEY_CLOSE		GPIO_PIN_3


#define ALL_BUTTONS_F		(PF4_KEY_AUT_MAN | PF0_KEY_STOP)

#define ALL_BUTTONS_D		(PD1_KEY_UP | PD2_KEY_DOWN | PD3_KEY_MODE)

#define ALL_BUTTONS_E		(PE1_KEY_ENTER | PE2_KEY_OPEN | PE3_KEY_CLOSE)

//#define INSTALLATION_KEYS		(PF3_KEY_CLOSE | PF4_KEY_STOP | PF5_KEY_OPEN)
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
		uint32_t Key_Open_pressed:1;	//0
		uint32_t Key_Stop_pressed:1;	//1
		uint32_t Key_Close_pressed:1;	//2
		uint32_t Key_Up_pressed:1;		//3
		uint32_t Key_Down_pressed:1;	//4
		uint32_t Key_Mode_pressed:1;	//5
		uint32_t Key_Enter_pressed:1;	//6
		uint32_t Key_AutMan_pressed:1;	//7

		uint32_t Key_Open_released:1;	//8
		uint32_t Key_Stop_released:1;	//9
		uint32_t Key_Close_released:1;	//10
		uint32_t Key_Up_released:1;		//11
		uint32_t Key_Down_released:1;	//12
		uint32_t Key_Mode_released:1;	//13
		uint32_t Key_Enter_released:1;	//14
		uint32_t Key_AutMan_released:1;	//15

		uint32_t Keys2_3secStpCls_pressed:1;	//16
		uint32_t Keys2_3secStpOpn_pressed:1;	//17
		uint32_t Keys3_3secOpStCl_pressed:1;	//18 - Any key combination
		uint32_t Key_3secCls_pressed:1;			//19 - Used only for debug/TO BE USED ENTER
		uint32_t Key_3secEnter_pressed:1;		//20 -
		uint32_t bit21;
		uint32_t bit22;
		uint32_t bit23;
		uint32_t bit24;
		uint32_t bit25;
		uint32_t bit26;
		uint32_t bit27;
		uint32_t bit28;
		uint32_t bit29;
		uint32_t bit30;
		uint32_t bit31;
	} bits;

} _KEYS_STATUS;

extern _KEYS_STATUS gKeysStatus;
//static uint8_t g_ui8ButtonStates = ALL_BUTTONS;

static uint8_t g_ui8ButtonStates_F = ALL_BUTTONS_F;
static uint8_t g_ui8ButtonStates_D = ALL_BUTTONS_D;
static uint8_t g_ui8ButtonStates_E = ALL_BUTTONS_E;
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
extern void initKeys(void);

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
//extern void keysPoll(uint8_t *pui8Delta, uint8_t *pui8RawState);
extern void keysPoll_F(uint8_t *pui8Delta, uint8_t *pui8RawState);
extern void keysPoll_D(uint8_t *pui8Delta, uint8_t *pui8RawState);
extern void keysPoll_E(uint8_t *pui8Delta, uint8_t *pui8RawState);

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
void keysProcessFlags_F(uint8_t keyState, uint8_t keyChanged);
void keysProcessFlags_D(uint8_t keyState, uint8_t keyChanged);
void keysProcessFlags_E(uint8_t keyState, uint8_t keyChanged);

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
