/*********************************************************************************
 * FileName: system.h
 * Description:
 * This source file contains the prototype definition of all the services of ....
 * Version: 0.1D
 *
 *
 **********************************************************************************/

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

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
 *  Macro definitions:
 ****************************************************************************/
//#define DISABLE_WATCHDOG
//#define Enable_WATCHDOG_CHECK     //add define   20170421   201703_No.39
/****************************************************************************/


/****************************************************************************
 *  Global variables:
 ****************************************************************************/


/****************************************************************************/

/****************************************************************************
 *  Code selection
 ****************************************************************************/

#define DEVELOPMENT_BOARD
#define PROTOTYPE_BOARD

//#define VER2_TEST_WITH_VER1_HARDWARE
//#define VERSION_1HARDWARE
#define VERSION_2HARDWARE

#define	WIRELESS_3PBS_AS_VERSION_3HARDWARE

//#define APPLICATION
//#define	TEST_UART
//#define	TEST_EEPROM
//#define	TEST_RTC
//#define	TEST_DEBOUNCE
#define TEST_CMDr

/******************************************************************************
 * systemInit
 *
 * Function Description:

 *
 * Function Parameters:
 *
 * Function Returns:
 *
 ********************************************************************************/

void
systemInit(void);

/********************************************************************************/

#endif /*__SYSTEM_H__*/
