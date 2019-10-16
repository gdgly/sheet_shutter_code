/*********************************************************************************
* FileName: CommandHandler.c
* Description:
* This source file contains the definition of all the functions for CommandHandler.
* It implements all the functions required for handling commands from the Control Board.
 * 
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
 *  22/04/2014            iGate          Initial Creation                                                               
*****************************************************************************/
#include <p33Exxxx.h>
#include <string.h>
#include "CommandHandler.h"
#include "./Common/UserDefinition/Userdef.h"
#include "./Application/RampGenerator/RampGenerator.h"
#include "Application.h"
#include "./Middleware/ParameterDatabase/eeprom.h"
#include "./Middleware/CommunicationStack/serial.h"
#include "./Drivers/Timer/Timer.h"
#include "./Common/Extern/Extern.h"
#include "../Common/Delay/Delay.h"



#define FOUR_BYTE_PARAM_FRAME_DATA_LEN 	(6 + 4 + DRIVE_BLOCK_CRC_DAT_LENGTH) // dest ID + source ID + data len + command ID + 16 bit param index + 4 byte data + crc 
#define TWO_BYTE_PARAM_FRAME_DATA_LEN 	(6 + 2 + DRIVE_BLOCK_CRC_DAT_LENGTH) // dest ID + source ID + data len + command ID + 16 bit param index + 2 byte data + crc 
#define ONE_BYTE_PARAM_FRAME_DATA_LEN 	(6 + 1 + DRIVE_BLOCK_CRC_DAT_LENGTH) // dest ID + source ID + data len + command ID + 16 bit param index + 1 byte data + crc 

#define PARAM_INDEX_MSB_LOCATION	0
#define PARAM_INDEX_LSB_LOCATION 	1 
#define PARAM_DATA_START_INDEX		2

#define POC 1
#define DEBUG_UART 

BYTE a;

UINT32 x;

//*****************************************************************************
//
// This macro executes one iteration of the CRC-16.
//
//*****************************************************************************
#define CRC16_ITER(crc, data)   (((crc) >> 8) ^                               \
                                 g_pui16Crc16[(UINT8)((crc) ^ (data))])


// TBD - Expected CRCs below are not used except for debugging, to be removed later

// Constants defining the CRC of the commands from CB 
//Following CRCs are to be updated. this is just arbitrary data 
#define EXPECTED_CRC_RUN_MOTOR_CW				0x50F2
#define EXPECTED_CRC_RUN_MOTOR_CCW 				0x51B2  
#define EXPECTED_CRC_STOP_MOTOR 				0x9173
#define EXPECTED_CRC_CHANGE_MOTOR_TYPE_750W 	0x5332
#define EXPECTED_CRC_CHANGE_MOTOR_TYPE_1500W 	0x93F3
#define EXPECTED_CRC_SET_SPEED                  0xF2B7
#define EXPECTED_CRC_OPEN_FAN                   0x5272  
#define EXPECTED_CRC_CLOSE_FAN                  0x5632
#define EXPECTED_CRC_WRITE_EEPROM               0xE387
#define EXPECTED_CRC_READ_EEPROM 				0x97B3
#define EXPECTED_CRC_MOTOR_RUN_CYCLE_CW         0x5772
#define EXPECTED_CRC_MOTOR_RUN_CYCLE_CCW        0x9533
             

enum {
	no_error = 0, 
	UART_channel_disabled,
	invalid_UART_channel, 
	UART_busy, 
	receive_buffer_is_empty, 
	number_of_channel_errors
}eUARTChannelCheck_cmd; // error list - used when checking if UART channel number is valid 



enum{
	no_command = number_of_channel_errors, 
	command_recv_in_progress, 
	new_command_recd
}eUARTCommandRxStatus_cmd;   // used by - stTxRxBuffer[].uchCommandRXStatus 


/* Command names */
enum _PCCommand
{
	run_motor_cw = 1, 
	run_motor_ccw, 
	stop_motor, 
	change_motor_type_750w, 
    change_motor_type_1500w,
	set_speed, 
	open_fan, 
	close_fan, 
	write_eeprom, 
	read_eeprom, 
    motor_run_cycle_cw,
    motor_run_cycle_ccw,
	
}enPCCommand; /* Used to identify command coming from CB */ 


///* Jog percentage */
//enum _jogPercentage
//{
//	no_jog = 0,
//	ten_percent_jog, 
//	fifty_percent_jog
//}enCmdJogPercentage;

#define ten_percent_jog 10
#define fifty_percent_jog 50

