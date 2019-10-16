/*********************************************************************************
* FileName: upgradefirmwarescreens.c
* Description:
* This source file contains the definition of all the services of ...
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
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inc/hw_types.h>
#include <inc/hw_nvic.h>
#include <driverlib/gpio.h>
#include <driverlib/flash.h>
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "grlib/grlib.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "userinterface.h"
#include "intertaskcommunication.h"
#include "parameterlist.h"
#include "Middleware/sdcard.h"
#include "Middleware/rtc.h"
#include "flashmemory.h"
#include "Middleware/paramdatabase.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/

/****************************************************************************/

/****************************************************************************
 *  Global variables for other files:
****************************************************************************/
extern stMenu gSDCardMenu;

uint32_t gui8UpgradeViaApplicationStatusFlags = 0;

unsigned char upgradeFirmwareFilenames[MAX_FILES_FOR_SDPARAM - 1][20];

unsigned char fileNameToUpgrade[20] = {0};
uint8_t gCurrentFocusIndex = 0;

stMenuItem gUpgradeFirmwareMenuItems[MAX_FILES_FOR_SDPARAM];
stMenu gUpgradeFirmwareMenu;

stMenuItem gBoardsMenuItems[] =
{
	{"ユニット_センタク:", 0, 0,"SELECT BOARD:"},
	{"Cユニット", &gUpgradeFirmwareMenu, 0,"CONTROL BOARD      "},
	{"Dユニット", &gUpgradeFirmwareMenu, 0,"DISPLAY BOARD      "},
	{"Mユニット", &gUpgradeFirmwareMenu, 0,"DRIVE BOARD        "}
};


stMenu gBoardsMenu = {&gSDCardMenu, 4, gBoardsMenuItems, 0};

uint8_t gUpgradeStage = 0;

uint8_t diplayBoardFirmwareUpgradeStart = 0;

const unsigned char cucSelectFilesString_english[] = "SELECT FILES:      ";
const unsigned char cucSelectFilesString_japanese[] = "ファイル_センタク:";
char cFilenameInitials[5] = {0};

/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
****************************************************************************/
uint8_t displayActiveMenuItems(void);

/****************************************************************************/

/******************************************************************************
 * FunctionName: jumpToBootLoader
 *
 * Function Description:
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void jumpToBootLoader(void)
{
	HWREG(NVIC_VTABLE) = (uint32_t)BOOT_LOADER_AREA_STARTING_ADDRESS;

	/*__asm(" ldr r1, [r0]\n"
			" mov sp, r1");*/
	__asm(" ldr r1, [r0]");
	__asm(" mov sp, r1");

	/*__asm(" ldr r0, [r0, #4]\n"
			" bx r0\n");*/
	__asm(" ldr r0, [r0, #4]");
	__asm(" bx r0\n");
}

