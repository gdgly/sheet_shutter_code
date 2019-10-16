/********************************************************************************
* FileName: EEPROM.h
* Description:  
* This header file contains the declaration of all the attributes and 
* services for EEPROM.c file. It implements the handling of  
* EEPROM interface, reading parameters to RAM on init, 
* and writing back as and when required or on Power down 
*********************************************************************************/

/****************************************************************************
 * Copyright 2014 Bunka Shutters.
 * This program is the property of the Bunka Shutters
 * Company, Inc.and it shall not be reproduced, distributed or used
 * without permission of an authorized company official.This is an
 * unpublished work subject to Trade Secret and Copyright
 * protection.
*****************************************************************************/

/****************************************************************************
 *  Modification History
 *  
 *  Date                  Name          Comments 
 *  07/05/2014            iGate          Initial Creation                                                               
*****************************************************************************/
#ifndef EEPROM_H
#define EEPROM_H
#include "./Common/Typedefs/Typedefs.h"

#define ZERO_OFFSET 				0
#define DRIVE_BLOCK_CRC_DAT_LENGTH	sizeof(WORD)
#define BLOCK_SEPERATION_LEN    64

#define BEAD_SHUTTER    0
#define M1_SHUTTER      1

///* ** EEPRPM commands to read write data from EEPROM** */

//#define EEPROM_PAGE_SIZE    (unsigned)64
//#define EEPROM_PAGE_MASK    (unsigned)0x003f
//#define EEPROM_CMD_READ     (unsigned)0b00000011//Read data from memory array beginning at selected address
//#define EEPROM_CMD_WRITE    (unsigned)0b00000010//Write data to memory array beginning at selected address
//#define EEPROM_CMD_WRDI     (unsigned)0b00000100//Reset the write enable latch (disable write operations)	
//#define EEPROM_CMD_WREN     (unsigned)0b00000110//Set the write enable latch (enable write operations)
//#define EEPROM_CMD_RDSR     (unsigned)0b00000101//Read STATUS register
//#define EEPROM_CMD_WRSR     (unsigned)0b00000001//Write STATUS register


///************************************************************************
//* Aliases for IOs registers related to SPI connected to EEPROM          *
//*                                                                       *    
//************************************************************************/

//#define EEPROM_SS_TRIS      TRISBbits.TRISB0
//#define EEPROM_SS_PORT      PORTBbits.RB0
//#define EEPROM_SCK_TRIS     TRISCbits.TRISC3
//#define EEPROM_SDO_TRIS     TRISAbits.TRISA4
//#define EEPROM_SDI_TRIS     TRISAbits.TRISA9

///************************************************************************
//* Structure STATREG and union _EEPROMStatus_                            *
//*                                                                       *
//* Overview: Provide a bits and byte access to EEPROM status value.      *
//*                                                                       *
//************************************************************************/
//struct  STATREG{
//	unsigned    WIP:1;
//	unsigned    WEL:1;
//	unsigned    BP0:1;
//	unsigned    BP1:1;
//	unsigned    RESERVED:3;
//	unsigned    WPEN:1;
//};

//union _EEPROMStatus_{
//	struct  STATREG Bits;
//	unsigned char	Char;
//};

//*************************************************************************************
// This block defines the indexes required to access the EEPROM block that will contain 
// data that will be used by both the Application and the Ramp Generator modules  

#define UPPER_STOPPING_POS_DAT_LENGTH 		sizeof(WORD)
#define LOWER_STOPPING_POS_DAT_LENGTH 		sizeof(WORD)
#define PHOTOELEC_POS_DAT_LENGTH 			sizeof(WORD)
#define ORIGIN_SENS_POS_DAT_LENGTH 			sizeof(WORD)
#define ORIGIN_SENSOR_DRIFT_DAT_LENGTH		sizeof(WORD)
#define CURRENT_VAL_MONITOR_DAT_LENGTH 		sizeof(WORD)
#define APERTURE_HEIGHT_POS_DAT_LENGTH 		sizeof(WORD)
#define APERTURE_MODE_ENABLED_DAT_LENGTH 	sizeof(BYTE)
#define COMMON_BLOCK_DAT_LENGTH				(UPPER_STOPPING_POS_DAT_LENGTH + LOWER_STOPPING_POS_DAT_LENGTH \
												+ PHOTOELEC_POS_DAT_LENGTH + ORIGIN_SENS_POS_DAT_LENGTH \
												+ ORIGIN_SENSOR_DRIFT_DAT_LENGTH + CURRENT_VAL_MONITOR_DAT_LENGTH \
												+ APERTURE_HEIGHT_POS_DAT_LENGTH + APERTURE_MODE_ENABLED_DAT_LENGTH \
												+ DRIVE_BLOCK_CRC_DAT_LENGTH)

#define EEP_COMMON_BLOCK_START 		0x0000  
#define EEP_UPPER_STOPPING_POS		EEP_COMMON_BLOCK_START 		+ ZERO_OFFSET
#define EEP_LOWER_STOPPING_POS		EEP_UPPER_STOPPING_POS 		+ UPPER_STOPPING_POS_DAT_LENGTH 
#define EEP_PHOTOELEC_POS			EEP_LOWER_STOPPING_POS 		+ LOWER_STOPPING_POS_DAT_LENGTH
#define EEP_ORIGIN_SENS_POS			EEP_PHOTOELEC_POS 			+ PHOTOELEC_POS_DAT_LENGTH  
#define EEP_ORIGIN_SENS_DRIFT_POS	EEP_ORIGIN_SENS_POS 		+ ORIGIN_SENS_POS_DAT_LENGTH 
#define EEP_CURRENT_VAL_MONITOR		EEP_ORIGIN_SENS_DRIFT_POS	+ ORIGIN_SENSOR_DRIFT_DAT_LENGTH
#define EEP_APERTURE_HEIGHT_POS		EEP_CURRENT_VAL_MONITOR 	+ CURRENT_VAL_MONITOR_DAT_LENGTH  
#define EEP_APERTURE_MODE_ENABLED	EEP_APERTURE_HEIGHT_POS 	+ APERTURE_HEIGHT_POS_DAT_LENGTH  
#define EEP_COMMON_BLOCK_CRC		EEP_APERTURE_MODE_ENABLED 	+ APERTURE_MODE_ENABLED_DAT_LENGTH  
#define EEP_COMMON_BLOCK_LENGTH 	EEP_COMMON_BLOCK_CRC 		+ DRIVE_BLOCK_CRC_DAT_LENGTH



//*************************************************************************************
// This block defines the indexes required to access the EEPROM block that will contain 
// data that is used by the Application module 

