/*********************************************************************************
* FileName: paramdatabase.c
* Description:
* This header file contains definitions for Parameter Database mechanism.
* Version: 0.6D
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

//11 Jun 14 - ToDo -
//Done on 13 Jun14-Add logic for writing in RAM instead of EEPROM

//25 jun 14 - ToDo -
//Done-Update Size byte(32->16) for Drive Status parameter A605. Update Address table.

/****************************************************************************
 *  Modification History
 *
 *  Revision		Date                  Name          			Comments
 *  	0.8D	18/08/2014									  Handled Error Code
 *  	0.7D	06/08/14									  Param A010 size changed to 4 bytes
 *  	0.6D	23/07/14									  Addition of Parameters 801, 802, 805 and 806
 *  	0.5D	10/07/14									  Updated Control Board and Drive Board Status and Fault registers according to IntertaskComm module
 *  	0.4D	27/06/14									  Control Anom Hist size changed to 24 from 28 bytes
 *  	0.3D	25/06/14									  Param A605 Drive Status parameter - Global RAM table update
 *  														  Use of existing variable defined in intertaskcommunication module.
 *  														  *** Size Byte and Addr UPDATED
 *  	0.2D	13/06/2014									  Added logic for writing in RAM instead of EEPROM.
 *  	0.1D	20/05/2014      	iGATE Offshore team       Initial Creation
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
#include "eeprom.h"
#include "paramdatabase.h"
#include "Application/interTaskCommunication.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/

#define _MAX_EEPROM_READVALSIZE 35


/****************************************************************************
 *  Global variables
****************************************************************************/
// Control Board Parameters Size Table
const uint8_t guc_Paramsize_Ctrl[_NO_OF_PARAMS] =
{
		1,	1,	1,	1,	1,	1,	1,	1,	1,	4,
		1,	1,  1,	1,	4,	1,	1,	1,	1,	1,
		1,	1,	4,	4,  4,	4,	1,	1,	4,	2,
		1,	1,	2,	4,	2,	1, 	1, 	4, 	2, 24,
		24,24, 24, 24, 24, 24, 24, 24, 24,  1,
		2,	2,  2, 	2, 	1, 	1, 	1,	1,	1
};

// Control Board Parameters Address Table
const uint32_t gui_ParamAddr_Ctrl[_NO_OF_PARAMS] =
{

		0,		3,	 6,	  9,  12,	15,	  18,	 21,  24,	 27,
		33,	   36,  39,	 42,  45,	51,	  54,	 57,  60,	 63,
		66,	   69,  72,	 78,  84,	90,	  96,	 99,  102,	108,
		112,  115, 118,	122, 128,  132,  135,	138,  144,	148,
		174,  200, 226,	252, 278,  304,  330,	356,  382,	408,
		411,  415, 419, 423, 427,  430,  433,   436,  439

};

// Control Board Parameters Number Table
const uint16_t gui_ParamNo_Ctrl[_NO_OF_PARAMS] =
{
		0,	   1,   2,	 3,	  4,   5,   6,	 7,	  9,  10,
		16,	  20,  21,	27,  28,  61,  62,  63,  71,  72,
	   120,	 131, 200, 450, 451, 600, 601, 602, 605, 606,
	   607,  608, 609, 610, 611, 612, 613, 614, 615, 626,
	   627,  628, 629, 630, 631, 632, 633, 634, 635, 638,
	   801,  802, 805, 806,  73,  74,  75,  78,   8,

};

// Table of param no. to initialize w.r.t. A021INIT_VALUESET
const uint16_t gui_InitValSetParamNo_table[22] =
{
		0,	1,	2,	3,	4,	5,
		6,	7,	9,	10,	16,	27,
		28,	61,	62,	63,	71,	72,
		131, 73, 74, 75
};

// Table of param no. to initialize w.r.t. A120INIT_SHEETPOSPAR
//const uint16_t gui_InitSheetPosParamNo_table[5] =
//{
//		100, 101, 102, 130, 131
//};

