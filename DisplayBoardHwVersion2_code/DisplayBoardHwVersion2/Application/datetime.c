/*********************************************************************************
 * FileName: datetime.c
 * Description:
 * This source file contains the definitions of date and time operations. Functions
 * defined here are used when active functional block is Date and time functional
 * block. It also defines date and time functional block object.
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
 *  	0.1D	16/04/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <driverlib/gpio.h>
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "Middleware/rtc.h"
#include "grlib/grlib.h"
//#include "Middleware/cfal96x64x16.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "userinterface.h"
#include "Middleware/paramdatabase.h"

/****************************************************************************
 *  Macro definitions:
****************************************************************************/

#define FIRST_LINE_Y_POS		0
#define	SECOND_LINE_Y_POS		16
#define THIRD_LINE_Y_POS		32
#define FOURTH_LINE_Y_POS		48

/****************************************************************************
 *  Date and Time items Focus Index
****************************************************************************/
#define	YEAR_THOUSANDS			0
#define	YEAR_HUNDREDS			1
#define	YEAR_TENS				2
#define	YEAR_ONES				3
#define	MONTH					4
#define	DAY						5
#define	HOURS					7
#define	MINUTES					8
#define TWELVE_HOUR_CLOCK		6

#define TOTAL_DATE_TIME_ITEMS	9

/****************************************************************************
 *  Months in a year
****************************************************************************/
#define JAN		0
#define FEB		1
#define MAR		2
#define APR		3
#define MAY		4
#define JUN		5
#define JUL		6
#define AUG		7
#define SEP		8
#define OCT		9
#define NOV		10
#define DEC		11

/***************************************************************************/

/****************************************************************************
 *  Structures
****************************************************************************/

/*****************************************************************************
 * Structure Name: stYear
 *
 * Structure Description:
 * This structure is used to store individual digits of year value
 *
 * Structure Members:
 * thousands	:	stores thousands digit of year value
 * hundreds		:	stores hundreds digit of year value
 * tens			:	stores tens digit of year value
 * ones			:	stores ones digit of year value
 *
 ****************************************************************************/
typedef struct _stYear
{
	uint8_t thousands;
	uint8_t hundreds;
	uint8_t tens;
	uint8_t ones;
} stYear;

/****************************************************************************
 *  Enumerations
****************************************************************************/
enum _ERROR_DATE_TIME
{
	errINVALID_DATE = 1,
	errYEAR_OUT_OF_BOUNDS
};

enum TwelveHourClock
{
	eAM,
	ePM
} twelveHourClock;

/***************************************************************************/

/****************************************************************************
 *  Global variables
****************************************************************************/