/* Reply to CB - None, ACK, NACK */
enum _ackToCB
{
	no_reply_reqd = 0, 
	ack,
	nack
}ackToCB; 

enum _commandState
{
	no_cmnd = 0,
	new_cmd, 
	invalid, 
	processing,
	complete
}commandState; 

_uCBCommand uCBCommand; /* CB Command under process */ 

BOOL txAnomalyHistInProgress; 

BOOL txInProgress; 
WORD txResetCount;

//	Added this flag to handle issue related to stop action (stopping was not smooth) when stop key is pressed
BYTE gui8StopKeyPressed = 0;

BYTE wriData = 0;
BYTE redData = 0;

BYTE DriveRDY = 0;
BYTE MotorRunInCycle = 0;
BYTE uart_motor_stop=0;
//*****************************************************************************
//
// The CRC-16 table for the polynomial C(x) = x^16 + x^15 + x^2 + 1 (standard
// CRC-16, also known as CRC-16-IBM and CRC-16-ANSI).
//
//*****************************************************************************
static const UINT16 g_pui16Crc16[256] =
{
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};


// Create const structure containing definitions for all incoming commands,
// reply - ACK/ NACK and the command from drive to Restart communication 


// TBD - Expected CRC in below const is not used, to be removed 

//Destination Address,   Source Address,   Length in Bytes including CRC,   Command,   Data,   CRC 16 Byte 1,   CRC 16 Byte 2
// List of commands from Control board - this constant is required for validating commands received from the CB 
const _stCBCommand constCBCommandList[NUM_OF_PC_COMMANDS] = 
{
	{DRIVE_BOARD_ADDRESS, PC_ADDRESS, RUN_MOTOR_CW_CMND_LEN, 			run_motor_cw, 				{(EXPECTED_CRC_RUN_MOTOR_CW & 0x00FF) , ((EXPECTED_CRC_RUN_MOTOR_CW & 0xFF00) >> 8)}, no_cmnd }, 
	{DRIVE_BOARD_ADDRESS, PC_ADDRESS, RUN_MOTOR_CCW_CMND_LEN, 			run_motor_ccw,				{(EXPECTED_CRC_RUN_MOTOR_CCW & 0x00FF) , ((EXPECTED_CRC_RUN_MOTOR_CCW & 0xFF00) >> 8)}, no_cmnd }, 
	{DRIVE_BOARD_ADDRESS, PC_ADDRESS, STOP_MOTOR_CMND_LEN,              stop_motor,                 {(EXPECTED_CRC_STOP_MOTOR & 0x00FF) , ((EXPECTED_CRC_STOP_MOTOR & 0xFF00) >> 8)}, no_cmnd }, 

	{DRIVE_BOARD_ADDRESS, PC_ADDRESS, CHANGE_MOTOR_TYPE_750W_CMND_LEN, 	change_motor_type_750w, 	{(EXPECTED_CRC_CHANGE_MOTOR_TYPE_750W & 0x00FF) , ((EXPECTED_CRC_CHANGE_MOTOR_TYPE_750W & 0xFF00) >> 8)}, no_cmnd }, 
	{DRIVE_BOARD_ADDRESS, PC_ADDRESS, CHANGE_MOTOR_TYPE_1500W_CMND_LEN,	change_motor_type_1500w,	{(EXPECTED_CRC_CHANGE_MOTOR_TYPE_1500W & 0x00FF) , ((EXPECTED_CRC_CHANGE_MOTOR_TYPE_1500W & 0xFF00) >> 8)}, no_cmnd }, 

	{DRIVE_BOARD_ADDRESS, PC_ADDRESS, SET_SPEED_CMND_LEN,               set_speed,              	{(EXPECTED_CRC_SET_SPEED & 0x00FF) , ((EXPECTED_CRC_SET_SPEED & 0xFF00) >> 8)}, no_cmnd }, 
	{DRIVE_BOARD_ADDRESS, PC_ADDRESS, OPEN_FAN_CMND_LEN,                open_fan,               	{(EXPECTED_CRC_OPEN_FAN & 0x00FF) , ((EXPECTED_CRC_OPEN_FAN & 0xFF00) >> 8)}, no_cmnd }, 
	{DRIVE_BOARD_ADDRESS, PC_ADDRESS, CLOSE_FAN_CMND_LEN,               close_fan,                  {(EXPECTED_CRC_CLOSE_FAN & 0x00FF) , ((EXPECTED_CRC_CLOSE_FAN & 0xFF00) >> 8)}, no_cmnd }, 

	{DRIVE_BOARD_ADDRESS, PC_ADDRESS, WRITE_EEPROM_CMND_LEN, 			write_eeprom,				{(EXPECTED_CRC_WRITE_EEPROM & 0x00FF) , ((EXPECTED_CRC_WRITE_EEPROM & 0xFF00) >> 8)}, no_cmnd }, 
	{DRIVE_BOARD_ADDRESS, PC_ADDRESS, READ_EEPROM_CMND_LEN, 			read_eeprom,				{(EXPECTED_CRC_READ_EEPROM & 0x00FF) , ((EXPECTED_CRC_READ_EEPROM & 0xFF00) >> 8)}, no_cmnd }, 
    {DRIVE_BOARD_ADDRESS, PC_ADDRESS, MOTOR_RUN_CYCLE_CW_CMND_LEN, 		motor_run_cycle_cw,			{(EXPECTED_CRC_MOTOR_RUN_CYCLE_CW & 0x00FF) , ((EXPECTED_CRC_MOTOR_RUN_CYCLE_CW & 0xFF00) >> 8)}, no_cmnd },
    {DRIVE_BOARD_ADDRESS, PC_ADDRESS, MOTOR_RUN_CYCLE_CCW_CMND_LEN,     motor_run_cycle_ccw,		{(EXPECTED_CRC_MOTOR_RUN_CYCLE_CCW & 0x00FF) , ((EXPECTED_CRC_MOTOR_RUN_CYCLE_CCW & 0xFF00) >> 8)}, no_cmnd },

}; 