/******************************************************************************
 * FunctionName: getUpgradeFileNameListDisplayBoard
 *
 * Function Description:
 * This function is used to gather filenames which can be used for firmware
 * upgrade into a menu list form.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void getUpgradeFileNameListDisplayBoard(void)
{
	uint8_t i = 0;
	if(gu8_language == Japanese_IDX)
	{
	gUpgradeFirmwareMenuItems[i].pcText_japanese = (unsigned char *)cucSelectFilesString_japanese;
	}
	else
	{
	gUpgradeFirmwareMenuItems[i].pcText_english = (unsigned char *)cucSelectFilesString_english;
	}
	gUpgradeFirmwareMenuItems[i].psChildMenu = 0;
	gUpgradeFirmwareMenuItems[i].childFunctionalBlock = 0;

	for(i = 0; i < MAX_FILES_FOR_SDPARAM; i++)
	{
		if(list_SDCard(cFilenameInitials, i+1, (char *)upgradeFirmwareFilenames[i]) != FR_OK)
			break;

		if(gu8_language == Japanese_IDX)
	   {
		gUpgradeFirmwareMenuItems[i+1].pcText_japanese = upgradeFirmwareFilenames[i];
	   }
		else
		{
		gUpgradeFirmwareMenuItems[i+1].pcText_english = upgradeFirmwareFilenames[i];
		}
		gUpgradeFirmwareMenuItems[i+1].psChildMenu = 0;
		gUpgradeFirmwareMenuItems[i+1].childFunctionalBlock = &gsUpgradeProcessFunctionalBlock;
	}

	//
	// update menu details - total menu items, current focus index and menu items list
	//
	gUpgradeFirmwareMenu.psParent = &gBoardsMenu;
	gUpgradeFirmwareMenu.ui8Items = i+1;
	gUpgradeFirmwareMenu.ui8FocusIndex = 0;
	gUpgradeFirmwareMenu.psMenuItems = gUpgradeFirmwareMenuItems;

	//
	// Set upgrade firmware menu as active menu.
	//
	psActiveMenu = &gUpgradeFirmwareMenu;
}

/******************************************************************************
 * FunctionName: displayUpgradeFirmwarePaint
 *
 * Function Description:
 * This function should be called after setting Display Upgrade Firmware Functional Block
 * as active functional block. This function will draw the first screen to be shown to
 * user when a firmware upgrade is needed.
 *
 * Function Parameter: void
 *
 * Function Returns:
 * 0 on success
 *
 ********************************************************************************/
uint8_t displayUpgradeFirmwarePaint(void)
{
	//
	// Clear screen
	//
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, true, true);
	//
	// Display firmware upgrade menu
	//
	psActiveMenu = &gBoardsMenu;
	displayActiveMenuItems();

	gUpgradeStage=0;


	return 0;
}




/******************************************************************************
 * FunctionName: displayUpgradeFirmwareRunTime
 *
 * Function Description:
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
uint8_t displayUpgradeFirmwareRunTime(void)
{
	static uint8_t lsui8StartControlBoardUpgrade = 0;

	//
	// Check whether control board firmware upgrade is required
	//
	if(	(1 == gstControlBoardStatus.bits.controlBoardBootLoader) &&
		(0 == lsui8StartControlBoardUpgrade)
	)
	{
		GrRectFIllBolymin(0, 126, 0, 63, true, true);

		if(gu8_language == Japanese_IDX)
		{
		//
		displayText("Cユニット", 2, 0, false, false, false, false, false, false);
		//
		//
		displayText("サイシンノFWニ", 1, 16, false, false, false, false, false, false);
		displayText("カキカエテクダサイ", 2, 32, false, false, false, false, false, false);
		}
		else
		{
			displayText("CONTROL BOARD", 2, 0, false, false, false, false,false,true);
			displayText("FIRMWARE UPGRADE", 1, 16, false, false, false, false, false, true);
			displayText("IS NEEDED", 2, 32, false, false, false, false, false, true);
		}
		displayText("PRESS <ENTER>", 2, 48, false, false, false, false, false, true);

		//
		// Set start control board upgrade flag
		//
		lsui8StartControlBoardUpgrade = 1;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: displayActiveMenuItems
 *
 * Function Description:
 * This function is used to display all the filenames of the files that can be used
 * for firmware upgrade in a menu list format. User can choose any of these files to
 * start firmware upgrade using menu keys.
 *
 * Function Parameter: void
 *
 * Function Returns:
 * 0 on success
 *
 ********************************************************************************/
uint8_t displayActiveMenuItems(void)
{
	uint8_t ui8YMin = 0, ui8x = 0;
	uint8_t lastItemIndexToRender = 0;

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
	lastItemIndexToRender = psActiveMenu->ui8Items;

	//
	// Set focus on the first file name in list.
	//
	psActiveMenu->ui8FocusIndex = 1;

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
			displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 2, ui8YMin, true, false, false, true, false, false);
			}
			else
			{
				displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 2, ui8YMin, true, false, false, true, false, true);
			}
		}
		else
		{
			if(gu8_language == Japanese_IDX)
			{
			displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 2, ui8YMin, false, false, false, true, false, false);
			}
			else
			{
			displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 2, ui8YMin, false, false, false, true, false, true);
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
 * FunctionName: displayUpgradeFirmwareUp
 *
 * Function Description:
 * This function is used to traverse filenames in upward direction using UP button.
 *
 * Function Parameter: void
 *
 * Function Returns:
 * 0 on success
 *
 ********************************************************************************/
