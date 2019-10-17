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



#define FOUR_BYTE_PARAM_FRAME_DATA_LEN 	(6 + 4 + DRIVE_BLOCK_CRC_DAT_LENGTH) // dest ID + source ID + data len + command ID + 16 bit param index + 4 byte data + crc
#define TWO_BYTE_PARAM_FRAME_DATA_LEN 	(6 + 2 + DRIVE_BLOCK_CRC_DAT_LENGTH) // dest ID + source ID + data len + command ID + 16 bit param index + 2 byte data + crc
#define ONE_BYTE_PARAM_FRAME_DATA_LEN 	(6 + 1 + DRIVE_BLOCK_CRC_DAT_LENGTH) // dest ID + source ID + data len + command ID + 16 bit param index + 1 byte data + crc

#define PARAM_INDEX_MSB_LOCATION	0
#define PARAM_INDEX_LSB_LOCATION 	1
#define PARAM_DATA_START_INDEX		2

#define POC 1
#define DEBUG_UART

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
#define EXPECTED_CRC_RUN_DRIVE					0xA093
#define EXPECTED_CRC_STOP_DRIVE 				0xA103  //A1D3?
#define EXPECTED_CRC_STOPPING_COMM 				0x6112
#define EXPECTED_CRC_START_INSTALL 				0xA353
#define EXPECTED_CRC_CONFIRM_SUBSTATE_INST 		0x6392

#define EXPECTED_CRC_OPEN_SHTER 				0x62D2
#define EXPECTED_CRC_OPEN_SHTER_JOG_10 			0xCD73
#define EXPECTED_CRC_OPEN_SHTER_JOG_50			0xCC33
#define EXPECTED_CRC_OPEN_SHTER_APER_HT 		0xA653

#define EXPECTED_CRC_CLOSE_SHTER 				0x6692
#define EXPECTED_CRC_CLOSE_SHTER_JOG_10			0x5D77
#define EXPECTED_CRC_CLOSE_SHTER_JOG_50			0x5C37
#define EXPECTED_CRC_CLOSE_SHTER_APER_HT 		0xA713
#define EXPECTED_CRC_CLOSE_SHTER_IGNORE_SENS 	0x6552

#define EXPECTED_CRC_STOP_SHUTTER				0xA593
#define EXPECTED_CRC_GET_PARAM					0xA4D3 // TBD - cannot generalize CRC for commands containing data
#define EXPECTED_CRC_SET_PARAM					0x6412 // TBD - cannot generalize CRC for commands containing data

#define EXPECTED_CRC_GET_ERROR_LIST				0x6C92  //0x11
#define EXPECTED_CRC_FIRMWARE_UPGRADE			0xBB93 // Command yet to be defined

#define EXPECTED_CRC_START_POWER_ON_CALIBRATION				0x7792
#define EXPECTED_CRC_STOP_POWER_ON_CALIBRATION				0x76D2
//Added by AOYAGI_ST 20160418 for clean error
#define EXPECTED_CRC_CLEAN_ERROR                0xB253
#define EXPECTED_CRC_APERTUREHEIGHT                0x7292

