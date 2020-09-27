/*********************************************************************************
 * FileName: homescreen.c
 * Description:
 * This source file contains the definitions of home screen operations. Functions
 * defined here are used when active functional block is Home screen functional
 * block. It also defines home screen functional block object.
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
 *  	0.1D	14/04/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "grlib/grlib.h"
//#include "Middleware/cfal96x64x16.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "Middleware/paramdatabase.h"
#include "userinterface.h"
#include "intertaskcommunication.h"
#include "ledhandler.h"
#include "Gesture_Sensor/ram_cyw.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
//#define SKIP_GET_OPERATION_COUNT
//#define ENABLE_ERROR_DISPLAY
//#define ENABLE_HOMESCREEN_REFRESH
#define ENABLE_INDEPENDENT_ANOMALIES_DISPLAY

#define OPERATION_CNT_PARAM_NUM		600
#define PORTF_POWERLEDPIN	(GPIO_PIN_4)
#define PORTE_AUTOMANUALLEDPIN (GPIO_PIN_0)

//	Updating operation count logic made configurable using following macros - YG - NOV 2015
#define GET_OPERATION_COUNT_UPPER_APERTURE_LIMIT
//#define GET_OPERATION_COUNT_LOWER_LIMIT
//**************************************************************************
// Home Screen States
//**************************************************************************
#define GET_OPERATION_COUNT					0
#define FATAL_ERROR_CHECK					1
#define DISPLAY_FATAL_ERRORS				2
#define DISPLAY_ANOMALIES					3
#define DELAY_3_SECONDS						4
#define CHECK_FOR_RUNTIME_CALIBRATION		5
#define WAITING_FOR_CALIBRATION_TO_OVER		6
#define PAINT_HOME_SCREEN					7
#define KEY_PRESS_HANDLE					8
#define STATE_UNDEFINED						9

//**************************************************************************
// Operation Count States
//**************************************************************************
#define CHECK_FOR_LOWER_LIMIT		0
#define SEND_COMMAND				1
#define	COMMAND_SENT				2

/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
extern volatile uint32_t g_ui32TickCount;
extern uint32_t gTickCount3Seconds;
uint32_t gTickCount1Seconds = 0;

extern struct errorDB gsActiveAnomalyList[20];
extern uint8_t setting_flag;//?????????o???
extern uint32_t gShutterType;
extern const unsigned char cucA537_SH_TYPE_States[][40] ;
extern unsigned char menu_gesture_flag_cyw;
extern uint8_t open_disable_enable_cyw;//0-enable 1-disable
extern uint8_t close_disable_enable_cyw;//0-enable 1-disable
extern bool bFatalErrorOccurred;
//**************************************************************************
// Current state of Home screen functional block
//**************************************************************************

#ifndef SKIP_GET_OPERATION_COUNT
uint8_t gHomeScreenState = GET_OPERATION_COUNT;
#endif

#ifdef SKIP_GET_OPERATION_COUNT
uint8_t	gHomeScreenState = FATAL_ERROR_CHECK;
#endif

//**************************************************************************
// Next state of home screen functional block
//**************************************************************************
uint8_t gNextHSState = 0xFF;

//	Update operation count when shutter reaches lower limit - YG - NOV 2015
#ifdef GET_OPERATION_COUNT_LOWER_LIMIT
uint8_t gOperationCountCommandState = SEND_COMMAND;
#endif

//	Update operation count when shutter reaches upper limit - YG - NOV 2015
#ifdef GET_OPERATION_COUNT_UPPER_APERTURE_LIMIT
uint8_t gucSentOperationCountCmd = 0;
#endif

//**************************************************************************
// Current anomaly index in active anomaly list
//**************************************************************************
uint8_t gAnomalyIndex = 0;

//**************************************************************************
// Delay Start Flags
//**************************************************************************
uint8_t gDelay3SecStart = 0;
uint8_t gDelay1SecStart = 0;

uint8_t AutoMan_His_cyw = 0;
uint8_t StopRun_His_cyw = 0;
uint8_t Guesture_His_cyw = 0;
uint32_t OPERATE_His_cyw = 0;
extern uint8_t  LCD_DISP_GUESTURE;
/****************************************************************************/

/******************************************************************************
 * FunctionName: homescreenLEDHandler
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns: void
 *
 ********************************************************************************/
extern uint32_t guesture_led_fre ;
void homescreenLEDHandler(void)
{
	
	if(gstLEDcontrolRegister.guestureLED)
	{
        if(get_timego(guesture_led_fre)>5)
		{
			guesture_led_fre = g_ui32TickCount;
			gstLEDcontrolRegister.guestureLED--;
			LCD_BACKLIGHT_TOGGLE();
			LED_AOTUMAU_TOGGLE();
			if(gstLEDcontrolRegister.guestureLED==0)
			{
              // LCD_BACKLIGHT_SETSTATUS();
                      Set_lcdlightON();
                     if(1 == gstControlBoardStatus.bits.autoManual)
                     {
                         LED_AOTUMAU_ON();
                       }
                      else
                      {
                        LED_AOTUMAU_OFF();
                        }
                     
			}
		} 

	}
	else if(1 == gstControlBoardStatus.bits.autoManual)
	{
		gstLEDcontrolRegister.autoManualLED = LED_ON;
	}
	else
	{
		gstLEDcontrolRegister.autoManualLED = LED_OFF;
	}

	//
	// Update fault LED status
	//
	if(
		( (1 == gstDriveBoardStatus.bits.driveFault) ||
		 (1 == gstDriveBoardStatus.bits.driveFaultUnrecoverable) ||
		 (1 == gstControlBoardStatus.bits.controlFault) ||
		 (1 == gstControlBoardStatus.bits.controlFaultUnrecoverable)||
		 (1 == gstDisplayBoardStatus.bits.displayFault) ||
		 (1 == gstDisplayBoardStatus.bits.displayFaultUnrecoverable)
		) //&& ( 0xFF != gstEMtoUM.faultLEDstatus )
	  )
	{
		gstLEDcontrolRegister.faultLED = gstEMtoUM.faultLEDstatus;
	}
	else if(
			(   (1 != gstDriveBoardStatus.bits.driveFault) &&
				(1 != gstDriveBoardStatus.bits.driveFaultUnrecoverable) &&
				(1 != gstControlBoardStatus.bits.controlFault) &&
				(1 != gstControlBoardStatus.bits.controlFaultUnrecoverable) &&
				(1 != gstDisplayBoardStatus.bits.displayFault) &&
				(1 != gstDisplayBoardStatus.bits.displayFaultUnrecoverable)
			) //|| ( 0xFF == gstEMtoUM.faultLEDstatus )
		  )
	{
		gstLEDcontrolRegister.faultLED = 0;
	}

	gstLEDcontrolRegister.powerLED = LED_ON;
}

