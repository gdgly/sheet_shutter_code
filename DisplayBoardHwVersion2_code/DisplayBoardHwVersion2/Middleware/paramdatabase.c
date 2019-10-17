/*********************************************************************************
* FileName: paramdatabase.c
* Description:
* This header file contains definitions for Parameter Database mechanism.
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
 *  	0.5D	18/08/2014									Handled Error Code
 *  	0.4D	23/07/2014									Addition of Parameters 800 and 804
 *  	0.3D	19/05/2014									Addition of paramLoadDefault()
 *  	0.2D	16/05/2014									Addition of Display DB
 *  	0.1D	06/05/2014      	iGATE Offshore team     Initial Creation
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/sw_crc.h>
#include "Middleware/eeprom.h"
#include "Middleware/uartstdio.h"
#include "paramdatabase.h"
#include "Application/interTaskCommunication.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/




/****************************************************************************
 *  Global variables
****************************************************************************/
// Display Parameter Size Table
const uint8_t guc_Paramsize_Disp[_NO_OF_PARAMS] =
{
		28,	28,	28,	28,	28,	28,	28,	28,	28,	28,	1, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 1, 4, 4, 2, 2, 2,
		28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 1,  1, 4 , 4 ,  4,  4,  4,  4,  4,  4,  4, 4, 4 , 4 ,  4,  4,//20170414      201703_No.31
		4,  4,  4,  4,  4, 4
};

// Display Parameter Address Table
//Testing 13Jun 14   2CRC
const uint32_t gui_ParamAddr_Disp[_NO_OF_PARAMS] =
{
		0, 30, 60, 90, 120, 150, 180, 210, 240, 270, 300, 303, 321, 339, 357, 375, 393, 411, 429, 447, 465, 483, 486, 492, 498, 502, 506,
		510,540,570,600,630,660,690,720,750,780,810, 813, 816, 822, 828, 834, 840 , 846,852,858,864,870,876,882,888,894,900,906,912, 918,
		924,930
};////20170414      201703_No.31

// Display Board Parameters Number Table
const uint16_t gui_ParamNo_Disp[_NO_OF_PARAMS] =
{
		30,	31,	32,	33,	34,	35,	36,	37,	38,	39,	40,	50,	51,	52,	53,	54,	55,	56,	57,	58,	59,	60,	452, 453, 800, 804,26,
		900,901,902,903,904,905,906,907,908,909,41,42
};

//Default Values for Display Parameter. This is saved in Flash.
const struct stControlAnomaly gstAH_anomhist_1_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_2_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_3_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_4_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_5_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_6_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_7_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_8_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_9_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_10_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const uint8_t gu8_erase_anomhist_DEF = 0;
const struct stChangeSettingHistory gstCSH_chgsethist_1_DEF = {0x0000,0x00000000,0x00000000,0x00000000};
const struct stChangeSettingHistory gstCSH_chgsethist_2_DEF = {0x0000,0x00000000,0x00000000,0x00000000};
const struct stChangeSettingHistory gstCSH_chgsethist_3_DEF = {0x0000,0x00000000,0x00000000,0x00000000};
const struct stChangeSettingHistory gstCSH_chgsethist_4_DEF = {0x0000,0x00000000,0x00000000,0x00000000};
const struct stChangeSettingHistory gstCSH_chgsethist_5_DEF = {0x0000,0x00000000,0x00000000,0x00000000};
const struct stChangeSettingHistory gstCSH_chgsethist_6_DEF = {0x0000,0x00000000,0x00000000,0x00000000};
const struct stChangeSettingHistory gstCSH_chgsethist_7_DEF = {0x0000,0x00000000,0x00000000,0x00000000};
const struct stChangeSettingHistory gstCSH_chgsethist_8_DEF = {0x0000,0x00000000,0x00000000,0x00000000};
const struct stChangeSettingHistory gstCSH_chgsethist_9_DEF = {0x0000,0x00000000,0x00000000,0x00000000};
const struct stChangeSettingHistory gstCSH_chgsethist_10_DEF = {0x0000,0x00000000,0x00000000,0x00000000};
const uint8_t gu8_erase_chgsethist_DEF = 0;
const uint32_t gu32_disp_fwver_DEF = 0x00000005;
//const uint32_t gu32_disp_hwver_DEF = 0x00000001;
const uint16_t gu16_crcfail_di2ct_DEF = 0;
const uint16_t gu16_commbuffof_di2ct_DEF = 0;
const uint8_t gu8_guestue_DEF = 1;//20160918 from 0 to 1
const uint16_t gu16_backlight_DEF = 30;
const uint8_t gu8_language_DEF = 0;
const struct stControlAnomaly gstAH_anomhist_11_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_12_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_13_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_14_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_15_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_16_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_17_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_18_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_19_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
const struct stControlAnomaly gstAH_anomhist_20_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
//20170414      201703_No.31 start
const uint32_t gu32_Anomaly_OP_1_DEF = 0;
const uint32_t gu32_Anomaly_OP_2_DEF = 0;
const uint32_t gu32_Anomaly_OP_3_DEF = 0;
const uint32_t gu32_Anomaly_OP_4_DEF = 0;
const uint32_t gu32_Anomaly_OP_5_DEF = 0;
const uint32_t gu32_Anomaly_OP_6_DEF = 0;
const uint32_t gu32_Anomaly_OP_7_DEF = 0;
const uint32_t gu32_Anomaly_OP_8_DEF = 0;
const uint32_t gu32_Anomaly_OP_9_DEF = 0;
const uint32_t gu32_Anomaly_OP_10_DEF = 0;
const uint32_t gu32_Anomaly_OP_11_DEF = 0;
const uint32_t gu32_Anomaly_OP_12_DEF = 0;
const uint32_t gu32_Anomaly_OP_13_DEF = 0;
const uint32_t gu32_Anomaly_OP_14_DEF = 0;
const uint32_t gu32_Anomaly_OP_15_DEF = 0;
const uint32_t gu32_Anomaly_OP_16_DEF = 0;
const uint32_t gu32_Anomaly_OP_17_DEF = 0;
const uint32_t gu32_Anomaly_OP_18_DEF = 0;
const uint32_t gu32_Anomaly_OP_19_DEF = 0;
const uint32_t gu32_Anomaly_OP_20_DEF = 0;
//20170414      201703_No.31 end