// Default Values for Control Parameter. This is saved in Flash.
const uint8_t gu8_upplim_stptime_DEF = 10;
const uint8_t gu8_intlck_prior_DEF = 0;
const uint8_t gu8_intlck_valid_DEF = 1;
const uint8_t gu8_goup_oprdelay_DEF = 0;
const uint8_t gu8_godn_oprdelay_DEF = 0;
const uint8_t gu8_en_oprdelay_DEF = 2;   //20171114   0--->2
const uint8_t gu8_modefix_DEF = 0;
const uint8_t gu8_en_switchpan_DEF = 0;
const uint8_t gu8_opr_rsrttmr_DEF = 20;//20160930 modify 10 to 20
const uint8_t gu32_intlck_deltmr_DEF = 1; //version 0.7D
const uint8_t gu8_close_oprset_DEF = 0;
const uint8_t gu8_opr_cntres_DEF = 0;
const uint8_t gu8_init_valueset_DEF = 0;
const uint8_t gu8_motor_shaft_ctrl_DEF = 1;
const uint32_t gu32_opr_cnt_in_DEF = 0;
const uint8_t gu8_sensor_in_DEF = 1;//2016 09 21 modify 0 to 1
const uint8_t gu8_1pb_in_DEF = 1;//2016 09 21 modify 0 to 1
const uint8_t gu8_3pb_in_DEF = 1;//2016 09 21 modify 0 to 1
const uint8_t gu8_multi_fn_out1_DEF = 0;
const uint8_t gu8_multi_fn_out2_DEF = 1;
const uint8_t gu8_init_sheetpospar_DEF = 0;
const uint8_t gu8_en_apheight_ctl_DEF = 0;
//-------------------------------------------
//const uint32_t gu32_ctrl_brd_iostat_DEF = 0; //Removed 30 Jul 14
//-------------------------------------------

//*******************************************
// Set following variable for Firmware version of Control Board
// Format:
// First byte = Not used, 		Second Byte = Not used
// Third byte = Major Version, 	Fourth byte = Minor version
//const uint32_t gu32_ctrl_fwver_DEF = 0x00000409; //Read Default Firmware Version
const uint32_t gu32_ctrl_fwver_DEF = 20030;   //xxxx.x   ﾀ�逎ｺ 17041   17ﾄ�ﾔﾂ ｵﾚ1ｰ豎ｾ

// In dbhandler.c the gu32_ctrl_fwver_DEF variable is used to send response to the display board for version information
//*******************************************

//-------------------------------------------
//const uint32_t gu32_ctrl_hwver_DEF = 1; //Disabled as we need to calculate Hardware firmware version
//-------------------------------------------
const uint32_t gu32_oprcnt_DEF = 0; //Added 25 Jun 14
const uint8_t gu8_mode_auto_man_DEF = 0;
const uint8_t gu8_run_stop_ctrl_DEF = 0;
//-------------------------------------------
//const uint32_t gu16_stat_drive_brdc_DEF = 0;
//const uint16_t gu16_stat_drive_instc_DEF = 0;
//const uint8_t gu8_stat_drive_faultc_DEF = 0;
//const uint32_t gu32_drv_commerrc_DEF = 0;
//const uint32_t gu32_drv_motorerrc_DEF = 0;
//const uint32_t gu32_drv_apperrc_DEF = 0;
//const uint32_t gu32_drv_procerrc_DEF = 0;
//const uint32_t gu32_stat_ctrl_brd_DEF = 0;
//const uint32_t gu32_ctrl_commerr_DEF = 0;
//const uint32_t gu32_ctrl_apperr_DEF = 0;
//const uint32_t gu32_ctrl_procerr_DEF = 0;
//-------------------------------------------
const struct stControlAnomaly gstAH_anomhist_1_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const struct stControlAnomaly gstAH_anomhist_2_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const struct stControlAnomaly gstAH_anomhist_3_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const struct stControlAnomaly gstAH_anomhist_4_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const struct stControlAnomaly gstAH_anomhist_5_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const struct stControlAnomaly gstAH_anomhist_6_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const struct stControlAnomaly gstAH_anomhist_7_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const struct stControlAnomaly gstAH_anomhist_8_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const struct stControlAnomaly gstAH_anomhist_9_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const struct stControlAnomaly gstAH_anomhist_10_DEF = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//-------------------------------------------
//const uint8_t gu8_stat_ctrl_fault_DEF = 0;
//-------------------------------------------
const uint16_t gu16_crcfail_ct2di_DEF = 0;
const uint16_t gu16_crcfail_ct2dr_DEF = 0;
const uint16_t gu16_commbuffof_ct2di_DEF = 0;
const uint16_t gu16_commbuffof_ct2dr_DEF = 0;