//CONST UINT32 drive_fw_version = 0x00000408;  //bug_NO.64
CONST UINT32 drive_fw_version = 17105;    //Drive version 1704.1        20170418   201703_No.29

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
enum _CBCommand
{
	run_drive = 1,
	stop_drive,
	stopping_drive_comm,
	start_install,
	confirm_sub_state_install,
	open_shutter,
	open_shutter_jog,
	open_shutter_aperture_height,
	close_shutter,
	close_shutter_jog,
	close_shutter_aperture_height,
	close_shutter_ignoring_sensors,
	stop_shutter,
	get_parameter,
	set_parameter,
	firmware_upgrade, // firmware upgrade and get error list are interchanged to sync with CB
    system_init_complete,
	get_error_list,
	//	Added on 03 FEB 2015 to implement user control on power up calibration
	start_power_on_calibraion = 0x35,
	stop_power_on_calibraion = 0x36,
    //Added by AOYAGI_ST 20160418 for clean error
    clean_error = 0x38,
    start_apertureHeight =0x39,
}enCBCommand; /* Used to identify command coming from CB */


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
const _stCBCommand constCBCommandList[NUM_OF_CONTROL_BOARD_COMMANDS] =
{
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, RUN_DRIVE_CMND_LEN, 			run_drive, 				{(EXPECTED_CRC_RUN_DRIVE & 0x00FF) , ((EXPECTED_CRC_RUN_DRIVE & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, STOP_DRIVE_CMND_LEN, 			stop_drive,				{(EXPECTED_CRC_STOP_DRIVE & 0x00FF) , ((EXPECTED_CRC_STOP_DRIVE & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, STOPPING_COMM_CMND_LEN, 		stopping_drive_comm,	{(EXPECTED_CRC_STOPPING_COMM & 0x00FF) , ((EXPECTED_CRC_STOPPING_COMM & 0xFF00) >> 8)}, no_cmnd },

	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, START_INSTALL_CMND_LEN, 			start_install, 				{(EXPECTED_CRC_START_INSTALL & 0x00FF) , ((EXPECTED_CRC_START_INSTALL & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, CONFIRM_SUBSTATE_INST_CMND_LEN,	confirm_sub_state_install,	{(EXPECTED_CRC_CONFIRM_SUBSTATE_INST & 0x00FF) , ((EXPECTED_CRC_CONFIRM_SUBSTATE_INST & 0xFF00) >> 8)}, no_cmnd },

	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, OPEN_SHTER_CMND_LEN, 			open_shutter, 					{(EXPECTED_CRC_OPEN_SHTER & 0x00FF) , ((EXPECTED_CRC_OPEN_SHTER & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, OPEN_SHTER_JOG_CMND_LEN, 		open_shutter_jog,				{(EXPECTED_CRC_OPEN_SHTER_JOG_10 & 0x00FF) , ((EXPECTED_CRC_OPEN_SHTER_JOG_10 & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, OPEN_SHTER_APER_HT_CMND_LEN, 	open_shutter_aperture_height,	{(EXPECTED_CRC_OPEN_SHTER_APER_HT & 0x00FF) , ((EXPECTED_CRC_OPEN_SHTER_APER_HT & 0xFF00) >> 8)}, no_cmnd },

	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, CLOSE_SHTER_CMND_LEN, 				close_shutter,					{(EXPECTED_CRC_CLOSE_SHTER & 0x00FF) , ((EXPECTED_CRC_CLOSE_SHTER & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, CLOSE_SHTER_JOG_CMND_LEN, 			close_shutter_jog,				{(EXPECTED_CRC_CLOSE_SHTER_JOG_10 & 0x00FF) , ((EXPECTED_CRC_CLOSE_SHTER_JOG_10 & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, CLOSE_SHTER_APER_HT_CMND_LEN, 		close_shutter_aperture_height,	{(EXPECTED_CRC_CLOSE_SHTER_APER_HT & 0x00FF) , ((EXPECTED_CRC_CLOSE_SHTER_APER_HT & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, CLOSE_SHTER_IGNORE_SENS_CMND_LEN, 	close_shutter_ignoring_sensors,	{(EXPECTED_CRC_CLOSE_SHTER_IGNORE_SENS & 0x00FF) , ((EXPECTED_CRC_CLOSE_SHTER_IGNORE_SENS & 0xFF00) >> 8)}, no_cmnd },

	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, STOP_SHUTTER_CMND_LEN, 	stop_shutter,		{(EXPECTED_CRC_STOP_SHUTTER & 0x00FF) , ((EXPECTED_CRC_STOP_SHUTTER & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, GET_PARAM_CMND_LEN, 		get_parameter,		{(EXPECTED_CRC_GET_PARAM & 0x00FF) , ((EXPECTED_CRC_GET_PARAM & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, SET_PARAM_CMND_LEN, 		set_parameter,		{(EXPECTED_CRC_SET_PARAM & 0x00FF) , ((EXPECTED_CRC_SET_PARAM & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, FIRMWARE_UPGRADE_CMND_LEN,	firmware_upgrade,	{(EXPECTED_CRC_FIRMWARE_UPGRADE & 0x00FF) , ((EXPECTED_CRC_FIRMWARE_UPGRADE & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, GET_ERROR_LIST_CMND_LEN, 	get_error_list,		{(EXPECTED_CRC_GET_ERROR_LIST & 0x00FF) , ((EXPECTED_CRC_GET_ERROR_LIST & 0xFF00) >> 8)}, no_cmnd },
	//	Added on 03 FEB 2015 to implement user control on power up calibration
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, START_POWER_ON_CALIBRATION_CMND_LEN,	start_power_on_calibraion,	{(EXPECTED_CRC_START_POWER_ON_CALIBRATION & 0x00FF) , ((EXPECTED_CRC_START_POWER_ON_CALIBRATION & 0xFF00) >> 8)}, no_cmnd },
	{DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, STOP_POWER_ON_CALIBRATION_CMND_LEN,	stop_power_on_calibraion,	{(EXPECTED_CRC_STOP_POWER_ON_CALIBRATION & 0x00FF) , ((EXPECTED_CRC_STOP_POWER_ON_CALIBRATION & 0xFF00) >> 8)}, no_cmnd },
    //Added by AOYAGI_ST 20160418 for adding clean error function
    {DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, CLEAN_ERROR_CMND_LEN,	clean_error,	{(EXPECTED_CRC_CLEAN_ERROR & 0x00FF) , ((EXPECTED_CRC_CLEAN_ERROR & 0xFF00) >> 8)}, no_cmnd },
    {DRIVE_BOARD_ADDRESS, CONTROL_BOARD_ADDRESS, CLEAN_APERTUREHEIGHT_CMND_LEN,	start_apertureHeight,	{(EXPECTED_CRC_APERTUREHEIGHT & 0x00FF) , ((EXPECTED_CRC_APERTUREHEIGHT & 0xFF00) >> 8)}, no_cmnd },

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

	// TBD - validate if we already read this command in the previous cycle, ->>>  this should not be done
	// we may receive back to back OPEN_SHUTTER_JOG for example


	// also validate if command is as expected wrt byte sequences, and it's CRC
	for(index = 0; index < NUM_OF_CONTROL_BOARD_COMMANDS; index++)
	{
		if(uCBCommand.stCBCommand.commandName == constCBCommandList[index].commandName)
		{
			bCommandFound = TRUE;

			if((uCBCommand.stCBCommand.sourceAddr == constCBCommandList[index].sourceAddr)
				&& (uCBCommand.stCBCommand.destAddr == constCBCommandList[index].destAddr)
				&& (uCBCommand.stCBCommand.dataLength == constCBCommandList[index].dataLength)
				&& ((uCBCommand.command[uCBCommand.stCBCommand.dataLength - 2]) == ((calculatedCRC & 0xFF00) >> 8)) // CRC MSB
				&& ((uCBCommand.command[uCBCommand.stCBCommand.dataLength - 1]) == (calculatedCRC & 0x00FF)) // CRC LSB
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
					uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveCommunicationFault.bits.crcError = TRUE;
					uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.crcCheckFailedCountA803++;
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
	UINT32 parameter = 0;
    UINT32 rdParameter = 0;
	UINT16 paramIndex = 0;
	BOOL result = FALSE;
	UINT8 byteCount = 0;
    BOOL faultTrgFlag = FALSE;
    UINT8 uart_cmd=0;
    BOOL flag_uart_cmd=FALSE;
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
    flag_uart_cmd=readCmndFromCommBuffer();
 #ifdef BUG_No82_UartRxTimeOut1S     //20170606  201703_No.82   
   if(flag_uart_cmd){Time_uart_count=0; LED_YELLOW=1; }             
   else if(Time_uart_count>=50){initCommandHandler();Time_uart_count=0; LED_YELLOW=0;}  
#endif    
    
    if((flag_uart_cmd)||((FLAG_CMD_open_shutter==1)&&(TIME_CMD_open_shutter==0)&&(TIME_CMD_close_shutter==0)))
    {
        if((new_cmd == uCBCommand.stCBCommand.recdCmdState)||((FLAG_CMD_open_shutter==1)&&(TIME_CMD_open_shutter==0)&&(TIME_CMD_close_shutter==0)))
        {
            status = ack;
            if(flag_uart_cmd==TRUE)uart_cmd=uCBCommand.stCBCommand.commandName;
            else uart_cmd=CMD_open_shutter;
            switch(uart_cmd)
            {
                case run_drive:
                    // This command is for future usage, no implementation as of now
                    status = nack;

                    break;

                case stop_drive:
                    // This command is for future usage, no implementation as of now
                    status = nack;

                    break;

                case stopping_drive_comm:
                        //This command is not required
                        status = nack;

                    break;

                case start_install:
                        startInstallation();
                        /*************add 20161017 start************************/
                            inputFlags.value = STOP_SHUTTER;
                            rampCurrentState = RAMP_STOP;
                            gui8StopKeyPressed = 1;
                            stopShutter();
                        /*************add 20161017 end************************/
                    break;
                case  start_apertureHeight:
                       startApertureHeight();
                       /*************add 20161017 start************************/
                            inputFlags.value = STOP_SHUTTER;
                            rampCurrentState = RAMP_STOP;
                            gui8StopKeyPressed = 1;
                            stopShutter();
                       /*************add 20161017 end************************/
                    break;
                case confirm_sub_state_install:
                    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
                    {
                        if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installA100 == TRUE)    /*******20160914 bug_No.99      start*********/
                        {
                            if(originSensorSts)
                               shutterInstall.enterCmdRcvd = TRUE;
                            else status = nack;
                        }
#ifdef BUG_NoCQ07_Limit_enterCmdRcvd    //20170627  201703_No.CQ07
                        else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveInstallationStatus.bits.installationValidation == TRUE)   
                            shutterInstall.enterCmdRcvd = FALSE;
                        else 
#else
                        else                                                                                                        /*******20160914 bug_No.99      end*********/                        
#endif                    
                          shutterInstall.enterCmdRcvd = TRUE;
                    }
                    else
                    {

                        status = nack;
                    }

                    break;

                case open_shutter:
                    //If is at upper limit then do not process command
                    if((TIME_CMD_open_shutter==0)&&(TIME_CMD_close_shutter==0))
                    {
                        FLAG_CMD_open_shutter=0;
                        if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady)
                        {
                            //instruct function for to not scan PE sensor - YG - Nov 2015
                            faultTrgFlag = readCurrSensorState(FALSE)
                                |uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.powerFail;
                            if(faultTrgFlag == FALSE)
                            {
                                //if((!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)&&(inputFlags.value!=OPEN_SHUTTER)) 20170318
								if((!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)&&(inputFlags.value!=OPEN_SHUTTER)&&(inputFlags.value!=OPEN_SHUTTER_JOG_10)&&(inputFlags.value!=OPEN_SHUTTER_JOG_50))
                                {
                                    inputFlags.value = OPEN_SHUTTER;
                                    rampStatusFlags.rampOpenInProgress = 0;   //20160906 bug_No.87
                                    TIME_CMD_open_shutter=100;
                                    //if shutter is moving then calculate min distance travel required.
                                    if(rampOutputStatus.shutterMoving)
                                    {
                                        calcShtrMinDistValue();
                                    }
    #ifdef DEBUG_SHUTTER_MOVEMENT_IN_WRONG_DIRECTION
                                    //	Clear false direction movement debug count
                                    gucTempFalseMovementCount = 0;
    #endif
                                }
                                else
                                {
                                    status = nack;
                                }
                            }
                            else
                            {
                                status = nack;
                            }
                        }
                        else
                        {
                            status = nack;
                        }
                    }
                    else {
                        FLAG_CMD_open_shutter=1;
                        CMD_open_shutter=open_shutter;
                    }
                    break;
                case open_shutter_jog:
                    //Check if we have received Jog or inch command
                    if(uCBCommand.stCBCommand.commandDataWithCRC[0] == ten_percent_jog)
                    {
                        //It is a inch command. It is valid during installation only
                        if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
                        {
                            //if((!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)&&(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveApertureHeight))
                            //     inputFlags.value = STOP_SHUTTER;
                            //else
                                inputFlags.value = OPEN_SHUTTER_JOG_10;
                        }
                        else
                        {
                            status = nack;
                        }
                    }
                    else if(uCBCommand.stCBCommand.commandDataWithCRC[0] == fifty_percent_jog)
                    {
                        //If we are at upper limit then do not process command
                        if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)
                        {
                            inputFlags.value = OPEN_SHUTTER_JOG_50;
                        }
                        else
                        {
                            status = nack;
                        }
                    }
                    else
                    {
                        //not valid command
                        status = nack;
                    }

                    break;
                case open_shutter_aperture_height:
                    //If shutter is at upper limit then do not process command
                    if((TIME_CMD_open_shutter==0)&&(TIME_CMD_close_shutter==0))
                    {
                        FLAG_CMD_open_shutter=0;
                            if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady)
                            {
                                //if(((uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit ||
                                   //uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterBetweenLowlmtAphgt))&&(inputFlags.value!=OPEN_SHUTTER_APERTURE))
							    //if((!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)&&(inputFlags.value!=OPEN_SHUTTER_APERTURE)) //20161202 //20170318
                                if((!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterUpperLimit)&&(inputFlags.value!=OPEN_SHUTTER_APERTURE)&&(inputFlags.value!=OPEN_SHUTTER_JOG_10)&&(inputFlags.value!=OPEN_SHUTTER_JOG_50)) 

                                {
                                    if(FLAG_StartApertureCorrection==1){FLAG_StartApertureCorrection++;inputFlags.value = OPEN_SHUTTER; }   //bug_No.12
                                    else if(FLAG_StartApertureCorrection>1){FLAG_StartApertureCorrection=0;inputFlags.value = OPEN_SHUTTER_APERTURE;}
                                    else inputFlags.value = OPEN_SHUTTER_APERTURE;
                                    TIME_CMD_open_shutter=100;
                                    bUpApertureCmdRecd = TRUE;
                                    //if shutter is moving then calculate min distance travel required.
                                    if(rampOutputStatus.shutterMoving)
                                    {
                                        calcShtrMinDistValue();
                                    }
                                }
                                else
                                {
                                    status = nack;
                                }
                            }
                            else
                            {
                                status = nack;
                            }
                    }
                    else {
                        FLAG_CMD_open_shutter=1;
                        CMD_open_shutter=open_shutter_aperture_height;
                    }
                    break;
                case close_shutter:
                case close_shutter_aperture_height:			// 2016/11/28 ADD close aperture height NG
                    //If shutter is at lower limit then do not process command
                    if((TIME_CMD_open_shutter==0)&&(TIME_CMD_close_shutter==0))
                    {
                        if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady)
                        {
                            //instruct function for to not scan PE sensor - YG - Nov 2015
                            faultTrgFlag = readCurrSensorState(TRUE)
                                |uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.powerFail;
                            if(faultTrgFlag == FALSE)
                            {
                                if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)
                                {
                                    inputFlags.value = CLOSE_SHUTTER;
                                    TIME_CMD_close_shutter=160;    //20160907  bug_No.107
                                    //if shutter is moving then calculate min distance travel required.
                                    if(rampOutputStatus.shutterMoving)
                                    {
                                        calcShtrMinDistValue();
                                    }
    #ifdef DEBUG_SHUTTER_MOVEMENT_IN_WRONG_DIRECTION
                                    //	Clear false direction movement debug count
                                    gucTempFalseMovementCount = 0;
    #endif
                                }
                                else
                                {
                                    status = nack;
                                }
                            }
                            else
                            {
                                status = nack;
                            }
                        }
                        else
                        {
                            status = nack;
                        }
                    }
                    else status =no_reply_reqd;
                    break;
                case close_shutter_jog:
                    //Check if we have received Jog or inch command
                    if(uCBCommand.stCBCommand.commandDataWithCRC[0] == ten_percent_jog)
                    {
                        //It is a inch command. It is valid during installation only
                        if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
                        {
                            //if((!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)&&(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveApertureHeight))
                           //      inputFlags.value = STOP_SHUTTER;
                           // else
                            inputFlags.value = CLOSE_SHUTTER_JOG_10;
                        }
                        else
                        {
                            status = nack;
                        }
                    }
                    else if(uCBCommand.stCBCommand.commandDataWithCRC[0] == fifty_percent_jog)
                    {
                        //If we are at upper limit then do not process command
                        if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)
                        {
                            inputFlags.value = CLOSE_SHUTTER_JOG_50;
                        }
                        else
                        {
                            status = nack;
                        }
                    }
                    else
                    {
                        //not valid command
                        status = nack;
                    }

                    break;
/* 2016/11/28 close aperture height NG
                case close_shutter_aperture_height:
                    FLAG_CMD_open_shutter=0;
                    //If shutter is at upper limit then do not process command
                    if((TIME_CMD_open_shutter==0)&&(TIME_CMD_close_shutter==0))
                    {
                            if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady)
                            {
                                //instruct function for to not scan PE sensor - YG - Nov 2015
                                faultTrgFlag = readCurrSensorState(TRUE)
                                    |uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.powerFail;

                                if(faultTrgFlag == FALSE)
                                {
                                    if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)
                                    {
                                        inputFlags.value = CLOSE_SHUTTER_APERTURE;
                                        TIME_CMD_close_shutter=160;    //20160907  bug_No.107
                                        bDownApertureCmdRecd = TRUE;
                                        //if shutter is moving then calculate min distance travel required.
                                        if(rampOutputStatus.shutterMoving)
                                        {
                                            calcShtrMinDistValue();
                                        }
                                    }
                                    else
                                    {
                                        status = nack;
                                    }
                                }
                                else
                                {
                                    status = nack;
                                }
                            }
                            else
                            {
                                status = nack;
                            }
                    }
                    else status =no_reply_reqd;
                    break;
*/
                case close_shutter_ignoring_sensors:

                    if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterLowerLimit)
                    {
                        inputFlags.value = CLOSE_SHUTTER;
                    }
                    else
                    {
                        status = nack;
                    }

                    break;

                case stop_shutter:
                    //if shutter is already stopped then do not process the command
                    //if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterStopped)
                    //{
                    //    inputFlags.value = STOP_SHUTTER;
                    //}
                    //else
                    //{
                    //    status = nack;
                    //}
                    FLAG_CMD_open_shutter=0;
                    inputFlags.value = STOP_SHUTTER;
                    //If drive is ready then travel min distance before stop
                    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady)
                    {
                        rampCurrentState = RAMP_STOP;
						//calcShtrStopDistValue();

						// Minimum distance travel before reverse action is required open key press during down operation and vice versa
						// The same is not required for stop operation as it increase delay while stoping action
						// For same 'calcShtrMinDistValue' is commented - YG - NOV 15
						//calcShtrMinDistValue();

						//	Set flag to indicate user has pressed stop key
						gui8StopKeyPressed = 1;
						stopShutter();
                    }

                    break;

                case get_parameter:
                        // Validation of parameter will take place, accordingly status can return with ACK or NACK
                        // NACK if i) Invalid parameter
                        //ii) Parameter read error from eeprom

                        paramIndex = (UINT16)(((UINT16)(uCBCommand.stCBCommand.commandDataWithCRC[PARAM_INDEX_MSB_LOCATION]) << 8)
                                        | (uCBCommand.stCBCommand.commandDataWithCRC[PARAM_INDEX_LSB_LOCATION]));

                        result = getParameter(paramIndex, &parameter, &byteCount);
                        if(!result)
                        {
                            status = nack;  // Invalid parameter
                            // TBD set appropriate error flag

                        }
                        else
                        {
                            //if shutter parameter A106, A107 and A108 are requested then calculate it from
                            //lower limit
                            if((paramIndex == 106) || (paramIndex == 107) || (paramIndex == 108))
                            {
                                parameter = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 - parameter;
                            }

                            if(paramIndex == 549)     //bug_NO.64
                            {
                                parameter  = drive_fw_version;
                            }
#ifdef BUG_No76or73_powerUpCalib_osToggle      //20170607  201703_No.76 or 73
                            if(paramIndex == 537)
                                        Flag_powerUpCalib_osToggle=1;                  
#endif                            
                           if((paramIndex == 605)&&(FLAG_StartApertureCorrection>0))
                            {
                               if((parameter&0x00000040)==0x00000040)
                               {
                                   parameter=parameter&0xFFFFFFBF;
                                   parameter=parameter|0x00000080;
                               }
                            }
                            if((paramIndex == 605)&&((parameter&0x00200008)==0x00200008))
                            {
                                  parameter=parameter&0xFFFFFFF7;
                            }
                            transmitParameter(parameter, paramIndex, byteCount);
                        }

                        //if it is a status poll then reset sensor status
                        if(paramIndex == 610)
                        {
                            if(updateSenStsCmd && !rampOutputStatus.shutterMoving)
                            {
                                resetSensorStatus();
                            }
                        }

                    break;

                case set_parameter:
                        // Validation of parameter will take place, accordingly status can return with ACK or NACK

                        // NACK conditions are:
                        // i) Invalid parameter
                        // ii) Parameter write error in eeprom
                        // iii) System is in 'Run' mode
                        if(!rampOutputStatus.shutterMoving)
                        {
                            paramIndex = (((UINT16)(uCBCommand.stCBCommand.commandDataWithCRC[PARAM_INDEX_MSB_LOCATION]) << 8)
                                            | (UINT16)(uCBCommand.stCBCommand.commandDataWithCRC[PARAM_INDEX_LSB_LOCATION]));

                            if(FOUR_BYTE_PARAM_FRAME_DATA_LEN == uCBCommand.stCBCommand.dataLength)
                            {
                                parameter = (UINT32)(((UINT32)(uCBCommand.stCBCommand.commandDataWithCRC[PARAM_DATA_START_INDEX]) << 24)
                                            | ((UINT32)(uCBCommand.stCBCommand.commandDataWithCRC[PARAM_DATA_START_INDEX+1]) << 16)
                                            | ((UINT32)(uCBCommand.stCBCommand.commandDataWithCRC[PARAM_DATA_START_INDEX+2]) << 8)
                                            | ((UINT32)(uCBCommand.stCBCommand.commandDataWithCRC[PARAM_DATA_START_INDEX+3]))
                                            );
                            }
                            else if(TWO_BYTE_PARAM_FRAME_DATA_LEN == uCBCommand.stCBCommand.dataLength)
                            {
                                parameter = (UINT16) (((UINT16)(uCBCommand.stCBCommand.commandDataWithCRC[PARAM_DATA_START_INDEX]) << 8)
                                            | (uCBCommand.stCBCommand.commandDataWithCRC[PARAM_DATA_START_INDEX+1])
                                            );
                            }
                            else if(ONE_BYTE_PARAM_FRAME_DATA_LEN == uCBCommand.stCBCommand.dataLength)
                            {
                                parameter = (UINT8)(uCBCommand.stCBCommand.commandDataWithCRC[PARAM_DATA_START_INDEX]);
                            }
                            else // data length is incorrect
                            {
                                status = nack;
                                // TBD set appropriate error flag - none defined, upto CB to recover

                                break;
                            }

                            //if shutter parameter A106, A107 and A108 are requested then calculate it from
                            //lower limit
                            if((paramIndex == 106) || (paramIndex == 107) || (paramIndex == 108))
                            {
                                parameter = uDriveCommonBlockEEP.stEEPDriveCommonBlock.lowerStoppingPos_A101 - parameter;
                            }

                            //Read the parameter and if the value is changed then only update in EEPROM
                            result = getParameter(paramIndex, &rdParameter, &byteCount);
                            if(rdParameter != parameter)
                            {
                                result = setParameter(paramIndex, parameter);
                            }
                            else
                            {
                                result = TRUE;
                            }

                            if(!result)
                            {
                                status = nack;  // Invalid parameter
                                // TBD set appropriate error flag - none defined, upto CB to recover

                            }
                            else
                            {
                                status = ack; // ACK to be sent to CB

                                //check if shutter type is changed if shutter type is changed then reset all parameters
                                if(currShutterType != uEEPDriveMotorCtrlBlock.stEEPDriveMotorCtrlBlock.shutterType_A537)
								{
                                    uDriveApplBlockEEP.stEEPDriveApplBlock.initialValSetting_A021 = 1;
								}

                                //if snow mode time is changed then update sensor debounce time
                                if(paramIndex == 8)
                                {
                                    updatePhotoElectricDebounceTime();
                                }
                            }
                        }
                        else
                        {
                            status = nack; // shutter is moving
                        }
                    break;

                case firmware_upgrade:
                        // Validation of parameter will take place, accordingly status can return with ACK or NACK
                    break;

                case get_error_list:
                        // Validation of parameter will take place, accordingly status can return with ACK or NACK
                        // NACK if i) Empty error list
                        //ii) Read error from eeprom
                        txAnomalyHistInProgress = TRUE;

                    break;

                case system_init_complete:
                        //This command is received when display and control board initialization gets completed
                        //Set the flag to set system initialization completed
                        sysInitCompleted = TRUE;
                    break;

				//	Added on 03 FEB 2015 to implement user control on power up calibration
				case start_power_on_calibraion:
					//	Set global flag to indicate power on calibration is initiated by user from display board
                    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration ||
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration)
                    {
                                        powerOnCalibration = INITIATED;
#ifdef BUG_No76or73_powerUpCalib_osToggle      //20170607  201703_No.76
                                        if(Flag_powerUpCalib_osToggle==1)
                                        {
                                           Flag_powerUpCalib_osToggle=0;
                                           powerUpCalib.osToggle=0;  
                                        }                  
#endif                                        
                    }
                    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
                    {
                        ShutterInstallationEnabled = TRUE;
                        //inputFlags.value = OPEN_SHUTTER_JOG_50;
                        //shutterInstall.currentState = INSTALL_SEARCH_ORG;

                        inputFlags.value = inputFlags_Installation.value;
                    }
                    break;

				//	Added on 03 FEB 2015 to implement user control on power up calibration
				case stop_power_on_calibraion:
					//	Clear global flag to indicate power on calibration is cancelled by user from display board
					//if shutter is already stopped then do not process the command
                    //if(!uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.shutterStopped)
                    //{
                    //    inputFlags.value = STOP_SHUTTER;
                    //}
                    //else
                    //{
                    //    status = nack;
                    //}

                                    inputFlags.value = STOP_SHUTTER;
                    //If drive is ready then travel min distance before stop
                    //if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveReady)
                    //{
                    //    calcShtrMinDistValue();
                    //}

					//	Set global flag to indicate power on calibration is terminated by user from display board
                    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.drivePowerOnCalibration ||
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveRuntimeCalibration)
                    {
                                    powerOnCalibration = TERMINATED;
                    }
                    if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveStatus.bits.driveInstallation)
                    {
                        ShutterInstallationEnabled = FALSE;
                    }
                    break;
//added by AOYAGI_ST 20160418 for adding clean error function
                case clean_error:
                    if(!rampOutputStatus.shutterMoving)
                    {
                        if((uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorStall)||
                           (uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorExceedingTorque)||
                           (uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorSusOC)||
                           (uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorPWMCosting)||

                           (uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osNotDetectUP)||           //bug_No.101  20160909
                           (uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osNotDetectDown)||
                           (uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osDetectOnUp)||
                           (uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osDetectOnDown)||
                           (uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osFailValidation))
                        {
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorStall = FALSE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorExceedingTorque = FALSE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorSusOC = FALSE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorPWMCosting = FALSE;

                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osNotDetectUP = FALSE;      //bug_No.101  20160909
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osNotDetectDown = FALSE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osDetectOnUp = FALSE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osDetectOnDown = FALSE;
                            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.osFailValidation = FALSE;
                            //initSensorList();
                            //initApplication();
                            //initRampGenerator();
                            //startRampGenerator();
                            //transmitRestartComm();
                            status = ack;
                        }
                        else
                        {
                            status = nack;
                        }
                    }
                    /*if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorStall)
                    {
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorStall = FALSE;
                    }
                    else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorExceedingTorque)
                    {
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorExceedingTorque = FALSE;
                    }
                    else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorSusOC)
                    {
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorSusOC = FALSE;
                    }
                    else if(uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorPWMCosting)
                    {
                        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorPWMCosting = FALSE;
                    }*/
                    else
                    {
                        status = nack;
                    }
                    break;
                default:
                        // could not recognise the command, hence reply with NACK - code is not expected to come here
                        status = nack; // NACK to be sent to CB
                    break;
            }


            // mark the command read and processing complete
            uCBCommand.stCBCommand.recdCmdState = complete;

            // TBD - since the command processing is completed, we can now erase the same? - not necc,
            // as of now commented as this is handy for debugging
            //memcpy((UINT8 *)&uCBCommand.command[0], '\0', (COMMAND_RX_BUFFER_SIZE + CMD_STATE_SIZE));

            if(ack == status)	// transmit ACK
            {
                transmitACK(TRUE);
            }
            else if(nack == status) // transmit NACK
            {
                transmitACK(FALSE);
            }

            //check parameter reset
            checkResetParameter();
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

#ifdef BUG_No41_microSwSensorTrigrd     //20170606  201703_No.41 
    if(microSwSensorTrigrd && scanPE_Sensor == TRUE)      
#else    
    if(microSwSensorTrigrd)
#endif 
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.microSwitch = TRUE;
        sts = TRUE;
    }


#ifdef BUG_No79_FaultPeObstacle     //20170608  201703_No.79
    if(photoElecObsSensTrigrd && scanPE_Sensor == TRUE && (rampCurrentPosition < uDriveCommonBlockEEP.stEEPDriveCommonBlock.photoElecPosMonitor_A102))    
#else
    if(photoElecObsSensTrigrd && scanPE_Sensor == TRUE)    
#endif
    {
        uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveApplicationFault.bits.peObstacle = TRUE;
        sts = TRUE;
    }

	// When the regenerative resistance dose not work,the voltage. By IME 2016/12/14
	//if(tempSensTrigrd)
    if(tempSensTrigrd||(gucOverVoltageFailFlag>MAXIMUM_DURATION))
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
	UINT16 crc = 0;
	UINT8 buffer[RESTART_DRIVE_COMM_CMND_LEN];

	buffer[0] = CONTROL_BOARD_ADDRESS;
	buffer[1] = DRIVE_BOARD_ADDRESS;
	buffer[2] = RESTART_DRIVE_COMM_CMND_LEN;
	buffer[3] = RESTART_DRIVE_COMM_COMMAND;

	crc = crc16((UINT8*)&buffer[0], (RESTART_DRIVE_COMM_CMND_LEN - DRIVE_BLOCK_CRC_DAT_LENGTH), CRC_SEED);
	buffer[4] = (UINT8)(crc); // lsb
	buffer[5] = (UINT8)(crc >> 8); // msb

	uartSendTxBuffer (APPL_ASSIGNED_UART_CHANNEL, buffer, RESTART_DRIVE_COMM_CMND_LEN);
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

