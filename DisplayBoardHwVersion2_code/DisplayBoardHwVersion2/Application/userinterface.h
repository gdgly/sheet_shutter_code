/*********************************************************************************
* FileName: userinterface.h
* Description:
* This source file contains the types used for user interface creation.
* Version: 0.2D
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
 *  	0.2D	23/06/2014									Added Timeout DEFINES
 *  	0.1D	11/04/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

#ifndef USERINTERFACE_H_
#define USERINTERFACE_H_

/****************************************************************************
 * Include files:
****************************************************************************/
#include <stdbool.h>
#include "intertaskcommunication.h"

/****************************************************************************
 *  Macro definitions:
****************************************************************************/

#define MAX_PACKET_SIZE		10

#define MAX_FILES_FOR_SDPARAM (10+1)
/****************************************************************************
 * Shutter States
****************************************************************************/
#define SHUTTER_STOPPED		0
#define SHUTTER_RUNNING 	1


/****************************************************************************
 * Shutter Modes
****************************************************************************/
#define SHUTTER_IN_AUTO_MODE	1
#define SHUTTER_IN_MANUAL_MODE 	0

/****************************************************************************/

/****************************************************************************
 * Shutter Parameters
****************************************************************************/
#define PARAM_NUM_OPERATION_COUNT		100
#define PARAM_NUM_SHUTTER_STATE		 	101
#define PARAM_NUM_SHUTTER_MODE			102
/****************************************************************************/

/****************************************************************************
 * MENU TIMEOUTS
 * **************************************************************************/
#define _TIMEOUTSYSTICK_15SEC ((15*100)/MS_PER_SYSTICK) //w.r.t. 10ms Systick count
#define _TIMEOUTSYSTICK_5SEC ((5*100)/MS_PER_SYSTICK) //w.r.t. 10ms Systick count
#define _WAITSYSTICK_3SEC ((3*100)/MS_PER_SYSTICK) //w.r.t. 10ms Systick count
#define _WAITSYSTICK(secs) ((secs*100)/MS_PER_SYSTICK) //w.r.t. 10ms Systick count

/****************************************************************************/

extern uint8_t isMenutitle;
/****** Structures and Variables for ***********************/
// Data Structure for Flags for Internal modules
typedef union
{
	uint8_t val;
	struct
	{
		//ParamInitFlags module flag
		uint8_t anomhistDump	: 1;
		uint8_t anomhistErase	: 1;
		uint8_t bit2			: 1;
		uint8_t bit3			: 1;
		uint8_t bit4			: 1;
		uint8_t bit5			: 1;
		uint8_t bit6			: 1;
		uint8_t bit7			: 1;
	} flags;
} _AnomHistFlags;

// Define Flags for Internal modules
extern _AnomHistFlags AnomHistFlags; //Defined in anomhistorydump.c

// Data Structure for Flags for Internal modules
typedef union
{
	uint8_t val;
	struct
	{
		//ParamInitFlags module flag
		uint8_t cshistErase		: 1;
		uint8_t cshistDump		: 1;
		uint8_t bit2			: 1;
		uint8_t bit3			: 1;
		uint8_t bit4			: 1;
		uint8_t bit5			: 1;
		uint8_t bit6			: 1;
		uint8_t bit7			: 1;
	} flags;
} _ChgSetHistFlags;

// Define Flags for Internal modules
extern _ChgSetHistFlags chgSetHistFlags; //Defined in changesettinghistoryerase.c

extern uint8_t gUserModuleState;
/****************************************************************************
 *  Structures:
****************************************************************************/

typedef union _uBoardVersion
{
	uint8_t   ui8VersionBytes[4];
	uint32_t  ui32VersionWord;
} uBoardVersion;

/*****************************************************************************
 * Structure Name: stInternalFunctions
 *
 * Structure Description:
 * The structure that describes a functional block
 *
 * Structure Members:
 * parentInternalFunctions	:	Pointer to functional block of parent
 * childInternalFunctions	:	Pointer to functional block of child
 * pfnPaintFirstScreen		:	Pointer to the function that displays initial screen of any functional
 * 								block.
 * pfnRunTimeOperation		:	Pointer to the function that is called periodically to perform any
 * 								run time operations.
 * pfnUp					:	Pointer to the functions that is called in response to Up key press.
 * pfnDown					:	Pointer to the functions that is called in response to Down key press.
 * pfnMode					:	Pointer to the functions that is called in response to Mode key press.
 * pfnEnter					:	Pointer to the functions that is called in response to Enter key press.
 *
 ****************************************************************************/
typedef struct _stInternalFunctions
{
	struct _stInternalFunctions *parentInternalFunctions;
	struct _stInternalFunctions *childInternalFunctions;
	uint8_t (*pfnPaintFirstScreen) ();
	uint8_t (*pfnRunTimeOperation) ();
	uint8_t (*pfnUp) ();
	uint8_t (*pfnDown) ();
	uint8_t (*pfnMode) ();
	uint8_t (*pfnEnter) ();
}
stInternalFunctions;

