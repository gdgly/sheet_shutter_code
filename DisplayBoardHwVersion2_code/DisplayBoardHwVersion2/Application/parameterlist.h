/*********************************************************************************
 * FileName: parameterlist.h
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
#ifndef PARAMETERLIST_H_
#define PARAMETERLIST_H_

/****************************************************************************
 *  Includes:
****************************************************************************/
#include "intertaskcommunication.h"
/****************************************************************************/

/****************************************************************************
 *  Macros
****************************************************************************/
#define TOTAL_PARAMETERS			65  //60  //63 //61	//57 //80 //103          //201806_Bug_No.9

#define TOTAL_PARAMETERS1	21  //17            201806_Bug_No.9
#define TOTAL_PARAMETERS2	44	//35 //34 //30	//54 //76
#define TOTAL_SERVICE_PARAMETERS	 6

#define SHUTTER_PARAM_START_INDEX	0
#define DRIVE_PARAM_START_INDEX		21  //17   //201806_Bug_No.9
#define SERVICE_PARAM_START_INDEX	64  //61	//57 //55	//74   //201806_Bug_No.9

#define NUMBER_OF_LINES		4
#define MENU_ITEM_HEIGHT	16

#define NUMBER_OF_COLUMNS	40

#define PARA_START 24
/****************************************************************************/

/****************************************************************************
 *  Enumerations
****************************************************************************/
enum parameterType
{
	eVAL_INT,
	eVAL_FLOAT,
	eSTATE
};

