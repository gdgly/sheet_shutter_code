/********************************************************************************
* FileName: CommandHandler.h
* Description:  
* This header file contains the decleration of all the attributes and 
* services for CommandHandler.c file. It implements the handling of  
* commands from the Control board
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
 *  22/04/2014            iGate          Initial Creation                                                               
*****************************************************************************/
#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H
#include "./Common/Typedefs/Typedefs.h"

#define OPEN_SHUTTER 					0x01
#define CLOSE_SHUTTER					0x02
#define STOP_SHUTTER					0x04
#define OPEN_SHUTTER_JOG_50 			0x11
#define OPEN_SHUTTER_JOG_10				0x09
#define OPEN_SHUTTER_APERTURE			0x21
#define CLOSE_SHUTTER_JOG_50			0x12
#define CLOSE_SHUTTER_JOG_10			0x0A
#define CLOSE_SHUTTER_APERTURE			0x22
#define CLOSE_SHUTTER_IGNORE_SENSORS	0x02

#define COMMAND_RX_BUFFER_SIZE			30
#define MAX_COMMAND_DATA_LENGTH			16
#define ACK_COMMAND_LENGTH				1 
#define CRC_LENGTH						2 
//	Changed from 17 to 19 as power on calibration control commands added
//  Changed from 19 to 20 by AOYAGI_ST to add clean error function
#define NUM_OF_PC_COMMANDS	12//19
#define CMD_STATE_SIZE					1

#define CRC_SEED 0 //0xFFFF

#define DRIVE_INSTALL_STATUS_A100 			0x01
#define DRIVE_INSTALL_STATUS_A101 			0x02
#define DRIVE_INSTALL_STATUS_A102 			0x04
#define DRIVE_INSTALL_STATUS_CALIBRATION 	0x08
#define DRIVE_INSTALL_STATUS_SUCCESS 		0x10
#define DRIVE_INSTALL_STATUS_FAILED 		0x20


// Constants defining the length of the commands from CB 
#define RUN_MOTOR_CW_CMND_LEN 				0x06
#define RUN_MOTOR_CCW_CMND_LEN 				0x06
#define STOP_MOTOR_CMND_LEN 				0x06
#define CHANGE_MOTOR_TYPE_750W_CMND_LEN 	0x06
#define CHANGE_MOTOR_TYPE_1500W_CMND_LEN    0x06
#define SET_SPEED_CMND_LEN                  0x08
#define OPEN_FAN_CMND_LEN                   0x06
#define CLOSE_FAN_CMND_LEN                  0x06

#define WRITE_EEPROM_CMND_LEN 				0x07
#define READ_EEPROM_CMND_LEN                0x06
#define MOTOR_RUN_CYCLE_CW_CMND_LEN            0x06
#define MOTOR_RUN_CYCLE_CCW_CMND_LEN            0x06


#define MAX_CMND_LENGTH 			0x0a//SET_PARAM_CMND_LEN		

#define LENGTH_OF_ACK_PACKET 1

#define DATA_BYTE_COUNT 4
#define MAX_GET_PARAMETER_REPLY_LENGTH 	9 // 12 
#define FRAME_HEADER_FOOTER_LENGTH 		(3 + DRIVE_BLOCK_CRC_DAT_LENGTH) // dest ID, source ID, data length, CRC 


// Data structure for commands from CB - this is required to be defined outside the following union as
// we need to create a const containing the command list for command validation 
#pragma pack(1)
typedef struct 	
{
	UINT8 destAddr; 
	UINT8 sourceAddr; 
	UINT8 dataLength;
	UINT8 commandName; 
	UINT8 commandDataWithCRC[MAX_COMMAND_DATA_LENGTH + CRC_LENGTH]; // TBD - most commands would be upto 4 bytes length? extra bytes here are only to match the 30 byte union 
	UINT8 recdCmdState; 
}_stCBCommand; 

// Data structure for commands from CB  // TBD - whether to use same format while issuing RESTART_DRIVE_COMM command to CB  
typedef union 
{
	UINT8 command[COMMAND_RX_BUFFER_SIZE + CMD_STATE_SIZE]; // UART Rx buffer size is 30, allocate (30 + 1) here 
	_stCBCommand stCBCommand; 	
}_uCBCommand; 

EXTERN _uCBCommand uCBCommand; 

EXTERN BOOL txAnomalyHistInProgress; 
EXTERN BOOL txInProgress;

/* Function to read and validate the command in communication buffer */ 
BOOL readCmndFromCommBuffer(VOID);

/* This function initializes globals used by the command handler */
VOID initCommandHandler(VOID); 

BOOL processCMDFromCB(UINT8* readData, UINT8 dataLength); 

/* This function implements the command handler */ 
VOID commandHandler(VOID); 

/* This function implements ACK/ NACK replies from the Drive to the Control board  */
VOID transmitACK(BOOL bReply); 
VOID transmitDat(BYTE data);

/* This function implements GET_PARAMETER replies from the Drive to the Control board  */
VOID transmitParameter(UINT32 data, UINT16 paramNumber, UINT8 byteCount); 

VOID transmitRestartComm(VOID); 
VOID checkSerialTxCompleted(VOID);
// ******************************************************************************************************
// Function modified to not scan "PE Sensor" depending on the input parameter - YG - Nov 2015
// Input variable
// BOOL scanPE_Sensor : The variable will decide, whether to scan PE sensor or not
// FALSE = do not scan pe sensor, TRUE = scan pe sensor
// ******************************************************************************************************
BOOL readCurrSensorState(BOOL scanPE_Sensor);

VOID transmitMotorStatus(VOID);

VOID transmitRunMotorCWCMD(VOID);

VOID transmitRunMotorCCWCMD(VOID);

VOID transmitStopMotorCMD(VOID);

VOID transmitMotorPhaseAngle(UINT16 CNT);


/* Calculate 16 bit CRC */
//UINT16 cumulativeCrc16(UINT8 *buf, UINT16 bsize, UINT16 crcSeed); 
UINT16 crc16(const UINT8 *pui8Data, UINT32 ui32Count, UINT16 ui16Crc); 



#endif /* COMMAND_HANDLER_H */
