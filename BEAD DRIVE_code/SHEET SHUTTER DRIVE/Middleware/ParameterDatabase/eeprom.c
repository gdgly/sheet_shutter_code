/*********************************************************************************
* FileName: EEPROM.c
* Description:
* This source file contains the definition of all the functions for handling EEPROM
**********************************************************************************/

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
#include <p33Exxxx.h>
#include <stdlib.h>
#include <string.h>
#include "eeprom.h"
#include "defaultParamValues.h"
#include "./Common/UserDefinition/Userdef.h"
#include "./Application/CommandHandler.h"
#include "./Middleware/ParameterDatabase/spi.h"
#include "./Middleware/ParameterDatabase/spieeprom.h"
#include "./Application/Application.h"
#include "./Common/Extern/Extern.h"


#define USE_EEPROM_PARAM_MAP TRUE
#define EEPROM_WRITE_RETRY_CNT  3

UINT32 SysCntCopy = 0;   //bug_NO.59

/* Indexes of Drive EEPROM parameters */
enum _DriveEEPROMData
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
	A604_APERTURE_HEIGHT_OPER_COUNT,
	A020_OPERATION_COUNT_RESET,
	A028_OPERATION_COUNTER, // appl block end 
    
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
	A625_ANOMALY_HISTORY_10, 
	A803_CRC_FAIL_COUNT, 
	A807_BUF_OVRFLW_COUNT, // appl status block end 
    
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
	
	A506_TIMER2_PRE_SCALAR, 
	A507_TIMER2_TO_RPM, 
	A508_TIMER2_MIN, 
	A509_TIMER2_MAX, 
	A510_MIN_DUTY_CYCLE, 
	A511_BREAK_ENABLED, 
	A512_SPEED_PI_PROP_GAIN, 
	A513_SPEED_PI_INTEGRAL_GAIN,
	A514_CURRENT_PI_PROP_GAIN,
	A515_CURRENT_PI_INTEGRAL_GAIN,
	A516_OUTPUT_FREQ, 
	A517_INCH_SPEED, 
	A518_DRIVE_STATUS, 
	A519_CURRENT_ERROR, 
	A520_ACCELERATION1_UP, 
	A521_DECELERATION1_UP, 
	A522_S1_UP, 
	A523_S2_UP, 
	A524_S3_UP, 
	A525_UP_STEP_COUNT, 
	A526_ACCELERATION1_DOWN, 
	A527_DECELERATION1_DOWN, 
	A528_S1_DOWN, 
	A529_S2_DOWN, 
	A530_S3_DOWN, 
	A531_DOWN_STEP_COUNT, 
	A536_SHUTTER_LENGTH, 
	A537_SHUTTER_TYPE, 
	A538_OV_LIMIT, 
	A539_OI_LIMIT, 
	A540_OS_LIMIT, 
	A541_OF_LIMIT, 
	A542_THERMAL_PROTECTION, 
	A543_TORQUE_CONST,
	A544_BACKEMF_CONST, 
	A545_SPEED_CONST, 
	A546_RATED_SPEED, 
	A547_RATED_CURRENT, 
	A548_POLE_PAIRS, 
	A551_JOG_SPEED, // motor block end 
    
	NUMBER_OF_DRIVE_PARAMETERS
		
}enDriveEEPROMData; 


#ifdef USE_EEPROM_PARAM_MAP 
const UINT16 parameterMap[NUMBER_OF_DRIVE_PARAMETERS] 
= {
	100, //A100_UPPER_STOPPING_POS = 0, 
	101, //A101_LOWER_STOPPING_POS, 
	102, //A102_PHOTO_ELEC_POS, 
	128, //A128_ORIGIN_SENS_POS, 
	637, //A637_ORIGIN_SENS_DRIFT, 
	129, //A129_CURRENT_VAL_MONITOR, 
	130, //A130_APERTURE_HEIGHT_POS,
	131, //A131_APERTURE_MODE_ENABLE, // common block end 
	
	8, 	 //A008_SNOW_MODE_PHOTOELEC, 
	21,  //A021_INITIAL_VAL_SETTING, 
	25,  //A025_MAINTENANCE_COUNT_LIMIT,
	636, //A636_MAINTENANCE_COUNT_VALUE, 
	80,  //A080_MICRO_SENS_COUNT, 
	81,  //A081_MICRO_SENS_COUNT_RESET, 
	112, //A112_OVERRUN_PROTECT, 
	120, //A120_RESET_TO_DEFAULT, 
	125, //A125_POWERUP_CALIB, 
	126, //A126_CORRECTED_FREQ_APERTURE,
	127, //A127_AUTO_CORRECT_ENABLED, 
	549, //A549_DRIVE_FW_VER,
	550, //A550_DRIVE_HW_VER, 
	600, //A600_OPERATION_COUNT, 
	603, //A603_MICRO_SENS_LIM_VALUE, 
	604, //A604_APERTURE_HEIGHT_OPER_COUNT, 
	20,  //A020_OPERATION_COUNT_RESET,
	28,  //A028_OPERATION_COUNTER,  	// appl block end 
		
	605, //A605_DRIVE_BOARD_STATUS, 
	606, //A606_DRIVE_INSTALLATION, 
	607, //A607_DRIVE_FAULT, 
	608, //A608_DRIVE_BOARD_COMM_FAULT, 
	609, //A609_DRIVE_MOTOR_FAULT, 
	610, //A610_DRIVE_APPL_FAULT, 
	611, //A611_DRIVE_PROCESSOR_FAULT, 

	616, //A616_ANOMALY_HISTORY_1,
	617, //A617_ANOMALY_HISTORY_2,
	618, //A618_ANOMALY_HISTORY_3,
	619, //A619_ANOMALY_HISTORY_4,
	620, //A620_ANOMALY_HISTORY_5,
	621, //A621_ANOMALY_HISTORY_6,
	622, //A622_ANOMALY_HISTORY_7,
	623, //A623_ANOMALY_HISTORY_8,
	624, //A624_ANOMALY_HISTORY_9,
	625, //A625_ANOMALY_HISTORY_10, 
	803, //A803_CRC_FAIL_COUNT, 
	807, //A807_BUF_OVRFLW_COUNT,	// appl status block end 

	11, //A011_DECEL_BY_PHOTO_ELEC_BLOCKING_LIM, 
	11, //A011_WAIT_FOR_STOPPAGE, 

	103, //A103_RISE_CHANGE_GEAR_POS1, 
	104, //A104_RISE_CHANGE_GEAR_POS2,
	105, //A105_RISE_CHANGE_GEAR_POS3,

	106, //A106_FALL_CHANGE_GEAR_POS1, 
	107, //A107_FALL_CHANGE_GEAR_POS2, 
	108, //A108_FALL_CHANGE_GEAR_POS3, 

	110, //A110_SHUTTER_REVERSE_OP_MIN_LIM, 
	500, //A500_PWM_FREQ_MOTOR_CTRL, 
	501, //A501_STARTUP_DUTY_CYCLE, 
	504, //A504_MAX_STARTUP_TIME_LIM, 
	505, //A505_STARTUP_SECTOR_CONST, 
	
	506, //A506_TIMER2_PRE_SCALAR, 
	507, //A507_TIMER2_TO_RPM, 
	508, //A508_TIMER2_MIN, 
	509, //A509_TIMER2_MAX, 
	510, //A510_MIN_DUTY_CYCLE, 
	511, //A511_BREAK_ENABLED, 
	512, //A512_SPEED_PI_PROP_GAIN, 
	513, //A513_SPEED_PI_INTEGRAL_GAIN,
	514, //A514_CURRENT_PI_PROP_GAIN,
	515, //A515_CURRENT_PI_INTEGRAL_GAIN,
	516, //A516_OUTPUT_FREQ, 
	517, //A517_INCH_SPEED, 
	518, //A518_DRIVE_STATUS, 
	519, //A519_CURRENT_ERROR, 
	520, //A520_ACCELERATION1_UP, 
	521, //A521_DECELERATION1_UP, 
	522, //A522_S1_UP, 
	523, //A523_S2_UP, 
	524, //A524_S3_UP, 
	525, //A525_UP_STEP_COUNT, 
	526, //A526_ACCELERATION1_DOWN, 
	527, //A527_DECELERATION1_DOWN, 
	528, //A528_S1_DOWN, 
	529, //A529_S2_DOWN, 
	530, //A530_S3_DOWN, 
	531, //A531_DOWN_STEP_COUNT, 
	536, //A536_SHUTTER_LENGTH, 
	537, //A537_SHUTTER_TYPE, 
	538, //A538_OV_LIMIT, 
	539, //A539_OI_LIMIT, 
	540, //A540_OS_LIMIT, 
	541, //A541_OF_LIMIT, 
	542, //A542_THERMAL_PROTECTION, 
	543, //A543_TORQUE_CONST,
	544, //A544_BACKEMF_CONST, 
	545, //A545_SPEED_CONST, 
	546, //A546_RATED_SPEED, 
	547, //A547_RATED_CURRENT, 
	548, //A548_POLE_PAIRS,
	551, //A551_JOG_SPEED, 
	
}; 