#define SNOW_MODE_DAT_LENGTH					sizeof(BYTE)
#define INIT_VAL_SETTING_DAT_LENGTH				sizeof(BYTE)
#define MAINTENANCE_COUNT_LIMIT_DAT_LENGTH		sizeof(WORD)
#define MAINTENANCE_COUNT_VALUE_DAT_LENGTH		sizeof(WORD)
#define MICRO_SENSOR_COUNT_DAT_LENGTH			sizeof(BYTE)
#define MICRO_SENS_COUNT_RESET_DAT_LENGTH		sizeof(BYTE)
#define OVERRUN_PROTECT_DAT_LENGTH				sizeof(BYTE)
#define RESET_TO_DEFAULT_DAT_LENGTH				sizeof(BYTE)
#define POWER_UP_CALIB_DAT_LENGTH				sizeof(BYTE)
//	Data length of correctedFreqAperture_A126 changed to WORD from BYTE as parameter range is 10 to 9999 - Jan 2016
#define CORRECTED_FREQ_APERTURE_DAT_LENGTH		sizeof(WORD)
#define CORRECTION_ENABLED_DAT_LENGTH			sizeof(BYTE)
#define DRIVE_FW_VER_DAT_LENGTH					sizeof(DWORD)
#define DRIVE_HW_VER_DAT_LENGTH					sizeof(DWORD)
#define OPERATION_COUNT_DAT_LENGTH				sizeof(DWORD)
#define MICRO_SENSOR_LIMIT_VAL_DAT_LENGTH		sizeof(BYTE)
#define APERTURE_HEIGHT_OP_COUNT_DAT_LENGTH		sizeof(DWORD)
#define OPERATION_COUNT_RESET_DAT_LENGTH		sizeof(BYTE)
#define OPERATION_COUNTER_DAT_LENGTH			sizeof(DWORD)
#define APPL_BLOCK_DAT_LENGTH					(SNOW_MODE_DAT_LENGTH + INIT_VAL_SETTING_DAT_LENGTH \
													+ MAINTENANCE_COUNT_LIMIT_DAT_LENGTH + MAINTENANCE_COUNT_VALUE_DAT_LENGTH \
													+ MICRO_SENSOR_COUNT_DAT_LENGTH \
													+ MICRO_SENS_COUNT_RESET_DAT_LENGTH + OVERRUN_PROTECT_DAT_LENGTH \
													+ RESET_TO_DEFAULT_DAT_LENGTH + POWER_UP_CALIB_DAT_LENGTH \
													+ CORRECTED_FREQ_APERTURE_DAT_LENGTH + CORRECTION_ENABLED_DAT_LENGTH \
													+ DRIVE_FW_VER_DAT_LENGTH + DRIVE_HW_VER_DAT_LENGTH \
													+ OPERATION_COUNT_DAT_LENGTH + MICRO_SENSOR_LIMIT_VAL_DAT_LENGTH \
													+ APERTURE_HEIGHT_OP_COUNT_DAT_LENGTH + OPERATION_COUNT_RESET_DAT_LENGTH \
													+ OPERATION_COUNTER_DAT_LENGTH + DRIVE_BLOCK_CRC_DAT_LENGTH)




#define EEP_APPL_BLOCK_START 			EEP_COMMON_BLOCK_LENGTH + BLOCK_SEPERATION_LEN
#define EEP_SNOW_MODE_PHOTOELEC			EEP_APPL_BLOCK_START 			+ ZERO_OFFSET
#define EEP_INITIAL_VAL_SETTING			EEP_SNOW_MODE_PHOTOELEC 		+ SNOW_MODE_DAT_LENGTH
#define EEP_MAINTENANCE_COUNT_LIMIT		EEP_INITIAL_VAL_SETTING 		+ INIT_VAL_SETTING_DAT_LENGTH
#define EEP_MAINTENANCE_COUNT_VALUE		EEP_MAINTENANCE_COUNT_LIMIT		+ MAINTENANCE_COUNT_LIMIT_DAT_LENGTH
#define EEP_MICRO_SENS_COUNT			EEP_MAINTENANCE_COUNT_VALUE		+ MAINTENANCE_COUNT_VALUE_DAT_LENGTH
#define EEP_MICRO_SENS_COUNT_RESET		EEP_MICRO_SENS_COUNT 			+ MICRO_SENSOR_COUNT_DAT_LENGTH 
#define EEP_OVERRUN_PROTECT				EEP_MICRO_SENS_COUNT_RESET 		+ MICRO_SENS_COUNT_RESET_DAT_LENGTH
#define EEP_RESET_TO_DEFAULT			EEP_OVERRUN_PROTECT				+ OVERRUN_PROTECT_DAT_LENGTH
#define EEP_POWER_UP_CALIB				EEP_RESET_TO_DEFAULT 			+ RESET_TO_DEFAULT_DAT_LENGTH
#define EEP_CORRECTED_FREQ_APERTURE		EEP_POWER_UP_CALIB 				+ POWER_UP_CALIB_DAT_LENGTH
#define EEP_CORRECTION_ENABLED			EEP_CORRECTED_FREQ_APERTURE 	+ CORRECTED_FREQ_APERTURE_DAT_LENGTH
#define EEP_DRIVE_FW_VER				EEP_CORRECTION_ENABLED 			+ CORRECTION_ENABLED_DAT_LENGTH
#define EEP_DRIVE_HW_VER				EEP_DRIVE_FW_VER 				+ DRIVE_FW_VER_DAT_LENGTH
#define EEP_OPERATION_COUNT				EEP_DRIVE_HW_VER 				+ DRIVE_HW_VER_DAT_LENGTH
#define EEP_MICRO_SENSOR_LIMIT_VAL		EEP_OPERATION_COUNT 			+ OPERATION_COUNT_DAT_LENGTH
#define EEP_APERTURE_HEIGHT_OP_COUNT	EEP_MICRO_SENSOR_LIMIT_VAL 		+ MICRO_SENSOR_LIMIT_VAL_DAT_LENGTH
#define EEP_OPERATION_COUNT_RESET		EEP_APERTURE_HEIGHT_OP_COUNT 	+ APERTURE_HEIGHT_OP_COUNT_DAT_LENGTH
#define EEP_OPERATION_COUNTER			EEP_OPERATION_COUNT_RESET		+ OPERATION_COUNT_RESET_DAT_LENGTH
#define EEP_APPL_BLOCK_CRC				EEP_OPERATION_COUNTER			+ OPERATION_COUNTER_DAT_LENGTH
#define EEP_APPL_BLOCK_LENGTH 			EEP_APPL_BLOCK_CRC 				+ DRIVE_BLOCK_CRC_DAT_LENGTH





