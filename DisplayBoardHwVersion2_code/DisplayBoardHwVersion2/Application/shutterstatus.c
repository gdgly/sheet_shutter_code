/*********************************************************************************
 * FileName: shutterstatus.c
 * Description: This file contains functions for handling shutter status when
 * active functional block is Shutter Status Functional Block.
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
 *  	0.1D	30/07/2014      	iGATE Offshore team       Initial Creation
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
#include "Middleware/paramdatabase.h"
#include "intertaskcommunication.h"
#include "communicationmodule.h"
#include "Middleware/sdcard.h"
/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
uint32_t gui32ShutterStatusParamA200 = 0;
uint32_t gui32ParamA200StoredValue = 0;
bool gRefreshStatusScreen = true;
extern uint8_t open_disable_enable_cyw;//0-enable 1-disable
extern uint8_t close_disable_enable_cyw;//0-enable 1-disable
/*const unsigned char gStateSymbols[4][5] =
{
	{'B', 'I', 'D', 'U', 'T'} ,
	{'M', '1', 'E', 'L', 'M'} ,
	{'O', 'U', '1', 'D', 'A'} ,
	{'S', 'S', '2', 'I'}
};*/		//	Removed on 04 Dec 2014 for new requirement from client and added following.

const unsigned char gStateSymbols[4][5] =
{
	{"BIDUC"} ,
	{"M1ELS"} ,
	{"OUFD1"} ,
	{"SSXO"}
};

/******************************************************************************
 * FunctionName: shutterStatusPaint
 *
 * Function Description: Paints current shutter status
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0 on success
 *
 ********************************************************************************/
uint8_t shutterStatusPaint(void)
{
	uint8_t i = 0, j = 0, y = 0, x = 4;
	uint32_t currentBit = 0x00040000;
	unsigned char buff[3] = {0};

	//
	// Clear screen if active functional block is switched to shutter status functional block.
	// Paint status items.
	//
	if(true == gRefreshStatusScreen)
	{
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, true, true);
		//
		// Paint status items
		//
		for(i = 0; i < 5; i++)
		{
			for(j = 0; j < 4; j++)
			{
				//
				// Modify y coordinate to paint row.
				//
				y = 16 * j;

				usnprintf((char *)buff, sizeof(buff), "%c:", gStateSymbols[j][i]);
		//		memset(buff_cyw,0x20,sizeof(buff_cyw));
		//		usnprintf_nU_cyw((char*)buff_cyw,2,(char*)buff);
				//displayText(buff, x, y, false, false, false, false);
				displayText(buff, x, y, false, false, false, false,false ,true);
			}

			//
			// Modify x coordinate to paint next column
			//
			x += 24;
		}
	}

	//
	// Change x coordinate for painting status items values
	//
	x = 14;

	//
	// Paint status item values
	//
	for(i = 0; i < 5; i++)
	{
		for(j = 0; j < 4; j++)
		{
			y = 16 * j;

			//
			// Clear current state if there is a state change occurred
			//
			if( (gui32ShutterStatusParamA200 & currentBit) != (gui32ParamA200StoredValue & currentBit) ||
				(true == gRefreshStatusScreen)
			  )
			{
				if( gui32ShutterStatusParamA200 & currentBit )
					//displayText(" O", x, y, true, true, false, false);
					displayText("o", x, y, true, true, false, false, false, true);
				else
					//displayText(" -", x, y, false, true, false, false);
					displayText("-", x, y, false, true, false, false, false, true);
			}

			if(1 == currentBit)
				break;

			currentBit /= 2;
		}

		//
		// Modify x coordinate to paint next column.
		//
		x += 24;
	}

	if(true == gRefreshStatusScreen)
		gRefreshStatusScreen = false;

	return 0;
}

/******************************************************************************
 * FunctionName: shutterStatusRunTime
 *
 * Function Description:
 * This is a default function.
 *
 * Function Parameters: None
 *
 * Function Returns: 0 on success.
 *
 ********************************************************************************/
