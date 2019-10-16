/*********************************************************************************
 * FileName: communicationmodule.c
 * Description:
 * This source file contains the definition of all the services of communication
 * module (serial port) of the display board.
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


/****************************************************************************
 *  Modification History
 *
 *  Revision		Date                  Name          			Comments
 *  	0.1D	06/06/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Include:
 ****************************************************************************/
#include <string.h>
#include <stdint.h>
#include <driverlib/sw_crc.h>

#include "communicationmodule.h"
#include "flashmemory.h"
#include "Middleware/serial.h"
#include "Drivers/systicktimer.h"
#include "Middleware/paramdatabase.h"	// added for debugging of A537_shutter_type getParameter command
#include "userinterface.h"

extern uint8_t  Flag_askok_wireless ;
extern uint8_t now_wirelessing_cyw ;
extern uint8_t flag_commask_loginok;
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/
#define COMM_MODULE_WAIT_TIME 	400	// 4sec @ 10mS systick

#define WAIT_BEFORE_SENDING_NEXT_COMMAND	1//1	// 10mS @ 10mS systick

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
 *  Global variables for other files:
 ****************************************************************************/
// Control to drive communication error occurrence count
uint8_t guchCommunicationCRC_ErrorOccurrence[UART_CHANNEL_COUNT];

/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/
_CommunicationModuleInnerTaskComm lstCommunicationModuleInnerTaskComm;

/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
 ****************************************************************************/
/*******************************************************************
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
 *******************************************************************/

enum commandResponseType insertCommandIDandData (uint8_t * commandBuffer, uint8_t * wrIndex);

/********************************************************************
 * formControlCommandPacket
 *
 * Function Description:
 * This function forms a frame of command to the drive board.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************/

enum commandResponseType formControlCommandPacket (void);

/****************************************************************************/







/******************************************************************************
 * initCommunicationModuleGlobalRegisters
 *
 * Function Description:
 * Initialize global registers being used for CMDr inner task communication.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void initCommunicationModuleGlobalRegisters(void)
{
	lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
	lstCommunicationModuleInnerTaskComm.commandToControlBoard.val = 0;
	lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
	lstCommunicationModuleInnerTaskComm.commandResponseACK_Status = eNO_StatusAcknowledgement;
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

	switch(lstCommunicationModuleInnerTaskComm.commandToControlBoard.val)
	{
	case 0x01:
		*commandBuffer = AUTO_MAN_SELECT;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x02:
		*commandBuffer = RUN_CONTROL_BOARD;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x04:
		*commandBuffer = STOP_CONTROL_BOARD;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x08:
		*commandBuffer = UP_PRESSED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x10:
		*commandBuffer = UP_RELEASED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x20:
		*commandBuffer = DOWN_PRESSED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x40:
		*commandBuffer = DOWN_RELEASED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x80:
		*commandBuffer = OPEN_PRESSED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x100:
		*commandBuffer = OPEN_RELEASED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x200:
		*commandBuffer = CLOSE_PRESSED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x400:
		*commandBuffer = CLOSE_RELEASED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x800:
		*commandBuffer = STOP_PRESSED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x1000:
		*commandBuffer = STOP_RELEASED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x2000:
		*commandBuffer = ENTER_PRESSED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x4000:
		*commandBuffer = ENTER_RELEASED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x8000:
		*commandBuffer = GET_PARAMETER_CMD_FROM_DISPLAY;
		commandBuffer++;
		/*
		*commandBuffer = (lstCommunicationModuleInnerTaskComm.parameterNumber & 0xFF00) >> 8;
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = lstCommunicationModuleInnerTaskComm.parameterNumber & 0xFF;
		*wrIndex+=1;
		*/

		lunWord16.halfWord.val = lstCommunicationModuleInnerTaskComm.parameterNumber;
		*commandBuffer = lunWord16.byte[1];
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = lunWord16.byte[0];
		*wrIndex+=1;
		leCommandResponseType = eResponseString;
		break;
	case 0x10000:
		*commandBuffer = SET_PARAMETER_CMD_FROM_DISPLAY;
		commandBuffer++;
		/*
		*commandBuffer = (lstCommunicationModuleInnerTaskComm.parameterNumber & 0xFF00) >> 8;
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = lstCommunicationModuleInnerTaskComm.parameterNumber & 0xFF;
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = (lstCommunicationModuleInnerTaskComm.parameterValue & 0xFF000000) >> 24;
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = (lstCommunicationModuleInnerTaskComm.parameterValue & 0xFF0000) >> 16;
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = (lstCommunicationModuleInnerTaskComm.parameterValue & 0xFF00) >> 8;
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = lstCommunicationModuleInnerTaskComm.parameterValue & 0xFF;
		*wrIndex+=1;
		*/
		lunWord16.halfWord.val = lstCommunicationModuleInnerTaskComm.parameterNumber;

		*commandBuffer = lunWord16.byte[1];
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = lunWord16.byte[0];
		commandBuffer++;
		*wrIndex+=1;

		lunWord32.word.val = lstCommunicationModuleInnerTaskComm.parameterValue;

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
	case 0x20000:
		*commandBuffer = SET_TIMESTAMP;
		commandBuffer++;
		/*
		*commandBuffer = (lstCommunicationModuleInnerTaskComm.parameterValue & 0xFF000000) >> 24;
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = (lstCommunicationModuleInnerTaskComm.parameterValue & 0xFF0000) >> 16;
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = (lstCommunicationModuleInnerTaskComm.parameterValue & 0xFF00) >> 8;
		commandBuffer++;
		*wrIndex+=1;
		*commandBuffer = lstCommunicationModuleInnerTaskComm.parameterValue & 0xFF;
		*wrIndex+=1;
		*/

		lunWord32.word.val = lstCommunicationModuleInnerTaskComm.parameterValue;

		*commandBuffer = lunWord32.byte[3];
		*wrIndex+=1;
		commandBuffer++;
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
	case 0x40000:
		*commandBuffer = FIRMWARE_UPGRADECMD_FROM_DISPLAY;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x80000:
		*commandBuffer = START_INSTALLATION_CMD_FROM_DISPLAY;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x100000:
		*commandBuffer = SYSTEM_INIT_COMPLETE;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x200000:
		*commandBuffer = MONITOR_LED_CONTROL;
		commandBuffer++;
		*commandBuffer = lstCommunicationModuleInnerTaskComm.additionalCommandData;
		*wrIndex+=1;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x400000:
		*commandBuffer = GET_ERROR_LIST_CMD_FROM_DISPLAY;
		leCommandResponseType = eResponseString;
		break;

/*
	case 0x800000:	//	mode_pressed

		break;
	case 0x1000000:	//	mode_released

		break;
*/

	case 0x2000000:
		*commandBuffer = WIRELESS_MODE_CHANGE_PRESSED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x4000000:
		*commandBuffer = WIRELESS_MODE_CHANGE_RELEASED;
		leCommandResponseType = eResponseACK_NACK;
		break;
	//	Added to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
	case 0x8000000:
		*commandBuffer = SETTINGS_MODE_STATUS;
		commandBuffer++;
		*commandBuffer = lstCommunicationModuleInnerTaskComm.additionalCommandData;
		*wrIndex+=1;
		leCommandResponseType = eResponseACK_NACK;
		break;
	case 0x10000000:
		*commandBuffer = ADD_LOGIN;
		commandBuffer++;
		*commandBuffer =lstCommunicationModuleInnerTaskComm.additionalCommandData;
		*wrIndex+=1;
	   leCommandResponseType = eResponseACK_NACK;
	    break;
	default:
		*commandBuffer = INVALID_COMMAND_ID;
		break;

	} //switch(lstCommunicationModuleInnerTaskComm.commandToControlBoard.val)

	return leCommandResponseType;

} // enum commandResponseType insertCommandIDandData (uint8_t * commandBuffer, uint8_t * wrIndex)

/******************************************************************************
 * formControlCommandPacket
 *
 * Function Description:
 * This function forms a frame of command to the drive board.
 *
 * Function Parameter: void
 *
 * Function Returns: enum commandResponseType
 *
 ********************************************************************************/

enum commandResponseType formControlCommandPacket (void)
{
	uint8_t frameWrIndex = 0;
	uint8_t commandPacket[TRANSMIT_BUFFER_SIZE];
	uint16_t lucCRCvalue;

	union ui16TempData lunWord16;

	enum commandResponseType leCommandResponseType = eResponseACK_NACK;

	// Clear drive UART transmit buffer
	memset(commandPacket,'\0',sizeof(commandPacket));

	// Clear drive UART transmit byte count

	// Initialize transmit buffer index to zero

	/*Start Packet formation*/

	// Insert destination address
	frameWrIndex = DESTINATION_ADDRESS_POSITION;
	if(lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter == 1 ||
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.setParameter == 1 ||
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.firmwareUpgrade == 1 ||
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.recover_anomaly ==1)
	{
		if(lstCommunicationModuleInnerTaskComm.destination == eDestControlBoard)
		{
			commandPacket [frameWrIndex] = CONTROL_BOARD_ADDRESS;
		}
		else if(lstCommunicationModuleInnerTaskComm.destination == eDestDriveBoard)
		{
			commandPacket [frameWrIndex] = DRIVE_BOARD_ADDRESS;
		}
	}
	else
	{
		commandPacket [frameWrIndex] = CONTROL_BOARD_ADDRESS;
	}
	// Insert source address
	frameWrIndex = SOURCE_ADDRESS_POSITION;
	commandPacket [frameWrIndex] = DISPLAY_BOARD_ADDRESS;