//*************************************************************************************
// This block defines the indexes required to access the EEPROM block that will contain 
// Drive Status and Fault Registers + Anomaly History 

#define DRIVE_STATUS_DAT_LENGTH				sizeof(DWORD)
#define DRIVE_INTALL_STATUS_DAT_LENGTH		sizeof(BYTE)
#define DRIVE_FAULT_DAT_LENGTH				sizeof(BYTE)
#define DRIVE_COMM_FAULT_DAT_LENGTH			sizeof(BYTE)
#define DRIVE_MOTOR_FAULT_DAT_LENGTH		sizeof(WORD)
#define DRIVE_APPL_FAULT_DAT_LENGTH			sizeof(DWORD)
#define DRIVE_PROCESR_FAULT_DAT_LENGTH		sizeof(WORD)
#define DRIVE_ANOMALY_HIST_DAT_LENGTH		sizeof(BYTE)
#define DRIVE_ANOMALY_HIST_BLK_LENGTH		10 // 10 bytes block 
#define DRIVE_CRC_FAILURE_COUNT_DAT_LENGTH	sizeof(WORD)
#define DRIVE_COMM_BUF_OVRFLW_CNT_DAT_LENTH	sizeof(WORD)
#define DRIVE_APPL_STAT_FAULT_BLOCK_DAT_LENGTH	(DRIVE_STATUS_DAT_LENGTH + DRIVE_INTALL_STATUS_DAT_LENGTH \
													+ DRIVE_FAULT_DAT_LENGTH + DRIVE_COMM_FAULT_DAT_LENGTH \
													+ DRIVE_MOTOR_FAULT_DAT_LENGTH + DRIVE_APPL_FAULT_DAT_LENGTH \
													+ DRIVE_PROCESR_FAULT_DAT_LENGTH + DRIVE_ANOMALY_HIST_BLK_LENGTH \
													+ DRIVE_CRC_FAILURE_COUNT_DAT_LENGTH + DRIVE_COMM_BUF_OVRFLW_CNT_DAT_LENTH \
													+ DRIVE_BLOCK_CRC_DAT_LENGTH)

#define EEP_APPL_STAT_FAULT_BLOCK_START 	0x09FF		// TBD - arbitrary value assumed for development 
#define EEP_DRIVE_STATUS 					EEP_APPL_STAT_FAULT_BLOCK_START 	+ ZERO_OFFSET
#define EEP_DRIVE_INSTALL_STATUS 			EEP_DRIVE_STATUS 					+ DRIVE_STATUS_DAT_LENGTH
#define EEP_DRIVE_FAULT		 				EEP_DRIVE_INSTALL_STATUS 			+ DRIVE_INTALL_STATUS_DAT_LENGTH
#define EEP_DRIVE_COMM_FAULT	 			EEP_DRIVE_FAULT 					+ DRIVE_FAULT_DAT_LENGTH
#define EEP_DRIVE_MOTOR_FAULT				EEP_DRIVE_COMM_FAULT 				+ DRIVE_COMM_FAULT_DAT_LENGTH
#define EEP_DRIVE_APPL_FAULT				EEP_DRIVE_MOTOR_FAULT 				+ DRIVE_MOTOR_FAULT_DAT_LENGTH
#define EEP_DRIVE_PROCESR_FAULT				EEP_DRIVE_APPL_FAULT 				+ DRIVE_APPL_FAULT_DAT_LENGTH
#define EEP_DRIVE_ANOMALY_HIST_1			EEP_DRIVE_PROCESR_FAULT 			+ DRIVE_PROCESR_FAULT_DAT_LENGTH
#define EEP_DRIVE_ANOMALY_HIST_2			EEP_DRIVE_ANOMALY_HIST_1 			+ DRIVE_ANOMALY_HIST_DAT_LENGTH
#define EEP_DRIVE_ANOMALY_HIST_3			EEP_DRIVE_ANOMALY_HIST_2 			+ DRIVE_ANOMALY_HIST_DAT_LENGTH
#define EEP_DRIVE_ANOMALY_HIST_4			EEP_DRIVE_ANOMALY_HIST_3 			+ DRIVE_ANOMALY_HIST_DAT_LENGTH
#define EEP_DRIVE_ANOMALY_HIST_5			EEP_DRIVE_ANOMALY_HIST_4 			+ DRIVE_ANOMALY_HIST_DAT_LENGTH
#define EEP_DRIVE_ANOMALY_HIST_6			EEP_DRIVE_ANOMALY_HIST_5 			+ DRIVE_ANOMALY_HIST_DAT_LENGTH
#define EEP_DRIVE_ANOMALY_HIST_7			EEP_DRIVE_ANOMALY_HIST_6 			+ DRIVE_ANOMALY_HIST_DAT_LENGTH
#define EEP_DRIVE_ANOMALY_HIST_8			EEP_DRIVE_ANOMALY_HIST_7 			+ DRIVE_ANOMALY_HIST_DAT_LENGTH
#define EEP_DRIVE_ANOMALY_HIST_9			EEP_DRIVE_ANOMALY_HIST_8 			+ DRIVE_ANOMALY_HIST_DAT_LENGTH
#define EEP_DRIVE_ANOMALY_HIST_10			EEP_DRIVE_ANOMALY_HIST_9 			+ DRIVE_ANOMALY_HIST_DAT_LENGTH
#define EEP_DRIVE_CRC_FAIL_COUNT			EEP_DRIVE_ANOMALY_HIST_10			+ DRIVE_ANOMALY_HIST_DAT_LENGTH
#define EEP_DRIVE_BUF_OVRFLW_COUNT			EEP_DRIVE_CRC_FAIL_COUNT			+ DRIVE_CRC_FAILURE_COUNT_DAT_LENGTH
#define EEP_APPL_STAT_FAULT_BLOCK_CRC 		EEP_DRIVE_BUF_OVRFLW_COUNT			+ DRIVE_COMM_BUF_OVRFLW_CNT_DAT_LENTH
#define EEP_APPL_STAT_FAULT_BLOCK_LENGTH 	EEP_APPL_STAT_FAULT_BLOCK_CRC 		+ DRIVE_BLOCK_CRC_DAT_LENGTH



//*************************************************************************************
// This block defines the indexes required to access the EEPROM block that will contain 
// data that is used by the Motor control module 

#define DECEL_BY_PHOTO_BLCK_LIM_DAT_LEN		sizeof(BYTE)
#define WAIT_FOR_STOPPAGE_DAT_LENGTH		sizeof(BYTE)
#define RISE_CHANGE_GEAR_POS1_DAT_LENGTH	sizeof(WORD)
#define RISE_CHANGE_GEAR_POS2_DAT_LENGTH	sizeof(WORD)
#define RISE_CHANGE_GEAR_POS3_DAT_LENGTH	sizeof(WORD)