/******************************************************************************
 * FunctionName: operationKeysHandler
 *
 * Function Description:
 * This function handle open, close and stop key press and
 * initiate respective commands.
 *
 * Function Parameters: None
 *
 * Function Returns: void
 *
 ********************************************************************************/
void operationKeysHandler(void)
{
	static uint8_t lsui8OpenState = 0;
	static uint8_t lsui8CloseState = 0;
	static uint8_t lsui8StopState = 0;
	static uint8_t lsui8StopState_count=0;   //201806_Bug_No.22

	//
	//	Added check to see whether system is in healthy state. If a fatal error has occurred then
	//	don't process operation keys - Jan 2016
	//
	if(
			(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
			(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)
	)
	{
		//
		// Check whether open key is pressed
		//
		if((gKeysStatus.bits.Key_Open_pressed) && (0 == lsui8CloseState) && (0 == lsui8StopState)&&(open_disable_enable_cyw==0))
		{
			//
			// Initiate open command
			//
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				gKeysStatus.bits.Key_Open_pressed = 0;  //2016  opening 70ms send open
				lsui8OpenState = 1;
				gstUMtoCMoperational.commandToControlBoard.bits.openPressed = 1;
				gstUMtoCMoperational.commandRequestStatus = eACTIVE;
			}
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

		//
		// Check whether close key is pressed
		//
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
				lsui8StopState_count++;     //201806_Bug_No.22    //ÈÃSTOP·¢2´ÎÃüÁî
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

		//
		// Check whether auto/manual select key is pressed
		//
		if(gKeysStatus.bits.Key_AutMan_pressed)
		{
			//
			// Initiate stop command
			//
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				gKeysStatus.bits.Key_AutMan_pressed = 0;
				gstUMtoCMoperational.commandToControlBoard.bits.autoManSelect = 1;
				gstUMtoCMoperational.commandRequestStatus = eACTIVE;
			}
		}
	} /*if((gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
	(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0))*/

	//
	// Check for command response and take respective action
	//
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
			else if(gstUMtoCMoperational.commandToControlBoard.bits.autoManSelect == 1)
			{
				gstUMtoCMoperational.commandToControlBoard.bits.autoManSelect = 0;
				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
			}

		}

		else if( (gstUMtoCMoperational.commandResponseStatus == eTIME_OUT) ||
				 (gstUMtoCMoperational.commandResponseStatus == eFAIL)
				)
		{
#if 0
			//
			// Set communication fault flag
			//
			if(gstUMtoCMoperational.commandResponseStatus == eTIME_OUT)
			{
				gstDisplayBoardFault.bits.displayCommunication = 1;
				gstDisplayCommunicationFault.bits.commFailControl = 1;
			}

			if(gstUMtoCMoperational.commandResponseStatus == eFAIL)
			{
				gstDisplayBoardFault.bits.displayCommunication = 1;
				gstDisplayCommunicationFault.bits.crcError = 1;
			}
#endif

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
			else if(gstUMtoCMoperational.commandToControlBoard.bits.autoManSelect == 1)
			{
				gstUMtoCMoperational.commandToControlBoard.bits.autoManSelect = 0;
				gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
				gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
			}
		}
	}

}


/******************************************************************************
 * FunctionName: homeScreenRunTime
 *
 * Function Description:
 * Handles home screen runtime activities which are -
 * 1. Fetching operation count and displaying on screen.
 * 2. Displaying recoverable and fatal errors (if any).
 * 3. Check for runtime calibration.
 * 4. Operational key press handling.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * returns 0 on success
 *
 ********************************************************************************/
