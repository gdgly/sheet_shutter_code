/*********************************************************************************
 * FileName: sensorsdebounce.h
 * Description:
 * This header file contains interfaces for the Sensors Debounce module mechanism.
 * Version: 0.2D
 *
 * Sensor Pins Structure:
 *
 * | SAFETY | INTERLOCKIP | OBSTACLE | 1PBS | MULTIFUNCIP2 | MULTIFUNCIP1 |
 * |	PD2    | PE3         | PD1      | PD3  | PD0          | PE5          |
 * |	bit5   | bit4        | bit3     | bit2 | bit1         | bit0         |
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
 *  	0.2D	07/07/2014									Making GPIOs as per new schematic
 *  	0.1D	17/06/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/
#ifndef __SENSORSDEBOUNCE_H__
#define __SENSORSDEBOUNCE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <driverlib/gpio.h>
#include "Middleware/system.h"
/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/
#define GPIOB_PERIPH     SYSCTL_PERIPH_GPIOB
#define GPIOB_BASE       GPIO_PORTB_BASE

#define GPIOD_PERIPH     SYSCTL_PERIPH_GPIOD
#define GPIOD_BASE       GPIO_PORTD_BASE

#define GPIOE_PERIPH     SYSCTL_PERIPH_GPIOE
#define GPIOE_BASE       GPIO_PORTE_BASE

#define GPIOC_PERIPH     SYSCTL_PERIPH_GPIOC
#define GPIOC_BASE       GPIO_PORTC_BASE

#define NUM_SENSORS	6

#ifdef VERSION_1HARDWARE
#define PIN_SAFETY_PD2			GPIO_PIN_2
#endif

#ifdef VERSION_2HARDWARE
#define PIN_SAFETY_PD2/*PIN_SAFETY_PD1*/			GPIO_PIN_2/*GPIO_PIN_1*/
#endif

#ifdef VERSION_1HARDWARE
#define PIN_INTERLOCKIP_PE3		GPIO_PIN_3
#endif

#ifdef VERSION_2HARDWARE
#define PIN_INTERLOCKIP_PC7		GPIO_PIN_7
#endif

#ifdef VERSION_1HARDWARE
#define PIN_OBSTACLE_PD1		GPIO_PIN_1
#endif

#ifdef VERSION_2HARDWARE
#define PIN_OBSTACLE_PD1/*PIN_OBSTACLE_PD2*/		GPIO_PIN_1/*GPIO_PIN_2*/
#endif

#ifdef VERSION_1HARDWARE
#define PIN_1PBS_PD3			GPIO_PIN_3
#endif

#ifdef VERSION_2HARDWARE
#define PIN_1PBS_PB5			GPIO_PIN_5
#endif

#define PIN_MULTIFUNCIP2_PD0	GPIO_PIN_0
#define PIN_MULTIFUNCIP1_PE5	GPIO_PIN_5

/**********************************************************************************************/
//	Added on 05 Dec 2014 as per new requirement from client
#define GPIOA_PERIPH     			SYSCTL_PERIPH_GPIOA
#define GPIOA_BASE       			GPIO_PORTA_BASE

#define PIN_WIRELESS_1PBS_PA3		GPIO_PIN_3
#define PIN_WIRELESS_MONITOR_PA4	GPIO_PIN_4

#define PIN_WIRELESS_1PBS			(1 << 1)
#define PIN_WIRELESS_MONITOR		(1 << 0)

#define ALL_SENSORS_PA		(PIN_WIRELESS_1PBS_PA3 | PIN_WIRELESS_MONITOR_PA4)
/**********************************************************************************************/

#define PIN_MONITOR			(1 << 6)

#ifdef VERSION_1HARDWARE
#define PIN_SAFETY			(1 << 5)
#endif

#ifdef VERSION_2HARDWARE
#define PIN_SAFETY			(1 << 5)
#endif

#ifdef VERSION_1HARDWARE
#define PIN_INTERLOCKIP		(1 << 4)
#endif

#ifdef VERSION_2HARDWARE
#define PIN_INTERLOCKIP		(1 << 4)
#endif

#ifdef VERSION_1HARDWARE
#define PIN_OBSTACLE		(1 << 3)
#endif

#ifdef VERSION_2HARDWARE
#define PIN_OBSTACLE		(1 << 3)
#endif

#ifdef VERSION_1HARDWARE
#define PIN_1PBS			(1 << 2)
#endif