	// Insert command ID
	frameWrIndex = COMMAND_ID_POSITION;
	leCommandResponseType = insertCommandIDandData(&commandPacket[frameWrIndex], &frameWrIndex);


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

	//	Added on 06 Nov 14 to implement firmware upgrade
	if(lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.firmwareUpgrade == 1)
	{
		gui8FirmwareUpgradeCommandResponseReceived = 2;
	}
	// Send first byte of command frame to initiate transmit interrupt
	uartSendTxBuffer(UART_control,commandPacket,commandPacket[FRAME_LENGTH_POSITION]/*+3*/);

	// Return command response type
	return leCommandResponseType;

} //enum commandResponseType formControlCommandPacket (void)


/******************************************************************************
 * handleControlCmdResp
 *
 * Function Description:
 * This function handles sending command to the control board and receiving
 * command response from the drive board.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void handleControlCmdResp(void)
{
	static uint8_t sucCommandSendAttempts = 0;

	static uint8_t sucMaxCommandSendAttempts = MAX_CMD_SEND_ATTEMPTS_FOR_CONTROL;

	static uint8_t sucTxPendingBytes = 0;

	static uint32_t suiWaitBeforeSendingNextCommand = 0;

	static uint8_t sucaResponseReceiveBuffer[RECEIVE_BUFFER_SIZE];
	static uint16_t sucRxCRC, sucCalcCRC;
	static uint8_t sucRxIndex = 0;
	static uint8_t sucRxCount = 0;

	uint8_t lucTemp;
	uint8_t lucDummyRxByte;

	static uint8_t sucRxDataLength = 0;

//	static uint32_t luiTxInstance = 0;

	static uint32_t luiRespInstance = 0;

	enum functionState
	{
		eIdle,
		eSendingCommand,
		eWaitingForResp,
		eWaitBeforeSendingNextCommand,
	};
	static enum functionState lucCmdAndRespState = eIdle;

	static enum commandResponseType sleCommandResponseType = eResponseACK_NACK;


	switch(lucCmdAndRespState)
	{
	case eIdle:

		if(lstCommunicationModuleInnerTaskComm.commandRequestStatus == eACTIVE &&
				lstCommunicationModuleInnerTaskComm.commandResponseStatus == eNO_STATUS /*&&
				lstCommunicationModuleInnerTaskComm.commandResponseACK_Status == eNO_StatusAcknowledgement*/)
		{
			// Flush global receive data buffer
			if(!uartCheckNotFreeRxBuffer(UART_control,&lucTemp))
			{
				while(lucTemp)	// Bytes present in global receive buffer
				{
					uartGetRxBuffer(UART_control,&lucDummyRxByte);
					uartCheckNotFreeRxBuffer(UART_control,&lucTemp);
				}
			}

			// Update maximum attempts for sending command based on destination ID
			if(eDestControlBoard == lstCommunicationModuleInnerTaskComm.destination)
			{
				sucMaxCommandSendAttempts = MAX_CMD_SEND_ATTEMPTS_FOR_CONTROL;
			}
			else if (eDestDriveBoard == lstCommunicationModuleInnerTaskComm.destination)
			{
				sucMaxCommandSendAttempts = MAX_CMD_SEND_ATTEMPTS_FOR_DRIVE;
			}

			// Form packet and send command
			sleCommandResponseType = formControlCommandPacket();

			// Start local timer for command send timeout
//			luiTxInstance = g_ui32TickCount;

			lucCmdAndRespState = eSendingCommand;

//uartSendTxBuffer(0,"&",1);		// debug

			// Reset receive buffer write index
			sucRxIndex = 0;
			sucRxCount = 0;
		}
		else if(lstCommunicationModuleInnerTaskComm.commandResponseACK_Status == eResponseAcknowledgement_ACK)
		{

			// Form packet and send ACK
//			uartSendTxBuffer(0,"S",1);	// debug
			lucTemp = ACK;
			uartSendTxBuffer(UART_control,&lucTemp,1);
			lstCommunicationModuleInnerTaskComm.commandResponseACK_Status = eResponseAcknowledgementProcessed;

		}
		else if(lstCommunicationModuleInnerTaskComm.commandResponseACK_Status == eResponseAcknowledgement_NACK)
		{

			// Form packet and send ACK
			lucTemp = NACK;
			uartSendTxBuffer(UART_control,&lucTemp,1);
			lstCommunicationModuleInnerTaskComm.commandResponseACK_Status = eResponseAcknowledgementProcessed;

		}
		break;

	case eSendingCommand:

		uartCheckFreeTxBuffer(UART_control, &sucTxPendingBytes);
		if(!sucTxPendingBytes)	// Command sent completely
		{
			sucCommandSendAttempts++;
			lucCmdAndRespState = eWaitingForResp;

			// Start local timer for response timeout
			luiRespInstance = g_ui32TickCount;
			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eWAITING;
		}
