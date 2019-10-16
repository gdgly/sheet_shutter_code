/*********************************************************************************
* FileName: systicktimer.h
* Description:
* This header file contains the prototypes for SysTick module
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
 *  	0.2D	30/04/2014									  Changed to 1ms INT
 *  	0.1D	07/04/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

#ifndef __SYSTICKTIMER_H__
#define __SYSTICKTIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
//*****************************************************************************
// The clock rate for the SysTick interrupt and a counter of system clock
// ticks.  The SysTick interrupt is used for basic timing in the application.
//*****************************************************************************
#define CLOCK_RATE              1000 //Changed on 30-4-14 for 1ms SysTick
#define MS_PER_SYSTICK          (1000 / CLOCK_RATE)

/****************************************************************************/


/****************************************************************************
 *  Global variables:
****************************************************************************/
extern volatile uint32_t g_ui32TickCount;
extern volatile uint32_t g_ui32TickCount10ms;
/****************************************************************************/


/******************************************************************************
 * Function Prototypes
 ********************************************************************************/
void Init_SysTick(void);
void SysTickIntHandler(void);
uint32_t GetTickms(void);
/********************************************************************************/

#endif /*__SYSTICKTIMER_H__*/