#endif 


UINT8 driveParamDatLengths[NUMBER_OF_DRIVE_PARAMETERS]
= {
	UPPER_STOPPING_POS_DAT_LENGTH, 
	LOWER_STOPPING_POS_DAT_LENGTH, 
	PHOTOELEC_POS_DAT_LENGTH, 
	ORIGIN_SENS_POS_DAT_LENGTH, 
	ORIGIN_SENSOR_DRIFT_DAT_LENGTH, 
	CURRENT_VAL_MONITOR_DAT_LENGTH, 
	APERTURE_HEIGHT_POS_DAT_LENGTH, 
	APERTURE_MODE_ENABLED_DAT_LENGTH, 		// common block end 
	 
	SNOW_MODE_DAT_LENGTH, 
	INIT_VAL_SETTING_DAT_LENGTH, 
	MAINTENANCE_COUNT_LIMIT_DAT_LENGTH, 
	MAINTENANCE_COUNT_VALUE_DAT_LENGTH, 
	MICRO_SENSOR_COUNT_DAT_LENGTH,
	MICRO_SENS_COUNT_RESET_DAT_LENGTH,
	OVERRUN_PROTECT_DAT_LENGTH, 
	RESET_TO_DEFAULT_DAT_LENGTH, 
	POWER_UP_CALIB_DAT_LENGTH,
	CORRECTED_FREQ_APERTURE_DAT_LENGTH, 
	CORRECTION_ENABLED_DAT_LENGTH, 
	DRIVE_FW_VER_DAT_LENGTH, 
	DRIVE_HW_VER_DAT_LENGTH, 
	OPERATION_COUNT_DAT_LENGTH,
	MICRO_SENSOR_LIMIT_VAL_DAT_LENGTH, 
	APERTURE_HEIGHT_OP_COUNT_DAT_LENGTH,
	OPERATION_COUNT_RESET_DAT_LENGTH, 	
	OPERATION_COUNTER_DAT_LENGTH, 		// appl block end 

	DRIVE_STATUS_DAT_LENGTH, 
	DRIVE_INTALL_STATUS_DAT_LENGTH, 
	DRIVE_FAULT_DAT_LENGTH, 
	DRIVE_COMM_FAULT_DAT_LENGTH, 
	DRIVE_MOTOR_FAULT_DAT_LENGTH, 
	DRIVE_APPL_FAULT_DAT_LENGTH, 
	DRIVE_PROCESR_FAULT_DAT_LENGTH, 

	DRIVE_ANOMALY_HIST_DAT_LENGTH,		// 10 anomaly history entries 
	DRIVE_ANOMALY_HIST_DAT_LENGTH,
	DRIVE_ANOMALY_HIST_DAT_LENGTH,
	DRIVE_ANOMALY_HIST_DAT_LENGTH,
	DRIVE_ANOMALY_HIST_DAT_LENGTH,
	DRIVE_ANOMALY_HIST_DAT_LENGTH,
	DRIVE_ANOMALY_HIST_DAT_LENGTH,
	DRIVE_ANOMALY_HIST_DAT_LENGTH,
	DRIVE_ANOMALY_HIST_DAT_LENGTH,
	DRIVE_ANOMALY_HIST_DAT_LENGTH, 
	DRIVE_CRC_FAILURE_COUNT_DAT_LENGTH,
	DRIVE_COMM_BUF_OVRFLW_CNT_DAT_LENTH, // appl status block end 

	DECEL_BY_PHOTO_BLCK_LIM_DAT_LEN,
	WAIT_FOR_STOPPAGE_DAT_LENGTH,	
	RISE_CHANGE_GEAR_POS1_DAT_LENGTH, 
	RISE_CHANGE_GEAR_POS2_DAT_LENGTH,
	RISE_CHANGE_GEAR_POS3_DAT_LENGTH,
	FALL_CHANGE_GEAR_POS1_DAT_LENGTH,
	FALL_CHANGE_GEAR_POS2_DAT_LENGTH,
	FALL_CHANGE_GEAR_POS3_DAT_LENGTH,
	SHTER_REV_OP_MIN_LIM_DAT_LENGTH	,
	PWM_FREQ_MOTOR_CTRL_DAT_LENGTH,
	STARTUP_DUTY_CYCLE_DAT_LENGTH,
	MAX_STARTUP_TIME_LIM_DAT_LENGTH,
	STARTUP_SECTOR_CONST_DAT_LENGTH,

	TIMER2_PRE_SCALAR_DAT_LENGTH,
	TIMER2_TO_RPM_DAT_LENGTH,
	TIMER2_MIN_DAT_LENGTH,
	TIMER2_MAX_DAT_LENGTH,
	MIN_DUTY_CYCLE_DAT_LENGTH,
	BREAK_ENABLED_DAT_LENGTH,
	SPEED_PI_KP_DAT_LENGTH,
	SPEED_PI_KI_DAT_LENGTH,
	CURRENT_PI_KP_DAT_LENGTH,
	CURRENT_PI_KI_DAT_LENGTH,
	OUTPUT_FREQ_DAT_LENGTH,

	INCH_SPEED_DAT_LENGTH,
	DRIVE_STAT_DAT_LENGTH, 
	CURRENT_ERROR_DAT_LENGTH,
	ACCEL1_UP_DAT_LENGTH, 
	DECEL1_UP_DAT_LENGTH, 
	S1_UP_DAT_LENGTH,
	S2_UP_DAT_LENGTH,
	S3_UP_DAT_LENGTH,			

	UP_STEP_COUNT_DAT_LENGTH,
	ACCEL1_DOWN_DAT_LENGTH,
	DECEL1_DOWN_DAT_LENGTH,
	S1_DOWN_DAT_LENGTH, 
	S2_DOWN_DAT_LENGTH,
	S3_DOWN_DAT_LENGTH,
	DOWN_STEP_COUNT_DAT_LENGTH,
	SHUTTER_LEN_DAT_LENGTH,
	SHUTTER_TYPE_DAT_LENGTH,
	OV_LIM_DAT_LENGTH,
	OI_LIM_DAT_LENGTH,
	OS_LIM_DAT_LENGTH,
	OF_LIM_DAT_LENGTH,
	THERMAL_PROTECT_DAT_LENGTH,
	TORQUE_CONST_DAT_LENGTH,
	BACK_EMF_CONST_DAT_LENGTH,
	SPEED_CONST_DAT_LENGTH,
	RATED_SPEED_DAT_LENGTH,
	RATED_CURRENT_DAT_LENGTH,
	POLE_PAIRS_DAT_LENGTH,
	JOG_SPEED_DAT_LENGTH // Motor control block end 
	
}; 