/******************************************************************************
 * readCmndFromCommBuffer
 *
 * This function reads the command from Control board (if any) in communication buffer  
 * Validation of the command also comes within the scope of this function. 
 *
 * PARAMETER REQ:  none 
 *
 * RETURNS: TRUE if command received is valid 
 *
 * ERRNO: none
 ********************************************************************************/
BOOL readCmndFromCommBuffer(VOID)
{
	UINT8 readData[COMMAND_RX_BUFFER_SIZE]; 
	UINT8 dataLength = 0; 
	
	BOOL bCommandValid = FALSE; 
	if(new_command_recd == uartCheckNewCommandReceived(APPL_ASSIGNED_UART_CHANNEL))
	{		
		if(processing != uCBCommand.stCBCommand.recdCmdState) // if currently a command is being processed, then return from here, wait 
		{
			if(no_error == uartGetCommand(APPL_ASSIGNED_UART_CHANNEL, readData, &dataLength)) // this function call has read and erased the command from UART RX buffer 
			{
                bCommandValid = processCMDFromCB(readData, dataLength); 
			}
		}		
	}

	return bCommandValid; 	
}

BOOL processCMDFromCB(UINT8* readData, UINT8 dataLength)
{
	BOOL bCommandValid = FALSE; 
	UINT16 calculatedCRC = 0;
	UINT8 index; 
	BOOL bCommandFound = FALSE; 
	
	// save the command into local buffer, read command of byteCountRxBuffer bytes 
	memcpy((char *)&uCBCommand.command[0], (char *)&readData[0], dataLength); 
		
	uCBCommand.stCBCommand.recdCmdState = new_cmd; 
	calculatedCRC = crc16(readData, (uCBCommand.stCBCommand.dataLength-CRC_LENGTH), CRC_SEED); 
    x = calculatedCRC;


	// TBD - validate if we already read this command in the previous cycle, ->>>  this should not be done 
	// we may receive back to back OPEN_SHUTTER_JOG for example 


	// also validate if command is as expected wrt byte sequences, and it's CRC 
	for(index = 0; index < NUM_OF_PC_COMMANDS; index++)
	{
		if(uCBCommand.stCBCommand.commandName == constCBCommandList[index].commandName)
		{	
			bCommandFound = TRUE; 
                if((uCBCommand.stCBCommand.sourceAddr == constCBCommandList[index].sourceAddr)
                    && (uCBCommand.stCBCommand.destAddr == constCBCommandList[index].destAddr)
                    && (uCBCommand.stCBCommand.dataLength == constCBCommandList[index].dataLength)				      
                    )
                {
                    bCommandValid = TRUE; 	
                    // Clear Drive communication fault - CRC error 
                    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveCommunicationFault.bits.crcError = FALSE;
                    break; 
                }

			// this is required while testing with Hyperterm, not necc while testing with control board 
			// TBD start of debug code, to be removed later!!!!!!!!!!!!
			// for POC, stop command CRC is not checked
			// only for stop command, even if packet is transmitted correctly, it is recd with wrong CRC
			//else if(uCBCommand.stCBCommand.commandName == stop_shutter)
			//{
			//	bCommandValid = TRUE; 	
			//	break;
			//}
			// end of debug code !!!!!		

			else // command received is not valid 
			{
				if(!(((uCBCommand.command[uCBCommand.stCBCommand.dataLength - 2]) == ((calculatedCRC & 0xFF00) >> 8)) // CRC MSB
					&& ((uCBCommand.command[uCBCommand.stCBCommand.dataLength - 1]) == (calculatedCRC & 0x00FF))))
				{
					// Log Drive communication fault - CRC error 
                    PORTAbits.RA7 = 0;
                    PORTCbits.RC0 = 0;
					uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveCommunicationFault.bits.crcError = TRUE;	
				}
				else // invalid destination address and/ or data length 
				{
					// Log Drive communication fault - command frame error 
					uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveCommunicationFault.bits.commandFrameError = TRUE;
				}

				uCBCommand.stCBCommand.recdCmdState = invalid; 
				break; 
			}
		}
	}

	if(!bCommandFound) // invalid command 
	{
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveCommunicationFault.bits.commandFrameError = TRUE;
		uCBCommand.stCBCommand.recdCmdState = invalid; 
	}
	else 
	{
		// Clear Drive communication fault - command frame error 
		uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveCommunicationFault.bits.commandFrameError = FALSE;
	}

	return bCommandValid; 

}