/*
		else
		{
			if((g_ui32TickCount - luiTxInstance) >= TxTIME_OUT)	// Command transmit timeout
			{
				sucCommandSendAttempts++;
				if(sucCommandSendAttempts == sucMaxCommandSendAttempts)	// Maximum attempts to send command reached
				{
					lstCommunicationModuleInnerTaskComm.commandResponseStatus = eCMD_SEND_FAIL;		// Command send failed
					sucCommandSendAttempts = 0;
				}
				lucCmdAndRespState = eIdle;		// Reset function state to idle
			}
		}
*/
		break;

	case eWaitingForResp:
		if(get_timego(luiRespInstance) >= RxTIME_OUT)	// Command response timeout
		{

			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
			if(sucCommandSendAttempts == sucMaxCommandSendAttempts)	// Maximum attempts to send command reached
			{

				lstCommunicationModuleInnerTaskComm.commandResponseStatus = eTIME_OUT;		// Command response timeout
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
			//	Check if data is available in the global receive buffer
			if(!uartCheckNotFreeRxBuffer(UART_control,&lucTemp))
			{	// Response data available in global buffer

				while(lucTemp)	// Data available in receive buffer
				{
					uint8_t lucGetRxBuffData; // It is used to hold the fetch byte from circular buffer

					//	Copy data available in global receive buffer into local buffer
					//uartGetRxBuffer(UART_control,&sucaResponseReceiveBuffer[sucRxIndex++]);
					uartGetRxBuffer(UART_control,&lucGetRxBuffData);

					// Validate first byte of packet depending on reply type
					if (
							(
									(sucRxCount != 0) ||
									(
											(sucRxCount == 0) &&
											(
													(sleCommandResponseType == eResponseACK_NACK && (lucGetRxBuffData == ACK || lucGetRxBuffData == NACK)) ||
													(sleCommandResponseType == eResponseString && lucGetRxBuffData == DISPLAY_BOARD_ADDRESS)
											)
									)
							) &&
							(sucRxCount < RECEIVE_BUFFER_SIZE)
					)
					{

						// copy local fetched data in static buffer
						sucaResponseReceiveBuffer[sucRxIndex++] = lucGetRxBuffData;

						//	Increment receive byte count
						sucRxCount++;

						// Compute data length
						if(sucRxIndex == FRAME_LENGTH_POSITION + 1)
						{

							//	Copy received data byte length into local buffer
							sucRxDataLength = sucaResponseReceiveBuffer[FRAME_LENGTH_POSITION];

							//validate length possible min and max length. If not found correct then capturing the packet from start
							if (sucRxDataLength < MIN_COMMAND_RESPONSE_LENGTH || sucRxDataLength > MAX_COMMAND_RESPONSE_LENGTH)
							{

								sucRxCount = 0;
								sucRxIndex = 0;
								sucRxDataLength = 0;

							} //if (sucRxDataLength < MIN_COMMAND_RESPONSE_LENGTH || sucRxDataLength > MAX_COMMAND_RESPONSE_LENGTH)

						} // if(sucRxIndex == FRAME_LENGTH_POSITION + 1)

					} // Validate first byte of packet depending on reply type

					//	Check whether the next byte is available
					uartCheckNotFreeRxBuffer(UART_control,&lucTemp);

				} //while(lucTemp)	// Data available in receive buffer

			} //if(!uartCheckNotFreeRxBuffer(UART_control,&lucTemp))

			if(
					(sucRxCount) &&
					(
							sucRxCount == sucRxDataLength /*+ 3*/ 	||	//	Complete frame (Data length + one byte each of destination ID, source ID and length byte)
							sucaResponseReceiveBuffer[0] == ACK 	||	//	ACK received
							sucaResponseReceiveBuffer[0] == NACK		//	NACK received
					)
			)
			{

				sucCalcCRC = 0;

				if((sucRxCount > 1) && (sucRxCount == sucRxDataLength /*+ 3*/))
				{

					// if response is not ACK or NACK then
					// Validate packet with CRC check
					//uartSendTxBuffer(0,"W",1);	// debug
					sucRxCRC = sucaResponseReceiveBuffer[sucRxCount-2];
					sucRxCRC <<= 8;
					sucRxCRC |= sucaResponseReceiveBuffer[sucRxCount-1];

					sucCalcCRC = Crc16(0,sucaResponseReceiveBuffer,sucRxCount-2);
				}

				if(sucRxCRC == sucCalcCRC || sucaResponseReceiveBuffer[0] == ACK || sucaResponseReceiveBuffer[0] == NACK)
				{
					// CRC passed or ACK/NACK
					// Copy received data from local buffer into inner task manager response buffer
					memcpy((void*)lstCommunicationModuleInnerTaskComm.uchRxBuffer,(const void*)sucaResponseReceiveBuffer,sucRxCount);

					//	Update number of bytes received into inner task manager
					lstCommunicationModuleInnerTaskComm.uchRxBufferLen = sucRxCount;

					//	Command successful
					lstCommunicationModuleInnerTaskComm.commandResponseStatus = eSUCCESS;


					if(lstCommunicationModuleInnerTaskComm.commandToControlBoard.val==0x10000000)
					{
						Flag_askok_wireless  = lstCommunicationModuleInnerTaskComm.additionalCommandData;
						if((lstCommunicationModuleInnerTaskComm.additionalCommandData==0x04)&&(sucaResponseReceiveBuffer[0] == ACK))//ACK
						{
							flag_commask_loginok = 1;
						}
						if((lstCommunicationModuleInnerTaskComm.additionalCommandData==0x04)&&(sucaResponseReceiveBuffer[0] == NACK))
					    {
						    flag_commask_loginok = 0;
						}
					}

					//	Reset command send attempts
					sucCommandSendAttempts = 0;

					//	Reset local receive buffer count
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

					if(sucCommandSendAttempts == sucMaxCommandSendAttempts)	// Maximum attempts to send command reached
					{
						lstCommunicationModuleInnerTaskComm.commandResponseStatus = eFAIL;		// Command response timeout
						sucCommandSendAttempts = 0;
					}
					else
					{
						lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
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
		if(get_timego(suiWaitBeforeSendingNextCommand) >= WAIT_BEFORE_SENDING_NEXT_COMMAND)
		{
			lucCmdAndRespState = eIdle;
			clear_uart_buff_cyw(UART_control);

		}
		break;

	default:
		break;

	}	// switch(lucCmdAndRespState)
}	// handleDriveCmdResp



/******************************************************************************
 * pollDriveControlStatusFault
 *
 * Function Description:
 * This function will Poll the Status Register of the Drive and Control
 * If the Installation and/or Fault bit in Status Register is Set, then function will poll the Sub Installation and Fault Register.
 * Function will use 'handleControlCmdResp' to send physical command and to process the reply
 * Function will store the reply in respective Global Intertask Communication Structure
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void pollDriveControlStatusFault(void)
{
	static uint32_t suiStatusFaultPollTimer = 0;

	union ui16TempData lunWord16;
	union ui32TempData lunWord32;

	enum functionState
	{
		eStartStatusPollTimer = 0,
		eStartedStatusPollTimer,

		eSendStatusPollCmdToDrive,
		eSentStatusPollCmdWaitForReplyFromDrive,
		eSendFaultPollCmdToDrive,
		eSentFaultCmdWaitForReplyFromDrive,
		eSendCommFaultPollCmdToDrive,
		eSentCommFaultPollCmdWaitForReplyFromDrive,
		eSendAppFaultPollCmdToDrive,
		eSentAppFaultPollCmdWaitForReplyFromDrive,
		eSendMotorFaultPollCmdToDrive,
		eSentMotorFaultPollCmdWaitForReplyFromDrive,
		eSendProcessorFaultPollCmdToDrive,
		eSentProcessorFaultPollCmdWaitForReplyFromDrive,
		eSendInstalltionPollCmdToDrive,
		eSentInstalltionPollCmdWaitForReplyFromDrive,

		eSendStatusPollCmdToControl,
		eSentStatusPollCmdWaitForReplyFromControl,
		eSendFaultPollCmdToControl,
		eSentFaultCmdWaitForReplyFromControl,
		eSendCommFaultPollCmdToControl,
		eSentCommFaultPollCmdWaitForReplyFromControl,
		eSendAppFaultPollCmdToControl,
		eSentAppFaultPollCmdWaitForReplyFromControl,
		eSendProcessorFaultPollCmdToControl,
		eSentProcessorFaultPollCmdWaitForReplyFromControl,

	};

	static unsigned char sucPollDriveControlStatusFaultState = eStartStatusPollTimer;

	switch(sucPollDriveControlStatusFaultState)
	{

	case eStartStatusPollTimer:

		suiStatusFaultPollTimer = g_ui32TickCount;
		sucPollDriveControlStatusFaultState = eStartedStatusPollTimer;

		break;

	case eStartedStatusPollTimer:

		if(get_timego( suiStatusFaultPollTimer) >= STATUS_FAULT_POLL_TIMEOUT)
		{

			sucPollDriveControlStatusFaultState = eSendStatusPollCmdToDrive;

		} // if(g_ui32TickCount - suiStatusFaultPollTimer >= STATUS_FAULT_POLL_TIMEOUT)

		break;

	case eSendStatusPollCmdToDrive:
	case eSendStatusPollCmdToControl:

		if(
				(gstUMtoCMoperational.commandRequestStatus == eINACTIVE) &&
				(gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) &&
				(gstEMtoCM_monitorLED.commandRequestStatus == eINACTIVE) &&
				(gstEMtoCM_errorList.commandRequestStatus == eINACTIVE) &&
				(lstCommunicationModuleInnerTaskComm.commandRequestStatus == eINACTIVE)
		)
		{
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 1;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE;

			if (sucPollDriveControlStatusFaultState == eSendStatusPollCmdToDrive)
			{
				lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;   // Right now we are polling drive board status available with control board
				lstCommunicationModuleInnerTaskComm.parameterNumber = DRIVE_STATUS_PARAM_NO;				// A605: Drive board status register
				sucPollDriveControlStatusFaultState = eSentStatusPollCmdWaitForReplyFromDrive;
			}
			else
			{
				lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
				lstCommunicationModuleInnerTaskComm.parameterNumber = CONTROL_STATUS_PARAM_NO;				// A612: Control board status register
				sucPollDriveControlStatusFaultState = eSentStatusPollCmdWaitForReplyFromControl;
			}

		}

		break;

	case eSentStatusPollCmdWaitForReplyFromDrive:
	case eSentStatusPollCmdWaitForReplyFromControl:

		if (lstCommunicationModuleInnerTaskComm .commandResponseStatus == eSUCCESS)
		{

			/*lui16Temp = lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 1];
			lui16Temp = lui16Temp << 8;
			lui16Temp = lui16Temp | lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 2];*/
			// Need to check whether response format same for both the status register

			if (sucPollDriveControlStatusFaultState == eSentStatusPollCmdWaitForReplyFromDrive)
			{
				lunWord32.byte[3] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION];
				lunWord32.byte[2] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 1];
				lunWord32.byte[1] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 2];
				lunWord32.byte[0] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];

				gstDriveBoardStatus.val = lunWord32.word.val; 			// Update global drive status declared in control board
			}
			else
			{
				lunWord16.byte[1] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 2];
				lunWord16.byte[0] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];

				gstControlBoardStatus.val = lunWord16.halfWord.val; 	// Update global control status declared in control board
			}

			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 0;

			//
			//	Clear display-control communication error flag
			//
			if(
					(gstDisplayCommunicationFault.bits.commFailControl == 1) ||
					// Added on 15 Sep 2014 to avoid communication error logging
					// during power fail condition
					(gucDItoCT_CommError == 1)
			)
			{
				gstDisplayCommunicationFault.bits.commFailControl = 0;
				gucDItoCT_CommError = 0;
			}
			//
			//	Clear display board CRC error flag
			//
			if(gstDisplayCommunicationFault.bits.crcError == 1)
			{
				gstDisplayCommunicationFault.bits.crcError = 0;
			}
			//
			// Clear buffer overflow flag
			//
			if(gstDisplayCommunicationFault.bits.uartError == 1)
			{
				gstDisplayCommunicationFault.bits.uartError = 0;
			}

			if(0 == gstDisplayCommunicationFault.val)
			{
				gstDisplayBoardFault.bits.displayCommunication = 0;
			}

			if(0 == gstDisplayBoardFault.val)
			{
				gstDisplayBoardStatus.bits.displayFault = 0;
			}

			if (sucPollDriveControlStatusFaultState == eSentStatusPollCmdWaitForReplyFromDrive)
			{
				sucPollDriveControlStatusFaultState = eSendFaultPollCmdToDrive;
			}
			else
			{
				sucPollDriveControlStatusFaultState = eSendFaultPollCmdToControl;
			}

		}

		if (lstCommunicationModuleInnerTaskComm.commandResponseStatus == eTIME_OUT ||
				lstCommunicationModuleInnerTaskComm .commandResponseStatus == eFAIL)
		{
			//
			//	Display board communication fault
			//
//			gstDisplayBoardFault.bits.displayCommunication = 1;

			if (lstCommunicationModuleInnerTaskComm .commandResponseStatus == eTIME_OUT)
			{
				//
				//	Control communication failed
				//

				// Added on 15 Sep 2014 to avoid communication error logging
				// during power fail condition
//				gstDisplayBoardStatus.bits.displayFault = 1;
//				gstDisplayBoardFault.bits.displayCommunication = 1;
//				gstDisplayCommunicationFault.bits.commFailControl = 1;
				gucDItoCT_CommError = 1;
			}
			else
			{
				//
				//	CRC error in response from control
				//
				gstDisplayBoardStatus.bits.displayFault = 1;
				gstDisplayBoardFault.bits.displayCommunication = 1;
				gstDisplayCommunicationFault.bits.crcError = 1;
			}

			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 0;

			sucPollDriveControlStatusFaultState = eStartStatusPollTimer;
		}

		break;

	case eSendFaultPollCmdToDrive:
	case eSendFaultPollCmdToControl:

		if ( 	(sucPollDriveControlStatusFaultState == eSendFaultPollCmdToDrive &&
				(gstDriveBoardStatus.bits.driveFault != 0 || gstDriveBoardStatus.bits.driveFaultUnrecoverable != 0) ) ||
				(sucPollDriveControlStatusFaultState == eSendFaultPollCmdToControl &&
						(gstControlBoardStatus.bits.controlFault != 0 || gstControlBoardStatus.bits.controlFaultUnrecoverable != 0))	)
		{

			if(
					(gstUMtoCMoperational.commandRequestStatus == eINACTIVE) &&
					(gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) &&
					(gstEMtoCM_monitorLED.commandRequestStatus == eINACTIVE) &&
					(gstEMtoCM_errorList.commandRequestStatus == eINACTIVE) &&
					(lstCommunicationModuleInnerTaskComm.commandRequestStatus == eINACTIVE)
			)
			{

				lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 1;
				lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE;

				if (sucPollDriveControlStatusFaultState == eSendFaultPollCmdToDrive)
				{
					lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard; // Right now we are polling drive board status avaialble with control board
					lstCommunicationModuleInnerTaskComm.parameterNumber = DRIVE_FAULT_PARAM_NO;		// A607: Drive fault
					sucPollDriveControlStatusFaultState = eSentFaultCmdWaitForReplyFromDrive;
				}
				else
				{
					lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
					lstCommunicationModuleInnerTaskComm.parameterNumber = CONTROL_FAULT_PARAM_NO;		// A: Control fault
					sucPollDriveControlStatusFaultState = eSentFaultCmdWaitForReplyFromControl;
				}

			}

		}
		else
		{

			if (sucPollDriveControlStatusFaultState == eSendFaultPollCmdToDrive)
			{
				sucPollDriveControlStatusFaultState = eSendInstalltionPollCmdToDrive;
				gstDriveBoardFault.val = 0;											// Clear global drive fault declared in control board
				gstDriveApplicationFault.val = 0;
				gstDriveCommunicationFault.val = 0;
//				gstDriveInstallation.val = 0;
				gstDriveMotorFault.val = 0;
				gstDriveProcessorfault.val = 0;
			}
			else
			{
				sucPollDriveControlStatusFaultState = eStartStatusPollTimer;
				gstControlBoardFault.val = 0;							   	    // Clear global control fault declared in control board
				gstControlApplicationFault.val = 0;
				gstControlCommunicationFault.val = 0;
				gstControlProcessorFault.val = 0;
			}

		}

		break;

	case eSentFaultCmdWaitForReplyFromDrive:
	case eSentFaultCmdWaitForReplyFromControl:

		if (lstCommunicationModuleInnerTaskComm .commandResponseStatus == eSUCCESS)
		{

			// Need to check whether it is common reply for both status read
			if (sucPollDriveControlStatusFaultState == eSentFaultCmdWaitForReplyFromDrive)
			{
				gstDriveBoardFault.val = lstCommunicationModuleInnerTaskComm .uchRxBuffer[RESPONSE_DATA_POSITION + 3]; 							// Update global drive fault declared in control board
			}
			else
			{
				gstControlBoardFault.val = lstCommunicationModuleInnerTaskComm .uchRxBuffer[RESPONSE_DATA_POSITION + 3]; 					// Update global drive fault declared in control board
			}

			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 0;

			//
			//	Clear display-control communication error flag
			//
			if(
					(gstDisplayCommunicationFault.bits.commFailControl == 1) ||
					// Added on 15 Sep 2014 to avoid communication error logging
					// during power fail condition
					(gucDItoCT_CommError == 1)
			)
			{
				gstDisplayCommunicationFault.bits.commFailControl = 0;
				gucDItoCT_CommError = 0;
			}
			//
			//	Clear display board CRC error flag
			//
			if(gstDisplayCommunicationFault.bits.crcError == 1)
			{
				gstDisplayCommunicationFault.bits.crcError = 0;
			}
			//
			// Clear buffer overflow flag
			//
			if(gstDisplayCommunicationFault.bits.uartError == 1)
			{
				gstDisplayCommunicationFault.bits.uartError = 0;
			}

			if(0 == gstDisplayCommunicationFault.val)
			{
				gstDisplayBoardFault.bits.displayCommunication = 0;
			}

			if(0 == gstDisplayBoardFault.val)
			{
				gstDisplayBoardStatus.bits.displayFault = 0;
			}

			if (sucPollDriveControlStatusFaultState == eSentFaultCmdWaitForReplyFromDrive)
			{
				sucPollDriveControlStatusFaultState = eSendCommFaultPollCmdToDrive;
			}
			else
			{
				sucPollDriveControlStatusFaultState = eSendCommFaultPollCmdToControl;
			}

		}

		if (lstCommunicationModuleInnerTaskComm.commandResponseStatus == eTIME_OUT ||
				lstCommunicationModuleInnerTaskComm .commandResponseStatus == eFAIL)
		{

			//
			//	Display board communication fault
			//
//			gstDisplayBoardFault.bits.displayCommunication = 1;

			if (lstCommunicationModuleInnerTaskComm .commandResponseStatus == eTIME_OUT)
			{
				//				uartSendTxBuffer(0,"!",1);	// debug
				//
				//	Control communication failed
				//

				// Added on 15 Sep 2014 to avoid communication error logging
				// during power fail condition
//				gstDisplayBoardStatus.bits.displayFault = 1;
//				gstDisplayBoardFault.bits.displayCommunication = 1;
//				gstDisplayCommunicationFault.bits.commFailControl = 1;
				gucDItoCT_CommError = 1;
			}
			else
			{
				//				uartSendTxBuffer(0,"@",1);	// debug
				//
				//	CRC error in response from control
				//
				gstDisplayBoardStatus.bits.displayFault = 1;
				gstDisplayBoardFault.bits.displayCommunication = 1;
				gstDisplayCommunicationFault.bits.crcError = 1;
			}

			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 0;

			sucPollDriveControlStatusFaultState = eStartStatusPollTimer;

		}

		break;

	case eSendCommFaultPollCmdToDrive:
	case eSendAppFaultPollCmdToDrive:
	case eSendMotorFaultPollCmdToDrive:
	case eSendProcessorFaultPollCmdToDrive:

	case eSendCommFaultPollCmdToControl:
	case eSendAppFaultPollCmdToControl:
	case eSendProcessorFaultPollCmdToControl:

		if (
				(sucPollDriveControlStatusFaultState == eSendCommFaultPollCmdToDrive &&
						gstDriveBoardFault.bits.driveCommunication == 1) ||
				(sucPollDriveControlStatusFaultState == eSendAppFaultPollCmdToDrive &&
						gstDriveBoardFault.bits.driveApplication == 1) ||
				(sucPollDriveControlStatusFaultState == eSendMotorFaultPollCmdToDrive &&
						gstDriveBoardFault.bits.driveMotor == 1) ||
				(sucPollDriveControlStatusFaultState == eSendProcessorFaultPollCmdToDrive &&
						gstDriveBoardFault.bits.driveProcessor == 1) ||

				(sucPollDriveControlStatusFaultState == eSendCommFaultPollCmdToControl &&
						gstControlBoardFault.bits.controlCommunication == 1) ||
				(sucPollDriveControlStatusFaultState == eSendAppFaultPollCmdToControl &&
						gstControlBoardFault.bits.controlApplication == 1) ||
				(sucPollDriveControlStatusFaultState == eSendProcessorFaultPollCmdToControl &&
						gstControlBoardFault.bits.controlProcessor == 1)

		)
		{

			if(
					(gstUMtoCMoperational.commandRequestStatus == eINACTIVE) &&
					(gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) &&
					(gstEMtoCM_monitorLED.commandRequestStatus == eINACTIVE) &&
					(gstEMtoCM_errorList.commandRequestStatus == eINACTIVE) &&
					(lstCommunicationModuleInnerTaskComm.commandRequestStatus == eINACTIVE)
			)
			{

				lstCommunicationModuleInnerTaskComm .commandToControlBoard.bits.getParameter = 1;

				if (sucPollDriveControlStatusFaultState == eSendCommFaultPollCmdToDrive)
				{
					lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
					lstCommunicationModuleInnerTaskComm.parameterNumber = DRIVE_COMM_FAULT_PARAM_NO;		// A606: Drive communication error
					sucPollDriveControlStatusFaultState = eSentCommFaultPollCmdWaitForReplyFromDrive;
				}
				else if (sucPollDriveControlStatusFaultState == eSendAppFaultPollCmdToDrive)
				{
					lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
					lstCommunicationModuleInnerTaskComm.parameterNumber = DRIVE_APPLN_FAULT_PARAM_NO;		// A608: Drive application error
					sucPollDriveControlStatusFaultState = eSentAppFaultPollCmdWaitForReplyFromDrive;
				}
				else if (sucPollDriveControlStatusFaultState == eSendMotorFaultPollCmdToDrive)
				{
					lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
					lstCommunicationModuleInnerTaskComm.parameterNumber = DRIVE_MOTOR_FAULT_PARAM_NO;		// A607: Drive motor error
					sucPollDriveControlStatusFaultState = eSentMotorFaultPollCmdWaitForReplyFromDrive;
				}
				else if (sucPollDriveControlStatusFaultState == eSendProcessorFaultPollCmdToDrive)
				{
					lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
					lstCommunicationModuleInnerTaskComm.parameterNumber = DRIVE_PROCESSOR_FAULT_PARAM_NO;		// A609: Drive processor error
					sucPollDriveControlStatusFaultState = eSentProcessorFaultPollCmdWaitForReplyFromDrive;
				}

				else if (sucPollDriveControlStatusFaultState == eSendCommFaultPollCmdToControl)
				{
					lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard; // Right now we are polling drive board status avaialble with control board
					lstCommunicationModuleInnerTaskComm.parameterNumber = CONTROL_COMM_FAULT_PARAM_NO;		// A611: Control communication error
					sucPollDriveControlStatusFaultState = eSentCommFaultPollCmdWaitForReplyFromControl;
				}
				else if (sucPollDriveControlStatusFaultState == eSendAppFaultPollCmdToControl)
				{
					lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard; // Right now we are polling drive board status avaialble with control board
					lstCommunicationModuleInnerTaskComm.parameterNumber = CONTROL_APPLN_FAULT_PARAM_NO;		// A612: Control application error
					sucPollDriveControlStatusFaultState = eSentAppFaultPollCmdWaitForReplyFromControl;
				}
				else if (sucPollDriveControlStatusFaultState == eSendProcessorFaultPollCmdToControl)
				{
					lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard; // Right now we are polling drive board status avaialble with control board
					lstCommunicationModuleInnerTaskComm.parameterNumber = CONTROL_PROCESSOR_FAULT_PARAM_NO;		// A613: Control processor error
					sucPollDriveControlStatusFaultState = eSentProcessorFaultPollCmdWaitForReplyFromControl;
				}


				lstCommunicationModuleInnerTaskComm .commandRequestStatus = eACTIVE;

			}

		}
		else
		{

			if (sucPollDriveControlStatusFaultState == eSendCommFaultPollCmdToDrive)
			{
				sucPollDriveControlStatusFaultState = eSendAppFaultPollCmdToDrive;
				gstDriveCommunicationFault.val = 0;											// Clear global drive fault declared in control board
			}
			else if (sucPollDriveControlStatusFaultState == eSendAppFaultPollCmdToDrive)
			{
				sucPollDriveControlStatusFaultState = eSendMotorFaultPollCmdToDrive;
				gstDriveApplicationFault.val = 0;											// Clear global drive fault declared in control board
			}
			else if (sucPollDriveControlStatusFaultState == eSendMotorFaultPollCmdToDrive)
			{
				sucPollDriveControlStatusFaultState = eSendProcessorFaultPollCmdToDrive;
				gstDriveMotorFault.val = 0;													// Clear global drive fault declared in control board
			}
			else if (sucPollDriveControlStatusFaultState == eSendProcessorFaultPollCmdToDrive)
			{
				sucPollDriveControlStatusFaultState = eSendInstalltionPollCmdToDrive;
				gstDriveProcessorfault.val = 0;												// Clear global drive fault declared in control board
			}

			else if (sucPollDriveControlStatusFaultState == eSendCommFaultPollCmdToControl)
			{
				sucPollDriveControlStatusFaultState = eSendAppFaultPollCmdToControl;
				gstControlCommunicationFault.val = 0;										// Clear global control fault declared in control board
			}
			else if (sucPollDriveControlStatusFaultState == eSendAppFaultPollCmdToControl)
			{
				sucPollDriveControlStatusFaultState = eSendProcessorFaultPollCmdToControl;
				gstControlApplicationFault.val = 0;											// Clear global control fault declared in control board
			}
			else if (sucPollDriveControlStatusFaultState == eSendProcessorFaultPollCmdToControl)
			{
				sucPollDriveControlStatusFaultState = eStartStatusPollTimer;
				gstControlProcessorFault.val = 0;											// Clear global control fault declared in control board
			}

		}

		break;

	case eSentCommFaultPollCmdWaitForReplyFromDrive:
	case eSentAppFaultPollCmdWaitForReplyFromDrive:
	case eSentMotorFaultPollCmdWaitForReplyFromDrive:
	case eSentProcessorFaultPollCmdWaitForReplyFromDrive:

	case eSentCommFaultPollCmdWaitForReplyFromControl:
	case eSentAppFaultPollCmdWaitForReplyFromControl:
	case eSentProcessorFaultPollCmdWaitForReplyFromControl:

		if (lstCommunicationModuleInnerTaskComm .commandResponseStatus == eSUCCESS)
		{

			if (sucPollDriveControlStatusFaultState == eSentCommFaultPollCmdWaitForReplyFromDrive ||
					sucPollDriveControlStatusFaultState == eSentCommFaultPollCmdWaitForReplyFromControl)
			{

				if (sucPollDriveControlStatusFaultState == eSentCommFaultPollCmdWaitForReplyFromDrive)
				{
					// Update global drive fault declared in control board
					gstDriveCommunicationFault.val = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];
					sucPollDriveControlStatusFaultState = eSendAppFaultPollCmdToDrive;
				}
				else
				{
					// Update global drive fault declared in control board
					gstControlCommunicationFault.val = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];

					// Added on 15 Sep 2014 to avoid communication error logging
					// during power fail condition
					if(1 == gstControlCommunicationFault.bits.commFailDrive)
					{
						gucCTtoDR_CommError = 1;
					}
					else
					{
						gucCTtoDR_CommError = 0;
					}

					sucPollDriveControlStatusFaultState = eSendAppFaultPollCmdToControl;
				}

			}
			else if (sucPollDriveControlStatusFaultState == eSentAppFaultPollCmdWaitForReplyFromDrive ||
					sucPollDriveControlStatusFaultState == eSentAppFaultPollCmdWaitForReplyFromControl)
			{
				/*lui32Temp = lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 1];
				lui32Temp = lui32Temp << 8;
				lui32Temp = lui32Temp | lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 2];
				lui32Temp = lui32Temp << 8;
				lui32Temp = lui32Temp | lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 3];
				lui32Temp = lui32Temp << 8;
				lui32Temp = lui32Temp | lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 4];*/

				if (sucPollDriveControlStatusFaultState == eSentAppFaultPollCmdWaitForReplyFromDrive)
				{
					lunWord32.byte[3] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION];
					lunWord32.byte[2] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 1];
					lunWord32.byte[1] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 2];
					lunWord32.byte[0] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];

					//
					//	Update global drive fault declared in control board
					//
					gstDriveApplicationFault.val = lunWord32.word.val;
					sucPollDriveControlStatusFaultState = eSendMotorFaultPollCmdToDrive;
				}
				else
				{
					//
					//	Update global control application fault declared in display board
					//
					gstControlApplicationFault.val = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];
					sucPollDriveControlStatusFaultState = eSendProcessorFaultPollCmdToControl;
				}

			}
			else if (sucPollDriveControlStatusFaultState == eSentMotorFaultPollCmdWaitForReplyFromDrive)
			{
				/*lui16Temp = lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 1];
				lui16Temp = lui16Temp << 8;
				lui16Temp = lui16Temp | lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 2];*/
				lunWord16.byte[1] = lstCommunicationModuleInnerTaskComm .uchRxBuffer[RESPONSE_DATA_POSITION + 2];
				lunWord16.byte[0] = lstCommunicationModuleInnerTaskComm .uchRxBuffer[RESPONSE_DATA_POSITION + 3];

				//
				//	Update global drive motor fault status declared in display board
				//
				gstDriveMotorFault.val = lunWord16.halfWord.val;

				sucPollDriveControlStatusFaultState = eSendProcessorFaultPollCmdToDrive;
			}
			else if (sucPollDriveControlStatusFaultState == eSentProcessorFaultPollCmdWaitForReplyFromDrive || sucPollDriveControlStatusFaultState == eSentProcessorFaultPollCmdWaitForReplyFromControl)
			{
				/*lui16Temp = lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 1];
				lui16Temp = lui16Temp << 8;
				lui16Temp = lui16Temp | lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 2];*/
				lunWord16.byte[1] = lstCommunicationModuleInnerTaskComm .uchRxBuffer[RESPONSE_DATA_POSITION + 2];
				lunWord16.byte[0] = lstCommunicationModuleInnerTaskComm .uchRxBuffer[RESPONSE_DATA_POSITION + 3];

				if (sucPollDriveControlStatusFaultState == eSentProcessorFaultPollCmdWaitForReplyFromDrive)
				{
					//
					//	Update global drive processor status declared in display board
					//
					gstDriveProcessorfault.val = lunWord16.halfWord.val;
					sucPollDriveControlStatusFaultState = eSendInstalltionPollCmdToDrive;
				}
				else
				{
					//
					//	Update global drive status declared in control board
					//
					gstControlProcessorFault.val = lunWord16.halfWord.val;
					sucPollDriveControlStatusFaultState = eStartStatusPollTimer;
				}
			}


			//
			//	Clear display-control communication error flag
			//
			if(
					(gstDisplayCommunicationFault.bits.commFailControl == 1) ||
					// Added on 15 Sep 2014 to avoid communication error logging
					// during power fail condition
					(gucDItoCT_CommError == 1)
			)
			{
				gstDisplayCommunicationFault.bits.commFailControl = 0;
				gucDItoCT_CommError = 0;
			}
			//
			//	Clear display board CRC error flag
			//
			if(gstDisplayCommunicationFault.bits.crcError == 1)
			{
				gstDisplayCommunicationFault.bits.crcError = 0;
			}
			//
			// Clear buffer overflow flag
			//
			if(gstDisplayCommunicationFault.bits.uartError == 1)
			{
				gstDisplayCommunicationFault.bits.uartError = 0;
			}

			if(0 == gstDisplayCommunicationFault.val)
			{
				gstDisplayBoardFault.bits.displayCommunication = 0;
			}

			if(0 == gstDisplayBoardFault.val)
			{
				gstDisplayBoardStatus.bits.displayFault = 0;
			}

			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 0;

		}

		if (lstCommunicationModuleInnerTaskComm.commandResponseStatus == eTIME_OUT ||
				lstCommunicationModuleInnerTaskComm.commandResponseStatus == eFAIL)
		{

			//
			//	Display board communication fault
			//
//			gstDisplayBoardFault.bits.displayCommunication = 1;

			if (lstCommunicationModuleInnerTaskComm.commandResponseStatus == eTIME_OUT)
			{
				//	uartSendTxBuffer(0,"!",1);	// debug
				//
				//	Control communication failed
				//

				// Added on 15 Sep 2014 to avoid communication error logging
				// during power fail condition
//				gstDisplayBoardStatus.bits.displayFault = 1;
//				gstDisplayBoardFault.bits.displayCommunication = 1;
//				gstDisplayCommunicationFault.bits.commFailControl = 1;
				gucDItoCT_CommError = 1;
			}
			else
			{
				//	uartSendTxBuffer(0,"@",1);	// debug
				//
				//	CRC error in response from control
				//
				gstDisplayBoardStatus.bits.displayFault = 1;
				gstDisplayBoardFault.bits.displayCommunication = 1;
				gstDisplayCommunicationFault.bits.crcError = 1;
			}

			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 0;

			sucPollDriveControlStatusFaultState = eStartStatusPollTimer;

		}

		break;

	case eSendInstalltionPollCmdToDrive:

		if (gstDriveBoardStatus.bits.driveInstallation != 0)
		{

			if(
					(gstUMtoCMoperational.commandRequestStatus == eINACTIVE) &&
					(gstUMtoCMdatabase.commandRequestStatus == eINACTIVE) &&
					(gstEMtoCM_monitorLED.commandRequestStatus == eINACTIVE) &&
					(gstEMtoCM_errorList.commandRequestStatus == eINACTIVE) &&
					(lstCommunicationModuleInnerTaskComm.commandRequestStatus == eINACTIVE)
			)
			{

				lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard; // Right now we are polling drive board status available with control board
				lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 1;
				lstCommunicationModuleInnerTaskComm.parameterNumber = DRIVE_INSTALTION_STATUS_PARAM_NO;		// Drive installation
				lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE;

				sucPollDriveControlStatusFaultState = eSentInstalltionPollCmdWaitForReplyFromDrive;

			}

		}
		else
		{

			sucPollDriveControlStatusFaultState = eSendStatusPollCmdToControl;
			gstDriveInstallation.val = 0;											// Clear global drive fault declared in control board

		}

		break;


	case eSentInstalltionPollCmdWaitForReplyFromDrive:

		if (lstCommunicationModuleInnerTaskComm .commandResponseStatus == eSUCCESS)
		{

			/*lui16Temp = lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 1];
			lui16Temp = lui16Temp << 8;
			lui16Temp = lui16Temp | lstCommunicationModuleInnerTaskComm .uchRxBuffer[COMMAND_ID + 2];*/
			lunWord16.byte[1] = lstCommunicationModuleInnerTaskComm .uchRxBuffer[RESPONSE_DATA_POSITION + 2];
			lunWord16.byte[0] = lstCommunicationModuleInnerTaskComm .uchRxBuffer[RESPONSE_DATA_POSITION + 3];

			//
			//	Update global drive status declared in control board
			//
			gstDriveInstallation.val = lunWord16.halfWord.val;

			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 0;

			//
			//	Clear display board CRC error flag
			//
			if(gstDisplayCommunicationFault.bits.crcError == 1)
			{
				gstDisplayCommunicationFault.bits.crcError = 0;
			}
			//
			// Clear buffer overflow flag
			//
			if(gstDisplayCommunicationFault.bits.uartError == 1)
			{
				gstDisplayCommunicationFault.bits.uartError = 0;
			}
			//
			// Clear buffer overflow flag
			//
			if(gstDisplayCommunicationFault.bits.uartError == 1)
			{
				gstDisplayCommunicationFault.bits.uartError = 0;
			}

			sucPollDriveControlStatusFaultState = eSendStatusPollCmdToControl; // now scan all status of control board

		}

		if (lstCommunicationModuleInnerTaskComm.commandResponseStatus == eTIME_OUT ||
				lstCommunicationModuleInnerTaskComm.commandResponseStatus == eFAIL)
		{

			//
			//	Display board communication fault
			//
//			gstDisplayBoardFault.bits.displayCommunication = 1;

			if (lstCommunicationModuleInnerTaskComm.commandResponseStatus == eTIME_OUT)
			{
				//				uartSendTxBuffer(0,"!",1);	// debug
				//
				//	Control communication failed
				//

				// Added on 15 Sep 2014 to avoid communication error logging
				// during power fail condition
//				gstDisplayBoardStatus.bits.displayFault = 1;
//				gstDisplayBoardFault.bits.displayCommunication = 1;
//				gstDisplayCommunicationFault.bits.commFailControl = 1;
				gucDItoCT_CommError = 1;
			}
			else
			{
				//				uartSendTxBuffer(0,"@",1);	// debug
				//
				//	CRC error in response from control
				//
				gstDisplayBoardStatus.bits.displayFault = 1;
				gstDisplayBoardFault.bits.displayCommunication = 1;
				gstDisplayCommunicationFault.bits.crcError = 1;
			}

			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 0;

			sucPollDriveControlStatusFaultState = eStartStatusPollTimer;

		}

		break;


	default:

		break;

	} // switch(sucPollDriveControlStatusFaultState)

} // pollDriveStatusFault



