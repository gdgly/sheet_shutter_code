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
 *  TODO:
 *  09/05/14- Done-Add support for separate Anomaly History and Change Setting
 *  History Indexing
 *
 ****************************************************************************/

/****************************************************************************
 *  Modification History
 *
 *  Revision		Date                  Name          			Comments
 *  	0.4D	23/07/2014									  Addition of Parameters 800 and 804
 *  	0.3D	16/05/2014									  Addition of Display DB
 *  	0.2D	09/05/2014									  ParamDB module complete
 *  	0.1D	06/05/2014      	iGATE Offshore team       Initial Creation
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
#define _MAX_EEPROM_READVALSIZE 35

// Total parameters with Display Board
//#define _NO_OF_PARAMS 39//add guest ann lcd backlight
#define _NO_OF_PARAMS 59  //add 20 err opreation count  ////20170414      201703_No.31

// Max logs for Anomaly History and Change Setting History
#define _MAX_ANOMALY_LOGS	20
#define _MAX_CHGSETHIST_LOGS 10

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
#define ANOMHIST_START_IDX		A030ANOMHIST_1
#define ANOMHIST_END_IDX		A030ANOMHIST_1 + _MAX_ANOMALY_LOGS - 1

#define CHGSETHIST_START_IDX	A050CHGSETHIST_1
#define CHGSETHIST_END_IDX	A050CHGSETHIST_1 + _MAX_CHGSETHIST_LOGS - 1

#define Japanese_IDX  0
#define English_IDX 1


#define Para_Guesture_Index_cyw        11
#define Para_LcdBackLight_Index_cyw    10
#define Para_Languange_Index_cyw       12
/****************************************************************************/


/****************************************************************************
 *  Flash variables:
****************************************************************************/
/* Indexes of Drive EEPROM parameters */
typedef enum _DriveEEPROMData
{
                A100_UPPER_STOPPING_POS = 0,
                A101_LOWER_STOPPING_POS,
                A102_PHOTO_ELEC_POS,
                A128_ORIGIN_SENS_POS,
                A637_ORIGIN_SENS_DRIFT,
                A129_CURRENT_VAL_MONITOR,
                A130_APERTURE_HEIGHT_POS,
                A131_APERTURE_MODE_ENABLE, // common block end

                A008_SNOW_MODE_PHOTOELEC,
                A021_INITIAL_VAL_SETTING,
                A025_MAINTENANCE_COUNT_LIMIT,
                A636_MAINTENANCE_COUNT_VALUE,
                A080_MICRO_SENS_COUNT,
                A081_MICRO_SENS_COUNT_RESET,
                A112_OVERRUN_PROTECT,
                A120_RESET_TO_DEFAULT,
                A125_POWERUP_CALIB,
                A126_CORRECTED_FREQ_APERTURE,
                A127_AUTO_CORRECT_ENABLED,
                A549_DRIVE_FW_VER,
                A550_DRIVE_HW_VER,
                A600_OPERATION_COUNT,
                A603_MICRO_SENS_LIM_VALUE,
                A604_APERTURE_HEIGHT_OPER_COUNT, // appl block end

                A605_DRIVE_BOARD_STATUS,
                A606_DRIVE_INSTALLATION,
                A607_DRIVE_FAULT,
                A608_DRIVE_BOARD_COMM_FAULT,
                A609_DRIVE_MOTOR_FAULT,
                A610_DRIVE_APPL_FAULT,
                A611_DRIVE_PROCESSOR_FAULT,

                A616_ANOMALY_HISTORY_1,
                A617_ANOMALY_HISTORY_2,
                A618_ANOMALY_HISTORY_3,
                A619_ANOMALY_HISTORY_4,
                A620_ANOMALY_HISTORY_5,
                A621_ANOMALY_HISTORY_6,
                A622_ANOMALY_HISTORY_7,
                A623_ANOMALY_HISTORY_8,
                A624_ANOMALY_HISTORY_9,
                A625_ANOMALY_HISTORY_10, // appl status block end

                A011_DECEL_BY_PHOTO_ELEC_BLOCKING_LIM,
                A011_WAIT_FOR_STOPPAGE,

                A103_RISE_CHANGE_GEAR_POS1,
                A104_RISE_CHANGE_GEAR_POS2,
                A105_RISE_CHANGE_GEAR_POS3,

                A106_FALL_CHANGE_GEAR_POS1,
                A107_FALL_CHANGE_GEAR_POS2,
                A108_FALL_CHANGE_GEAR_POS3,

                A110_SHUTTER_REVERSE_OP_MIN_LIM,
                A500_PWM_FREQ_MOTOR_CTRL,
                A501_STARTUP_DUTY_CYCLE,
                A504_MAX_STARTUP_TIME_LIM,
                A505_STARTUP_SECTOR_CONST,

                A506_timer2_pre_scalar,
                A507_timer2_to_rpm,
                A508_timer2_min,
                A509_timer2_max,
                A510_min_duty_cycle,
                A511_break_enabled,
                A512_speed_PI_prop_gain,
                A513_speed_PI_integral_gain,
                A514_current_PI_prop_gain,
                A515_current_PI_integral_gain,
                A516_output_freq,
                A517_inch_speed,
                A518_drive_status,
                A519_current_error,
                A520_acceleration1_up,
                A521_deceleration1_up,
                A522_s1_up,
                A523_s2_up,
                A524_s3_up,
                A525_up_step_count,
                A526_acceleration1_down,
                A527_deceleration1_down,
                A528_s1_down,
                A529_s2_down,
                A530_s3_down,
                A531_down_step_count,
                A536_shutter_length,
                A537_shutter_type,
                A538_OV_limit,
                A539_OI_limit,
                A540_OS_limit,
                A541_OF_limit,
                A542_thermal_protection,
                A543_torque_const,
                A544_backEMF_const,
                A545_speed_const,
                A546_rated_speed,
                A547_rated_current,
                A548_pole_pairs, // motor block end

                number_of_drive_parameters

}enDriveEEPROMData;