_EEPDriveCommonBlock uDriveCommonBlockEEP; 
_EEPDriveStatFaultBlock uDriveStatusFaultBlockEEP; 
_EEPDriveApplBlock uDriveApplBlockEEP; 
_EEPDriveMotorCtrlBlock uEEPDriveMotorCtrlBlock; 

#define INVALID_PARAMETER 0xFFFF 
UINT16 setParamIndex = INVALID_PARAMETER;
/******************************************************************************
 * readBYTE
 *
 * This function reads and returns one byte data read from EEPROM at the address specified
 *
 * PARAMETER REQ: address - from where byte needs to be read 
 *
 * RETURNS: data byte read from EEPROM at the specified address 
 *
 * ERRNO: none 
 ********************************************************************************/
UINT8 readBYTE(UINT16 address)
{
    UINT8 byte;    
    byte = EEPROMReadByte(address);  
    
    return byte;
}

/******************************************************************************
 * readWORD
 *
 * This function reads and returns one word data read from EEPROM at the address specified
 *
 * PARAMETER REQ: address - from where word needs to be read 
 *
 * RETURNS: data word read from EEPROM at the specified address 
 *
 * ERRNO: none 
 ********************************************************************************/
UINT16 readWORD(UINT16 address)
{
    UINT16 word;    
    word = EEPROMReadWord(address);
    
    return word;
}

/******************************************************************************
 * readDWORD
 *
 * This function reads and returns one double word data read from EEPROM at the address specified
 *
 * PARAMETER REQ: address - from where double word needs to be read 
 *
 * RETURNS: data double word read from EEPROM at the specified address 
 *
 * ERRNO: none 
 ********************************************************************************/
UINT32 readDWORD(UINT16 address)
{
	UINT32 dword;    
    dword = EEPROMReadDword(address);
    
    return dword;
}

/******************************************************************************
 * writeBYTE
 *
 * This function writes one byte of data to the EEPROM address specified
 *
 * PARAMETER REQ: 	address 	- to where the byte needs to be written, 
 * 					byte 	- data to be written to EEPROM 
 *
 * RETURNS: TRUE if no error while writing 
 *
 * ERRNO: Application fault will need to be triggered 
 ********************************************************************************/
VOID writeBYTE(UINT16 address, UINT8 byte)
{	
    BOOL writeSucess;
    UINT8 readByte;
    UINT8 retryCnt = 0;
    
    do
    {
        writeSucess = EEPROMWriteByte(byte, address);
        readByte = readBYTE(address);
    }while((readByte != byte) && (++retryCnt < EEPROM_WRITE_RETRY_CNT));
        
    if(retryCnt >= EEPROM_WRITE_RETRY_CNT)
    {
        writeSucess = FALSE;
    }
    
    //if write is not sucessful then set eeprom write error flag
    if(!writeSucess)
    {
        //uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.bits.eepromProgramming = TRUE;
        //PORTAbits.RA7 = 0;
    }
}

/******************************************************************************
 * writeWORD
 *
 * This function writes one word of data to the EEPROM address specified
 *
 * PARAMETER REQ: 	address 	- to where the word needs to be written, 
 * 					word 	- data to be written to EEPROM 
 *
 * RETURNS: TRUE if no error while writing 
 *
 * ERRNO: Application fault will need to be triggered 
 ********************************************************************************/
//VOID writeWORD(UINT16 address, UINT16 word)
//{
//    BOOL writeSucess;
//    UINT16 readWord;
//    UINT8 retryCnt = 0;
//    
//    do
//    {
//        writeSucess = EEPROMWriteWord(word, address);
//        readWord = readWORD(address);
//    }while((readWord != word) && (++retryCnt < EEPROM_WRITE_RETRY_CNT));
//    
//    if(retryCnt >= EEPROM_WRITE_RETRY_CNT)
//    {
//        writeSucess = FALSE;
//    }
//        
//    //if write is not sucessful then set eeprom write error flag
//    if(!writeSucess)
//    {
//        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.bits.eepromProgramming = TRUE;
//        //PORTAbits.RA7 = 0;
//    }
//}


/******************************************************************************
 * writeDWORD
 *
 * This function writes one double word of data to the EEPROM address specified
 *
 * PARAMETER REQ: 	address 	- to where double word needs to be written, 
 * 					dword 	- data to be written to EEPROM 
 *
 * RETURNS: TRUE if no error while writing 
 *
 * ERRNO: Application fault will need to be triggered 
 ********************************************************************************/
//VOID writeDWORD(UINT16 address, UINT32 dword)
//{
//    BOOL writeSucess;
//    UINT32 readDword;
//    UINT8 retryCnt = 0;
//    
//    do
//    {
//        writeSucess = EEPROMWriteDword(dword, address);
//        readDword = readDWORD(address);
//    }while((readDword != dword) && (++retryCnt < EEPROM_WRITE_RETRY_CNT));
//    
//    if(retryCnt >= EEPROM_WRITE_RETRY_CNT)
//    {
//        writeSucess = FALSE;
//    }
//        
//    //if write is not sucessful then set eeprom write error flag
//    if(!writeSucess)
//    {
//        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.bits.eepromProgramming = TRUE;
//        PORTAbits.RA7 = 0;
//    }
//}

//VOID writeDWORD(UINT16 address, UINT32 dword)
//{
//    EEPROMWriteDword(dword, address);
//    ClrWdt();
//}

VOID writeWORD(UINT16 address, UINT16 word)
{
    UINT8* pBuffer = (UINT8*)&word;    
    BYTE byteCntr = 0;
    
    for(byteCntr = 0; byteCntr < 2; byteCntr++)
    {
        writeBYTE(address, *pBuffer);  
        address++;
        pBuffer++;
    }   
}

VOID writeDWORD(UINT16 address, UINT32 dword)
{
    UINT8* pBuffer = (UINT8*)&dword;    
    BYTE byteCntr = 0;

    for(byteCntr = 0; byteCntr < 4; byteCntr++)
    {
        writeBYTE(address, *pBuffer);  
        address++;
        pBuffer++;
    }   
}

/******************************************************************************
 * readApplParameters
 *
 * This function reads a block of data from EEPROM
 *
 * PARAMETER REQ: none
 *
 * RETURNS: TRUE if no error 
 *
 * ERRNO: Application fault will need to be triggered 
 ********************************************************************************/
VOID readBlock(UINT16 address, UINT8* pBuffer, UINT16 dataLength)
{
    EEPROMReadBlock(address , dataLength , pBuffer);
}


/******************************************************************************
 * writeApplParameters
 *
 * This function writes a block of data to EEPROM
 *
 * PARAMETER REQ: none
 *
 * RETURNS: TRUE if no error while writing 
 *
 * ERRNO: Application fault will need to be triggered 
 ********************************************************************************/
VOID writeBlock(UINT16 address, UINT8* pBuffer, UINT16 dataLength)
{
    BOOL writeSucess;
    
	writeSucess = EEPROMWriteBlock(address , dataLength , pBuffer);
    
    //if write is not sucessful then set eeprom write error flag
    if(!writeSucess)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.bits.eepromProgramming = TRUE;
        //PORTAbits.RA7 = 0;
    }
}

