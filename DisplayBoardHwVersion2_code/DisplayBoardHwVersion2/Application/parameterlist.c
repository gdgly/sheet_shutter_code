/*********************************************************************************
 * FileName: parameterlist.c
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
#include <driverlib/gpio.h>
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "grlib/grlib.h"
//#include "Middleware/cfal96x64x16.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "userinterface.h"
#include "parameterlist.h"
#include "Middleware/paramdatabase.h"
#include "logger.h"

/****************************************************************************/

/****************************************************************************
 *  Macros
****************************************************************************/
//#define EN_OP_CMDS

#define NUM_LINES  4
#define PARA_START 24
#define ENABLE_SERVICE_PARAMS

/****************************************************************************/

/****************************************************************************
 *  Structures
****************************************************************************/

/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
uint8_t gParameterFocusIndex = 0;
uint8_t gHighlightedItemIndex = 0;
uint8_t gCurrentStartIndex = 0;
uint8_t gTotalParamItemsToRender = 0;
uint8_t gParamListRenderStarted = 0;

extern  uint16_t gu16_lcdlight;
extern unsigned char menu_gesture_flag_cyw;
extern const uint16_t gu16_backlight_DEF;
extern const uint8_t gu8_guestue_DEF;

const unsigned char cucUnitSec[] = "sec";
const unsigned char cucUnitmA[] = "mA";
const unsigned char cucUnitPercentage[] = "%";
const unsigned char cucUnitmsec[] = "msec";
const unsigned char cucUnitmeter[] = "m";
const unsigned char cucUnitVolt[] = "V";
const unsigned char cucUnitAmpere[] = "A";
const unsigned char cucUnitRPM[] = "rpm";
const unsigned char cucUnitX1000[] = "x1000";  //"x1K";  //add 20161018
const unsigned char cucUnitS[]="s";

const unsigned char cucYES_NO_State[][40] =
{
	"ムコウ",
	"ユウコウ"
};
const unsigned char cucYES_NO_State_1[][40] =
{
	"ゼンカイ",
	"ハンカイ"
};
const unsigned char cucYES_NO_State_english[][40] =
{
	"NO",
	"YES"
};
const unsigned char cucHigh_Low_Normal[][40]=
{
	"テイソ ク",
	"チュウソ ク",
	"コウソ ク"
};
const unsigned char cucHigh_Low_Normal_english[][40]=
{
	"LOW",
	"NORMAL",
	"HIGH"
};
const unsigned char cucEnable_Disable_State[][40] =
{
	"ユウコウ",//ENABLE
	"ムコウ"//DISABLE
};
const unsigned char cucEnable_Disable_State_english[][40] =
{
	"ENABLE",//ENABLE
	"DISABLE"//DISABLE
};
const unsigned char cucValid_Invalid_State[][40] =
{
	"ユウコウ",//VALID
	"ムコウ"//INVALID
};
const unsigned char cucValid_Invalid_State_english[][40] =
{
	"VALID",//VALID
	"INVALID"//INVALID
};
const unsigned char cucPRIORITY_States[][40] =
{
	"ユウセン_モード",//PRIORITIZE
	"ヒユウセン_モード"//NON PRIORITIZE
};
const unsigned char cucPRIORITY_States_english[][40] =
{
	"PRIORITIZE",//PRIORITIZE
	"NON PRIORITIZE"//NON PRIORITIZE
};
const unsigned char cucA005EN_UP_DWN_DELAY_States[][40] =
{
	"オート_ユウコウ",//EN IN AUTO
	"マニュアル_ユウコウ",//EN IN MANUAL
	"オート+マニュアル"//EN AUTO MANUAL
};
const unsigned char cucA005EN_UP_DWN_DELAY_States_english[][40] =
{
	"EN IN AUTO",//EN IN AUTO
	"EN IN MANUAL",//EN IN MANUAL
	"EN AUTO MANUAL"//EN AUTO MANUAL
};
const unsigned char cucA006AUTO_MAN_MODE_FIX_States[][40] =
{
	"ユウコウ",//ENABLE ON PANEL
	"オート_モード ロック",//オート_モード　ロック
	"マニュアル_モードロック"//マニュアル_モード　ロック
};
const unsigned char cucA006AUTO_MAN_MODE_FIX_States_english[][40] =
{
	"ENABLE ON PANEL",//ENABLE ON PANEL
	"FIX TO AUTO",//オート_モード　ロック
	"FIX TO MANUAL"//マニュアル_モード　ロック
};
const unsigned char cucA537_SH_TYPE_States[][40] =
{
	"BD-0.75",
	"M1-0.75",
	"M2-1.5"

};

const unsigned char cucMult_Func_Out_State[][40] =
{
	"ジョウゲン",//UPPER LIMIT
	"カゲン",//LOWER LIMIT
	"Iロックシュツリョク",//INTERLOCK OP
	"ドウサチュウ",//OPERATING
	"ジョウショウチュウ",//RISING
	"カコウチュウ",//DROPPING
	"ミドリ_ランプ",//GREEN LAMP
	"アカ_ランプ",//RED LAMP
	"オートモード",
	"マニュアルモード",
	"エラーシュツリョク",
};
const unsigned char cucMult_Func_Out_State_english[][40] =
{
	"UPPER LIMIT",//UPPER LIMIT
	"LOWER LIMIT",//LOWER LIMIT
	"INTERLOCK OP",//INTERLOCK OP
	"OPERATING",//OPERATING
	"RISING",//RISING
	"DROPPING",//DROPPING
	"GREEN LAMP",//GREEN LAMP
	"RED LAMP",//RED LAMP
	"AUTO MODE OP",
	"MANUAL MODE OP",
	"ERROR OP",
};

const unsigned char cucWR_1PBS_OR_OPEN[][40] =
{
	"1PBSタイプ",//"WIRELESS 1PBS",
	"3PBSタイプ"//"WIRELESS 3PB OPEN"
};
const unsigned char cucWR_1PBS_OR_OPEN_english[][40] =
{
	"WIRELESS 1PBS",//"WIRELESS 1PBS",
	"WIRELESS 3PB OPEN"//"WIRELESS 3PB OPEN"
};
const unsigned char cucCLOSE_OPR_STATE[][40] =
{
	"ノーマル",//"NORMAL",
	"オシキリ"//"CONTINUOUS PRESS"
};
const unsigned char cucCLOSE_OPR_STATE_english[][40] =
{
	"NORMAL",//"NORMAL",
	"CONTINUOUS PRESS"//"CONTINUOUS PRESS"
};
const unsigned char cucSnow_Mode_State[][40] =
{
	"ノーマル",//"NORMAL",
	"0.25sec",//"0.6sec",//"0.5 SEC",
	"0.5sec"//"2sec"//"1 SEC"
};
const unsigned char cucSnow_Mode_State_english[][40] =
{
	"NORMAL",//"NORMAL",
	"0.25sec",//"0.5 SEC",
	"0.5sec"//"1 SEC"
};
const unsigned char language_Mode_State[][40] =
{
	"カタカナ",//""
	"ENGLISH"//"ENGLISH"
};


