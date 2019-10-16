/*********************************************************************************
 * FileName: system.c
 * Description:
 * This source file contains the definition of all the services of ...
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
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Include:
 ****************************************************************************/
#include "system.h"
#include <stdint.h>
#include <stdbool.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/eeprom.h>
#include <stdint.h>
#include <stdbool.h>
#include <driverlib/sysctl.h>
#include "Middleware/rtc.h"
#include "TestModule/testdebounce.h"
#include "Middleware/debounce.h"
#include "Drivers/systicktimer.h"
#include "Middleware/paramdatabase.h"
#include "Application/dbhandler.h"
#include "Drivers/ustdlib.h"
#include "Middleware/eeprom.h"
#include "Drivers/watchdogtimer.h"
#include "Middleware/sensorsdebounce.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/


/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
 ****************************************************************************/


/****************************************************************************/


/******************************************************************************
 * systemInit
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void
systemInit(void)
{
	//
	// Enable lazy stacking for interrupt handlers.  This allows floating-point
	// instructions to be used within interrupt handlers, but at the expense of
	// extra stack usage.
	//
	ROM_FPULazyStackingEnable();

	//
	// Set the clocking to run directly from the crystal.
	//
/*	ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
			SYSCTL_XTAL_16MHZ);*/
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
			SYSCTL_XTAL_16MHZ);
	Init_SysTick();
	initSensors();

	//
	// Enable processor interrupts.
	//
	ROM_IntMasterEnable();

	//
	//	Initialize EEPROM
	//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
	EEPROMInit();
//	EEPROMMassErase();

	//
	//	Initialize RTC
	//
//	initRTC();

	initKeys();
	initOutputGPIOs();
	//
	//	De-bounce init
	//
	//testDebounceSetup();



	uint8_t iTmp;
	/*uint8_t tempwriteeep[108];
	for(iTmp = 0; iTmp < 27; iTmp++)
	{
		int randno = urand();
		tempwriteeep[iTmp*4] = (randno & 0x000000FF);
		tempwriteeep[iTmp*4 + 1] = (randno & 0x0000FF00) >> 8;
		tempwriteeep[iTmp*4 + 2] = (randno & 0x00FF0000) >> 16;
		tempwriteeep[iTmp*4 + 3] = (randno & 0xFF000000) >> 24;
	}
	EEPROMProgramByte(tempwriteeep, 0, 108);
	EEPROMProgramByte(tempwriteeep, 108, 108);
	EEPROMProgramByte(tempwriteeep, 216, 108);
	EEPROMProgramByte(tempwriteeep, 324, 108);
	EEPROMProgramByte(tempwriteeep, 432, 64);*/

	for(iTmp = 0; iTmp < _NO_OF_PARAMS; iTmp++)
	{
		initParameterDB((PARAM_CTRL)iTmp);
	}

	initdbhandler();

#ifndef DISABLE_WATCHDOG
    initWatchdog();
#endif
}

/********************************************************************************/