//Pointer to Default values in Display Board
uint8_t * const guc_ptrParamDefault_Disp[_NO_OF_PARAMS] =
{
		(uint8_t*)&gstAH_anomhist_1_DEF,
		(uint8_t*)&gstAH_anomhist_2_DEF,
		(uint8_t*)&gstAH_anomhist_3_DEF,
		(uint8_t*)&gstAH_anomhist_4_DEF,
		(uint8_t*)&gstAH_anomhist_5_DEF,
		(uint8_t*)&gstAH_anomhist_6_DEF,
		(uint8_t*)&gstAH_anomhist_7_DEF,
		(uint8_t*)&gstAH_anomhist_8_DEF,
		(uint8_t*)&gstAH_anomhist_9_DEF,
		(uint8_t*)&gstAH_anomhist_10_DEF,
		(uint8_t*)&gu8_erase_anomhist_DEF,
		(uint8_t*)&gstCSH_chgsethist_1_DEF,
		(uint8_t*)&gstCSH_chgsethist_2_DEF,
		(uint8_t*)&gstCSH_chgsethist_3_DEF,
		(uint8_t*)&gstCSH_chgsethist_4_DEF,
		(uint8_t*)&gstCSH_chgsethist_5_DEF,
		(uint8_t*)&gstCSH_chgsethist_6_DEF,
		(uint8_t*)&gstCSH_chgsethist_7_DEF,
		(uint8_t*)&gstCSH_chgsethist_8_DEF,
		(uint8_t*)&gstCSH_chgsethist_9_DEF,
		(uint8_t*)&gstCSH_chgsethist_10_DEF,
		(uint8_t*)&gu8_erase_chgsethist_DEF,
		(uint8_t*)&gu32_disp_fwver_DEF,
		0, //		(uint8_t*)&gu32_disp_hwver_DEF,
		(uint8_t*)&gu16_crcfail_di2ct_DEF,
		(uint8_t*)&gu16_commbuffof_di2ct_DEF,
		(uint8_t*)&gu16_backlight_DEF,//lcd back 10s  A026
		(uint8_t*)&gstAH_anomhist_11_DEF,
		(uint8_t*)&gstAH_anomhist_12_DEF,
		(uint8_t*)&gstAH_anomhist_13_DEF,
		(uint8_t*)&gstAH_anomhist_14_DEF,
		(uint8_t*)&gstAH_anomhist_15_DEF,
		(uint8_t*)&gstAH_anomhist_16_DEF,
		(uint8_t*)&gstAH_anomhist_17_DEF,
		(uint8_t*)&gstAH_anomhist_18_DEF,
		(uint8_t*)&gstAH_anomhist_19_DEF,
		(uint8_t*)&gstAH_anomhist_20_DEF,
		(uint8_t*)&gu8_guestue_DEF,
		(uint8_t*)&gu8_language_DEF,
		//20170414      201703_No.31 start
		(uint8_t*)&gu32_Anomaly_OP_1_DEF,
		(uint8_t*)&gu32_Anomaly_OP_2_DEF,
		(uint8_t*)&gu32_Anomaly_OP_3_DEF,
		(uint8_t*)&gu32_Anomaly_OP_4_DEF,
		(uint8_t*)&gu32_Anomaly_OP_5_DEF,
		(uint8_t*)&gu32_Anomaly_OP_6_DEF,
		(uint8_t*)&gu32_Anomaly_OP_7_DEF,
		(uint8_t*)&gu32_Anomaly_OP_8_DEF,
		(uint8_t*)&gu32_Anomaly_OP_9_DEF,
		(uint8_t*)&gu32_Anomaly_OP_10_DEF,
		(uint8_t*)&gu32_Anomaly_OP_11_DEF,
		(uint8_t*)&gu32_Anomaly_OP_12_DEF,
		(uint8_t*)&gu32_Anomaly_OP_13_DEF,
		(uint8_t*)&gu32_Anomaly_OP_14_DEF,
		(uint8_t*)&gu32_Anomaly_OP_15_DEF,
		(uint8_t*)&gu32_Anomaly_OP_16_DEF,
		(uint8_t*)&gu32_Anomaly_OP_17_DEF,
		(uint8_t*)&gu32_Anomaly_OP_18_DEF,
		(uint8_t*)&gu32_Anomaly_OP_19_DEF,
		(uint8_t*)&gu32_Anomaly_OP_20_DEF,
		//20170414      201703_No.31 end
};

