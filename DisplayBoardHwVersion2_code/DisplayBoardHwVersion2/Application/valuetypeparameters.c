/*********************************************************************************
 * FileName: valuetypeparameters.c
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
 *  	0.1D	11/06/2014      	iGATE Offshore team       	Initial Creation.
 *  	0.2D						iGATE Offshore team			Display of digits in parameter value is set w.r.t. to current
 *  															value of parameter obtained in get parameter command reply.
 *  	0.3D						iGATE offshore team			Value type parameter first screen display after set parameter success.
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
#include "userinterface.h"
#include "Middleware/paramdatabase.h"
#include "logger.h"
#include "parameterlist.h"

/****************************************************************************/

/****************************************************************************
 *  Macros
****************************************************************************/

#define PARAM_OP_CNT_INPUT 	28

//***************************************************************************
// Parameter setting focus indices
//****************************************************************************/
#define PARAM_ONE_MILLIONS			0
#define PARAM_HUNDRED_THOUSANDS		1
#define PARAM_TEN_THOUSANDS			2
#define PARAM_THOUSANDS				3
#define PARAM_HUNDREDS				4
#define PARAM_TENS					5
#define PARAM_ONES					6

#define MAX_DIGITS					7

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
typedef struct _stValue
{
	uint8_t oneMillions;
	uint8_t hundredThousands;
	uint8_t tenThousands;
	uint8_t thousands;
	uint8_t hundreds;
	uint8_t tens;
	uint8_t ones;
} stValue;
/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
uint32_t gCurrentParameterValue = 0;
uint32_t gGetParameterValue = 0;
int8_t gValueTypeItemFocusIndex = -1;

uint8_t gValueParamSetStarted = 0;
uint8_t gValueParamGetStarted = 0;

stValue gsValue;
uint8_t gDigitCount = 0;

/****************************************************************************/