/******************************************************************************
 * writeApplParameters
 *
 * This function writes a block of data to EEPROM
 *
 * PARAMETER REQ: none
 *
 * RETURNS: TRUE if no error while writing 
 *
 * ERRNO: Application fault will need to be triggered 
 ********************************************************************************/
VOID eraseBlock(UINT16 address, UINT16 dataLength)
{
    BOOL writeSucess;
    
	writeSucess = EEPROMEraseBlock(address , dataLength);
    
    //if write is not sucessful then set eeprom write error flag
    if(!writeSucess)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveProcessorFault.bits.eepromProgramming = TRUE;
    }
    /*else
    {
        for(i=0;i<=20;i++)
        {
            PORTAbits.RA7 ^= 1;
            //PORTCbits.RC0 ^= 1;
            delayMs(100);
            ClrWdt();
            //ClrWdt();
        }
    }*/
}

VOID wrBlUsingWriteByte(UINT16 address, UINT8* pBuffer, UINT16 dataLength)
{
    BYTE byteCntr = 0;

    while(dataLength)
    {   
        //if block size is more than page read size then process the requrest in multiple pages
        if(dataLength > BLOCK_WRITE_SIZE)
        {   
            for(byteCntr = 0; byteCntr < BLOCK_WRITE_SIZE; byteCntr++)
            {               
                writeBYTE(address, *pBuffer);  
                address++;
                pBuffer++;
            }         
            dataLength -= BLOCK_WRITE_SIZE;     
        }
        //if block size is less than page read size then process the requrest in one pages
        else
        {                     
            for(byteCntr = 0; byteCntr < dataLength; byteCntr++)
            {
                writeBYTE(address, *pBuffer);  
                address++;
                pBuffer++;
            }         
            dataLength = 0;       
        }
    } 
}

/******************************************************************************
 * initMotorControlBlock
 *
 * This function initializes the structure in RAM that stores Motor control block parameters 
 *
 * PARAMETER REQ: none
 *
 * RETURNS: 
 *
 * ERRNO: 
 ********************************************************************************/
BOOL initMotorControlBlock(VOID)
{
    UINT16 calculatedCRC; 
    UINT16 storedCRC; 
    UINT8 retryCnt = 0;
	BOOL crcValid = TRUE; 
    
    do
    {
        readBlock(EEP_MOTOR_CTRL_BLOCK_START, uEEPDriveMotorCtrlBlock.val, DRIVE_MOTOR_BLOCK_DAT_LENGTH); 
        calculatedCRC = crc16(uEEPDriveMotorCtrlBlock.val, (DRIVE_MOTOR_BLOCK_DAT_LENGTH - CRC_LENGTH), CRC_SEED);
        storedCRC = uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.blockCRC;
    }while((storedCRC != calculatedCRC) && (++retryCnt < EEPROM_WRITE_RETRY_CNT));
    
    if(calculatedCRC != storedCRC)
    {        
#ifdef MOTOR_750W_BD
       uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock = uEEP_BeadDriveMotorCtrlBlockDefault.stEEPDriveMotorCtrlBlock;
#endif
#ifdef MOTOR_750W_M1
       uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock = uEEP_M1DriveMotorCtrlBlockDefault.stEEPDriveMotorCtrlBlock;
#endif        
        //writeBlock(EEP_MOTOR_CTRL_BLOCK_START, uEEPDriveMotorCtrlBlock.val, DRIVE_MOTOR_BLOCK_DAT_LENGTH);
        wrBlUsingWriteByte(EEP_MOTOR_CTRL_BLOCK_START, uEEPDriveMotorCtrlBlock.val, DRIVE_MOTOR_BLOCK_DAT_LENGTH);
        updateMotorBlockCrc();
        PORTAbits.RA7 = 0;
        crcValid = FALSE;
    }
    else
    {
        PORTCbits.RC0 = 0;
    }
    
	return crcValid;      
}

/******************************************************************************
 * initApplBlock
 *
 * This function initializes the structure in RAM that stores Application block parameters 
 *
 * PARAMETER REQ: none
 *
 * RETURNS: 
 *
 * ERRNO: 
 ********************************************************************************/

BOOL initApplBlock(VOID)
{
	UINT16 calculatedCRC; 
    UINT16 storedCRC; 
    UINT8 retryCnt = 0;    
	BOOL crcValid = TRUE; 

    do
    {
        readBlock(EEP_APPL_BLOCK_START, uDriveApplBlockEEP.val, APPL_BLOCK_DAT_LENGTH); 
        calculatedCRC = crc16(uDriveApplBlockEEP.val, (APPL_BLOCK_DAT_LENGTH - CRC_LENGTH), CRC_SEED);
        storedCRC = uDriveApplBlockEEP.stEEPDriveApplBlock.blockCRC;
    }while((storedCRC != calculatedCRC) && (++retryCnt < EEPROM_WRITE_RETRY_CNT));
    
    if(calculatedCRC != storedCRC)
    {
        uDriveApplBlockEEP.stEEPDriveApplBlock = uDriveApplBlockEEPDefault.stEEPDriveApplBlock;
        //writeBlock(EEP_APPL_BLOCK_START, uDriveApplBlockEEP.val, APPL_BLOCK_DAT_LENGTH);
        wrBlUsingWriteByte(EEP_APPL_BLOCK_START, uDriveApplBlockEEP.val, APPL_BLOCK_DAT_LENGTH);
        updateApplBlockCrc();
        PORTAbits.RA7 = 0;
        crcValid = FALSE;
    }
    else
    {
        PORTCbits.RC0 = 0;
    }
    
	return crcValid;     
}


VOID testEEPRomData(VOID)
{
    BYTE rdData = 0;
    BYTE wrData = 0xAA;
    
    writeBYTE(0, wrData);
    rdData = readBYTE(0);
    
    if(wrData == rdData)
    {
        PORTCbits.RC0 = 0;
    }
    else
    {
        PORTAbits.RA7 = 0;
    }
    
    //transmitACK(rdData);
    transmitDat(rdData);
    
    //transmitDat(0xAA);
}


/******************************************************************************
 * initCommonBlock
 *
 * This function initializes the structure in RAM that stores Common status block parameters 
 *
 * PARAMETER REQ: none
 *
 * RETURNS: 
 *
 * ERRNO: 
 ********************************************************************************/
BOOL initCommonBlock(VOID)
{
    UINT16 calculatedCRC; 
    UINT16 storedCRC; 
    UINT8 retryCnt = 0;    
	BOOL crcValid = TRUE; 
    
    do
    {
        readBlock(EEP_COMMON_BLOCK_START, uDriveCommonBlockEEP.val, COMMON_BLOCK_DAT_LENGTH); 
        calculatedCRC = crc16(uDriveCommonBlockEEP.val, (COMMON_BLOCK_DAT_LENGTH - CRC_LENGTH), CRC_SEED);
        storedCRC = uDriveCommonBlockEEP.stEEPDriveCommonBlock.blockCRC;
    }while((storedCRC != calculatedCRC) && (++retryCnt < EEPROM_WRITE_RETRY_CNT));
    
    if(calculatedCRC != storedCRC)
    {
        uDriveCommonBlockEEP.stEEPDriveCommonBlock = uDriveCommonBlockEEPDefault.stEEPDriveCommonBlock;
        //writeBlock(EEP_COMMON_BLOCK_START, uDriveCommonBlockEEP.val, COMMON_BLOCK_DAT_LENGTH);
        wrBlUsingWriteByte(EEP_COMMON_BLOCK_START, uDriveCommonBlockEEP.val, COMMON_BLOCK_DAT_LENGTH);
        updateCommonBlockCrc();
        PORTAbits.RA7 = 0;
        crcValid = FALSE;
    }
    else
    {
        PORTCbits.RC0 = 0;
    }
    
	return crcValid;     
}