#ifdef VERSION_2HARDWARE
#define PIN_1PBS			(1 << 2)
#endif

#ifdef VERSION_1HARDWARE
#define PIN_MULTIFUNCIP2	(1 << 1)
#define PIN_MULTIFUNCIP1	(1 << 0)
#endif

#ifdef VERSION_1HARDWARE
#define ALL_SENSORS_PD	(PIN_SAFETY_PD2 | PIN_OBSTACLE_PD1 | PIN_1PBS_PD3 | PIN_MULTIFUNCIP2_PD0)
#endif

#ifdef VERSION_2HARDWARE
#define ALL_SENSORS_PD	(/*PIN_SAFETY_PD1*/PIN_SAFETY_PD2 | PIN_OBSTACLE_PD1/*PIN_OBSTACLE_PD2*/ | PIN_1PBS_PB5 | PIN_MULTIFUNCIP2_PD0)
#endif

#ifdef VERSION_1HARDWARE
#define ALL_SENSORS_PE	(PIN_INTERLOCKIP_PE3 | PIN_MULTIFUNCIP1_PE5)
#endif

#ifdef VERSION_2HARDWARE
#define ALL_SENSORS_PC	(PIN_INTERLOCKIP_PC7)
#endif

#define SENSOR_ACTIVE(sensor, sensors, changed)                              \
        (((sensor) & (changed)) && ((sensor) & (sensors)))

#define SENSOR_INACTIVE(sensor, sensors, changed)                             \
        (((sensor) & (changed)) && !((sensor) & (sensors)))

/****************************************************************************/

/****************************************************************************
 *  Global variables:
 ****************************************************************************/
typedef union {
	uint32_t Val; 	//4bytes
	uint16_t w[2];
	uint8_t v[4];

	struct {
		uint16_t LW;
		uint16_t HW;
	} word;

	struct {
		uint8_t LB_Active;
		uint8_t HB_Released;
		uint8_t UB;
		uint8_t MB;
	} byte;

	struct {
		uint32_t Sensor_Safety_active :1;	//0
		uint32_t Sensor_InterlockIP_active :1;	//1
		uint32_t Sensor_Obstacle_active :1;	//2
		uint32_t Sensor_1PBS_active :1;					//3
		uint32_t Sensor_MultifuncIP2_active :1;					//4
		uint32_t Sensor_MultifuncIP1_active :1;					//5
		uint32_t Sensor_Wireless_1PBS_active :1;					//6
		uint32_t bit7 :1;					//7

		uint32_t Sensor_Safety_inactive :1;	//8
		uint32_t Sensor_InterlockIP_inactive :1;	//9
		uint32_t Sensor_Obstacle_inactive :1;	//10
		uint32_t Sensor_1PBS_inactive :1;					//11
		uint32_t Sensor_MultifuncIP2_inactive :1;					//12
		uint32_t Sensor_MultifuncIP1_inactive :1;					//13
		uint32_t Sensor_Wireless_1PBS_inactive :1;					//14
		uint32_t bit12 :1;					//15

		uint32_t bit16 :1;	//16
		uint32_t bit17 :1;	//17
		uint32_t bit18 :1;	//18
		uint32_t bit19 :1;			//19
		uint32_t bit20 :1;					//20 -
		uint32_t bit21 :1;					//21
		uint32_t bit22 :1;					//22
		uint32_t bit23 :1;					//23

		uint32_t bit24 :1;					//24
		uint32_t bit25 :1;					//25
		uint32_t bit26 :1;					//26
		uint32_t bit27 :1;					//27
		uint32_t bit28 :1;					//28
		uint32_t bit29 :1;					//29
		uint32_t bit30 :1;					//30
		uint32_t bit31 :1;					//31
	} bits;

} _SENSOR_STATUS;

extern _SENSOR_STATUS gSensorStatus;

//*****************************************************************************
// Function prototypes.
//*****************************************************************************
/******************************************************************************
 * Function Name: initSensors
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
void initSensors(void);

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
void sensorsPoll(uint8_t *pui8Delta, uint8_t *pui8RawState);

/******************************************************************************
 * Function Name: sensorsProcessFlags
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
void sensorsProcessFlags(uint8_t sensorState, uint8_t sensorChanged);

/********************************************************************************/

#endif /*__SENSORSDEBOUNCE_H__*/