/******************************************************************************
 * initCommandHandler
 *
 * This function initializes globals used by the command handler 
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initCommandHandler(VOID)
{
	UINT8 index; 

    txInProgress = FALSE; 
    
	// initialize the current command to all zero 
	for(index = 0; index < (COMMAND_RX_BUFFER_SIZE + CMD_STATE_SIZE); index++)
	{
		uCBCommand.command[index] = 0; 
	}

	initUARTBuffers(); 

    PORTCbits.RC4 = 0;
    
	configureUART(APPL_ASSIGNED_UART_CHANNEL); 

	txAnomalyHistInProgress = FALSE; 
    
    currShutterType = MotorType;

}

VOID checkSerialTxCompleted(VOID)
{
    if((txInProgress == TRUE) && (stTxRxBuffer[0].uchTxBufferByteCount == 0))
    {
        if(++txResetCount > 2)
        {
            PORTCbits.RC4 = 0;
            txInProgress = FALSE;
        }
    }
}
/******************************************************************************
 * CommandHandler
 *
 * This function implements the command handler 
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID commandHandler(VOID)
{
	unsigned char status = no_reply_reqd;
    
	//UINT32 parameter = 0; 
   // UINT32 rdParameter = 0;
	UINT16 paramIndex = 0; 
	//BOOL result = FALSE; 
	//UINT8 byteCount = 0; 
   // BOOL faultTrgFlag = FALSE;

    //DWORD delayCnt = 0x2492; //2ms delay
	
	//if((txInProgress == TRUE) && (stTxRxBuffer[0].uchTxBufferByteCount == 0))
    //{
    //    while(delayCnt--);
    //    PORTCbits.RC4 = 0;
    //    txInProgress = FALSE;
    //}

    //if(!readCmndFromCommBuffer()) // command is invalid 
    //{
    //    if(invalid == uCBCommand.stCBCommand.recdCmdState)
    //    {
    //        // what do we do with invalid commands? we have already raised CRC/ UART fault, so we do nothing here
    //        // simply reply with ACK, and then overwrite this command when next one comes
    //        status = nack; 
    //    }
    //}
    
    if(readCmndFromCommBuffer())
    {
        if(new_cmd == uCBCommand.stCBCommand.recdCmdState)
        {
            status = ack;
            switch(uCBCommand.stCBCommand.commandName)
            {
                case run_motor_cw:  
//                    //PORTAbits.RA7 = 1;
//                    //PORTCbits.RC0 = 0;
//                    //flags.CMDFromPC = 1;
//                    flags.StartStop = 1;
//                    MotorRunInCycle = 0;
//                    MotorRunCount = 0;
//                    if ((flags.StartStop == 1) && (!flags.motorRunning))
//                    {
//                        lockRelease;
//                        delayMs(100); 
//                        startMotorCW();
//                    }
//                    else if((flags.motorRunning) && (requiredDirection == CCW) && (currentDirection == CCW))
//                    {
//                        //lockApply;
//                        stopMotor();
//                        delayMs(1000);
//                        //delayMs(1000);
//                        startMotorCW();
//                    }                    
                    break; 
                    
                case run_motor_ccw:
//                    //flags.CMDFromPC = 1;
//                    flags.StartStop = 1;
//                    MotorRunInCycle = 0;
//                    MotorRunCount = 0;
//                    if ((flags.StartStop == 1) && (!flags.motorRunning))
//                    {
//                        lockRelease;
//                        delayMs(100); 
//                        startMotorCCW();
//                    }
//                    else if((flags.motorRunning) && (requiredDirection == CW) && (currentDirection == CW))
//                    {
//                        //lockApply;
//                        stopMotor();
//                        delayMs(1000);
//                        //delayMs(1000);
//                        startMotorCCW();
//                        
//                    }
                    break; 
                    
                case stop_motor: 
                    //MotorRunInCycle = 0;
                    //MotorRunCount = 0;
//                    PORTAbits.RA7 = 0;
//                    PORTCbits.RC0 = 0;
                    
                    MotorRunInCycle=1;
                    MotorRunCount = 0;
                    uart_motor_stop=1;
                    //stopMotor();
                    //delayMs(100);
                    //lockApply;   
                    break; 
                    
                case change_motor_type_750w:
//                    MotorType = MOTOR_750W;
//                    DriveRDY = 1;
                    //transmitDat(0x04);
                    break;
                    
                case change_motor_type_1500w:    						
//                    MotorType = MOTOR_1500W;
//                    DriveRDY = 1;
                    //transmitDat(0x05);
                    break;
                    
                case set_speed:
//                    transmitDat(0x06);
                     break;       
                case open_fan:
                    //PORTAbits.RA7 = 0;
                    //PORTCbits.RC0 = 1;
                    fanON;
//                    T9CONbits.TON = 1;
                    //transmitDat(0x07);
                    break;
                case close_fan:
                    fanOFF;
                    //transmitDat(0x08);
//                    T9CONbits.TON = 0;
                    break;
                case write_eeprom:
                    //transmitDat(0x09);
                    paramIndex = (UINT16)(uCBCommand.stCBCommand.commandDataWithCRC[PARAM_INDEX_MSB_LOCATION]);
                    wriData = paramIndex;
                    writeBYTE(0, wriData);
                    break;
                case read_eeprom:
                    //testEEPRomData();
                    redData = readBYTE(0);
                    transmitDat(redData);
                    break;
                case motor_run_cycle_cw:
                    if ((!flags.motorRunning))
                    {
                        MotorRunInCycle = 1;
                        flags.RunDirection = CW;
                        MotorRunCount = 0;
                        startMotorCW();
                        //MotorRunCycle();
                    }
                    break;  
                case motor_run_cycle_ccw:
                    if ((!flags.motorRunning))
                    {
                        MotorRunInCycle = 1;
                        flags.RunDirection = CCW;
                        MotorRunCount = 0;
                        startMotorCCW();
                        //MotorRunCycle();
                    }
                    break;
                default: 
                        // could not recognise the command, hence reply with NACK - code is not expected to come here
                    PORTAbits.RA7 = 0;
                    PORTCbits.RC0 = 0;
                    //status = nack; // NACK to be sent to CB   
                    break; 
            }
            // mark the command read and processing complete 
            uCBCommand.stCBCommand.recdCmdState = complete; 
            //checkResetParameter();
        }
    }
}

// ******************************************************************************************************
// Function modified to not scan "PE Sensor" depending on the input parameter - YG - Nov 2015
// Input variable
// BOOL scanPE_Sensor : The variable will decide, whether to scan PE sensor or not
// FALSE = do not scan pe sensor, TRUE = scan pe sensor
// ******************************************************************************************************
BOOL readCurrSensorState(BOOL scanPE_Sensor)
{
    BOOL sts = FALSE;
    
    if(microSwSensorTrigrd)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.microSwitch = TRUE;
        sts = TRUE;
    }
    

    if(photoElecObsSensTrigrd && scanPE_Sensor == TRUE)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.peObstacle = TRUE;
        sts = TRUE;
    }
    
    if(tempSensTrigrd)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorOverheat = TRUE;  
        sts = TRUE;
    }
    
    if(emergencySensorTrigrd)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.emergencyStop = TRUE; 
        sts = TRUE;
    }
    
    //if(OvercurrentfaultTrigrd)
    //{
    //    uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorOverCurrent = TRUE; 
    //    sts = TRUE;
    //}
    
    if(!originSensorDetected && uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osDetectOnUp = TRUE;
        sts = TRUE;
    }
    
    if(originSensorDetected && uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osDetectOnDown = TRUE;
        sts = TRUE;
    }

    return(sts);
}


VOID transmitRestartComm(VOID)
{
	/*UINT16 crc = 0; 
	UINT8 buffer[RESTART_DRIVE_COMM_CMND_LEN]; 
	
	buffer[0] = CONTROL_BOARD_ADDRESS; 
	buffer[1] = DRIVE_BOARD_ADDRESS;
	buffer[2] = RESTART_DRIVE_COMM_CMND_LEN;
	buffer[3] = RESTART_DRIVE_COMM_COMMAND;

	crc = crc16((UINT8*)&buffer[0], (RESTART_DRIVE_COMM_CMND_LEN - DRIVE_BLOCK_CRC_DAT_LENGTH), CRC_SEED);
	buffer[4] = (UINT8)(crc); // lsb
	buffer[5] = (UINT8)(crc >> 8); // msb

	uartSendTxBuffer (APPL_ASSIGNED_UART_CHANNEL, buffer, RESTART_DRIVE_COMM_CMND_LEN);*/
}

