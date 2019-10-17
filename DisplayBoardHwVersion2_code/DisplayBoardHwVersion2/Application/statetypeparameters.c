/*********************************************************************************
 * FileName: statetypeparameters.c
 * Description:
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
 *  	0.1D	11/06/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_types.h"
#include <driverlib/gpio.h>
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "grlib/grlib.h"
//#include "Middleware/cfal96x64x16.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "Middleware/paramdatabase.h"
#include "userinterface.h"
#include "logger.h"
#include "parameterlist.h"
#include "Gesture_Sensor/GP2AP054A_cyw.h"//add cyw
#include "Gesture_Sensor/ram_cyw.h"

/****************************************************************************/

/****************************************************************************
 *  Macros
****************************************************************************/

//***************************************************************************
// State type parameter setting states
//****************************************************************************/
#define PARAM_SET_STOPPED		0
#define START_PARAM_SETTING		1
#define PARAM_SET_STARTED		2


#define SERVICED		0
#define ACTIVATED		1
#define DEACTIVATED		2
/****************************************************************************/

/****************************************************************************
 *  Enumerations
****************************************************************************/

/****************************************************************************/

/****************************************************************************
 *  Structures
****************************************************************************/

/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
uint8_t gCurrentParameterState = 0;
uint8_t gStateParamGetStarted = 0;
uint8_t gGetParameterState = 0;

uint8_t gStateTypeParamSetStates = PARAM_SET_STOPPED;

extern uint32_t gShutterType;
static uint8_t  cyw_mode=0;
uint8_t flag_out_setting_cyw=0;
/****************************************************************************/