uint8_t displayUpgradeFirmwareUp(void)
{
	uint8_t ui8YMin = 0, ui8x = 0;
	uint8_t lastItemIndexToRender = 0;

    if(gKeysStatus.bits.Key_Up_pressed)
    {
    	gKeysStatus.bits.Key_Up_pressed = 0;

    	gUpgradeStage=0;
		//
		// Handle exception if there is no active menu.
		//
		if( (psActiveMenu == NULL) || (psActiveFunctionalBlock == NULL) )
			return 1;

		//
		// Decrement Focus index if focus is not on the first menu item of the
		// active menu.
		//
		if(psActiveMenu->ui8FocusIndex > 1)
			psActiveMenu->ui8FocusIndex--;

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
				displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 2, ui8YMin, true, false, false, true, false, false);
				}
				else
				{
					displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 2, ui8YMin, true, false, false, true, false, true);
				}
			}
			else
			{
				if(gu8_language == Japanese_IDX)
				{
				displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 2, ui8YMin, false, false, false, true, false, false);
				}
				else
				{
					displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 2, ui8YMin, false, false, false, true, false, true);
				}
			}

			//
			// Increment Y coordinate of the top left corner of the menu item to be displayed
			//
			ui8YMin += MENU_ITEM_HEIGHT;

			ui8x++;
		}
    }


	return 0;
}

/******************************************************************************
 * FunctionName: displayUpgradeFirmwareDown
 *
 * Function Description:
 * This function is used to traverse filenames in downward direction using DOWN button.
 *
 * Function Parameter: void
 *
 * Function Returns:
 * 0 on success
 *
 ********************************************************************************/
uint8_t displayUpgradeFirmwareDown(void)
{
	uint8_t ui8YMin = 0, ui8x = 0;
	uint8_t lastItemIndexToRender = 0;

    if(gKeysStatus.bits.Key_Down_pressed)
    {
    	gKeysStatus.bits.Key_Down_pressed = 0;


    	gUpgradeStage=0;
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
				displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 2, ui8YMin, true, false, false, true, false, false);
				}
				else
				{
				displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 2, ui8YMin, true, false, false, true, false, true);
				}
			}
			else
			{
				if(gu8_language == Japanese_IDX)
				{
				displayText(psActiveMenu->psMenuItems[ui8x].pcText_japanese, 2, ui8YMin, false, false, false, true, false, false);

				}
				else
				{
					displayText(psActiveMenu->psMenuItems[ui8x].pcText_english, 2, ui8YMin, false, false, false, true, false, true);
				}
			}
			//
			// Increment Y coordinate of the top left corner of the menu item to be displayed
			//
			ui8YMin += MENU_ITEM_HEIGHT;

			ui8x++;
		}
    }


	return 0;
}

/******************************************************************************
 * FunctionName: displayUpgradeFirmwareMode
 *
 * Function Description:
 * This function handle MODE key press events during firmware upgrade process.
 *
 * Function Parameter: void
 *
 * Function Returns:
 * 0 on success
 *
 ********************************************************************************/