VOID transmitRunMotorCWCMD(VOID)
{
	UINT16 crc = 0; 
	UINT8 buffer[0x06]; 
	
	buffer[0] = PC_ADDRESS; 
	buffer[1] = DRIVE_BOARD_ADDRESS;
	buffer[2] = 0x06;
	buffer[3] = 0x01;

	crc = crc16((UINT8*)&buffer[0], 0x04, CRC_SEED);
	buffer[4] = (UINT8)(crc); // lsb
	buffer[5] = (UINT8)(crc >> 8); // msb

	uartSendTxBuffer (APPL_ASSIGNED_UART_CHANNEL, buffer, 0x06);
}

VOID transmitRunMotorCCWCMD(VOID)
{
	UINT16 crc = 0; 
	UINT8 buffer[0x06]; 
	
	buffer[0] = PC_ADDRESS; 
	buffer[1] = DRIVE_BOARD_ADDRESS;
	buffer[2] = 0x06;
	buffer[3] = 0x02;

	crc = crc16((UINT8*)&buffer[0], 0x04, CRC_SEED);
	buffer[4] = (UINT8)(crc); // lsb
	buffer[5] = (UINT8)(crc >> 8); // msb

	uartSendTxBuffer (APPL_ASSIGNED_UART_CHANNEL, buffer, 0x06);
}