uint8_t homeScreenRunTime(void)
{

#ifndef ENABLE_INDEPENDENT_ANOMALIES_DISPLAY
	static uint8_t lsAnomalyCounter = 0;
#endif
	unsigned char lbuff[21]={0};
	unsigned char lBuff[21]={0};
        setting_flag = 0;
	//	Moved this check at the beginning of the function to avoid sending any command in home-screen before processing response - DEC 2015
	//
	// Check whether shutter system is going under installation
	//
	if(gstDriveBoardStatus.bits.driveInstallation == 1)
	{
		gHomeScreenState = GET_OPERATION_COUNT;

		//
		// Set active functional block as installation functional block
		//
		psActiveFunctionalBlock = &gsInstallationFunctionalBlock;
		psActiveFunctionalBlock->pfnPaintFirstScreen();
	}
	switch(gHomeScreenState)
	{
		case GET_OPERATION_COUNT:
		{
			//	Update operation count when shutter reaches lower limit - YG - NOV 2015
			#ifdef GET_OPERATION_COUNT_LOWER_LIMIT
			//***************************************************************************
			//START Get operation count when shutter is at Lower Limit ******************
			//***************************************************************************

			//
			// Check whether shutter is a lower limit
			//
			if( (gstDriveBoardStatus.bits.shutterLowerLimit == 1) &&
			    (gOperationCountCommandState == CHECK_FOR_LOWER_LIMIT)
			  )
			{
				//
				// Allow GET_PARAMETER command for fetching operation count
				// by operation count command state change.
				//
				gOperationCountCommandState = SEND_COMMAND;
			}

			//
			// Check whether initiating GET_PARAMTER command for fetching operation
			// count is allowed.
			//
			if(gOperationCountCommandState == SEND_COMMAND)
			{
				//
				// Initiate GET_PARAMETER command for fetching operation count
				//
				if(gstUMtoCMdatabase.commandRequestStatus == eINACTIVE)
				{
					//
					// Select GET_PARAMETER command to initiate
					//
					gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 1;

					//
					// Supply input parameter number
					//
					gstUMtoCMdatabase.dataToControlBoard.parameterNumber = OPERATION_CNT_PARAM_NUM; // operation count parameter number
//					gstUMtoCMdatabase.dataToControlBoard.parameterNumber = A600_OPERATION_COUNT;

					//
					// Add destination ID
					//
					gstUMtoCMdatabase.destination = eDestDriveBoard;

					//
					// Set command request status active
					//
					gstUMtoCMdatabase.commandRequestStatus = eACTIVE;

					//
					// Modify operation count command state
					//
					gOperationCountCommandState = COMMAND_SENT;
				}
			}

			//
			// Reset operation count command state if shutter is not at lower limit
			//
			if( (gstDriveBoardStatus.bits.shutterLowerLimit == 0) &&
			    (gOperationCountCommandState == COMMAND_SENT) &&
			    (gstUMtoCMdatabase.commandRequestStatus == eINACTIVE)
			  )
			{
				gOperationCountCommandState = CHECK_FOR_LOWER_LIMIT;
			}

			//***************************************************************************
			//END Get operation count when shutter is at Lower Limit ********************
			//***************************************************************************
			#endif

			//	Update operation count when shutter reaches upper/aperture limit - YG - NOV 2015
			#ifdef GET_OPERATION_COUNT_UPPER_APERTURE_LIMIT
			//***************************************************************************
			//START Get operation count when shutter without checking shutter position **
			//***************************************************************************

			static uint8_t sucSentOperationCountCmdOnUpperLimit = 0;
			static uint8_t sucSentOperationCountCmdOnApertureLimit = 0;

			//
			// Check whether initiating GET_PARAMTER command for fetching operation
			// count is allowed.
			//
			if(
					(
					(gucSentOperationCountCmd == 0)||
					(sucSentOperationCountCmdOnUpperLimit == 0 && gstDriveBoardStatus.bits.shutterUpperLimit == 1)||
					(sucSentOperationCountCmdOnApertureLimit == 0 && gstDriveBoardStatus.bits.shutterApertureHeight == 1)
					) &&
					//	Added this check to disable getParameter - Operation count command while we are in installation - Dec 2015
					(gstDriveBoardStatus.bits.driveInstallation == 0)
			  )
			{
				//
				// Initiate GET_PARAMETER command for fetching operation count
				//
				if(gstUMtoCMdatabase.commandRequestStatus == eINACTIVE)
				{
					//
					// Select GET_PARAMETER command to initiate
					//
					gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 1;

					//
					// Supply input parameter number
					//
					gstUMtoCMdatabase.dataToControlBoard.parameterNumber = OPERATION_CNT_PARAM_NUM; // operation count parameter number

					//
					// Add destination ID
					//
					gstUMtoCMdatabase.destination = eDestDriveBoard;

					//
					// Set command request status active
					//
					gstUMtoCMdatabase.commandRequestStatus = eACTIVE;

					//
					// Set flag to indicate command sent
					//
					if (gucSentOperationCountCmd == 0)
					{
						gucSentOperationCountCmd = 1;
					}

					if (sucSentOperationCountCmdOnUpperLimit == 0 && gstDriveBoardStatus.bits.shutterUpperLimit == 1)
					{
						sucSentOperationCountCmdOnUpperLimit = 1;
					}

					if (sucSentOperationCountCmdOnApertureLimit == 0 && gstDriveBoardStatus.bits.shutterApertureHeight == 1)
					{
						sucSentOperationCountCmdOnApertureLimit = 1;
					}

				}
			}

			//
			// Reset command sent flag
			//
			if (sucSentOperationCountCmdOnUpperLimit == 1 && gstDriveBoardStatus.bits.shutterUpperLimit == 0)
			{
				sucSentOperationCountCmdOnUpperLimit = 0;
			}

			if (sucSentOperationCountCmdOnApertureLimit == 1 && gstDriveBoardStatus.bits.shutterApertureHeight == 0)
			{
				sucSentOperationCountCmdOnApertureLimit = 0;
			}

			//***************************************************************************
			//END Get operation count when shutter without checking shutter position
			//***************************************************************************
			#endif

			//
			// Check for GET_PARAMETER response
			//
			if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
			{
				//
				// Check for response success
				//
				if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS)
				{
					//
					// store received operation count value
					//
					ui32OperationCount = gstUMtoCMdatabase.getParameterValue;

					//
					// Reset request and response status
					//
					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
					gstUMtoCMdatabase.commandToControlBoard.val = 0;

					//
					// Change state
					//
#ifndef ENABLE_INDEPENDENT_ANOMALIES_DISPLAY
					gHomeScreenState = FATAL_ERROR_CHECK;
#endif

#ifdef ENABLE_INDEPENDENT_ANOMALIES_DISPLAY
					gHomeScreenState = CHECK_FOR_RUNTIME_CALIBRATION;
#endif

					//
					// Reset operation count command state
					//
					//gOperationCountCommandState = CHECK_FOR_LOWER_LIMIT;
				}
				else if( (gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT) ||
						 (gstUMtoCMdatabase.commandResponseStatus == eFAIL)
						)
				{
#if 0
					//
					// Set communication fault flags
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
					gstUMtoCMdatabase.commandToControlBoard.val = 0;

					//
					// Reset operation count command state
					//
					//gOperationCountCommandState = CHECK_FOR_LOWER_LIMIT;

					//
					// Clear screen
					//
//					GrRectFIllBolymin(0, 126, 0, 63, true, true);

					//
					// Paint communication failure message
					//
//					displayText("COMMUNICATION", 2, 0, false, false, false, false);
//					displayText("FAILED", 2, 16, false, false, false, false);

					//
					// Change state
					//
					gHomeScreenState = DELAY_3_SECONDS;

#ifndef ENABLE_INDEPENDENT_ANOMALIES_DISPLAY
					gNextHSState = FATAL_ERROR_CHECK;
#endif

#ifdef ENABLE_INDEPENDENT_ANOMALIES_DISPLAY
					gHomeScreenState = CHECK_FOR_RUNTIME_CALIBRATION;
#endif
				}
			}
			else
			{

#ifndef ENABLE_INDEPENDENT_ANOMALIES_DISPLAY
				//
				// Change state
				//
				gHomeScreenState = FATAL_ERROR_CHECK;
#endif

#ifdef ENABLE_INDEPENDENT_ANOMALIES_DISPLAY
				gHomeScreenState = CHECK_FOR_RUNTIME_CALIBRATION;
#endif
			}

			break;
		}


#ifndef ENABLE_INDEPENDENT_ANOMALIES_DISPLAY
		case FATAL_ERROR_CHECK:
		{
			if(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 1)
			{
				//
				// Clear screen
				//
				GrRectFIllBolymin(0, 126, 0, 63, true, true);

				//
				// Paint screen title
				//
				displayText("FATAL ERRORS!", 2, 0, true, true, false, false);
				GrLineDrawHorizontalBolymin(0, 126, 15, false);	// line draw function

				//
				// Paint fatal error message
				//
				displayText("SHUTTER SYSTEM STOP", 2, 16, false, false, false, false);
				displayText("RESTART THE SYSTEM", 2, 32, false, false, false, false);

				//
				// Change state
				//
				gHomeScreenState = DISPLAY_FATAL_ERRORS;
			}
			else
			{
				//
				// Change state
				//
				gHomeScreenState = DISPLAY_ANOMALIES;
			}

			break;
		}

		case DISPLAY_FATAL_ERRORS:
		{
			if(gAnomalyIndex < 20)
			{
				if( (gsActiveAnomalyList[gAnomalyIndex].errorCode != 0) &&
				    (  (gstDriveBoardStatus.bits.driveFaultUnrecoverable == 1) ||
				       (gstControlBoardStatus.bits.controlFaultUnrecoverable == 1) ||
				       (gstDisplayBoardStatus.bits.displayFaultUnrecoverable == 1)
				    )
				  )
				{
					//
					// Start 3 seconds delay
					//
					if(gDelay3SecStart == 0)
					{
						//
						// Clear fourth line
						//
						GrRectFIllBolymin(0, 126, 48, 63, true, true);

						//
						// Display anomaly log on fourth line
						//
						usnprintf((char*)lbuff, sizeof(lbuff), "E%03d:%s", gsActiveAnomalyList[gAnomalyIndex].errorCode,
								gsActiveAnomalyList[gAnomalyIndex].errordescription);

#ifndef ENABLE_ERROR_DISPLAY
						displayText(lbuff, 2, 48, false, false, false, false);
#endif

						//
						// Capture time for delay generation
						//
						gTickCount3Seconds = g_ui32TickCount;

						//
						// Delay start
						//
						gDelay3SecStart = 1;
					}

					//
					// Check whether delay is achieved
					//
					//if( (g_ui32TickCount == 300 + gTickCount3Seconds) &&
					if( ( (g_ui32TickCount - gTickCount3Seconds) > 300 ) &&
						(gDelay3SecStart == 1)
					  )
					{
						//
						// Reset 3 seconds delay start flag
						//
						gDelay3SecStart = 0;
					}

				}

				//
				// Increment anomaly index
				//
				if(gDelay3SecStart == 0)
					gAnomalyIndex++;

			}
			else
			{
				gAnomalyIndex = 0;
			}

			break;
		}

		case DISPLAY_ANOMALIES:
		{
			if(gDelay3SecStart == 0)
			{
				//
				// Clear fourth line
				//
				GrRectFIllBolymin(0, 126, 48, 63, true, true);

				//
				// loop to display anomalies on fourth line
				//
				while(gDelay3SecStart == 0)
				{
					lsAnomalyCounter++;

					if(gsActiveAnomalyList[gAnomalyIndex].errorCode != 0)
					{
						//
						// Display anomaly log on fourth line
						//
						usnprintf((char*)lbuff, sizeof(lbuff), "E%03d:%s", gsActiveAnomalyList[gAnomalyIndex].errorCode,
								gsActiveAnomalyList[gAnomalyIndex].errordescription);

#ifndef ENABLE_ERROR_DISPLAY
						displayText(lbuff, 2, 48, false, false, false, false);
#endif

						//
						// Increment anomaly index
						//
						gAnomalyIndex++;

						//
						// Reset anomaly counter
						//
						lsAnomalyCounter = 0;

						//
						// Capture time for delay generation
						//
						gTickCount3Seconds = g_ui32TickCount;

						//
						// Set 3 sec delay start flag
						//
						gDelay3SecStart = 1;
					}
					else
					{
						//
						// Increment anomaly index
						//
						gAnomalyIndex++;

						if(gAnomalyIndex >= 20)
						{
							gAnomalyIndex = 0;
						}

						//
						// Check whether to reset anomaly counter and start 3 seconds delay flag
						//
						if(lsAnomalyCounter >= 10)
						{
							gDelay3SecStart = 1;
							lsAnomalyCounter = 0;
						}
					}
				}
			}
			else
			{
				//if( (g_ui32TickCount == 300 + gTickCount3Seconds) &&
				if( ( get_timego( gTickCount3Seconds) > 300 ) &&
					(gDelay3SecStart == 1)
				  )
				{
					gDelay3SecStart = 0;
				}

				gHomeScreenState = CHECK_FOR_RUNTIME_CALIBRATION;
			}

			break;
		}
#endif


		case DELAY_3_SECONDS:
		{
			//
			// Check for 3 seconds delay achieved. If yes, then change state.
			//
			//if(g_ui32TickCount == 300 + gTickCount3Seconds)
			if( get_timego(gTickCount3Seconds) > 300 )
			{
				gTickCount3Seconds = 0;
				gHomeScreenState = gNextHSState;
			}

			break;
		}

		case CHECK_FOR_RUNTIME_CALIBRATION:
		{
			if(gstDriveBoardStatus.bits.driveRunTimeCalibration == 1)
			{
				//
				// Clear screen
				//
				//GrRectFIllBolymin(0, 126, 0, 63, true, true);
				GrRectFIllBolymin(0, 127, 0, 63, true, true);

				//
				// Paint title
				//
				//displayText(" B X   S H U T T E R S", 30, 0, false, false, false, false);
				//displayText("BX SHUTTERS", 15, 0, false, false, false, false,true,false);
				usnprintf((unsigned char*)lBuff, sizeof((unsigned char*)lBuff), "   %s", cucA537_SH_TYPE_States[gShutterType]);//OK
                displayText((unsigned char*)lBuff, 10, 0, false, false, false, false, false, false);
				GrLineDrawHorizontalBolymin(0, 126, 14, false);	// line draw function

				//
				// Display self calibration message
				//
				
                                if(gu8_language == Japanese_IDX)
                                {
                                displayText("ƒWƒhƒEƒzƒZƒCƒVƒeƒCƒ}ƒX", 2, 16, false, false, false, false,false,false);
                                }
                                else
                                {
                                 displayText("SELF CALIBRATION", 2, 16, false, false, false, false,false,true);
				displayText("IN PROGRESS", 2, 32, false, false, false, false, false, true);
                                 }
				//
				// Capture time for delay generation
				//
				gTickCount3Seconds = g_ui32TickCount;

				//
				// Change state
				//
				gHomeScreenState = DELAY_3_SECONDS;
				gNextHSState = WAITING_FOR_CALIBRATION_TO_OVER;
			}
			else
			{
				//
				// Change State
				//
				gHomeScreenState = PAINT_HOME_SCREEN;

				
			}

			break;
		}

		case WAITING_FOR_CALIBRATION_TO_OVER:
		{
			if(gstDriveBoardStatus.bits.driveRunTimeCalibration == 0)
			{
				//
				// Change state
				//
				gHomeScreenState = PAINT_HOME_SCREEN;

				//
				// Reset 3 seconds and 5 seconds delay start flags
				//
				gDelay3SecStart = 0;
				gDelay1SecStart = 0;

				//
				// Clear first 3 lines on screen
				//
				//GrRectFIllBolymin(0, 126, 0, 47, true, true);
				GrRectFIllBolymin(0, 127, 0, 47, true, true);

				//
				// Draw first line of home screen
				//
				//displayText(" B X   S H U T T E R S", 30, 0, false, false, false, false);
				//displayText("BX SHUTTERS", 15, 0, false, false, false, false,true,false);
				usnprintf(lBuff, sizeof(lBuff), "   %s", cucA537_SH_TYPE_States[gShutterType]);//OK
				                displayText((unsigned char*)lBuff, 10, 0, false, false, false, false, false, false);
				GrLineDrawHorizontalBolymin(0, 126, 14, false);

				//
				// Display Operation count in second line
				//
				//displayText("OPERATION COUNT", 2, 16, false, false, false, false);
                                if(gu8_language == Japanese_IDX)
                                {
				displayText("ƒhƒEƒTƒJƒEƒ“ƒg", 2, 16, false, false, false, false, false, false);
                                }
                                else
                                {
                                 displayText("OPERATION", 2, 16, false, false, false, false,false,true);
                                 }
                                psActiveFunctionalBlock->pfnPaintFirstScreen();
                                
			}

			break;
		}

		case PAINT_HOME_SCREEN:
		{
			

			if(bFatalErrorOccurred == false) 
			{
			
			//
			// Start 1 seconds delay
			//
			if(gDelay1SecStart == 0)
			{
				//
				// Capture time for delay generation
				//
				gTickCount1Seconds = g_ui32TickCount;

				//
				// Delay start
				//
				gDelay1SecStart = 1;

#ifdef ENABLE_HOMESCREEN_REFRESH
				//
				// Clear first 3 lines on screen
				//
				GrRectFIllBolymin(0, 126, 0, 47, true, true);

				//
				// Draw first line of home screen
				//
				//displayText(" B X   S H U T T E R S", 30, 0, false, false, false, false);
				displayText(" B X   S H U T T E R S", 10, 0, false, false, false, false);
				GrLineDrawHorizontalBolymin(0, 126, 14, false);

				//
				// Display Operation count in second line
				//
				//
                                if(gu8_language == Japanese_IDX)
                                {
				displayText("ƒhƒEƒTƒJƒEƒ“ƒg", 2, 16, false, false, false, false, false, false);
                                }
                                else
                                {
                                displayText("OPERATION", 2, 16, false, false, false, false,false,true);
                                 }
#endif

				//
				// Display current value of operation count
				//
				
				//
				// Display Shutter Mode in third line
				//

                               
                                if((AutoMan_His_cyw != gstControlBoardStatus.bits.autoManual)||(StopRun_His_cyw !=gstControlBoardStatus.bits.runStop)||(OPERATE_His_cyw != ui32OperationCount))
                                {
                                AutoMan_His_cyw = gstControlBoardStatus.bits.autoManual;
                                StopRun_His_cyw =gstControlBoardStatus.bits.runStop;
                                 OPERATE_His_cyw = ui32OperationCount;

                            //    Guesture_His_cyw  = LCD_DISP_GUESTURE;
				if(1 == gstControlBoardStatus.bits.autoManual)
				{
					//displayText("AUTO  ", 48, 32, true, true, true, false);
                                          if(gu8_language == Japanese_IDX)
                                        {
                                        displayText("              ", 1, 32, false,false, false, false,false,false);
					displayText("ƒI[ƒg ", 37, 32, true, true, true, false,false,false);
                                        }
                                        else
                                        {
					displayText("AUTO  ", 38, 32, true, true, true, false,false,true);
					}
					//ROM_GPIOPinWrite(GPIO_PORTE_BASE, PORTE_AUTOMANUALLEDPIN, PORTE_AUTOMANUALLEDPIN); //ON
					gstLEDcontrolRegister.autoManualLED = LED_ON;
				}
				else
				{
					
                                         if(gu8_language == Japanese_IDX)
                                         {
					 displayText("              ", 1, 32, false,false, false, false,false,false);
                                       displayText("ƒ}ƒjƒ…ƒAƒ‹", 28, 32, true,  false, true, false,false,false);
                                          }
                                          else
                                          {
                                           displayText("MANUAL", 38, 32, true, true, true, false,false,true);
                                           }
					//ROM_GPIOPinWrite(GPIO_PORTE_BASE, PORTE_AUTOMANUALLEDPIN, 0); //OFF
					gstLEDcontrolRegister.autoManualLED = LED_OFF;
				}
				if(gu8_language == Japanese_IDX)
				{
                                if(1 == gstControlBoardStatus.bits.runStop)
				{
					//displayText(" R U N  ", 94, 32, true, true, true, false);
					displayText("RUN ", 83, 32, true, true, true, false,false,false);
				}
				else
				{
					//displayText(" S T O P", 94, 32, true, true, true, false);
					displayText("STOP", 83, 32, true, true, true, false,false,false);
				}
				}
				else
				{
                                 if(1 == gstControlBoardStatus.bits.runStop)
				{
					//displayText(" R U N  ", 94, 32, true, true, true, false);
					displayText(" RUN  ", 83, 32, true, true, true, false,false,true);
				}
				else
				{
					//displayText(" S T O P", 94, 32, true, true, true, false);
					displayText(" STOP ", 83, 32, true, true, true, false,false,true);
				}
				}

                               usnprintf((char*)lbuff, sizeof(lbuff), "%07d", ui32OperationCount);//OPERATIONS COUNT
				//memset(lbuff_cyw,0x20,sizeof(lbuff_cyw));
				//usnprintf_D07_cyw((char*)lbuff_cyw,sizeof(lbuff_cyw),(char*)lbuff);
				//displayText(lbuff_cyw, 2, 32, false, false, false, false);
				displayText(lbuff, 68, 16, false, false, false, false,false,true);

                                  
                               
                             }

				if((gstDriveBoardStatus.bits.drivePowerOnCalibration != 1)&&(gstDriveBoardStatus.bits.driveRunTimeCalibration!=1)&&
                                         (gstDriveBoardStatus.bits.driveInstallation!=1)&&(gstDriveBoardStatus.bits.driveFaultUnrecoverable!=1))
				{
                               if(((menu_gesture_flag_cyw == 0)||((menu_gesture_flag_cyw == 2)&&(gstControlBoardStatus.bits.autoManual==1)))&&(menu_gesture_flag_A007==0))  ////201806_Bug_No.10
                              ////201806_Bug_No.10
                               {
                                       if(LCD_DISP_GUESTURE == 1)
                                       {
                                          
                                         DISP_GUESTER_13_16(2, 14, 32, 45);
                                          //DISP_GUESTER_9_16(4, 12, 32, 45);
                                        LCD_DISP_GUESTURE   =0;
                                       }
                                       else
                                          //DISP_GUESTER_KONG(4, 12, 32, 45);
                                         DISP_GUESTER_FAN(2, 14, 32, 45);
                                }
                               else
                               {
                                     //  GrRectFIllBolymin(4, 12, 32, 45, true, true);
                                      //  GrRectFIllBolymin(2, 14, 32, 45, true, true);

					
					
                                       DISP_GUESTER_FAN_DIS(2, 14, 32, 45);
					
                                } 
				}
				
			}

			//
			// Refresh home screen after 1 seconds delay achieved
			//
			//if( (g_ui32TickCount == 500 + gTickCount5Seconds) &&
			if( ( get_timego( gTickCount1Seconds) > 100 ) &&
				(gDelay1SecStart == 1)
			  )
			{
				//
				// Reset 5 seconds delay start flag
				//
				gDelay1SecStart = 0;
			}

			//
			// Change State
			//
			gHomeScreenState = KEY_PRESS_HANDLE;

			break;
			}
		}
		case KEY_PRESS_HANDLE:
		{
			//
			// Handle operation key press events
			//
			operationKeysHandler();

			//
			// Reset home screen state
			//
			gHomeScreenState = GET_OPERATION_COUNT;

#ifndef ENABLE_INDEPENDENT_ANOMALIES_DISPLAY
			gHomeScreenState = FATAL_ERROR_CHECK;
#endif

			break;
		}

		case STATE_UNDEFINED:
		{

			break;
		}

		default:
		{

			break;
		}

	}

	//
	// Handle LEDs states at home screen.
	//
	homescreenLEDHandler();