/******************************************************************************
 * initDriveStatusFaultBlock
 *
 * This function initializes the structure in RAM that stores Drive status block parameters 
 *
 * PARAMETER REQ: none
 *
 * RETURNS: 
 *
 * ERRNO: 
 ********************************************************************************/
BOOL initDriveStatusFaultBlock(VOID)
{
	UINT8 pBuffer[EEP_APPL_STAT_FAULT_BLOCK_LENGTH]; 
	UINT16 calculatedCRC; 
	BOOL crcValid = FALSE;     
		
	//UINT16 index = 0; 
	//UINT8 blockStartLocation, blockStartArrayIndex, blockLength = 0; 
	
	// Validate EEPROM block with CRC -16 - if CRC error, then write the complete block to default values 
	readBlock(EEP_APPL_STAT_FAULT_BLOCK_START, pBuffer, EEP_APPL_STAT_FAULT_BLOCK_LENGTH); 

	calculatedCRC = crc16(pBuffer, (EEP_APPL_STAT_FAULT_BLOCK_LENGTH - CRC_LENGTH), CRC_SEED); 
	if((((UINT8)calculatedCRC) == pBuffer[EEP_APPL_STAT_FAULT_BLOCK_LENGTH - CRC_LENGTH])
		&& ((UINT8)(calculatedCRC >> 8) == pBuffer[EEP_APPL_STAT_FAULT_BLOCK_LENGTH - CRC_LENGTH + 1])
		) // valid CRC 
	{
		crcValid = TRUE; 
		memcpy((UINT8 *)&uDriveStatusFaultBlockEEP.val[0], (UINT8 *)&pBuffer[0], EEP_APPL_STAT_FAULT_BLOCK_LENGTH); // copy to RAM structure 
	}
	else // invalid hence initialize from flash 
	{
		//// Drive status and fault block parameters - index between A605_DRIVE_BOARD_STATUS to A625_ANOMALY_HISTORY_10 
		//getBlockStartAndLength(A605_DRIVE_BOARD_STATUS, A625_ANOMALY_HISTORY_10, &blockLength, &blockStartArrayIndex,&blockStartLocation);
		//for(index = 0; index <= blockLength; index++)	
		//{
		//	uDriveStatusFaultBlockEEP.val[index] = fetchByteFrmFlash(blockStartArrayIndex + index); // to determine where in flash data will be saved 
		//																			// assumed here that same address as in EEPROM - will change on board
		//}

        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock = uDriveStatusFaultBlockEEPDefault.stEEPDriveStatFaultBlock;
	}

	return crcValid; 

}

/******************************************************************************
 * getParameter
 *
 * Read and return the value of the drive board parameter
 *
 * PARAMETER REQ: 
 *
 * RETURNS: 
 *
 * ERRNO: 
 ********************************************************************************/
BOOL getParameter(UINT16 paramNumber, UINT32* parameter, UINT8* byteCount)  
{
	UINT8 byteLocation = 0; 
	UINT16  paramIndex = 0; 

#ifdef USE_EEPROM_PARAM_MAP
#define PARAMETER_NOT_FOUND 0xFFFF 

	UINT8 index = 0; 
	paramIndex = PARAMETER_NOT_FOUND;
	

	for(index = 0; index < NUMBER_OF_DRIVE_PARAMETERS; index++)
	{
		if(paramNumber == parameterMap[index])
		{
			paramIndex = index; 
			break; 
		}
	}

	if(PARAMETER_NOT_FOUND != paramIndex)
#else 
	paramIndex = paramNumber; 
#endif 
	{
		if(NUMBER_OF_DRIVE_PARAMETERS < paramIndex)
		{
			return FALSE; 
		}
		byteLocation = getParamStartAndLength(paramIndex, byteCount); 	
		
		if(paramIndex <= A131_APERTURE_MODE_ENABLE) // parameter to be read from common block 
		{
			if(sizeof(BYTE) == *byteCount)
			{
				*parameter = uDriveCommonBlockEEP.val[(byteLocation)]; 
			}
			else if(sizeof(WORD) == *byteCount)
			{
				*parameter = (UINT16)(uDriveCommonBlockEEP.val[byteLocation] 
										| ((UINT16)(uDriveCommonBlockEEP.val[byteLocation+1])  << 8)); 
			}
			else if(sizeof(DWORD) == *byteCount)
			{
				*parameter = (UINT32)(uDriveCommonBlockEEP.val[byteLocation] 
										| ((UINT32)(uDriveCommonBlockEEP.val[byteLocation+1]) << 8)
										| ((UINT32)(uDriveCommonBlockEEP.val[byteLocation+2]) << 16)
										| ((UINT32)(uDriveCommonBlockEEP.val[byteLocation+3]) << 24)); 
			}
			
		}
		else if((paramIndex > A131_APERTURE_MODE_ENABLE)
					&& (paramIndex <= A028_OPERATION_COUNTER)) // parameter to be read from appl block 
		{
			if(sizeof(BYTE) == *byteCount)
			{
				*parameter = uDriveApplBlockEEP.val[(byteLocation)]; 
			}
			else if(sizeof(WORD) == *byteCount)
			{
				*parameter = (UINT16)(uDriveApplBlockEEP.val[byteLocation] 
										| ((UINT16)(uDriveApplBlockEEP.val[byteLocation+1])  << 8)); 
			}
			else if(sizeof(DWORD) == *byteCount)
			{
				*parameter = (UINT32)(uDriveApplBlockEEP.val[byteLocation] 
										| ((UINT32)(uDriveApplBlockEEP.val[byteLocation+1]) << 8)
										| ((UINT32)(uDriveApplBlockEEP.val[byteLocation+2]) << 16)
										| ((UINT32)(uDriveApplBlockEEP.val[byteLocation+3]) << 24)); 
			}
		}
		else if((paramIndex > A028_OPERATION_COUNTER)
					&& (paramIndex <= A807_BUF_OVRFLW_COUNT)) // appl status block parameter 
		{
			if(sizeof(BYTE) == *byteCount)
			{
				*parameter = uDriveStatusFaultBlockEEP.val[(byteLocation)]; 
			}
			else if(sizeof(WORD) == *byteCount)
			{
				*parameter = (UINT16)(uDriveStatusFaultBlockEEP.val[byteLocation] 
										| ((UINT16)(uDriveStatusFaultBlockEEP.val[byteLocation+1]) << 8)); 
			}
			else if(sizeof(DWORD) == *byteCount)
			{
				*parameter = (UINT32)(uDriveStatusFaultBlockEEP.val[byteLocation] 
										| ((UINT32)(uDriveStatusFaultBlockEEP.val[byteLocation+1]) << 8)
										| ((UINT32)(uDriveStatusFaultBlockEEP.val[byteLocation+2]) << 16)
										| ((UINT32)(uDriveStatusFaultBlockEEP.val[byteLocation+3]) << 24));
			}
		}
		else if((paramIndex > A807_BUF_OVRFLW_COUNT)
			 		&& (paramIndex <= A551_JOG_SPEED)) // motor control block 
		{
			if(sizeof(BYTE) == *byteCount)
			{
				*parameter = uEEPDriveMotorCtrlBlock.val[(byteLocation)]; 
			}
			else if(sizeof(WORD) == *byteCount)
			{
				*parameter = (UINT16)(uEEPDriveMotorCtrlBlock.val[byteLocation] 
										| ((UINT16)(uEEPDriveMotorCtrlBlock.val[byteLocation+1])  << 8)); 
			}
			else if(sizeof(DWORD) == *byteCount)
			{
				*parameter = (UINT32)(uEEPDriveMotorCtrlBlock.val[byteLocation] 
										| ((UINT32)(uEEPDriveMotorCtrlBlock.val[byteLocation+1]) << 8)
										| ((UINT32)(uEEPDriveMotorCtrlBlock.val[byteLocation+2]) << 16)
										| ((UINT32)(uEEPDriveMotorCtrlBlock.val[byteLocation+3]) << 24));
			}
		}
		else // parameter not found in blocks 
		{
			return FALSE; 
		}
	
	}
	return TRUE; 
}