VOID transmitStopMotorCMD(VOID)
{
	UINT16 crc = 0; 
	UINT8 buffer[0x06]; 
	
	buffer[0] = PC_ADDRESS; 
	buffer[1] = DRIVE_BOARD_ADDRESS;
	buffer[2] = 0x06;
	buffer[3] = 0x03;

	crc = crc16((UINT8*)&buffer[0], 0x04, CRC_SEED);
	buffer[4] = (UINT8)(crc); // lsb
	buffer[5] = (UINT8)(crc >> 8); // msb

	uartSendTxBuffer (APPL_ASSIGNED_UART_CHANNEL, buffer, 0x06);
}

VOID transmitMotorPhaseAngle(UINT16 CNT)
{
//	UINT16 crc = 0; 
	UINT8 buffer[0x03]; 
    
	
	buffer[0] = (UINT8) ((PhaseURAT[CNT] & 0xFF00) >> 8); 
	buffer[1] = (UINT8) (PhaseURAT[CNT] & 0x00FF);
	buffer[2] = 0xFF;
//	buffer[3] = 0x03;

//	crc = crc16((UINT8*)&buffer[0], 0x04, CRC_SEED);
//	buffer[4] = (UINT8)(crc); // lsb
//	buffer[5] = (UINT8)(crc >> 8); // msb

	uartSendTxBuffer (APPL_ASSIGNED_UART_CHANNEL, buffer, 0x03);
}

