/*********************************************************************************
* FileName: CMDr.c
* Description:
* This source file contains the definition of all the services of ...
* Version: 0.1D
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


/****************************************************************************************************************************************************
 *  Modification History
 *
 *  Revision		Date                  Name          			Comments
 *  	0.5D	23/07/2014			iGATE Offshore team			Command response type check added
 *  	0.4D	03/07/2014			iGATE Offshore team			Poll parameter numbers updated as per new parameter number list
 *  	0.3D	13/06/2014			iGATE Offshore team			ResponseStatus == eNo_Status check added to avoid retransmission of command
 *  	0.2D	19/06/2014			iGATE Offshore team			Packet length changed from data length to complete frame length
 *  	0.1D	21/05/2014      	iGATE Offshore team			Initial Creation
****************************************************************************************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdint.h>
#include <string.h>
#include <inc/hw_memmap.h>
#include <driverlib/sw_crc.h>
#include <driverlib/rom.h>
#include <Drivers/ustdlib.h>
#include "cmdr.h"
#include "cmdi.h"
#include "interTaskCommunication.h"
#include "Middleware/serial.h"
#include "Drivers/systicktimer.h"
/****************************************************************************/

/****************************************************************************
 *  extern:
****************************************************************************/
extern uint8_t guchTxBuffer [UART_CHANNEL_COUNT] [TRANSMIT_BUFFER_SIZE];		// Transmit Buffer Array
extern uint8_t guchTxBufferByteCount [UART_CHANNEL_COUNT];					// Length of the buffer to be transmitted
extern uint8_t guchTxBufferIndex [UART_CHANNEL_COUNT];						// Index to the current byte in transmission
extern uint8_t guchUARTMapToGlobalBuffer[MAX_UART_AVAILABLE];					// Index buffer
extern uint8_t guchRxBuffer [UART_CHANNEL_COUNT] [RECEIVE_BUFFER_SIZE];		// Receive Buffer Array
extern uint8_t guchRxBufferByteCount [UART_CHANNEL_COUNT];					// Number of bytes received

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define PARAMETER_NO
#define CMDr_MODULE_WAIT_TIME	2000	// 2sec @ 1mS Systick

#define WAIT_BEFORE_SENDING_NEXT_COMMAND	10	// 10mS @ 1mS Systick

/****************************************************************************/

/****************************************************************************
 *  enums for this file:
****************************************************************************/
enum commandResponseType
{
	eResponseACK_NACK = 0,
	eResponseString,
};

enum functionState
{
	eCaptureTime = 0,
	eWait,
	eExecute,
};

/****************************************************************************/

/****************************************************************************
 * Local variables for this file
****************************************************************************/



/****************************************************************************/

/****************************************************************************
 *  Global variables for other files:
****************************************************************************/
// Control to drive communication error occurrence count
uint8_t guchCMDrCRC_ErrorOccurrence[UART_CHANNEL_COUNT];


uint32_t get_timego(uint32_t x_data_his);
/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
****************************************************************************/

_CMDrInnerTaskComm lstCMDrInnerTaskComm;

/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
****************************************************************************/

/******************************************************************************
 * insertCommandIDandData
 *
 * Function Description:
 * This function identifies command ID based upon active bit in CMDr inner command list
 * and inserts  that command ID and command data (if any) into the command frame to be
 * sent to the drive board.
 *
 * Function Parameter: void
 *
 * Function Returns: command ID
 *
 ********************************************************************************/

enum commandResponseType insertCommandIDandData (uint8_t * commandBuffer, uint8_t * wrIndex);

/******************************************************************************
 * formCMDrCommandPacket
 *
 * Function Description:
 * This function forms a frame of command to the drive board.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

enum commandResponseType formCMDrCommandPacket (void);

/****************************************************************************/


/******************************************************************************
 * initCMDrGlobalRegisters
 *
 * Function Description:
 * Initialize global registers being used for CMDr inner task communication.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void initCMDrGlobalRegisters(void)
{
	lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;
	lstCMDrInnerTaskComm.commandType = RAW;
	lstCMDrInnerTaskComm.commandToDriveBoard.val = 0;
	lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
	lstCMDrInnerTaskComm.commandResponseACK_Status = eNO_StatusAcknowledgement;
}



/******************************************************************************
 * insertCommandIDandData
 *
 * Function Description:
 * This function identifies command ID based upon active bit in CMDr inner command list
 * and inserts  that command ID and command data (if any) into the command frame to be
 * sent to the drive board.
 *
 * Function Parameter: void
 *
 * Function Returns: command ID
 *
 ********************************************************************************/

enum commandResponseType insertCommandIDandData (uint8_t * commandBuffer, uint8_t * wrIndex)
{
	enum commandResponseType leCommandResponseType = eResponseACK_NACK;

	union ui16TempData lunWord16;
	union ui32TempData lunWord32;

	switch(lstCMDrInnerTaskComm.commandToDriveBoard.val)
	{
	case 0x01:
		*commandBuffer = RUN_DRIVE;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x02:
		*commandBuffer = STOP_DRIVE;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x04:
		*commandBuffer = STOPPING_DRIVE_COMMUNICATION;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x08:
		*commandBuffer = START_INSTALLATION_CMD_FROM_CONTROL;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x10:
		*commandBuffer = CONFIRM_SUBSTATE_INSTALLATION;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x20:
		*commandBuffer = OPEN_SHUTTER;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x40:
		*commandBuffer = OPEN_SHUTTER_JOG;
		commandBuffer++;
		*commandBuffer = lstCMDrInnerTaskComm.additionalCommandData;
		commandBuffer++;
		*wrIndex+=1;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x80:
		*commandBuffer = OPEN_SHUTTER_APERTURE;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x100:
		*commandBuffer = CLOSE_SHUTTER;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x200:
		*commandBuffer = CLOSE_SHUTTER_JOG;
		commandBuffer++;
		*commandBuffer = lstCMDrInnerTaskComm.additionalCommandData;
		commandBuffer++;
		*wrIndex+=1;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x400:
		*commandBuffer = CLOSE_SHUTTER_APERTURE;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x800:
		*commandBuffer = CLOSE_SHUTTER_IGNORE_SENSOR;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x1000:
		*commandBuffer = STOP_SHUTTER;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x2000:
		*commandBuffer = GET_PARAMETER_CMD_FROM_CONTROL;
		commandBuffer++;
		/*
		*commandBuffer++ = (lstCMDrInnerTaskComm.parameterNumber & 0xFF00) >> 8;
		*wrIndex+=1;
		*commandBuffer++ = lstCMDrInnerTaskComm.parameterNumber & 0xFF;
		*wrIndex+=1;
		*/

		lunWord16.halfWord.val = lstCMDrInnerTaskComm.parameterNumber;

		*commandBuffer = lunWord16.byte[1];
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = lunWord16.byte[0];
		*wrIndex+=1;
		leCommandResponseType = eResponseString;
		break;
	case 0x4000:
		*commandBuffer = SET_PARAMETER_CMD_FROM_CONTROL;
		commandBuffer++;
		/*
		*commandBuffer++ = (lstCMDrInnerTaskComm.parameterNumber & 0xFF00) >> 8;
		*wrIndex+=1;
		*commandBuffer++ = lstCMDrInnerTaskComm.parameterNumber & 0xFF;
		*wrIndex+=1;
		*commandBuffer++ = (lstCMDrInnerTaskComm.parameterValue & 0xFF000000) >> 24;
		*wrIndex+=1;
		*commandBuffer++ = (lstCMDrInnerTaskComm.parameterValue & 0xFF0000) >> 16;
		*wrIndex+=1;
		*commandBuffer++ = (lstCMDrInnerTaskComm.parameterValue & 0xFF00) >> 8;
		*wrIndex+=1;
		*commandBuffer++ = lstCMDrInnerTaskComm.parameterValue & 0xFF;
		*wrIndex+=1;
		*/

		lunWord16.halfWord.val = lstCMDrInnerTaskComm.parameterNumber;

		*commandBuffer = lunWord16.byte[1];
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = lunWord16.byte[0];
		commandBuffer++;
		*wrIndex+=1;

		lunWord32.word.val = lstCMDrInnerTaskComm.parameterValue;

		*commandBuffer = lunWord32.byte[3];
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = lunWord32.byte[2];
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = lunWord32.byte[1];
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = lunWord32.byte[0];
		*wrIndex+=1;

		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x8000:
		*commandBuffer = GET_ERROR_LIST_CMD_FROM_CONTROL;
		leCommandResponseType = eResponseString;
		break;
	case 0x10000:
		*commandBuffer = START_POWER_ON_CALIBRATION;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x20000:
		*commandBuffer = STOP_POWER_ON_CALIBRATION;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x40000:
		*commandBuffer = RECOVER_ANMOLY;
		leCommandResponseType = eResponseACK_NACK;
		break;
	default:
		*commandBuffer = INVALID_COMMAND_ID;
		break;
	}
	return leCommandResponseType;
//	lstCMDrInnerTaskComm.commandToDriveBoard.val = 0x0;
//	lstCMDrInnerTaskComm.additionalCommandData = 0x0;
}