#ifdef ENABLE_INDEPENDENT_ANOMALIES_DISPLAY
	//
	// Display errors on 4th line, if any
	//
	displayAnomalies();

        RecoveredAnomalies();

	//
	// Check whether any fatal error has occurred.
	//
	if(	(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 1) ||
	    (gstControlBoardStatus.bits.controlFaultUnrecoverable == 1) ||
	    (gstDisplayBoardStatus.bits.displayFaultUnrecoverable == 1)
	  )
	{
		//
		// Change state
		//
		gHomeScreenState = STATE_UNDEFINED;
	}
#endif

	return 0;
}

/******************************************************************************
 * FunctionName: homeScreenPaint
 *
 * Function Description:
 * Paints the Home screen on display area.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 for success
 *
 ********************************************************************************/
uint8_t homeScreenPaint(void)
{
	unsigned char lbuff[20]={0};
	unsigned char lBuff[20]={0};
	//
	// Clear first 3 lines on screen
	//
	//GrRectFIllBolymin(0, 126, 0, 47, true, true);
	GrRectFIllBolymin(0, 127, 0, 47, true, true);

	//
	// Draw first line of home screen
	//
	//displayText(" B X   S H U T T E R S", 30, 0, false, false, false, false);
//	displayText("BX SHUTTERS", 15, 0, false, false, false, false,true,false);
	usnprintf(lBuff, sizeof(lBuff), "   %s", cucA537_SH_TYPE_States[gShutterType]);//OK
	                displayText((unsigned char*)lBuff, 10, 0, false, false, false, false, false, false);
	GrLineDrawHorizontalBolymin(0, 126, 14, false);

	//
	// Display Operation count in second line
	//
	//displayText("OPERATION COUNT", 2, 16, false, false, false, false);
        
         if(gu8_language == Japanese_IDX)
         {
	displayText("ƒhƒEƒTƒJƒEƒ“ƒg", 2, 16, false, false, false, false, false, false);
         }
         else
         {
         displayText("OPERATION", 2, 16, false, false, false, false,false,true);
          }

	usnprintf((char*)lbuff, sizeof(lbuff), "%07d", ui32OperationCount);
	//memset(lbuff_cyw,0x20,sizeof(lbuff_cyw));
	//	usnprintf_D07_cyw((char*)lbuff_cyw,sizeof(lbuff_cyw),(char*)lbuff);
						//displayText(lbuff_cyw, 2, 32, false, false, false, false);
	//displayText(lbuff, 2, 32, false, false, false, false);
		displayText(lbuff, 68, 16, false, false, false, false,false,true);

	//
	// Display Shutter Mode in third line
	//
/*	switch(gstControlBoardStatus.bits.autoManual)
	{
		case SHUTTER_IN_AUTO_MODE:
				displayText("AUTO  ", 48, 32, true, true, true, false);
				break;

		case SHUTTER_IN_MANUAL_MODE:
				displayText("MANUAL", 48, 32, true, true, true, false);
				break;

		default:
				break;
	}

	//
	// Display Shutter State in third line
	//
	switch(gstControlBoardStatus.bits.runStop)
	{
		case SHUTTER_RUNNING:
				displayText("RUN ", 94, 32, true, true, true, false);
				break;

		case SHUTTER_STOPPED:
				displayText("STOP", 94, 32, true, true, true, false);
				break;

		default:
				break;
	}*/

	if(1 == gstControlBoardStatus.bits.autoManual)
	{
		
                  if(gu8_language == Japanese_IDX)
                 {
		 displayText("              ", 1, 32, false,false, false, false,false,false);
                 displayText("ƒI[ƒg ", 37, 32, true, true, true, false,false,false);
		}
		else
		{
		displayText("AUTO  ", 38, 32, true, true, true, false,false,true);
		}
		//ROM_GPIOPinWrite(GPIO_PORTE_BASE, PORTE_AUTOMANUALLEDPIN, PORTE_AUTOMANUALLEDPIN); //ON
		gstLEDcontrolRegister.autoManualLED = LED_ON;
	}
	else
	{
		
		 if(gu8_language == Japanese_IDX)
		{
		 displayText("              ", 1, 32, false,false, false, false,false,false);
               displayText("ƒ}ƒjƒ…ƒAƒ‹", 28, 32, true,  false, true, false,false,false);
		}
		else
		{
		displayText("MANUAL", 38, 32, true, true, true, false,false,true);
		}
		//ROM_GPIOPinWrite(GPIO_PORTE_BASE, PORTE_AUTOMANUALLEDPIN, 0); //OFF
		gstLEDcontrolRegister.autoManualLED = LED_OFF;
	}
        if(gu8_language == Japanese_IDX)
	{
	if(1 == gstControlBoardStatus.bits.runStop)
	{
		//displayText(" R U N  ", 94, 32, true, true, true, false);
		displayText("RUN ", 83, 32, true, true, true, false,false,false);
	}
	else
	{
		//displayText(" S T O P", 94, 32, true, true, true, false);
		displayText("STOP",83, 32, true, true, true, false,false,false);
	}
	}
	else
	{
        if(1 == gstControlBoardStatus.bits.runStop)
	{
		//displayText(" R U N  ", 94, 32, true, true, true, false);
		displayText(" RUN  ", 83, 32, true, true, true, false,false,true);
	}
	else
	{
		//displayText(" S T O P", 94, 32, true, true, true, false);
		displayText(" STOP ",83, 32, true, true, true, false,false,true);
	}
	}
if((gstDriveBoardStatus.bits.drivePowerOnCalibration != 1)&&(gstDriveBoardStatus.bits.driveRunTimeCalibration!=1)&&
                                         (gstDriveBoardStatus.bits.driveInstallation!=1)&&(gstDriveBoardStatus.bits.driveFaultUnrecoverable!=1))
{
	    if(((menu_gesture_flag_cyw == 0)||((menu_gesture_flag_cyw == 2)&&(gstControlBoardStatus.bits.autoManual==1)))&&(menu_gesture_flag_A007==0))  ////201806_Bug_No.10
                                 //201806_Bug_No.10
        {
        if(LCD_DISP_GUESTURE == 1)
        {
                                          
            DISP_GUESTER_13_16(2, 14, 32, 45);
                                        
          }
          else
         {                                 
          DISP_GUESTER_FAN(2, 14, 32, 45);
           }
       }
        else
        {
      //  // GrRectFIllBolymin(4, 12, 32, 45, true, true);
      ///    GrRectFIllBolymin(2, 14, 32, 45, true, true);
              DISP_GUESTER_FAN_DIS(2, 14, 32, 45);
         }
      }
	return 0;
}