//Parameter Global Variables
struct stControlAnomaly gstAH_anomhist_1 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_2 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_3 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_4 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_5 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_6 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_7 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_8 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_9 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_10 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
uint8_t gu8_erase_anomhist = 0;
struct stChangeSettingHistory gstCSH_chgsethist_1 = {0x0000,0x00000000,0x00000000,0x00000000};
struct stChangeSettingHistory gstCSH_chgsethist_2 = {0x0000,0x00000000,0x00000000,0x00000000};
struct stChangeSettingHistory gstCSH_chgsethist_3 = {0x0000,0x00000000,0x00000000,0x00000000};
struct stChangeSettingHistory gstCSH_chgsethist_4 = {0x0000,0x00000000,0x00000000,0x00000000};
struct stChangeSettingHistory gstCSH_chgsethist_5 = {0x0000,0x00000000,0x00000000,0x00000000};
struct stChangeSettingHistory gstCSH_chgsethist_6 = {0x0000,0x00000000,0x00000000,0x00000000};
struct stChangeSettingHistory gstCSH_chgsethist_7 = {0x0000,0x00000000,0x00000000,0x00000000};
struct stChangeSettingHistory gstCSH_chgsethist_8 = {0x0000,0x00000000,0x00000000,0x00000000};
struct stChangeSettingHistory gstCSH_chgsethist_9 = {0x0000,0x00000000,0x00000000,0x00000000};
struct stChangeSettingHistory gstCSH_chgsethist_10 = {0x0000,0x00000000,0x00000000,0x00000000};
uint8_t gu8_erase_chgsethist = 0;
uint32_t gu32_disp_fwver = 0;
uint32_t gu32_disp_hwver = 0;
uint16_t gu16_crcfail_di2ct = 0;
uint16_t gu16_commbuffof_di2ct = 0;
extern unsigned char menu_gesture_flag_cyw;
uint16_t gu16_lcdlight = 0;
uint8_t gu8_language = 0;
struct stControlAnomaly gstAH_anomhist_11 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_12 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_13 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_14 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_15 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_16 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_17 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_18 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_19 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};
struct stControlAnomaly gstAH_anomhist_20 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00000000};