/******************************************************************************
 * setParameter
 *
 * Set the value of the drive board parameter specified 
 * Function will also handle following special cases for set parameter: 
 * 			A021 - Initial value setting,  A081 - micro sensor count(A080) reset, 
 * 			A120 - Initialization of sheet position params(A100, A101, A102, A130) to default
 *
 * PARAMETER REQ: 
 *
 * RETURNS: 
 *
 * ERRNO: 
 ********************************************************************************/
BOOL setParameter(UINT16 paramNumber, UINT32 data)
{
	// TBD - these parameters should immediately get written to memory as well 
	// if write fails  -- flag EEPROM error and return from here with false 

	UINT8 byteLocation, byteCount = 0; 
	UINT16  parameterIndex = 0; 

#ifdef USE_EEPROM_PARAM_MAP
#define PARAMETER_NOT_FOUND 0xFFFF 

	UINT8 index = 0; 
	parameterIndex = PARAMETER_NOT_FOUND;
	

	for(index = 0; index < NUMBER_OF_DRIVE_PARAMETERS; index++)
	{
		if(paramNumber == parameterMap[index])
		{
			parameterIndex = index; 
			break; 
		}
	}

	if(PARAMETER_NOT_FOUND != parameterIndex)
#else 
	parameterIndex = paramNumber; 
#endif 
	{
		if(NUMBER_OF_DRIVE_PARAMETERS < parameterIndex)
		{
			return FALSE; 
		}
		byteLocation = getParamStartAndLength(parameterIndex, &byteCount); 

		if(parameterIndex <= A131_APERTURE_MODE_ENABLE) // parameter from common block 
		{
			switch(byteCount)
			{
				case (sizeof(BYTE)): 
					{
						uDriveCommonBlockEEP.val[(byteLocation)] = (UINT8)data;
						writeBYTE((EEP_COMMON_BLOCK_START+byteLocation),(UINT8)data); 
                        updateCommonBlockCrc();
					}
					break; 
				case (sizeof(WORD)): 
					{
						uDriveCommonBlockEEP.val[byteLocation] = (UINT8)(data); // lsb;
						uDriveCommonBlockEEP.val[byteLocation+1] = (UINT8)(data >> 8); // msb	
						writeWORD((EEP_COMMON_BLOCK_START+byteLocation),(UINT16)data);  
                        updateCommonBlockCrc();
					}
					break;  
				case (sizeof(DWORD)): 
					{
						uDriveCommonBlockEEP.val[byteLocation] = (UINT8)(data); // lsb;
						uDriveCommonBlockEEP.val[byteLocation+1] = (UINT8)(data >> 8); 
						uDriveCommonBlockEEP.val[byteLocation+2] = (UINT8)(data >> 16);
						uDriveCommonBlockEEP.val[byteLocation+3] = (UINT8)(data >> 24);						
						writeDWORD((EEP_COMMON_BLOCK_START+byteLocation),(UINT32)data); 
                        updateCommonBlockCrc();
					}
					break; 
				default: 
					break; 
			}; 
			
		}
		else if((parameterIndex > A131_APERTURE_MODE_ENABLE)
					&& (parameterIndex <= A028_OPERATION_COUNTER)) // parameter to be read from appl block 
		{
			switch(byteCount)
			{
				case (sizeof(BYTE)): 
					{
						uDriveApplBlockEEP.val[(byteLocation)] = (UINT8)data; 
						writeBYTE((EEP_APPL_BLOCK_START+byteLocation),(UINT8)data); 
                        updateApplBlockCrc();
					}
					break; 
				case (sizeof(WORD)): 
					{
						uDriveApplBlockEEP.val[byteLocation] = (UINT8)(data); // lsb;
						uDriveApplBlockEEP.val[byteLocation+1] = (UINT8)(data >> 8); // msb						
						writeWORD((EEP_APPL_BLOCK_START+byteLocation),(UINT16)data);
                        updateApplBlockCrc();
					}
					break;  
				case (sizeof(DWORD)): 
					{
						uDriveApplBlockEEP.val[byteLocation] = (UINT8)(data); // lsb;
						uDriveApplBlockEEP.val[byteLocation+1] = (UINT8)(data >> 8); 
						uDriveApplBlockEEP.val[byteLocation+2] = (UINT8)(data >> 16);
						uDriveApplBlockEEP.val[byteLocation+3] = (UINT8)(data >> 24);						
						writeDWORD((EEP_APPL_BLOCK_START+byteLocation),(UINT32)data);
                        updateApplBlockCrc();
					}
					break; 
				default: 
					break; 
			}; 
			
		}
		else if((parameterIndex > A028_OPERATION_COUNTER)
					&& (parameterIndex <= A807_BUF_OVRFLW_COUNT)) // appl status block parameter 
		{
			switch(byteCount)
			{
				case (sizeof(BYTE)): 
					{
						uDriveStatusFaultBlockEEP.val[(byteLocation)] = (UINT8)data; 
						//writeBYTE((EEP_APPL_STAT_FAULT_BLOCK_START+byteLocation),(UINT8)data);
					}
					break; 
				case (sizeof(WORD)): 
					{
						uDriveStatusFaultBlockEEP.val[byteLocation] = (UINT8)(data); // lsb;
						uDriveStatusFaultBlockEEP.val[byteLocation+1] = (UINT8)(data >> 8); // msb
						//writeWORD((EEP_APPL_STAT_FAULT_BLOCK_START+byteLocation),(UINT16)data);
					}
					break;  
				case (sizeof(DWORD)): 
					{
						uDriveStatusFaultBlockEEP.val[byteLocation] = (UINT8)(data); // lsb;
						uDriveStatusFaultBlockEEP.val[byteLocation+1] = (UINT8)(data >> 8); 
						uDriveStatusFaultBlockEEP.val[byteLocation+2] = (UINT8)(data >> 16);
						uDriveStatusFaultBlockEEP.val[byteLocation+3] = (UINT8)(data >> 24);
						//writeDWORD((EEP_APPL_STAT_FAULT_BLOCK_START+byteLocation),(UINT32)data);
					}
					break; 
				default: 
					break; 
			}; 
		}
		else if((parameterIndex > A807_BUF_OVRFLW_COUNT)
			 		&& (parameterIndex <= A551_JOG_SPEED)) // motor control block 
		{
			switch(byteCount)
			{
				case (sizeof(BYTE)): 
					{
						uEEPDriveMotorCtrlBlock.val[(byteLocation)] = (UINT8)data; 
						writeBYTE((EEP_MOTOR_CTRL_BLOCK_START+byteLocation),(UINT8)data);
                        updateMotorBlockCrc();
					}
					break; 
				case (sizeof(WORD)): 
					{
						uEEPDriveMotorCtrlBlock.val[byteLocation] = (UINT8)(data); // lsb;
						uEEPDriveMotorCtrlBlock.val[byteLocation+1] = (UINT8)(data >> 8); // msb
						writeWORD((EEP_MOTOR_CTRL_BLOCK_START+byteLocation),(UINT16)data);
                        updateMotorBlockCrc();
					}
					break;  
				case (sizeof(DWORD)): 
					{
						uEEPDriveMotorCtrlBlock.val[byteLocation] = (UINT8)(data); // lsb;
						uEEPDriveMotorCtrlBlock.val[byteLocation+1] = (UINT8)(data >> 8); 
						uEEPDriveMotorCtrlBlock.val[byteLocation+2] = (UINT8)(data >> 16);
						uEEPDriveMotorCtrlBlock.val[byteLocation+3] = (UINT8)(data >> 24);
						writeDWORD((EEP_MOTOR_CTRL_BLOCK_START+byteLocation),(UINT32)data);
                        updateMotorBlockCrc();
					}
					break; 
				default: 
					break; 
			}; 
		}
		else // parameter not found in blocks 
		{
			return FALSE; 
		}
        setParamIndex = parameterIndex;
	}
	return TRUE; 
}