const stParamDatabase gsParamDatabase[TOTAL_PARAMETERS] =
{
		//
		// Shutter Parameters
		//
		//{	0	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A000 クド"		,	eVAL_INT	,	0	,	{	0	,	60	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	}	 }	,
		{	0	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A000 ジョウゲンテイシ タイマ"		,	eVAL_INT	,	0	,	{	1	,	60	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	},"A000 UP STOP TIME"	 }	,
		{	1	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A001 I-ロック ユウセン"			,	eSTATE		,	1	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucPRIORITY_States				,	2,(unsigned char *)cucPRIORITY_States_english		},"A001 I-LOCK PRIORITY"	 }	,
		{	2	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A002 I-ロック ON/OFF"		    ,	eSTATE		,	2	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucValid_Invalid_State			,	2,(unsigned char *)cucValid_Invalid_State_english	},"A002 I-LOCK VALID"	 }	,
		{	3	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A003 UPチエン タイマ"			,	eVAL_INT	,	3	,	{	0	,	10	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	},"A003 UP DELAY"	 }	,
		{	4	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A004 DOWNチエン タイマ"		    ,	eVAL_INT	,	4	,	{	0	,	10	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	},"A004 DOWN DELAY"	 }	,
		{	5	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A005 チエンタイマON/OFF"		,	eSTATE		,	5	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucA005EN_UP_DWN_DELAY_States	,	3,(unsigned char *)cucA005EN_UP_DWN_DELAY_States_english	},"A005 EN UP DWN DELAY"	 }	,
		{	6	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A006 モードコテイ"				,	eSTATE		,	6	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucA006AUTO_MAN_MODE_FIX_States,	3,(unsigned char *)cucA006AUTO_MAN_MODE_FIX_States_english	},"A006 AUTO MAN FIXING"	 }	,
		{	7	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A007 Dユニット SWムコウ"		    ,	eSTATE		,	7	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucEnable_Disable_State		,	2,(unsigned char *)cucEnable_Disable_State_english	},"A007 EN PANEL SWITCH"	 }	,
		{	8	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A008 ユキモード"				,	eSTATE	    ,	8	,	{	0	,	0	,	0	                                    }	,	{	(unsigned char *)cucSnow_Mode_State	            ,	3,(unsigned char *)cucSnow_Mode_State_english	},"A008 SNOW MODE"	 }	,
		{	9	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A010 I-ロック チエンタイマ"		,	eVAL_INT	,	10	,	{	0	,	2	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	},"A010 INTERLOCK DELAY"	 }	,
		//	{   7   ,    true   ,    false  ,    false  ,   eDestDisplayBoard   ,   "A200 GUEST ON/OFF"         ,   eSTATE      ,   26 ,   {   0   ,   0   ,   0                                       }   ,   {   (unsigned char *)cucValid_Invalid_State			,	2   }    }  ,
		{   10   ,    true   ,    false  ,    false  ,  eDestDisplayBoard   ,   "A026 バックライト"            	,   eVAL_INT    ,   26 ,   {   0   ,   60  ,   (unsigned char *)cucUnitS               }   ,   {   0                                               ,   0   },"A026 LCD BACKLIGHT"    }  ,
		{	11	,	 true	,	 false	,	 false	,	eDestDisplayBoard	,	"A041 テカザシ_センサ"		    ,	eSTATE		,	37	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucEnable_Disable_State		,	2,(unsigned char *)cucEnable_Disable_State_english	},"A041 EN GESTURE"	 }	,
		{	12	,	 true	,	 false	,	 false	,	eDestDisplayBoard	,	"A042 ゲンゴ"		            ,	eSTATE		,	38	,	{	0	,	0	,	0										}	,	{	(unsigned char *)language_Mode_State		    ,	2,(unsigned char *)language_Mode_State	},"A042 LANGUAGE"	 }	,
		{	13	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A061 センサON/OFF"			,	eSTATE		,	61	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State_1				,	2,(unsigned char *)cucYES_NO_State_english	},"A061 SENSR IP SETING"	 }	,    //add A061,201806_Bug_No.9
		{	14	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A062 1PBS　ON/OFF"		    ,	eSTATE		,	62	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State_1				,	2,(unsigned char *)cucYES_NO_State_english	},"A062 1 PBS IP SETING"	 }	,    //add A062,201806_Bug_No.9
		{	15	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A063 3PBS　ON/OFF"		    ,	eSTATE		,	63	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State_1				,	2,(unsigned char *)cucYES_NO_State_english	},"A063 3 PBS IP SETING"	 }	,    //add A063,201806_Bug_No.9
		{	16	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A130 ハンカイ セットイチ"			,	eVAL_INT	,	130	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A130 APR HGT POS"	 }	,
		{	17	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A131 ハンカイ ON/OFF"		    ,	eSTATE		,	131	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State				,	2,(unsigned char *)cucYES_NO_State_english	},"A131 EN APERT HEIGHT"	 }	,
		{	18	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A522 ジョウショウ ソ クド"	        ,	eSTATE      ,	522	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucHigh_Low_Normal				,	3,(unsigned char *)cucHigh_Low_Normal_english	},"A522 S1 UP"	 }	,
		{	19	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A528 カコウ ソ クド"	            ,	eSTATE      ,	528	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucHigh_Low_Normal				,	3,(unsigned char *)cucHigh_Low_Normal_english	},"A528 S1 DOWN"	 }	,

		//
		// Drive Parameters
		//
		{	20	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A000 ジョウゲンテイシ タイマ"		,	eVAL_INT	,	0	,	{	1	,	60	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	},"A000 UP STOP TIME"	 }	,
		{	21	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A001 I-ロック ユウセン"			,	eSTATE		,	1	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucPRIORITY_States				,	2,(unsigned char *)cucPRIORITY_States_english	},"A001 I-LOCK PRIORITY"	 }	,
		{	22	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A002 I-ロック ON/OFF"		    ,	eSTATE		,	2	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucValid_Invalid_State			,	2,(unsigned char *)cucValid_Invalid_State_english	},"A002 I-LOCK VALID"	 }	,
		{	23	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A003 UPチエン タイマ"			,	eVAL_INT	,	3	,	{	0	,	10	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	},"A003 UP DELAY"	 }	,
		{	24	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A004 DOWNチエン タイマ"		    ,	eVAL_INT	,	4	,	{	0	,	10	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	},"A004 DOWN DELAY"	 }	,
		{	25	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A005 チエンタイマON/OFF"		,	eSTATE		,	5	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucA005EN_UP_DWN_DELAY_States	,	3,(unsigned char *)cucA005EN_UP_DWN_DELAY_States_english	},"A005 EN UP DWN DELAY"	 }	,
		{	26	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A006 モードコテイ"				,	eSTATE		,	6	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucA006AUTO_MAN_MODE_FIX_States,	3,(unsigned char *)cucA006AUTO_MAN_MODE_FIX_States_english	},"A006 AUTO MAN FIXING"	 }	,
		{	27	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A007 Dユニット SWムコウ"		    ,	eSTATE		,	7	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucEnable_Disable_State		,	2,(unsigned char *)cucEnable_Disable_State_english	},"A007 EN PANEL SWITCH"	 }	,
		{	28	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A008 ユキモード"				,	eSTATE	    ,	8	,	{	0	,	0	,	0	                                    }	,	{	(unsigned char *)cucSnow_Mode_State	            ,	3,(unsigned char *)cucSnow_Mode_State_english	},"A008 SNOW MODE"	 }	,
		{	29	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A009 ドウサセイゲン タイマ"		,	eVAL_INT	,	9	,	{	0	,	30	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	},"A009 OPR RESTRIC TMR"	 }	,
		{	30	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A010 I-ロック チエンタイマ"		,	eVAL_INT	,	10	,	{	0	,	2	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	},"A010 INTERLOCK DELAY"	 }	,
		{	31	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A016 カコウドウサ セッテイ"		    ,	eSTATE		,	16	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucCLOSE_OPR_STATE				,	2,(unsigned char *)cucCLOSE_OPR_STATE_english},"A016 CLOSE OPR SET"	 }	,
		{	32	,	 true	,	 false	,	 false	,	eDestDriveBoard	    ,	"A025 メンテナンスカウント"			,	eVAL_INT	,	25	,	{	1	,	9999,	(unsigned char *)cucUnitX1000   	}	,	{	0												,	0	},"A025 MAINTAIN CNT"	 }	,
		{   33   ,   true   ,    false  ,    false  ,  eDestDisplayBoard    ,   "A026 バックライト"            ,   eVAL_INT    ,   26 ,    {   0   ,   60  ,   (unsigned char *)cucUnitS               }   ,   {   0                                              ,   0   },"A026 LCD BACKLIGHT"    }  ,
		{	34	,	 true	,	 false	,	 false	,	eDestDisplayBoard	,	"A041 テカザシ_センサ"		    ,	eSTATE		,	37	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucEnable_Disable_State		,	2,(unsigned char *)cucEnable_Disable_State_english	},"A041 EN GESTURE"	 }	,
		{	35	,	 true	,	 false	,	 false	,	eDestDisplayBoard	,	"A042 ゲンゴ"		            ,	eSTATE		,	38	,	{	0	,	0	,	0										}	,	{	(unsigned char *)language_Mode_State		    ,	2,(unsigned char *)language_Mode_State	},"A042 LANGUAGE"	 }	,
		{	36	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A061 センサON/OFF"			,	eSTATE		,	61	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State_1				,	2,(unsigned char *)cucYES_NO_State_english	},"A061 SENSR IP SETING"	 }	,    //20170613  201703_No.52
		{	37	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A062 1PBS　ON/OFF"		    ,	eSTATE		,	62	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State_1				,	2,(unsigned char *)cucYES_NO_State_english	},"A062 1 PBS IP SETING"	 }	,    //20170613  201703_No.52
		{	38	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A063 3PBS　ON/OFF"		    ,	eSTATE		,	63	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State_1				,	2,(unsigned char *)cucYES_NO_State_english	},"A063 3 PBS IP SETING"	 }	,    //20170613  201703_No.52
		{	39	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A071 OPリレー1"				,	eSTATE		,	71	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucMult_Func_Out_State			,	11,(unsigned char *)cucMult_Func_Out_State_english},"A071 MULTI FUNC OP 1"	 }	,
		{	40	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A072 OPリレー2"				,	eSTATE		,	72	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucMult_Func_Out_State			,	11,(unsigned char *)cucMult_Func_Out_State_english},"A072 MULTI FUNC OP 2"	 }	,
		{	41	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A073 OPリレー3"				,	eSTATE		,	73	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucMult_Func_Out_State			,	11,(unsigned char *)cucMult_Func_Out_State_english},"A073 MULTI FUNC OP 3" }	,
		{	42	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A074 OPリレー4"				,	eSTATE		,	74	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucMult_Func_Out_State			,	11,(unsigned char *)cucMult_Func_Out_State_english},"A074 MULTI FUNC OP 4"	 }	,
		{	43	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A075 OPリレー5"				,	eSTATE		,	75	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucMult_Func_Out_State			,	11,(unsigned char *)cucMult_Func_Out_State_english},"A075 MULTI FUNC OP 5"	 }	,
		{	44	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A078 リモコン センタク"			,	eSTATE		,	78	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucWR_1PBS_OR_OPEN				,	2,(unsigned char *)cucWR_1PBS_OR_OPEN_english},"A078 WR 1PBS OR OPEN"	 }	,
		{	45	,	 true	,	 true	,	 false	,	eDestDriveBoard		,	"A080 マイクロセンサカウント"		,	eVAL_INT	,	80	,	{	0	,	99		,	0									}	,	{	0												,	0	},"A080 MICRO CNT MON"	 }	,
		{	46	,	 true	,	 true	,	 false	,	eDestDriveBoard		,	"A100 ジョウゲン セットイチ"		    ,	eVAL_INT	,	100	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A100 UP STP POS"	 }	,
		{	47	,	 true	,	 true	,	 false	,	eDestDriveBoard		,	"A101 カゲン セットイチ"			,	eVAL_INT	,	101	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A101 LOW STP POS"	 }	,
		{	48	,	 true	,	 true	,	 false	,	eDestDriveBoard		,	"A102 コウデンギリ セットイチ"		,	eVAL_INT	,	102	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A102 PE POSITION"	 }	,
		{	49	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A103 ジョウショウ　ゲンソ ク1"		,	eVAL_INT	,	103	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A103 RCG POS 1"	 }	,
		{	50	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A104 ジョウショウ　ゲンソ ク2"		,	eVAL_INT	,	104	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A104 RCG POS 2"	 }	,
		{	51	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A105 ジョウショウ　ゲンソ ク3"		,	eVAL_INT	,	105	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A105 RCG POS 3"	 }	,
		{	52	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A106 カコウ　ゲンソ ク1"			,	eVAL_INT	,	106	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A106 FCG POS 1"	 }	,
		{	53	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A107 カコウ　ゲンソ ク2"			,	eVAL_INT	,	107	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A107 FCG POS 2"	 }	,
		{	54	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A108 カコウ　ゲンソ ク3"			,	eVAL_INT	,	108	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A108 FCG POS 3"	 }	,
		{	55	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A126 ゲンテンホセイカイスウ"		,	eVAL_INT	,	126	,	{	10	,	9999	,	0									}	,	{	0												,	0	},"A126 COR FRQ APER"	 }	,
		{	56	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A127 ゲンテンホセイセッテイ"		,	eSTATE		,	127	,	{	0	,	0		,	0									}	,	{	(unsigned char *)cucYES_NO_State				,	2,(unsigned char *)cucYES_NO_State_english	},"A127 AUT COR EN"	 }	,
		{	57	,	 true	,	 true	,	 false	,	eDestDriveBoard		,	"A128 ゲンテン セットイチ"			,	eVAL_INT	,	128	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A128 ORG SENS POS"	 }	,
		{	58	,	 true	,	 true	,	 true	,	eDestDriveBoard		,	"A129 ゲンザイノ シートイチ"		,	eVAL_INT	,	129	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A129 CUR VAL MON"	 }	,
		{	59	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A130 ハンカイ セットイチ"			,	eVAL_INT	,	130	,	{	0	,	9000	,	0									}	,	{	0												,	0	},"A130 APR HGT POS"	 }	,
        {	60	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A131 ハンカイ ON/OFF"		    ,	eSTATE		,	131	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State				,	2,(unsigned char *)cucYES_NO_State_english	},"A131 EN APERT HEIGHT"}	,
		//{  41   ,    true   ,    false  ,    false  ,   eDestDisplayBoard   ,   "A200 GUEST ON/OFF"      ,   eSTATE      ,   26 ,   {   0   ,   0   ,   0                                       }   ,   {   (unsigned char *)cucValid_Invalid_State	    ,	2   }    }  ,
		{	61	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A522 ジョウショウ ソ クド"	        ,	eSTATE      ,	522	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucHigh_Low_Normal				,	3,(unsigned char *)cucHigh_Low_Normal_english},"A522 S1 UP"	 }	,
		{	62	,	 true	,	 false	,	 false	,	eDestDriveBoard		,	"A528 カコウ ソ クド"	            ,	eSTATE      ,	528	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucHigh_Low_Normal				,	3,(unsigned char *)cucHigh_Low_Normal_english},"A528 S1 DOWN"	 }	,
   //     {	60	,	 true	,	 false	,	 false	,	eDestDriveBoard	    ,	"A537 シャッタータイプ"			,	eSTATE		,	537	,	{	0	,	0		,	0									}	,	{	(unsigned char *)cucA537_SH_TYPE_States			,	3,(unsigned char *)cucA537_SH_TYPE_States	},"A537 SHUTTER TYPE"	 }	,
		
		//
		// Service Parameters
		//
		//{	44	,	 true	,	false	,	false	,	eDestDisplayBoard	,	"A800 FAIL DI<CT"			,	eVAL_INT	,	A800CRCFAIL_DI2CT	,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
		//{	45	,	 true	,	false	,	false	,	eDestControlBoard	,	"A801 FAIL CT<DI"			,	eVAL_INT	,	801					,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
		//{	46	,	 true	,	false	,	false	,	eDestControlBoard	,	"A802 FAIL CT<DR"			,	eVAL_INT	,	802					,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
		//{	47	,	 true	,	false	,	false	,	eDestDisplayBoard	,	"A804 OVER DI<CT"			,	eVAL_INT	,	A804COMMBUFFOF_DI2CT,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
		//{	48	,	 true	,	false	,	false	,	eDestControlBoard	,	"A805 OVER CT<DI"			,	eVAL_INT	,	805					,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
		//{	49	,	 true	,	false	,	false	,	eDestControlBoard	,	"A806 OVER CT<DR"			,	eVAL_INT	,	806					,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
};

//const stParamDatabase gsParamDatabase[TOTAL_PARAMETERS] =
//{
//		//
//		// Shutter Parameters
//		//
//		{	0	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A000 UP STOP TIME"		,	eVAL_INT	,	0	,	{	0	,	60	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	}	 }	,
//		{	1	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A001 I-LOCK PRIORITY"	,	eSTATE		,	1	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucPRIORITY_States				,	2	}	 }	,
//		{	2	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A002 I-LOCK VALID"		,	eSTATE		,	2	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucValid_Invalid_State			,	2	}	 }	,
//		{	3	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A003 UP DELAY"			,	eVAL_INT	,	3	,	{	0	,	10	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	}	 }	,
//		{	4	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A004 DOWN DELAY"		,	eVAL_INT	,	4	,	{	0	,	10	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	}	 }	,
//		{	5	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A005 EN UP DWN DELAY"	,	eSTATE		,	5	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucA005EN_UP_DWN_DELAY_States	,	3	}	 }	,
//		{	6	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A006 AUTO MAN FIXING"	,	eSTATE		,	6	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucA006AUTO_MAN_MODE_FIX_States,	3	}	 }	,
//		{	7	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A007 EN PANEL SWITCH"	,	eSTATE		,	7	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucEnable_Disable_State		,	2	}	 }	,
//		{	8	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A008 SNOW  MODE	 "	,	eSTATE	    ,	  8	,	{	0	,	0	,	0	                                    }	,	{	(unsigned char *)cucSnow_Mode_State	            ,	3	}	 }	,
//		{	9	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A009 OPR RESTRIC TMR"	,	eVAL_INT	,	9	,	{	0	,	30	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	}	 }	,
//		{	10	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A010 INTERLOCK DELAY"	,	eVAL_INT	,	10	,	{	0	,	2	,	(unsigned char *)cucUnitSec				}	,	{	0												,	0	}	 }	,
//		{	11	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A016 CLOSE OPR SET"	,	eSTATE		,	16	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucCLOSE_OPR_STATE				,	2	}	 }	,
//		{	12	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A061 SENSR IP SETING"	,	eSTATE		,	61	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State				,	2	}	 }	,
//		{	13	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A062 1 PBS IP SETING"	,	eSTATE		,	62	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State				,	2	}	 }	,
//		{	14	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A063 3 PBS IP SETING"	,	eSTATE		,	63	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State				,	2	}	 }	,
//		{	15	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A071 MULTI FUNC OP 1"	,	eSTATE		,	71	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucMult_Func_Out_State			,	11	}	 }	,
//		{	16	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A072 MULTI FUNC OP 2"	,	eSTATE		,	72	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucMult_Func_Out_State			,	11	}	 }	,
//		{	17	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A073 MULTI FUNC OP 3"	,	eSTATE		,	73	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucMult_Func_Out_State			,	11	}	 }	,
//		{	18	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A074 MULTI FUNC OP 4"	,	eSTATE		,	74	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucMult_Func_Out_State			,	11	}	 }	,
//		{	19	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A075 MULTI FUNC OP 5"	,	eSTATE		,	75	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucMult_Func_Out_State			,	11	}	 }	,
//		{	20	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A078 WR 1PBS OR OPEN"	,	eSTATE		,	78	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucWR_1PBS_OR_OPEN				,	2	}	 }	,
//		{	21	,	 true	,	 false	,	 false	,	eDestControlBoard	,	"A131 EN APERT HEIGHT"	,	eSTATE		,	131	,	{	0	,	0	,	0										}	,	{	(unsigned char *)cucYES_NO_State				,	2	}	 }	,
//
//
//		//
//		// Drive Parameters
//		//
//		{	22	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A025 MAINTAIN CNT"			,	eVAL_INT	,	25	,	{	1	,	9999	,	(unsigned char *)cucUnitX1000   	}	,	{	0												,	0	}	 }	,
//		{	23	,	 false	,	 false	,	 false	,	eDestDriveBoard	,	"A028 OP CNT IN"			,	eVAL_INT	,	28	,	{	0	,	9999999	,	0									}	,	{	0												,	0	}	 }	,
//		{	24	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A080 MICRO CNT MON"		,	eVAL_INT	,	80	,	{	0	,	99		,	0									}	,	{	0												,	0	}	 }	,
//		{	25	,	 true	,	 true	,	 false	,	eDestDriveBoard	,	"A100 UP STP POS"			,	eVAL_INT	,	100	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	26	,	 true	,	 true	,	 false	,	eDestDriveBoard	,	"A101 LOW STP POS"			,	eVAL_INT	,	101	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	27	,	 true	,	 true	,	 false	,	eDestDriveBoard	,	"A102 PE POS"				,	eVAL_INT	,	102	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	28	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A103 RCG POS1"				,	eVAL_INT	,	103	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	29	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A104 RCG POS2"				,	eVAL_INT	,	104	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	30	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A105 RCG POS3"				,	eVAL_INT	,	105	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	31	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A106 FCG POS1"				,	eVAL_INT	,	106	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	32	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A107 FCG POS2"				,	eVAL_INT	,	107	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	33	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A108 FCG POS3"				,	eVAL_INT	,	108	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	34	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A110 SHREV OP MINLIM"		,	eVAL_INT	,	110	,	{	0	,	100		,	0									}	,	{	0												,	0	}	 }	,
//		{	35	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A112 OVERRUN PRO"			,	eVAL_INT	,	112	,	{	0	,	100		,	0									}	,	{	0												,	0	}	 }	,
//		{	36	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A126 COR FRQ APER"			,	eVAL_INT	,	126	,	{	10	,	9999	,	0									}	,	{	0												,	0	}	 }	,
//		{	37	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A127 AUT COR EN"			,	eSTATE		,	127	,	{	0	,	0		,	0									}	,	{	(unsigned char *)cucYES_NO_State				,	2	}	 }	,
//		{	38	,	 true	,	 true	,	 false	,	eDestDriveBoard	,	"A128 ORG SENS POS"			,	eVAL_INT	,	128	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	39	,	 true	,	 true	,	 true	,	eDestDriveBoard	,	"A129 CUR VAL MON"			,	eVAL_INT	,	129	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	40	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A130 APR HGT POS"			,	eVAL_INT	,	130	,	{	0	,	9000	,	0									}	,	{	0												,	0	}	 }	,
//		{	41	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A512 UP SPEED KP"			,	eVAL_INT	,	512	,	{	0	,	300		,	0									}	,	{	0												,	0	}	 }	,
//		{	42	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A513 UP SPEED KI"			,	eVAL_INT	,	513	,	{	0	,	250		,	0									}	,	{	0												,	0	}	 }	,
//		{	43	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A514 DOWN SPEED KP"		,	eVAL_INT	,	514	,	{	0	,	300		,	0									}	,	{	0												,	0	}	 }	,
//		{	44	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A515 DOWN SPEED KI"		,	eVAL_INT	,	515	,	{	0	,	250		,	0									}	,	{	0												,	0	}	 }	,
//		{	45	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A517 INCH SPD"				,	eVAL_INT	,	517	,	{	1	,	100		,	(unsigned char *)cucUnitPercentage	}	,	{	0												,	0	}	 }	,
//		{	46	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A520 ACCELERATE TIME"		,	eVAL_INT	,	520	,	{	100	,	9999	,	(unsigned char *)cucUnitmsec		}	,	{	0												,	0	}	 }	,
//		{	47	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A521 DECELERATE TIME"		,	eVAL_INT	,	521	,	{	100	,	9999	,	(unsigned char *)cucUnitmsec		}	,	{	0												,	0	}	 }	,
//		{	48	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A522 S1 UP"				,	eVAL_INT	,	522	,	{	150	,	3600	,	(unsigned char *)cucUnitRPM			}	,	{	0												,	0	}	 }	,
//		{	49	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A523 S2 UP"				,	eVAL_INT	,	523	,	{	150	,	3600	,	(unsigned char *)cucUnitRPM			}	,	{	0												,	0	}	 }	,
//		{	50	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A524 S3 UP"				,	eVAL_INT	,	524	,	{	150	,	3600	,	(unsigned char *)cucUnitRPM			}	,	{	0												,	0	}	 }	,
//		{	51	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A525 UP STP CNT"			,	eVAL_INT	,	525	,	{	2	,	3		,	0									}	,	{	0												,	0	}	 }	,
//		{	52	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A528 S1 DOWN"				,	eVAL_INT	,	528	,	{	150	,	3600	,	(unsigned char *)cucUnitRPM			}	,	{	0												,	0	}	 }	,
//		{	53	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A529 S2 DOWN"				,	eVAL_INT	,	529	,	{	150	,	3600	,	(unsigned char *)cucUnitRPM			}	,	{	0												,	0	}	 }	,
//		{	54	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A530 S3 DOWN"				,	eVAL_INT	,	530	,	{	150	,	3600	,	(unsigned char *)cucUnitRPM			}	,	{	0												,	0	}	 }	,
//		{	55	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A531 DOWN STP CNT"			,	eVAL_INT	,	531	,	{	2	,	3		,	0									}	,	{	0												,	0	}	 }	,
//		{	56	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A537 SHUTTER TYPE"			,	eSTATE		,	537	,	{	0	,	0		,	0									}	,	{	(unsigned char *)cucA537_SH_TYPE_States			,	2	}	 }	,
//		{	57	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A546 RATED SPD"			,	eVAL_INT	,	546	,	{	0	,	3600	,	(unsigned char *)cucUnitRPM			}	,	{	0												,	0	}	 }	,
//		{	58	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A551 JOG SPEED"			,	eVAL_INT	,	551	,	{	1	,	100		,	(unsigned char *)cucUnitPercentage	}	,	{	0												,	0	}	 }	,
//		//	A500, A501, A504 range increased for tuning purpose - YG - 2015
//		{	59	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A500 FOR FUTURE USE"		,	eVAL_INT	,	500	,	{	0	,	32000  	,	0									}	,	{	0												,	0	}	 }	,
//		{	60	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A501 FOR FUTURE USE"		,	eVAL_INT	,	501	,	{	0	,	32000	,	0									}	,	{	0												,	0	}	 }	,
//		{	61	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A504 FOR FUTURE USE" 		,	eVAL_INT	,	504	,	{	0	,	5000	,	0									}	,	{	0												,	0	}	 }	,
//		{	62	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A505 FOR FUTURE USE"		,	eVAL_INT	,	505	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	63	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A506 FOR FUTURE USE"		,	eVAL_INT	,	506	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	64	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A507 FOR FUTURE USE"		,	eVAL_INT	,	507	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	65	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A509 FOR FUTURE USE"		,	eVAL_INT	,	509	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	66	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A510 FOR FUTURE USE"		,	eVAL_INT	,	510	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	67	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A511 FOR FUTURE USE"		,	eVAL_INT	,	511	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	68	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A516 FOR FUTURE USE" 		,	eVAL_INT	,	516	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	69	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A518 FOR FUTURE USE"		,	eVAL_INT	,	518	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	70	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A519 FOR FUTURE USE"		,	eVAL_INT	,	519	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	71	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A526 FOR FUTURE USE"		,	eVAL_INT	,	526	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	72	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A527 FOR FUTURE USE"		,	eVAL_INT	,	527	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	73	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A536 FOR FUTURE USE"		,	eVAL_INT	,	536	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	74	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A538 FOR FUTURE USE"		,	eVAL_INT	,	538	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	75	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A539 FOR FUTURE USE"		,	eVAL_INT	,	539	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	76	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A540 FOR FUTURE USE"		,	eVAL_INT	,	540	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	77	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A541 FOR FUTURE USE"		,	eVAL_INT	,	541	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	78	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A542 FOR FUTURE USE"		,	eVAL_INT	,	542	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	79	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A543 FOR FUTURE USE"		,	eVAL_INT	,	543	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	80	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A544 FOR FUTURE USE"		,	eVAL_INT	,	544	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	81	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A545 FOR FUTURE USE"		,	eVAL_INT	,	545	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	82	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A547 FOR FUTURE USE"		,	eVAL_INT	,	547	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	83	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A548 FOR FUTURE USE"		,	eVAL_INT	,	548	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	84	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A549 FOR FUTURE USE"		,	eVAL_INT	,	549	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//		{	85	,	 true	,	 false	,	 false	,	eDestDriveBoard	,	"A550 FOR FUTURE USE"		,	eVAL_INT	,	550	,	{	0	,	1		,	0									}	,	{	0												,	0	}	 }	,
//
//		//
//		// Service Parameters
//		//
//		{	86	,	 true	,	false	,	false	,	eDestDisplayBoard	,	"A800 CRCFAIL DI<CT"	,	eVAL_INT	,	A800CRCFAIL_DI2CT	,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
//		{	87	,	 true	,	false	,	false	,	eDestControlBoard	,	"A801 CRCFAIL CT<DI"	,	eVAL_INT	,	801					,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
//		{	88	,	 true	,	false	,	false	,	eDestControlBoard	,	"A802 CRCFAIL CT<DR"	,	eVAL_INT	,	802					,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
////		{	77	,	 true	,	false	,	false	,	eDestDriveBoard		,	"A803"					,	eVAL_INT	,	0					,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,				// Not implemented A803
//		{	89	,	 true	,	false	,	false	,	eDestDisplayBoard	,	"A804 BUFFOV DI<CT"		,	eVAL_INT	,	A804COMMBUFFOF_DI2CT,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
//		{	90	,	 true	,	false	,	false	,	eDestControlBoard	,	"A805 BUFFOV CT<DI"		,	eVAL_INT	,	805					,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
//		{	91	,	 true	,	false	,	false	,	eDestControlBoard	,	"A806 BUFFOV CT<DR"		,	eVAL_INT	,	806					,	{	0	,	65535	,	0	}	,	{	0	,	0	}	 }	,
//};

/****************************************************************************/
void Para_On_Display_Board_init_cyw(void)
{
	//ui32OperationCount = 0;

	gu16_lcdlight =gu16_backlight_DEF;
	menu_gesture_flag_cyw =gu8_guestue_DEF;
	writeParameterUpdateInDB((PARAM_DISP)gsParamDatabase[Para_LcdBackLight_Index_cyw].paramEEPROMIndex, (uint8_t *)&gu16_lcdlight);
	writeParameterUpdateInDB((PARAM_DISP)gsParamDatabase[Para_Guesture_Index_cyw].paramEEPROMIndex, (uint8_t *)&menu_gesture_flag_cyw);

}
/******************************************************************************
 * FunctionName: parameterListFirstScreen
 *
 * Function Description:
 * This function paints a list of active/enabled parameters from parameter database
 * with first enabled parameter in list highlighted.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
 ********************************************************************************/
uint8_t parameterListFirstScreen()
{
	uint8_t ui8YMin = 0;
	uint16_t i = 0;
	uint16_t lStartIndex = 0, lLastIndex = 0;
	uint8_t lineCounter = 0;
	  char cccccc[5]={0};
	//
	// Handle exception if there is no active functional block.
	//
	if(psActiveFunctionalBlock == NULL)
		return 1;

	//
	// Check which parameter list to render - shutter or drive parameters.
	// Get start and end indices accordingly.
	//
	if(0 == psActiveMenu->ui8FocusIndex)
	{
		//
		// Indices for shutter parameters
		//
		lStartIndex = SHUTTER_PARAM_START_INDEX;
		lLastIndex = TOTAL_PARAMETERS1;
	}
	else if(1 == psActiveMenu->ui8FocusIndex)
	{
		//
		// Indices for drive parameters
		//
		lStartIndex = DRIVE_PARAM_START_INDEX;
		lLastIndex = DRIVE_PARAM_START_INDEX + TOTAL_PARAMETERS2;
	}
#ifdef ENABLE_SERVICE_PARAMS
	else if(2 == psActiveMenu->ui8FocusIndex)
	{
		//
		// Indices for service parameters
		//
		lStartIndex = SERVICE_PARAM_START_INDEX;
		lLastIndex = SERVICE_PARAM_START_INDEX + TOTAL_SERVICE_PARAMETERS;
	}
#endif
	else
	{
		return 1;
	}

	//
	// Calculate total number of parameter items to render
	//
	for(i = lStartIndex; i <lLastIndex; i++)
	{
		if(gsParamDatabase[i].paramEnabled == true)
		{
			gTotalParamItemsToRender++;
		}
	}

	//
	// Check if it is first time call to this function.
	//
	if(0 == gParamListRenderStarted)
	{
		//
		// Get current start index.
		//
		gCurrentStartIndex = lStartIndex;

		//
		// Calculate index of item to highlight.
		//
		for(i = lStartIndex; i <lLastIndex; i++)
		{
			if(gsParamDatabase[i].paramEnabled == true)
			{
				gHighlightedItemIndex = i;
				break;
			}
		}

		//
		// Set flag to ensure that parameter listing is started.
		//
		gParamListRenderStarted = 1;
	}

	//
	// Clear Screen.
	//
	//GrRectFIllBolymin(0, 126, 0, 63, true, true);
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

	//
	//	Loop while there exists a parameter item to render on screen.
	//
	for(i = gCurrentStartIndex; i < lLastIndex; i++)
	{
		if(gsParamDatabase[i].paramEnabled == true)
		{
			//
			// Draw parameter items on screen. Highlight the parameter item which has current focus
			//
			if(gHighlightedItemIndex == i)
			{
				//cccccc[0]=gsParamDatabase[i].paramName[0];
				//										cccccc[1]=gsParamDatabase[i].paramName[1];
				//										cccccc[2]=gsParamDatabase[i].paramName[2];
				//										cccccc[3]=gsParamDatabase[i].paramName[3];
				GrRectFIllBolymin_cyw(0,126,ui8YMin,ui8YMin+11,0xff,true);
			if(gu8_language == Japanese_IDX)
			{
			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[i].paramName_japanese,4);
			}
			else
			{
			memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[i].paramName_english,4);
			}
			displayText((unsigned char *)cccccc, 1, ui8YMin, true, false, false, false,true , true);
			if(gu8_language == Japanese_IDX)
			{
			displayText((unsigned char *)gsParamDatabase[i].paramName_japanese+5, PARA_START+4, ui8YMin, true , false, false,false, true, false);
			}
			else
			{
			//displayText((unsigned char *)gsParamDatabase[i].paramName_english, 0, ui8YMin, true , false, false,true, true, false);
			displayText((unsigned char *)gsParamDatabase[i].paramName_english+5, PARA_START+4, ui8YMin, true , false, false,false, false, true);
			}
			}
			else
			{
				//cccccc[0]=gsParamDatabase[i].paramName[0];
				//														cccccc[1]=gsParamDatabase[i].paramName[1];
				//														cccccc[2]=gsParamDatabase[i].paramName[2];
				//														cccccc[3]=gsParamDatabase[i].paramName[3];
				GrRectFIllBolymin(0,126,ui8YMin,ui8YMin+11,true,true);
				if(gu8_language == Japanese_IDX)
				{
				memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[i].paramName_japanese,4);
				}
				else
				{
				memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[i].paramName_english,4);
				}
				displayText((unsigned char *)cccccc, 1, ui8YMin, false, false, false, true, false, true);
				if(gu8_language == Japanese_IDX)
				{
				displayText((unsigned char *)gsParamDatabase[i].paramName_japanese+5, PARA_START+4, ui8YMin, false, false, false, true, true, false);
				}
				else
				{
				displayText((unsigned char *)gsParamDatabase[i].paramName_english+5, PARA_START+4, ui8YMin, false, false, false, true, false, true);
				}
			}

			//
			// Increment Y coordinate of the top left corner of the menu item to be displayed
			//
			ui8YMin += MENU_ITEM_HEIGHT;

			lineCounter++;
			if(NUMBER_OF_LINES == lineCounter)
				break;
		}
	}

	return 0;
}

/******************************************************************************
 * FunctionName: parameterListRunTime
 *
 * Function Description:
 * This is a default function.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t parameterListRunTime()
{
	updateFaultLEDStatus();

#ifdef EN_OP_CMDS
	operationKeysHandler();
#endif

	return 0;
}

/******************************************************************************
 * FunctionName: parameterListUp
 *
 * Function Description:
 * This function shifts the highlight on parameter item in upward direction by
 * decrementing parameter focus index. This function also paints parameter list
 * again to reflect the change.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t parameterListUp()
{
	uint8_t ui8YMin = 0;
	int i=0, k=0;
	uint8_t paintCounter = 0;
	uint8_t lineCounter = 0;

	uint16_t ui16i = 0;
	uint16_t lLastIndex = 0;
	 char cccccc[5]={0};

    if(gKeysStatus.bits.Key_Up_pressed)
    {
    	gKeysStatus.bits.Key_Up_pressed = 0;

		//
		// Handle exception if there is no active menu.
		//
		if(psActiveFunctionalBlock == NULL)
			return 1;

		if(0 == psActiveMenu->ui8FocusIndex)
		{
			lLastIndex = TOTAL_PARAMETERS1;
		}
		else if(1 == psActiveMenu->ui8FocusIndex)
		{
			lLastIndex = DRIVE_PARAM_START_INDEX + TOTAL_PARAMETERS2;
		}
		else if(2 == psActiveMenu->ui8FocusIndex)
		{
			lLastIndex = SERVICE_PARAM_START_INDEX + TOTAL_SERVICE_PARAMETERS;
		}
		else
		{
			return 1;
		}

		//
		// decrement the focus index if focus is not on the first parameter item of
		// parameter list.
		//
		if(gParameterFocusIndex > 0)
			gParameterFocusIndex--;

		if(gParameterFocusIndex > NUM_LINES -1)
		{
			paintCounter = (gParameterFocusIndex / 4) * 4;
		}

		//
		// Check whether screen switching is required. If yes, then
		// find index of the first item to render on screen.
		//
		if( (gParameterFocusIndex + 1) % 4 == 0 )
		{
			for(i = 4, k = gHighlightedItemIndex; i >= 0;)
			{
				if(gsParamDatabase[k].paramEnabled == true)
				{
					i--;
					gCurrentStartIndex = k;
				}

				if(k == 0)
					break;
				k--;
			}
		}

		//
		// Clear screen
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		//
		//	Loop while there exists a menu item to render on screen.
		//
		for(ui16i = gCurrentStartIndex; ui16i <lLastIndex; ui16i++)
		{
			if(gsParamDatabase[ui16i].paramEnabled == true)
			{

				//
				// Draw parameter item on screen. Highlight the parameter item which has current focus
				// given by gParameterFocusIndex.
				//
				if(paintCounter == gParameterFocusIndex)
				{
					///cccccc[0]=gsParamDatabase[ui16i].paramName[0];
					//					cccccc[1]=gsParamDatabase[ui16i].paramName[1];
					//					cccccc[2]=gsParamDatabase[ui16i].paramName[2];
					//					cccccc[3]=gsParamDatabase[ui16i].paramName[3];
					GrRectFIllBolymin_cyw(0,126,ui8YMin,ui8YMin+11,0xff,true);
					if(gu8_language == Japanese_IDX)
					{
					memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[ui16i].paramName_japanese,4);
					}
					else
					{
					memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[ui16i].paramName_english,4);
					}
					displayText((unsigned char*)cccccc, 1, ui8YMin, true, false, false, false,false, true);
					if(gu8_language == Japanese_IDX)
					{
					displayText((unsigned char*)gsParamDatabase[ui16i].paramName_japanese+5, PARA_START+4, ui8YMin, true, false, false, false,true, false);
					}
					else
					{
					displayText((unsigned char*)gsParamDatabase[ui16i].paramName_english+5, PARA_START+4, ui8YMin, true, false, false, false,false, true);

					}
					gHighlightedItemIndex = ui16i;
				}
				else
				{
					//cccccc[0]=gsParamDatabase[ui16i].paramName[0];
					//					cccccc[1]=gsParamDatabase[ui16i].paramName[1];
					//					cccccc[2]=gsParamDatabase[ui16i].paramName[2];
					//					cccccc[3]=gsParamDatabase[ui16i].paramName[3];
					GrRectFIllBolymin(0,126,ui8YMin,ui8YMin+11,true,true);
					if(gu8_language == Japanese_IDX)
					{
					memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[ui16i].paramName_japanese,4);
					}
					else
					{
					memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[ui16i].paramName_english,4);
					}
					displayText((unsigned char *)cccccc, 1, ui8YMin, false, false, false, true,false, true);
					if(gu8_language == Japanese_IDX)
					{
					displayText((unsigned char *)gsParamDatabase[ui16i].paramName_japanese+5, PARA_START+4, ui8YMin, false, false, false, true,true, false);
					}
					else
					{
					displayText((unsigned char *)gsParamDatabase[ui16i].paramName_english+5, PARA_START+4, ui8YMin, false, false, false, true,false, true);

					}

				}

				//
				// Increment Y coordinate of the top left corner of the menu item to be displayed
				//
				ui8YMin += MENU_ITEM_HEIGHT;

				paintCounter++;

				lineCounter++;
				if(NUMBER_OF_LINES == lineCounter)
					break;
			}
		}
    }

	return 0;
}

/******************************************************************************
 * FunctionName: parameterListDown
 *
 * Function Description:
 * This function shifts the highlight on parameter item in downward direction by
 * incrementing parameter focus index. This function also paints parameter list
 * again to reflect the change.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/

uint8_t parameterListDown()
{
	uint8_t ui8YMin = 0;
	uint8_t paintCounter = 0;
	uint8_t lineCounter = 0;

	uint16_t ui16i = 0;
	uint16_t lLastIndex = 0;
    char cccccc[5]={0};

    if(gKeysStatus.bits.Key_Down_pressed)
    {
    	gKeysStatus.bits.Key_Down_pressed = 0;

		//
		// Handle exception if there is no active functional block.
		//
		if(psActiveFunctionalBlock == NULL)
			return 1;

		if(0 == psActiveMenu->ui8FocusIndex)
		{
			lLastIndex = TOTAL_PARAMETERS1;
		}
		else if(1 == psActiveMenu->ui8FocusIndex)
		{
			lLastIndex = DRIVE_PARAM_START_INDEX + TOTAL_PARAMETERS2;
		}
		else if(2 == psActiveMenu->ui8FocusIndex)
		{
			lLastIndex = SERVICE_PARAM_START_INDEX + TOTAL_SERVICE_PARAMETERS;
		}
		else
		{
			return 1;
		}

		//
		// Increment the focus index if focus is not on the last menu item of
		// active menu.
		//
		if(gParameterFocusIndex < gTotalParamItemsToRender - 1)
		{
			gParameterFocusIndex++;

			if( ((gParameterFocusIndex % 4) == 0) //&&
				//(gParameterFocusIndex < gTotalParamItemsToRender)
			  )
			{
				ui8YMin = 0;

				if(gCurrentStartIndex < lLastIndex - 1)
					gCurrentStartIndex = gHighlightedItemIndex + 1;
			}
		}

		if(gParameterFocusIndex > NUM_LINES - 1)
		{
			paintCounter = (gParameterFocusIndex / 4) * 4;
		}

		//
		// Clear screen
		//
		//GrRectFIllBolymin(0, 126, 0, 63, true, true);
		GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);

		//
		//	Loop while there exists a menu item to render on screen.
		//
		for(ui16i = gCurrentStartIndex; ui16i < lLastIndex; ui16i++)
		{
			if(gsParamDatabase[ui16i].paramEnabled == true)
			{

				//
				// Draw parameter item on screen. Highlight the parameter item which has current focus
				// given by gParameterFocusIndex.
				//
				if(paintCounter == gParameterFocusIndex)
				{
					//cccccc[0]=gsParamDatabase[ui16i].paramName[0];
					///cccccc[1]=gsParamDatabase[ui16i].paramName[1];
					//cccccc[2]=gsParamDatabase[ui16i].paramName[2];
					//cccccc[3]=gsParamDatabase[ui16i].paramName[3];
					GrRectFIllBolymin_cyw(0,126,ui8YMin,ui8YMin+11,0xff,true);
					if(gu8_language == Japanese_IDX)
					{
					memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[ui16i].paramName_japanese,4);
					}
					else
					{
					memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[ui16i].paramName_english,4);
					}
					displayText((unsigned char*)cccccc, 1, ui8YMin, true, false, false, false,false, true);
					if(gu8_language == Japanese_IDX)
				    {
					displayText((unsigned char*)gsParamDatabase[ui16i].paramName_japanese+5, PARA_START+4, ui8YMin, true, false, false, false,true, false);
				    }
					else
					{
						displayText((unsigned char*)gsParamDatabase[ui16i].paramName_english+5, PARA_START+4, ui8YMin, true, false, false, false,false, true);
					}
					gHighlightedItemIndex = ui16i;
				}
				else
				{
					//cccccc[0]=gsParamDatabase[ui16i].paramName[0];
					//					cccccc[1]=gsParamDatabase[ui16i].paramName[1];
					//					cccccc[2]=gsParamDatabase[ui16i].paramName[2];
					//					cccccc[3]=gsParamDatabase[ui16i].paramName[3];
					GrRectFIllBolymin(0,126,ui8YMin,ui8YMin+11,true,true);
					if(gu8_language == Japanese_IDX)
					{
					memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[ui16i].paramName_japanese,4);
					}
					else
					{
					memcpy((unsigned char *)cccccc,(unsigned char *)gsParamDatabase[ui16i].paramName_english,4);
					}
					displayText((unsigned char *)cccccc, 1, ui8YMin, false, false, false, true,false, true);
					if(gu8_language == Japanese_IDX)
					{
					displayText((unsigned char *)gsParamDatabase[ui16i].paramName_japanese+5, PARA_START+4, ui8YMin, false, false, false, true,true, false);
					}
					else
					{
					displayText((unsigned char *)gsParamDatabase[ui16i].paramName_english+5, PARA_START+4, ui8YMin, false, false, false, true,false, true);
					}
				}

				//
				// Increment Y coordinate of the top left corner of the menu item to be displayed
				//
				ui8YMin += MENU_ITEM_HEIGHT;

				paintCounter++;

				lineCounter++;
				if(NUMBER_OF_LINES == lineCounter)
					break;
			}
		}
    }
	return 0;
}

/******************************************************************************
 * FunctionName: parameterListMode
 *
 * Function Description:
 * This function changes the active functional block to parent functional block.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t parameterListMode()
{
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
    	gKeysStatus.bits.Key_Mode_pressed = 0;

		//
		// Handle exception if there is no active menu.
		//
		if(psActiveFunctionalBlock == NULL)
			return 1;

		if(psActiveFunctionalBlock->parentInternalFunctions != NULL)
		{
			gParameterFocusIndex = 0;
			gTotalParamItemsToRender = 0;
			gHighlightedItemIndex = 0;
			gCurrentStartIndex = 0;
			gParamListRenderStarted = 0;

			psActiveFunctionalBlock = psActiveFunctionalBlock->parentInternalFunctions;
			psActiveFunctionalBlock->pfnPaintFirstScreen();
		}
		else
		{
			return 1;
		}
    }

	return 0;
}

/******************************************************************************
 * FunctionName: parameterListEnter
 *
 * Function Description:
 * This function changes active functional block value as per the type of parameter
 * under setting process.
 *
 * Function Parameters: None
 *
 * Function Returns:
 * 0	:	Success
 *
*********************************************************************************/
uint8_t parameterListEnter()
{
    if(gKeysStatus.bits.Key_Enter_pressed)
    {
    	gKeysStatus.bits.Key_Enter_pressed = 0;

    	if(gsParamDatabase[gHighlightedItemIndex].paramType == eVAL_INT)
    	{
    		gTotalParamItemsToRender = 0;

    		//
    		// change active functional block to value type parameter functional block
    		//
    		psActiveFunctionalBlock = &gsValueTypeParamFunctionalBlock;
    		psActiveFunctionalBlock->pfnPaintFirstScreen();
    	}
    	else if(gsParamDatabase[gHighlightedItemIndex].paramType == eSTATE)
    	{
    		gTotalParamItemsToRender = 0;

    		//
    		// Change active functional block to state type parameter functional block;
    		//
    		psActiveFunctionalBlock = &gsStateTypeParamFunctionalBlock;
    		psActiveFunctionalBlock->pfnPaintFirstScreen();
    	}
    }

	return 0;
}

/******************************************************************************
 * Define Parameter Set Functional Block
*********************************************************************************/
stInternalFunctions gsParameterListFunctionalBlock =
{
	&gsMenuFunctionalBlock,
	0,
	parameterListFirstScreen,
	parameterListRunTime,
	parameterListUp,
	parameterListDown,
	parameterListMode,
	parameterListEnter
};

/****************************************************************************/