//pointer to Global Param values
uint8_t *guc_ptrParam_Disp[_NO_OF_PARAMS] = {
		(uint8_t*)&gstAH_anomhist_1,
		(uint8_t*)&gstAH_anomhist_2,
		(uint8_t*)&gstAH_anomhist_3,
		(uint8_t*)&gstAH_anomhist_4,
		(uint8_t*)&gstAH_anomhist_5,
		(uint8_t*)&gstAH_anomhist_6,
		(uint8_t*)&gstAH_anomhist_7,
		(uint8_t*)&gstAH_anomhist_8,
		(uint8_t*)&gstAH_anomhist_9,
		(uint8_t*)&gstAH_anomhist_10,
		(uint8_t*)&gu8_erase_anomhist,
		(uint8_t*)&gstCSH_chgsethist_1,
		(uint8_t*)&gstCSH_chgsethist_2,
		(uint8_t*)&gstCSH_chgsethist_3,
		(uint8_t*)&gstCSH_chgsethist_4,
		(uint8_t*)&gstCSH_chgsethist_5,
		(uint8_t*)&gstCSH_chgsethist_6,
		(uint8_t*)&gstCSH_chgsethist_7,
		(uint8_t*)&gstCSH_chgsethist_8,
		(uint8_t*)&gstCSH_chgsethist_9,
		(uint8_t*)&gstCSH_chgsethist_10,
		(uint8_t*)&gu8_erase_chgsethist,
		(uint8_t*)&gu32_disp_fwver,
		(uint8_t*)&gu32_disp_hwver,
		(uint8_t*)&gu16_crcfail_di2ct,
		(uint8_t*)&gu16_commbuffof_di2ct,
		(uint8_t*)&gu16_lcdlight,
		(uint8_t*)&gstAH_anomhist_11,
		(uint8_t*)&gstAH_anomhist_12,
		(uint8_t*)&gstAH_anomhist_13,
		(uint8_t*)&gstAH_anomhist_14,
		(uint8_t*)&gstAH_anomhist_15,
		(uint8_t*)&gstAH_anomhist_16,
		(uint8_t*)&gstAH_anomhist_17,
		(uint8_t*)&gstAH_anomhist_18,
		(uint8_t*)&gstAH_anomhist_19,
		(uint8_t*)&gstAH_anomhist_20,
		(uint8_t*)&menu_gesture_flag_cyw,
		(uint8_t*)&gu8_language
};

/****************************************************************************/


/****************************************************************************
 *  Function definitions for this file:
****************************************************************************/
uint8_t initParameterDB(PARAM_DISP paramindex)
{
	uint8_t psize = guc_Paramsize_Disp[paramindex];
	uint8_t param_eeprom_val[_MAX_EEPROM_READVALSIZE];
	uint16_t paramreadcrc;
	uint16_t calccrc;
	uint8_t iSize;
	uint8_t returnfuncid;

	memset(param_eeprom_val, 0, sizeof(param_eeprom_val));

	if(guc_ptrParamDefault_Disp[paramindex]) {
		if(EEPROMReadByte(param_eeprom_val, gui_ParamAddr_Disp[paramindex], psize+2)) {
			//		UARTprintf("EEPROM_READ_ERROR");
			//Log Error
			return _ERR_EEPROM;
		}
		else;

		//Extract CRC information from read value
		paramreadcrc = (((uint16_t)param_eeprom_val[psize] & 0xFFFF) << 8) |
				param_eeprom_val[psize+1];

		//Calculate CRC of read data value
		calccrc = ROM_Crc16(0, param_eeprom_val, psize);

		if(calccrc != paramreadcrc) {
			if(paramLoadDefault(paramindex) == _SUCCESS)
				returnfuncid = _SUCCESS_DEFAULT;
			else
				return _ERR_EEPROM;
		}
		else {
			//Update global param variable
			if (guc_ptrParam_Disp[paramindex])  //Check if not null
				for (iSize = 0; iSize < psize; iSize++) {
					*(guc_ptrParam_Disp[paramindex] + (psize - 1) - iSize) =
							param_eeprom_val[iSize];
				}

			returnfuncid = _SUCCESS_EEPROM;
		}
	}
	else {
		if (guc_ptrParam_Disp[paramindex])  {//Check if not null
			for (iSize = 0; iSize < psize; iSize++)
				*(guc_ptrParam_Disp[paramindex] + (psize - 1) - iSize) = 0;
			returnfuncid = _SUCCESS_DEFAULT;
		}
		else
			returnfuncid = _ERR_PARAMNOTFOUND;
	}

	return returnfuncid;
}
/*****************************************************************************/