///* Indexes of Drive EEPROM parameters */
//typedef enum _PARAM_DRV
//{
//	A100_UPPER_STOPPING_POS = 0,
//	A101_LOWER_STOPPING_POS,
//	A102_PHOTO_ELEC_POS,
//	A128_ORIGIN_SENS_POS,
//	A619_ORIGIN_SENS_DRIFT,
//	A129_CURRENT_VAL_MONITOR,
//	A130_APERTURE_HEIGHT_POS,
//	A131_APERTURE_MODE_ENABLE,                              // common block end
//	A008_SNOW_MODE_PHOTOELEC,
//	A021_INITIAL_VAL_SETTING,
//	A025_MAINTENANCE_COUNT_LIMIT,
//	A618_MAINTENANCE_COUNT_VALUE,
//	A080_MICRO_SENS_COUNT,
//	A081_MICRO_SENS_COUNT_RESET,
//	A112_OVERRUN_PROTECT,
//	A120_RESET_TO_DEFAULT,
//	A125_POWERUP_CALIB,
//	A126_CORRECTED_FREQ_APERTURE,
//	A127_AUTO_CORRECT_ENABLED,
//	A549_DRIVE_FW_VER,
//	A550_DRIVE_HW_VER,
//	A600_OPERATION_COUNT,
//	A603_MICRO_SENS_LIM_VALUE,
//	A604_APERTURE_HEIGHT_OPER_COUNT,                 		// appl block end
//	A605_DRIVE_BOARD_STATUS,
//	A606_DRIVE_INSTALLATION,
//	A607_DRIVE_FAULT,
//	A608_DRIVE_BOARD_COMM_FAULT,
//	A609_DRIVE_MOTOR_FAULT,
//	A610_DRIVE_APPL_FAULT,
//	A611_DRIVE_PROCESSOR_FAULT,
//	A616_ANOMALY_HISTORY_1,
//	A616_ANOMALY_HISTORY_2,
//	A616_ANOMALY_HISTORY_3,
//	A616_ANOMALY_HISTORY_4,
//	A616_ANOMALY_HISTORY_5,
//	A616_ANOMALY_HISTORY_6,
//	A616_ANOMALY_HISTORY_7,
//	A616_ANOMALY_HISTORY_8,
//	A616_ANOMALY_HISTORY_9,
//	A616_ANOMALY_HISTORY_10,                                 // appl status block end
//	A011_DECEL_BY_PHOTO_ELEC_BLOCKING_LIM,
//	A011_WAIT_FOR_STOPPAGE,
//	A103_RISE_CHANGE_GEAR_POS1,
//	A104_RISE_CHANGE_GEAR_POS2,
//	A105_RISE_CHANGE_GEAR_POS3,
//	A106_FALL_CHANGE_GEAR_POS1,
//	A107_FALL_CHANGE_GEAR_POS2,
//	A108_FALL_CHANGE_GEAR_POS3,
//	A110_SHUTTER_REVERSE_OP_MIN_LIM,
//	A500_PWM_FREQ_MOTOR_CTRL,
//	A501_STARTUP_DUTY_CYCLE,
//	A504_MAX_STARTUP_TIME_LIM,
//	A505_STARTUP_SECTOR_CONST,
//	A506_timer2_pre_scalar,
//	A507_timer2_to_rpm,
//	A508_timer2_min,
//	A509_timer2_max,
//	A510_min_duty_cycle,
//	A511_break_enabled,
//	A512_speed_PI_prop_gain,
//	A513_speed_PI_integral_gain,
//	A514_current_PI_prop_gain,
//	A515_current_PI_integral_gain,
//	A516_output_freq,
//	A517_inch_speed,
//	A518_drive_status,
//	A519_current_error,
//	A520_acceleration1_up,
//	A521_deceleration1_up,
//	A522_s1_up,
//	A523_s2_up,
//	A524_s3_up,
//	A525_up_step_count,
//	A526_acceleration1_down,
//	A527_deceleration1_down,
//	A528_s1_down,
//	A529_s2_down,
//	A530_s3_down,
//	A531_down_step_count,
//	A536_shutter_length,
//	A537_shutter_type,
//	A538_OV_limit,
//	A539_OI_limit,
//	A540_OS_limit,
//	A541_OF_limit,
//	A542_thermal_protection,
//	A543_torque_const,
//	A544_backEMF_const,
//	A545_speed_const,
//	A546_rated_speed,
//	A547_rated_current,
//	A548_pole_pairs, // motor block end
//} PARAM_DRV;
//
///* Indexes of Display EEPROM parameters */
//typedef enum _PARAM_DISP{
//   A030ANOMHIST_1,
//   A031ANOMHIST_2,
//   A032ANOMHIST_3,
//   A033ANOMHIST_4,
//   A034ANOMHIST_5,
//   A035ANOMHIST_6,
//   A036ANOMHIST_7,
//   A037ANOMHIST_8,
//   A038ANOMHIST_9,
//   A039ANOMHIST_10,
//   A040ERASE_ANOMHIST, //Anomaly Reset Parameter
//   A050CHGSETHIST_1,
//   A051CHGSETHIST_2,
//   A052CHGSETHIST_3,
//   A053CHGSETHIST_4,
//   A054CHGSETHIST_5,
//   A055CHGSETHIST_6,
//   A056CHGSETHIST_7,
//   A057CHGSETHIST_8,
//   A058CHGSETHIST_9,
//   A059CHGSETHIST_10,
//   A060ERASE_CHGSETHIST, //Change Setting Reset Parameter
//   A452DISP_FWVER,
//   A453DISP_HWVER
//} PARAM_DISP;
//
///* Indexes of Control EEPROM parameters */
//typedef enum _PARAM_CTRL{
//   A000UPPLIM_STPTIME,
//   A001INTLCK_PRIOR,
//   A002INTLCK_VALID,
//   A003GOUP_OPRDELAY,
//   A004GODN_OPRDELAY,
//   A005EN_OPRDELAY,
//   A006MODEFIX,
//   A007EN_SWITCHPAN,
//   A009OPR_RSRTTMR,
//   A010INTLCK_DELTMR,
//   A016_CLOSE_OPR_SET,
//   A020OPR_CNTRES,
//   A021INIT_VALUESET,
//   A027MOTOR_SHAFT_CTRL,
//   A028OPR_CNT_IN,
//   A061SENSOR_IN,
//   A0621PB_IN,
//   A0633PB_IN,
//   A071MULTI_FN_OUT1,
//   A072MULTI_FN_OUT2,
//   A120INIT_SHEETPOSPAR,
//   A131EN_APHEIGHT_CTL,
//   A200CTRL_BRD_IOSTAT,
//   A450CTRL_FWVER,
//   A451CTRL_HWVER,
//   A600OPRCNT,
//   A601MODE_AUTO_MAN,
//   A602RUN_STOP_CTRL,
//   A605STAT_DRIVE_BRD,
//   A610STAT_CTRL_BRD,
//   A611CTRL_COMMERR,
//   A612CTRL_APPERR,
//   A613CTRL_PROCERR,
//   A614ANOMHIST_1,
//   A615ANOMHIST_2,
//   A616ANOMHIST_3,
//   A617ANOMHIST_4,
//   A618ANOMHIST_5,
//   A619ANOMHIST_6,
//   A620ANOMHIST_7,
//   A621ANOMHIST_8,
//   A622ANOMHIST_9,
//   A623ANOMHIST_10
//} PARAM_CTRL;


/****************************************************************************/

/****************************************************************************
 *  Structures
****************************************************************************/
typedef struct _stParamDatabase
{
	uint8_t paramIndex;
	bool paramEnabled;
	bool readOnly;
	bool isSigned;
	enum destinationAddress destination;
	//unsigned char paramName[21];
	unsigned char paramName_japanese[40];
	uint8_t paramType;
	uint16_t paramEEPROMIndex;

	struct _valueTypeEntities
	{
		union values
		{
			uint32_t ui32Val;
			float 	fVal;
		} minVal, maxVal;

		unsigned char *pUnitString;

	} valueTypeEntities;

	struct _stateTypeEntities
	{
		unsigned char *pStateString_japanese;
		uint8_t paramStateCount;
		unsigned char *pStateString_english;

	} stateTypeEntities;
	unsigned char paramName_english[40];

} stParamDatabase;

/****************************************************************************/

/****************************************************************************
 *  Global variables for other files:
****************************************************************************/
extern const stParamDatabase gsParamDatabase[TOTAL_PARAMETERS];
extern const unsigned char cucA537_SH_TYPE_States[][40];

extern uint8_t gParameterFocusIndex;
extern uint8_t gHighlightedItemIndex;
extern uint8_t gCurrentStartIndex;
extern uint8_t gTotalParamItemsToRender;
extern uint8_t gParamListRenderStarted;
/****************************************************************************/








#endif /* PARAMETERLIST_H_ */