/* Parameter Name Enum (_PARAM) Display Board */
typedef enum _PARAM_DISP{
	A030ANOMHIST_1,
	A031ANOMHIST_2,
	A032ANOMHIST_3,
	A033ANOMHIST_4,
	A034ANOMHIST_5,
	A035ANOMHIST_6,
	A036ANOMHIST_7,
	A037ANOMHIST_8,
	A038ANOMHIST_9,
	A039ANOMHIST_10,
	A040ERASE_ANOMHIST, //Anomaly Reset Parameter
	A050CHGSETHIST_1,
	A051CHGSETHIST_2,
	A052CHGSETHIST_3,
	A053CHGSETHIST_4,
	A054CHGSETHIST_5,
	A055CHGSETHIST_6,
	A056CHGSETHIST_7,
	A057CHGSETHIST_8,
	A058CHGSETHIST_9,
	A059CHGSETHIST_10,
	A060ERASE_CHGSETHIST, //Change Setting Reset Parameter
	A452DISP_FWVER,
	A453DISP_HWVER,
	A800CRCFAIL_DI2CT,
	A804COMMBUFFOF_DI2CT,
	A026BACK_LIGHT,
	A900ANOMHIST_11,
	A901ANOMHIST_12,
	A902ANOMHIST_13,
	A903ANOMHIST_14,
	A904ANOMHIST_15,
	A905ANOMHIST_16,
	A906ANOMHIST_17,
	A907ANOMHIST_18,
	A908ANOMHIST_19,
	A909ANOMHIST_20,
	A041GUESTURE,
	A042lANGUANG,
	//20170414      201703_No.31 start
	A910ANOMHIST_OP1,
	A911ANOMHIST_OP2,
	A912ANOMHIST_OP3,
	A913ANOMHIST_OP4,
	A914ANOMHIST_OP5,
	A915ANOMHIST_OP6,
	A916ANOMHIST_OP7,
	A917ANOMHIST_OP8,
	A918ANOMHIST_OP9,
	A919ANOMHIST_OP10,
	A920ANOMHIST_OP11,
	A921ANOMHIST_OP12,
	A922ANOMHIST_OP13,
	A923ANOMHIST_OP14,
	A924ANOMHIST_OP15,
	A925ANOMHIST_OP16,
	A926ANOMHIST_OP17,
	A927ANOMHIST_OP18,
	A928ANOMHIST_OP19,
	A929ANOMHIST_OP20,
	////20170414      201703_No.31 end
} PARAM_DISP;

/* Parameter Name Enum (_PARAM) Control Board */
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

extern const uint8_t guc_Paramsize_Disp[_NO_OF_PARAMS];
extern const uint32_t gui_ParamAddr_Disp[_NO_OF_PARAMS];
extern const uint16_t gui_ParamNo_Disp[_NO_OF_PARAMS];

extern const uint32_t gu32_disp_fwver_DEF;

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
extern uint8_t gu8_erase_anomhist;
extern struct stChangeSettingHistory gstCSH_chgsethist_1;
extern struct stChangeSettingHistory gstCSH_chgsethist_2;
extern struct stChangeSettingHistory gstCSH_chgsethist_3;
extern struct stChangeSettingHistory gstCSH_chgsethist_4;
extern struct stChangeSettingHistory gstCSH_chgsethist_5;
extern struct stChangeSettingHistory gstCSH_chgsethist_6;
extern struct stChangeSettingHistory gstCSH_chgsethist_7;
extern struct stChangeSettingHistory gstCSH_chgsethist_8;
extern struct stChangeSettingHistory gstCSH_chgsethist_9;
extern struct stChangeSettingHistory gstCSH_chgsethist_10;
extern uint8_t gu8_erase_chgsethist;
extern uint32_t gu32_disp_fwver;
extern uint32_t gu32_disp_hwver;
extern uint16_t gu16_crcfail_di2ct;
extern uint16_t gu16_commbuffof_di2ct;
extern uint8_t  gu8_language;
//*****************************************************************************
// Function prototypes.
//*****************************************************************************
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
uint8_t initParameterDB(PARAM_DISP paramindex);


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
uint8_t writeParameterUpdateInDB(PARAM_DISP paramindex,uint8_t* ParamValue);


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
uint8_t readParameterFromDB(PARAM_DISP paramindex,uint8_t* readParamValue);


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
uint8_t computeIndex(PARAM_DISP AHorCSH_Idx);

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
uint8_t clearLogs(PARAM_DISP AHorCSH_Idx);

/*****************************************************************************
 * Function Name: paramLoadDefault
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns: void
 *
 * **************************************************************************/
uint8_t paramLoadDefault(PARAM_DISP paramindex);

/********************************************************************************/

#endif /*__PARAMDB_H__*/