uint8_t writeParameterUpdateInDB(PARAM_DISP paramindex,uint8_t* ParamValue)
{
	uint8_t psize = guc_Paramsize_Disp[paramindex];
	uint8_t paramEEP_Write_Val[_MAX_EEPROM_READVALSIZE];
	uint16_t calccrc;
	uint8_t iSize;

	memset(paramEEP_Write_Val, 0, sizeof(paramEEP_Write_Val));

	//Add code to check if Default Value is present or not

	for (iSize = 0; iSize < psize; iSize++) {
		//		paramEEP_Write_Val[iSize] = ParamValue[iSize]; //Disabled for debug
		paramEEP_Write_Val[iSize] = *(ParamValue + (psize-1)-iSize);
	}

	if(guc_ptrParamDefault_Disp[paramindex]) {
		calccrc = ROM_Crc16(0, paramEEP_Write_Val, psize);

		paramEEP_Write_Val[psize] = (calccrc >> 8) & 0xFF;
		paramEEP_Write_Val[psize+1] = calccrc & 0xFF;


		//Write value into EEPROM
		if(EEPROMProgramByte(paramEEP_Write_Val, gui_ParamAddr_Disp[paramindex], psize+2))
		{
			//		UARTprintf("EEPROM_WRITE_ERROR");
			//Log Error
			return _ERR_EEPROM;
		}
		else;
	}
	//Update global param variable
	if(guc_ptrParam_Disp[paramindex]) //Check if not null
		for (iSize = 0; iSize < psize; iSize++) {
			*(guc_ptrParam_Disp[paramindex] + (psize-1)-iSize) = paramEEP_Write_Val[iSize];
		}

	return _SUCCESS;
}
/*****************************************************************************/

uint8_t readParameterFromDB(PARAM_DISP paramindex,uint8_t* readParamValue)
{
	uint8_t psize = guc_Paramsize_Disp[paramindex];
	uint8_t param_EEP_Read_Val[_MAX_EEPROM_READVALSIZE];
	uint16_t calccrc, paramreadcrc;
	uint8_t iSize;

	if(guc_ptrParam_Disp[paramindex]) { //Check if not null

		for (iSize = 0; iSize < psize; iSize++) {
//			readParamValue[(psize-1)-iSize] = *(guc_ptrParam[paramindex] + (psize-1)-iSize);
			readParamValue[iSize] = *(guc_ptrParam_Disp[paramindex] + iSize);
		}
	}
	else {
		//Read from EEPROM
		memset(param_EEP_Read_Val, 0, sizeof(param_EEP_Read_Val));

		if(EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Disp[paramindex], psize+2)) {
//			UARTprintf("EEPROM_READ_ERROR");
			//Log Error
			return _ERR_EEPROM;
		}
		else;

		//Extract CRC information from read value
		paramreadcrc = (((uint16_t)param_EEP_Read_Val[psize] & 0xFFFF) << 8) |
				param_EEP_Read_Val[psize+1];

		//Calculate CRC of read data value
		calccrc = ROM_Crc16(0, param_EEP_Read_Val, psize);

		if(paramreadcrc == calccrc) {
			gstDisplayProcessorFault.bits.flashImageCRC = 0;
			for (iSize = 0; iSize < psize; iSize++) {
				readParamValue[(psize-1)-iSize] = param_EEP_Read_Val[iSize];
			}
		}
		else { //CRC MISMATCH
//			UARTprintf("CRC_MISMATCH");
			gstDisplayProcessorFault.bits.flashImageCRC = 1;
			return _ERR_PARAMDB_CRC;
		}

	}

	return _SUCCESS;
}
/*****************************************************************************/

#if 0

#if 0
uint8_t computeIndex(PARAM_DISP AHorCSH_Idx)
{
	uint8_t psize = guc_Paramsize_Disp[AHorCSH_Idx];
	uint8_t iSize, mSize;
	uint8_t toWriteIdx = 1;
	uint8_t param_EEP_Read_Val[_MAX_EEPROM_READVALSIZE];

	for (iSize = 0; iSize < _MAX_ANOMALY_LOGS; iSize++)
	{
		//		if(!guc_ptrParam[gui8_AnomalyHistory_ParamIdx + iSize])
		memset(param_EEP_Read_Val, 0, sizeof(param_EEP_Read_Val));
		if(!EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Disp[AHorCSH_Idx + iSize], psize))
		{
//		if(!readParameterFromDB(gui8_AnomalyHistory_ParamIdx + iSize, param_EEP_Read_Val)) {
			for(mSize = 0; mSize < psize; mSize++)
			{
				if(param_EEP_Read_Val[mSize])
				{
					toWriteIdx++;
					break;
				}
			} //for loop
			if(mSize == psize)
				break;
		} //if readParameter
	} //outer for loop

	return ( (toWriteIdx > _MAX_ANOMALY_LOGS)?1:toWriteIdx );
}
#endif



