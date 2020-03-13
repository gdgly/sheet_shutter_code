
/*********************************************************************************
* FileName: paramdatabase.h
* Description:
* This header file contains interfaces for the Parameter Database mechanism.
* Version: 0.4D
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
 *  	0.4D	23/07/14									  Addition of Parameters 801, 802, 805 and 806
 *  	0.3D	25/06/2014									  Addition of Param A600
 *  														  Modification of size and addr w.r.t. change of Parameter list as of today;A606 and A607
 *  	0.2D	13/06/2014									  Added Param not found Error code
 *  	0.1D	20/05/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/
#ifndef __PARAMDB_H__
#define __PARAMDB_H__

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
// Total parameters with Control Board
#define _NO_OF_PARAMS 59

// Max logs for Anomaly History
#define _MAX_ANOMALY_LOGS	10

// Success with value loaded from Flash Default
#define _SUCCESS_DEFAULT 0

#define _SUCCESS _SUCCESS_DEFAULT

// Success with value loaded from EEPROM
#define	_SUCCESS_EEPROM 1

// Error code for EEPROM Write or Read operations
#define _ERR_EEPROM 2

// Error code for CRC Mismatch
#define _ERR_PARAMDB_CRC 3

//Error Code if Default and RAM params are not alternately absent
#define _ERR_PARAMNOTFOUND 4

// Starting Index Enum name for Anomaly History and Change Setting History
#define ANOMHIST_START_IDX A626ANOMHIST_1
/****************************************************************************/


/****************************************************************************
 *  Flash variables:
****************************************************************************/
/* Parameter Name Enum (_PARAM) */
typedef enum _PARAM_CTRL{
	A000UPPLIM_STPTIME,
	A001INTLCK_PRIOR,
	A002INTLCK_VALID,
	A003GOUP_OPRDELAY,
	A004GODN_OPRDELAY,
	A005EN_OPRDELAY,
	A006MODEFIX,
	A007EN_SWITCHPAN,
	A009OPR_RSRTTMR,
	A010INTLCK_DELTMR,
	A016CLOSE_OPRSET,
	A020OPR_CNTRES,
	A021INIT_VALUESET,
	A027MOTOR_SHAFT_CTRL,
	A028OPR_CNT_IN,
	A061SENSOR_IN,
	A0621PB_IN,
	A0633PB_IN,
	A071MULTI_FN_OUT1,
	A072MULTI_FN_OUT2,
	A120INIT_SHEETPOSPAR,
	A131EN_APHEIGHT_CTL,
	A200CTRL_BRD_IOSTAT,
	A450CTRL_FWVER,
	A451CTRL_HWVER,
	A600OPRCNT, //Added 25 Jun 14
	A601MODE_AUTO_MAN,
	A602RUN_STOP_CTRL,
	A605STAT_DRIVE_BRDc,
	A606STAT_DRIVE_INSTc,
	A607STAT_DRIVE_FAULTc,
	A608DRV_COMMERRc,
	A609DRV_MOTORERRc,
	A610DRV_APPERRc,
	A611DRV_PROCERRc,
	A612STAT_CTRL_BRD,
	A613CTRL_COMMERR,
	A614CTRL_APPERR,
	A615CTRL_PROCERR,
	A626ANOMHIST_1,
	A627ANOMHIST_2,
	A628ANOMHIST_3,
	A629ANOMHIST_4,
	A630ANOMHIST_5,
	A631ANOMHIST_6,
	A632ANOMHIST_7,
	A633ANOMHIST_8,
	A634ANOMHIST_9,
	A635ANOMHIST_10,
	A638STAT_CTRL_FAULT,
	A801CRCFAIL_CT2DI,
	A802CRCFAIL_CT2DR,
	A805COMMBUFFOF_CT2DI,
	A806COMMBUFFOF_CT2DR,
} PARAM_CTRL;

extern const uint8_t guc_Paramsize_Ctrl[_NO_OF_PARAMS];
extern const uint32_t gui_ParamAddr_Ctrl[_NO_OF_PARAMS];
extern const uint16_t gui_ParamNo_Ctrl[_NO_OF_PARAMS];
extern const uint16_t gui_InitValSetParamNo_table[22];
extern const uint16_t gui_InitSheetPosParamNo_table[5];