#define FALL_CHANGE_GEAR_POS1_DAT_LENGTH	sizeof(WORD)
#define FALL_CHANGE_GEAR_POS2_DAT_LENGTH	sizeof(WORD)
#define FALL_CHANGE_GEAR_POS3_DAT_LENGTH	sizeof(WORD)

#define SHTER_REV_OP_MIN_LIM_DAT_LENGTH	sizeof(BYTE)
#define PWM_FREQ_MOTOR_CTRL_DAT_LENGTH	sizeof(WORD)
#define STARTUP_DUTY_CYCLE_DAT_LENGTH	sizeof(WORD)
#define MAX_STARTUP_TIME_LIM_DAT_LENGTH	sizeof(WORD)
#define STARTUP_SECTOR_CONST_DAT_LENGTH	sizeof(WORD)

#define TIMER2_PRE_SCALAR_DAT_LENGTH	sizeof(WORD) 
#define TIMER2_TO_RPM_DAT_LENGTH		sizeof(WORD)
#define TIMER2_MIN_DAT_LENGTH		sizeof(WORD)
#define TIMER2_MAX_DAT_LENGTH		sizeof(WORD)
#define MIN_DUTY_CYCLE_DAT_LENGTH	sizeof(WORD)
#define BREAK_ENABLED_DAT_LENGTH	sizeof(BYTE)
#define SPEED_PI_KP_DAT_LENGTH		sizeof(WORD)
#define SPEED_PI_KI_DAT_LENGTH		sizeof(BYTE)
#define CURRENT_PI_KP_DAT_LENGTH	sizeof(WORD)
#define CURRENT_PI_KI_DAT_LENGTH	sizeof(BYTE)
#define OUTPUT_FREQ_DAT_LENGTH		sizeof(WORD)

#define INCH_SPEED_DAT_LENGTH		sizeof(BYTE)
#define DRIVE_STAT_DAT_LENGTH		sizeof(BYTE)
#define CURRENT_ERROR_DAT_LENGTH	sizeof(BYTE)
#define ACCEL1_UP_DAT_LENGTH		sizeof(DWORD)
#define DECEL1_UP_DAT_LENGTH		sizeof(DWORD)
#define S1_UP_DAT_LENGTH			sizeof(WORD)
#define S2_UP_DAT_LENGTH			sizeof(WORD)
#define S3_UP_DAT_LENGTH			sizeof(WORD)

#define UP_STEP_COUNT_DAT_LENGTH	sizeof(BYTE)
#define ACCEL1_DOWN_DAT_LENGTH		sizeof(DWORD)
#define DECEL1_DOWN_DAT_LENGTH		sizeof(DWORD)
#define S1_DOWN_DAT_LENGTH			sizeof(WORD)
#define S2_DOWN_DAT_LENGTH			sizeof(WORD)
#define S3_DOWN_DAT_LENGTH			sizeof(WORD)
#define DOWN_STEP_COUNT_DAT_LENGTH	sizeof(BYTE)
#define SHUTTER_LEN_DAT_LENGTH		sizeof(BYTE)

#define SHUTTER_TYPE_DAT_LENGTH		sizeof(BYTE)
#define OV_LIM_DAT_LENGTH			sizeof(WORD)
#define OI_LIM_DAT_LENGTH			sizeof(WORD)
#define OS_LIM_DAT_LENGTH			sizeof(WORD)
#define OF_LIM_DAT_LENGTH			sizeof(WORD)
#define THERMAL_PROTECT_DAT_LENGTH	sizeof(BYTE)

#define TORQUE_CONST_DAT_LENGTH		sizeof(WORD)
#define BACK_EMF_CONST_DAT_LENGTH	sizeof(WORD)
#define SPEED_CONST_DAT_LENGTH		sizeof(WORD)
#define RATED_SPEED_DAT_LENGTH		sizeof(WORD)
#define RATED_CURRENT_DAT_LENGTH	sizeof(DWORD)
#define POLE_PAIRS_DAT_LENGTH		sizeof(BYTE)
#define JOG_SPEED_DAT_LENGTH		sizeof(BYTE)

#define DRIVE_MOTOR_BLOCK_DAT_LENGTH (DECEL_BY_PHOTO_BLCK_LIM_DAT_LEN + WAIT_FOR_STOPPAGE_DAT_LENGTH + RISE_CHANGE_GEAR_POS1_DAT_LENGTH \
									+ RISE_CHANGE_GEAR_POS2_DAT_LENGTH + RISE_CHANGE_GEAR_POS3_DAT_LENGTH + FALL_CHANGE_GEAR_POS1_DAT_LENGTH \
									+ FALL_CHANGE_GEAR_POS2_DAT_LENGTH + FALL_CHANGE_GEAR_POS3_DAT_LENGTH + SHTER_REV_OP_MIN_LIM_DAT_LENGTH \
									+ PWM_FREQ_MOTOR_CTRL_DAT_LENGTH + STARTUP_DUTY_CYCLE_DAT_LENGTH + MAX_STARTUP_TIME_LIM_DAT_LENGTH \
									+ STARTUP_SECTOR_CONST_DAT_LENGTH + TIMER2_PRE_SCALAR_DAT_LENGTH + TIMER2_TO_RPM_DAT_LENGTH \
									+ TIMER2_MIN_DAT_LENGTH + TIMER2_MAX_DAT_LENGTH + MIN_DUTY_CYCLE_DAT_LENGTH \
									+ BREAK_ENABLED_DAT_LENGTH + SPEED_PI_KP_DAT_LENGTH + SPEED_PI_KI_DAT_LENGTH \
									+ CURRENT_PI_KP_DAT_LENGTH + CURRENT_PI_KI_DAT_LENGTH + OUTPUT_FREQ_DAT_LENGTH \
									+ INCH_SPEED_DAT_LENGTH + DRIVE_STAT_DAT_LENGTH + CURRENT_ERROR_DAT_LENGTH \
									+ ACCEL1_UP_DAT_LENGTH + DECEL1_UP_DAT_LENGTH + S1_UP_DAT_LENGTH \
									+ S2_UP_DAT_LENGTH + S3_UP_DAT_LENGTH + UP_STEP_COUNT_DAT_LENGTH + ACCEL1_DOWN_DAT_LENGTH \
									+ DECEL1_DOWN_DAT_LENGTH + S1_DOWN_DAT_LENGTH + S2_DOWN_DAT_LENGTH + S3_DOWN_DAT_LENGTH \
									+ DOWN_STEP_COUNT_DAT_LENGTH + SHUTTER_LEN_DAT_LENGTH + SHUTTER_TYPE_DAT_LENGTH \
									+ OV_LIM_DAT_LENGTH + OI_LIM_DAT_LENGTH + OS_LIM_DAT_LENGTH + OF_LIM_DAT_LENGTH \
									+ THERMAL_PROTECT_DAT_LENGTH + TORQUE_CONST_DAT_LENGTH + BACK_EMF_CONST_DAT_LENGTH \
									+ SPEED_CONST_DAT_LENGTH + RATED_SPEED_DAT_LENGTH + RATED_CURRENT_DAT_LENGTH + POLE_PAIRS_DAT_LENGTH \
									+ JOG_SPEED_DAT_LENGTH + DRIVE_BLOCK_CRC_DAT_LENGTH)//832