//	Added on 04 Dec for new requirement from client
const uint8_t gu8_multi_fn_out3_DEF = 2;
const uint8_t gu8_multi_fn_out4_DEF = 3;
const uint8_t gu8_multi_fn_out5_DEF = 4;
//	Added on 09 Dec for new requirement from client
const uint8_t gu8_wireless_1pbs_3pbs_o_DEF = 1;

const uint8_t gu8_snow_mode_DEF = 0;



//--------------------------------------------------------------------------


//Pointer to Default values in Control Board
const uint8_t * const guc_ptrParamDefault_Ctrl[_NO_OF_PARAMS] =
{
		(uint8_t*)&gu8_upplim_stptime_DEF,
		(uint8_t*)&gu8_intlck_prior_DEF,
		(uint8_t*)&gu8_intlck_valid_DEF,
		(uint8_t*)&gu8_goup_oprdelay_DEF,
		(uint8_t*)&gu8_godn_oprdelay_DEF,
		(uint8_t*)&gu8_en_oprdelay_DEF,
		(uint8_t*)&gu8_modefix_DEF,
		(uint8_t*)&gu8_en_switchpan_DEF,
		(uint8_t*)&gu8_opr_rsrttmr_DEF,
		(uint8_t*)&gu32_intlck_deltmr_DEF, //version 0.7D
		(uint8_t*)&gu8_close_oprset_DEF,
		(uint8_t*)&gu8_opr_cntres_DEF,
		(uint8_t*)&gu8_init_valueset_DEF,
		(uint8_t*)&gu8_motor_shaft_ctrl_DEF,
		(uint8_t*)&gu32_opr_cnt_in_DEF,
		(uint8_t*)&gu8_sensor_in_DEF,
		(uint8_t*)&gu8_1pb_in_DEF,
		(uint8_t*)&gu8_3pb_in_DEF,
		(uint8_t*)&gu8_multi_fn_out1_DEF,
		(uint8_t*)&gu8_multi_fn_out2_DEF,
		(uint8_t*)&gu8_init_sheetpospar_DEF,
		(uint8_t*)&gu8_en_apheight_ctl_DEF,
		0, //		(uint8_t*)&gu32_ctrl_brd_iostat_DEF,
		(uint8_t*)&gu32_ctrl_fwver_DEF, //Read Default Firmware Version
		0, //		(uint8_t*)&gu32_ctrl_hwver_DEF,  //Disabled as we need to calculate Hardware firmware version
		(uint8_t*)&gu32_oprcnt_DEF, //Added 25 Jun 14
		(uint8_t*)&gu8_mode_auto_man_DEF,
		(uint8_t*)&gu8_run_stop_ctrl_DEF,
		0, //		(uint8_t*)&gu16_stat_drive_brdc_DEF,
		0, //		(uint8_t*)&gu16_stat_drive_instc_DEF,
		0, //		(uint8_t*)&gu8_stat_drive_faultc_DEF,
		0, //		(uint8_t*)&gu32_drv_commerrc_DEF,
		0, //		(uint8_t*)&gu32_drv_motorerrc_DEF,
		0, //		(uint8_t*)&gu32_drv_apperrc_DEF,
		0, //		(uint8_t*)&gu32_drv_procerrc_DEF,
		0, //		(uint8_t*)&gu32_stat_ctrl_brd_DEF,
		0, //		(uint8_t*)&gu32_ctrl_commerr_DEF,
		0, //		(uint8_t*)&gu32_ctrl_apperr_DEF,
		0, //		(uint8_t*)&gu32_ctrl_procerr_DEF,
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
		0, //		(uint8_t*)&gu8_stat_ctrl_fault_DEF,
		(uint8_t*)&gu16_crcfail_ct2di_DEF,
		(uint8_t*)&gu16_crcfail_ct2dr_DEF,
		(uint8_t*)&gu16_commbuffof_ct2di_DEF,
		(uint8_t*)&gu16_commbuffof_ct2dr_DEF,

		(uint8_t*)&gu8_multi_fn_out3_DEF,
		(uint8_t*)&gu8_multi_fn_out4_DEF,
		(uint8_t*)&gu8_multi_fn_out5_DEF,
		(uint8_t*)&gu8_wireless_1pbs_3pbs_o_DEF,

		(uint8_t*)&gu8_snow_mode_DEF,

};