extern const uint32_t gu32_ctrl_fwver_DEF;

//----- Parameter Declarations here ---------------------------
extern uint8_t gu8_upplim_stptime;
extern uint8_t gu8_intlck_prior;
extern uint8_t gu8_intlck_valid;
extern uint8_t gu8_goup_oprdelay;
extern uint8_t gu8_godn_oprdelay;
extern uint8_t gu8_en_oprdelay;
extern uint8_t gu8_modefix;
extern uint8_t gu8_en_switchpan;
extern uint8_t gu8_opr_rsrttmr;
extern uint32_t gu32_intlck_deltmr; //version 0.7D of C file
extern uint8_t gu8_close_oprset;
extern uint8_t gu8_opr_cntres;
extern uint8_t gu8_init_valueset;
extern uint8_t gu8_motor_shaft_ctrl;
extern uint32_t gu32_opr_cnt_in;
extern uint8_t gu8_sensor_in;
extern uint8_t gu8_1pb_in;
extern uint8_t gu8_3pb_in;
extern uint8_t gu8_multi_fn_out1;
extern uint8_t gu8_multi_fn_out2;
extern uint8_t gu8_init_sheetpospar;
extern uint8_t gu8_en_apheight_ctl;
//extern uint32_t gu32_ctrl_brd_iostat;
extern uint32_t gu32_ctrl_fwver;
extern uint32_t gu32_ctrl_hwver;
extern uint32_t gu32_oprcnt; //Added 25 Jun 14
extern uint8_t gu8_mode_auto_man;
extern uint8_t gu8_run_stop_ctrl;
//extern uint16_t gu16_stat_drive_brdc;
//extern uint16_t gu16_stat_drive_instc;
//extern uint8_t gu8_stat_drive_faultc;
//extern uint8_t gu8_drv_commerrc;
//extern uint16_t gu16_drv_motorerrc;
//extern uint32_t gu32_drv_apperrc;
//extern uint16_t gu16_drv_procerrc;
//extern uint32_t gu8_stat_ctrl_brd;
//extern uint8_t gu8_ctrl_commerr;
//extern uint32_t gu32_ctrl_apperr;
//extern uint16_t gu16_ctrl_procerr;
extern struct stControlAnomaly gstAH_anomhist_1;
extern struct stControlAnomaly gstAH_anomhist_2;
extern struct stControlAnomaly gstAH_anomhist_3;
extern struct stControlAnomaly gstAH_anomhist_4;
extern struct stControlAnomaly gstAH_anomhist_5;
extern struct stControlAnomaly gstAH_anomhist_6;
extern struct stControlAnomaly gstAH_anomhist_7;
extern struct stControlAnomaly gstAH_anomhist_8;
extern struct stControlAnomaly gstAH_anomhist_9;
extern struct stControlAnomaly gstAH_anomhist_10;
//extern uint8_t gu8_stat_ctrl_fault;
extern uint16_t gu16_crcfail_ct2di;
extern uint16_t gu16_crcfail_ct2dr;
extern uint16_t gu16_commbuffof_ct2di;
extern uint16_t gu16_commbuffof_ct2dr;

extern uint8_t gu8_multi_fn_out3;
extern uint8_t gu8_multi_fn_out4;
extern uint8_t gu8_multi_fn_out5;
extern uint8_t gu8_wireless_1pbs_3pbs_o;

extern uint8_t gu8_snow_mode;

//-----------------------------------------------------------------

//*****************************************************************************
// Function prototypes.
//*****************************************************************************

uint8_t initParameterDB(PARAM_CTRL paramindex);

uint8_t writeParameterUpdateInDB(PARAM_CTRL paramindex,uint8_t* ParamValue);

uint8_t readParameterFromDB(PARAM_CTRL paramindex,uint8_t* readParamValue);

uint8_t computeIndex(PARAM_CTRL AHorCSH_Idx);

uint8_t clearLogs(PARAM_CTRL AHorCSH_Idx);

uint8_t paramLoadDefault(PARAM_CTRL paramindex);

/********************************************************************************/

#endif /*__PARAMDB_H__*/