uint8_t firmware_mode_cyw=0;
uint8_t displayUpgradeFirmwareMode(void)
{
	if(gKeysStatus.bits.Key_Mode_pressed)
	{

		//gKeysStatus.bits.Key_Mode_pressed = 0;
		gUpgradeStage=0;
		if(psActiveMenu == &gBoardsMenu)
		{
			psActiveFunctionalBlock = &gsMenuFunctionalBlock;

			//psActiveFunctionalBlock = &gsMenuFunctionalBlock;
			//			psActiveFunctionalBlock->pfnPaintFirstScreen();
			//psActiveFunctionalBlock = &gsMenuFunctionalBlock;
		    //psActiveFunctionalBlock->pfnPaintFirstScreen();//addcyw
		}
		else if(psActiveMenu == &gUpgradeFirmwareMenu)
		{
			gKeysStatus.bits.Key_Mode_pressed = 0;
			psActiveMenu = &gBoardsMenu;

			if(0 == gUpgradeStage)
			{
				displayActiveMenuItems();
			}
			else if(1 == gUpgradeStage)
			{
				if(psActiveMenu->ui8FocusIndex == 0)
				{
					memcpy(cFilenameInitials, "Cont", 4);
					getUpgradeFileNameListDisplayBoard();
					displayActiveMenuItems();
				}
				else if(psActiveMenu->ui8FocusIndex == 1)
				{
					memcpy(cFilenameInitials, "Disp", 4);
					getUpgradeFileNameListDisplayBoard();
					displayActiveMenuItems();
				}
				else if(psActiveMenu->ui8FocusIndex == 2)
				{
					memcpy(cFilenameInitials, "Driv", 4);
					getUpgradeFileNameListDisplayBoard();
					displayActiveMenuItems();
				}

				gUpgradeStage = 0;
			}
		}
	}

	return 0;
}

/******************************************************************************
 * FunctionName: displayUpgradeFirmwareEnter
 *
 * Function Description:
 * This function handle ENTER key press events during firmware upgrade process.
 *
 * Function Parameter: void
 *
 * Function Returns:
 * 0 on success
 *
 ********************************************************************************/