/******************************************************************************
 * handleCommandFromUM_EM_EH
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
void handleCommandFromUM_EM_EH(void)
{

	enum functionState
		{
			eIdleSendOperationCmd = 0,
			eSentOperationalCmdWaitingReply,
			eWaitingOperationalCmdReplyAck,
			eSentOperationalCmdReplyAckWaitingReply,
		};

	static unsigned char sucHandleCommandUM_EM_EH_State = eIdleSendOperationCmd;

//	unsigned char lucTemp;

//	union ui16TempData lunWord16;
	union ui32TempData lunWord32;

	unsigned char lucCommandSupportReponseAck;

	switch(sucHandleCommandUM_EM_EH_State)
	{

#if 0
	case eIdleSendOperationCmd: // Send command through 'handleDriveCmdResp'

		if(
				(
						((gstUMtoCMoperational.commandRequestStatus == eACTIVE) &&
								(gstUMtoCMoperational.commandResponseStatus == eNO_STATUS))||
						((gstUMtoCMdatabase.commandRequestStatus == eACTIVE) &&
								(gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS))||
						((gstEMtoCM_monitorLED.commandRequestStatus == eACTIVE) &&
								(gstEMtoCM_monitorLED.commandResponseStatus == eNO_STATUS))||
						((gstEMtoCM_errorList.commandRequestStatus == eACTIVE) &&
								(gstEMtoCM_errorList.commandResponseStatus == eNO_STATUS))
				) &&
				(lstCommunicationModuleInnerTaskComm.commandRequestStatus == eINACTIVE)
		)
		{

			if((gstUMtoCMoperational.commandRequestStatus == eACTIVE) && (gstUMtoCMoperational.commandResponseStatus == eNO_STATUS))
			{


				uartSendTxBuffer(UART_debug,"1",1);

				lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;

				// Read command request from 'UM' and request to 'handleControlCmdResp' to send same

				if (gstUMtoCMoperational.commandToControlBoard.bits.autoManSelect)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.autoManualSelect = 1;
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.runControlBoard)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.runControlBoard = 1;
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.stopControlBoard)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.stopControlBoard = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.upPressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.upPressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.upReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.upReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.downPressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.downPressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.downReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.downReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.openPressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.openPressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.openReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.openReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.closePressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.closePressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.closeReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.closeReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.stopPressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.stopPressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.stopReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.stopReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.enterPressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.enterPressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.enterReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.enterReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.modePressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.modePressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.modeReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.modeReleased = 1;
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.startInstallation)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.startInstallation = 1;
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.systemInitComplete)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.systemInitComplete = 1;
				}

				gstUMtoCMoperational.commandResponseStatus = eWAITING;

			}
			else if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
			{

				uartSendTxBuffer(UART_debug,"2",1);

				// Removed below line and added in individual command cases on 07 Jul 14
//				lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;

				// Passing command data to 'lstCommunicationModuleInnerTaskComm' is pending

				if (gstUMtoCMdatabase.commandToControlBoard.bits.setTimeStamp)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.setTimeStamp = 1;

					//
					//	Copy time-stamp to inner task parameter value
					//
					lstCommunicationModuleInnerTaskComm.parameterValue = gstUMtoCMdatabase.dataToControlBoard.commandData.timeStamp;

					//	Added on 07 Jul 14 to incorporate destination ID for getParam command
					lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
				}
				else if (gstUMtoCMdatabase.commandToControlBoard.bits.getParameter)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 1;

					//	Added on 07 Jul 14 to incorporate destination ID for getParam command
					lstCommunicationModuleInnerTaskComm.destination = gstUMtoCMdatabase.destination;

					//
					//	Copy parameter number to inner task parameter number
					//
					lstCommunicationModuleInnerTaskComm.parameterNumber = gstUMtoCMdatabase.dataToControlBoard.parameterNumber;

				}
				else if (gstUMtoCMdatabase.commandToControlBoard.bits.setParameter)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.setParameter = 1;

					//
					//	Copy parameter number to inner task parameter number
					//
					lstCommunicationModuleInnerTaskComm.parameterNumber = gstUMtoCMdatabase.dataToControlBoard.parameterNumber;

					//
					//	Copy parameter value to inner task parameter value
					//
					lstCommunicationModuleInnerTaskComm.parameterValue = gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue;

					//	Added on 07 Jul 14 to incorporate destination ID for setParam command
					lstCommunicationModuleInnerTaskComm.destination = gstUMtoCMdatabase.destination;
				}

				gstUMtoCMdatabase.commandResponseStatus = eWAITING;

			}
			else if(gstEMtoCM_errorList.commandRequestStatus == eACTIVE)
			{

				uartSendTxBuffer(UART_debug,"3",1);

				lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;

				lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getError = 1;

				gstEMtoCM_errorList.commandResponseStatus = eWAITING;

			}
			else if(gstEMtoCM_monitorLED.commandRequestStatus == eACTIVE)
			{

				uartSendTxBuffer(UART_debug,"4",1);

				lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;

				// Passing command data to 'lstCommunicationModuleInnerTaskComm' is pending

				lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.monitorLED_control = 1;

				gstEMtoCM_monitorLED.commandResponseStatus = eWAITING;

			}

			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE; // inform 'handleControlCmdResp' about command

			sucHandleCommandUM_EM_EH_State = eSentOperationalCmdWaitingReply; // go to next state for processing the response

		} //if(
		  //		(
		  //				gstUMtoCMoperational.commandRequestStatus == eACTIVE ||
		  //				gstUMtoCMdatabase.commandRequestStatus == eACTIVE ||
		  //				gstEMtoCM_monitorLED.commandRequestStatus == eACTIVE ||
		  //				gstEMtoCM_monitorLED.gstEMtoCM_errorList == eACTIVE
		  //		) &&
		  //		(lstCommunicationModuleInnerTaskComm.commandRequestStatus == eINACTIVE)

		break; //case eIdleSendOperationCmd: // Send command through 'handleControlCmdResp'
#endif


	case eIdleSendOperationCmd: // Send command through 'handleDriveCmdResp'

		if(	(lstCommunicationModuleInnerTaskComm.commandRequestStatus == eINACTIVE)	)
		{

			if((gstUMtoCMoperational.commandRequestStatus == eACTIVE) && (gstUMtoCMoperational.commandResponseStatus == eNO_STATUS) )
			{


				lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;

				// Read command request from 'UM' and request to 'handleControlCmdResp' to send same

				if (gstUMtoCMoperational.commandToControlBoard.bits.autoManSelect)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.autoManualSelect = 1;
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.runControlBoard)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.runControlBoard = 1;
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.stopControlBoard)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.stopControlBoard = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.upPressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.upPressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.upReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.upReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.downPressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.downPressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.downReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.downReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.openPressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.openPressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.openReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.openReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.closePressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.closePressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.closeReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.closeReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.stopPressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.stopPressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.stopReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.stopReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.enterPressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.enterPressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.enterReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.enterReleased = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.modePressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.modePressed = 1; //
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.modeReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.modeReleased = 1;
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.startInstallation)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.startInstallation = 1;
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.systemInitComplete)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.systemInitComplete = 1;
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangePressed)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.wirelessModeChangePressed = 1;
				}
				else if (gstUMtoCMoperational.commandToControlBoard.bits.wirelessModeChangeReleased)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.wirelessModeChangeReleased = 1;
				}
				//	Added this command to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
				else if (gstUMtoCMoperational.commandToControlBoard.bits.settingsModeStatus)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.settingsModeStatus = 1;
					lstCommunicationModuleInnerTaskComm.additionalCommandData = gstUMtoCMoperational.additionalCommandData;
				}

				gstUMtoCMoperational.commandResponseStatus = eWAITING;

				lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE; // inform 'handleControlCmdResp' about command

				sucHandleCommandUM_EM_EH_State = eSentOperationalCmdWaitingReply; // go to next state for processing the response

			}
			else if((gstUMtoCMdatabase.commandRequestStatus == eACTIVE) && (gstUMtoCMdatabase.commandResponseStatus == eNO_STATUS))
			{


				// Removed below line and added in individual command cases on 07 Jul 14
//				lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;

				// Passing command data to 'lstCommunicationModuleInnerTaskComm' is pending

				if (gstUMtoCMdatabase.commandToControlBoard.bits.setTimeStamp)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.setTimeStamp = 1;

					//
					//	Copy time-stamp to inner task parameter value
					//
					lstCommunicationModuleInnerTaskComm.parameterValue = gstUMtoCMdatabase.dataToControlBoard.commandData.timeStamp;

					//	Added on 07 Jul 14 to incorporate destination ID for getParam command
					lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
				}
				else if (gstUMtoCMdatabase.commandToControlBoard.bits.getParameter)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 1;

					//	Added on 07 Jul 14 to incorporate destination ID for getParam command
					lstCommunicationModuleInnerTaskComm.destination = gstUMtoCMdatabase.destination;

					//
					//	Copy parameter number to inner task parameter number
					//
					lstCommunicationModuleInnerTaskComm.parameterNumber = gstUMtoCMdatabase.dataToControlBoard.parameterNumber;

				}
				else if (gstUMtoCMdatabase.commandToControlBoard.bits.setParameter)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.setParameter = 1;

					//
					//	Copy parameter number to inner task parameter number
					//
					lstCommunicationModuleInnerTaskComm.parameterNumber = gstUMtoCMdatabase.dataToControlBoard.parameterNumber;

					//
					//	Copy parameter value to inner task parameter value
					//
					lstCommunicationModuleInnerTaskComm.parameterValue = gstUMtoCMdatabase.dataToControlBoard.commandData.setParameterValue;

					//	Added on 07 Jul 14 to incorporate destination ID for setParam command
					lstCommunicationModuleInnerTaskComm.destination = gstUMtoCMdatabase.destination;
				}
				//
				//	Added on 06 Nov 14 to implement firmware upgrade command
				//
				else if (gstUMtoCMdatabase.commandToControlBoard.bits.firmwareUpgrade)
				{
					lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.firmwareUpgrade = 1;

					lstCommunicationModuleInnerTaskComm.destination = gstUMtoCMdatabase.destination;
				}


				lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE; // inform 'handleControlCmdResp' about command

				sucHandleCommandUM_EM_EH_State = eSentOperationalCmdWaitingReply; // go to next state for processing the response

				gstUMtoCMdatabase.commandResponseStatus = eWAITING;

			}
			else if((gstEMtoCM_errorList.commandRequestStatus == eACTIVE) && (gstEMtoCM_errorList.commandResponseStatus == eNO_STATUS))
			{


				lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;

				lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getError = 1;

				gstEMtoCM_errorList.commandResponseStatus = eWAITING;

				lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE; // inform 'handleControlCmdResp' about command

				sucHandleCommandUM_EM_EH_State = eSentOperationalCmdWaitingReply; // go to next state for processing the response

			}
			else if((gstEMtoCM_monitorLED.commandRequestStatus == eACTIVE) && (gstEMtoCM_monitorLED.commandResponseStatus == eNO_STATUS))
			{


				lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;

				// Passing command data to 'lstCommunicationModuleInnerTaskComm' is pending

				lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.monitorLED_control = 1;

				gstEMtoCM_monitorLED.commandResponseStatus = eWAITING;


				lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE; // inform 'handleControlCmdResp' about command

				sucHandleCommandUM_EM_EH_State = eSentOperationalCmdWaitingReply; // go to next state for processing the response

			}


		} //if(
		  //		(
		  //				gstUMtoCMoperational.commandRequestStatus == eACTIVE ||
		  //				gstUMtoCMdatabase.commandRequestStatus == eACTIVE ||
		  //				gstEMtoCM_monitorLED.commandRequestStatus == eACTIVE ||
		  //				gstEMtoCM_monitorLED.gstEMtoCM_errorList == eACTIVE
		  //		) &&
		  //		(lstCommunicationModuleInnerTaskComm.commandRequestStatus == eINACTIVE)

		break; //case eIdleSendOperationCmd: // Send command through 'handleControlCmdResp'

	case eSentOperationalCmdWaitingReply: // Process reply from 'handleControlCmdResp'

		if (lstCommunicationModuleInnerTaskComm.commandResponseStatus >= eSUCCESS &&
				lstCommunicationModuleInnerTaskComm.commandResponseStatus <= eFAIL)
		{

			if (gstUMtoCMoperational.commandResponseStatus == eWAITING)
			{
				// process the reply from 'handleDriveCmdResp' and pass it to 'Logic Solver'
				if (lstCommunicationModuleInnerTaskComm.uchRxBuffer[0] == ACK)
				{
					gstUMtoCMoperational.acknowledgementReceived = eACK;
//					uartSendTxBuffer(0,"R",1);	// debug
				}
				else
				{
					gstUMtoCMoperational.acknowledgementReceived = eNACK;
				}

				gstUMtoCMoperational.commandResponseStatus = lstCommunicationModuleInnerTaskComm.commandResponseStatus;

				lucCommandSupportReponseAck = 0;

			}
			else if(gstEMtoCM_errorList.commandResponseStatus == eWAITING)
			{
				// process the reply from 'handleControlCmdResp' and pass it to 'Error Module' (Copying anomaly history pending)
				gstEMtoCM_errorList.commandResponseStatus = lstCommunicationModuleInnerTaskComm.commandResponseStatus;

				lucCommandSupportReponseAck = 1;

			}
			else if(gstUMtoCMdatabase.commandResponseStatus == eWAITING)
			{
				// process the reply from 'handleControlCmdResp' and pass it to 'UM' (Copying Data pending)
				gstUMtoCMdatabase.commandResponseStatus = lstCommunicationModuleInnerTaskComm.commandResponseStatus;

				if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS)
				{
					if(lstCommunicationModuleInnerTaskComm.uchRxBuffer[0] == NACK)
					{
						gstUMtoCMdatabase.acknowledgementReceived = eNACK;
					}
					else
					{
						lunWord32.byte[3] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION];
						lunWord32.byte[2] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 1];
						lunWord32.byte[1] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 2];
						lunWord32.byte[0] = lstCommunicationModuleInnerTaskComm.uchRxBuffer[RESPONSE_DATA_POSITION + 3];

						gstUMtoCMdatabase.acknowledgementReceived = eACK;

						//	Added on 06 Nov 14 to implement firmware upgrade
						if(
								(gstUMtoCMdatabase.commandToControlBoard.bits.firmwareUpgrade == 1) &&
								(gui8FirmwareUpgradeCommandResponseReceived == 2)
						)
						{
							gui8FirmwareUpgradeCommandResponseReceived = 1;
						}
					}

					gstUMtoCMdatabase.getParameterValue = lunWord32.word.val;
				}

				lucCommandSupportReponseAck = 0;

			}
			else if (gstEMtoCM_monitorLED.commandResponseStatus == eWAITING)
			{
				// process the reply from 'handleDriveCmdResp' and pass it to 'Error Module (Led Handler)'
				if (lstCommunicationModuleInnerTaskComm.uchRxBuffer[0] == ACK)
				{
					gstEMtoCM_monitorLED.acknowledgementReceived = eACK;
//					uartSendTxBuffer(0,"R",1);	// debug
				}
				else
				{
					gstEMtoCM_monitorLED.acknowledgementReceived = eNACK;
				}

				gstEMtoCM_monitorLED.commandResponseStatus = lstCommunicationModuleInnerTaskComm.commandResponseStatus;

				lucCommandSupportReponseAck = 0;

			}


			// Reset the state machine, if command not support response ack or command response fail or timeout
			if (
					(lstCommunicationModuleInnerTaskComm.commandResponseStatus >= eTIME_OUT &&
							lstCommunicationModuleInnerTaskComm.commandResponseStatus <= eFAIL) ||
					(lucCommandSupportReponseAck == 0)
			   )
			{

				lstCommunicationModuleInnerTaskComm.commandToControlBoard.val = 0;

				lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
				lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;

				sucHandleCommandUM_EM_EH_State = eIdleSendOperationCmd; // Command processing over, now reset the state machine

			} // Reset the state machine, if command not support response ack or command response fail or timeout
			else
			{

				sucHandleCommandUM_EM_EH_State = eWaitingOperationalCmdReplyAck; // Command reply require an acknowledgment. Go next state to wait and process same

				/*****************************************************************************/
				/*added later as getError command was getting fired twice*/
				lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
				/*****************************************************************************/

			}

		} //if (lstCommunicationModuleInnerTaskComm.commandResponseStatus >= eSUCCESS && lstCommunicationModuleInnerTaskComm.commandResponseStatus <= eFAIL)

		break; //case eSentOperationalCmdWaitingReply: // Process reply from 'handleDriveCmdResp'

	case eWaitingOperationalCmdReplyAck: // Wait for command response acknowledgment

		if (gstEMtoCM_errorList.commandResponseACK_Status == eResponseAcknowledgement_ACK)
		{

			lstCommunicationModuleInnerTaskComm.commandResponseACK_Status = eResponseAcknowledgement_ACK;

			/*****************************************************************************/
			/*added later as getError command was getting fired twice*/
			gstEMtoCM_errorList.commandResponseStatus = eNO_STATUS;
			/*****************************************************************************/

			sucHandleCommandUM_EM_EH_State = eSentOperationalCmdReplyAckWaitingReply; // Sent operational command reply ack. Now wait for the reply
//			uartSendTxBuffer(0,"A",1);	// debug

		}
		else if (gstEMtoCM_errorList.commandResponseACK_Status == eResponseAcknowledgement_NACK)
		{
#if 0
			// To not send Response Acknowledgment as NACK to Drive board
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.val = 0;

			lstCommunicationModuleInnerTaskComm.commandResponseStatus == eNO_STATUS;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;

			sucHandleCommandUM_EM_EH_State = eIdleSendOperationCmd; // Command processing over, now reset the state machine

			gstEMtoCM_errorList.commandResponseACK_Status = eResponseACKProcessed;
#endif

#if 1
			// To send Response Acknowledgment as NACK to Drive board
			lstCommunicationModuleInnerTaskComm.commandResponseACK_Status = eResponseAcknowledgement_NACK;

			sucHandleCommandUM_EM_EH_State = eSentOperationalCmdReplyAckWaitingReply; // Sent operational command reply ack. Now wait for the reply
#endif
		}

		break; //case eWaitingOperationalCmdReplyAck: // Wait for command response acknowledgment

	case eSentOperationalCmdReplyAckWaitingReply: //Sent operational command reply acknowledgment now wait for the reply

		if (lstCommunicationModuleInnerTaskComm.commandResponseACK_Status == eResponseAcknowledgementProcessed)
		{

			lstCommunicationModuleInnerTaskComm.commandToControlBoard.val = 0;

			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;

			sucHandleCommandUM_EM_EH_State = eIdleSendOperationCmd; // Command processing over, now reset the state machine

			gstEMtoCM_errorList.commandResponseACK_Status = eResponseAcknowledgementProcessed;

			/*****************************************************************************/
			/*added later as getError command was getting fired twice*/
			lstCommunicationModuleInnerTaskComm.commandResponseACK_Status = eNO_StatusAcknowledgement;
			/*****************************************************************************/
//			uartSendTxBuffer(0,"D",1);	// debug

		}

		break;

	default:

		break;

	} //switch(suchandleCmdLogicSolverErrModCMDiState)

} // handleCommandLS_EM_CMDi