#define EEP_MOTOR_CTRL_BLOCK_START 		EEP_APPL_BLOCK_LENGTH + BLOCK_SEPERATION_LEN
#define EEP_DECEL_BY_PHOTO_BLCK_LIM		EEP_MOTOR_CTRL_BLOCK_START 	+ ZERO_OFFSET
#define EEP_WAIT_FOR_STOPPAGE			EEP_DECEL_BY_PHOTO_BLCK_LIM + DECEL_BY_PHOTO_BLCK_LIM_DAT_LEN
#define EEP_RISE_CHANGE_GEAR_POS1		EEP_WAIT_FOR_STOPPAGE 		+ WAIT_FOR_STOPPAGE_DAT_LENGTH
#define EEP_RISE_CHANGE_GEAR_POS2		EEP_RISE_CHANGE_GEAR_POS1 	+ RISE_CHANGE_GEAR_POS1_DAT_LENGTH
#define EEP_RISE_CHANGE_GEAR_POS3		EEP_RISE_CHANGE_GEAR_POS2 	+ RISE_CHANGE_GEAR_POS2_DAT_LENGTH
#define EEP_FALL_CHANGE_GEAR_POS1		EEP_RISE_CHANGE_GEAR_POS3 	+ RISE_CHANGE_GEAR_POS3_DAT_LENGTH
#define EEP_FALL_CHANGE_GEAR_POS2		EEP_FALL_CHANGE_GEAR_POS1 	+ FALL_CHANGE_GEAR_POS1_DAT_LENGTH
#define EEP_FALL_CHANGE_GEAR_POS3		EEP_FALL_CHANGE_GEAR_POS2 	+ FALL_CHANGE_GEAR_POS2_DAT_LENGTH
#define EEP_SHTER_REV_OP_MIN_LIM		EEP_FALL_CHANGE_GEAR_POS3 	+ FALL_CHANGE_GEAR_POS3_DAT_LENGTH
#define EEP_PWM_FREQ_MOTOR_CTRL			EEP_SHTER_REV_OP_MIN_LIM 	+ SHTER_REV_OP_MIN_LIM_DAT_LENGTH
#define EEP_STARTUP_DUTY_CYCLE			EEP_PWM_FREQ_MOTOR_CTRL 	+ PWM_FREQ_MOTOR_CTRL_DAT_LENGTH
#define EEP_MAX_STARTUP_TIME_LIM		EEP_STARTUP_DUTY_CYCLE 		+ STARTUP_DUTY_CYCLE_DAT_LENGTH
#define EEP_STARTUP_SECTOR_CONST		EEP_MAX_STARTUP_TIME_LIM 	+ MAX_STARTUP_TIME_LIM_DAT_LENGTH
#define EEP_TIMER2_PRE_SCALAR			EEP_STARTUP_SECTOR_CONST 	+ STARTUP_SECTOR_CONST_DAT_LENGTH
#define EEP_TIMER2_TO_RPM				EEP_TIMER2_PRE_SCALAR 		+ TIMER2_PRE_SCALAR_DAT_LENGTH
#define EEP_TIMER2_MIN					EEP_TIMER2_TO_RPM 			+ TIMER2_TO_RPM_DAT_LENGTH
#define EEP_TIMER2_MAX					EEP_TIMER2_MIN 			+ TIMER2_MIN_DAT_LENGTH
#define EEP_MIN_DUTY_CYCLE				EEP_TIMER2_MAX 			+ TIMER2_MAX_DAT_LENGTH
#define EEP_BREAK_ENABLED				EEP_MIN_DUTY_CYCLE 		+ MIN_DUTY_CYCLE_DAT_LENGTH
#define EEP_SPEED_PI_KP					EEP_BREAK_ENABLED 		+ BREAK_ENABLED_DAT_LENGTH
#define EEP_SPEED_PI_KI					EEP_SPEED_PI_KP			+ SPEED_PI_KP_DAT_LENGTH
#define EEP_CURRENT_PI_KP				EEP_SPEED_PI_KI			+ SPEED_PI_KI_DAT_LENGTH
#define EEP_CURRENT_PI_KI				EEP_CURRENT_PI_KP		+ CURRENT_PI_KP_DAT_LENGTH 
#define EEP_OUTPUT_FREQ					EEP_CURRENT_PI_KI 		+ CURRENT_PI_KI_DAT_LENGTH
#define EEP_INCH_SPEED					EEP_OUTPUT_FREQ 		+ OUTPUT_FREQ_DAT_LENGTH
#define EEP_DRIVE_STAT					EEP_INCH_SPEED	 		+ INCH_SPEED_DAT_LENGTH
#define EEP_CURRENT_ERROR				EEP_DRIVE_STAT		 	+ DRIVE_STAT_DAT_LENGTH
#define EEP_ACCEL1_UP					EEP_CURRENT_ERROR 		+ CURRENT_ERROR_DAT_LENGTH
#define EEP_DECEL1_UP					EEP_ACCEL1_UP 			+ ACCEL1_UP_DAT_LENGTH
#define EEP_S1_UP						EEP_DECEL1_UP 			+ DECEL1_UP_DAT_LENGTH
#define EEP_S2_UP						EEP_S1_UP 				+ S1_UP_DAT_LENGTH
#define EEP_S3_UP 						EEP_S2_UP				+ S2_UP_DAT_LENGTH
#define EEP_UP_STEP_COUNT				EEP_S3_UP				+ S3_UP_DAT_LENGTH
#define EEP_ACCEL1_DOWN					EEP_UP_STEP_COUNT 		+ UP_STEP_COUNT_DAT_LENGTH
#define EEP_DECEL1_DOWN					EEP_ACCEL1_DOWN			+ ACCEL1_DOWN_DAT_LENGTH
#define EEP_S1_DOWN						EEP_DECEL1_DOWN 		+ DECEL1_DOWN_DAT_LENGTH
#define EEP_S2_DOWN						EEP_S1_DOWN 			+ S1_DOWN_DAT_LENGTH
#define EEP_S3_DOWN						EEP_S2_DOWN 			+ S2_DOWN_DAT_LENGTH
#define EEP_DOWN_STEP_COUNT				EEP_S3_DOWN				+ S3_DOWN_DAT_LENGTH
#define EEP_SHUTTER_LEN					EEP_DOWN_STEP_COUNT		+ DOWN_STEP_COUNT_DAT_LENGTH
#define EEP_SHUTTER_TYPE				EEP_SHUTTER_LEN			+ SHUTTER_LEN_DAT_LENGTH
#define EEP_OV_LIM						EEP_SHUTTER_TYPE 		+ SHUTTER_TYPE_DAT_LENGTH
#define EEP_OI_LIM						EEP_OV_LIM 				+ OV_LIM_DAT_LENGTH
#define EEP_OS_LIM						EEP_OI_LIM 				+ OI_LIM_DAT_LENGTH
#define EEP_OF_LIM						EEP_OS_LIM 				+ OS_LIM_DAT_LENGTH
#define EEP_THERMAL_PROTECT				EEP_OF_LIM				+ OF_LIM_DAT_LENGTH
#define EEP_TORQUE_CONST				EEP_THERMAL_PROTECT		+ THERMAL_PROTECT_DAT_LENGTH
#define EEP_BACK_EMF					EEP_TORQUE_CONST		+ TORQUE_CONST_DAT_LENGTH
#define EEP_SPEED_CONST					EEP_BACK_EMF 			+ BACK_EMF_CONST_DAT_LENGTH
#define EEP_RATED_SPEED					EEP_SPEED_CONST 		+ SPEED_CONST_DAT_LENGTH
#define EEP_RATED_CURRENT				EEP_RATED_SPEED			+ RATED_SPEED_DAT_LENGTH
#define EEP_POLE_PAIRS					EEP_RATED_CURRENT		+ RATED_CURRENT_DAT_LENGTH
#define EEP_JOG_SPEED					EEP_POLE_PAIRS			+ POLE_PAIRS_DAT_LENGTH
#define EEP_DRIVE_BLOCK_CRC				EEP_JOG_SPEED			+ JOG_SPEED_DAT_LENGTH
#define EEP_MOTOR_CTRL_BLOCK_LENGTH 	EEP_DRIVE_BLOCK_CRC		+ DRIVE_BLOCK_CRC_DAT_LENGTH