uint8_t displayUpgradeFirmwareEnter(void)
{
	uint32_t tempData = 0;
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;

		if(1 == gstControlBoardStatus.bits.controlBoardBootLoader)
		{
			jumpToBootLoader();
		}
		else if(psActiveMenu == &gBoardsMenu)
		{
			if(psActiveMenu->ui8FocusIndex == 1)	// control board
			{
				if(0 == gUpgradeStage)
				{
					if(
						(gstDriveBoardStatus.bits.shutterUpperLimit) ||
						(gstDriveBoardStatus.bits.shutterLowerLimit) ||
						(gstDriveBoardStatus.bits.shutterStopped)
					)
					{
						gUpgradeStage = 1;

						GrRectFIllBolymin(0, 126, 0, 63, true, true);
						//displayText("ARE YOU SURE TO", 2, 0, false, false, false, false, true, false);
						//displayText("UPGRADE CONTROL", 2, 16, false, false, false, false, true, false);
						//displayText("BOARD FIRMWARE", 2, 32, false, false, false, false, false, false);
						if(gu8_language == Japanese_IDX)
						{
						displayText("CユニットノFWヲ", 2, 0, false, false, false, false, false, false);
						displayText("カキカエシマスカ?", 2, 16, false, false, false, false, false, false);
						displayText("Y:<ENT> N:<MOD>", 2, 48, false, false, false, false, false, true);
						}
						else
						{
							displayText("ARE YOU SURE TO", 2, 0, false, false, false, false, false, true);
							displayText("UPGRADE CONTROL", 2, 16, false, false, false, false, false, true);
							displayText("BOARD FIRMWARE", 2, 32, false, false, false, false, false, true);
							displayText("Y:<ENT> N:<MOD>", 2, 48, false, false, false, false, false, true);
						}
					}
					else
					{
						GrRectFIllBolymin(0, 126, 0, 63, true, true);
						//displayText("CANNOT UPGRADE", 2, 0, false, false, false, false, false, false);
						//displayText("FIRMWARE. PLEASE", 1, 16, false, false, false, false, true, false);
						//displayText("STOP OPERATION", 2, 32, false, false, false, false, false, false);

						if(gu8_language == Japanese_IDX)
						{
							displayText("CANNOT UPGRADE", 2, 0, false, false, false, false, false, true);
							displayText("FIRMWARE. PLEASE", 1, 16, false, false, false, false, false, true);
							displayText("STOP OPERATION", 2, 32, false, false, false, false, false, true);
							displayText("PRESS <MODE>", 2, 48, false, false, false, false, false, true);
						}
						else
						{
							displayText("CANNOT UPGRADE", 2, 0, false, false, false, false, false, true);
							displayText("FIRMWARE. PLEASE", 1, 16, false, false, false, false, false, true);
							displayText("STOP OPERATION", 2, 32, false, false, false, false, false, true);
							displayText("PRESS <MODE>", 2, 48, false, false, false, false, false, true);
						}

					}
				}
				else if( (gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) &&
						 (1 == gUpgradeStage)
					 )
				{
					//	Set command flag to upgrade control board firmware
					gstUMtoCMdatabase.commandToControlBoard.bits.firmwareUpgrade = 1;
					gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
					gstUMtoCMdatabase.destination = eDestControlBoard;

					//
					// Display wait message on screen
					//
					GrRectFIllBolymin(0, 126, 0, 63, true, true);
					//displayText("PLEASE WAIT....", 2, 0, false, false, false, false);
					if(gu8_language == Japanese_IDX)
					displayText("オマチクダサイ....", 2, 0, false, false, false, false, false, false);
					else
					displayText("PLEASE WAIT....", 2, 0, false, false, false, false,false,true);

					psActiveMenu = NULL;	// this will restrict any further enter key press event
				}
			}

			else if(psActiveMenu->ui8FocusIndex == 2)	// display board
			{
				if(0 == gUpgradeStage)
				{
					gUpgradeStage = 1;

					GrRectFIllBolymin(0, 126, 0, 63, true, true);
					//displayText("ARE YOU SURE TO", 2, 0, false, false, false, false, true, false);
					//displayText("UPGRADE DISPLAY", 2, 16, false, false, false, false, true, false);
					//displayText("BOARD FIRMWARE", 2, 32, false, false, false, false, false, false);
					if(gu8_language == Japanese_IDX)
					{
					displayText("DユニットノFWヲ", 2, 0, false, false, false, false, false, false);
					displayText("カキカエシマスカ?", 2, 16, false, false, false, false, false, false);
					displayText("Y:<ENT> N:<MOD>", 2, 48, false, false, false, false, false, true);
					}
					else
					{
					displayText("ARE YOU SURE TO", 2, 0, false, false, false, false, false, true);
					displayText("UPGRADE DISPLAY", 2, 16, false, false, false, false, false, true);
					displayText("BOARD FIRMWARE", 2, 32, false, false, false, false, false, true);
					displayText("Y:<ENT> N:<MOD>", 2, 48, false, false, false, false, false, true);
					}
				}
				else if(1 == gUpgradeStage)
				{
					//
					// Set flag in flash for firmware upgrade initiation.
					//
					tempData = 0x01;	//	Display board firmware upgrade initiated from user menu

					FlashErase(FLASH_FIRMWARE_UPGRADE_FROM_APP_FLAG);

					writeApplicationAreaInFlash(&tempData, FLASH_FIRMWARE_UPGRADE_FROM_APP_FLAG, FLASH_ONE_WORD_SIZE);
/*					memcpy((void*)&gui8UpgradeViaApplicationStatusFlags, (const void*)0x387F4, sizeof(gui8UpgradeViaApplicationStatusFlags));
					gui8UpgradeViaApplicationStatusFlags = 0x01;
					FlashProgram(&gui8UpgradeViaApplicationStatusFlags, 0x387F4, sizeof(gui8UpgradeViaApplicationStatusFlags));*/

					psActiveMenu = NULL;	// this will restrict any further enter key press event

					jumpToBootLoader();
				}

			}

			else if(psActiveMenu->ui8FocusIndex == 3)	// drive board
			{
				if(0 == gUpgradeStage)
				{
					if(
						(gstDriveBoardStatus.bits.shutterUpperLimit) ||
						(gstDriveBoardStatus.bits.shutterLowerLimit) ||
						(gstDriveBoardStatus.bits.shutterStopped)
					)
					{
						gUpgradeStage = 1;

						GrRectFIllBolymin(0, 126, 0, 63, true, true);

						if(gu8_language == Japanese_IDX)
						{
						displayText("MユニットノFWヲ", 2, 0, false, false,false, false, false, false);
						displayText("カキカエシマスカ?", 2, 16, false, false,false, false, false, false);

						displayText("Y:<ENT> N:<MOD>", 2, 48, false, false, false, false, false, true);
						}
						else
						{
						displayText("ARE YOU SURE TO", 2, 0, false, false,false, false, false, true);
						displayText("UPGRADE DRIVE", 2, 16, false, false,false, false, false, true);
						displayText("BOARD FIRMWARE", 2, 32, false, false,false, false, false, true);
						displayText("Y:<ENT> N:<MOD>", 2, 48, false, false, false, false, false, true);
						}
					}
					else
					{
						GrRectFIllBolymin(0, 126, 0, 63, true, true);
						displayText("CANNOT UPGRADE", 2, 0, false, false,false, false, false, true);
						displayText("FIRMWARE. PLEASE", 1, 16, false,false, false, false, false, true);
						displayText("STOP OPERATION", 2, 32, false, false,false, false, false, true);
						displayText("PRESS <MODE>", 2, 48, false, false, false, false, false, true);
					}
				}
				else if( (gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) &&
						 (1 == gUpgradeStage)
					 )
				{
					//	Set command flag to upgrade drive board firmware
					gstUMtoCMdatabase.commandToControlBoard.bits.firmwareUpgrade = 1;
					gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
					gstUMtoCMdatabase.destination = eDestDriveBoard;

					//
					// Display wait message on screen
					//
					GrRectFIllBolymin(0, 126, 0, 63, true, true);
					//displayText("PLEASE WAIT....", 2, 0, false, false, false, false);
					if(gu8_language == Japanese_IDX)
					displayText("オマチクダサイ....", 2, 0, false, false, false, false, false, false);
					else
					displayText("PLEASE WAIT....", 2, 0, false, false, false, false,false,true);
					psActiveMenu = NULL;	// this will restrict any further enter key press event
				}
			}
		}
	}

	//
	// Handle command response
	//
	if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
	{
		if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS)
		{
			if(
					(gstUMtoCMdatabase.commandToControlBoard.bits.firmwareUpgrade == 1) &&
					(gui8FirmwareUpgradeCommandResponseReceived == 1)
			)
			{
				gstUMtoCMdatabase.commandToControlBoard.bits.firmwareUpgrade = 0;

				gui8FirmwareUpgradeCommandResponseReceived = 0;

				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;

				//
				// Set flag in flash for firmware upgrade initiation.
				//
/*				memcpy((void*)&gui8UpgradeViaApplicationStatusFlags, (const void*)0x387F4, sizeof(gui8UpgradeViaApplicationStatusFlags));

				if(gstUMtoCMdatabase.destination == eDestControlBoard)
					gui8UpgradeViaApplicationStatusFlags = 0x02;
				if(gstUMtoCMdatabase.destination == eDestDriveBoard)
					gui8UpgradeViaApplicationStatusFlags = 0x04;

				FlashProgram(&gui8UpgradeViaApplicationStatusFlags, 0x387F4, sizeof(gui8UpgradeViaApplicationStatusFlags));*/

//				uartSendTxBuffer(UART_debug,"3",1);
				jumpToBootLoader();
			}
		}

		else if( (gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT) ||
				 (gstUMtoCMdatabase.commandResponseStatus == eFAIL)
				)
		{

			if(gstUMtoCMdatabase.commandToControlBoard.bits.firmwareUpgrade == 1)
			{
				gstUMtoCMdatabase.commandToControlBoard.bits.firmwareUpgrade = 0;

				gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
				gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
			}

		}
	}

	return 0;
}