/******************************************************************************
 * FunctionName: homeScreenMode
 *
 * Function Description:
 * Paints the main menu screen on display area.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t homeScreenMode(void)
{
	//
	// Handle Mode key press
	//
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
        //???????????EMODE
    	setting_flag  = 1;

    	gKeysStatus.bits.Key_Mode_pressed = 0;

    	//
    	// Allow mode key functionality if and only if there are no
    	// fatal/unrecoverable errors.
    	//
    	if(
    			(gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) &&
    			//	Added to check whether a control board fatal error has occurred - Jan 2016
    			(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)
    	)
    	{
			//
			// Handle Exception if there is no active functional block
			//
			if(psActiveFunctionalBlock == NULL)
				return 1;

			//
			// Reset Home screen variables
			//
			gTickCount1Seconds = 0;
			gTickCount3Seconds = 0;
			gDelay1SecStart = 0;
			gDelay3SecStart = 0;
			gHomeScreenState = GET_OPERATION_COUNT;
			gNextHSState = 0xFF;

			#ifdef GET_OPERATION_COUNT_LOWER_LIMIT
			gOperationCountCommandState = CHECK_FOR_LOWER_LIMIT;
			#endif

			#ifdef GET_OPERATION_COUNT_UPPER_APERTURE_LIMIT
			gucSentOperationCountCmd = 0;
			#endif

			gAnomalyIndex = 0;



			//
			// Set the active functional block as the child of currently active
			// functional block.
			//
			psActiveFunctionalBlock = psActiveFunctionalBlock->childInternalFunctions;

			//
			// Set active menu as main menu.
			//
			psActiveMenu = &gsMainMenu;

			//
			// Call the first screen paint function of active functional block.
			//
			psActiveFunctionalBlock->pfnPaintFirstScreen();
    	}

    }
 //
    	// Handle Mode key release
    	//
        //	It was observed that stop+open/stop+close was not working during installtion after
        //	initialize parameters or reset installation parameters. It was happening as mode release
        //	flag was not getting cleared. Added - Feb 2016
        if(gKeysStatus.bits.Key_Mode_released)
        {
        	gKeysStatus.bits.Key_Mode_released = 0;
        }

	return 0;
}

/******************************************************************************
 * FunctionName: homeScreenEnter
 *
 * Function Description:
 * This is a default function for enter key press on home screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t homeScreenEnter(void)
{
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;
	}

	if(gKeysStatus.bits.Key_Enter_released)
	{
		gKeysStatus.bits.Key_Enter_released = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: homeScreenUp
 *
 * Function Description:
 * This is a default function for up key press on home screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t homeScreenUp(void)
{
	if(gKeysStatus.bits.Key_Up_pressed)
	{
		gKeysStatus.bits.Key_Up_pressed = 0;
	}

	if(gKeysStatus.bits.Key_Up_released)
	{
		gKeysStatus.bits.Key_Up_released = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: homeScreenDown
 *
 * Function Description:
 * This is a default function for down key press on home screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t homeScreenDown(void)
{
	if(gKeysStatus.bits.Key_Down_pressed)
	{
		gKeysStatus.bits.Key_Down_pressed = 0;
	}

	if(gKeysStatus.bits.Key_Down_released)
	{
		gKeysStatus.bits.Key_Down_released = 0;
	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsHomeScreenFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	homeScreenPaint,
	homeScreenRunTime,
	homeScreenUp,
	homeScreenDown,
	homeScreenMode,
	homeScreenEnter
};