#define DRIVE_FLASH_BLOCK_LENGTH ((COMMON_BLOCK_DAT_LENGTH \
								+ EEP_COMMON_BLOCK_LENGTH \
								+ APPL_BLOCK_DAT_LENGTH \
								+ EEP_APPL_BLOCK_LENGTH \
								+ DRIVE_APPL_STAT_FAULT_BLOCK_DAT_LENGTH \
								+ EEP_APPL_STAT_FAULT_BLOCK_LENGTH \
								+ DRIVE_MOTOR_BLOCK_DAT_LENGTH \
								+ EEP_MOTOR_CTRL_BLOCK_LENGTH) - 8)

#define EEP_SHUTTER_INSTALLATION_STEP_LENGTH   sizeof(BYTE)    //bug_NO.43

#define EEP_SHUTTER_INSTALLATION_STEP   (EEP_MOTOR_CTRL_BLOCK_LENGTH + BLOCK_SEPERATION_LEN + BLOCK_SEPERATION_LEN + EEP_SHUTTER_INSTALLATION_STEP_LENGTH)

typedef union // Overall union for containing Motor Control EEPROM block in RAM 
{
	UINT8 val[DRIVE_MOTOR_BLOCK_DAT_LENGTH]; 
	struct 
	{
		UINT8 decelByPhotoElecBlckingLim_A011;
		UINT8 waitForStoppage_A011; 
		UINT16 riseChangeGearPos1_A103; 
		UINT16 riseChangeGearPos2_A104; 
		UINT16 riseChangeGearPos3_A105; 
		
		UINT16 fallChangeGearPos1_A106; 
		UINT16 fallChangeGearPos2_A107; 
		UINT16 fallChangeGearPos3_A108; 

		UINT8 	shtrRevOperMinLimit_A110; 
		UINT16 	PWMFreqMotorCtrl_A500; 
		UINT16 	startupDutyCycle_A501; 
		UINT16 	maxStartupTimeLim_A504; 
		UINT16	startupSectorConst_A505; 

		UINT16  timer2PreScalar_A506; 
		UINT16	timer2RPM_A507; 
		UINT16	timer2Min_A508; 
		UINT16	timer2Max_A509; 
		UINT16	minDutyCycle_A510; 

		UINT8	breakEnabled_A511; 
		UINT16	speed_PI_KP_A512; 
		UINT8	speed_PI_KI_A513; 
		UINT16	current_PI_KP_A514;
		UINT8	current_PI_KI_A515;

		UINT16	outputFreq_A516;
		UINT8	inchSpeed_A517; 
		UINT8	driveStatus_A518; 
		UINT8	currentError_A519; 
		UINT32	accel1Up_A520; 
		UINT32	decel1Up_A521; 

		UINT16  s1Up_A522; 
		UINT16  s2Up_A523; 
		UINT16  s3Up_A524; 
		UINT8	upStepCount_A525; 
		UINT32	accel1Down_A526; 
		UINT32	decel1Down_A527; 

		UINT16  s1Down_A528; 
		UINT16  s2Down_A529; 
		UINT16  s3Down_A530; 
		UINT8 	downStepCount_A531; 
		UINT8 	shutterLength_A536; 
		UINT8 	shutterType_A537; 

		UINT16  OVLim_A538; 
		UINT16  OILim_A539; 
		UINT16  OSLim_A540; 
		UINT16  OFLim_A541; 

		UINT8 	thermalProtect_A542; 
		UINT16	torqueConst_A543; 
		UINT16	backEMFConst_A544; 
		UINT16	speedConst_A545; 

		UINT16	ratedSpeed_A546; 
		UINT32	ratedCurrent_A547; 
		UINT8	polePairs_A548; 
		UINT8	jogSpeed_A551; 
		UINT16 	blockCRC; 

	}stEEPDriveMotorCtrlBlock; 
	
}_EEPDriveMotorCtrlBlock;

EXTERN _EEPDriveMotorCtrlBlock uEEPDriveMotorCtrlBlock; 

//*******************************************************************************************

 

//*************************************************************************************
// This block defines the data structure in RAM that will hold the EEPROM data block that contains 
// data that will be used by both the Application and the Ramp Generator 