/******************************************************************************
 * formCMDrCommandPacket
 *
 * Function Description:
 * This function forms a frame of command to the drive board.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

enum commandResponseType formCMDrCommandPacket (void)
{
	uint8_t frameWrIndex = 0;
	uint8_t commandPacket[TRANSMIT_BUFFER_SIZE];
	uint16_t lucCRCvalue;

	union ui16TempData lunWord16;

	enum commandResponseType leCommandResponseType = eResponseACK_NACK;

	// Clear drive UART transmit buffer
//	memset(commandPacket,'\0',sizeof(commandPacket));

	// Clear drive UART transmit byte count

	// Initialize transmit buffer index to zero

	/*Start Packet formation*/

	// Insert destination address
	frameWrIndex = DESTINATION_ADDRESS_POSITION;
	commandPacket [frameWrIndex] = DRIVE_BOARD_ADDRESS;

	// Insert source address
	frameWrIndex = SOURCE_ADDRESS_POSITION;
	commandPacket [frameWrIndex] = CONTROL_BOARD_ADDRESS;

	// Insert command ID
	frameWrIndex = COMMAND_ID_POSITION;
	leCommandResponseType = insertCommandIDandData(&commandPacket [frameWrIndex], &frameWrIndex);


	// Insert command length including CRC bytes
	commandPacket[FRAME_LENGTH_POSITION] = frameWrIndex + 3;

	// Compute CRC and insert in frame
	lucCRCvalue = Crc16(0,commandPacket,commandPacket [FRAME_LENGTH_POSITION]-2/*+1*/);
	/*
	commandPacket [++frameWrIndex] = (lucCRCvalue & 0xFF00) >> 8;
	commandPacket [++frameWrIndex] = lucCRCvalue & 0xFF;
	 */
	lunWord16.halfWord.val = lucCRCvalue;

	commandPacket [++frameWrIndex] = lunWord16.byte[1];
	commandPacket [++frameWrIndex] = lunWord16.byte[0];

	// Assign frame length to global transmit buffer byte count


	// Send first byte of command frame to initiate transmit interrupt
	uartSendTxBuffer(UART_drive,commandPacket,commandPacket[FRAME_LENGTH_POSITION]);

	// Return command response type
	return leCommandResponseType;
}

void disable_move_cyw()
{
	if(gstControlApplicationFault.bits.operationRestrictionTimer == 1)
	{
	lstCMDrInnerTaskComm.commandToDriveBoard.bits.openShutter = 0;
	lstCMDrInnerTaskComm.commandToDriveBoard.bits.openShutterJog = 0;
	lstCMDrInnerTaskComm.commandToDriveBoard.bits.openShutterApperture = 0;
	lstCMDrInnerTaskComm.commandToDriveBoard.bits.closeShutter = 0;
	lstCMDrInnerTaskComm.commandToDriveBoard.bits.closeShutterJog = 0;
	lstCMDrInnerTaskComm.commandToDriveBoard.bits.closeShutterApperture = 0;
	}
}

/******************************************************************************
 * handleDriveCmdResp
 *
 * Function Description:
 * This function handles sending command to the drive board and receiving
 * command response from the drive board.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void handleDriveCmdResp(void)
{
	static uint8_t sucCommandSendAttempts = 0;

	static uint8_t sucTxPendingBytes = 0;

	static uint8_t sucaResponseReceiveBuffer[RECEIVE_BUFFER_SIZE];
	static uint16_t sucRxCRC = 0;
	static uint16_t sucCalcCRC = 0;
	static uint8_t sucRxIndex = 0;
	static uint8_t sucRxCount = 0;

	static uint32_t suiWaitBeforeSendingNextCommand = 0;

	static enum commandResponseType sleCommandResponseType = eResponseACK_NACK;

	uint8_t lucTemp = 0;
	uint8_t lucDummyRxByte = 0;

	static uint8_t sucRxDataLength = 0;

//	static uint32_t luiTxInstance = 0;

	static uint32_t luiRespInstance = 0;

	enum functionState
	{
		eIdle,
		eSendingCommand,
		eWaitingForResp,
		eWaitBeforeSendingNextCommand
	};
	static enum functionState lucCmdAndRespState = eIdle;

	switch(lucCmdAndRespState)
	{
	case eIdle:
		if(lstCMDrInnerTaskComm.commandRequestStatus == eACTIVE &&
				lstCMDrInnerTaskComm.commandResponseStatus == eNO_STATUS)/* &&
				lstCMDrInnerTaskComm.commandResponseACK_Status == eNO_StatusAcknowledgement)*/
		{
			if(lstCMDrInnerTaskComm.commandType != DIRECT)		// Command is not from CMDi
			{
				//
				// Flush global receive data buffer
				//
				if(!uartCheckNotFreeRxBuffer(UART_drive,&lucTemp))
				{
					while(lucTemp)	// Bytes present in global receive buffer
					{
						uartGetRxBuffer(UART_drive,&lucDummyRxByte);
						uartCheckNotFreeRxBuffer(UART_drive,&lucTemp);
					}
				}
				//
				// Form packet and send command
				//

				disable_move_cyw();
				sleCommandResponseType = formCMDrCommandPacket();
			}
			else
			{
				//
				// Flush global receive data buffer
				//
				if(!uartCheckNotFreeRxBuffer(UART_drive,&lucTemp))
				{
					while(lucTemp)
					{
						uartGetRxBuffer(UART_drive,&lucDummyRxByte);
						uartCheckNotFreeRxBuffer(UART_drive,&lucTemp);
					}
				}
				// Set response type as string or ACK_NACK
				if(lstCMDrInnerTaskComm.uchTxBuffer[COMMAND_ID_POSITION] == GET_PARAMETER_CMD_FROM_CONTROL)
				{
					sleCommandResponseType = eResponseString;
				}
				else if(
						(lstCMDrInnerTaskComm.uchTxBuffer[COMMAND_ID_POSITION] == SET_PARAMETER_CMD_FROM_CONTROL) ||
						(lstCMDrInnerTaskComm.uchTxBuffer[COMMAND_ID_POSITION] == FIRMWARE_UPGRADECMD_FROM_DISPLAY)
						)
				{
					sleCommandResponseType = eResponseACK_NACK;
				}
				//
				// Pass through the command received from CMDi
				//
				uartSendTxBuffer(UART_drive,lstCMDrInnerTaskComm.uchTxBuffer,lstCMDrInnerTaskComm.uchTxBufferLen);
			}
			// Start local timer for command send timeout
//			luiTxInstance = g_ui32TickCount;

			lucCmdAndRespState = eSendingCommand;

			// Reset receive buffer write index
			sucRxIndex = 0;
			sucRxCount = 0;
		}
		else if(lstCMDrInnerTaskComm.commandResponseACK_Status == eResponseAcknowledgement_ACK)
		{
			// Form packet and send ACK
			lucTemp = ACK;
			uartSendTxBuffer(UART_drive,&lucTemp,1);
			lstCMDrInnerTaskComm.commandResponseACK_Status = eResponseAcknowledgementProcessed;
		}
		else if(lstCMDrInnerTaskComm.commandResponseACK_Status == eResponseAcknowledgement_NACK)
		{
			// Form packet and send ACK
			lucTemp = NACK;
			uartSendTxBuffer(UART_drive,&lucTemp,1);
			lstCMDrInnerTaskComm.commandResponseACK_Status = eResponseAcknowledgementProcessed;
		}
		break;
	case eSendingCommand:
		uartCheckFreeTxBuffer(UART_drive, &sucTxPendingBytes);
		if(!sucTxPendingBytes)	// Command sent completely
		{
			sucCommandSendAttempts++;
			lucCmdAndRespState = eWaitingForResp;

			// Start local timer for response timeout
			luiRespInstance = g_ui32TickCount;
		}