uint8_t shutterStatusRunTime(void)
{
	static uint32_t lsTickCount1Seconds = 0;
	static uint8_t lsDelay1SecStart = 0;
       static uint8_t lsui8OpenState = 0;
	static uint8_t lsui8CloseState = 0;
	static uint8_t lsui8StopState = 0;
      static uint8_t lsui8StopState_count=0;   //201806_Bug_No.22
	//
	// Start 5 seconds delay
	//
	if(lsDelay1SecStart == 0)
	{
		if(gstUMtoCMdatabase.commandRequestStatus == eINACTIVE)
		{
			//
			// Initiate GET_PARAMETER command for parameter number A200
			// i.e. Shutter Status
			//
			gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 1;
			gstUMtoCMdatabase.dataToControlBoard.parameterNumber = PARAM_SHUTTER_STATUS;
			gstUMtoCMdatabase.destination = eDestControlBoard;

			//
			// Set command request status as active
			//
			gstUMtoCMdatabase.commandRequestStatus = eACTIVE;

			//
			// Capture time
			//
			lsTickCount1Seconds = g_ui32TickCount;

			//
			// Set 500 milliseconds delay start flag
			//
			lsDelay1SecStart = 1;
		}
	}

	//
	// Check whether delay is achieved.
	//
	if( (get_timego( lsTickCount1Seconds) > 50) &&
		(1 == lsDelay1SecStart)
	  )
	{
		//
		// Reset 1 seconds delay start flag
		//
		lsDelay1SecStart = 0;
	}

	//
	// Check Get parameter response status. Save A200 value
	//
	if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
	{
		if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS)
		{
			//
			// Save received value of A200
			//
			gui32ParamA200StoredValue = gui32ShutterStatusParamA200;
			gui32ShutterStatusParamA200 = gstUMtoCMdatabase.getParameterValue;

			//
			// Paint current shutter status
			//
			shutterStatusPaint();

			//
			// Reset request and response status
			//
			gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
			gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
		}

		else if( (gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT) ||
				 (gstUMtoCMdatabase.commandResponseStatus == eFAIL)
			   )
		{
#if 0
			//
			// Set communication fault flag
			//
			if(gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT)
			{
				gstDisplayBoardFault.bits.displayCommunication = 1;
				gstDisplayCommunicationFault.bits.commFailControl = 1;
			}

			if(gstUMtoCMdatabase.commandResponseStatus == eFAIL)
			{
				gstDisplayBoardFault.bits.displayCommunication = 1;
				gstDisplayCommunicationFault.bits.crcError = 1;
			}
#endif
			//
			// Reset request and response status
			//
			gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
			gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
		}
	}
