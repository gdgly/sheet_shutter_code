/*********************************************************************************
 * FileName: menulist.c
 * Description:
 * This source file contains the definitions of menu operations. This file defines
 * complete menu tree. Functions defined here are used when active functional
 * block is Menu functional block. It also defines Menu functional
 * block object.
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
 *  	0.2D	30/07/2014									Added Titlebased Menu structure
 *  	0.1D	11/04/2014      	iGATE Offshore team       Initial Creation
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
#include "userinterface.h"//modify cyw
#include "ledhandler.h"
//#include "japanesestrings.h"
#include "Gesture_Sensor/GP2AP054A_cyw.h"//add cyw
#include "Middleware/paramdatabase.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
//#define EN_OP_CMDS

#define NUMBER_OF_LINES		4
#define MENU_ITEM_HEIGHT	16
/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
uint8_t isMenutitle = 0; //1 - Menu Title

extern uint8_t setting_flag;//���������ò���

struct stLanguageSupport
{
	unsigned char englishText[40][20];
	unsigned char japaneseText[40][40];
};

struct stLanguageSupport gLanguageSupportDatabase =
{
	//
	// English Strings
	//
	{
		//
		// Main Menu Strings (0,1,2)
		//
		"OPERATIONS", "STATUS", "SETTINGS",

		//
		// Operations Menu Strings (3,4,5)
		//
		"INSTALLATION", "RUN_STOP"," ",

		//
		// Settings Menu Strings (6,7,8,9,10,11//12,13,14,15,16//17)
		//
		"PARAMETER_1", "PARAMETER_2", "DATE AND TIME", "SD CARD", "PARAMETER RESET", "INITIALIZE PARAM   ",
		"DOWNLOAD PARAMETERS", "UPLOAD PARAMETERS", "UPLOAD FIRMWARE  ", "INSTALLATION PARAMS", "OPERATION COUNT  ",
		"MICROSWITCH COUNT",

		//
		// Status Menu Strings (18,19,20,21,22,23//24,25,26,27,28,29,30)
		//
		" ", "SHUTTER STATUS ", " ", "VERSION", "SETTING HISTORY", "ANOMALY HISTORY",
		"CONTROL BOARD      ", "DISPLAY BOARD      ", "DRIVE BOARD        ", "DISPLAY", "DUMP TO SD CARD", "ERASE", " ",

		// SDCard Upload/Download parameter Strings (31,32)
		"SELECT BOARD:      ", "SELECT FILE:       "
	},

	//
	// Japanese Strings
	//
	{

		//
		// Main Menu Strings (0,1,2)
		//
		"�I�y���[�V����_���j���[", "�X�e�[�^�X_���j���[", "�Z�b�e�C_���j���[",

		//
		// Operations Menu Strings (3,4,5)
		//
		"���~�b�g_�Z�b�e�C_���[�h", "�h�E�T_�Z�b�e�C"," ",

		//
		// Settings Menu Strings (6,7,8,9,10,11//12,13,14,15,16//17)
		//
		"���[�U�[�p�����[�^","�����e�i���X�p�����[�^","�W�J��_�Z�b�e�C","SD�J�[�h","�p�����[�^���Z�b�g", "�I�[�����Z�b�g",
		//"�Z�[�u _�p�����[�^", "���[�h _�p�����[�^", "���[�h _�\�t�g�E�F�A", "���~�b�g _�Z�b�e�C _���[�h", "�h�E�T�J�E���g",
		"�Z�[�u_�p�����[�^", "���[�h_�p�����[�^", "���[�h_�\ �t�g�E�F�A", "���~�b�g_�Z�b�e�C", "�h�E�T�J�E���g",
		"�}�C�N���Z���T_�J�E���g",

		//
		// Status Menu Strings (18,19,20,21,22,23//24,25,26,27,28,29,30)
		//
		" ", "�V���b�^�[_�X�e�[�^�X", " ", "�o�[�W����", "�Z�b�e�C_�����L", "�G���[_�����L",
		"C���j�b�g", "D���j�b�g", "M���j�b�g", "�f�B�X�v���C", "SD�J�[�h�փR�s�[", "�N���A", " ",

		// SDCard Upload/Download parameter Strings (31,32)
		"���j�b�g_�Z���^�N:", "�t�@�C��_�Z���^�N:"
	}
};

//**************************************************************************
// Define Main Menu
//**************************************************************************
stMenu gOperationMenu;
stMenu gStatusMenu;
stMenu gSettingsMenu;

stMenuItem gsMainMenuItems[] =
{
    {gLanguageSupportDatabase.japaneseText[0], &gOperationMenu, 0,gLanguageSupportDatabase.englishText[0]},
    {gLanguageSupportDatabase.japaneseText[1], &gStatusMenu, 0,gLanguageSupportDatabase.englishText[1]},
    {gLanguageSupportDatabase.japaneseText[2], &gSettingsMenu, 0,gLanguageSupportDatabase.englishText[2]},
};

stMenu gsMainMenu =
{
    0, (sizeof(gsMainMenuItems) / sizeof(stMenuItem)), gsMainMenuItems, 0
};

//**************************************************************************
// Define Operations Menu
//**************************************************************************
stMenuItem gOperationMenuItems[] =
{
    {gLanguageSupportDatabase.japaneseText[3], 0, &gsInstallationFunctionalBlock,gLanguageSupportDatabase.englishText[3]},
    {gLanguageSupportDatabase.japaneseText[4], 0, &gsShutterRunStopFunctionalBlock,gLanguageSupportDatabase.englishText[4]},
   // {gLanguageSupportDatabase.japaneseText[5], 0, &gsShutterStopFunctionalBlock}
};

stMenu gOperationMenu =
{
    &gsMainMenu, (sizeof(gOperationMenuItems) / sizeof(stMenuItem)),
    gOperationMenuItems, 0
};

//**************************************************************************
// Define Settings Menu
//**************************************************************************
stMenu gSDCardMenu;
stMenu gParameterResetMenu;

stMenuItem gSettingsMenuItems[] =
{
	{gLanguageSupportDatabase.japaneseText[6], 0, &gsParameterListFunctionalBlock,gLanguageSupportDatabase.englishText[6]},
	{gLanguageSupportDatabase.japaneseText[7], 0, &gsParameterListFunctionalBlock,gLanguageSupportDatabase.englishText[7]},
	{"�����R��_�g�E���N", 0, &gsWirelessFunctionalBlockcyw,"WIRELESS LOGIN "},//SERVICE PARAM
	{gLanguageSupportDatabase.japaneseText[8], 0, &gsDateTimeFunctionalBlock,gLanguageSupportDatabase.englishText[8]},
	{gLanguageSupportDatabase.japaneseText[9], &gSDCardMenu, 0,gLanguageSupportDatabase.englishText[9]},
	{gLanguageSupportDatabase.japaneseText[10], &gParameterResetMenu, 0,gLanguageSupportDatabase.englishText[10]},
	//{gLanguageSupportDatabase.japaneseText[11], 0, &gsParamInitFunctionalBlock},
	//{"�e�J�U�V_�Z���T", 0, &gsGestureFunctionalBlock},//add cyw GESTURE SENSOR
};

stMenu gSettingsMenu =
{
    &gsMainMenu, (sizeof(gSettingsMenuItems) / sizeof(stMenuItem)),
    gSettingsMenuItems, 0
};

//**************************************************************************
// SD Card Menu
//**************************************************************************

stMenu gSDCardUPParamMenu;
stMenu gSDCardUPParamCTRLbrdMenu;
stMenu gSDCardUPParamDRVbrdMenu;
stMenu gSDCardUploadFirmwareMenu;

stMenuItem gSDCardMenuItems[] =
{
	{gLanguageSupportDatabase.japaneseText[12], 0, &gsDownloadParametersFunctionalBlock,gLanguageSupportDatabase.englishText[12]}, 
        //{gLanguageSupportDatabase.japaneseText[13], &gSDCardUPParamMenu, 0,gLanguageSupportDatabase.englishText[13]},
        {gLanguageSupportDatabase.japaneseText[13], &gSDCardUPParamDRVbrdMenu, &gsUploadDrvParametersFunctionalBlock,gLanguageSupportDatabase.englishText[13]},
	{gLanguageSupportDatabase.japaneseText[14], 0, &gsDisplayUpgradeFirmwareFunctionalBlock,gLanguageSupportDatabase.englishText[14]}
};

stMenu gSDCardMenu =
{
	&gSettingsMenu, (sizeof(gSDCardMenuItems) / sizeof(stMenuItem)),
	gSDCardMenuItems, 0
};


/*stMenuItem gSDCardUploadFirmwareMenuItems[] =
{
	{gLanguageSupportDatabase.japaneseText[24], 0, 0},
	{gLanguageSupportDatabase.japaneseText[25], 0, 0},
	{gLanguageSupportDatabase.japaneseText[26], 0, 0}
};

stMenu gSDCardUploadFirmwareMenu =
{
	&gSDCardMenu, (sizeof(gSDCardUploadFirmwareMenuItems) / sizeof(stMenuItem)),
	gSDCardUploadFirmwareMenuItems, 0
};*/