typedef union // Overall union for containing Common EEPROM block in RAM 
{
	UINT8 val[COMMON_BLOCK_DAT_LENGTH]; 
	struct 
	{
		INT16 	upperStoppingPos_A100; 
		INT16 	lowerStoppingPos_A101; 
		INT16 	photoElecPosMonitor_A102; 
		INT16 	originSensorPosMonitor_A128; 
		INT16 	originSensorDrift_A637;  // signed value
		INT16 	currentValueMonitor_A129; // signed value 
		INT16 	apertureHeightPos_A130; 
		UINT8 	apertureModeEnable_A131; 
		
		UINT16 	blockCRC; 

	}stEEPDriveCommonBlock; 
	
}_EEPDriveCommonBlock;

EXTERN _EEPDriveCommonBlock uDriveCommonBlockEEP; 


//*************************************************************************************
// This block defines the data structure in RAM that will hold the EEPROM data block that contains 
// Application data 
typedef union // Overall union for containing Application EEPROM block in RAM 
{
	UINT8 val[APPL_BLOCK_DAT_LENGTH]; 
	struct 
	{
		UINT8 	snowModePhotoelec_A008; 
		UINT8 	initialValSetting_A021; 
		UINT16 	maintenanceCountLimit_A025; 
		UINT16 	maintenanceCountValue_A636; 	
		UINT8 	microSensorCounter_A080; 
		UINT8 	microSensorCountReset_A081; 
		UINT8 	overrunProtection_A112; 
		UINT8 	resetToDefaultValues_A120; 
		UINT8 	powerUpCalib_A125; 
		//	Data length of correctedFreqAperture_A126 changed to WORD from BYTE as parameter range is 10 to 9999 - Jan 2016
		UINT16 	correctedFreqAperture_A126; 
		UINT8 	autoCorrectionEnabled_A127; 
		UINT32 	driveFWVersion_A549; 
		UINT32 	driveHWVersion_A550; 
		UINT32 	operationCount_A600; 
		UINT8 	microSensorLimValue_A603;
		UINT32 	apertureHeightOperCount_A604; 
		UINT8	operationCounterReset_A020; 
		UINT32	operationCount_A028; 
		
		UINT16 blockCRC; 

	}stEEPDriveApplBlock; 
	
}_EEPDriveApplBlock;

EXTERN _EEPDriveApplBlock uDriveApplBlockEEP; 


//*************************************************************************************
// This block defines the data structure in RAM that will hold the EEPROM data block that contains 
// Drive Status and Fault Registers + Anomaly History 

typedef union // Drive status register 
{
       UINT32 val;
       struct stDriveStatus
       {
			// drive system states 
			UINT32 driveReady				: 1; // Running/ ON actually 
			UINT32 drivePowerOnCalibration	: 1; // this is doing same as runtime calibration, this flag can be removed 
			UINT32 driveRuntimeCalibration	: 1;
			UINT32 driveInstallation		: 1; 

			// drive fault status - can be Fault + Fault unrecoverable at same time 
			UINT32 driveFault				: 1;
			UINT32 driveFaultUnrecoverable	: 1;

			// drive position status 
			UINT32 shutterUpperLimit		: 1;
			UINT32 shutterApertureHeight	: 1;
			UINT32 shutterLowerLimit		: 1;
			UINT32 upDecelStartReached		: 1; 	// Upper Deceleration Start point reached 
			UINT32 ignPhotoElecSensLimRchd	: 1;	// Ignore Photo Electric Obstacle Sensor Limit reached

			// drive movement status 
			UINT32 shutterStopped			: 1;
			UINT32 shutterMovingUp			: 1;
			UINT32 shutterMovingDown		: 1;
			UINT32 shuttrMovDwnIgnoreSens	: 1; //ignore all sensors when this bit is set,
                                                 //this bit is set by control board and process required command
                                                 //open/close/stop is valid
            UINT32 originSensorStatus		: 1;
			UINT32 microSwitchSensorStatus	: 1;
			UINT32 peSensorStatus		    : 1;            

            UINT32 driveBoardBootloader		: 1;
            UINT32 shutterBetweenUplmtAphgt	: 1;	//STT  shutter position between upperlimit and ApertureHeight     //bug_NO.12?13?14?15
            UINT32 shutterBetweenLowlmtAphgt	: 1;	//STT  shutter position between lowerlimit and ApertureHeight  //bug_NO.12?13?14?15
            UINT32 unused						: 11;

	   } bits;

} _DriveStatus_A605;

typedef union  // Drive installation status register 
{
       UINT8 val;
       struct stDriveInstallation
       {
			  UINT8 installA100              : 1;
              UINT8 installA101              : 1;
              UINT8 installA102              : 1;
              UINT8 installationValidation   : 1; // this is doing same as runtime calibration, this flag can be removed 
              UINT8 installationSuccess      : 1; // success can be flagged on completion of A102, then runtime calibration 
              									  // can be taken up separately 
              UINT8 installationFailed       : 1;
              UINT8 unused                   : 2;
       } bits;
} _DriveInstallationStatus_A606;



typedef union  // Drive fault register 
{
       UINT8 val;
       struct stDriveFault
       {
              UINT8 driveCommunicationFault   : 1;
              UINT8 driveMotorFault           : 1;
              UINT8 driveApplicationFault     : 1;
              UINT8 driveProcessorFault       : 1;
              UINT8 unused                    : 1;
       } bits;

} _DriveFault_A607;

typedef union  // Drive communication fault register 
{
       UINT8 val;
       struct stDriveCommunicationFault
       {
              UINT8 crcError                  : 1;
              UINT8 uartError                 : 1;
			  UINT8 commandFrameError		  : 1; 
              UINT8 unused                    : 5;
       } bits;
} _DriveCommunicationFault_A608;

typedef union  // Drive motor fault register 
{
       UINT16 val;
       struct stDriveMotorFault
       {
              UINT16 motorOpenphase           : 1;
              UINT16 motorDCbusOverVoltage    : 1;
              UINT16 motorOverCurrent         : 1;
              UINT16 motorExceedingTorque     : 1; //set this flag when current injection time exceeded
              UINT16 motorPFCshutdown         : 1; 
              UINT16 motorStall               : 1;
              UINT16 motorOverheat            : 1;
              UINT16 motorSusOC               : 1;              
              UINT16 motorPWMCosting          : 1;
              UINT16 motorAnyOther            : 1;
              UINT16 unused                   : 6;
       } bits;
} _DriveMotorFault_A609;