/*uint8_t displayUpgradeFirmwareEnter(void)
{
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;

		if(1 == diplayBoardFirmwareUpgradeStart)
		{
			//
			// Jump to display board boot loader.
			//
			jumpToBootLoader();
		}

		if(psActiveMenu == &gBoardsMenu)
		{
			if(psActiveMenu->ui8FocusIndex == 0)
			{
				memcpy(cFilenameInitials, "Cont", 4);
				getUpgradeFileNameListDisplayBoard();
				displayActiveMenuItems();
			}
			else if(psActiveMenu->ui8FocusIndex == 1)
			{
				diplayBoardFirmwareUpgradeStart = 1;

				//
				// Clear screen
				//
				GrRectFIllBolymin(0, 126, 0, 63, true, true);

				displayText("ARE YOU SURE TO", 2, 0, false, false, false, false);
				displayText("UPGRADE BOARD", 2, 16, false, false, false, false);
				displayText("FIRMWARE", 2, 32, false, false, false, false);
				displayText("Y:<ENTER> N:<MODE>", 2, 48, false, false, false, false);

			}
			else if(psActiveMenu->ui8FocusIndex == 2)
			{
				memcpy(cFilenameInitials, "Driv", 4);
				getUpgradeFileNameListDisplayBoard();
				displayActiveMenuItems();
			}
		}
		else if(psActiveMenu == &gUpgradeFirmwareMenu)
		{
			if(0 == gUpgradeStage)
			{
				//
				// Clear screen
				//
				GrRectFIllBolymin(0, 126, 0, 63, true, true);

				displayText("ARE YOU SURE TO", 2, 0, false, false, false, false);
				displayText("UPGRADE BOARD", 2, 16, false, false, false, false);
				displayText("FIRMWARE", 2, 32, false, false, false, false);
				displayText("Y:<ENTER> N:<MODE>", 2, 48, false, false, false, false);

				gUpgradeStage = 1;
			}
			else if(1 == gUpgradeStage)
			{
				//
				// Clear screen
				//
				GrRectFIllBolymin(0, 126, 0, 63, true, true);

				displayText("UPGRADING", 2, 0, false, false, false, false);
				displayText("FIRMWARE....", 2, 16, false, false, false, false);

				gCurrentFocusIndex = psActiveMenu->ui8FocusIndex;
				memcpy(fileNameToUpgrade, psActiveMenu->psMenuItems[gCurrentFocusIndex].pcText, 20);
				psActiveMenu->psMenuItems[gCurrentFocusIndex].childFunctionalBlock->pfnEnter();
			}
		}
	}

	return 0;
}*/