stMenuItem gSDCardUPParamMenuItems[] =
{
	{gLanguageSupportDatabase.japaneseText[31], 0, 0,gLanguageSupportDatabase.englishText[31]}, 
        {gLanguageSupportDatabase.japaneseText[24], &gSDCardUPParamCTRLbrdMenu, &gsUploadCtrlParametersFunctionalBlock,gLanguageSupportDatabase.englishText[24]},
	{gLanguageSupportDatabase.japaneseText[26], &gSDCardUPParamDRVbrdMenu, &gsUploadDrvParametersFunctionalBlock,gLanguageSupportDatabase.englishText[26]}
};

stMenu gSDCardUPParamMenu =
{
	&gSDCardMenu, (sizeof(gSDCardUPParamMenuItems) / sizeof(stMenuItem)),
	gSDCardUPParamMenuItems, 0
};

stMenuItem gSDCardUPParamCTRLbrdMenuItems[MAX_FILES_FOR_SDPARAM];

stMenu gSDCardUPParamCTRLbrdMenu =
{
	&gSDCardUPParamMenu, (sizeof(gSDCardUPParamCTRLbrdMenuItems) / sizeof(stMenuItem)),
	gSDCardUPParamCTRLbrdMenuItems, 0
};

/*stMenuItem gSDCardUPParamDRVbrdMenuItems[3] =
{
	{gLanguageSupportDatabase.japaneseText[32], 0, 0}, {"DRV1.XXX", 0, 0},
	{"DRV2.XXX", 0, 0}
};*/