VOID transmitMotorStatus(VOID)
{
    UINT8 buffer[0x0A]; 
    UINT16 crc = 0; 
    //UINT16 IBUS = 0;
    
    //IBUS = (iTotalADCCnt - 511) * 322
//	buffer[0] = (UINT8) ((PhaseURAT & 0xFF00) >> 8); 
//	buffer[1] = (UINT8) (PhaseURAT & 0x00FF);
	buffer[0] = PC_ADDRESS; 
	buffer[1] = DRIVE_BOARD_ADDRESS;
	buffer[2] = (UINT8) ((measuredSpeed & 0xFF00) >> 8);
	buffer[3] = (UINT8) (measuredSpeed & 0x00FF);
//    buffer[2] = (UINT8) ((PhaseURAT & 0xFF00) >> 8);
//	buffer[3] = (UINT8) (PhaseURAT & 0x00FF);
    buffer[4] = (UINT8) ((UBUS & 0xFF00) >> 8);
	buffer[5] = (UINT8) (UBUS & 0x00FF);
//    buffer[4] = (UINT8) ((controlOutput & 0xFF00) >> 8);
//	buffer[5] = (UINT8) (controlOutput & 0x00FF);
//    
    buffer[6] = (UINT8) ((measurediTotal & 0xFF00) >> 8);
	buffer[7] = (UINT8) (measurediTotal & 0x00FF);

	crc = crc16((UINT8*)&buffer[0], 0x08 , CRC_SEED);
	buffer[8] = (UINT8)(crc); // lsb
	buffer[9] = (UINT8)(crc >> 8); // msb

	uartSendTxBuffer (APPL_ASSIGNED_UART_CHANNEL, buffer, 0x0A);
}