/******************************************************************************
 * FunctionName: valueTypeParamFirstScreen
 *
 * Function Description:
 * This function renders name and current value of the parameter that is going
 * through setting process.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
uint8_t valueTypeParamFirstScreen()
{
	char lBuff[21];
	uint32_t lui32CurrentVal = 0;
	uint32_t n = 0;
	uint8_t count = 0,Tp_long=0;
	char cccccc[5]={0};

#if 1	// check whether destination for highlighted item is display board
		if(eDestDisplayBoard == gsParamDatabase[gHighlightedItemIndex].destination)
		{
			//
			// Read current value of parameter
			//
			readParameterFromDB((PARAM_DISP)gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex, (uint8_t *)&gCurrentParameterValue);
			gGetParameterValue = gCurrentParameterValue;
		}
#endif

	lui32CurrentVal = gCurrentParameterValue;

	//
	// Get digits of current value
	//
	gsValue.oneMillions = lui32CurrentVal/1000000;
	lui32CurrentVal %= 1000000;
	gsValue.hundredThousands = lui32CurrentVal/100000;
	lui32CurrentVal %= 100000;
	gsValue.tenThousands = lui32CurrentVal/10000;
	lui32CurrentVal %= 10000;
	gsValue.thousands = lui32CurrentVal/1000;
	lui32CurrentVal %= 1000;
	gsValue.hundreds = lui32CurrentVal/100;
	lui32CurrentVal %= 100;
	gsValue.tens = lui32CurrentVal/10;
	lui32CurrentVal %= 10;
	gsValue.ones = lui32CurrentVal;

	//
	// Calculate maximum digits to be displayed and focus index
	//
	n = gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.maxVal.ui32Val;
	while(n != 0)
	{
		n /= 10;
		++count;
	}
	gDigitCount = count;
	gValueTypeItemFocusIndex = MAX_DIGITS - gDigitCount -1;

	//
	// Clear Screen
	//
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, true, true);

	//
	// Display parameter information first line
	//
	if(gu8_language == Japanese_IDX)
	{
	memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
	displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
    displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);
	}
	else
	{
	memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
	displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
	displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);
	}


	//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

	//
	// Display parameter value and unit in second line
	//
	if( (1 == gValueParamGetStarted) || (eDestDisplayBoard == gsParamDatabase[gHighlightedItemIndex].destination) )
	{
		if(0 != gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString)
		{
			if(gsParamDatabase[gHighlightedItemIndex].isSigned == true)
				usnprintf(lBuff, 21, "%d %s", (int16_t)gCurrentParameterValue, gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString);
			else
				usnprintf(lBuff, 21, "%d %s", gCurrentParameterValue, gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString);
			//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
			//usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));

			displayText((unsigned char *)lBuff, 2, 16, false, false, false, false, false, true);
		}
		else
		{
			if(gsParamDatabase[gHighlightedItemIndex].isSigned == true)
				usnprintf(lBuff, 21, "%d", (int16_t)gCurrentParameterValue);
			else
				usnprintf(lBuff, 21, "%d", gCurrentParameterValue);

		//			memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
		//						usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
			displayText((unsigned char *)lBuff, 2, 16, false, false, false, false, false, true);
		}
	}

	return 0;
}

/******************************************************************************
 * FunctionName: valueTypeParamRunTime
 *
 * Function Description:
 * This function performs runtime activities. This function initiates parameter get
 * request to fetch current value of parameter.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
uint8_t valueTypeParamRunTime()
{
	static uint8_t lsDelay3SecStart = 0;
	static uint32_t lsTickCount3Seconds = 0;
	char cccccc[5]={0};

	if( ((MAX_DIGITS - gDigitCount -1) == gValueTypeItemFocusIndex) && (0 == gValueParamSetStarted) )
	{
		if( (eINACTIVE == gstUMtoCMdatabase.commandRequestStatus) && (0 == gValueParamGetStarted) )
		{
			//
			// Return if highlighted item is a display board command
			//
			if(eDestDisplayBoard == gsParamDatabase[gHighlightedItemIndex].destination)
			{
				return 0;
			}

			//
			// Initiate Get Parameter Command
			//
			gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 1;
			gstUMtoCMdatabase.dataToControlBoard.parameterNumber = gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex;
			//if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber==130)
			//{
			//	gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 129;
			//	gui8SettingsModeStatus = DEACTIVATED;
			//	gui8SettingsScreen = DEACTIVATED;
			//}
			//
			// Set command destination
			//
			gstUMtoCMdatabase.destination = gsParamDatabase[gHighlightedItemIndex].destination;

			//
			// command request status as active
			//
			gstUMtoCMdatabase.commandRequestStatus = eACTIVE;

			//
			// Set Get parameter started flag
			//
			gValueParamGetStarted = 1;
		}

		//
		// Check for response
		//
		if( (eACTIVE == gstUMtoCMdatabase.commandRequestStatus) && (1 == gstUMtoCMdatabase.commandToControlBoard.bits.getParameter) )
		{
			if(eSUCCESS == gstUMtoCMdatabase.commandResponseStatus)
			{
				if(eACK == gstUMtoCMdatabase.acknowledgementReceived)
				{
					//
					// save current value of parameter
					//
					gCurrentParameterValue = gstUMtoCMdatabase.getParameterValue;
					gGetParameterValue = gCurrentParameterValue;

					//
					// Paint value type parameter first screen.
					//
					valueTypeParamFirstScreen();

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
		    		gCurrentParameterValue = 0;

		    		//
		    		// Start delay
		    		//
		    		if(0 == lsDelay3SecStart)
		    		{
			    		//
			    		// Clear Screen
			    		//
			    		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		    			GrRectFIllBolymin(0, 127, 0, 63, true, true);

			    		//
			    		// Display parameter information first line
			    		//
		    			if(gu8_language == Japanese_IDX)
		    			{
		    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
		    				displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
		    			    displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);
		    			}
		    			else
		    			{
		    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
		    			displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
		    			displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);
		    			}
			    		//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

			    		//
			    		// Display parameter setting action failed message
			    		//
			    		//displayText("PARAMETER SETTING", 2, 16, false, false, false, false, false, true);
			    		//displayText("ACTION FAILED", 2, 32, false, false, false, false, false, false);
		    			if(gu8_language == Japanese_IDX)
		    			{
		    			displayText("パラメータセッティング", 2, 16, false, false, false, false, false, false);
		    		    displayText("NG", 2, 32, false, false, false, false, false, false);
		    			}
		    			else
		    			{
		    			displayText("PARAMETER SETTING", 2, 16, false, false, false, false, false, true);
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

				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

	    		//
	    		// Reset current parameter value
	    		//
	    		gCurrentParameterValue = 0;

	    		//
	    		// Start delay
	    		//
	    		if(0 == lsDelay3SecStart)
	    		{
		    		//
		    		// Clear Screen
		    		//
		    		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	    			GrRectFIllBolymin(0, 127, 0, 63, true, true);

		    		//
		    		// Display parameter information first line
		    		//
	    			if(gu8_language == Japanese_IDX)
	    			{
	    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
	    			displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
	    			displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);

		    		//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

		    		//
		    		// Display failed message on second line
		    		//
		    		//displayText("PARAMETER GET FAILED", 2, 16, false, false, false, false, false, true);
	    			displayText("パラメータヨミコミNG", 2, 16, false, false, false, false, false, false);
		    		//
		    		// Paint error reason on fourth line
		    		//
		    		//displayText("COMM FAILURE", 2, 48, false, false, false, false, false, false);
	    			displayText("ツウシンエラー", 2, 48, false, false, false, false, false, false);
	    			}
	    			else
	    			{
	    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
	    			displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
	    			displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);

	    			//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

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
			gValueParamGetStarted = 0;
		}
	}

	updateFaultLEDStatus();

	return 0;
}

/******************************************************************************
 * FunctionName: valueTypeParamUp
 *
 * Function Description:
 * This function increments the value of a digit between 0-9 and paints the value
 * again on screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t valueTypeParamUp()
{
	uint8_t ui8x = 0;
	char lBuff[21];
	char cccccc[5]={0};



	int lui8ReturnValue = 2;

    if(gKeysStatus.bits.Key_Up_pressed)
    {
    	gKeysStatus.bits.Key_Up_pressed = 0;

    	if(gValueTypeItemFocusIndex >= (MAX_DIGITS - gDigitCount))
    	//if( -1 != gValueTypeItemFocusIndex )
    	{
			//
			// Display parameter range information in third line
			//
			memset(lBuff, 0, sizeof(char) * 21);

			if(0 == gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString)
			{
				usnprintf(lBuff, 21, "RANGE:%u-%u", gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.minVal,
						gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.maxVal);
			}
			else
			{
				usnprintf(lBuff, 21, "RANGE:%u-%u %s", gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.minVal,
						gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.maxVal,
						gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString);
			}
	//		memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
		//								usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
			displayText((unsigned char *)lBuff, 2, 32, false, false, false, false, false, false);

			//
			// Clear second line
			//
			//GrRectFIllBolymin(0, 126, 16, 31, true, true);
			GrRectFIllBolymin(0, 127, 16, 31, true, true);

			for(ui8x = MAX_DIGITS - gDigitCount; ui8x < MAX_DIGITS; ui8x++)
			{
				switch(ui8x)
				{
					case PARAM_ONE_MILLIONS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.oneMillions < 9))
						{
							gsValue.oneMillions++;
						}

						usnprintf(lBuff, 2, "%u", gsValue.oneMillions);
				//		memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//							usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_HUNDRED_THOUSANDS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.hundredThousands < 9))
						{
							gsValue.hundredThousands++;
						}

						usnprintf(lBuff, 2, "%u", gsValue.hundredThousands);
					//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//							usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_TEN_THOUSANDS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.tenThousands < 9))
						{
							gsValue.tenThousands++;
						}

						usnprintf(lBuff, 2, "%u", gsValue.tenThousands);
					//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//								usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_THOUSANDS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.thousands < 9))
						{
							gsValue.thousands++;
						}

						usnprintf(lBuff, 2, "%u", gsValue.thousands);
					//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
							//						usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_HUNDREDS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.hundreds < 9))
						{
							gsValue.hundreds++;
						}

						usnprintf(lBuff, 2, "%u", gsValue.hundreds);
						//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
								//					usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_TENS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.tens < 9))
						{
							gsValue.tens++;
						}

						usnprintf(lBuff, 2, "%u", gsValue.tens);
					//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
							//						usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_ONES:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.ones < 9))
						{
							gsValue.ones++;
						}

						usnprintf(lBuff, 2, "%u", gsValue.ones);
					//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//							usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}
				}

				if(ui8x == gValueTypeItemFocusIndex)
					//lui8ReturnValue = displayText((unsigned char *)lBuff_cyw, lui8ReturnValue, 16, true, true, false, false, false, false);
					lui8ReturnValue = displayText((unsigned char *)lBuff, lui8ReturnValue, 16, true, true, false, false, false, false);

					else
					//lui8ReturnValue = displayText((unsigned char *)lBuff_cyw, lui8ReturnValue, 16, false, false, false, false, false, false);
						lui8ReturnValue = displayText((unsigned char *)lBuff, lui8ReturnValue, 16, false, false, false, false, false, false);

			}

			if(0 != gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString)
			{
				usnprintf(lBuff, 21, " %s", gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString);
				//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//							usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
				displayText((unsigned char *)lBuff, lui8ReturnValue , 16, false, false, false, false, false, false);
			}
    	}
    }

    return 0;
}

/******************************************************************************
 * FunctionName: valueTypeParamDown
 *
 * Function Description:
 * This function decrements the value of a digit between 0-9 and paints the value
 * again on screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t valueTypeParamDown()
{
	uint8_t ui8x = 0;
	char lBuff[21];

	int lui8ReturnValue = 2;

    if(gKeysStatus.bits.Key_Down_pressed)
    {
    	gKeysStatus.bits.Key_Down_pressed = 0;

    	if(gValueTypeItemFocusIndex >= (MAX_DIGITS - gDigitCount))
    	//if( -1 != gValueTypeItemFocusIndex )
    	{
			//
			// Display parameter range information in third line
			//
			memset(lBuff, 0, sizeof(char) * 21);

			if(0 == gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString)
			{
				usnprintf(lBuff, 21, "RANGE:%u-%u", gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.minVal,
						gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.maxVal);
			}
			else
			{
				usnprintf(lBuff, 21, "RANGE:%u-%u %s", gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.minVal,
						gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.maxVal,
						gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString);
			}

			//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
			//							usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));

			displayText((unsigned char *)lBuff, 2, 32, false, false, false, false, false, false);


			for(ui8x = MAX_DIGITS - gDigitCount; ui8x < MAX_DIGITS; ui8x++)
			{
				switch(ui8x)
				{
					case PARAM_ONE_MILLIONS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.oneMillions > 0))
						{
							gsValue.oneMillions--;
						}

						usnprintf(lBuff, 2, "%u", gsValue.oneMillions);
					//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//							usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_HUNDRED_THOUSANDS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.hundredThousands > 0))
						{
							gsValue.hundredThousands--;
						}

						usnprintf(lBuff, 2, "%u", gsValue.hundredThousands);
						//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//							usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_TEN_THOUSANDS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.tenThousands > 0))
						{
							gsValue.tenThousands--;
						}

						usnprintf(lBuff, 2, "%u", gsValue.tenThousands);
						//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//							usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_THOUSANDS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.thousands > 0))
						{
							gsValue.thousands--;
						}

						usnprintf(lBuff, 2, "%u", gsValue.thousands);
						//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//							usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_HUNDREDS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.hundreds > 0))
						{
							gsValue.hundreds--;
						}

						usnprintf(lBuff, 2, "%u", gsValue.hundreds);
					//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//							usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_TENS:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.tens > 0))
						{
							gsValue.tens--;
						}

						usnprintf(lBuff, 2, "%u", gsValue.tens);
					//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
							//						usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case PARAM_ONES:
					{
						if((ui8x == gValueTypeItemFocusIndex)&&(gsValue.ones > 0))
						{
							gsValue.ones--;
						}

						usnprintf(lBuff, 2, "%u", gsValue.ones);
						//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
							//						usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}
				}

				if(ui8x == gValueTypeItemFocusIndex)
					//lui8ReturnValue = displayText((unsigned char *)lBuff_cyw, lui8ReturnValue, 16, true, true, false, false, false, false);
					lui8ReturnValue = displayText((unsigned char *)lBuff, lui8ReturnValue, 16, true, true, false, false, false, false);

					else
					//lui8ReturnValue = displayText((unsigned char *)lBuff_cyw, lui8ReturnValue, 16, false, false, false, false, false, false);
						lui8ReturnValue = displayText((unsigned char *)lBuff, lui8ReturnValue, 16, false, false, false, false, false, false);

			}

			if(0 != gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString)
			{
				usnprintf(lBuff, 21, " %s", gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString);
			//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
			//								usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
				displayText((unsigned char *)lBuff, lui8ReturnValue , 16, false, false, false, false, false, false);
			}
    	}
    }

    return 0;
}

/******************************************************************************
 * FunctionName: valueTypeParamMode
 *
 * Function Description:
 * This function shifts the focus on digits from right to left. If mode button is
 * pressed on the leftmost digit, then active functional block changes to the parent
 * of the current functional block.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t valueTypeParamMode()
{
	uint8_t ui8x = 0;
	char lBuff[21];
    const char Tp_blank[17]="   ";
	int lui8ReturnValue = 2;

    if(gKeysStatus.bits.Key_Mode_pressed)
    {
    	gKeysStatus.bits.Key_Mode_pressed = 0;

		//
		// Decrement value type item focus index
		//
		if((MAX_DIGITS - gDigitCount -1) != gValueTypeItemFocusIndex)
			gValueTypeItemFocusIndex--;

		if((MAX_DIGITS - gDigitCount -1) == gValueTypeItemFocusIndex)
		{
			//
			// Reset parameter setting started flag
			//
			gValueParamSetStarted = 0;

    		//
    		// Reset Get parameter started flag
    		//
    		gValueParamGetStarted = 0;

			//
			// Return to parameter list functional block.
			//
			psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
			psActiveFunctionalBlock->pfnPaintFirstScreen();

			return 0;
		}
		// displayText((unsigned char *)Tp_blank, 0, 16, false, false, false, false, false, false);


		for(ui8x = MAX_DIGITS - gDigitCount; ui8x < MAX_DIGITS; ui8x++)
		{
			switch(ui8x)
			{
				case PARAM_ONE_MILLIONS:
				{
					usnprintf(lBuff, 2, "%u", gsValue.oneMillions);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//							usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case PARAM_HUNDRED_THOUSANDS:
				{
					usnprintf(lBuff, 2, "%u", gsValue.hundredThousands);
				//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//												usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case PARAM_TEN_THOUSANDS:
				{
					usnprintf(lBuff, 2, "%u", gsValue.tenThousands);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//													usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case PARAM_THOUSANDS:
				{
					usnprintf(lBuff, 2, "%u", gsValue.thousands);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//													usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case PARAM_HUNDREDS:
				{
					usnprintf(lBuff, 2, "%u", gsValue.hundreds);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//													usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case PARAM_TENS:
				{
					usnprintf(lBuff, 2, "%u", gsValue.tens);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//												usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case PARAM_ONES:
				{
					usnprintf(lBuff, 2, "%u", gsValue.ones);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//												usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}
			}





			if(ui8x == gValueTypeItemFocusIndex)
				//lui8ReturnValue = displayText((unsigned char *)lBuff_cyw, lui8ReturnValue, 16, true, true, false, false, false, false);
				lui8ReturnValue = displayText((unsigned char *)lBuff, lui8ReturnValue, 16, true, true, false, false, false, false);

				else
				//lui8ReturnValue = displayText((unsigned char *)lBuff_cyw, lui8ReturnValue, 16, false, false, false, false, false, false);
				lui8ReturnValue = displayText((unsigned char *)lBuff, lui8ReturnValue, 16, false, false, false, false, false, false);

		}

		//
		// Display parameter range information in third line
		//
		memset(lBuff, 0, sizeof(char) * 21);

		if(0 == gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString)
		{
			usnprintf(lBuff, 21, "RANGE:%u-%u", gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.minVal,
					gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.maxVal);
		}
		else
		{
			usnprintf(lBuff, 21, "RANGE:%u-%u %s", gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.minVal,
					gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.maxVal,
					gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString);
		}
	//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
		//												usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
		displayText((unsigned char *)lBuff, 2, 32, false, false, false, false, false, false);
    }

    return 0;
}

/******************************************************************************
 * FunctionName: valueTypeParamEnter
 *
 * Function Description:
 * This function shifts the focus on digits from left to right. If enter button is
 * pressed on the rightmost digit, then set parameter command is initiated and active
 * functional block changes to the parent of the current functional block after setting
 * parameter set response.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t valueTypeParamEnter()
{
	uint8_t ui8x = 0;
	char lBuff[21];
	char cccccc[5]={0};

	static uint8_t lsDelay3SecStart = 0;
	static uint32_t lsTickCount3Seconds = 0;

	int lui8ReturnValue = 2;

    if(gKeysStatus.bits.Key_Enter_pressed)
    {
    	gKeysStatus.bits.Key_Enter_pressed = 0;

    	if( (0 == gValueParamSetStarted) &&
//    	    (1 != gstDisplayBoardFault.bits.displayCommunication)
    		(false == gsParamDatabase[gHighlightedItemIndex].readOnly)
    	  )
    	{
			//
			// Increment value type item focus index
			//
			if( (gValueTypeItemFocusIndex < MAX_DIGITS) || (gValueTypeItemFocusIndex == 0xFF) )
				gValueTypeItemFocusIndex++;

#if 1
			if( (MAX_DIGITS == gValueTypeItemFocusIndex) &&
				( (1 != gstDriveBoardStatus.bits.shutterStopped) //||
				 // ( (1 != gstDriveBoardStatus.bits.shutterUpperLimit) && (1 != gstDriveBoardStatus.bits.shutterLowerLimit) )
				)
			  )	// Condition for restricting set parameter command
			{
				//
				// Clear Screen
				//
				//GrRectFIllBolymin(0, 126, 0, 63, true, true);
				GrRectFIllBolymin(0, 127, 0, 63, true, true);

				//
				// Display parameter information first line
				//
				if(gu8_language == Japanese_IDX)
				{
				memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
			    displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
			    displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);

				//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

				//
				// Display failed message on second line
				//
			//	displayText("PARAMETER SET FAILED", 2, 16, false, false, false, false, false, true);
			    displayText("パラメータセットNG", 2, 16, false, false, false, false, false, false);
				//
				// Paint error reason on fourth line
				//
				//displayText("SHUTTER MOVING", 2, 48, false, false, false, false, false, false);
			    displayText("シャッタードウサチュウ", 2, 48, false, false, false, false, false, false);
				}
				else
				{
				memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
			    displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
			    displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);

				//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

				//
				// Display failed message on second line
				//
				displayText("PARAMETER SET FAILED", 2, 16, false, false, false, false, false, true);

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

				//
				// Set focus index to default value
				//
				gValueTypeItemFocusIndex = 0xFF;
			}

			else if( (MAX_DIGITS == gValueTypeItemFocusIndex) &&
					 (1 == gstDriveBoardStatus.bits.shutterStopped)// &&
					 //( (1 == gstDriveBoardStatus.bits.shutterUpperLimit) || (1 == gstDriveBoardStatus.bits.shutterLowerLimit) )
			       ) // Condition for allowing set parameter command
#endif
			//if(MAX_DIGITS == gValueTypeItemFocusIndex)
			{
				//
				// Set parameter setting started flag
				//
				gValueParamSetStarted = 1;

				//
				// Value to set
				//
				gCurrentParameterValue = (gsValue.oneMillions * 1000000) + (gsValue.hundredThousands * 100000) + (gsValue.tenThousands * 10000) +
										 (gsValue.thousands * 1000) + (gsValue.hundreds * 100) + (gsValue.tens * 10) + gsValue.ones;

#if 1			//
				//set value for parameter on display board
				//
				if(eDestDisplayBoard == gsParamDatabase[gHighlightedItemIndex].destination)
				{
					//
					// Set display board parameter
					//
					if( (gCurrentParameterValue >= gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.minVal.ui32Val) &&
										(gCurrentParameterValue <= gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.maxVal.ui32Val)
									  )
					{
					writeParameterUpdateInDB((PARAM_DISP)gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex, (uint8_t *)&gCurrentParameterValue);

					//
					// log change setting history
					//
		    		gstUMtoLM_write.commandRequestStatus = eACTIVE;
		    		gstUMtoLM_write.commandResponseStatus = eNO_STATUS;
		    		gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory = 1;
		    		gstUMtoLM_write.changeSetting.newValue = gCurrentParameterValue;
		    		gstUMtoLM_write.changeSetting.oldValue = gGetParameterValue;
		    		gstUMtoLM_write.changeSetting.parameterNumber = gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex;
		    		gstUMtoLM_write.changeSetting.timeStamp = (HWREG(0x400FC000));
		    		writeChangeSettingsHistory();

		    		//if(gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex == 130)
		    		//{
		    		//	gui8SettingsModeStatus = ACTIVATED;
		    		//	gui8SettingsScreen = ACTIVATED;
		    		//}
					//
					// Paint value type parameter first screen.
					//
					valueTypeParamFirstScreen();

					//cyw  guanbiaofuwei
					gValueTypeItemFocusIndex = 0xFF;

					gCurrentParameterValue = 0;
					gDigitCount = 0;

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

					if(gu8_language == Japanese_IDX)
					{
					  displayText(" カンリョウ", 2, 32, false, false, false, false, false, false);
					}
					else
					{
					displayText("COMPLETED", 2, 32, false, false, false, false, false, true);
					}
					}
					else
					{
						//
					    // Clear Screen
					    //

						GrRectFIllBolymin(0, 127, 0, 63, true, true);

						//
						// Display parameter information first line
						//
						if(gu8_language == Japanese_IDX)
						{
						memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
						displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
						displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);


						//
					    // Display failed message on second line
						//
						//displayText("PARAMETER SET FAILED", 2, 16, false, false, false, false, false, true);
						displayText("パラメータセットNG", 2, 16, false, false, false, false, false, false);
						//
					    // Paint error reason on fourth line
						//
						//displayText("VALIDATION FAILED", 2, 48, false, false, false, false, false, true);
						displayText("カクニンNG", 2, 48, false, false, false, false, false, false);
						}
						else
						{
						memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
						displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
						displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);


						//
					    // Display failed message on second line
						//
						displayText("PARAMETER SET FAILED", 2, 16, false, false, false, false, false, true);

						//
					    // Paint error reason on fourth line
						//
						displayText("VALIDATION FAILED", 2, 48, false, false, false, false, false, true);


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
						// Set focus index to default value
						//
						gValueTypeItemFocusIndex = 0xFF;
					}
		    		return 0;
				}
#endif

				//
				// Validate current parameter's value against minimum and maximum values permitted
				//
				if( (gCurrentParameterValue >= gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.minVal.ui32Val) &&
					(gCurrentParameterValue <= gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.maxVal.ui32Val)
				  )
				{
					//
					// Initiate set parameter command
					//

					if(eINACTIVE == gstUMtoCMdatabase.commandRequestStatus)
					{

						gstUMtoCMdatabase.commandToControlBoard.bits.setParameter = 1;
						gstUMtoCMdatabase.dataToControlBoard.parameterNumber = gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex;
						gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue = gCurrentParameterValue;
						gstUMtoCMdatabase.destination = gsParamDatabase[gHighlightedItemIndex].destination;

						//
						// Set command request status as active
						//
						gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
					}
				}
				else
				{
					//
					// Clear Screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, true, true);

					//
					// Display parameter information first line
					//
					if(gu8_language == Japanese_IDX)
					{
					memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
					displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
					displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);

					//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

					//
					// Display failed message on second line
					//
					//displayText("PARAMETER SET FAILED", 2, 16, false, false, false, false, false, true);
					displayText("パラメータセットNG", 2, 16, false, false, false, false, false, false);
					//
					// Paint error reason on fourth line
					//
					//displayText("VALIDATION FAILED", 2, 48, false, false, false, false, false, true);
					displayText("カクニンNG", 2, 48, false, false, false, false, false, false);
					}
					else
					{
					memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
					displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
					displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);

					//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

					//
					// Display failed message on second line
					//
					displayText("PARAMETER SET FAILED", 2, 16, false, false, false, false, false, true);

					//
					// Paint error reason on fourth line
					//
					displayText("VALIDATION FAILED", 2, 48, false, false, false, false, false, true);


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
					// Set focus index to default value
					//
					gValueTypeItemFocusIndex = 0xFF;
				}
			}
			else
			{
				//
				// Clear line
				//
				//GrRectFIllBolymin(0, 126, 16, 32, true, true);
				GrRectFIllBolymin(0, 127, 16, 32, true, true);

				for(ui8x = MAX_DIGITS - gDigitCount; ui8x < MAX_DIGITS; ui8x++)
				{
					switch(ui8x)
					{
						case PARAM_ONE_MILLIONS:
						{
							usnprintf(lBuff, 2, "%u", gsValue.oneMillions);
						//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//															usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
							break;
						}

						case PARAM_HUNDRED_THOUSANDS:
						{
							usnprintf(lBuff, 2, "%u", gsValue.hundredThousands);
						///memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//															usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
							break;
						}

						case PARAM_TEN_THOUSANDS:
						{
							usnprintf(lBuff, 2, "%u", gsValue.tenThousands);
						//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
							//														usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
							break;
						}

						case PARAM_THOUSANDS:
						{
							usnprintf(lBuff, 2, "%u", gsValue.thousands);
						//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
							//														usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
							break;
						}

						case PARAM_HUNDREDS:
						{
							usnprintf(lBuff, 2, "%u", gsValue.hundreds);
							//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
							//														usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
							break;
						}

						case PARAM_TENS:
						{
							usnprintf(lBuff, 2, "%u", gsValue.tens);
							//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
							//														usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
							break;
						}

						case PARAM_ONES:
						{
							usnprintf(lBuff, 2, "%u", gsValue.ones);
							//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
								//													usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
							break;
						}
					}

					if(ui8x == gValueTypeItemFocusIndex)
						lui8ReturnValue = displayText((unsigned char *)lBuff, lui8ReturnValue, 16, true, true, false, false, false, false);
					else
						lui8ReturnValue = displayText((unsigned char *)lBuff, lui8ReturnValue, 16, false, false, false, false, false, false);
				}

				if(0 != gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString)
				{
					usnprintf(lBuff, 21, " %s", gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
					displayText((unsigned char *)lBuff, lui8ReturnValue , 16, false, false, false, false, false, false);
				}

				//
				// Display parameter range information in third line
				//
				memset(lBuff, 0, sizeof(char) * 21);

				if(0 == gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString)
				{
					usnprintf(lBuff, 21, "RANGE:%u-%u", gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.minVal,
							gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.maxVal);
				}
				else
				{
					usnprintf(lBuff, 21, "RANGE:%u-%u %s", gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.minVal,
							gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.maxVal,
							gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString);
				}
				///memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
				displayText((unsigned char *)lBuff, 2, 32, false, false, false, false, false, false);
			}

    	}
    }

    //
    // Check for set parameter response
    //
    if( (MAX_DIGITS == gValueTypeItemFocusIndex) &&
    	(eACTIVE == gstUMtoCMdatabase.commandRequestStatus) &&
    	(1 == gstUMtoCMdatabase.commandToControlBoard.bits.setParameter)
      )
    {
    	if(eSUCCESS == gstUMtoCMdatabase.commandResponseStatus)
    	{
    		//
    		// Check whether NACK or ACK is received
    		//
    		if(eNACK == gstUMtoCMdatabase.acknowledgementReceived)
    		{
        		//
        		// Clear Screen
        		//
        		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
    			GrRectFIllBolymin(0, 127, 0, 63, true, true);

        		//
        		// Display parameter information first line
        		//
    			if(gu8_language == Japanese_IDX)
    			{
    			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
    											    displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
    											    displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);

        		//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

        		//
        		// Display failed message on second line
        		//
        		//displayText("PARAMETER SET FAILED", 2, 16, false, false, false, false, false, true);
                displayText("パラメータセットNG", 2, 16, false, false, false, false, false, false);
    			}
    			else
    			{memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
			    displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
			    displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);

                //displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

                //
                // Display failed message on second line
                //
                displayText("PARAMETER SET FAILED", 2, 16, false, false, false, false, false, true);


    			}
    		}

    		else if(eACK == gstUMtoCMdatabase.acknowledgementReceived)
    		{
				gstUMtoLM_write.commandRequestStatus = eACTIVE;
				gstUMtoLM_write.commandResponseStatus = eNO_STATUS;
				gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory = 1;
				gstUMtoLM_write.changeSetting.newValue = gCurrentParameterValue;
				gstUMtoLM_write.changeSetting.oldValue = gGetParameterValue;
				gstUMtoLM_write.changeSetting.parameterNumber = gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex;
				gstUMtoLM_write.changeSetting.timeStamp = (HWREG(0x400FC000));
				writeChangeSettingsHistory();

				//
				// Update display board operation count on screen if A028 - operation count input is changed
				//
				//if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == PARAM_OP_CNT_INPUT)
				//{
				//	ui32OperationCount = gCurrentParameterValue;
				//}

				//
				// Clear Screen
				//
				//GrRectFIllBolymin(0, 126, 0, 63, true, true);
				GrRectFIllBolymin(0, 127, 0, 63, true, true);

				//
				// Display parameter information first line
				//
				if(gu8_language == Japanese_IDX)
				{
				memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
												    displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
												    displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);
				}
				else
				{
				memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
			    displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
			    displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);


				}

				//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

				//
				// Display parameter value and unit in second line
				//
				if(0 == gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString)
				{
					usnprintf(lBuff, 21, "%d", gCurrentParameterValue);
				}
				else
				{
					usnprintf(lBuff, 21, "%d %s", gCurrentParameterValue, gsParamDatabase[gHighlightedItemIndex].valueTypeEntities.pUnitString);
				}
				//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//usnprintf_nU_cyw((char*)(lBuff_cyw),strlen(lBuff),(char*)(lBuff));
				displayText((unsigned char *)lBuff, 2, 16, false, false, false, false, false, false);

				//
				// Display "completed" message on third line
				//
				//displayText(" COMPLETED", 2, 32, false, false, false, false, false, false);
				if(gu8_language == Japanese_IDX)
				displayText(" カンリョウ", 2, 32, false, false, false, false, false, false);
				else
			    displayText(" COMPLETED", 2, 32, false, false, false, false, false, true);
    		}

    		gCurrentParameterValue = 0;
			gDigitCount = 0;

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

    		//
    		// Set focus index to default value
    		//
    		gValueTypeItemFocusIndex = 0xFF;
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
			GrRectFIllBolymin(0, 127, 0, 63, true, true);

    		//
    		// Display parameter information first line
    		//
			if(gu8_language == Japanese_IDX)
			{
			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese,4);
											    displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
											    displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_japanese+5, PARA_START+4, 0, false, false, false, true, true, false);
			}
			else
			{
			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english,4);
		    displayText((unsigned char *)cccccc, 1, 0, false, false, false, true, false, true);
		    displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName_english+5, PARA_START+4, 0, false, false, false, true, false, true);


			}
			//displayText((unsigned char *)gsParamDatabase[gHighlightedItemIndex].paramName, 2, 0, false, false, false, false, false, false);

    		//
    		// Display failed message on second line
    		//
    		//displayText("PARAMETER SET FAILED", 2, 16, false, false, false, false, false, true);
			displayText("パラメータセットNG", 2, 16, false, false, false, false, false, false);
    		//
    		// Paint error reason on fourth line
    		//
    		//displayText("COMM FAILURE", 2, 48, false, false, false, false, false, false);
			displayText("ツウシンエラー", 2, 48, false, false, false, false, false, false);

    		gCurrentParameterValue = 0;
    		gDigitCount = 0;

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

    		//
    		// Set focus index to default value
    		//
    		gValueTypeItemFocusIndex = 0xFF;
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
		//psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
		psActiveFunctionalBlock->pfnPaintFirstScreen();

		//
		// Reset parameter setting started flag
		//
		gValueParamSetStarted = 0;

		//
		// Reset Get parameter started flag
		//
		gValueParamGetStarted = 0;

		//
		// Reset delay start flag
		//
		lsDelay3SecStart = 0;
	}

    return 0;
}

/******************************************************************************
 * Define Value type parameter Functional Block
*********************************************************************************/
stInternalFunctions gsValueTypeParamFunctionalBlock =
{
	&gsParameterListFunctionalBlock,
	0,
	valueTypeParamFirstScreen,
	valueTypeParamRunTime,
	valueTypeParamUp,
	valueTypeParamDown,
	valueTypeParamMode,
	valueTypeParamEnter
};