//Parameter Global Variables
uint8_t gu8_upplim_stptime = 0;
uint8_t gu8_intlck_prior = 0;
uint8_t gu8_intlck_valid = 0;
uint8_t gu8_goup_oprdelay = 0;
uint8_t gu8_godn_oprdelay = 0;
uint8_t gu8_en_oprdelay = 0;
uint8_t gu8_modefix = 0;
uint8_t gu8_en_switchpan = 0;
uint8_t gu8_opr_rsrttmr = 0;
uint32_t gu32_intlck_deltmr = 0; //version 0.7D
uint8_t gu8_close_oprset = 0;
uint8_t gu8_opr_cntres = 0;
uint8_t gu8_init_valueset = 0;
uint8_t gu8_motor_shaft_ctrl = 0;
uint32_t gu32_opr_cnt_in = 0;
uint8_t gu8_sensor_in = 0;
uint8_t gu8_1pb_in = 0;
uint8_t gu8_3pb_in = 0;
uint8_t gu8_multi_fn_out1 = 0;
uint8_t gu8_multi_fn_out2 = 0;
uint8_t gu8_init_sheetpospar = 0;
uint8_t gu8_en_apheight_ctl = 0;
//uint32_t gu32_ctrl_brd_iostat = 0;
uint32_t gu32_ctrl_fwver = 0;
uint32_t gu32_ctrl_hwver = 0;
uint32_t gu32_oprcnt = 0; //Added 25 Jun 14
uint8_t gu8_mode_auto_man = 0;
uint8_t gu8_run_stop_ctrl = 0;
//uint16_t gu16_stat_drive_brdc = 0;
//uint16_t gu16_stat_drive_instc = 0;
//uint8_t gu8_stat_drive_faultc = 0;
//uint8_t gu8_drv_commerrc = 0;
//uint16_t gu16_drv_motorerrc = 0;
//uint32_t gu32_drv_apperrc = 0;
//uint16_t gu16_drv_procerrc = 0;
//uint32_t gu8_stat_ctrl_brd = 0;
//uint8_t gu8_ctrl_commerr = 0;
//uint32_t gu32_ctrl_apperr = 0;
//uint16_t gu16_ctrl_procerr = 0;
struct stControlAnomaly gstAH_anomhist_1 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
struct stControlAnomaly gstAH_anomhist_2 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
struct stControlAnomaly gstAH_anomhist_3 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
struct stControlAnomaly gstAH_anomhist_4 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
struct stControlAnomaly gstAH_anomhist_5 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
struct stControlAnomaly gstAH_anomhist_6 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
struct stControlAnomaly gstAH_anomhist_7 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
struct stControlAnomaly gstAH_anomhist_8 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
struct stControlAnomaly gstAH_anomhist_9 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
struct stControlAnomaly gstAH_anomhist_10 = {0x00000000, 0x0000, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//uint8_t gu8_stat_ctrl_fault = 0;
uint16_t gu16_crcfail_ct2di = 0;
uint16_t gu16_crcfail_ct2dr = 0;
uint16_t gu16_commbuffof_ct2di = 0;
uint16_t gu16_commbuffof_ct2dr = 0;

uint8_t gu8_multi_fn_out3 = 0;
uint8_t gu8_multi_fn_out4 = 0;
uint8_t gu8_multi_fn_out5 = 0;
uint8_t gu8_wireless_1pbs_3pbs_o = 0;

uint8_t gu8_snow_mode = 0;

//--------------------------------------------------------------------


//pointer to Global Param values
uint8_t *guc_ptrParam_Ctrl[_NO_OF_PARAMS] = {
		(uint8_t*)&gu8_upplim_stptime,
		(uint8_t*)&gu8_intlck_prior,
		(uint8_t*)&gu8_intlck_valid,
		(uint8_t*)&gu8_goup_oprdelay,
		(uint8_t*)&gu8_godn_oprdelay,
		(uint8_t*)&gu8_en_oprdelay,
		(uint8_t*)&gu8_modefix,
		(uint8_t*)&gu8_en_switchpan,
		(uint8_t*)&gu8_opr_rsrttmr,
		(uint8_t*)&gu32_intlck_deltmr, //version 0.7D
		(uint8_t*)&gu8_close_oprset,
		(uint8_t*)&gu8_opr_cntres,
		(uint8_t*)&gu8_init_valueset,
		(uint8_t*)&gu8_motor_shaft_ctrl,
		(uint8_t*)&gu32_opr_cnt_in,
		(uint8_t*)&gu8_sensor_in,
		(uint8_t*)&gu8_1pb_in,
		(uint8_t*)&gu8_3pb_in,
		(uint8_t*)&gu8_multi_fn_out1,
		(uint8_t*)&gu8_multi_fn_out2,
		(uint8_t*)&gu8_init_sheetpospar,
		(uint8_t*)&gu8_en_apheight_ctl,
		//(uint8_t*)&gu32_ctrl_brd_iostat, //No EEPROM storage
		(uint8_t*)&(gstDriveStatusMenu.val), //No EEPROM storage
		(uint8_t*)&gu32_ctrl_fwver, //No EEPROM storage
		(uint8_t*)&gu32_ctrl_hwver, //No EEPROM storage
		(uint8_t*)&gu32_oprcnt, //Added 25 Jun 14
		(uint8_t*)&gu8_mode_auto_man,
		(uint8_t*)&gu8_run_stop_ctrl,
		(uint8_t*)&(gstDriveStatus.val), //		(uint8_t*)&gu32_stat_drive_brdc, //Add-Change - 25 Jun 14 - Use of existing param
		(uint8_t*)&(gstDriveInstallation.val), //		(uint8_t*)&gu16_stat_drive_instc,
		(uint8_t*)&(gstDriveBoardFault.val), //		(uint8_t*)&gu8_stat_drive_faultc,
		(uint8_t*)&(gstDriveCommunicationFault.val), //		(uint8_t*)&gu8_drv_commerrc,
		(uint8_t*)&(gstDriveMotorFault.val), //		(uint8_t*)&gu16_drv_motorerrc,
		(uint8_t*)&(gstDriveApplicationFault.val), //		(uint8_t*)&gu32_drv_apperrc,
		(uint8_t*)&(gstDriveProcessorfault.val), //		(uint8_t*)&gu16_drv_procerrc,
		(uint8_t*)&(gstControlBoardStatus.val), //		(uint8_t*)&gu8_stat_ctrl_brd,
		(uint8_t*)&(gstControlCommunicationFault.val), //		(uint8_t*)&gu8_ctrl_commerr,
		(uint8_t*)&(gstControlApplicationFault.val), //		(uint8_t*)&gu32_ctrl_apperr,
		(uint8_t*)&(gstControlProcessorFault.val), //		(uint8_t*)&gu16_ctrl_procerr,
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
		(uint8_t*)&(gstControlBoardFault.val), //		(uint8_t*)&gu8_stat_ctrl_fault,
		(uint8_t*)&gu16_crcfail_ct2di,
		(uint8_t*)&gu16_crcfail_ct2dr,
		(uint8_t*)&gu16_commbuffof_ct2di,
		(uint8_t*)&gu16_commbuffof_ct2dr,

		(uint8_t*)&gu8_multi_fn_out3,
		(uint8_t*)&gu8_multi_fn_out4,
		(uint8_t*)&gu8_multi_fn_out5,
		(uint8_t*)&gu8_wireless_1pbs_3pbs_o,

		(uint8_t*)&gu8_snow_mode,

};

/****************************************************************************/


/****************************************************************************
 *  Function definitions for this file:
****************************************************************************/
/******************************************************************************
 * Function Name: initParameterDB
 *
 * Function Description: This function must be called during startup to
 * initialize the Parameter values based on CRC check on EEPROM values. If
 * calculated CRC of EEPROM Param Data value doesn't matches with the EEPROM
 * CRC value, then write Default values stored in Flash alongwith CRC in
 * EEPROM. Copy that Param value to global Parameter table.
 *
 * Function Parameters:
 * paramindex - index to the parameter value in table
 *
 * Function Returns:
 * _SUCCESS_DEFAULT	: Success with value loaded from Flash Default
 * _SUCCESS_EEPROM	: Success with value loaded from EEPROM
 * _ERROR_EEPROM :	EEPROM Error
 *
 ********************************************************************************/
uint8_t initParameterDB(PARAM_CTRL paramindex)
{
	uint8_t psize = guc_Paramsize_Ctrl[paramindex];
	uint8_t param_eeprom_val[_MAX_EEPROM_READVALSIZE] = {0};
	uint16_t paramreadcrc;
	uint16_t calccrc;
	uint8_t iSize;
	uint8_t returnfuncid;

	memset(param_eeprom_val, 0, sizeof(param_eeprom_val));

	if(guc_ptrParamDefault_Ctrl[paramindex]) {
		if(EEPROMReadByte(param_eeprom_val, gui_ParamAddr_Ctrl[paramindex], psize+2)) {
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
			if (guc_ptrParam_Ctrl[paramindex])  //Check if not null
				for (iSize = 0; iSize < psize; iSize++) {
					*(guc_ptrParam_Ctrl[paramindex] + (psize - 1) - iSize) =
							param_eeprom_val[iSize];
				}
			returnfuncid = _SUCCESS_EEPROM;
		}
	}
	else {
		if (guc_ptrParam_Ctrl[paramindex])  {//Check if not null
			for (iSize = 0; iSize < psize; iSize++)
				*(guc_ptrParam_Ctrl[paramindex] + (psize - 1) - iSize) = 0;
			returnfuncid = _SUCCESS_DEFAULT;
		}
		else
			returnfuncid = _ERR_PARAMNOTFOUND;
	}

	return returnfuncid;
}
/*****************************************************************************/


/******************************************************************************
 * Function Name: writeParameterUpdateInDB
 *
 * Function Description: This function
 *
 * Function Parameters:
 * paramindex - index to the parameter value in table
 * ParamValue - pointer to Parameter value to write
 *
 * Function Returns:
 * _SUCCESS : Success
 * _ERROR_EEPROM :	EEPROM Error
 *
 ********************************************************************************/
uint8_t writeParameterUpdateInDB(PARAM_CTRL paramindex,uint8_t* ParamValue)
{
	uint8_t psize = guc_Paramsize_Ctrl[paramindex];
	uint8_t paramEEP_Write_Val[_MAX_EEPROM_READVALSIZE] = {0};
	uint16_t calccrc;
	uint8_t iSize;

	memset(paramEEP_Write_Val, 0, sizeof(paramEEP_Write_Val));

	for (iSize = 0; iSize < psize; iSize++) {
		//		paramEEP_Write_Val[iSize] = ParamValue[iSize]; //Disabled for debug
		paramEEP_Write_Val[iSize] = *(ParamValue + (psize-1)-iSize);
	}

	if(guc_ptrParamDefault_Ctrl[paramindex]) {
		calccrc = ROM_Crc16(0, paramEEP_Write_Val, psize);

		paramEEP_Write_Val[psize] = (calccrc >> 8) & 0xFF;
		paramEEP_Write_Val[psize+1] = calccrc & 0xFF;


		//Write value into EEPROM
		if(EEPROMProgramByte(paramEEP_Write_Val, gui_ParamAddr_Ctrl[paramindex], psize+2))
		{

			//Log Error
			return _ERR_EEPROM;
		}
		else;
	}
	//Update global param variable
	if(guc_ptrParam_Ctrl[paramindex]) //Check if not null
		for (iSize = 0; iSize < psize; iSize++) {
			*(guc_ptrParam_Ctrl[paramindex] + (psize-1)-iSize) =
					//		*(guc_ptrParam[paramindex] + iSize) =
					paramEEP_Write_Val[iSize];
		}

	return _SUCCESS;
}
/*****************************************************************************/


/******************************************************************************
 * Function Name: readParameterFromDB
 *
 * Function Description: Read value from Global Parameter, if present.
 * If global variable not available, then very and read from EEPROM.
 * IMPORTANT - Call initParameterDB() before using this function
 *
 * Function Parameters:
 * paramindex - index to the parameter value in table
 * readParamValue - read value in this variable
 *
 * Function Returns:
 * _SUCCESS : Success
 * _ERROR_EEPROM :	EEPROM Error
  * _ERR_PARAMDB_CRC : CRC Mismatch Error
 *
 ********************************************************************************/
uint8_t readParameterFromDB(PARAM_CTRL paramindex,uint8_t* readParamValue)
{
	uint8_t psize = guc_Paramsize_Ctrl[paramindex];
	uint8_t param_EEP_Read_Val[_MAX_EEPROM_READVALSIZE] = {0};
	uint16_t calccrc, paramreadcrc;
	uint8_t iSize;

	if(guc_ptrParam_Ctrl[paramindex]) { //Check if not null

		for (iSize = 0; iSize < psize; iSize++) {
//			readParamValue[(psize-1)-iSize] = *(guc_ptrParam[paramindex] + (psize-1)-iSize);
			readParamValue[iSize] = *(guc_ptrParam_Ctrl[paramindex] + iSize);
		}
	}
	else {
		//Read from EEPROM
		memset(param_EEP_Read_Val, 0, sizeof(param_EEP_Read_Val));

		if(EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Ctrl[paramindex], psize+2)) {
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
			gstControlProcessorFault.bits.eepromParameterDbCRC = 0;
			for (iSize = 0; iSize < psize; iSize++) {
				readParamValue[(psize-1)-iSize] = param_EEP_Read_Val[iSize];
			}
		}
		else { //CRC MISMATCH
			gstControlProcessorFault.bits.eepromParameterDbCRC = 1;
			return _ERR_PARAMDB_CRC;
		}

	}