uint8_t computeIndex(PARAM_DISP AHorCSH_Idx)
{
	uint8_t psize = guc_Paramsize_Disp[AHorCSH_Idx];
	uint8_t iSize, mSize;
	uint8_t toWriteIdx = 1;
	uint8_t param_EEP_Read_Val[_MAX_EEPROM_READVALSIZE];

	for (iSize = 0; iSize < _MAX_ANOMALY_LOGS; iSize++)
	{
		//		if(!guc_ptrParam[gui8_AnomalyHistory_ParamIdx + iSize])
		memset(param_EEP_Read_Val, 0, sizeof(param_EEP_Read_Val));
		if(!EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Disp[AHorCSH_Idx + iSize], psize))
		{
//		if(!readParameterFromDB(gui8_AnomalyHistory_ParamIdx + iSize, param_EEP_Read_Val)) {
			for(mSize = 0; mSize < psize; mSize++)
			{
				if(param_EEP_Read_Val[mSize])
				{
					toWriteIdx++;
					break;
				}
			} //for loop
			if(mSize == psize)
				break;
		} //if readParameter
	} //outer for loop

	return ( (toWriteIdx > _MAX_ANOMALY_LOGS)?1:toWriteIdx );
}

#endif

#if 1
uint8_t computeIndex(PARAM_DISP AHorCSH_Idx)
{
	uint8_t iSize;
	uint8_t psize = guc_Paramsize_Disp[AHorCSH_Idx];
	uint8_t param_EEP_Read_Val[_MAX_EEPROM_READVALSIZE];
	//uint8_t param_EEP_Read_Val1[_MAX_EEPROM_READVALSIZE];
	time_t timeStamp,timeStamp1;
	uint8_t toWriteIdx = 1;
    uint8_t Tp_anomaly_offset[_MAX_ANOMALY_LOGS]={
    		A030ANOMHIST_1,A031ANOMHIST_2,A032ANOMHIST_3,A033ANOMHIST_4,A034ANOMHIST_5,
            A035ANOMHIST_6,A036ANOMHIST_7,A037ANOMHIST_8,A038ANOMHIST_9,A039ANOMHIST_10,
            A900ANOMHIST_11,A901ANOMHIST_12,A902ANOMHIST_13,A903ANOMHIST_14,A904ANOMHIST_15,
			 A905ANOMHIST_16,A906ANOMHIST_17,A907ANOMHIST_18,A908ANOMHIST_19,A909ANOMHIST_20};

	for (iSize = 0; iSize < (_MAX_ANOMALY_LOGS - 1); iSize++)
	{
		memset(param_EEP_Read_Val, 0, sizeof(param_EEP_Read_Val));


		// extract the time field
		if (AHorCSH_Idx == A030ANOMHIST_1)
		{
		EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Disp[Tp_anomaly_offset[iSize]], psize);
		timeStamp= param_EEP_Read_Val[24];
		timeStamp = timeStamp << 8;
		timeStamp |= param_EEP_Read_Val[25];
		timeStamp = timeStamp << 8;
		timeStamp |= param_EEP_Read_Val[26];
		timeStamp = timeStamp << 8;
		timeStamp |= param_EEP_Read_Val[27];
		}
		else
		{
		EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Disp[AHorCSH_Idx + iSize], psize);
		timeStamp= param_EEP_Read_Val[0];
		timeStamp = timeStamp << 8;
		timeStamp |= param_EEP_Read_Val[1];
		timeStamp = timeStamp << 8;
		timeStamp |= param_EEP_Read_Val[2];
		timeStamp = timeStamp << 8;
		timeStamp |= param_EEP_Read_Val[3];
		}

		if (timeStamp == 0)
		{
			iSize = _MAX_ANOMALY_LOGS; // break for loop
		}
		else
		{

			toWriteIdx++;
			memset(param_EEP_Read_Val, 0, sizeof(param_EEP_Read_Val));


			// extract the time field
			if (AHorCSH_Idx == A030ANOMHIST_1)
			{
		    EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Disp[Tp_anomaly_offset[iSize+1]], psize);
			timeStamp1= param_EEP_Read_Val[24];
			timeStamp1 = timeStamp1 << 8;
			timeStamp1 |= param_EEP_Read_Val[25];
			timeStamp1 = timeStamp1 << 8;
			timeStamp1 |= param_EEP_Read_Val[26];
			timeStamp1 = timeStamp1 << 8;
			timeStamp1 |= param_EEP_Read_Val[27];
			}
			else
			{
		    EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Disp[AHorCSH_Idx + iSize + 1], psize);
			timeStamp1= param_EEP_Read_Val[0];
			timeStamp1 = timeStamp1 << 8;
			timeStamp1 |= param_EEP_Read_Val[1];
			timeStamp1 = timeStamp1 << 8;
			timeStamp1 |= param_EEP_Read_Val[2];
			timeStamp1 = timeStamp1 << 8;
			timeStamp1 |= param_EEP_Read_Val[3];
			}


			if (timeStamp1 == 0)
			{
			iSize = _MAX_ANOMALY_LOGS; // break for loop
			}
			else
			{

				if (timeStamp1 < timeStamp) // Log 2 date is less than log 1 date
				{
					iSize = _MAX_ANOMALY_LOGS; // break for loop
				}
				else
				{
					if (toWriteIdx == _MAX_ANOMALY_LOGS)
					{
						toWriteIdx = 1;
					}
				}
			}

		}

	} //outer for loop

	return (toWriteIdx);
}
#endif