/******************************************************************************
 * FunctionName: stateTypeParamFirstScreen
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
uint8_t stateTypeParamFirstScreen()
{
	char lBuff[41];
	char cccccc[5]={0};
	//uint8_t tp_test;


	if(eDestDisplayBoard == gsParamDatabase[gHighlightedItemIndex].destination)
	{
				//
				// Read current value of parameter
				//
		readParameterFromDB((PARAM_DISP)gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex, (uint8_t *)&gCurrentParameterState);
		gGetParameterState = gCurrentParameterState;
	}
	//
	// Clear Screen
	//
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, 0, true);

	//
	// Display parameter information in first line
	//
	if(gu8_language == Japanese_IDX)
	{
	memcpy(cccccc,gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
	displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
	displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);
	}
	else
	{
	//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english, 2, 0, false, false, false, false, false, true);
	memcpy(cccccc,gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
	displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
	displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);

	}
	//
	// Display parameter state in second line
	//
	if((1 == gStateParamGetStarted)||(eDestDisplayBoard == gsParamDatabase[gHighlightedItemIndex].destination))
	{
		if(gu8_language == Japanese_IDX)
		{
		  usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_japanese + (gCurrentParameterState * NUMBER_OF_COLUMNS));
		}
		else
		{
			usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_english + (gCurrentParameterState * NUMBER_OF_COLUMNS));
		}
//		memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
//				 usnprintf_nU_cyw((char*)(lBuff_cyw),4,(char*)(lBuff));
											//	memset(lBuff_cyw+6,0x00,sizeof(lBuff_cyw)-6);
	//			 tp_test = strlen(lBuff) -2;
	//		  memcpy((char*)(lBuff_cyw+6),(char*)(lBuff+3),tp_test);
		if(gu8_language == Japanese_IDX)
		displayText((unsigned char *)lBuff, 1, 16, false, false, false, false, true, false);
		else
		displayText((unsigned char *)lBuff, 1, 16, false, false, false, false, false, true);
	}

	return 0;
}

/******************************************************************************
 * FunctionName: stateTypeParamRunTime
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
uint8_t stateTypeParamRunTime()
{
	static uint8_t lsDelay3SecStart = 0;
	static uint32_t lsTickCount3Seconds = 0;
	char cccccc[5]={0};

	if(PARAM_SET_STOPPED == gStateTypeParamSetStates)
	{
		if( (eINACTIVE == gstUMtoCMdatabase.commandRequestStatus) && (0 == gStateParamGetStarted) )
		{

			if(eDestDisplayBoard == gsParamDatabase[gHighlightedItemIndex].destination)
						{
							return 0;
						}

			//
			// Initiate Get Parameter command
			//
			gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 1;
			gstUMtoCMdatabase.dataToControlBoard.parameterNumber = gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex;

			//
			// Set command destination
			//
			gstUMtoCMdatabase.destination = gsParamDatabase[gHighlightedItemIndex].destination;

			//
			// command request status as active
			//
			gstUMtoCMdatabase.commandRequestStatus = eACTIVE;

			//
			// Set get parameter started flag
			//
			gStateParamGetStarted = 1;
		}

		//
		// Check for response
		//
		if(eACTIVE == gstUMtoCMdatabase.commandRequestStatus)
		{
			if(eSUCCESS == gstUMtoCMdatabase.commandResponseStatus)
			{
				if(eACK == gstUMtoCMdatabase.acknowledgementReceived)
				{
					//
					// save current value of parameter
					//
					if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == 522)
					{
						if(gShutterType == 0)
						{
							if(gstUMtoCMdatabase.getParameterValue == 900)
							{
								gCurrentParameterState =0;
							}
							else if(gstUMtoCMdatabase.getParameterValue == 1310)
							{
								gCurrentParameterState =1;
							}
							else if(gstUMtoCMdatabase.getParameterValue == 1750)
							{
								gCurrentParameterState =2;
							}
							else
							{
								gCurrentParameterState =2;
							}
						}
						if(gShutterType == 1)
						{
								if(gstUMtoCMdatabase.getParameterValue == 725)
								{
								gCurrentParameterState =0;
								}
								else if(gstUMtoCMdatabase.getParameterValue == 1090)
								{
						    	gCurrentParameterState =1;
								}
								else if(gstUMtoCMdatabase.getParameterValue == 1450)
								{
								gCurrentParameterState =2;
								}
								else
								{
								gCurrentParameterState =2;
								}
						}
						if(gShutterType == 2)
						{
							    if(gstUMtoCMdatabase.getParameterValue == 1200)//1250)     //add 20161018
								{
								gCurrentParameterState =0;
								}
								else if(gstUMtoCMdatabase.getParameterValue == 1875)
								{
								gCurrentParameterState =1;
								}
								else if(gstUMtoCMdatabase.getParameterValue == 2450)//2500)   //add 20161018
								{
								gCurrentParameterState =2;
								}
								else
								{
								gCurrentParameterState =2;
								}
						}
					}
					else if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == 528)
					{
						if(gShutterType == 0)
						{
							if(gstUMtoCMdatabase.getParameterValue == 500)
							{
								gCurrentParameterState =0;
							}
							else if(gstUMtoCMdatabase.getParameterValue == 700)
							{
								gCurrentParameterState =1;
							}
							else if(gstUMtoCMdatabase.getParameterValue == 900)
							{
								gCurrentParameterState =2;
							}
							else
							{
								gCurrentParameterState =2;
							}
						}
						if(gShutterType == 1)
						{
							if(gstUMtoCMdatabase.getParameterValue == 725)
							{
								gCurrentParameterState =0;
							}
							else if(gstUMtoCMdatabase.getParameterValue == 1090)
							{
								gCurrentParameterState =1;
							}
							else if(gstUMtoCMdatabase.getParameterValue == 1450)
							{
								gCurrentParameterState =2;
							}
							else
							{
							    gCurrentParameterState =2;
							}
						}
						if(gShutterType == 2)
						{
							if(gstUMtoCMdatabase.getParameterValue == 1200)//1250)    //add 20161018
							{
								gCurrentParameterState =0;
							}
							else if(gstUMtoCMdatabase.getParameterValue == 1875)
							{
								gCurrentParameterState =1;
							}
							else if(gstUMtoCMdatabase.getParameterValue == 2450)//2500)    //add 20161018
							{
								gCurrentParameterState =2;
							}
							else
							{
								gCurrentParameterState =2;
							}
						}
					}
					else
					{
				      gCurrentParameterState = gstUMtoCMdatabase.getParameterValue;
					}
					gGetParameterState = gCurrentParameterState;

					stateTypeParamFirstScreen();

					//
					// Reset command bit
					//
					gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 0;

					//
					// Reset Request and response status and acknowledgment status
					//
					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
					gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
				}
				else if(eNACK == gstUMtoCMdatabase.acknowledgementReceived)
				{
					//
					// Reset command bit
					//
					gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 0;

					//
					// Reset Request and response status and acknowledgment status
					//
					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
					gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;

		    		//
		    		// Reset current parameter value
		    		//
					gCurrentParameterState = 0;

		    		//
		    		// Start delay
		    		//
		    		if(0 == lsDelay3SecStart)
		    		{
			    		//
			    		// Clear Screen
			    		//
			    		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		    			GrRectFIllBolymin(0, 127, 0, 63, 0, true);

			    		//
			    		// Display parameter information first line
			    		//
		    			if(gu8_language == Japanese_IDX)
		    			{
			    		//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese, 2, 0, false, false, false, false, false, false);
                        memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
                        displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
                        displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);

                        //
			    		// Display parameter setting action failed message
			    		//

			    		displayText("パラメータセッティング", 2, 16, false, false, false, false,false,false);
			    		displayText("NG", 2, 32, false, false, false, false, false, false);
		    			}
		    			else
		    			{
		    				 memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
		    				 displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
		    				 displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);

		    				//
		    				// Display parameter setting action failed message
		    				//
		    				displayText("PARAMETER SETTING", 2, 16, false, false, false, false,false,true);
		    				displayText("ACTION FAILED", 2, 32, false, false, false, false, false, true);
		    			}
		    			//
		    			// Capture Time
		    			//
		    			lsTickCount3Seconds = g_ui32TickCount;

		    			//
		    			// Set delay start flag
		    			//
		    			lsDelay3SecStart = 1;
		    		}
				}
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
				// Reset command bit
				//
				gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 0;

				//
				// Reset request and response status
				//
				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

	    		//
	    		// Reset current parameter state
	    		//
	    		gCurrentParameterState = 0;

	    		//
	    		// Start delay
	    		//
	    		if(0 == lsDelay3SecStart)
	    		{
		    		//
		    		// Clear Screen
		    		//
		    		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	    			GrRectFIllBolymin(0, 127, 0, 63, 0, true);

		    		//
		    		// Display parameter information first line
		    		//

	    			if(gu8_language == Japanese_IDX)
	    			{
		    		//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese, 2, 0, false, false, false, false, false, false);
	    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
	    		    displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
	    			displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);

		    		//
		    		// Display failed message on second line
		    		//

		    		displayText("パラメータヨミコミNG", 2, 16, false, false, false, false, false, false);
		    		//
		    		// Paint error reason on fourth line
		    		//

		    		displayText("ツウシンエラー", 2, 48, false, false, false, false, false, false);
	    			}
	    			else
	    			{
	    			//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english, 2, 0, false, false, false, false, false, true);
	    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
	    			displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
	    		    displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);

	    			//
	    			// Display failed message on second line
	    			//
	    			displayText("PARAMETER GET FAILED", 2, 16, false, false, false, false, false, true);
	    			//
	    			// Paint error reason on fourth line
	    			//
	    			displayText("COMM FAILURE", 2, 48, false, false, false, false, false, true);

	    			}
	    			//
	    			// Capture Time
	    			//
	    			lsTickCount3Seconds = g_ui32TickCount;

	    			//
	    			// Set delay start flag
	    			//
	    			lsDelay3SecStart = 1;
	    		}
			}
		}

		//
		// Check whether delay is achieved
		//
		if( (get_timego( lsTickCount3Seconds) > 300) &&
			(1 == lsDelay3SecStart)
		  )
		{
			//
			// Return to parameter list functional block.
			//
			psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
			psActiveFunctionalBlock->pfnPaintFirstScreen();

			//
			// Reset delay start flag
			//
			lsDelay3SecStart = 0;

			//
			// Reset Get parameter started flag
			//
			gStateParamGetStarted = 0;
		}
	}

	updateFaultLEDStatus();

	return 0;
}

/******************************************************************************
 * FunctionName: stateTypeParamUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t stateTypeParamUp()
{
	char lBuff[41];
	//uint8_t tp_test;

    if(gKeysStatus.bits.Key_Up_pressed)
    {
    	gKeysStatus.bits.Key_Up_pressed = 0;

    	if(START_PARAM_SETTING == gStateTypeParamSetStates)
    	{
			if(gCurrentParameterState < gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.paramStateCount - 1)
				gCurrentParameterState++;


			//
			// Clear second line
			//
			//GrRectFIllBolymin(0, 126, 16, 31, true, true);
			GrRectFIllBolymin(0, 127, 16, 31, 0, true);

			//
			// Display parameter state in second line
			//
			if(gu8_language == Japanese_IDX)
			{
			usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_japanese + (gCurrentParameterState * NUMBER_OF_COLUMNS));
			}
			else
			{
			usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_english + (gCurrentParameterState * NUMBER_OF_COLUMNS));

			}
			//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
			//usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
		//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
		//			 usnprintf_nU_cyw((char*)(lBuff_cyw),4,(char*)(lBuff));
		//										//	memset(lBuff_cyw+6,0x00,sizeof(lBuff_cyw)-6);
			//		 tp_test = strlen(lBuff) -2;
		//		  memcpy((char*)(lBuff_cyw+6),(char*)(lBuff+3),tp_test);
			if(gu8_language == Japanese_IDX)
			displayText((unsigned char *)lBuff, 1, 16, true, false, false, false ,true,false);
			else
			displayText((unsigned char *)lBuff, 1, 16, true, false, false, false ,false,true);
    	}
    }

    return 0;
}

/******************************************************************************
 * FunctionName: stateTypeParamDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t stateTypeParamDown()
{
	char lBuff[41];
	//uint8_t  tp_test;

    if(gKeysStatus.bits.Key_Down_pressed)
    {
    	gKeysStatus.bits.Key_Down_pressed = 0;

    	if(START_PARAM_SETTING == gStateTypeParamSetStates)
    	{
			if(gCurrentParameterState > 0)
				gCurrentParameterState--;

			//
			// Clear second line
			//
			//GrRectFIllBolymin(0, 126, 16, 31, true, true);
			GrRectFIllBolymin(0, 127, 16, 31, 0, true);

			//
			// Display parameter state in second line
			//
			if(gu8_language == Japanese_IDX)
			{
			usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_japanese + (gCurrentParameterState * NUMBER_OF_COLUMNS));
			}
			else
			{
				usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_english + (gCurrentParameterState * NUMBER_OF_COLUMNS));
			}
			//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
			//			usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
	//		memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
		//	 usnprintf_nU_cyw((char*)(lBuff_cyw),4,(char*)(lBuff));
										//	memset(lBuff_cyw+6,0x00,sizeof(lBuff_cyw)-6);
		//	 tp_test = strlen(lBuff) -2;
		//  memcpy((char*)(lBuff_cyw+6),(char*)(lBuff+3),tp_test);
			if(gu8_language == Japanese_IDX)
			displayText((unsigned char *)lBuff, 1, 16, true, false, false, false ,true, false);
			else
			displayText((unsigned char *)lBuff, 1, 16, true, false, false, false ,false, true);
    	}
    }

    return 0;
}

/******************************************************************************
 * FunctionName: stateTypeParamMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t stateTypeParamMode()
{
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
    	gKeysStatus.bits.Key_Mode_pressed = 0;

		//
		// Reset parameter setting started flag
		//
		gStateTypeParamSetStates = PARAM_SET_STOPPED;

		//
		// Reset current parameter state
		//
		gCurrentParameterState = 0;

		//
		// Reset get parameter started flag
		//
		gStateParamGetStarted = 0;
		//




		//
		// Return to parameter list functional block.
		//
		psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
		psActiveFunctionalBlock->pfnPaintFirstScreen();

		cyw_mode = 1;
    }

    return 0;
}

/******************************************************************************
 * FunctionName: stateTypeParamEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t stateTypeParamEnter()
{
	char lBuff[41];
	//uint8_t tp_test;
	char cccccc[5]={0};

	static uint8_t lsDelay3SecStart = 0;
	static uint32_t lsTickCount3Seconds = 0;

	if(cyw_mode == 1)
    {
		lsDelay3SecStart = 0;
		cyw_mode = 0;
    }
    if(gKeysStatus.bits.Key_Enter_pressed)
    {
    	gKeysStatus.bits.Key_Enter_pressed = 0;

    	if( (PARAM_SET_STOPPED == gStateTypeParamSetStates) &&
    		(false == gsParamDatabase[gHighlightedItemIndex].readOnly)
    	  )
    	{
    		gStateTypeParamSetStates = START_PARAM_SETTING;

    		//
    		// Display highlighted parameter state in second line
    		//
    		if(gu8_language == Japanese_IDX)
    		{
    		usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_japanese + (gCurrentParameterState * NUMBER_OF_COLUMNS));
    		}
    		else
    		{
    		usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_english + (gCurrentParameterState * NUMBER_OF_COLUMNS));
    		}
    		//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
    		//						usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
    	//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
    	//			 usnprintf_nU_cyw((char*)(lBuff_cyw),4,(char*)(lBuff));
    											//	memset(lBuff_cyw+6,0x00,sizeof(lBuff_cyw)-6);
    	//			 tp_test = strlen(lBuff) -2;
    		//	  memcpy((char*)(lBuff_cyw+6),(char*)(lBuff+3),tp_test);
    		if(gu8_language == Japanese_IDX)
    		displayText((unsigned char *)lBuff, 1, 16, true, false, false, false ,true ,false);
    		else
    		displayText((unsigned char *)lBuff, 1, 16, true, false, false, false ,false ,true);
    	}
    	else if(START_PARAM_SETTING == gStateTypeParamSetStates)
    	{
    		if( (1 != gstDriveBoardStatus.bits.shutterStopped) // ||
    			//( (1 != gstDriveBoardStatus.bits.shutterUpperLimit) && (1 != gstDriveBoardStatus.bits.shutterLowerLimit) )
    		  ) // Condition for restricting set parameter command
    		{



    			//
        		// Clear Screen
        		//
        		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
    			GrRectFIllBolymin(0, 127, 0, 63, 0, true);

        		//
        		// Display parameter information first line
        		//
    			if(gu8_language == Japanese_IDX)
    			{
        		//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese, 2, 0, false, false, false, false,false, false);
    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
    			displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
    			displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);

        		//
        		// Display failed message on second line
        		//

        		displayText("パラメータセットNG", 2, 16, false, false, false, false,false,false);
        		//
        		// Paint error reason on fourth line
        		//

        		displayText("シャッタードウサチュウ", 2, 48, false, false, false, false, false, false);
    			}
    			else
    			{
    			//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english, 2, 0, false, false, false, false,false, true);
    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
    			displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
    			displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);

    			  //
    			  // Display failed message on second line
    			  //
    			   displayText("PARAMETER SET FAILED", 2, 16, false, false, false, false,false,true);

    			   //
    			   // Paint error reason on fourth line
    			   //
    			   displayText("SHUTTER MOVING", 2, 48, false, false, false, false, false, true);

    			}
        		//
        		// Start delay
        		//
        		if(0 == lsDelay3SecStart)
        		{
        			//
        			// Capture Time
        			//
        			lsTickCount3Seconds = g_ui32TickCount;

        			//
        			// Set delay start flag
        			//
        			lsDelay3SecStart = 1;
        		}
    		}

    		else if( (1 == gstDriveBoardStatus.bits.shutterStopped) //&&
					 //( (1 == gstDriveBoardStatus.bits.shutterUpperLimit) || (1 == gstDriveBoardStatus.bits.shutterLowerLimit) )
				   )
    		{
    			if(eDestDisplayBoard == gsParamDatabase[gHighlightedItemIndex].destination)
    			{

    				writeParameterUpdateInDB((PARAM_DISP)gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex, (uint8_t *)&gCurrentParameterState);

    				gstUMtoLM_write.commandRequestStatus = eACTIVE;
    				    		gstUMtoLM_write.commandResponseStatus = eNO_STATUS;
    				    		gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory = 1;
    				    		gstUMtoLM_write.changeSetting.newValue = gCurrentParameterState;
    				    		gstUMtoLM_write.changeSetting.oldValue = gGetParameterState;
    				    		//20170414      201703_No.28 start
    				    		gstUMtoLM_write.changeSetting.parameterNumber = (gsParamDatabase[gHighlightedItemIndex].paramName_english[1]-'0')*100 +
    				    				                                        (gsParamDatabase[gHighlightedItemIndex].paramName_english[2]-'0')*10  +
																				(gsParamDatabase[gHighlightedItemIndex].paramName_english[3]-'0');//gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex;
    				    		//20170414      201703_No.28 end
    				    		gstUMtoLM_write.changeSetting.timeStamp = (HWREG(0x400FC000));
    				    		writeChangeSettingsHistory();
    				    		GrRectFIllBolymin(0, 127, 0, 63, 0, true);

    				    	    		//
    				    	    		// Display parameter information first line
    				    	    		//
    				    		if(gu8_language == Japanese_IDX)
    				    		{
    				    		//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese, 2, 0, false, false, false, false, false, false);
    				    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
    				    		    displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
    				    			displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);

    				    		}
    				    		else
    				    		{
    				    		//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english, 2, 0, false, false, false, false, false, true);
    				    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
    				    			displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
    				    			displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);

    				    		}
    				    	    		//
    				    	    		// Clear second line
    				    	    		//
    				    	    		//GrRectFIllBolymin(0, 126, 16, 31, true, true);
    				    	    		GrRectFIllBolymin(0, 127, 16, 31, 0, true);

    				    	    		//
    				    	    		// Display parameter state in second line
    				    	    		//
    				    	    		if(gu8_language == Japanese_IDX)
    				    	    		{
    				    	    		usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_japanese + (gCurrentParameterState * NUMBER_OF_COLUMNS));
    				    	    		}
    				    	    		else
    				    	    		{
    				    	    			usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_english + (gCurrentParameterState * NUMBER_OF_COLUMNS));
    				    	    		}
    				    	    		//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
    				    	    		//						usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
    				    	    	//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
    				    	    	//			 usnprintf_nU_cyw((char*)(lBuff_cyw),4,(char*)(lBuff));
    				    	    	//										//	memset(lBuff_cyw+6,0x00,sizeof(lBuff_cyw)-6);
    				    	    		//		 tp_test = strlen(lBuff) -2;
    				    	    	//		  memcpy((char*)(lBuff_cyw+6),(char*)(lBuff+3),tp_test);
    				    	    		if(gu8_language == Japanese_IDX)
    				    	    		displayText((unsigned char *)lBuff, 1, 16, false, false, false, false, true, false);
    				    	    		else
    				    	    		displayText((unsigned char *)lBuff, 1, 16, false, false, false, false, false, true);

    				    	    		//
    				    	    		// Display completed message on third line
    				    	    		//
    				    	    		if(gu8_language == Japanese_IDX)
    				    	    		{
    				    	    		//
    				    	    		displayText("カンリョウ", 2, 32, false, false, false, false, false, false);
    				    	    		}
    				    	    		else
    				    	    		{
    				    	    		displayText("COMPLETED", 2, 32, false, false, false, false, false, true);
    				    	    		}
    				    	    		//

    				    	    		if(0 == lsDelay3SecStart)
    				    	    		    		{
    				    	    		    			//
    				    	    		    			// Capture Time
    				    	    		    			//
    				    	    		    			lsTickCount3Seconds = g_ui32TickCount;

    				    	    		    			//
    				    	    		    			// Set delay start flag
    				    	    		    			//
    				    	    		    			lsDelay3SecStart = 1;
    				    	    		    		}

    				    	    		    		//
    				    	    		    		// Reset command bit
    				    	    		    		//
    				    	    		    		gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 0;

    				    	    		    		//
    				    	    		    		// Reset command request and response status
    				    	    		    		//
    				    	    		    		gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
    				    	    		    		gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

    				    	    		    		return 0;

    			}


    			//
				// Set parameter setting started flag
				//
				gStateTypeParamSetStates = PARAM_SET_STARTED;
			    if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == 7)     //201806_Bug_No.10
			    	  menu_gesture_flag_A007= gCurrentParameterState;

				//
				// Initiate set parameter command
				//

				if(eINACTIVE == gstUMtoCMdatabase.commandRequestStatus)
				{
					gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 1;
					gstUMtoCMdatabase.dataToControlBoard.parameterNumber = gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex;
					if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber==522)//up
					{
						if(gCurrentParameterState == 0)
						{
							if(gShutterType == 0)//bd
							{
							   gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue =900;
							}
							else if(gShutterType == 1)
							{
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue =725;
							}
							else
							{
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue =1200;//1250;   //add 20161018
							}

						}
						if(gCurrentParameterState == 1)
						{
							if(gShutterType == 0)
							{
								 gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 1310;
							}
							else if(gShutterType == 1)
							{
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 1090;
						    }
							else
							{
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 1875;
							}

						}
						if(gCurrentParameterState == 2)
						{
							if(gShutterType == 0)
							{
								 gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 1750;
							}
							else if(gShutterType == 1)
							{
							     gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 1450;
							 }
							else
							{
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 2450;//2500;    //add 20161018
							}

						}
					}
					else if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber==528)
					{
						if(gCurrentParameterState == 0)
						{
							if(gShutterType == 0)
							{
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 500;
							}
							else if(gShutterType == 1)
						   {
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 725;
						   }
							else
							{
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 1200;//1250;    //add 20161018
							}

					   }
						if(gCurrentParameterState == 1)
						{
						   if(gShutterType == 0)
							{
								 gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 700;
							}
							else if(gShutterType == 1)
							{
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 1090;
							}
							else
							{
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 1875;
							}
						}
						if(gCurrentParameterState == 2)
					    {
							if(gShutterType == 0)
							{
								 gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 900;
							}
							else if(gShutterType == 1)
							{
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 1450;
							}
							else
							{
								gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = 2450;//2500;    //add 20161018
							}

						}
					}
					else
					{
					    gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = gCurrentParameterState;
					}
					gstUMtoCMdatabase.destination = gsParamDatabase[gHighlightedItemIndex].destination;

					//
					// Set command request status as active
					//
					gstUMtoCMdatabase.commandRequestStatus = eACTIVE;


				}
    		}
    	}
    }

    //
    // Check for set parameter response
    //
    if(eACTIVE == gstUMtoCMdatabase.commandRequestStatus)
    {
    	if(eSUCCESS == gstUMtoCMdatabase.commandResponseStatus)
    	{
    		gstUMtoLM_write.commandRequestStatus = eACTIVE;
    		gstUMtoLM_write.commandResponseStatus = eNO_STATUS;
    		gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory = 1;
    		gstUMtoLM_write.changeSetting.newValue = gCurrentParameterState;
    		gstUMtoLM_write.changeSetting.oldValue = gGetParameterState;
    		gstUMtoLM_write.changeSetting.parameterNumber = gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex;
    		gstUMtoLM_write.changeSetting.timeStamp = (HWREG(0x400FC000));
    		writeChangeSettingsHistory();

    		//
    		// Clear Screen
    		//
    		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
    		GrRectFIllBolymin(0, 127, 0, 63, 0, true);

    		//
    		// Display parameter information first line
    		//
    		if(gu8_language == Japanese_IDX)
    		{
    		//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese, 2, 0, false, false, false, false, false, false);
    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
    			displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
    			displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);

    		}
    		else
    		{
    		//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english, 2, 0, false, false, false, false, false, true);
    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
    			displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
    			displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);

    		}
    		//
    		// Clear second line
    		//
    		//GrRectFIllBolymin(0, 126, 16, 31, true, true);
    		GrRectFIllBolymin(0, 127, 16, 31, 0, true);

    		//
    		// Display parameter state in second line
    		//
    		if(gu8_language == Japanese_IDX)
    		{
    		usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_japanese + (gCurrentParameterState * NUMBER_OF_COLUMNS));
    		}
    		else
    		{
    			usnprintf(lBuff, 41, "%u: %s", gCurrentParameterState, gsParamDatabase[gHighlightedItemIndex].stateTypeEntities.pStateString_english + (gCurrentParameterState * NUMBER_OF_COLUMNS));
    		}
    		//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
    		//						usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
    	//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
    	//			 usnprintf_nU_cyw((char*)(lBuff_cyw),4,(char*)(lBuff));
    	//										//	memset(lBuff_cyw+6,0x00,sizeof(lBuff_cyw)-6);
    		//		 tp_test = strlen(lBuff) -2;
    	//		  memcpy((char*)(lBuff_cyw+6),(char*)(lBuff+3),tp_test);
    		if(gu8_language == Japanese_IDX)
    		displayText((unsigned char *)lBuff, 1, 16, false, false, false, false, true, false);
    		else
    		displayText((unsigned char *)lBuff, 1, 16, false, false, false, false, false, true);
    		//
    		// Display completed message on third line
    		//
    		if(gu8_language == Japanese_IDX)
    		{
    		//
    		displayText("カンリョウ", 2, 32, false, false, false, false, false, false);
    		}
    		else
    		{
    		displayText("COMPLETED", 2, 32, false, false, false, false, false, true);
    		}
    		//
    		// Start delay
    		//
    		if(0 == lsDelay3SecStart)
    		{
    			//
    			// Capture Time
    			//
    			lsTickCount3Seconds = g_ui32TickCount;

    			//
    			// Set delay start flag
    			//
    			lsDelay3SecStart = 1;
    		}

    		//
    		// Reset command bit
    		//
    		gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 0;

    		//
    		// Reset command request and response status
    		//
    		gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
    		gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

    	}
    	else if( (eTIME_OUT == gstUMtoCMdatabase.commandResponseStatus) ||
    			 (eFAIL == gstUMtoCMdatabase.commandResponseStatus)
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
    		// Clear Screen
    		//
    		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
			GrRectFIllBolymin(0, 127, 0, 63, 0, true);

    		//
    		// Display parameter information first line
    		//
			if(gu8_language == Japanese_IDX)
			{
    		//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese, 2, 0, false, false, false, false, false, false);
				memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
				displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
				displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);

			}
    		else
    		{
			//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english, 2, 0, false, false, false, false, false, true);
    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
    			displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
    			displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);

    		}
    		//
    		// Display failed message on second line
    		//
			if(gu8_language == Japanese_IDX)
			{
    		//
    		displayText("パラメータセットNG", 2, 16, false, false, false, false,false,false);
    		//
    		// Paint error reason on fourth line
    		//
    		//
    		displayText("ツウシンエラー", 2, 48, false, false, false, false, false, false);
			}
			else
			{
				displayText("PARAMETER SET FAILED", 2, 16, false, false, false, false,false,true);
				displayText("COMM FAILURE", 2, 48, false, false, false, false, false, true);
			}
    		//
    		// Start delay
    		//
    		if(0 == lsDelay3SecStart)
    		{
    			//
    			// Capture Time
    			//
    			lsTickCount3Seconds = g_ui32TickCount;

    			//
    			// Set delay start flag
    			//
    			lsDelay3SecStart = 1;
    		}

    		//
    		// Reset command bit
    		//
    		gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 0;

    		//
    		// Reset command request and response status
    		//
    		gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
    		gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
    	}
    }

	//
	// Check whether delay is achieved
	//
	if( (get_timego(lsTickCount3Seconds) > 300) &&
		(1 == lsDelay3SecStart)
	  )
	{
		//
		// Return to parameter list functional block.
		//
		//psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
		psActiveFunctionalBlock->pfnPaintFirstScreen();

		//
		// Reset parameter setting started flag
		//
		gStateTypeParamSetStates = PARAM_SET_STOPPED;

		//
		// Reset get parameter started flag
		//
		gStateParamGetStarted = 0;


		//
		// Reset delay start flag
		//
		lsDelay3SecStart = 0;

		gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
		gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

		if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber==537)
		{
			//psActiveMenu->ui8FocusIndex = 0;
			//psActiveFunctionalBlock = &gsHomeScreenFunctionalBlock;
			gShutterType =gCurrentParameterState ;
			//gui8SettingsModeStatus = DEACTIVATED;
			if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
			{
				gui8SettingsModeStatus = SERVICED;
				gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus = 1;
				gstUMtoCMoperational.additionalCommandData = 0;
				gstUMtoCMoperational.commandRequestStatus = eACTIVE;
				flag_out_setting_cyw = 1;
			}
			gui8SettingsModeStatus = DEACTIVATED;
			gui8SettingsScreen = DEACTIVATED;

		}

	}

    return 0;
}

/******************************************************************************
 * Define State type parameter Functional Block
*********************************************************************************/
stInternalFunctions gsStateTypeParamFunctionalBlock =
{
	&gsParameterListFunctionalBlock,
	0,
	stateTypeParamFirstScreen,
	stateTypeParamRunTime,
	stateTypeParamUp,
	stateTypeParamDown,
	stateTypeParamMode,
	stateTypeParamEnter
};