	return _SUCCESS;
}
/*****************************************************************************/


/******************************************************************************
 * Function Name: computeIndex
 *
 * Function Description: Computes the relative index for Anomaly History and
 * Change Setting History to write in EEPROM.
 * Indexing range is 1 to 10 for max 10 logs.
 *
 * Function Parameters: Parameter for Anomaly History or Change Setting log
 * starting index.
 * Possible Values: ANOMHIST_START_IDX, CHGSETHIST_START_IDX
 *
 * Function Returns: Relative index in the Log table
 *
 ********************************************************************************/
uint8_t computeIndex(PARAM_CTRL AHorCSH_Idx)
{
	uint8_t psize = guc_Paramsize_Ctrl[AHorCSH_Idx];
	uint8_t iSize, mSize;
	uint8_t toWriteIdx = 1;
	uint8_t param_EEP_Read_Val[_MAX_EEPROM_READVALSIZE];

	for (iSize = 0; iSize < _MAX_ANOMALY_LOGS; iSize++)
	{
		//		if(!guc_ptrParam[gui8_AnomalyHistory_ParamIdx + iSize])
		memset(param_EEP_Read_Val, 0, sizeof(param_EEP_Read_Val));
		if(!EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Ctrl[AHorCSH_Idx + iSize], psize))
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
/****************************************************************************/


/******************************************************************************
 * Function Name: clearLogs
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns: void
 *
 ********************************************************************************/
uint8_t clearLogs(PARAM_CTRL AHorCSH_Idx)
{
	uint8_t psize = guc_Paramsize_Ctrl[AHorCSH_Idx];
	uint8_t iSize, msize;
	uint8_t param_EEP_Write_Val[_MAX_EEPROM_READVALSIZE] = {0};

	memset(param_EEP_Write_Val, 0, sizeof(param_EEP_Write_Val));

	for (iSize = 0; iSize < _MAX_ANOMALY_LOGS; iSize++) //_MAX_ANOMALY_LOGS
	{
		//Write value into EEPROM
		if(EEPROMProgramByte(param_EEP_Write_Val, gui_ParamAddr_Ctrl[AHorCSH_Idx + iSize], psize+2))
		{
			//Log Error
			return _ERR_EEPROM;
		}
		else;

		//Update global param variable
		if(guc_ptrParam_Ctrl[AHorCSH_Idx + iSize]) //Check if not null
			for (msize = 0; msize < psize; msize++)
			{
				*(guc_ptrParam_Ctrl[AHorCSH_Idx + iSize] + (psize - 1) - msize) = 0;
			}
	}

	return _SUCCESS;
}
/****************************************************************************/


/*****************************************************************************
 * Function Name: paramLoadDefault
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns: _SUCCESS or _ERR_EEPROM
 *
 * **************************************************************************/
uint8_t paramLoadDefault(PARAM_CTRL paramindex)
{
	uint8_t psize = guc_Paramsize_Ctrl[paramindex];
	uint8_t param_eeprom_val[_MAX_EEPROM_READVALSIZE] = {0};
	uint16_t calccrc;
	uint8_t iSize;

	memset(param_eeprom_val, 0, sizeof(param_eeprom_val));
	for (iSize = 0; iSize < psize; iSize++)
	{
		// Check if Default Parameter in Flash is present
		if(guc_ptrParamDefault_Ctrl[paramindex])
			param_eeprom_val[iSize] = *(guc_ptrParamDefault_Ctrl[paramindex] + (psize-1)-iSize);
		else
			param_eeprom_val[iSize] = 0;
	}

//	for (iSize = 0; iSize < psize; iSize++) {
//		param_eeprom_val[iSize] = *(guc_ptrParamDefault_Ctrl[paramindex] +
//				(psize-1)-iSize);
//	}

	calccrc = ROM_Crc16(0, param_eeprom_val, psize);
	param_eeprom_val[psize] = (calccrc >> 8) & 0xFF;
	param_eeprom_val[psize+1] = calccrc & 0xFF;


	//Write Default param value into EEPROM
	if(EEPROMProgramByte(param_eeprom_val, gui_ParamAddr_Ctrl[paramindex], psize+2))
	{
		//Log Error
		return _ERR_EEPROM;
	}
	else;

	//Update global param variable
	if(guc_ptrParam_Ctrl[paramindex]) //Check if not null
	{
		for (iSize = 0; iSize < psize; iSize++)
		{
			//*(guc_ptrParam[paramindex] + (psize-1)-iSize) =
			*(guc_ptrParam_Ctrl[paramindex] + (psize - 1) - iSize) = param_eeprom_val[iSize];
		}
	}

	return _SUCCESS;
}