/******************************************************************************
 * transmitACK
 *
 * This function implements ACK/ NACK replies from the Drive to the Control board  
 *
 * PARAMETER REQ: bReply - TRUE for ACK, FALSE for NACK 
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/

VOID transmitACK(BOOL bReply)
{
	// Call the serial function to send out ACK/ NACK
	// Create a buffer with x bytes and pass to lower layer 
	UINT8 transmitBuf[LENGTH_OF_ACK_PACKET];
	
	if(bReply)
	{
		transmitBuf[0] = ASCII_ACK; 
	}
	else
	{
		transmitBuf[0] =  ASCII_NACK;
	}

	uartSendTxBuffer(APPL_ASSIGNED_UART_CHANNEL, transmitBuf, LENGTH_OF_ACK_PACKET);

	
}


VOID transmitDat(BYTE data)
{
	// Call the serial function to send out ACK/ NACK
	// Create a buffer with x bytes and pass to lower layer 
	UINT8 transmitBuf[LENGTH_OF_ACK_PACKET];
	
    transmitBuf[0] = data;
    uartSendTxBuffer(APPL_ASSIGNED_UART_CHANNEL, transmitBuf, LENGTH_OF_ACK_PACKET);
}


VOID transmitParameter(UINT32 data, UINT16 paramNumber, UINT8 byteCount)
{
	UINT8 replyPacket[MAX_GET_PARAMETER_REPLY_LENGTH]; 
	UINT16 crc = 0; 
	UINT8 index = 0; 
	
	//crc = crc16((UINT8*)&data, byteCount, CRC_SEED); 
	replyPacket[index] = CONTROL_BOARD_ADDRESS; 
	replyPacket[++index] = DRIVE_BOARD_ADDRESS; 
	replyPacket[++index] = DATA_BYTE_COUNT + FRAME_HEADER_FOOTER_LENGTH; // + PARAM_OVERHEADS; 

	//replyPacket[++index] = (UINT8)(paramNumber >> 8); // msb
	//replyPacket[++index] = (UINT8)(paramNumber); // lsb;

	//replyPacket[++index] = 0; // index - for drive board it is 0

	switch(byteCount)
	{
		case (sizeof(BYTE)): 
			{
				replyPacket[++index] = 0x00; // buffer
				replyPacket[++index] = 0x00; // buffer
				replyPacket[++index] = 0x00; // buffer
				replyPacket[++index] = (UINT8)data;
			}
			break; 
		case (sizeof(WORD)): 
			{
				replyPacket[++index] = 0x00; // buffer
				replyPacket[++index] = 0x00; // buffer
				replyPacket[++index] = (UINT8)(data >> 8); // msb
				replyPacket[++index] = (UINT8)(data); // lsb;
			}
			break;  
		case (sizeof(DWORD)): 
			{
				replyPacket[++index] = (UINT8)(data >> 24);// msb
				replyPacket[++index] = (UINT8)(data >> 16);
				replyPacket[++index] = (UINT8)(data >> 8); 
				replyPacket[++index] = (UINT8)(data); // lsb;
			}
			break; 
		default: 
			break; 
	}; 

    crc = crc16(replyPacket, (index+1), CRC_SEED); 
    
	replyPacket[++index] = (UINT8)(crc >> 8); // msb
	replyPacket[++index] = (UINT8)(crc); // lsb

    uartSendTxBuffer(APPL_ASSIGNED_UART_CHANNEL, replyPacket, (index+1));	
	
}

//*****************************************************************************
//
//! Calculates the CRC-16 of an array of bytes.
//!
//! \param ui16Crc is the starting CRC-16 value.
//! \param pui8Data is a pointer to the data buffer.
//! \param ui32Count is the number of bytes in the data buffer.
//!
//! This function is used to calculate the CRC-16 of the input buffer.  The
//! CRC-16 is computed in a running fashion, meaning that the entire data block
//! that is to have its CRC-16 computed does not need to be supplied all at
//! once.  If the input buffer contains the entire block of data, then
//! \b ui16Crc should be set to 0.  If, however, the entire block of data is
//! not available, then \b ui16Crc should be set to 0 for the first portion of
//! the data, and then the returned value should be passed back in as
//! \b ui16Crc for the next portion of the data.
//!
//! For example, to compute the CRC-16 of a block that has been split into
//! three pieces, use the following:
//!
//! \verbatim
//!     ui16Crc = Crc16(0, pui8Data1, ui32Len1);
//!     ui16Crc = Crc16(ui16Crc, pui8Data2, ui32Len2);
//!     ui16Crc = Crc16(ui16Crc, pui8Data3, ui32Len3);
//! \endverbatim
//!
//! Computing a CRC-16 in a running fashion is useful in cases where the data
//! is arriving via a serial link (for example) and is therefore not all
//! available at one time.
//!
//! \return The CRC-16 of the input data.
//
//*****************************************************************************
UINT16
crc16(const UINT8 *pui8Data, UINT32 ui32Count, UINT16 ui16Crc)
{
    UINT32 ui32Temp;

    //
    // If the data buffer is not 16 bit-aligned, then perform a single step of
    // the CRC to make it 16 bit-aligned.
    //
    if((UINT32)pui8Data & 1)
    {
        //
        // Perform the CRC on this input byte.
        //
        ui16Crc = CRC16_ITER(ui16Crc, *pui8Data);

        //
        // Skip this input byte.
        //
        pui8Data++;
        ui32Count--;
    }

    //
    // If the data buffer is not word-aligned and there are at least two bytes
    // of data left, then perform two steps of the CRC to make it word-aligned.
    //
    if(((UINT32)pui8Data & 2) && (ui32Count > 1))
    {
        //
        // Read the next 16 bits.
        //
        ui32Temp = *(UINT16 *)pui8Data;

        //
        // Perform the CRC on these two bytes.
        //
        ui16Crc = CRC16_ITER(ui16Crc, ui32Temp);
        ui16Crc = CRC16_ITER(ui16Crc, ui32Temp >> 8);

        //
        // Skip these input bytes.
        //
        pui8Data += 2;
        ui32Count -= 2;
    }

    //
    // While there is at least a word remaining in the data buffer, perform
    // four steps of the CRC to consume a word.
    //
    while(ui32Count > 3)
    {
        //
        // Read the next word.
        //
        ui32Temp = *(UINT32 *)pui8Data;

        //
        // Perform the CRC on these four bytes.
        //
        ui16Crc = CRC16_ITER(ui16Crc, ui32Temp);
        ui16Crc = CRC16_ITER(ui16Crc, ui32Temp >> 8);
        ui16Crc = CRC16_ITER(ui16Crc, ui32Temp >> 16);
        ui16Crc = CRC16_ITER(ui16Crc, ui32Temp >> 24);

        //
        // Skip these input bytes.
        //
        pui8Data += 4;
        ui32Count -= 4;
    }

    //
    // If there are two bytes left in the input buffer, then perform two steps
    // of the CRC.
    //
    if(ui32Count > 1)
    {
        //
        // Read the two bytes.
        //
        ui32Temp = *(UINT16 *)pui8Data;

        //
        // Perform the CRC on these two bytes.
        //
        ui16Crc = CRC16_ITER(ui16Crc, ui32Temp);
        ui16Crc = CRC16_ITER(ui16Crc, ui32Temp >> 8);

        //
        // Skip these input bytes.
        //
        pui8Data += 2;
        ui32Count -= 2;
    }

    //
    // If there is a final byte remaining in the input buffer, then perform a
    // single step of the CRC.
    //
    if(ui32Count != 0)
    {
        ui16Crc = CRC16_ITER(ui16Crc, *pui8Data);
    }

    //
    // Return the resulting CRC-16 value.
    //
    return(ui16Crc);
}