typedef struct _stAnomalyError
{
	//uint32_t timeStamp;
	uint16_t anomalyCode;
	unsigned char description[15];
	uint8_t errorType;
	//uint32_t operationCount;
} stAnomalyError;

/*****************************************************************************
 * Structure Name: stMenu
 *
 * Structure Description:
 * The structure that describes a menu item.
 *
 * Structure Members:
 * pcText					:	Text to be displayed for this menu item.
 * psChildMenu				:	Child menu of this menu item ( if any ).
 * childFunctionalBlock		:	Pointer to the child's functional block ( if any ).
 *
 ****************************************************************************/
typedef struct _stMenuItem
{
    unsigned char *pcText_japanese;
    struct _stMenu *psChildMenu;
    stInternalFunctions *childFunctionalBlock;
    unsigned char *pcText_english;
}
stMenuItem;

/*****************************************************************************
 * Structure Name: stMenu
 *
 * Structure Description:
 * The structure that describes a menu.
 *
 * Structure Members:
 * psParent				:	The parent menu of this menu.
 * ui8Items				:	The total number of items in this menu.
 * psMenuItems			:	A pointer to the array of menu item structures..
 * ui8FocusIndex		:	The menu item index that has the focus.
 *
 ****************************************************************************/
typedef struct _stMenu
{
	struct _stMenu *psParent;
    uint8_t ui8Items;
    struct _stMenuItem *psMenuItems;
    uint8_t ui8FocusIndex;
} stMenu;
/****************************************************************************/

/****************************************************************************
 *  Global variables:
****************************************************************************/
extern volatile uint32_t g_ui32TickCount;
extern uint32_t ui32OperationCount;

// Active functional block
extern stInternalFunctions *psActiveFunctionalBlock;

// Active menu
extern stMenu *psActiveMenu;

extern stMenu gSettingsMenu;

// Power On
extern stInternalFunctions gsPowerOnFunctionalBlock;

// Home screen
extern stInternalFunctions gsHomeScreenFunctionalBlock;
extern struct stUMToCM gsUMToCMRegister;
extern struct stDisplayStatusAndFault gsDisplayStatusAndFaultRegister;

// Installation
extern stInternalFunctions gsInstallationFunctionalBlock;

//apertureheight
extern stInternalFunctions gsApertureheightFunctionalBlock;


//Run
extern stInternalFunctions gsShutterRunFunctionalBlock;

//Stop
extern stInternalFunctions gsShutterStopFunctionalBlock;

//RunStop
extern stInternalFunctions gsShutterRunStopFunctionalBlock;

// Menus
extern stInternalFunctions gsMenuFunctionalBlock;
extern stMenu gsMainMenu;

// Parameter list functional block
extern stInternalFunctions gsParameterListFunctionalBlock;

// Value type parameter functional block
extern stInternalFunctions gsValueTypeParamFunctionalBlock;

// state type parameter functional block
extern stInternalFunctions gsStateTypeParamFunctionalBlock;

// Date and Time
extern stInternalFunctions gsDateTimeFunctionalBlock;

// Anomaly History
extern stInternalFunctions gsAnomalyHistoryFunctionalBlock;

// Anomaly History
extern stInternalFunctions gsAnomalyHistoryDumpFunctionalBlock;

// Anomaly History
extern stInternalFunctions gsAnomalyHistoryEraseFunctionalBlock;

// Change Setting History
extern stInternalFunctions gsChangeSettingHistoryFunctionalBlock;

// Change Setting History
extern stInternalFunctions gsChangeSettingHistoryDumpFunctionalBlock;

// Change Setting History
extern stInternalFunctions gsChangeSettingHistoryEraseFunctionalBlock;

// SD card download parameters
extern stInternalFunctions gDownloadParametersFunctionalBlock;

// SD card UploadFirmware
//extern stInternalFunctions gUploadFirmwareFunctionalBlock;

//Parameter Reset
extern stInternalFunctions gsParamResetA120FunctionalBlock;

//Parameter Reset
extern stInternalFunctions gsParamResetA020FunctionalBlock;

//Parameter Reset
extern stInternalFunctions gsParamResetA081FunctionalBlock;

//Parameter Initialize
extern stInternalFunctions gsParamInitFunctionalBlock;
//gestureset ini
extern stInternalFunctions gsGestureFunctionalBlock;//cyw add

extern stInternalFunctions gsWirelessFunctionalBlockcyw;
//Operation Count
extern stInternalFunctions gsOperationCountFunctionalBlock;

//Operation Count
extern stInternalFunctions gsCtrlVerInfoFunctionalBlock;

//Operation Count
extern stInternalFunctions gsDispVerInfoFunctionalBlock;