stMenuItem gSDCardUPParamDRVbrdMenuItems[MAX_FILES_FOR_SDPARAM];

stMenu gSDCardUPParamDRVbrdMenu =
{
	/*&gSDCardUPParamMenu*/&gSDCardMenu, (sizeof(gSDCardUPParamDRVbrdMenuItems) / sizeof(stMenuItem)),
	gSDCardUPParamDRVbrdMenuItems, 0
};

//**************************************************************************
// Parameter Reset Menu
//**************************************************************************
stMenuItem gParameterResetMenuItems[] =
{
	{gLanguageSupportDatabase.japaneseText[15], 0, &gsParamResetA120FunctionalBlock,gLanguageSupportDatabase.englishText[15]}, 
        {gLanguageSupportDatabase.japaneseText[16], 0, &gsParamResetA020FunctionalBlock,gLanguageSupportDatabase.englishText[16]},
	{gLanguageSupportDatabase.japaneseText[17], 0, &gsParamResetA081FunctionalBlock,gLanguageSupportDatabase.englishText[17]},
        {gLanguageSupportDatabase.japaneseText[11], 0, &gsParamInitFunctionalBlock,gLanguageSupportDatabase.englishText[11]},
};

stMenu gParameterResetMenu =
{
	&gSettingsMenu, (sizeof(gParameterResetMenuItems) / sizeof(stMenuItem)),
	gParameterResetMenuItems, 0
};

//**************************************************************************
// Define Status Menu
//**************************************************************************
stMenu gVersionMenu;
stMenu gSettingsHistoryMenu;
stMenu gAnomalyHistoryMenu;

stMenuItem gStatusMenuItems[] =
{
	//{gLanguageSupportDatabase.japaneseText[18], 0, &gsOperationCountFunctionalBlock}, 
        {gLanguageSupportDatabase.japaneseText[19], 0, &gsShutterStatusFunctionalBlock,gLanguageSupportDatabase.englishText[19]},
	//{gLanguageSupportDatabase.japaneseText[20], 0, 0}, 
        {gLanguageSupportDatabase.japaneseText[21], &gVersionMenu, 0,gLanguageSupportDatabase.englishText[21]},
	{gLanguageSupportDatabase.japaneseText[22], &gSettingsHistoryMenu, 0,gLanguageSupportDatabase.englishText[22]},
	{gLanguageSupportDatabase.japaneseText[23], &gAnomalyHistoryMenu, 0,gLanguageSupportDatabase.englishText[23]},
	//{gLanguageSupportDatabase.japaneseText[30], 0, &gsWirelessFunctionalBlock}
};