/*
		else
		{
			if(g_ui32TickCount - luiTxInstance >= TxTIME_OUT)	// Command transmit timeout
			{
				sucCommandSendAttempts++;
				if(sucCommandSendAttempts == MAX_CMD_SEND_ATTEMPTS)	// Maximum attempts to send command reached
				{
					lstCMDrInnerTaskComm.commandResponseStatus = eCMD_SEND_FAIL;		// Command send failed
					sucCommandSendAttempts = 0;
				}
				lucCmdAndRespState = eIdle;		// Reset function state to idle
			}
		}
*/

		break;
	case eWaitingForResp:
		if(get_timego( luiRespInstance) >= COMMAND_RESPONSE_RxTIME_OUT)	// Command response timeout
		{
			lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
//			sucCommandSendAttempts++;
			if(sucCommandSendAttempts == MAX_CMD_SEND_ATTEMPTS)	// Maximum attempts to send command reached
			{
				lstCMDrInnerTaskComm.commandResponseStatus = eTIME_OUT;		// Command response timeout
				sucCommandSendAttempts = 0;
			}
			//
			//	Wait before sending next command so that recipient will become ready to receive.
			//	Added on 18 Sep 2014
			//
			//lucCmdAndRespState = eIdle;
			suiWaitBeforeSendingNextCommand = g_ui32TickCount;
			lucCmdAndRespState = eWaitBeforeSendingNextCommand;
		}	// Command response timeout
		else
		{	// Capture command response
			//
			//	Check if data is available in the global receive buffer
			//
			if(!uartCheckNotFreeRxBuffer(UART_drive,&lucTemp))
			{	// Response data available in global buffer
				while(lucTemp)	// Data available
				{
					uint8_t lucGetRxBuffData = 0; // It is used to hold the fetched byte from circular buffer
					
					//
					//	Copy data available in global receive buffer into local buffer
					//
					//uartGetRxBuffer(UART_drive,&sucaResponseReceiveBuffer[sucRxIndex++]);
					uartGetRxBuffer(UART_drive,&lucGetRxBuffData);
					
					// Validate first byte of packet depending on reply type
					if (
							(
							(sucRxCount != 0) ||
							(
									sucRxCount == 0 &&
									(
											(sleCommandResponseType == eResponseACK_NACK && (lucGetRxBuffData == ACK || lucGetRxBuffData == NACK)) ||
											(sleCommandResponseType == eResponseString && lucGetRxBuffData == CONTROL_BOARD_ADDRESS)
									)
							)
							) &&
							(sucRxCount < RECEIVE_BUFFER_SIZE)
					   )
					{

						// copy local fetched data in static buffer
						sucaResponseReceiveBuffer[sucRxIndex++] = lucGetRxBuffData;

						//
						//	Increment receive byte count
						//
						sucRxCount++;

						//
						// Compute data length
						//
						if(sucRxIndex == FRAME_LENGTH_POSITION + 1)
						{
							//
							//	Copy received data byte length into local buffer
							//
							sucRxDataLength = sucaResponseReceiveBuffer[FRAME_LENGTH_POSITION];

							//validate length possible min and max length. If not found correct then capturing the packet from start
							if (sucRxDataLength < MIN_COMMAND_RESPONSE_LENGTH || sucRxDataLength > MAX_COMMAND_RESPONSE_LENGTH)
							{
								sucRxCount = 0;
								sucRxIndex = 0;
								sucRxDataLength = 0;
							}
						}

					} // Validate first byte of packet depending on reply type

					//
					//	Check whether the next byte is available
					//
					uartCheckNotFreeRxBuffer(UART_drive,&lucTemp);
				}
			}

			if(
					(sucRxCount) &&
					(
							sucRxCount == sucRxDataLength/* + 3*/ ||	//	Complete frame (Data length + one byte each of destination ID, source ID and length byte)
							sucaResponseReceiveBuffer[0] == ACK   ||	//	ACK received
							sucaResponseReceiveBuffer[0] == NACK
					)

			   )					//	NACK received
			{
				sucCalcCRC = 0;
				
				if((sucRxCount > 1) && (sucRxCount == sucRxDataLength/* + 3*/))
				{
					//
					// if response is not ACK or NACK then
					// Validate packet with CRC check
					//
					sucRxCRC = sucaResponseReceiveBuffer[sucRxCount-2];
					sucRxCRC <<= 8;
					sucRxCRC |= sucaResponseReceiveBuffer[sucRxCount-1];

					sucCalcCRC = Crc16(0,sucaResponseReceiveBuffer,sucRxCount-2);

					//
					//	TO bypass CRC validation from drive board
					//
//					sucCalcCRC = sucRxCRC;
				}

				if(sucRxCRC == sucCalcCRC || sucaResponseReceiveBuffer[0] == ACK || sucaResponseReceiveBuffer[0] == NACK)
				{
					//
					// CRC passed or ACK/NACK
					// Copy received data from local buffer into inner task manager response buffer
					//
					memcpy((void*)lstCMDrInnerTaskComm.uchRxBuffer,(const void*)sucaResponseReceiveBuffer,sucRxCount);
					//
					//	Update number of bytes received into inner task manager
					//
					lstCMDrInnerTaskComm.uchRxBufferLen = sucRxCount;

					//
					//	Command successful
					//
					lstCMDrInnerTaskComm.commandResponseStatus = eSUCCESS;

					//
					//	Reset command send attempts
					//
					sucCommandSendAttempts = 0;

					//
					//	Reset local receive buffer count
					//
					sucRxCount = 0;

					//
					//	Wait before sending next command so that recipient will become ready to receive.
					//	Added on 18 Sep 2014
					//
					//lucCmdAndRespState = eIdle;
					suiWaitBeforeSendingNextCommand = g_ui32TickCount;
					lucCmdAndRespState = eWaitBeforeSendingNextCommand;
				}	// CRC passed or ACK/NACK
				else
				{	// CRC failed
					guchCMDrCRC_ErrorOccurrence[guchUARTMapToGlobalBuffer[UART_drive]] = 1;
					if(sucCommandSendAttempts == MAX_CMD_SEND_ATTEMPTS)	// Maximum attempts to send command reached
					{
						lstCMDrInnerTaskComm.commandResponseStatus = eFAIL;		// Command response CRC failed
						sucCommandSendAttempts = 0;
					}
					else
					{
						lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
					}

					//
					//	Wait before sending next command so that recipient will become ready to receive.
					//	Added on 18 Sep 2014
					//
					//lucCmdAndRespState = eIdle;
					suiWaitBeforeSendingNextCommand = g_ui32TickCount;
					lucCmdAndRespState = eWaitBeforeSendingNextCommand;
				}
			}	// Complete response frame received or ACK/NACK received
		}
		break;
		//
		//	Wait before sending next command so that recipient will become ready to receive.
		//	Added on 18 Sep 2014
		//
	case eWaitBeforeSendingNextCommand:
		if(get_timego( suiWaitBeforeSendingNextCommand) >= WAIT_BEFORE_SENDING_NEXT_COMMAND)
		{
			lucCmdAndRespState = eIdle;
		}
		break;
	default:
		break;
	}	// switch(lucCmdAndRespState)
}	// handleDriveCmdResp


