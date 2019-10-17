/*********************************************************************************
 * FileName: paraminit.c
 * Description: Code for Parameter Initialization screen
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
 *  Revision        Date                  Name                      Comments
 *      0.1D    20/06/2014          iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/gpio.h>
#include "Middleware/bolymindisplay.h"
#include "parameterlist.h"
#include "Middleware/debounce.h"
#include "userinterface.h"
#include "Middleware/paramdatabase.h"

extern uint8_t KEY_PRESS_3SEC_MODE_FLAG_CYW;
extern uint8_t KEY_PRESS_3SEC_STOP_FLAG_CYW;
extern uint16_t gu16_lcdlight;
extern volatile uint32_t g_ui32TickCount;
extern  const uint16_t gu16_backlight_DEF;
//uint32_t backlight_count_now;
uint32_t backlight_count_his=0;
uint8_t Flag_lcdbackon = 0;

//uint8_t  Flag_lcd_light_on =0;
extern uint8_t flag_out_setting_cyw;
extern uint8_t setting_flag;
void guest_reinit(void);

void Set_lcdlightON(void)
{
	//if(Flag_lcd_light_on ==0)
	//{
	//lcd_init();
	LCD_BACKLIGHT_ON();
	backlight_count_his = g_ui32TickCount;
	Flag_lcdbackon = 1;
	//Flag_lcd_light_on = 1;
	//}
}

void Set_lcdlightOFF(void)
{
	uint32_t backlight_count_now=0;
	//uint32_t tp_g_ui32TickCount=0;

	if(Flag_lcdbackon == 1)
	{
	//	tp_g_ui32TickCount = g_ui32TickCount;
	//if(tp_g_ui32TickCount!=backlight_count_his)
	//{
		backlight_count_now = get_timego( backlight_count_his);
	//}
	if(gu16_lcdlight > gsParamDatabase[Para_LcdBackLight_Index_cyw].valueTypeEntities.maxVal.ui32Val)
	{
		gu16_lcdlight =gu16_backlight_DEF;
	}
	if((backlight_count_now >= (uint32_t)(gu16_lcdlight*100))&&(gu16_lcdlight!=0))
	{
		LCD_BACKLIGHT_OFF();
		Flag_lcdbackon = 0;
		//Flag_lcd_light_on = 0;
	}
	}

	if(KEY_PRESS_3SEC_STOP_FLAG_CYW == 1)
	{
		KEY_PRESS_3SEC_STOP_FLAG_CYW = 0;
		lcd_init();
		psActiveFunctionalBlock->pfnPaintFirstScreen();
	}
	//if(KEY_PRESS_3SEC_MODE_FLAG_CYW==1)
	//{
	//	KEY_PRESS_3SEC_MODE_FLAG_CYW =0;
	//	lcd_init();
	//}

}


void Out_of_settingmode_cyw(void)
{
	if((flag_out_setting_cyw == 1)||(flag_out_setting_cyw == 2))
	{
	if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
		{
			if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS)
			{
				if(gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus == 1)
				{
					gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus = 0;
					gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
					gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
					gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
					gstUMtoCMdatabase.commandToControlBoard.val = 0;
					psActiveMenu = &gSettingsMenu;
								psActiveMenu->ui8FocusIndex = 0;
								psActiveMenu = &gsMainMenu;
								psActiveMenu->ui8FocusIndex = 0;

								if(flag_out_setting_cyw==1)
								psActiveFunctionalBlock = &gsInstallationFunctionalBlock;
								if(flag_out_setting_cyw==2)
								psActiveFunctionalBlock = &gsApertureheightFunctionalBlock;

								//psActiveFunctionalBlock = &gsHomeScreenFunctionalBlock;
								psActiveFunctionalBlock->pfnPaintFirstScreen();
								isMenutitle = 0;
								//gHighlightedItemIndex =0;
								gParameterFocusIndex = 0;
								gTotalParamItemsToRender = 0;
								gHighlightedItemIndex = 0;
								gCurrentStartIndex = 0;
								gParamListRenderStarted = 0;

								guest_reinit();
								setting_flag = 1;//disable guesure
								flag_out_setting_cyw = 0;
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
					gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
					gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
					gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;
					gstUMtoCMdatabase.commandToControlBoard.val = 0;
					psActiveMenu = &gSettingsMenu;
								psActiveMenu->ui8FocusIndex = 0;
								psActiveMenu = &gsMainMenu;
								psActiveMenu->ui8FocusIndex = 0;

								if(flag_out_setting_cyw==1)
								psActiveFunctionalBlock = &gsInstallationFunctionalBlock;
								if(flag_out_setting_cyw==2)
								psActiveFunctionalBlock = &gsApertureheightFunctionalBlock;

								//psActiveFunctionalBlock = &gsHomeScreenFunctionalBlock;
								psActiveFunctionalBlock->pfnPaintFirstScreen();
								isMenutitle = 0;
								//gHighlightedItemIndex =0;
								gParameterFocusIndex = 0;
								gTotalParamItemsToRender = 0;
								gHighlightedItemIndex = 0;
								gCurrentStartIndex = 0;
								gParamListRenderStarted = 0;

								guest_reinit();
								setting_flag = 1;
								flag_out_setting_cyw = 0;//disable guesure
				}
			}
		}
	}
}