stMenu gStatusMenu =
{
    &gsMainMenu, (sizeof(gStatusMenuItems) / sizeof(stMenuItem)),
    gStatusMenuItems, 0
};

//**************************************************************************
// Define Version Menu
//**************************************************************************
stMenuItem gVersionMenuItems[] =
{
	{gLanguageSupportDatabase.japaneseText[24], 0, &gsCtrlVerInfoFunctionalBlock,gLanguageSupportDatabase.englishText[24]}, 
        {gLanguageSupportDatabase.japaneseText[25], 0, &gsDispVerInfoFunctionalBlock,gLanguageSupportDatabase.englishText[25]},
	{gLanguageSupportDatabase.japaneseText[26], 0, &gsDrvVerInfoFunctionalBlock,gLanguageSupportDatabase.englishText[26]}
};

stMenu gVersionMenu =
{
    &gStatusMenu, (sizeof(gVersionMenuItems) / sizeof(stMenuItem)),
    gVersionMenuItems, 0
};

//**************************************************************************
// Define Settings History Menu
//**************************************************************************
stMenuItem gSettingsHistoryMenuItems[] =
{
	//{gLanguageSupportDatabase.japaneseText[27], 0, &gsChangeSettingHistoryFunctionalBlock}, 
        {"�Z�b�e�C_�����L", 0, &gsChangeSettingHistoryFunctionalBlock,"SETTING HISTROY"},
        {gLanguageSupportDatabase.japaneseText[28], 0, &gsChangeSettingHistoryDumpFunctionalBlock,gLanguageSupportDatabase.englishText[28]},
	{gLanguageSupportDatabase.japaneseText[29], 0, &gsChangeSettingHistoryEraseFunctionalBlock,gLanguageSupportDatabase.englishText[29]}
};

stMenu gSettingsHistoryMenu =
{
    &gStatusMenu, (sizeof(gSettingsHistoryMenuItems) / sizeof(stMenuItem)),
    gSettingsHistoryMenuItems, 0
};

//**************************************************************************
// Define Anomaly History Menu
//**************************************************************************
stMenuItem gAnomalyHistoryMenuItems[] =
{
	//{gLanguageSupportDatabase.japaneseText[27], 0, &gsAnomalyHistoryFunctionalBlock},
         
        {"�G���[_�����L" , 0, &gsAnomalyHistoryFunctionalBlock,"ANOMALY HISTORY"},
        {gLanguageSupportDatabase.japaneseText[28], 0, &gsAnomalyHistoryDumpFunctionalBlock,gLanguageSupportDatabase.englishText[28]},
	{gLanguageSupportDatabase.japaneseText[29], 0, &gsAnomalyHistoryEraseFunctionalBlock,gLanguageSupportDatabase.englishText[29]}
};

stMenu gAnomalyHistoryMenu =
{
    &gStatusMenu, (sizeof(gAnomalyHistoryMenuItems) / sizeof(stMenuItem)),
    gAnomalyHistoryMenuItems, 0
};

//	Added this variable to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
#define SERVICED		0
#define ACTIVATED		1
#define DEACTIVATED		2

uint8_t gui8SettingsModeStatus = SERVICED;
uint8_t gui8SettingsScreen = SERVICED;
/*****************************************************************************/

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
uint8_t menuPaintFirstScreen()
{
	uint8_t ui8YMin = 0, ui8x = 0;
	uint8_t lastItemIndexToRender = 0;

	if((psActiveMenu == &gSDCardUPParamMenu) || (psActiveMenu == &gSDCardUPParamCTRLbrdMenu) || (psActiveMenu == &gSDCardUPParamDRVbrdMenu))
		isMenutitle = 1;
	else
		isMenutitle = 0;

	if(isMenutitle && !psActiveMenu->ui8FocusIndex)
		psActiveMenu->ui8FocusIndex++;
	//
	// Get index of first item to paint
	//
	ui8x = (psActiveMenu->ui8FocusIndex / 4) * 4;

	//
	// Handle exception if there is no active menu.
	//
	if( (psActiveMenu == NULL) || (psActiveFunctionalBlock == NULL) )
		return 1;

	//
	// Clear Screen.
	//
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, true, true);
	//
	// Get last item index to render
	//