#if 0
uint8_t computeIndex(PARAM_DISP AHorCSH_Idx)
{
	uint8_t psize = guc_Paramsize_Disp[AHorCSH_Idx];
	uint8_t iSize = 0;

	//uint8_t param_EEP_Read_Val[_MAX_EEPROM_READVALSIZE];
	//uint8_t param_EEP_Read_Val1[_MAX_EEPROM_READVALSIZE];

	struct stChangeSettingHistory changeSettingHistory;
	struct stControlAnomaly anomalyHistory;
	struct stChangeSettingHistory changeSettingHistory1;
	struct stControlAnomaly anomalyHistory1;

	time_t timeStamp,timeStamp1;
	uint8_t toWriteIdx = 1;

	for (iSize = 0; iSize < (_MAX_ANOMALY_LOGS - 1); iSize++)
	{
		if (AHorCSH_Idx == A030ANOMHIST_1)
		{

			memset(&anomalyHistory, 0, sizeof(struct stControlAnomaly));
			EEPROMReadByte((uint8_t *)&anomalyHistory, gui_ParamAddr_Disp[AHorCSH_Idx + iSize], psize);
			// extract the time field
			timeStamp= anomalyHistory.timeStamp;

		}
		else
		{
			memset(&changeSettingHistory, 0, sizeof(struct stChangeSettingHistory));
			EEPROMReadByte((uint8_t *)&changeSettingHistory, gui_ParamAddr_Disp[AHorCSH_Idx + iSize], psize);
			// extract the time field
			timeStamp= changeSettingHistory.timeStamp;

		}
		//memset(param_EEP_Read_Val, 0, sizeof(param_EEP_Read_Val));
		//EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Disp[AHorCSH_Idx + iSize], psize);


		if (timeStamp == 0)
		{
			iSize = _MAX_ANOMALY_LOGS; // break for loop
		}
		else
		{

			toWriteIdx++;
			if (AHorCSH_Idx == A030ANOMHIST_1)
			{

				memset(&anomalyHistory1, 0, sizeof(struct stControlAnomaly));
				EEPROMReadByte((uint8_t *)&anomalyHistory1, gui_ParamAddr_Disp[AHorCSH_Idx + iSize], psize);
				// extract the time field
				timeStamp= anomalyHistory1.timeStamp;

			}
			else
			{
				memset(&changeSettingHistory1, 0, sizeof(struct stChangeSettingHistory));
				EEPROMReadByte((uint8_t *)&changeSettingHistory1, gui_ParamAddr_Disp[AHorCSH_Idx + iSize], psize);
				// extract the time field
				timeStamp= changeSettingHistory1.timeStamp;

			}
			//memset(param_EEP_Read_Val1, 0, sizeof(param_EEP_Read_Val));
			//EEPROMReadByte(param_EEP_Read_Val1, gui_ParamAddr_Disp[AHorCSH_Idx + iSize + 1], psize);

			if (timeStamp == 0)
			{
			iSize = _MAX_ANOMALY_LOGS; // break for loop
			}
			else
			{

				if (timeStamp1 < timeStamp) // Log 2 date is less than log 1 date
				{
					iSize = _MAX_ANOMALY_LOGS; // break for loop
				}
				else
				{
					if (toWriteIdx == _MAX_ANOMALY_LOGS)
					{
						toWriteIdx = 1;
					}
				}
			}

		}

	} //outer for loop

	return (toWriteIdx);
}
#endif

/****************************************************************************/

