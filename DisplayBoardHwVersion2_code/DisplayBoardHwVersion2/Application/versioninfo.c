/*********************************************************************************
 * FileName: versioninfo.c
 * Description: Code for Version Info display screen
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
 *  	0.1D	26/06/2014      	iGATE Offshore team       Initial Creation
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
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "userinterface.h"
//#include "sdcardlogs.h"
#include "Middleware/paramdatabase.h"
//#include "logger.h"
#include "Middleware/sdcard.h"
//#include "Drivers/systicktimer.h"
/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
extern uint32_t gControlHardwareVersion;
extern uint32_t gControlFirmwareVersion;
extern uint32_t gDriveHardwareVersion;
extern uint32_t gDriveFirmwareVersion;
extern uint32_t gDisplayHardwareVersion;
extern uint32_t gDisplayFirmwareVersion;

extern const uint8_t display_fw[][3];
extern const uint8_t control_fw[][3];
extern const uint8_t drive_fw[][3];

uint32_t *ptrBoardVersions[6] = {
		&gControlHardwareVersion,
		&gControlFirmwareVersion,
		&gDisplayHardwareVersion,
		&gDisplayFirmwareVersion,
		&gDriveHardwareVersion,
		&gDriveFirmwareVersion
};

#define _IDX_CTRLVER 0
#define _IDX_DISPVER 1
#define _IDX_DRVVER 2

const char s1_japanese[] = " Cユニット";
const char s2_japanese[] = " Dユニット";
const char s3_japanese[] = " Mユニット";
const char s1_english[] = " CONTROL BOARD";
const char s2_english[] = " DISPLAY BOARD";
const char s3_english[] = " DRIVE BOARD";
const char* const strtblVerInfo_japanese[3] = {s1_japanese, s2_japanese, s3_japanese};
const char* const strtblVerInfo_english[3] = {s1_english, s2_english, s3_english};
const char uu_number[]={2,2,5};
/****************************************************************************/
uint8_t VerInfoPaintprocess(uint8_t stridx);
/******************************************************************************
 * FunctionName: paramResetRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t VerInfoRunTime(void)
{
	updateFaultLEDStatus();

	return 0;
}
/******************************************************************************
 * FunctionName: paramResetPaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t ctrlVerInfoPaint(void)
{
	VerInfoPaintprocess(_IDX_CTRLVER);
	return 0;
}

uint8_t dispVerInfoPaint(void)
{
	VerInfoPaintprocess(_IDX_DISPVER);
	return 0;
}

uint8_t drvVerInfoPaint(void)
{
	VerInfoPaintprocess(_IDX_DRVVER);
	return 0;
}
/******************************************************************************
 * FunctionName: paramResetMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t VerInfoMode(void)
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

		psActiveFunctionalBlock = &gsMenuFunctionalBlock;
		psActiveFunctionalBlock->pfnPaintFirstScreen();

    }

	return 0;
}

/******************************************************************************
 * FunctionName: paramResetEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t VerInfoEnter(void)
{
	return 0;
}

/******************************************************************************
 * FunctionName: paramResetUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t VerInfoUp(void)
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

	}

	return 0;
}

/******************************************************************************
 * FunctionName: paramResetDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t VerInfoDown(void)
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
stInternalFunctions gsCtrlVerInfoFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	ctrlVerInfoPaint,
	VerInfoRunTime,
	VerInfoUp,
	VerInfoDown,
	VerInfoMode,
	VerInfoEnter
};

stInternalFunctions gsDispVerInfoFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	dispVerInfoPaint,
	VerInfoRunTime,
	VerInfoUp,
	VerInfoDown,
	VerInfoMode,
	VerInfoEnter
};

stInternalFunctions gsDrvVerInfoFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	drvVerInfoPaint,
	VerInfoRunTime,
	VerInfoUp,
	VerInfoDown,
	VerInfoMode,
	VerInfoEnter
};

uint8_t VerInfoPaintprocess(uint8_t stridx)
{
	unsigned char viewbuff[MAX_CHARS_IN_LINE];
	uBoardVersion luBoardVer;

	// Clear Screen.
	GrRectFIllBolymin(0, 126, 0, 63, true, true);
	if(gu8_language == Japanese_IDX)
	usnprintf((char *)viewbuff, sizeof(viewbuff), "%s", strtblVerInfo_japanese[stridx]);
	else
	usnprintf((char *)viewbuff, sizeof(viewbuff), "%s", strtblVerInfo_english[stridx]);

	if(gu8_language == Japanese_IDX)
	displayText(viewbuff, 2, 0, false, false, false, false, false, false);
	else
	displayText(viewbuff, 2, 0, false, false, false, false, false, true);

	memset(&luBoardVer, 0, sizeof(uBoardVersion));
	luBoardVer.ui32VersionWord = *ptrBoardVersions[stridx*2];
	// usnprintf((char *)viewbuff, sizeof(viewbuff), "HW VERSION: %04u", *ptrBoardVersions[stridx*2]);
	if(gu8_language == Japanese_IDX)
	usnprintf((char *)viewbuff, sizeof(viewbuff), "HWバージョン:%u", luBoardVer.ui8VersionBytes[0] );
	else
	usnprintf((char *)viewbuff, sizeof(viewbuff), "HW VERSION:%u", luBoardVer.ui8VersionBytes[0] );

	if(gu8_language == Japanese_IDX)
	displayText(viewbuff, 2, 16, false, false, false, false, false, false);
	else
	displayText(viewbuff, 2, 16, false, false, false, false, false, true);

	memset(&luBoardVer, 0, sizeof(uBoardVersion));
	luBoardVer.ui32VersionWord = *ptrBoardVersions[stridx*2 + 1];
	//usnprintf((char *)viewbuff, sizeof(viewbuff), "FW VERSION: %04u", *ptrBoardVersions[stridx*2 + 1]);
	//usnprintf((char *)viewbuff, sizeof(viewbuff), "FW VERSION: 1.%u", luBoardVer.ui8VersionBytes[0] );
	 switch(stridx)
	 {
	 case _IDX_CTRLVER:
		 if(gu8_language == Japanese_IDX)
			 usnprintf((char *)viewbuff, sizeof(viewbuff), "FWバージョン:%u%u%u.%u", control_fw[luBoardVer.ui8VersionBytes[1]][0],control_fw[luBoardVer.ui8VersionBytes[1]][1],control_fw[luBoardVer.ui8VersionBytes[1]][2],luBoardVer.ui8VersionBytes[0] );
		 else
			 usnprintf((char *)viewbuff, sizeof(viewbuff), "FW VERSION:%u%u%u.%u", control_fw[luBoardVer.ui8VersionBytes[1]][0],control_fw[luBoardVer.ui8VersionBytes[1]][1],control_fw[luBoardVer.ui8VersionBytes[1]][2],luBoardVer.ui8VersionBytes[0] );
		 break;
	 case _IDX_DISPVER:
		if(gu8_language == Japanese_IDX)
			usnprintf((char *)viewbuff, sizeof(viewbuff), "FWバージョン:%u%u%u.%u", display_fw[luBoardVer.ui8VersionBytes[1]][0],display_fw[luBoardVer.ui8VersionBytes[1]][1],display_fw[luBoardVer.ui8VersionBytes[1]][2],luBoardVer.ui8VersionBytes[0] );
		else
			usnprintf((char *)viewbuff, sizeof(viewbuff), "FW VERSION:%u%u%u.%u", display_fw[luBoardVer.ui8VersionBytes[1]][0],display_fw[luBoardVer.ui8VersionBytes[1]][1],display_fw[luBoardVer.ui8VersionBytes[1]][2],luBoardVer.ui8VersionBytes[0] );
		break;
	 case _IDX_DRVVER:
		if(gu8_language == Japanese_IDX)
			usnprintf((char *)viewbuff, sizeof(viewbuff), "FWバージョン:%u%u%u.%u", drive_fw[luBoardVer.ui8VersionBytes[1]][0],drive_fw[luBoardVer.ui8VersionBytes[1]][1],drive_fw[luBoardVer.ui8VersionBytes[1]][2],luBoardVer.ui8VersionBytes[0] );
		else
			usnprintf((char *)viewbuff, sizeof(viewbuff), "FW VERSION:%u%u%u.%u",drive_fw[luBoardVer.ui8VersionBytes[1]][0],drive_fw[luBoardVer.ui8VersionBytes[1]][1],drive_fw[luBoardVer.ui8VersionBytes[1]][2],luBoardVer.ui8VersionBytes[0]  );
		break;
	 default:break;
	 }
	//displayText(viewbuff, 2, 32, false, false, false, false, false, false);
	//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));

	//usnprintf_UU_cyw(lBuff_cyw,uu_number[stridx],(char *)viewbuff);
	if(gu8_language == Japanese_IDX)
    displayText((unsigned char*)viewbuff, 2, 32, false, false, false, false, false, false);
	else
    displayText((unsigned char*)viewbuff, 2, 32, false, false, false, false, false, true);

	return 0;
}