/******************************************************************************
 * FunctionName: upgradeProcessDefaultFunction
 *
 * Function Description:
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
uint8_t upgradeProcessDefaultFunction(void)
{
	// do nothing
	return 0;
}

/******************************************************************************
 * FunctionName: upgradeProcessEnter
 *
 * Function Description:
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
uint8_t upgradeProcessEnter()
{
/*	int8_t li8ReturnValue = 0;
	uint8_t lui8Result = 0;
	//fileNameToUpgrade
	li8ReturnValue = displayBoardFirmwareUpgrade(fileNameToUpgrade);

	lui8Result  = (uint8_t) li8ReturnValue | 0x30;

	uartSendTxBuffer(UART_debug,&lui8Result,1);

	if(SUCCESS == li8ReturnValue)
	{
#if 1
		//
		//	Jump to application image in flash
		//
		jumpToApplicationCode();
# endif
	}*/

	return 0;
}


/******************************************************************************
 * Define Display firmware upgrade functional block object
*********************************************************************************/
stInternalFunctions gsDisplayUpgradeFirmwareFunctionalBlock =
{
	0,
	0,
	displayUpgradeFirmwarePaint,
	displayUpgradeFirmwareRunTime,
	displayUpgradeFirmwareUp,
	displayUpgradeFirmwareDown,
	displayUpgradeFirmwareMode,
	displayUpgradeFirmwareEnter
};

/******************************************************************************
 * Define Upgrade process functional block object
*********************************************************************************/
stInternalFunctions gsUpgradeProcessFunctionalBlock =
{
	0,
	0,
	upgradeProcessDefaultFunction,
	upgradeProcessDefaultFunction,
	upgradeProcessDefaultFunction,
	upgradeProcessDefaultFunction,
	upgradeProcessDefaultFunction,
	upgradeProcessEnter
};