VOID checkResetParameter(VOID)
{
    if(uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorCountReset_A081)
    {
        //reset max microSwitchSensorLimit error flag
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.microSwitchSensorLimit = FALSE; 
        uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorCounter_A080 = uDriveApplBlockEEPDefault.stEEPDriveApplBlock.microSensorCounter_A080; 
        uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorCountReset_A081 = 0;
        writeBYTE(EEP_MICRO_SENS_COUNT,uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorCounter_A080);
        writeBYTE(EEP_MICRO_SENS_COUNT_RESET,uDriveApplBlockEEP.stEEPDriveApplBlock.microSensorCountReset_A081);
        updateApplBlockCrc();
    }
    else if(uDriveApplBlockEEP.stEEPDriveApplBlock.resetToDefaultValues_A120)
    {
        resetShutterPositions(); 
    }
    else if(uDriveApplBlockEEP.stEEPDriveApplBlock.initialValSetting_A021)
    {
        resetAllParameters(); 
    }
    else if(uDriveApplBlockEEP.stEEPDriveApplBlock.operationCounterReset_A020)
    {
        uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600 = 0; 
		//uDriveApplBlockEEP.stEEPDriveApplBlock.maintenanceCountValue_A636 = 0;
        uDriveApplBlockEEP.stEEPDriveApplBlock.operationCounterReset_A020 = 0;
        writeDWORD(EEP_OPERATION_COUNT, uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600); 
		//writeWORD(EEP_MAINTENANCE_COUNT_VALUE,uDriveApplBlockEEP.stEEPDriveApplBlock.maintenanceCountValue_A636);
        writeBYTE(EEP_OPERATION_COUNT_RESET,uDriveApplBlockEEP.stEEPDriveApplBlock.operationCounterReset_A020);
        updateApplBlockCrc();
    }
    else if(A028_OPERATION_COUNTER == setParamIndex)
    {
        uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600 = uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A028;
		//uDriveApplBlockEEP.stEEPDriveApplBlock.maintenanceCountValue_A636 = uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A028;
        writeDWORD(EEP_OPERATION_COUNT, uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600); 
		//writeWORD(EEP_MAINTENANCE_COUNT_VALUE,uDriveApplBlockEEP.stEEPDriveApplBlock.maintenanceCountValue_A636);
        updateApplBlockCrc();    
        setParamIndex = INVALID_PARAMETER;
    }
}

VOID getBlockStartAndLength(UINT8 startIndex, UINT8 endIndex, UINT8 *blockLength, UINT8 *blockArrayIndex, UINT16 *blockPhysicalAddr)
{
    UINT8 index, blockStartLocation;
	*blockArrayIndex = 0;  
	*blockLength = 0;
	*blockPhysicalAddr = 0;
    if(startIndex <= A131_APERTURE_MODE_ENABLE) // parameter to be read from common block 
    {
    	blockStartLocation = A100_UPPER_STOPPING_POS;
    }
    else if((startIndex > A131_APERTURE_MODE_ENABLE) 
				&& (startIndex <= A028_OPERATION_COUNTER)) // parameter to be read from appl block 
    {
		blockStartLocation = A008_SNOW_MODE_PHOTOELEC;
    }
    else if((startIndex > A028_OPERATION_COUNTER)
				&& (startIndex <= A807_BUF_OVRFLW_COUNT)) // appl status block parameter 
    {
    	blockStartLocation = A605_DRIVE_BOARD_STATUS;
    }
    else if((startIndex > A807_BUF_OVRFLW_COUNT)
				&& (startIndex <= A551_JOG_SPEED)) // motor control block 
    {
		blockStartLocation = A011_DECEL_BY_PHOTO_ELEC_BLOCKING_LIM;
    }


    for(index = 0; index < blockStartLocation; index++)
    {
		*blockArrayIndex = *blockArrayIndex + driveParamDatLengths[index];
    }

    for(index = blockStartLocation; index < endIndex; index++)
    {
		*blockLength = *blockLength + driveParamDatLengths[index];  
    }


    if(startIndex <= A131_APERTURE_MODE_ENABLE) // parameter to be read from common block 
    {
		*blockPhysicalAddr = EEP_COMMON_BLOCK_START;
    }
    else if((startIndex > A131_APERTURE_MODE_ENABLE)
				&& (startIndex <= A028_OPERATION_COUNTER)) // parameter to be read from appl block 
    {
		*blockPhysicalAddr = EEP_APPL_BLOCK_START;
    }
    else if((startIndex > A028_OPERATION_COUNTER)
				&& (startIndex <= A807_BUF_OVRFLW_COUNT)) // appl status block parameter 
    {
		*blockPhysicalAddr = EEP_APPL_STAT_FAULT_BLOCK_START;
    }
    else if((startIndex > A807_BUF_OVRFLW_COUNT)
				&& (startIndex <= A551_JOG_SPEED)) // motor control block 
    {
		*blockPhysicalAddr = EEP_MOTOR_CTRL_BLOCK_START;
    }
                

}

UINT8 getParamStartAndLength(UINT8 paramIndex, UINT8* byteCount)
{
	UINT16 index, blockStartIndex = 0; 
	UINT8 byteLocation = 0; 

	if(paramIndex <= A131_APERTURE_MODE_ENABLE) // parameter to be read from common block 
	{
		blockStartIndex = A100_UPPER_STOPPING_POS;
	}
	else if((paramIndex > A131_APERTURE_MODE_ENABLE)
				&& (paramIndex <= A028_OPERATION_COUNTER)) // parameter to be read from appl block 
	{
		blockStartIndex = A008_SNOW_MODE_PHOTOELEC;
	}
	else if((paramIndex > A028_OPERATION_COUNTER)
				&& (paramIndex <= A807_BUF_OVRFLW_COUNT)) // appl status block parameter 
	{
		blockStartIndex = A605_DRIVE_BOARD_STATUS;
	}
	else if((paramIndex > A807_BUF_OVRFLW_COUNT)
		 		&& (paramIndex <= A551_JOG_SPEED)) // motor control block 
	{
		blockStartIndex = A011_DECEL_BY_PHOTO_ELEC_BLOCKING_LIM;
	}

	for(index = blockStartIndex; index < paramIndex; index++)
	{
		byteLocation = byteLocation + driveParamDatLengths[index]; 
	}
	
	*byteCount = driveParamDatLengths[paramIndex]; 

	return byteLocation; 
}


//UINT8 fetchByteFrmFlash(UINT8 index)
//{
//	return FlashArray[index]; 
//}