typedef union  // Drive application fault register 
{
       UINT32 val;
       struct stDriveApplicationFault
       {
              UINT32 peObstacle               : 1;
              UINT32 lowInputVoltage          : 1; //need to check on hardware remove if not present, below 90V
              UINT32 highInputVoltage         : 1; //upper limit 260V
              UINT32 hallSensor               : 1;
              UINT32 wraparound               : 1; // these 3 bits can be clubbed into 1 as any variant will have only one of these sensors 
              UINT32 microSwitch              : 1; // these 3 bits can be clubbed into 1 as any variant will have only one of these sensors //use only this
              UINT32 airSwitch                : 1; // these 3 bits can be clubbed into 1 as any variant will have only one of these sensors
              UINT32 emergencyStop            : 1;
              UINT32 osNotDetectUP            : 1;  // origin sensor not detected while rolling up
              UINT32 osNotDetectDown          : 1;  // origin sensor not detected while rolling down
              UINT32 osDetectOnUp             : 1;  // origin Sensor becomes ON while shutter is between the upper limit and origin sensor level. (expected is OFF)
              UINT32 osDetectOnDown           : 1;  // origin Sensor becomes OFF while shutter is between origin sensor level and lower limit (expected is ON)
              UINT32 osFailValidation         : 1;  // In case of teaching mode, error from limit calculation is more than or equal to 500mm
              UINT32 driveCalibrationFailed   : 1;
              UINT32 microSwitchSensorLimit   : 1;   //     A080
              UINT32 maintenanceCountOverflow : 1;   //     A025 //no action required
			  UINT32 igbtOverTemperature 	  : 1;   //     IGBT overtemperature
			  UINT32 powerFail			 	  : 1;   //     power fail condition
			  
			  // Added for displaying errors on display screen in case of false movement - RN - NOV 2015
			  UINT32 shutterFalseUpMovementCount 	  : 1;   //     False movement in up direction
			  UINT32 shutterFalseDownMovementCount 	  : 1;   //     False movement in down direction
			  //	Motor cable fault is declared when motor cable is disconnected - Dec 2015
			  UINT32 motorCableFault          : 1;
              UINT32 unused                   : 11;
       } bits;
} _DriveApplicationFault_A610;

typedef union  // Drive processor fault register 
{
       UINT16 val;
       struct stDriveProcessorfault
       {
              UINT16 flashImageCRC           : 1;
              UINT16 eepromParameterDbCRC    : 1;
              UINT16 watchdogTrip            : 1; 
              UINT16 eepromProgramming       : 1;
              UINT16 eepromErase             : 1;
              UINT16 unsued                  : 11;
       } bits;
} _DriveProcessorfault_A611;

//this structure can be removed
typedef union  // Drive anomaly history registers 
{
       UINT8 val[10];
       struct stDriveAnomalyHistory
       {
              UINT8 drive_anomaly_history_1;  // indicates oldest error           
              UINT8 drive_anomaly_history_2;
			  UINT8 drive_anomaly_history_3;
			  UINT8 drive_anomaly_history_4;
			  UINT8 drive_anomaly_history_5;
			  UINT8 drive_anomaly_history_6;
			  UINT8 drive_anomaly_history_7;
			  UINT8 drive_anomaly_history_8;
			  UINT8 drive_anomaly_history_9;
			  UINT8 drive_anomaly_history_10; // indicates latest error 
       } bytes;
} _DriveAnomalyHistory_A616;

typedef union // Overall union for containing Drive Status_Fault EEPROM block in RAM 
{
	UINT8 val[DRIVE_APPL_STAT_FAULT_BLOCK_DAT_LENGTH]; 
	struct 
	{
		// Drive Status 
		_DriveStatus_A605 uDriveStatus; 
		_DriveInstallationStatus_A606 uDriveInstallationStatus; 
		
		// Drive Faults 
		_DriveFault_A607 uDriveFault; 
		_DriveCommunicationFault_A608 uDriveCommunicationFault; 
		_DriveMotorFault_A609 uDriveMotorFault; 
		_DriveApplicationFault_A610 uDriveApplicationFault; 
		_DriveProcessorfault_A611 uDriveProcessorFault; 
		_DriveAnomalyHistory_A616 uDriveAnamolyHistory; 

		UINT16 crcCheckFailedCountA803;
		UINT16 commBufferOverflowCountA807; 

		UINT16 blockCRC; 

	}stEEPDriveStatFaultBlock; 
	
}_EEPDriveStatFaultBlock;

EXTERN _EEPDriveStatFaultBlock uDriveStatusFaultBlockEEP; 

//*************************************************************************************



//*************************************************************************************
// Following are the function declarations for EEPROM file 

UINT8 readBYTE(UINT16 address); 		// Read single byte from EEPROM
UINT16 readWORD(UINT16 address); 		// Read single word from EEPROM
UINT32 readDWORD(UINT16 address); 		// Read double word from EEPROM

VOID writeBYTE(UINT16 address, UINT8 byte);		// Write single byte to EEPROM
VOID writeWORD(UINT16 address, UINT16 word);	// Write single word to EEPROM
VOID writeDWORD(UINT16 address, UINT32 dword); 	// Write double word to EEPROM

VOID readBlock(UINT16 address, UINT8* pBuffer, UINT16 dataLength); 		// Read block of data from EEPROM into RAM struct 
VOID writeBlock(UINT16 address, UINT8* pBuffer, UINT16 dataLength); 		// Write block of data from RAM struct to EEPROM
VOID wrBlUsingWriteByte(UINT16 address, UINT8* pBuffer, UINT16 dataLength); //Write block using write byte command
VOID eraseBlock(UINT16 address, UINT16 dataLength);
    
BOOL initMotorControlBlock(VOID); 		// Read the block EEPROM values and initialize RAM structure 
BOOL initApplBlock(VOID); 					// Read the block EEPROM values and initialize RAM structure 
BOOL initDriveStatusFaultBlock(VOID); 	// Read the block EEPROM values and initialize RAM structure 
BOOL initCommonBlock(VOID); 				// Read the block EEPROM values and initialize RAM structure 

//UINT8 fetchByteFrmFlash(UINT8 index);  
VOID getBlockStartAndLength(UINT8 startIndex, UINT8 endIndex, UINT8 *blockLength, UINT8 *blockArrayIndex, UINT16 *blockPhysicalAddr);
UINT8 getParamStartAndLength(UINT8 paramIndex, UINT8* byteCount); 

VOID resetShutterPositions(VOID); 
VOID resetAllParameters(VOID); 

BOOL getParameter(UINT16 paramIndex, UINT32* parameter, UINT8* byteCount);  // Read and return the value of the drive board parameter 
BOOL setParameter(UINT16 parameterIndex, UINT32 data); // Set the value of the drive board parameter specified 
VOID checkResetParameter(VOID);

VOID testEEPRomData(VOID);

VOID updateCommonBlockCrc(VOID);
VOID updateApplBlockCrc(VOID);
VOID updateMotorBlockCrc(VOID);

//*************************************************************************************

#endif /* EEPROM_H */