//**************************************************************************
// Months in a year
//**************************************************************************
unsigned char gMonths[12][3] =
{
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
	"JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};

//**************************************************************************
// Date and time display item in focus
//**************************************************************************
uint8_t gui8DateTimeItemFocusIndex = 0xFF;

//**************************************************************************
// Date and time object
//**************************************************************************
struct tm gsDateTime;

//**************************************************************************
// Hours value for 12 hour clock
//**************************************************************************
uint8_t gui8Hours = 0;

//**************************************************************************
// Year individual digits object
//**************************************************************************
stYear gsYear;

//**************************************************************************
// Date validity flag
//**************************************************************************
uint8_t gui8DateInvalid = 0;


#define DATEANDTIME_start_cyw 2
/*****************************************************************************/

/******************************************************************************
 * FunctionName: dateTimeValidation
 *
 * Function Description:
 * This function validates a date value
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Correct date
 * 1	:	Incorrect date
 *
 ********************************************************************************/
uint8_t dateValidation(void)
{
	uint8_t ui8NotLeapYear = 0;

	uint32_t lui32Year;
	lui32Year = (gsYear.thousands * 1000) + (gsYear.hundreds * 100) + (gsYear.tens * 10) + gsYear.ones;

	if( ( lui32Year < 1970 ) || (lui32Year > 2037) )
		return errYEAR_OUT_OF_BOUNDS;

	if( (gsDateTime.tm_mday == 31) && (
		(gsDateTime.tm_mon == FEB) ||
		(gsDateTime.tm_mon == APR) ||
		(gsDateTime.tm_mon == JUN) ||
		(gsDateTime.tm_mon == SEP) ||
		(gsDateTime.tm_mon == NOV)    )
	  )
		return errINVALID_DATE;

	if( (gsDateTime.tm_mday == 30) && (gsDateTime.tm_mon == FEB) )
		return errINVALID_DATE;

	//
	// Check for leap year
	//
	if( (lui32Year % 400 == 0) || ( (lui32Year % 4 == 0) && (lui32Year % 100 != 0) ) )
		ui8NotLeapYear = 0;
	else
		ui8NotLeapYear = 1;

	if( (gsDateTime.tm_mday == 29) && (gsDateTime.tm_mon == FEB) && (ui8NotLeapYear == 1) )
		return errINVALID_DATE;

	return 0;
}

/******************************************************************************
 * FunctionName: dateTimeFirstScreen
 *
 * Function Description:
 * This function reads current date and time and displays on screen.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
uint8_t dateTimeFirstScreen()
{
	uint8_t ui8x;
	unsigned char lBuff[4];
	uint32_t lui32Year;
	//uint8_t lui8ReturnValue = 0;
	uint8_t lui8ReturnValue = DATEANDTIME_start_cyw;
	uint8_t lui8LineYPos = FIRST_LINE_Y_POS;

	//
	// Clear Screen
	//
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

	//
	// Display functional block title on first line
	//

	if(gu8_language == Japanese_IDX)
	{
	displayText("ジカン_セッテイ", 2, lui8LineYPos, false, false, false, false, false, false);
	}
	else
	{
    displayText("DATE AND TIME", 2, lui8LineYPos, false, false, false, false,false,true);
	}
	//
	// Get current date and time
	//
	RTCGet(&gsDateTime);

	lui32Year = gsDateTime.tm_year + 1900;

	//
	// Get digits of year value
	//
	gsYear.thousands = lui32Year/1000;
	lui32Year %= 1000;
	gsYear.hundreds = lui32Year/100;
	lui32Year %= 100;
	gsYear.tens = lui32Year/10;
	lui32Year %= 10;
	gsYear.ones = lui32Year;

	//
	// Set AM or PM w.r.t. current time
	//
	if(gsDateTime.tm_hour > 11)
		twelveHourClock = ePM;
	else
		twelveHourClock = eAM;

	//
	// Get hours value to be displayed on screen. Hours value displayed on screen can
	// take values only between 1 to 12  for a twelve hour clock.
	//
	gui8Hours = gsDateTime.tm_hour;
	if(gui8Hours > 12)
		gui8Hours = gui8Hours - 12;
	else if(gui8Hours == 0)
		gui8Hours = gui8Hours + 12;	// 0000 hrs as 12 midnight

	//
	// Switch to second line on display.
	//
	lui8LineYPos = SECOND_LINE_Y_POS;

	//
	// Paint screen with current date and time
	//
	for(ui8x = 0; ui8x < TOTAL_DATE_TIME_ITEMS; ui8x++)
	{
		switch(ui8x)
		{
			case DAY:
			{
				//
				// Add separator
				//
				lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

				usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_mday);
				//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
				break;
			}

			case MONTH:
			{
				//
				// Add separator
				//
				lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

				//usnprintf((char*)lBuff, sizeof(lBuff), "%s", &gMonths[gsDateTime.tm_mon]);
				usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_mon + 1);
				//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
				break;
			}

			case YEAR_THOUSANDS:
			{
				//
				// Add separator
				//
				//lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false);

				usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.thousands);
				//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//				usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
				break;
			}

			case YEAR_HUNDREDS:
			{
				usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.hundreds);
				//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//								usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
				break;
			}

			case YEAR_TENS:
			{
				usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.tens);
				//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//								usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
				break;
			}

			case YEAR_ONES:
			{
				usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.ones);
			//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//								usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
				break;
			}

			case HOURS:
			{
				usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gui8Hours);
				//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//								usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
				lui8ReturnValue += 6;
				//lui8LineYPos = THIRD_LINE_Y_POS;
				break;
			}

			case MINUTES:
			{
				//
				// Add separator
				//
				lui8ReturnValue = displayText(":", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

				usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_min);
				//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//								usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
				break;
			}

			case TWELVE_HOUR_CLOCK:
			{
				lui8ReturnValue += 6;

				if(twelveHourClock == ePM)
					memcpy(lBuff, "PM", sizeof(lBuff));
				else
					memcpy(lBuff, "AM", sizeof(lBuff));

				break;
			}
		}

		//
		// Display a date and time item on screen at position given by value returned from
		// below function.
		//
		lui8ReturnValue = displayText(lBuff, lui8ReturnValue, lui8LineYPos, false, false, false, false, false, true);
	}

	return 0;
}

/*******************************************************************************
 * FunctionName: dateTimeRunTime
 *
 * Function Description:
 * Function to be called from main function so as to read current date and time
 * periodically.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
uint8_t dateTimeRunTime()
{
	if( ( psActiveFunctionalBlock ==  &gsDateTimeFunctionalBlock ) &&
		( gui8DateTimeItemFocusIndex == 0xFF ) &&
		( gui8DateInvalid == 0 )
	  )
	{
		dateTimeFirstScreen();
	}

	updateFaultLEDStatus();

	return 0;
}

/******************************************************************************
 * FunctionName: dateTimeEnter
 *
 * Function Description:
 * This function moves highlight from one date and time display item to another
 * from left to right direction. If this function is called when the highlight
 * is on rightmost item, then it will save the edited date and time values.
 * Also parent menu is displayed on screen and active functional block will be
 * parent of current functional block.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
uint8_t dateTimeEnter()
{
	uint8_t ui8x;
	unsigned char lBuff[4];
	//int lui8ReturnValue = 0;
	int lui8ReturnValue = DATEANDTIME_start_cyw;
	uint8_t lui8LineYPos = FIRST_LINE_Y_POS;

#if 1	// Variables used to generate delays for date and time messages
	static uint8_t lsui8Delay3SecStart = 0;
	static uint32_t lsui32TickCount3Seconds = 0;
#endif

    if(gKeysStatus.bits.Key_Enter_pressed)
    {
    	gKeysStatus.bits.Key_Enter_pressed = 0;

    	if(0 == lsui8Delay3SecStart)
    	{
			//
			// Increment date time focus index
			//
			if( (gui8DateTimeItemFocusIndex < TOTAL_DATE_TIME_ITEMS) || (gui8DateTimeItemFocusIndex == 0xFF) )
				gui8DateTimeItemFocusIndex++;

			//
			// Check for Enter key press on last date time display item so as to
			// initiate date and time setting process. Also do date validation and take
			// respective action.
			//
			if(gui8DateTimeItemFocusIndex == TOTAL_DATE_TIME_ITEMS)
			{
				//
				// Reset gui8DateTimeItemFocusIndex
				//
				gui8DateTimeItemFocusIndex = 0xFF;

				//
				// Validate the date entered by the user
				//
				lui8ReturnValue = dateValidation();

				if ( 0 == lui8ReturnValue )
				{
					//
					// Get year field modified by user and decrement by 1900 because tm_year field is the number
					// of years passed since year 1900
					//
					gsDateTime.tm_year = (gsYear.thousands * 1000) + (gsYear.hundreds * 100) + (gsYear.tens * 10) + gsYear.ones;
					gsDateTime.tm_year = gsDateTime.tm_year - 1900;

					//
					// Get hours value modified by user
					//
					if(gui8Hours == 12)
						gui8Hours = 0;			// 12 represents 0000 hrs
					if(twelveHourClock == ePM)
						gsDateTime.tm_hour = gui8Hours + 12;	// tm_hour ranges from 0 to 23
					else
						gsDateTime.tm_hour = gui8Hours;

					//
					// Set RTC value with date and time modified by user.
					//
					RTCSet(&gsDateTime);

	#if 1	// Add Date and time set success message
					//
					// Clear Screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
					//
					// Display date and time set completion message
					//

					if(gu8_language == Japanese_IDX)
					{
						displayText("ジカン_セッテイ", 2, FIRST_LINE_Y_POS, false, false, false, false, false, false);
						displayText("セット_OK", 2, SECOND_LINE_Y_POS, false, false, false, false, false, false);
					}
					else
					{
						displayText("DATE AND TIME", 2, FIRST_LINE_Y_POS, false, false, false, false,false,true);
						displayText("SET COMPLETED", 2, SECOND_LINE_Y_POS, false, false, false, false,false,true);
					}


					//
					// Start 3 seconds delay
					//
					lsui8Delay3SecStart = 1;
					lsui32TickCount3Seconds = g_ui32TickCount;
	#endif

	#if 0
					//
					// return back to menu functional block
					//
					psActiveFunctionalBlock = &gsMenuFunctionalBlock;
					psActiveFunctionalBlock->pfnPaintFirstScreen();

	#endif
				}

				else if( errINVALID_DATE == lui8ReturnValue )
				{
					//
					// Set invalid date flag
					//
					gui8DateInvalid = 1;

					//
					// Clear Screen
					//
				//	GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

					//
					// Display functional block title on first line
					//

					//
					// Display Error message
					//

					if(gu8_language == Japanese_IDX)
					{
						displayText("ジカン_セッテイ", 2, FIRST_LINE_Y_POS, false, false, false, false, false, false);
						displayText("ムコウ", 2, SECOND_LINE_Y_POS, false, false, false, false, false, false);
					}
					else
					{
						displayText("DATE AND TIME", 2, FIRST_LINE_Y_POS, false, false, false, false,false,true);
						displayText("INVALID DATE", 2, SECOND_LINE_Y_POS, false, false, false, false,false,true);
					}







	#if 1	// Start 3 seconds delay
					lsui8Delay3SecStart = 1;
					lsui32TickCount3Seconds = g_ui32TickCount;
	#endif

				}

				else if( errYEAR_OUT_OF_BOUNDS == lui8ReturnValue )
				{
					//
					// Set invalid date flag
					//
					gui8DateInvalid = 1;

					//
					// Clear Screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);




					//
					// Display functional block title on first line
					//

					if(gu8_language == Japanese_IDX)
					{

						displayText("ジカン_セッテイ", 2, FIRST_LINE_Y_POS, false, false, false, false, false, false);
						displayText("ムコウ", 2, SECOND_LINE_Y_POS, false, false, false, false, false, false);
					}
					else
					{
						displayText("DATE AND TIME", 2, FIRST_LINE_Y_POS, false, false, false, false,false,true);
						displayText("YEAR NOT IN RANGE", 2, SECOND_LINE_Y_POS, false, false, false, false,false,true);
					}



					//
					// Display Error message
					//





					displayText("RANGE: 1970-2037", 1, THIRD_LINE_Y_POS, false, false, false, false,true,false);

	#if 1	// Start 3 seconds delay
					lsui8Delay3SecStart = 1;
					lsui32TickCount3Seconds = g_ui32TickCount;
	#endif
				}

				return 0;
			}

			//
			// Clear Screen
			//
			//GrRectFIllBolymin(0, 126, 0, 63, true, true);
			GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

			//
			// Display functional block title on first line
			//
			//displayText("DATE AND TIME", 2, FIRST_LINE_Y_POS, false, false, false, false);
			if(gu8_language == Japanese_IDX)
			{
			displayText("ジカン_セッテイ", 2, FIRST_LINE_Y_POS, false, false, false, false, false, false);
			}
			else
			{
		    displayText("DATE AND TIME", 2, FIRST_LINE_Y_POS, false, false, false, false,false,true);
			}
			//
			// Switch to second line on display.
			//
			lui8LineYPos = SECOND_LINE_Y_POS;

			//
			// Paint screen with current date and time
			//
			for(ui8x = 0; ui8x < TOTAL_DATE_TIME_ITEMS; ui8x++)
			{
				switch(ui8x)
				{
					case DAY:
					{
						//
						// Add separator
						//
						lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

						usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_mday);
						//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//			usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
						break;
					}

					case MONTH:
					{
						//
						// Add separator
						//
						lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

						//usnprintf((char*)lBuff, sizeof(lBuff), "%s", &gMonths[gsDateTime.tm_mon]);
						usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_mon + 1);
						//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//			usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
						break;
					}

					case YEAR_THOUSANDS:
					{
						//
						// Add separator
						//
						//lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false);

						usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.thousands);
						//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//			usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case YEAR_HUNDREDS:
					{
						usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.hundreds);
						//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//			usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case YEAR_TENS:
					{
						usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.tens);
					//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//			usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case YEAR_ONES:
					{
						usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.ones);
						//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
							//		usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
						break;
					}

					case HOURS:
					{
						usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gui8Hours);
					//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
						//			usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
						lui8ReturnValue += 6;
						//lui8LineYPos = THIRD_LINE_Y_POS;
						break;
					}

					case MINUTES:
					{
						//
						// Add separator
						//
						lui8ReturnValue = displayText(":", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

						usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_min);
					//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//				usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
						break;
					}

					case TWELVE_HOUR_CLOCK:
					{
						lui8ReturnValue += 6;

						if(twelveHourClock == ePM)
							memcpy(lBuff, "PM", sizeof(lBuff));
						else
							memcpy(lBuff, "AM", sizeof(lBuff));

						break;
					}
				}

				//
				// Display a date and time item on screen at position given by value returned from
				// displayText() function. Item having the current focus will be highlighted.
				//
				if(ui8x == gui8DateTimeItemFocusIndex)
					lui8ReturnValue = displayText(lBuff, lui8ReturnValue, lui8LineYPos, true, true, false, false, false, true);
				else
					lui8ReturnValue = displayText(lBuff, lui8ReturnValue, lui8LineYPos, false, false, false, false, false, true);
			}

    	}
    }

#if 1
    //
    // Check whether 3 seconds delay is over
    //
    if( (get_timego(lsui32TickCount3Seconds) >= 300 ) &&
    	(1 == lsui8Delay3SecStart)
      )
    {
    	psActiveFunctionalBlock->pfnPaintFirstScreen();
    	lsui8Delay3SecStart = 0;
    }

#endif

	return 0;
}

/******************************************************************************
 * FunctionName: dateTimeMode
 *
 * Function Description:
 * This function moves highlight from one date and time display item to another
 * from right to left. When this function is called when highlight is on the
 * leftmost item, then screen will display parent menu and active functional
 * block is set as parent of current functional block. Edited date and time
 * values will not be saved in this case.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
uint8_t dateTimeMode()
{
	uint8_t ui8x;
	unsigned char lBuff[4];
	//int lui8ReturnValue = 0;
	int lui8ReturnValue = DATEANDTIME_start_cyw;

	uint8_t lui8LineYPos = FIRST_LINE_Y_POS;

	//
	// Handle Mode key press
	//
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
    	gKeysStatus.bits.Key_Mode_pressed = 0;

		//
		// Increment date time focus index
		//
		if(gui8DateTimeItemFocusIndex != 0xFF)
			gui8DateTimeItemFocusIndex--;

		//
		// Check for Mode key press on first date time item. Return to menu
		// functional block.
		//
		if(gui8DateTimeItemFocusIndex == 0xFF)
		{
			//
			// Reset invalid Date flag
			//
			if( gui8DateInvalid == 1 )
				gui8DateInvalid = 0;

			//
			// Return to menu functional block.
			//
			psActiveFunctionalBlock = &gsMenuFunctionalBlock;
			psActiveFunctionalBlock->pfnPaintFirstScreen();

			return 0;
		}

		//
		// Clear Screen
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		//
		// Display functional block title on first line
		//
		//displayText("DATE AND TIME", 2, lui8LineYPos, false, false, false, false);
		if(gu8_language == Japanese_IDX)
		{
		displayText("ジカン_セッテイ", 2, lui8LineYPos, false, false, false, false, false, false);
		}
		else
		{
		displayText("DATE AND TIME", 2, lui8LineYPos, false, false, false, false,false,true);
		}
		//
		// Switch to second line on display.
		//
		lui8LineYPos = SECOND_LINE_Y_POS;

		//
		// Paint screen with current date and time
		//
		for(ui8x = 0; ui8x < TOTAL_DATE_TIME_ITEMS; ui8x++)
		{
			switch(ui8x)
			{
				case DAY:
				{
					//
					// Add separator
					//
					lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_mday);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//				usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
					break;
				}

				case MONTH:
				{
					//
					// Add separator
					//
					lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

					//usnprintf((char*)lBuff, sizeof(lBuff), "%s", &gMonths[gsDateTime.tm_mon]);
					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_mon + 1);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//				usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));

					break;
				}

				case YEAR_THOUSANDS:
				{
					//
					// Add separator
					//
					//lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false);

					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.thousands);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//				usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case YEAR_HUNDREDS:
				{
					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.hundreds);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//			usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case YEAR_TENS:
				{
					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.tens);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//			usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case YEAR_ONES:
				{
					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.ones);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//				usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case HOURS:
				{
					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gui8Hours);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//			usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));

					lui8ReturnValue += 6;
					//lui8LineYPos = THIRD_LINE_Y_POS;
					break;
				}

				case MINUTES:
				{
					//
					// Add separator
					//
					lui8ReturnValue = displayText(":", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_min);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//			usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
					break;
				}

				case TWELVE_HOUR_CLOCK:
				{
					lui8ReturnValue += 6;

					if(twelveHourClock == ePM)
						memcpy(lBuff, "PM", sizeof(lBuff));
					else
						memcpy(lBuff, "AM", sizeof(lBuff));

					break;
				}
			}

			//
			// Display a date and time item on screen at position given by value returned from
			// displayText() function. Item having the current focus will be highlighted.
			//
			if(ui8x == gui8DateTimeItemFocusIndex)
				lui8ReturnValue = displayText(lBuff, lui8ReturnValue, lui8LineYPos, true, true, false, false, false, true);
			else
				lui8ReturnValue = displayText(lBuff, lui8ReturnValue, lui8LineYPos, false, false, false, false, false, true);
		}
    }

	return 0;
}

/******************************************************************************
 * FunctionName: dateTimeUp
 *
 * Function Description:
 * This function changes the value of highlighted item in incrementing fashion.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
uint8_t dateTimeUp()
{
	uint8_t ui8x;
	unsigned char lBuff[4];

	//int lui8ReturnValue = 0;
	int lui8ReturnValue = DATEANDTIME_start_cyw;

	uint8_t lui8LineYPos = FIRST_LINE_Y_POS;

	//
	// Handle Up key press
	//
    if(gKeysStatus.bits.Key_Up_pressed)
    {
    	gKeysStatus.bits.Key_Up_pressed = 0;

		//
		// Clear Screen
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
    	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		//
		// Display functional block title on first line
		//
		//displayText("DATE AND TIME", 2, lui8LineYPos, false, false, false, false);
    	if(gu8_language == Japanese_IDX)
    	{
		displayText("ジカン_セッテイ", 2, lui8LineYPos, false, false, false, false, false, false);
    	}
    	else
    	{
    	displayText("DATE AND TIME", 2, lui8LineYPos, false, false, false, false,false,true);
    	}
		//
		// Switch to second line on display.
		//
		lui8LineYPos = SECOND_LINE_Y_POS;

		//
		// Paint screen with current date and time
		//
		for(ui8x = 0; ui8x < TOTAL_DATE_TIME_ITEMS; ui8x++)
		{

			switch(ui8x)
			{
				case DAY:
				{
					//
					// Add separator
					//
					lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

					//
					// Increment day value if it has the focus
					//
					if((ui8x == gui8DateTimeItemFocusIndex)&&(gsDateTime.tm_mday < 31))
						gsDateTime.tm_mday++;

					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_mday);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//				usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
					break;
				}

				case MONTH:
				{
					//
					// Add separator
					//
					lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

					//
					// Increment month value if it has the focus
					//
					if((ui8x == gui8DateTimeItemFocusIndex)&&(gsDateTime.tm_mon < 11))
						gsDateTime.tm_mon++;

					//usnprintf((char*)lBuff, sizeof(lBuff), "%s", &gMonths[gsDateTime.tm_mon]);
					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_mon + 1);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//				usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
					break;
				}

				case YEAR_THOUSANDS:
				{
					//
					// Add separator
					//
					//lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false);

					//
					// Increment thousands digit of year if it has the focus
					//
					if( (ui8x == gui8DateTimeItemFocusIndex) && (gsYear.thousands < 9) )
					{
						gsYear.thousands++;
					}

					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.thousands);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//				usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case YEAR_HUNDREDS:
				{
					//
					// Increment hundreds digit of year if it has the focus
					//
					if( (ui8x == gui8DateTimeItemFocusIndex) && (gsYear.hundreds < 9) )
					{
						gsYear.hundreds++;
					}

					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.hundreds);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//				usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case YEAR_TENS:
				{
					//
					// Increment tens digit of year if it has the focus
					//
					if( (ui8x == gui8DateTimeItemFocusIndex) && (gsYear.tens < 9) )
					{
						gsYear.tens++;
					}

					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.tens);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//				usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case YEAR_ONES:
				{
					//
					// Increment ones digit of year if it has the focus
					//
					if( (ui8x == gui8DateTimeItemFocusIndex) && (gsYear.ones < 9) )
					{
						gsYear.ones++;
					}

					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.ones);
				////	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//			usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case HOURS:
				{
					//
					// Increment hours if it has the focus
					//
					if((ui8x == gui8DateTimeItemFocusIndex)&&(gui8Hours < 12))
					{
						gui8Hours++;
					}

					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gui8Hours);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//				usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
					lui8ReturnValue += 6;
					//lui8LineYPos = THIRD_LINE_Y_POS;
					break;
				}

				case MINUTES:
				{
					//
					// Increment minutes if it has the focus
					//
					if((ui8x == gui8DateTimeItemFocusIndex)&&(gsDateTime.tm_min < 59))
					{
						gsDateTime.tm_min++;
					}

					//
					// Add separator
					//
					lui8ReturnValue = displayText(":", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_min);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//			usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
					break;
				}

				case TWELVE_HOUR_CLOCK:
				{
					lui8ReturnValue += 6;

					//
					// Change AM to PM or PM to AM, if this item has focus.
					//
					if(ui8x == gui8DateTimeItemFocusIndex)
					{
						if(twelveHourClock == ePM)
							twelveHourClock = eAM;
						else
							twelveHourClock = ePM;
					}

					if(twelveHourClock == ePM)
						memcpy(lBuff, "PM", sizeof(lBuff));
					else
						memcpy(lBuff, "AM", sizeof(lBuff));

					break;
				}
			}

			//
			// Display a date and time item on screen at position given by value returned from
			// displayText() function. Item having the current focus will be highlighted.
			//
			if(ui8x == gui8DateTimeItemFocusIndex)
				lui8ReturnValue = displayText(lBuff, lui8ReturnValue, lui8LineYPos, true, true, false, false, false, true);
			else
				lui8ReturnValue = displayText(lBuff, lui8ReturnValue, lui8LineYPos, false, false, false, false, false, true);
		}
    }
	return 0;
}

/******************************************************************************
 * FunctionName: dateTimeDown
 *
 * Function Description:
 * This function changes the value of highlighted item in decrementing fashion.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
uint8_t dateTimeDown()
{
	uint8_t ui8x;
	unsigned char lBuff[4];
	//int lui8ReturnValue = 0;
	int lui8ReturnValue = DATEANDTIME_start_cyw;
	uint8_t lui8LineYPos = FIRST_LINE_Y_POS;

	//
	// Handle Down key press
	//
    if(gKeysStatus.bits.Key_Down_pressed)
    {
    	gKeysStatus.bits.Key_Down_pressed = 0;

		//
		// Clear Screen
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
    	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
		//
		// Display functional block title on first line
		//


		//displayText("DATE AND TIME", 2, lui8LineYPos, false, false, false, false);
    	if(gu8_language == Japanese_IDX)
    	{
		displayText("ジカン_セッテイ", 2, lui8LineYPos, false, false, false, false, false, false);
    	}
    	else
    	{
    	displayText("DATE AND TIME", 2, lui8LineYPos, false, false, false, false,false,true);
    	}
		//
		// Switch to second line on display.
		//
		lui8LineYPos = SECOND_LINE_Y_POS;

		//
		// Paint screen with current date and time
		//
		for(ui8x = 0; ui8x < TOTAL_DATE_TIME_ITEMS; ui8x++)
		{

			switch(ui8x)
			{
				case DAY:
				{
					//
					// Add separator
					//
					lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

					//
					// Decrement day value if it has the focus.
					//
					if( (ui8x == gui8DateTimeItemFocusIndex) && (gsDateTime.tm_mday > 1) )
						gsDateTime.tm_mday--;

					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_mday);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//								usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
					break;
				}

				case MONTH:
				{
					//
					// Add separator
					//
					lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);

					//
					// Decrement month value if it has the focus.
					//
					if( (ui8x == gui8DateTimeItemFocusIndex) && (gsDateTime.tm_mon > 0) )
						gsDateTime.tm_mon--;

					//usnprintf((char*)lBuff, sizeof(lBuff), "%s", &gMonths[gsDateTime.tm_mon]);
					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_mon + 1);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//								usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
					break;
				}

				case YEAR_THOUSANDS:
				{
					//
					// Add separator
					//
					//lui8ReturnValue = displayText("/", lui8ReturnValue, lui8LineYPos, false, false, false, false);

					//
					// Decrement thousands digit of year if it has the focus.
					//
					if( (ui8x == gui8DateTimeItemFocusIndex) && (gsYear.thousands > 0) )
					{
							gsYear.thousands--;
					}

					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.thousands);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//								usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case YEAR_HUNDREDS:
				{
					//
					// Decrement hundreds digit of year if it has the focus.
					//
					if( (ui8x == gui8DateTimeItemFocusIndex) && (gsYear.hundreds > 0) )
					{
							gsYear.hundreds--;
					}

					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.hundreds);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//								usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case YEAR_TENS:
				{
					//
					// Decrement tens digit of year if it has the focus.
					//
					if( (ui8x == gui8DateTimeItemFocusIndex) && (gsYear.tens > 0) )
					{
							gsYear.tens--;
					}

					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.tens);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//									usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case YEAR_ONES:
				{
					//
					// Decrement ones digit of year if it has the focus.
					//
					if( (ui8x == gui8DateTimeItemFocusIndex) && (gsYear.ones > 0) )
					{
							gsYear.ones--;
					}

					usnprintf((char*)lBuff, sizeof(lBuff), "%u", gsYear.ones);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//									usnprintf_nU_cyw((char*)(lBuff_cyw),1,(char*)(lBuff));
					break;
				}

				case HOURS:
				{
					//
					// Decrement hours if it has the focus.
					//
					if((ui8x == gui8DateTimeItemFocusIndex)&&(gui8Hours > 1))
					{
						gui8Hours--;
					}

					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gui8Hours);
				//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
				//									usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
					lui8ReturnValue += 6;
					//lui8LineYPos = THIRD_LINE_Y_POS;
					break;
				}

				case MINUTES:
				{
					//
					// Decrement minutes if it has the focus.
					//
					if((ui8x == gui8DateTimeItemFocusIndex)&&(gsDateTime.tm_min > 0))
					{
						gsDateTime.tm_min--;
					}

					lui8ReturnValue = displayText(":", lui8ReturnValue, lui8LineYPos, false, false, false, false, false, false);
					usnprintf((char*)lBuff, sizeof(lBuff), "%02u", gsDateTime.tm_min);
					//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
					//								usnprintf_nU_cyw((char*)(lBuff_cyw),2,(char*)(lBuff));
					break;
				}

				case TWELVE_HOUR_CLOCK:
				{
					lui8ReturnValue += 6;

					//
					// Change AM to PM or PM to AM, if this item has focus.
					//
					if(ui8x == gui8DateTimeItemFocusIndex)
					{
						if(twelveHourClock == ePM)
							twelveHourClock = eAM;
						else
							twelveHourClock = ePM;
					}

					if(twelveHourClock == ePM)
					{
						memcpy(lBuff, "PM", sizeof(lBuff));
					}
					else
						memcpy(lBuff, "AM", sizeof(lBuff));

					break;
				}
			}

			//
			// Display a date and time item on screen at position given by value returned from
			// displayText() function. Item having the current focus will be highlighted.
			//
			if(ui8x == gui8DateTimeItemFocusIndex)
				lui8ReturnValue = displayText(lBuff, lui8ReturnValue, lui8LineYPos, true, true, false, false, false, true);
			else
				lui8ReturnValue = displayText(lBuff, lui8ReturnValue, lui8LineYPos, false, false, false, false, false, true);
		}
    }

	return 0;
}


/******************************************************************************
 * Define Date and Time Functional Block
*********************************************************************************/
stInternalFunctions gsDateTimeFunctionalBlock =
{
	&gsMenuFunctionalBlock,
	0,
	dateTimeFirstScreen,
	defaultFunction,
	dateTimeUp,
	dateTimeDown,
	dateTimeMode,
	dateTimeEnter
};