VOID resetShutterPositions(VOID)
{
	uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100 = uDriveCommonBlockEEPDefault.stEEPDriveCommonBlock.upperStoppingPos_A100;
	uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 = uDriveCommonBlockEEPDefault.stEEPDriveCommonBlock.lowerStoppingPos_A101;
	uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102 = uDriveCommonBlockEEPDefault.stEEPDriveCommonBlock.photoElecPosMonitor_A102;
	uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128 = uDriveCommonBlockEEPDefault.stEEPDriveCommonBlock.originSensorPosMonitor_A128 ;
	uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130 = uDriveCommonBlockEEPDefault.stEEPDriveCommonBlock.apertureHeightPos_A130; 
	uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637 = 0; // reset drift 
	uDriveApplBlockEEP.stEEPDriveApplBlock.resetToDefaultValues_A120 = 0;	 
    
    //INTCON2bits.GIE = 0; //Disable all interrupts
    //Update data in EEPROM
	writeWORD(EEP_UPPER_STOPPING_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.upperStoppingPos_A100); 
	writeWORD(EEP_LOWER_STOPPING_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101); 
	writeWORD(EEP_PHOTOELEC_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102); 
	writeWORD(EEP_ORIGIN_SENS_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorPosMonitor_A128); 				
	writeWORD(EEP_APERTURE_HEIGHT_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.apertureHeightPos_A130); 
	writeWORD(EEP_ORIGIN_SENS_DRIFT_POS, uDriveCommonBlockEEP.stEEPDriveCommonBlock.originSensorDrift_A637); 
	updateCommonBlockCrc();
    writeBYTE(EEP_RESET_TO_DEFAULT, uDriveApplBlockEEP.stEEPDriveApplBlock.resetToDefaultValues_A120);
    updateApplBlockCrc();
    //INTCON2bits.GIE = 1; //Enable all interrupts
    
    //Check limits and perform installation
    checkShutterInstallation();
}


VOID resetAllParameters(VOID)
{
//    if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == BEAD_SHUTTER)
//        uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock = uEEP_BeadDriveMotorCtrlBlockDefault.stEEPDriveMotorCtrlBlock;
//    else if(uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537 == M1_SHUTTER)
//        uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock = uEEP_M1DriveMotorCtrlBlockDefault.stEEPDriveMotorCtrlBlock;
#ifdef MOTOR_750W_BD
         uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock = uEEP_BeadDriveMotorCtrlBlockDefault.stEEPDriveMotorCtrlBlock;
#endif
#ifdef MOTOR_750W_M1
         uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock = uEEP_M1DriveMotorCtrlBlockDefault.stEEPDriveMotorCtrlBlock;
#endif    
    
    uDriveApplBlockEEP.stEEPDriveApplBlock.initialValSetting_A021 = 0;
    SysCntCopy = uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600;    //bug_NO.59
    uDriveApplBlockEEP.stEEPDriveApplBlock = uDriveApplBlockEEPDefault.stEEPDriveApplBlock;
    uDriveApplBlockEEP.stEEPDriveApplBlock.operationCount_A600 = SysCntCopy;   //bug_NO.59
    uDriveCommonBlockEEP.stEEPDriveCommonBlock = uDriveCommonBlockEEPDefault.stEEPDriveCommonBlock;
    
    //INTCON2bits.GIE = 0; //Disable all interrupts
    //writeBlock(EEP_MOTOR_CTRL_BLOCK_START, uEEPDriveMotorCtrlBlock.val, DRIVE_MOTOR_BLOCK_DAT_LENGTH);
    wrBlUsingWriteByte(EEP_MOTOR_CTRL_BLOCK_START, uEEPDriveMotorCtrlBlock.val, DRIVE_MOTOR_BLOCK_DAT_LENGTH);
    updateMotorBlockCrc();
    //writeBlock(EEP_APPL_BLOCK_START, uDriveApplBlockEEP.val, APPL_BLOCK_DAT_LENGTH);
    wrBlUsingWriteByte(EEP_APPL_BLOCK_START, uDriveApplBlockEEP.val, APPL_BLOCK_DAT_LENGTH);
    updateApplBlockCrc();
    //writeBlock(EEP_COMMON_BLOCK_START, uDriveCommonBlockEEP.val, COMMON_BLOCK_DAT_LENGTH);
    wrBlUsingWriteByte(EEP_COMMON_BLOCK_START, uDriveCommonBlockEEP.val, COMMON_BLOCK_DAT_LENGTH);
    updateCommonBlockCrc();
    //INTCON2bits.GIE = 1; //Enable all interrupts
#ifdef MOTOR_750W_BD
    currShutterType = BEAD_SHUTTER;//uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537;
#endif
#ifdef MOTOR_750W_M1
	currShutterType = M1_SHUTTER;//uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537;
#endif	

	gucInstallationCalledFrom = 2;
    checkShutterInstallation();
}

//VOID resetAllParameters(VOID)
//{     
//    uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock = uEEPDriveMotorCtrlBlockDefault.stEEPDriveMotorCtrlBlock;
//    uDriveApplBlockEEP.stEEPDriveApplBlock.initialValSetting_A021 = 0;
//    uDriveApplBlockEEP.stEEPDriveApplBlock = uDriveApplBlockEEPDefault.stEEPDriveApplBlock;
//    uDriveCommonBlockEEP.stEEPDriveCommonBlock = uDriveCommonBlockEEPDefault.stEEPDriveCommonBlock;

//    writeBlock(EEP_MOTOR_CTRL_BLOCK_START, uEEPDriveMotorCtrlBlock.val, DRIVE_MOTOR_BLOCK_DAT_LENGTH);
//    //wrBlUsingWriteByte(EEP_MOTOR_CTRL_BLOCK_START, uEEPDriveMotorCtrlBlock.val, DRIVE_MOTOR_BLOCK_DAT_LENGTH);
//    updateMotorBlockCrc();
//    writeBlock(EEP_APPL_BLOCK_START, uDriveApplBlockEEP.val, APPL_BLOCK_DAT_LENGTH);
//    //wrBlUsingWriteByte(EEP_APPL_BLOCK_START, uDriveApplBlockEEP.val, APPL_BLOCK_DAT_LENGTH);
//    updateApplBlockCrc();
//    writeBlock(EEP_COMMON_BLOCK_START, uDriveCommonBlockEEP.val, COMMON_BLOCK_DAT_LENGTH);
//    //wrBlUsingWriteByte(EEP_COMMON_BLOCK_START, uDriveCommonBlockEEP.val, COMMON_BLOCK_DAT_LENGTH);
//    updateCommonBlockCrc();

//}

VOID updateCommonBlockCrc(VOID)
{
    UINT16 calculatedCRC;
    
    calculatedCRC = crc16(uDriveCommonBlockEEP.val, (COMMON_BLOCK_DAT_LENGTH - CRC_LENGTH), CRC_SEED);
    uDriveCommonBlockEEP.stEEPDriveCommonBlock.blockCRC = calculatedCRC;
    writeWORD(EEP_COMMON_BLOCK_CRC,calculatedCRC);
}

VOID updateApplBlockCrc(VOID)
{
    UINT16 calculatedCRC;

    calculatedCRC = crc16(uDriveApplBlockEEP.val, (APPL_BLOCK_DAT_LENGTH - CRC_LENGTH), CRC_SEED);
    uDriveApplBlockEEP.stEEPDriveApplBlock.blockCRC = calculatedCRC;
    writeWORD(EEP_APPL_BLOCK_CRC,calculatedCRC);
}

VOID updateMotorBlockCrc(VOID)
{
    UINT16 calculatedCRC;
    
    calculatedCRC = crc16(uEEPDriveMotorCtrlBlock.val, (DRIVE_MOTOR_BLOCK_DAT_LENGTH - CRC_LENGTH), CRC_SEED);
    uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.blockCRC = calculatedCRC;
    writeWORD(EEP_DRIVE_BLOCK_CRC,calculatedCRC);
}