uint8_t clearLogs(PARAM_DISP AHorCSH_Idx)
{
	uint8_t psize = guc_Paramsize_Disp[AHorCSH_Idx];
	uint8_t iSize, msize;
	uint8_t param_EEP_Write_Val[_MAX_EEPROM_READVALSIZE];
	 uint8_t Tp_anomaly_offset[_MAX_ANOMALY_LOGS]={A030ANOMHIST_1,A031ANOMHIST_2,A032ANOMHIST_3,A033ANOMHIST_4,A034ANOMHIST_5,
	            A035ANOMHIST_6,A036ANOMHIST_7,A037ANOMHIST_8,A038ANOMHIST_9,A039ANOMHIST_10,
	            A900ANOMHIST_11,A901ANOMHIST_12,A902ANOMHIST_13,A903ANOMHIST_14,A904ANOMHIST_15,
				 A905ANOMHIST_16,A906ANOMHIST_17,A907ANOMHIST_18,A908ANOMHIST_19,A909ANOMHIST_20};

	if(AHorCSH_Idx == ANOMHIST_START_IDX)
	{
	 memset(param_EEP_Write_Val, 0, sizeof(param_EEP_Write_Val));

	for (iSize = 0; iSize < _MAX_ANOMALY_LOGS; iSize++) //_MAX_ANOMALY_LOGS
	{
		//Write value into EEPROM
		if(EEPROMProgramByte(param_EEP_Write_Val, gui_ParamAddr_Disp[Tp_anomaly_offset[iSize]], psize+2))
		{
//			UARTprintf("EEPROM_WRITE_ERROR");
			//Log Error
			return _ERR_EEPROM;
		}
		else;

		//Update global param variable
		if(guc_ptrParam_Disp[Tp_anomaly_offset[iSize]]) //Check if not null
			for (msize = 0; msize < psize; msize++)
			{
				*(guc_ptrParam_Disp[Tp_anomaly_offset[iSize]] + (psize - 1) - msize) = 0;
			}
	}
	}
	if(AHorCSH_Idx == CHGSETHIST_START_IDX)
		{
		 memset(param_EEP_Write_Val, 0, sizeof(param_EEP_Write_Val));

		for (iSize = 0; iSize < _MAX_CHGSETHIST_LOGS; iSize++) //_MAX_ANOMALY_LOGS
		{
			//Write value into EEPROM
			if(EEPROMProgramByte(param_EEP_Write_Val, gui_ParamAddr_Disp[AHorCSH_Idx + iSize], psize+2))
			{
	//			UARTprintf("EEPROM_WRITE_ERROR");
				//Log Error
				return _ERR_EEPROM;
			}
			else;

			//Update global param variable
			if(guc_ptrParam_Disp[AHorCSH_Idx + iSize]) //Check if not null
				for (msize = 0; msize < psize; msize++)
				{
					*(guc_ptrParam_Disp[AHorCSH_Idx + iSize] + (psize - 1) - msize) = 0;
				}
		}
		}
	return _SUCCESS;
}
/****************************************************************************/

uint8_t paramLoadDefault(PARAM_DISP paramindex)
{
	uint8_t psize = guc_Paramsize_Disp[paramindex];
	uint8_t param_eeprom_val[_MAX_EEPROM_READVALSIZE];
	uint16_t calccrc;
	uint8_t iSize;

	memset(param_eeprom_val, 0, sizeof(param_eeprom_val));
	for (iSize = 0; iSize < psize; iSize++) {
		// Check if Default Parameter in Flash is present
		if(guc_ptrParamDefault_Disp[paramindex])
			param_eeprom_val[iSize] = *(guc_ptrParamDefault_Disp[paramindex] + (psize-1)-iSize);
		else
			param_eeprom_val[iSize] = 0x00;
	}

	calccrc = ROM_Crc16(0, param_eeprom_val, psize);
	param_eeprom_val[psize] = (calccrc >> 8) & 0xFF;
	param_eeprom_val[psize+1] = calccrc & 0xFF;

	//Write Default param value into EEPROM
	if(EEPROMProgramByte(param_eeprom_val, gui_ParamAddr_Disp[paramindex], psize+2)) {
//		UARTprintf("EEPROM_WRITE_ERROR");
		//Log Error
		return _ERR_EEPROM;
	}
	else;

	//Update global param variable
	if(guc_ptrParam_Disp[paramindex]) //Check if not null
		for (iSize = 0; iSize < psize; iSize++) {
			//*(guc_ptrParam[paramindex] + (psize-1)-iSize) =
			*(guc_ptrParam_Disp[paramindex] + (psize - 1) - iSize) =
					param_eeprom_val[iSize];
		}

	return _SUCCESS;
}