/******************************************************************************
 * communicationModuleControlBoard
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
void communicationModuleControlBoard (void)
{
	static uint32_t communicationModuleDelay;
	static enum functionState communicaionModuleState = eCaptureTime;

	if(communicaionModuleState == eCaptureTime)
	{
		communicationModuleDelay = g_ui32TickCount;
		communicaionModuleState = eWait;
	}
	else if(communicaionModuleState == eWait)
	{
		if(get_timego( communicationModuleDelay) >= COMM_MODULE_WAIT_TIME)
		{
			communicaionModuleState = eExecute;
		}
	}
	else if(communicaionModuleState == eExecute)
	{
		handleCommandFromUM_EM_EH();
		pollDriveControlStatusFault();
		handleControlCmdResp();
	}
}


/******************************************************************************
 * userModuleToTestCM
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

void userModuleToTestCM(void)
{
	if(gstUMtoCMoperational.commandResponseStatus == eSUCCESS ||
			gstUMtoCMoperational.commandResponseStatus == eFAIL ||
			gstUMtoCMoperational.commandResponseStatus == eTIME_OUT)
	{
		gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
		gstUMtoCMoperational.commandToControlBoard.val = 0;
		gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
	}

	if(gstUMtoCMdatabase.commandResponseStatus == eSUCCESS ||
			gstUMtoCMdatabase.commandResponseStatus == eFAIL ||
			gstUMtoCMdatabase.commandResponseStatus == eTIME_OUT)
	{
		gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
		gstUMtoCMdatabase.commandToControlBoard.val = 0;
		gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
	}
}