/******************************************************************************
 * pollDriveStatusFault
 *
 * Function Description:
 * This function will Poll the Status Register of the Drive
 * If the Installation and/or Fault bit in Status Register is Set, then function will poll the Sub Installation and Fault Register.
 * Function will use 'handleDriveCmdResp' to send physical command and to process the reply
 * Function will store the reply in respective Global Intertask Communication Structure
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void pollDriveStatusFault(void)
{
	static uint32_t suiStatusFaultPollTimer = 0;

	union ui16TempData lunWord16;
	union ui32TempData lunWord32;

	enum functionState
		{
			eStartStatusPollTimer = 0,
			eStartedStatusPollTimer,
			eSendStatusPollCmd,
			eSentStatusPollCmdWaitForReply,
			eSendFaultPollCmd,
			eSentFaultCmdWaitForReply,
			eSendCommFaultPollCmd,
			eSentCommFaultPollCmdWaitForReply,
			eSendAppFaultPollCmd,
			eSentAppFaultPollCmdWaitForReply,
			eSendMotorFaultPollCmd,
			eSentMotorFaultPollCmdWaitForReply,
			eSendProcessorFaultPollCmd,
			eSentProcessorFaultPollCmdWaitForReply,
			eSendInstalltionPollCmd,
			eSentInstalltionPollCmdWaitForReply,
		};

	static unsigned char sucPollDriveStatusFaultState = eStartStatusPollTimer;

	switch(sucPollDriveStatusFaultState)
	{

	case eStartStatusPollTimer:

		suiStatusFaultPollTimer = g_ui32TickCount;
		sucPollDriveStatusFaultState = eStartedStatusPollTimer;

		break;

	case eStartedStatusPollTimer:

		if(get_timego( suiStatusFaultPollTimer) >= STATUS_FAULT_POLL_TIMEOUT)
		{

			sucPollDriveStatusFaultState = eSendStatusPollCmd;

		} // if(suiStatusFaultPollTimeElapsed - suiStatusFaultPollTimer >= STATUS_FAULT_POLL_TIMEOUT)

		break;

	case eSendStatusPollCmd:

		if(
				(gstLStoCMDr.commandRequestStatus == eINACTIVE) &&
				(gstEMtoCMDr.commandRequestStatus == eINACTIVE) &&
				(gstCMDitoCMDr.commandRequestStatus == eINACTIVE) &&
				(lstCMDrInnerTaskComm.commandRequestStatus == eINACTIVE)
		  )
		{

			lstCMDrInnerTaskComm.commandType = RAW;
			lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 1;
#ifdef PARAMETER_NO
			lstCMDrInnerTaskComm.parameterNumber = DRIVE_STATUS;	//24;	//605;				// A605: Drive board status register
#endif

#ifndef PARAMETER_NO
			lstCMDrInnerTaskComm.parameterNumber = A605_DRIVE_BOARD_STATUS	;	//24;	//605;				// A605: Drive board status register
#endif
			lstCMDrInnerTaskComm.commandRequestStatus = eACTIVE;

			sucPollDriveStatusFaultState = eSentStatusPollCmdWaitForReply;

		}

		break;

	case eSentStatusPollCmdWaitForReply:

		if (lstCMDrInnerTaskComm.commandResponseStatus == eSUCCESS)
		{

			/*lui16Temp = lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 1];
			lui16Temp = lui16Temp << 8;
			lui16Temp = lui16Temp | lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 2];*/
			lunWord32.byte[3] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION];
			lunWord32.byte[2] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 1];
			lunWord32.byte[1] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 2];
			lunWord32.byte[0] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];

			gstDriveStatus.val = lunWord32.word.val; 			// Update global drive status declared in control board

			gstDriveStatusMenu.bits.Upper_Limit_Reached_Status = gstDriveStatus.bits.shutterUpperLimit;
			gstDriveStatusMenu.bits.Lower_Limit_Reached_Status = gstDriveStatus.bits.shutterLowerLimit;

			lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 0;

			//
			//	Clear control-drive communication error flag
			//
			if(gstControlCommunicationFault.bits.commFailDrive == 1)
			{
				gstControlCommunicationFault.bits.commFailDrive = 0;
			}

			//
			//	Clear control board CRC error flag
			//
			if(gstControlCommunicationFault.bits.crcErrorDrive == 1)
			{
				gstControlCommunicationFault.bits.crcErrorDrive = 0;
			}

			if(0 == gstControlCommunicationFault.val)
			{
				gstControlBoardFault.bits.controlCommunication = 0;
			}

			if(0 == gstControlBoardFault.val)
			{
				gstControlBoardStatus.bits.controlFault = 0;
			}

			// Set parameter for drive status menu
			if (gstDriveStatus.bits.upDecelStartReached == 1)
			{
				gstDriveStatusMenu.bits.Upper_Deceleration_Strt_Pt_Reach_Status = 1;
			}
			else
			{
				gstDriveStatusMenu.bits.Upper_Deceleration_Strt_Pt_Reach_Status = 0;
			}

			// Set parameter for drive status menu
			if (gstDriveStatus.bits.ignPhotoElecSensLimRchd == 1)
			{
				gstDriveStatusMenu.bits.Ignore_PE_Limit_Reached_Status = 1;
			}
			else
			{
				gstDriveStatusMenu.bits.Ignore_PE_Limit_Reached_Status = 0;
			}

			// Set parameter for drive status menu
			if (gstDriveStatus.bits.microSwitchSensorStatus == 1)
			{
				gstDriveStatusMenu.bits.Mocro_Switch_Status = 1;
			}
			else
			{
				gstDriveStatusMenu.bits.Mocro_Switch_Status = 0;
			}

			// Set parameter for drive status menu
			if (gstDriveStatus.bits.originSensorStatus == 1)
			{
				gstDriveStatusMenu.bits.Origin_Status = 1;
			}
			else
			{
				gstDriveStatusMenu.bits.Origin_Status = 0;
			}

			// Set parameter for drive status menu
			if (gstDriveStatus.bits.peSensorStatus == 1)
			{
				gstDriveStatusMenu.bits.PE_Sensor_Status = 1;
			}
			else
			{
				gstDriveStatusMenu.bits.PE_Sensor_Status = 0;
			}

			sucPollDriveStatusFaultState = eSendFaultPollCmd;

		}

		if (lstCMDrInnerTaskComm.commandResponseStatus == eTIME_OUT || lstCMDrInnerTaskComm.commandResponseStatus == eFAIL)
		{

			//
			//	Control board status as fault
			//
			gstControlBoardStatus.bits.controlFault = 1;


			//
			//	Control board communication fault
			//
			gstControlBoardFault.bits.controlCommunication = 1;

			if (lstCMDrInnerTaskComm.commandResponseStatus == eTIME_OUT)
			{
				//
				//	Drive communication failed
				//
				gstControlCommunicationFault.bits.commFailDrive = 1;
			}
			else
			{
				//
				//	CRC error in response from drive
				//
				gstControlCommunicationFault.bits.crcErrorDrive = 1;
			}

			lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 0;

			sucPollDriveStatusFaultState = eStartStatusPollTimer;
		}

		break;

	case eSendFaultPollCmd:

		if (
				(gstDriveStatus.bits.driveFault != 0) ||
				(gstDriveStatus.bits.driveFaultUnrecoverable != 0)
		)
		{

			if(
				(gstLStoCMDr.commandRequestStatus == eINACTIVE) &&
				(gstEMtoCMDr.commandRequestStatus == eINACTIVE) &&
				(gstCMDitoCMDr.commandRequestStatus == eINACTIVE) &&
				(lstCMDrInnerTaskComm.commandRequestStatus == eINACTIVE)
			  )
			{

				lstCMDrInnerTaskComm.commandType = RAW;
				lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 1;
#ifdef PARAMETER_NO
				lstCMDrInnerTaskComm.parameterNumber = DRIVE_FAULT_STATUS;		// A616: Drive fault
#endif

#ifndef PARAMETER_NO
				lstCMDrInnerTaskComm.parameterNumber = A607_DRIVE_FAULT;		// A616: Drive fault
#endif
				lstCMDrInnerTaskComm.commandRequestStatus = eACTIVE;

				sucPollDriveStatusFaultState = eSentFaultCmdWaitForReply;

			}

		}
		else
		{

			sucPollDriveStatusFaultState = eSendInstalltionPollCmd;
			gstDriveBoardFault.val = 0;											// Clear global drive fault declared in control board
			gstDriveApplicationFault.val = 0;
			gstDriveCommunicationFault.val = 0;
			gstDriveMotorFault.val = 0;
			gstDriveProcessorfault.val = 0;

			// Set drive status menu parameter
			gstDriveStatusMenu.bits.Motor_Thermal_Input_Status = 0;
		}

		break;

	case eSentFaultCmdWaitForReply:

		if (lstCMDrInnerTaskComm.commandResponseStatus == eSUCCESS)
		{

			gstDriveBoardFault.val = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3]; 							// Update global drive fault declared in control board

			lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 0;

			//
			//	Clear control board communication error flag
			//
			if(gstControlBoardFault.bits.controlCommunication == 1)
			{
				gstControlBoardFault.bits.controlCommunication = 0;
			}
			//
			//	Clear control-drive communication error flag
			//
			if(gstControlCommunicationFault.bits.commFailDrive == 1)
			{
				gstControlCommunicationFault.bits.commFailDrive = 0;
			}
			//
			//	Clear control board CRC error flag
			//
			if(gstControlCommunicationFault.bits.crcErrorDrive == 1)
			{
				gstControlCommunicationFault.bits.crcErrorDrive = 0;
			}

			if(gstControlCommunicationFault.bits.uartErrorDrive == 1)
			{
				gstControlCommunicationFault.bits.uartErrorDrive = 0;
			}

			if(0 == gstControlCommunicationFault.val)
			{
				gstControlBoardFault.bits.controlCommunication = 0;
			}

			if(0 == gstControlBoardFault.val)
			{
				gstControlBoardStatus.bits.controlFault = 0;
			}

			sucPollDriveStatusFaultState = eSendCommFaultPollCmd;

		}

		if (lstCMDrInnerTaskComm.commandResponseStatus == eTIME_OUT || lstCMDrInnerTaskComm.commandResponseStatus == eFAIL)
		{

			//
			//	Control board communication fault
			//
			gstControlBoardFault.bits.controlCommunication = 1;

			if (lstCMDrInnerTaskComm.commandResponseStatus == eTIME_OUT)
			{
				//
				//	Drive communication failed
				//
				gstControlCommunicationFault.bits.commFailDrive = 1;
			}
			else
			{
				//
				//	CRC error in response from drive
				//
				gstControlCommunicationFault.bits.crcErrorDrive = 1;
			}

			lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 0;

			sucPollDriveStatusFaultState = eStartStatusPollTimer;

		}

		break;

	case eSendCommFaultPollCmd:
	case eSendAppFaultPollCmd:
	case eSendMotorFaultPollCmd:
	case eSendProcessorFaultPollCmd:

		if (
				(sucPollDriveStatusFaultState == eSendCommFaultPollCmd && gstDriveBoardFault.bits.driveCommunication == 1) ||
				(sucPollDriveStatusFaultState == eSendAppFaultPollCmd && gstDriveBoardFault.bits.driveApplication == 1) ||
				(sucPollDriveStatusFaultState == eSendMotorFaultPollCmd && gstDriveBoardFault.bits.driveMotor == 1) ||
				(sucPollDriveStatusFaultState == eSendProcessorFaultPollCmd && gstDriveBoardFault.bits.driveProcessor == 1)
	       )
		{

			if(
				(gstLStoCMDr.commandRequestStatus == eINACTIVE) &&
				(gstEMtoCMDr.commandRequestStatus == eINACTIVE) &&
				(gstCMDitoCMDr.commandRequestStatus == eINACTIVE) &&
				(lstCMDrInnerTaskComm.commandRequestStatus == eINACTIVE)
			  )
			{

				lstCMDrInnerTaskComm.commandType = RAW;
				lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 1;

					if (sucPollDriveStatusFaultState == eSendCommFaultPollCmd)
					{
#ifdef PARAMETER_NO
						lstCMDrInnerTaskComm.parameterNumber = DRIVE_COMMUNICATON_FAULT_STATUS;		// A606: Drive communication error
#endif

#ifndef PARAMETER_NO
						lstCMDrInnerTaskComm.parameterNumber = A608_DRIVE_BOARD_COMM_FAULT;		// A606: Drive communication error
#endif
						sucPollDriveStatusFaultState = eSentCommFaultPollCmdWaitForReply;
					}
					else if (sucPollDriveStatusFaultState == eSendAppFaultPollCmd)
					{
#ifdef PARAMETER_NO
						lstCMDrInnerTaskComm.parameterNumber = DRIVE_APPLICATION_FAULT_STATUS;		// A608: Drive application error
#endif

#ifndef PARAMETER_NO
						lstCMDrInnerTaskComm.parameterNumber = A610_DRIVE_APPL_FAULT;		// A608: Drive application error
#endif
						sucPollDriveStatusFaultState = eSentAppFaultPollCmdWaitForReply;
					}
					else if (sucPollDriveStatusFaultState == eSendMotorFaultPollCmd)
					{
#ifdef PARAMETER_NO
						lstCMDrInnerTaskComm.parameterNumber = DRIVE_MOTOR_FAULT_STATUS;		// A607: Drive motor error
#endif

#ifndef PARAMETER_NO
						lstCMDrInnerTaskComm.parameterNumber = A609_DRIVE_MOTOR_FAULT;		// A607: Drive motor error
#endif
						sucPollDriveStatusFaultState = eSentMotorFaultPollCmdWaitForReply;
					}
					else if (sucPollDriveStatusFaultState == eSendProcessorFaultPollCmd)
					{
#ifdef PARAMETER_NO
						lstCMDrInnerTaskComm.parameterNumber = DRIVE_PROCESSOR_FAULT_STATUS;		// A609: Drive processor error
#endif

#ifndef PARAMETER_NO
						lstCMDrInnerTaskComm.parameterNumber = A611_DRIVE_PROCESSOR_FAULT;		// A609: Drive processor error
#endif
						sucPollDriveStatusFaultState = eSentProcessorFaultPollCmdWaitForReply;
					}

				lstCMDrInnerTaskComm.commandRequestStatus = eACTIVE;

			}

		}
		else
		{

			if (sucPollDriveStatusFaultState == eSendCommFaultPollCmd)
			{
				sucPollDriveStatusFaultState = eSendAppFaultPollCmd;
				gstDriveCommunicationFault.val = 0;											// Clear global drive fault declared in control board
			}
			else if (sucPollDriveStatusFaultState == eSendAppFaultPollCmd)
			{
				sucPollDriveStatusFaultState = eSendMotorFaultPollCmd;
				gstDriveApplicationFault.val = 0;											// Clear global drive fault declared in control board
			}
			else if (sucPollDriveStatusFaultState == eSendMotorFaultPollCmd)
			{
				sucPollDriveStatusFaultState = eSendProcessorFaultPollCmd;
				gstDriveMotorFault.val = 0;													// Clear global drive fault declared in control board
			}
			else if (sucPollDriveStatusFaultState == eSendProcessorFaultPollCmd)
			{
				sucPollDriveStatusFaultState = eSendInstalltionPollCmd;
				gstDriveProcessorfault.val = 0;												// Clear global drive fault declared in control board
			}

		}

		break;

	case eSentCommFaultPollCmdWaitForReply:
	case eSentAppFaultPollCmdWaitForReply:
	case eSentMotorFaultPollCmdWaitForReply:
	case eSentProcessorFaultPollCmdWaitForReply:

		if (lstCMDrInnerTaskComm.commandResponseStatus == eSUCCESS)
		{

			if (sucPollDriveStatusFaultState == eSentCommFaultPollCmdWaitForReply)
			{
				gstDriveCommunicationFault.val = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];		// Update global drive fault declared in control board
				sucPollDriveStatusFaultState = eSendAppFaultPollCmd;
			}
			else if (sucPollDriveStatusFaultState == eSentAppFaultPollCmdWaitForReply)
			{
				/*lui32Temp = lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 1];
				lui32Temp = lui32Temp << 8;
				lui32Temp = lui32Temp | lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 2];
				lui32Temp = lui32Temp << 8;
				lui32Temp = lui32Temp | lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 3];
				lui32Temp = lui32Temp << 8;
				lui32Temp = lui32Temp | lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 4];*/
				lunWord32.byte[3] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION];
				lunWord32.byte[2] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 1];
				lunWord32.byte[1] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 2];
				lunWord32.byte[0] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];

				gstDriveApplicationFault.val = lunWord32.word.val; 							// Update global drive fault declared in control board

				//
				//	Clear control board communication error flag
				//
				if(gstControlBoardFault.bits.controlCommunication == 1)
				{
					gstControlBoardFault.bits.controlCommunication = 0;
				}
				//
				//	Clear control-drive communication error flag
				//
				if(gstControlCommunicationFault.bits.commFailDrive == 1)
				{
					gstControlCommunicationFault.bits.commFailDrive = 0;
				}
				//
				//	Clear control board CRC error flag
				//
				if(gstControlCommunicationFault.bits.crcErrorDrive == 1)
				{
					gstControlCommunicationFault.bits.crcErrorDrive = 0;
				}

				if(gstControlCommunicationFault.bits.uartErrorDrive == 1)
				{
					gstControlCommunicationFault.bits.uartErrorDrive = 0;
				}
				if(0 == gstControlCommunicationFault.val)
				{
					gstControlBoardFault.bits.controlCommunication = 0;
				}

				if(0 == gstControlBoardFault.val)
				{
					gstControlBoardStatus.bits.controlFault = 0;
				}
				sucPollDriveStatusFaultState = eSendMotorFaultPollCmd;
			}
			else if (sucPollDriveStatusFaultState == eSentMotorFaultPollCmdWaitForReply)
			{
				/*lui16Temp = lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 1];
				lui16Temp = lui16Temp << 8;
				lui16Temp = lui16Temp | lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 2];*/
				lunWord16.byte[1] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 2];
				lunWord16.byte[0] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];
				gstDriveMotorFault.val = lunWord16.halfWord.val; 			// Update global drive status declared in control board

				// Set parameter for drive status menu
				if (gstDriveMotorFault.bits.motorOverheat == 1)
				{
				gstDriveStatusMenu.bits.Motor_Thermal_Input_Status = 1;
				}
				else
				{
				gstDriveStatusMenu.bits.Motor_Thermal_Input_Status = 0;
				}

				sucPollDriveStatusFaultState = eSendProcessorFaultPollCmd;
			}
			else if (sucPollDriveStatusFaultState == eSentProcessorFaultPollCmdWaitForReply)
			{
				/*lui16Temp = lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 1];
				lui16Temp = lui16Temp << 8;
				lui16Temp = lui16Temp | lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 2];*/
				lunWord16.byte[1] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 2];
				lunWord16.byte[0] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];
				gstDriveProcessorfault.val = lunWord16.halfWord.val; 			// Update global drive status declared in control board

				sucPollDriveStatusFaultState = eSendInstalltionPollCmd;
			}

			lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 0;

		}

		if (lstCMDrInnerTaskComm.commandResponseStatus == eTIME_OUT || lstCMDrInnerTaskComm.commandResponseStatus == eFAIL)
		{

			//
			//	Control board communication fault
			//
			gstControlBoardFault.bits.controlCommunication = 1;
			if (lstCMDrInnerTaskComm.commandResponseStatus == eTIME_OUT)
			{
				//
				//	Drive communication failed
				//
				gstControlCommunicationFault.bits.commFailDrive = 1;
			}
			else
			{
				//
				//	CRC error in response from drive
				//
				gstControlCommunicationFault.bits.crcErrorDrive = 1;
			}

			lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 0;

			sucPollDriveStatusFaultState = eStartStatusPollTimer;

		}

		break;

	case eSendInstalltionPollCmd:

		if (gstDriveStatus.bits.driveInstallation != 0)
		{

			if(
					(gstLStoCMDr.commandRequestStatus == eINACTIVE) &&
					(gstEMtoCMDr.commandRequestStatus == eINACTIVE) &&
					(gstCMDitoCMDr.commandRequestStatus == eINACTIVE) &&
					(lstCMDrInnerTaskComm.commandRequestStatus == eINACTIVE)
			  )
			{

				lstCMDrInnerTaskComm.commandType = RAW;
				lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 1;
#ifdef PARAMETER_NO
				lstCMDrInnerTaskComm.parameterNumber = DRIVE_INSTALLATION_STATUS;		// Drive installation
#endif

#ifndef PARAMETER_NO
				lstCMDrInnerTaskComm.parameterNumber = A606_DRIVE_INSTALLATION;		// Drive installation
#endif
				lstCMDrInnerTaskComm.commandRequestStatus = eACTIVE;

				sucPollDriveStatusFaultState = eSentInstalltionPollCmdWaitForReply;

			}

		}
		else
		{

			sucPollDriveStatusFaultState = eStartStatusPollTimer;
			gstDriveInstallation.val = 0;											// Clear global drive fault declared in control board

		}

		break;


	case eSentInstalltionPollCmdWaitForReply:

		if (lstCMDrInnerTaskComm.commandResponseStatus == eSUCCESS)
		{

			/*lui16Temp = lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 1];
			lui16Temp = lui16Temp << 8;
			lui16Temp = lui16Temp | lstCMDrInnerTaskComm.uchRxBuffer[COMMAND_ID + 2];*/
			lunWord16.byte[1] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 2];
			lunWord16.byte[0] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];

			gstDriveInstallation.val = lunWord16.halfWord.val; 			// Update global drive status declared in control board

			lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 0;

			//
			//	Clear control board communication error flag
			//
			if(gstControlBoardFault.bits.controlCommunication == 1)
			{
				gstControlBoardFault.bits.controlCommunication = 0;
			}
			//
			//	Clear control-drive communication error flag
			//
			if(gstControlCommunicationFault.bits.commFailDrive == 1)
			{
				gstControlCommunicationFault.bits.commFailDrive = 0;
			}
			//
			//	Clear control board CRC error flag
			//
			if(gstControlCommunicationFault.bits.crcErrorDrive == 1)
			{
				gstControlCommunicationFault.bits.crcErrorDrive = 0;
			}

			if(gstControlCommunicationFault.bits.uartErrorDrive == 1)
			{
				gstControlCommunicationFault.bits.uartErrorDrive = 0;
			}

			if(0 == gstControlCommunicationFault.val)
			{
				gstControlBoardFault.bits.controlCommunication = 0;
			}

			if(0 == gstControlBoardFault.val)
			{
				gstControlBoardStatus.bits.controlFault = 0;
			}

			sucPollDriveStatusFaultState = eStartStatusPollTimer;

		}

		if (lstCMDrInnerTaskComm.commandResponseStatus == eTIME_OUT || lstCMDrInnerTaskComm.commandResponseStatus == eFAIL)
		{

			//
			//	Control board communication fault
			//
			gstControlBoardFault.bits.controlCommunication = 1;
			if (lstCMDrInnerTaskComm.commandResponseStatus == eTIME_OUT)
			{
				//
				//	Drive communication failed
				//
				gstControlCommunicationFault.bits.commFailDrive = 1;
			}
			else
			{
				//
				//	CRC error in response from drive
				//
				gstControlCommunicationFault.bits.crcErrorDrive = 1;
			}

			lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 0;

			sucPollDriveStatusFaultState = eStartStatusPollTimer;

		}

		break;


	default:

		break;

	} // switch(sucPollDriveStatusFaultState)

} // pollDriveStatusFault