//Operation Count
extern stInternalFunctions gsDrvVerInfoFunctionalBlock;

// Shutter Status
extern stInternalFunctions gsShutterStatusFunctionalBlock;

// SD card upload parameters
extern stInternalFunctions gsUploadDrvParametersFunctionalBlock;
extern stInternalFunctions gsUploadCtrlParametersFunctionalBlock;

// SDCard Parameter Upload Download variables
extern stMenuItem gSDCardUPParamDRVbrdMenuItems[MAX_FILES_FOR_SDPARAM];
extern stMenu gSDCardUPParamDRVbrdMenu;
extern stMenuItem gSDCardUPParamCTRLbrdMenuItems[MAX_FILES_FOR_SDPARAM];
extern stMenu gSDCardUPParamCTRLbrdMenu;

//SD Parameter Upload - Drive Board
extern stInternalFunctions gsSDUpParamDRVFunctionalBlock;

//SD Parameter Upload - Control Board
extern stInternalFunctions gsSDUpParamCTRLFunctionalBlock;

//SD Parameter Download -
extern stInternalFunctions gsDownloadParametersFunctionalBlock;

// Drive Status
extern stInternalFunctions gsDriveStatusFunctionalBlock;

// Wireless
extern stInternalFunctions gsWirelessFunctionalBlock;

// Firmware Upgrade
extern stInternalFunctions gsDisplayUpgradeFirmwareFunctionalBlock;
extern stInternalFunctions gsUpgradeProcessFunctionalBlock;

/****************************************************************************/

//*****************************************************************************
// Function prototypes.
//*****************************************************************************
void operationKeysHandler(void);
void updateFaultLEDStatus(void);
void displayAnomalies(void);
void RecoveredAnomalies(void);
uint8_t addToActiveAnomaly(struct errorDB* pstActiveAnomalyData);
uint8_t deleteFromActiveAnomaly(uint16_t lui16ErrorCode, bool bClearList);

extern uint8_t defaultFunction();
void communicationModule();
void installationTestFunction();
void installationRuntimeTestFunction();
void paramSettingTestFunction(void);
uint32_t get_timego(uint32_t x_data_his);

/******************************************************************************
 * FunctionName: dateTimeFirstScreen
 *
 * Function Description:
 * This function reads current date and time and displays on screen
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
extern uint8_t dateTimeFirstScreen();

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
extern uint8_t dateTimeEnter();

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
extern uint8_t dateTimeMode();

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
extern uint8_t dateTimeUp();

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
extern uint8_t dateTimeDown();

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
extern uint8_t dateTimeRunTime();

/******************************************************************************
 * FunctionName: homeScreenRunTime
 *
 * Function Description:
 * Handles runtime home screen variables. This function modify communication module
 * status registers so as to initiate process of getting current values of operation
 * count, shutter status and shutter mode.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * returns 0 on success
 *
 ********************************************************************************/
extern uint8_t homeScreenRunTime(void);

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
extern uint8_t homeScreenPaint(void);

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
 * Returns 1 if there is no active functional block
 *
 ********************************************************************************/
extern uint8_t homeScreenMode(void);

/******************************************************************************
 * FunctionName: menuPaintFirstScreen
 *
 * Function Description:
 * Paints the Main menu on screen. This function is called when any functional
 * block gives control of keys to the menu functional block.
 * It paints currently active menu defined by psActiveMenu variable.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 * 1	:	No active menu present
 *
 ********************************************************************************/
extern uint8_t menuPaintFirstScreen();

/******************************************************************************
 * FunctionName: menuDown
 *
 * Function Description:
 * Increments the current focus index and repaints screen. This gives effect of
 * menu item selection.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 * 1	:	No active menu present
 *
 ********************************************************************************/
extern uint8_t menuDown();

/******************************************************************************
 * FunctionName: menuUp
 *
 * Function Description:
 * Decrements the current focus index and repaints screen. This gives effect of
 * menu item selection.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 * 1	:	No active menu present
 *
 ********************************************************************************/
extern uint8_t menuUp();

/******************************************************************************
 * FunctionName: menuMode
 *
 * Function Description:
 * This functions paints the parent menu of active menu (if any).
 * If no parent menu is available then this function paints parent functional
 * block (if any)
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 * 1	:	Neither parent menu or parent functional block available
 *
 ********************************************************************************/
extern uint8_t menuMode();

/******************************************************************************
 * FunctionName: menuEnter
 *
 * Function Description:
 * This functions paints the child menu of focused menu item in active menu (if any).
 * If no child menu is available then this function paints child functional
 * block of focused menu item in active menu (if any).
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 * 1	:	Neither child menu nor child functional block of focused menu item
 * 			available.
 *
 ********************************************************************************/
extern uint8_t menuEnter();

extern uint8_t menuRunTime(void);

#endif /* USERINTERFACE_H_ */