//	if(psActiveMenu->ui8Items <= NUMBER_OF_LINES)
		lastItemIndexToRender = psActiveMenu->ui8Items;
//	else
//		lastItemIndexToRender = NUMBER_OF_LINES;

	//
	//	Loop while there exists a menu item to render on screen.
	//
	while ( ui8x < lastItemIndexToRender )
	{
		//
		// Draw menu items on screen. Highlight the menu item which has current focus
		// given by ui8FocusIndex.
		//
		if(ui8x == psActiveMenu->ui8FocusIndex)
		{
			if(gu8_language == Japanese_IDX)
                       {
                        displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 4, ui8YMin, true, false, false, true, false, false);
                        }
                       else
                       {
                        displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 4, ui8YMin, true, false, false, true, false, false);
                        }
		}
		else
		{
			if(gu8_language == Japanese_IDX)
                       {
                       displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 4, ui8YMin, false, false, false, true, false, false);
                        }
                        else
                        {
                       displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 4, ui8YMin, false, false, false, true, false, false);
                         }
		}

		//
		// Increment Y coordinate of the top left corner of the menu item to be displayed
		//
		ui8YMin += MENU_ITEM_HEIGHT;

		ui8x++;
	}

	return 0;
}

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
uint8_t menuUp()
{
	uint8_t ui8YMin = 0, ui8x = 0;
	uint8_t lastItemIndexToRender = 0;

    if(gKeysStatus.bits.Key_Up_pressed)
    {
    	gKeysStatus.bits.Key_Up_pressed = 0;

		//
		// Handle exception if there is no active menu.
		//
		if( (psActiveMenu == NULL) || (psActiveFunctionalBlock == NULL) )
			return 1;

		//
		// Decrement Focus index if focus is not on the first menu item of the
		// active menu.
		//
		if(psActiveMenu->ui8FocusIndex > 0)
			psActiveMenu->ui8FocusIndex--;

		if(isMenutitle && !psActiveMenu->ui8FocusIndex)
			psActiveMenu->ui8FocusIndex++;

		//
		// Find the index of the last menu item to be rendered on screen.
		//
		if(psActiveMenu->ui8FocusIndex > NUMBER_OF_LINES - 1)
		{
			//
			// Get index of first menu item to be rendered on screen.
			//
			ui8x = (psActiveMenu->ui8FocusIndex /4) * 4;

			//
			// Calculate index of last menu item to be rendered on screen.
			//
			if( (psActiveMenu->ui8Items - ui8x) < NUMBER_OF_LINES )
				lastItemIndexToRender = psActiveMenu->ui8Items;
			else
				lastItemIndexToRender = ui8x + NUMBER_OF_LINES;
		}
		else
			lastItemIndexToRender =  psActiveMenu->ui8Items;

		//
		// Clear Screen.
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, true, true);

		//
		//	Loop while there exists a menu item to render on screen.
		//
		while ( ui8x < lastItemIndexToRender )
		{
			//
			// Draw menu items on screen. Highlight the menu item which has current focus
			// given by ui8FocusIndex.
			//
			if(ui8x == psActiveMenu->ui8FocusIndex)
			{
				if(gu8_language == Japanese_IDX)
                                {
                                displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 4, ui8YMin, true, false, false, true, false, false);
                                }
                                else
                                {
                                  displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 4, ui8YMin, true, false, false, true, false, false);
                                }
			}
			else
			{
				if(gu8_language == Japanese_IDX)
                               {
                               displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 4, ui8YMin, false, false, false, true, false, false);
                               }
                               else
                                {
                               displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 4, ui8YMin, false, false, false, true, false, false);
                                 }
			}

			//
			// Increment Y coordinate of the top left corner of the menu item to be displayed
			//
			ui8YMin += MENU_ITEM_HEIGHT;

			ui8x++;
		}
    }

	if(gKeysStatus.bits.Key_Up_released)
	{
		gKeysStatus.bits.Key_Up_released = 0;
	}

	return 0;
}

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
uint8_t menuDown()
{
	uint8_t ui8YMin = 0, ui8x = 0;
	uint8_t lastItemIndexToRender = 0;

    if(gKeysStatus.bits.Key_Down_pressed)
    {
    	gKeysStatus.bits.Key_Down_pressed = 0;
		//
		// Handle exception if there is no active menu.
		//
		if( (psActiveMenu == NULL) || (psActiveFunctionalBlock == NULL) )
			return 1;

		//
		// Increment the focus index if focus is not on the last menu item of
		// active menu.
		//
		if(psActiveMenu->ui8FocusIndex < psActiveMenu->ui8Items - 1)
			psActiveMenu->ui8FocusIndex++;

		//
		// Find the index of the last menu item to be rendered on screen.
		//
		if(psActiveMenu->ui8FocusIndex > NUMBER_OF_LINES - 1)
		{
			//
			// Get index of first menu item to be rendered on screen.
			//
			ui8x = (psActiveMenu->ui8FocusIndex /4) * 4;

			//
			// Calculate index of last menu item to be rendered on screen.
			//
			if( (psActiveMenu->ui8Items - ui8x) < NUMBER_OF_LINES )
				lastItemIndexToRender = psActiveMenu->ui8Items;
			else
				lastItemIndexToRender = ui8x + NUMBER_OF_LINES;
		}
		else
			lastItemIndexToRender =  psActiveMenu->ui8Items;

		//
		// Clear screen
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, true, true);
		//
		//	Loop while there exists a menu item to render on screen.
		//
		while ( ui8x < lastItemIndexToRender )
		{
			//
			// Draw menu items on screen. Highlight the menu item which has current focus
			// given by ui8FocusIndex.
			//
			if(ui8x == psActiveMenu->ui8FocusIndex)
			{
				if(gu8_language == Japanese_IDX)
                               {
                                displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 4, ui8YMin, true, false, false, true, false, false);
                               }
                               else
                               {
                                displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 4, ui8YMin, true, false, false, true, false, false);
                                }
			}
			else
			{
				if(gu8_language == Japanese_IDX)
                              {
                              displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 4, ui8YMin, false, false, false, true, false, false);
                              }
                              else 
                              {
                              displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 4, ui8YMin, false, false, false, true, false, false);
                               }
			}

			//
			// Increment Y coordinate of the top left corner of the menu item to be displayed
			//
			ui8YMin += MENU_ITEM_HEIGHT;

			ui8x++;
		}
    }

	if(gKeysStatus.bits.Key_Down_released)
	{
		gKeysStatus.bits.Key_Down_released = 0;
	}

	return 0;
}

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
uint8_t menuMode()
{
    if(gKeysStatus.bits.Key_Mode_pressed)
    {

    	gKeysStatus.bits.Key_Mode_pressed = 0;



//    	if(isMenutitle)
    		isMenutitle = 0;
		//
		// Handle exception if there is no active menu.
		//
		if( (psActiveMenu == NULL) || (psActiveFunctionalBlock == NULL) )
			return 1;

/*		if((psActiveMenu->psParent != NULL) && (psActiveFunctionalBlock->parentInternalFunctions != NULL))
		{
//			psActiveMenu->ui8FocusIndex = 0;
			psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
			psActiveMenu = psActiveMenu->psParent;
//			menuPaintFirstScreen();
			psActiveFunctionalBlock->pfnPaintFirstScreen();
		}
		else */if(psActiveMenu->psParent != NULL)
		{
			psActiveMenu = psActiveMenu->psParent;

			menuPaintFirstScreen();
			//	Added to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
			if((psActiveMenu == &gsMainMenu) && (gui8SettingsScreen == ACTIVATED))
			{
				//	User has come out of settings menu. Enable shutter operations.
				gui8SettingsModeStatus = DEACTIVATED;
				gui8SettingsScreen = DEACTIVATED;
			}
		}
		else if(psActiveFunctionalBlock->parentInternalFunctions != NULL)
		{
			psActiveMenu->ui8FocusIndex = 0;
			psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
			psActiveFunctionalBlock->pfnPaintFirstScreen();
                gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
		gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
		gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
		gstUMtoCMdatabase.commandToControlBoard.val = 0;
			guest_reinit();
		}
		else
		{
			return 1;
		}
    }

    if(gKeysStatus.bits.Key_Mode_released)
    {
    	gKeysStatus.bits.Key_Mode_released = 0;
    }

	return 0;
}

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
uint8_t menuEnter()
{
	uint8_t ui8CurrentFocusIndex = 0;

    if(gKeysStatus.bits.Key_Enter_pressed)
    {
    	gKeysStatus.bits.Key_Enter_pressed = 0;

		ui8CurrentFocusIndex = psActiveMenu->ui8FocusIndex;

		//
		// Handle exception if there is no active menu.
		//
		if( (psActiveMenu == NULL) || (psActiveFunctionalBlock == NULL) )
			return 1;

		if((psActiveMenu->psMenuItems[ui8CurrentFocusIndex].psChildMenu != NULL) && (psActiveMenu->psMenuItems[ui8CurrentFocusIndex].childFunctionalBlock != NULL))
		{
			psActiveFunctionalBlock = psActiveMenu->psMenuItems[ui8CurrentFocusIndex].childFunctionalBlock;
			psActiveFunctionalBlock->pfnPaintFirstScreen();
			psActiveMenu = psActiveMenu->psMenuItems[ui8CurrentFocusIndex].psChildMenu;
			psActiveMenu->ui8FocusIndex = 0;
			menuPaintFirstScreen();
		}
		else if(psActiveMenu->psMenuItems[ui8CurrentFocusIndex].psChildMenu != NULL)
		{
			psActiveMenu = psActiveMenu->psMenuItems[ui8CurrentFocusIndex].psChildMenu;
			psActiveMenu->ui8FocusIndex = 0;
			menuPaintFirstScreen();
			//	Added to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
			if(psActiveMenu == &gSettingsMenu)
			{
				//	User has entered into settings menu. Disable shutter operations.
				gui8SettingsModeStatus = ACTIVATED;
				gui8SettingsScreen = ACTIVATED;
			}
		}
		else if(psActiveMenu->psMenuItems[ui8CurrentFocusIndex].childFunctionalBlock != NULL)
		{
			psActiveFunctionalBlock = psActiveMenu->psMenuItems[ui8CurrentFocusIndex].childFunctionalBlock;
			psActiveFunctionalBlock->pfnPaintFirstScreen();
		}
		else
		{
			return 1;
		}
    }

	if(gKeysStatus.bits.Key_Enter_released)
	{
		gKeysStatus.bits.Key_Enter_released = 0;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: menuRunTime
 *
 * Function Description:
 * This is a default function for menu runtime.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * Returns 0 on success
 *
 ********************************************************************************/
uint8_t menuRunTime(void)
{
	//Clear Operational Key flags
	//gKeysStatus.byte.LB_Pressed = gKeysStatus.byte.LB_Pressed & 0xF8; //(KEY_OPEN, KEY_STOP, KEY_CLOSE);
	//gKeysStatus.byte.HB_Released = gKeysStatus.byte.HB_Released & 0xF8; //(KEY_OPEN, KEY_STOP, KEY_CLOSE);

	//
	// Update fault LED status
	//
	updateFaultLEDStatus();

	//	Added to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015

	if(gui8SettingsModeStatus == ACTIVATED)
	{
		if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		{
			gui8SettingsModeStatus = SERVICED;
			gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus = 1;
			gstUMtoCMoperational.additionalCommandData = 1;
			gstUMtoCMoperational.commandRequestStatus = eACTIVE;
		}
	}
	else if(gui8SettingsModeStatus == DEACTIVATED)
	{
		if(gstUMtoCMoperational.commandRequestStatus == eINACTIVE)
		{
			gui8SettingsModeStatus = SERVICED;
			gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus = 1;
			gstUMtoCMoperational.additionalCommandData = 0;
			gstUMtoCMoperational.commandRequestStatus = eACTIVE;
		}
	}

	//
	// Check for command response and take respective action
	//
	if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
	{
		if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
		{
			if(gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus == 1)
			{
				gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus = 0;
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
		}
	}
#ifdef EN_OP_CMDS
	operationKeysHandler();
#endif

	return 0;
}

/******************************************************************************
 * Define Menu Functional Block
*********************************************************************************/
stInternalFunctions gsMenuFunctionalBlock =
{
	&gsHomeScreenFunctionalBlock,
	0,
	menuPaintFirstScreen,
	menuRunTime,
	menuUp,
	menuDown,
	menuMode,
	menuEnter
};