/******************************************************************************
 * handleCommandFromLS_EM_CMDi
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 *
 * Function Returns: void
 *
 *
 ********************************************************************************/
void handleCommandFromLS_EM_CMDi(void)
{

	enum functionState
		{
			eIdleSendOperationCmd = 0,
			eSentOperationalCmdWaitingReply,
			eWaitingOperationalCmdReplyAck,
			eSentOperationalCmdReplyAckWaitingReply,
		};

	static unsigned char sucHandleCommandLS_EM_CMDiState = eIdleSendOperationCmd;

	unsigned char lucTemp;

	unsigned char lucCommandSupportReponseAck;

	union ui32TempData lunWord32;

	switch(sucHandleCommandLS_EM_CMDiState)
	{

	case eIdleSendOperationCmd: // Send command through 'handleDriveCmdResp'
		if(
				(
						(gstLStoCMDr.commandRequestStatus == eACTIVE &&
								gstLStoCMDr.commandResponseStatus == eNO_STATUS) ||
						(gstEMtoCMDr.commandRequestStatus == eACTIVE &&
						gstEMtoCMDr.commandResponseStatus == eNO_STATUS) ||
						(gstCMDitoCMDr.commandRequestStatus == eACTIVE &&
								gstCMDitoCMDr.commandResponseStatus == eNO_STATUS)
				) &&
				(lstCMDrInnerTaskComm.commandRequestStatus == eINACTIVE)
		)
		{

			if(gstLStoCMDr.commandRequestStatus == eACTIVE)
			{
				// Set command as a Raw, So command formation is required by 'handleDriveCmdResp'
				lstCMDrInnerTaskComm.commandType = RAW;

				// Read command request from 'Logic solver' and request to 'handleDriveCmdResp' to send same
				if (gstLStoCMDr.commandToDriveBoard.bits.runDrive)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.runDrive = 1;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.stopDrive)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.stopDrive = 1;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.stoppingDriveCommunication)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.stoppingDriveCommunication = 1;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.startInstallation)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.startInstallation = 1;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.confirmSubstateInstallation)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.confirmSubstateInstallation = 1;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.openShutter)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.openShutter = 1;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.openShutterJog)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.openShutterJog = 1;
					lstCMDrInnerTaskComm.additionalCommandData = gstLStoCMDr.additionalCommandData;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.openShutterApperture)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.openShutterApperture = 1;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.closeShutter)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.closeShutter = 1;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.closeShutterJog)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.closeShutterJog = 1;
					lstCMDrInnerTaskComm.additionalCommandData = gstLStoCMDr.additionalCommandData;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.closeShutterApperture)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.closeShutterApperture = 1;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.closeShutterIgnoreSensor)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.closeShutterIgnoreSensor = 1;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.stopShutter)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.stopShutter = 1;
				}
				else if (gstLStoCMDr.commandToDriveBoard.bits.getOperationCount)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter = 1;
					lstCMDrInnerTaskComm.parameterNumber = OPERATION_COUNT;
				}
				else if(gstLStoCMDr.commandToDriveBoard.bits.startPowerOnCalibration)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.startPowerOnCalibration = 1;
				}
				else if(gstLStoCMDr.commandToDriveBoard.bits.stopPowerOnCalibration)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.stopPowerOnCalibration = 1;
				}
				//	Added to implement setParameter command for setting snow mode, on 10APR2015
				else if(gstLStoCMDr.commandToDriveBoard.bits.setParameter)
				{
					lstCMDrInnerTaskComm.commandToDriveBoard.bits.setParameter = 1;
					lstCMDrInnerTaskComm.parameterNumber = gstLStoCMDr.dataToDriveBoard.parameterNumber;
					lstCMDrInnerTaskComm.parameterValue = gstLStoCMDr.dataToDriveBoard.commandData.setParameterValue;
				}

				gstLStoCMDr.commandResponseStatus = eWAITING;

			}
			else if(gstEMtoCMDr.commandRequestStatus == eACTIVE)
			{
				// Set command as a Raw, So command formation is required by 'handleDriveCmdResp'
				lstCMDrInnerTaskComm.commandType = RAW;

				lstCMDrInnerTaskComm.commandToDriveBoard.bits.getError = 1;

				gstEMtoCMDr.commandResponseStatus = eWAITING;

			}
			else if(gstCMDitoCMDr.commandRequestStatus == eACTIVE)
			{
				// Set command as a Direct, no command formation required by 'handleDriveCmdResp'
				lstCMDrInnerTaskComm.commandType = DIRECT;

				for (lucTemp = 0;lucTemp <gstCMDitoCMDr.transmitCommandPacketLen;lucTemp++)
				{
					lstCMDrInnerTaskComm.uchTxBuffer[lucTemp] = gstCMDitoCMDr.transmitCommandPacket[lucTemp];
				}
				lstCMDrInnerTaskComm.uchTxBufferLen = gstCMDitoCMDr.transmitCommandPacketLen;

				gstCMDitoCMDr.commandResponseStatus = eWAITING;

			}

			lstCMDrInnerTaskComm.commandRequestStatus = eACTIVE; // inform 'handleDriveCmdResp' about command

			sucHandleCommandLS_EM_CMDiState = eSentOperationalCmdWaitingReply; // go to next state for processing the response

		} //if(
		  //    (gstLStoCMDr.commandRequestStatus == eACTIVE || gstEMtoCMDr.commandRequestStatus == eACTIVE || gstCMDitoCMDr.commandRequestStatus == eACTIVE) &&
		  //    (lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE)
	      //  )

		break; //case eIdleSendOperationCmd: // Send command through 'handleDriveCmdResp'

	case eSentOperationalCmdWaitingReply: // Process reply from 'handleDriveCmdResp'

		if (lstCMDrInnerTaskComm.commandResponseStatus >= eSUCCESS && lstCMDrInnerTaskComm.commandResponseStatus <= eFAIL)
		{

			if (gstLStoCMDr.commandResponseStatus == eWAITING)
			{
				// process the reply from 'handleDriveCmdResp' and pass it to 'Logic Solver'
				if(lstCMDrInnerTaskComm.commandToDriveBoard.bits.getParameter == 0)
				{
					if (lstCMDrInnerTaskComm.uchRxBuffer[0] == ACK)
					{
						gstLStoCMDr.acknowledgementReceived = eACK;
					}
					else
					{
						gstLStoCMDr.acknowledgementReceived = eNACK;
					}
				}
				else	// Copy operation count value
				{
					if (lstCMDrInnerTaskComm.commandResponseStatus == eSUCCESS)
					{
						lunWord32.byte[3] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION];
						lunWord32.byte[2] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 1];
						lunWord32.byte[1] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 2];
						lunWord32.byte[0] = lstCMDrInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];

						gstLStoCMDr.getParameterValue = lunWord32.word.val;
					}
				}

				gstLStoCMDr.commandResponseStatus = lstCMDrInnerTaskComm.commandResponseStatus;

				lucCommandSupportReponseAck = 0;

			}
			else if(gstEMtoCMDr.commandResponseStatus == eWAITING)
			{
				// process the reply from 'handleDriveCmdResp' and pass it to 'Error Module' (Copying anomaly history pending)
				gstEMtoCMDr.commandResponseStatus = lstCMDrInnerTaskComm.commandResponseStatus;


				lucCommandSupportReponseAck = 1;

			}
			else if(gstCMDitoCMDr.commandResponseStatus == eWAITING)
			{
				// process the reply from 'handleDriveCmdResp' and pass it to 'CMDi'
				for (lucTemp = 0;lucTemp <lstCMDrInnerTaskComm.uchRxBufferLen;lucTemp++)
				{
					gstCMDitoCMDr.receiveCommandPacket[lucTemp] = lstCMDrInnerTaskComm.uchRxBuffer[lucTemp];
				}
				gstCMDitoCMDr.receiveCommandPacketLen = lstCMDrInnerTaskComm.uchRxBufferLen;

				gstCMDitoCMDr.commandResponseStatus = lstCMDrInnerTaskComm.commandResponseStatus;

				lucCommandSupportReponseAck = 0;

			}

			// Reset the state machine, if command not support response ack or command response fail or timeout
			if (
					(lstCMDrInnerTaskComm.commandResponseStatus >= eTIME_OUT && lstCMDrInnerTaskComm.commandResponseStatus <= eFAIL) ||
					(lucCommandSupportReponseAck == 0)
			   )
			{

				lstCMDrInnerTaskComm.commandToDriveBoard.val = 0;

				lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
				lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;

				sucHandleCommandLS_EM_CMDiState = eIdleSendOperationCmd; // Command processing over, now reset the state machine

			} // Reset the state machine, if command not support response ack or command response fail or timeout
			else
			{

				sucHandleCommandLS_EM_CMDiState = eWaitingOperationalCmdReplyAck; // Command reply require an acknowledgment. Go next state to wait and process same

				/*****************************************************************************/
				/*added later as getError command was getting fired twice*/
//				lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;
				/*****************************************************************************/

			}

		} //if (lstCMDrInnerTaskComm.commandResponseStatus >= eSUCCESS && lstCMDrInnerTaskComm.commandResponseStatus <= eFAIL)

		break; //case eSentOperationalCmdWaitingReply: // Process reply from 'handleDriveCmdResp'

	case eWaitingOperationalCmdReplyAck: // Wait for command response acknowledgment

		if (gstEMtoCMDr.commandResponseACK_Status == eResponseAcknowledgement_ACK)
		{

			lstCMDrInnerTaskComm.commandResponseACK_Status = eResponseAcknowledgement_ACK;

			/*****************************************************************************/
			/*added later as getError command was getting fired twice*/
			gstEMtoCMDr.commandResponseStatus = eNO_STATUS;
			/*****************************************************************************/

			sucHandleCommandLS_EM_CMDiState = eSentOperationalCmdReplyAckWaitingReply; // Sent operational command reply ack. Now wait for the reply

		}
		else if (gstEMtoCMDr.commandResponseACK_Status == eResponseAcknowledgement_NACK)
		{
#if 0
			// To not send Response Acknowledgment as NACK to Drive board
			lstCMDrInnerTaskComm.commandToDriveBoard.val = 0;

			lstCMDrInnerTaskComm.commandResponseStatus == eNO_STATUS;
			lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;

			sucHandleCommandLS_EM_CMDiState = eIdleSendOperationCmd; // Command processing over, now reset the state machine

			gstEMtoCMDr.commandResponseACK_Status = eResponseACKProcessed;
#endif

#if 1
			// To send Response Acknowledgment as NACK to Drive board
			lstCMDrInnerTaskComm.commandResponseACK_Status = eResponseAcknowledgement_NACK;

			sucHandleCommandLS_EM_CMDiState = eSentOperationalCmdReplyAckWaitingReply; // Sent operational command reply ack. Now wait for the reply
#endif
		}

		break; //case eWaitingOperationalCmdReplyAck: // Wait for command response acknowledgment

	case eSentOperationalCmdReplyAckWaitingReply: //Sent operational command reply acknowledgment now wait for the reply

		if (lstCMDrInnerTaskComm.commandResponseACK_Status == eResponseAcknowledgementProcessed)
		{

			lstCMDrInnerTaskComm.commandToDriveBoard.val = 0;

			lstCMDrInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCMDrInnerTaskComm.commandRequestStatus = eINACTIVE;

			sucHandleCommandLS_EM_CMDiState = eIdleSendOperationCmd; // Command processing over, now reset the state machine

			gstEMtoCMDr.commandResponseACK_Status = eResponseAcknowledgementProcessed;

			/*****************************************************************************/
			/*added later as getError command was getting fired twice*/
			lstCMDrInnerTaskComm.commandResponseACK_Status = eNO_StatusAcknowledgement;
			/*****************************************************************************/

		}

		break;

	default:

		break;

	} //switch(suchandleCmdLogicSolverErrModCMDiState)

} // handleCommandLS_EM_CMDi


/******************************************************************************
 * communicationModuleDrive
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 *
 * Function Returns: void
 *
 *
 ********************************************************************************/
void communicationModuleDrive (void)
{
	static uint32_t cmdrModuleDelay;
	static enum functionState cmdrModuleState = eCaptureTime;

	if(cmdrModuleState == eCaptureTime)
	{
		cmdrModuleDelay = g_ui32TickCount;
		cmdrModuleState = eWait;
	}
	else if(cmdrModuleState == eWait)
	{
		if(get_timego(cmdrModuleDelay) >= CMDr_MODULE_WAIT_TIME)
		{
			cmdrModuleState = eExecute;
		}
	}
	else if(cmdrModuleState == eExecute)
	{
		handleCommandFromLS_EM_CMDi();

#ifndef TEST_CMDi
		pollDriveStatusFault();
#endif

		//
		//	Added for giving drive ready status to display board on 16 Jul 2014
		//
		//		gstDriveStatus.bits.driveReady = 1;
		handleDriveCmdResp();
	}


}
