/*********************************************************************************
 * FileName: changesettinghistory.c
 * Description: Code for displaying 'Display Change Settings Logs' screen
 * Version: 0.1D
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
 *  	0.2D	03/07/2014								Auto go back and other timers added.
 *  	0.1D	09/06/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/gpio.h>
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "grlib/grlib.h"
//#include "Middleware/cfal96x64x16.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "userinterface.h"
#include "intertaskcommunication.h"
#include "Middleware/paramdatabase.h"
#include "sdcardlogs.h"
#include "logger.h"
/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/

/****************************************************************************/

/******************************************************************************
 * FunctionName: changeSettingHistoryRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t changeSettingHistoryRunTime(void)
{
	//
	// This function is called periodically
	//

	updateFaultLEDStatus();

	return 0;
}
/******************************************************************************
 * FunctionName: changeSettingHistoryPaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t changeSettingHistoryPaint(void)
{
	//
	// This is first screen paint function
	//
	gstUMtoLM_read.commandRequestStatus = eACTIVE;
	gstUMtoLM_read.commandResponseStatus = eNO_STATUS;
	gstUMtoLM_read.commandToLMread.bits.readChangeSettingsHistory = 1;
	gstUMtoLM_read.historyOrAnomalyIndex = 1;
	readChangeSettingsHistory();
	showLogs(CHGSETHIST_START_IDX);
	return 0;
}

/******************************************************************************
 * FunctionName: changeSettingHistoryMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t changeSettingHistoryMode(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Mode key press
	//
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
    	gKeysStatus.bits.Key_Mode_pressed = 0;

    	gstUMtoLM_read.historyOrAnomalyIndex = 0;
		psActiveFunctionalBlock = &gsMenuFunctionalBlock;
		psActiveFunctionalBlock->pfnPaintFirstScreen();

    }

	return 0;
}

/******************************************************************************
 * FunctionName: changeSettingHistoryEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t changeSettingHistoryEnter(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Enter key press
	//
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;

		//
		// Write enter key functionality here
		//
	}

	return 0;
}

/******************************************************************************
 * FunctionName: changeSettingHistoryUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t changeSettingHistoryUp(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Up key press
	//
	if(gKeysStatus.bits.Key_Up_pressed)
	{
		gKeysStatus.bits.Key_Up_pressed = 0;

		gstUMtoLM_read.commandRequestStatus = eACTIVE;
		gstUMtoLM_read.commandResponseStatus = eNO_STATUS;
		gstUMtoLM_read.commandToLMread.bits.readChangeSettingsHistory = 1;
		if(gstUMtoLM_read.historyOrAnomalyIndex > 1)
			gstUMtoLM_read.historyOrAnomalyIndex--;
		else
			gstUMtoLM_read.historyOrAnomalyIndex = 10;
		readChangeSettingsHistory();
		showLogs(CHGSETHIST_START_IDX);
	}

	return 0;
}

/******************************************************************************
 * FunctionName: changeSettingHistoryDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t changeSettingHistoryDown(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Down key press
	//
	if(gKeysStatus.bits.Key_Down_pressed)
	{
		gKeysStatus.bits.Key_Down_pressed = 0;

		gstUMtoLM_read.commandRequestStatus = eACTIVE;
		gstUMtoLM_read.commandResponseStatus = eNO_STATUS;
		gstUMtoLM_read.commandToLMread.bits.readChangeSettingsHistory = 1;
		if(gstUMtoLM_read.historyOrAnomalyIndex < 10)
			gstUMtoLM_read.historyOrAnomalyIndex++;
		else
			gstUMtoLM_read.historyOrAnomalyIndex = 1;
		readChangeSettingsHistory();
		showLogs(CHGSETHIST_START_IDX);
	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsChangeSettingHistoryFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	changeSettingHistoryPaint,
	changeSettingHistoryRunTime,
	changeSettingHistoryUp,
	changeSettingHistoryDown,
	changeSettingHistoryMode,
	changeSettingHistoryEnter
};