if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
    {


         if((gKeysStatus.bits.Key_Open_pressed) && (0 == lsui8CloseState) && (0 == lsui8StopState)&&(open_disable_enable_cyw==0))
         {
        	 gKeysStatus.bits.Key_Open_pressed = 0;  //2016  opening 70ms send open
        	 lsui8OpenState = 1;
        	 gstUMtoCMoperational.commandToControlBoard.bits.openPressed = 1;
        	 gstUMtoCMoperational.commandRequestStatus = eACTIVE;
         }
         if(gKeysStatus.bits.Key_Open_released)
         {
         			//
         			// Initiate open command
         			//
         			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
         			{
         				gKeysStatus.bits.Key_Open_released = 0;
         				lsui8OpenState = 0;
         				gstUMtoCMoperational.commandToControlBoard.bits.openReleased = 1;
         				gstUMtoCMoperational.commandRequestStatus = eACTIVE;
         			}
         }
         if((gKeysStatus.bits.Key_Close_pressed) && (0 == lsui8OpenState) && (0 == lsui8StopState)&&(close_disable_enable_cyw==0))
         		{
         			//
         			// Initiate close command
         			//
         			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
         			{
         				gKeysStatus.bits.Key_Close_pressed = 0;
         				lsui8CloseState = 1;
         				gstUMtoCMoperational.commandToControlBoard.bits.closePressed = 1;
         				gstUMtoCMoperational.commandRequestStatus = eACTIVE;
         			}
         		}

         		if(gKeysStatus.bits.Key_Close_released)
         		{
         			//
         			// Initiate close command
         			//
         			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
         			{
         				gKeysStatus.bits.Key_Close_released = 0;
         				lsui8CloseState = 0;
         				gstUMtoCMoperational.commandToControlBoard.bits.closeReleased = 1;
         				gstUMtoCMoperational.commandRequestStatus = eACTIVE;
         			}
         		}

         		//
         		// Check whether stop key is pressed
         		//
         		if((gKeysStatus.bits.Key_Stop_pressed) && (0 == lsui8OpenState) && (0 == lsui8CloseState))
         		{
         			//
         			// Initiate stop command
         			//
         			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
         			{
         				lsui8StopState_count++;     //201806_Bug_No.22    //ÈÃSTOP·¢2´Î
         				if(lsui8StopState_count>=2)  //201806_Bug_No.22
         				    gKeysStatus.bits.Key_Stop_pressed = 0;
         				lsui8StopState = 1;
         				gstUMtoCMoperational.commandToControlBoard.bits.stopPressed = 1;
         				gstUMtoCMoperational.commandRequestStatus = eACTIVE;
         			}
         		}

         		if(gKeysStatus.bits.Key_Stop_released)
         		{
         			//
         			// Initiate stop command
         			//
         			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
         			{
         				gKeysStatus.bits.Key_Stop_released = 0;
         				lsui8StopState_count=0;    //201806_Bug_No.22
         				lsui8StopState = 0;
         				gstUMtoCMoperational.commandToControlBoard.bits.stopReleased = 1;
         				gstUMtoCMoperational.commandRequestStatus = eACTIVE;
         			}
         		}
    }


   // gui8SettingsModeStatus = DEACTIVATED;
   // gui8SettingsScreen     = DEACTIVATED;
	if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
		{
			if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
			{
				if(gstUMtoCMoperational.commandToControlBoard.bits.openPressed == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.openPressed = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
							else if(gstUMtoCMoperational.commandToControlBoard.bits.openReleased == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.openReleased = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
							else if(gstUMtoCMoperational.commandToControlBoard.bits.closePressed == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.closePressed = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
							else if(gstUMtoCMoperational.commandToControlBoard.bits.closeReleased == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.closeReleased = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
							else if(gstUMtoCMoperational.commandToControlBoard.bits.stopPressed == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.stopPressed = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
							else if(gstUMtoCMoperational.commandToControlBoard.bits.stopReleased == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.stopReleased = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
			}

			else if( (gstUMtoCMoperational.commandResponseStatus == eTIME_OUT) ||
					(gstUMtoCMoperational.commandResponseStatus == eFAIL)
			)
			{
				if(gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus = 0;
					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
				}
				else if(gstUMtoCMoperational.commandToControlBoard.bits.openPressed == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.openPressed = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
							else if(gstUMtoCMoperational.commandToControlBoard.bits.openReleased == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.openReleased = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
							else if(gstUMtoCMoperational.commandToControlBoard.bits.closePressed == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.closePressed = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
							else if(gstUMtoCMoperational.commandToControlBoard.bits.closeReleased == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.closeReleased = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
							else if(gstUMtoCMoperational.commandToControlBoard.bits.stopPressed == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.stopPressed = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
							else if(gstUMtoCMoperational.commandToControlBoard.bits.stopReleased == 1)
							{
								gstUMtoCMoperational.commandToControlBoard.bits.stopReleased = 0;
								gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
								gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
							}
			}
		}
	updateFaultLEDStatus();

	return 0;
}

/******************************************************************************
 * FunctionName: shutterStatusMode
 *
 * Function Description: Move back to status menus
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0 on success
 *
 ********************************************************************************/
uint8_t shutterStatusMode(void)
{
	//
	// Handle Mode key press
	//
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
    	gKeysStatus.bits.Key_Mode_pressed = 0;

		psActiveFunctionalBlock = &gsMenuFunctionalBlock;
		psActiveFunctionalBlock->pfnPaintFirstScreen();

		gRefreshStatusScreen = true;
    }

	return 0;
}

/******************************************************************************
 * FunctionName: shutterStatusEnter
 *
 * Function Description:
 * This is a default function.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0 on success.
 *
 ********************************************************************************/
uint8_t shutterStatusEnter(void)
{
	//
	// Handle Enter key press
	//
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: shutterStatusUp
 *
 * Function Description:
 * This is a default function.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0 on success.
 *
 ********************************************************************************/
uint8_t shutterStatusUp(void)
{
	//
	// Handle Up key press
	//
	if(gKeysStatus.bits.Key_Up_pressed)
	{
		gKeysStatus.bits.Key_Up_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: shutterStatusDown
 *
 * Function Description:
 * This is a default function.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0 on success.
 *
 ********************************************************************************/
uint8_t shutterStatusDown(void)
{
	//
	// Handle Down key press
	//
	if(gKeysStatus.bits.Key_Down_pressed)
	{
		gKeysStatus.bits.Key_Down_pressed = 0;
	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsShutterStatusFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	shutterStatusPaint,
	shutterStatusRunTime,
	shutterStatusUp,
	shutterStatusDown,
	shutterStatusMode,
	shutterStatusEnter
};
