/*********************************************************************************
 * FileName: cmdi.c
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


/****************************************************************************
 *  Modification History
 *
 *  Revision		Date                  Name          			Comments
 *  	0.5D	25/07/2014			iGATE Offshore team			Some more checks added to validate received data packets
 *  	0.4D	10/07/2014			iGATE Offshore team			Added a flag (sucCmdiDHBlockInitiatedByCMDi) to check who initiated CMDiDH command
 *  	0.3D	03/07/2014			iGATE Offshore team			Patch added to handle DHtoCMDi fail response status
 *  	0.2D	19/06/2014			iGATE Offshore team			Packet length changed from data length to complete frame length
 *  	0.1D	06/06/2014      	iGATE Offshore team       	Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Include:
 ****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "cmdr.h"
#include "cmdi.h"
#include <driverlib/gpio.h>
#include "intertaskcommunication.h"
#include "Middleware/serial.h"
#include "Drivers/systicktimer.h"
#include "driverlib/sw_crc.h"
#include "Middleware/debounce.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/
#define OPERATION_COUNT_RESET	20
#define OPERATION_COUNT_INPUT	28


uint8_t flag_addlogin=0;
uint16_t gsActiveAnomalylist[20];
static uint8_t gsActiveAnomalylist_count_cyw = 0;

/****************************************************************************/

/****************************************************************************
 *  extern:
****************************************************************************/
extern uint8_t guchUARTMapToGlobalBuffer[MAX_UART_AVAILABLE];
extern uint8_t flag_loginok;
extern  _CMDrInnerTaskComm lstCMDrInnerTaskComm;
/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/
// Display to Control communication error occurrence count
uint8_t guchCMDiCRC_ErrorOccurrence[UART_CHANNEL_COUNT];
uint32_t get_timego(uint32_t x_data_his);

uint8_t his_addlogin_step=0;
uint32_t his_rise_time = 0;
uint8_t  Clear_E111_FLAG = 0;
/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
 ****************************************************************************/


/****************************************************************************/


/******************************************************************************
 * communicationModuleDisplayToTestCMDr
 *
 * Function Description:
 * This function is temporarily created to test CMDr. It clears the CMdi to CMDr
 * command being activated in main.c file for functional testing.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
//uint8_t cyw_jump = 0;
void communicationModuleDisplayToTestCMDr(void)
{
	if(gstCMDitoCMDr.commandResponseStatus == eSUCCESS ||
			gstCMDitoCMDr.commandResponseStatus == eTIME_OUT ||
			gstCMDitoCMDr.commandResponseStatus == eFAIL)	// Active command processed
	{

		if(gstCMDitoCMDr.commandResponseStatus == eSUCCESS)
		{

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
		}

		if(gstCMDitoCMDr.commandResponseStatus == eTIME_OUT)
		{
			gstControlBoardStatus.bits.controlFault = 1;
			gstControlBoardFault.bits.controlCommunication = 1;
			gstControlCommunicationFault.bits.commFailDrive = 1;
		}

		if(gstCMDitoCMDr.commandResponseStatus == eFAIL)
		{
			gstControlBoardStatus.bits.controlFault = 1;
			gstControlBoardFault.bits.controlCommunication = 1;
			gstControlCommunicationFault.bits.crcErrorDrive = 1;
		}

		gstCMDitoCMDr.commandRequestStatus = eINACTIVE;
		gstCMDitoCMDr.commandResponseStatus = eNO_STATUS;	// Clear the processed command
	}
}

/********************************************************************************/

/******************************************************************************
 * communicationModuleDisplay
 *
 * Function Description:
 * This function receives commands from the Display board and sends response
 * to the display
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void communicationModuleDisplay(void)
{
	enum functionState
	{
		eIdle = 0,
		eWaitBeforeSendingResponse,
		eCommandReceivedFromDisplayWaitingForReply,
		eCommandACK_ReceivedFromDisplayWaitingForReply,
	};
   uint8_t Tp_cywsend[6]={0x00,0x01,0x06,0x38,0xb2,0x53};
	static uint32_t slui32CaptureTimeAtCommandReceive = 0;

	static unsigned char sucCommunicationModuleDisplayState = eIdle;

	uint8_t lucTemp = 0;

	static uint8_t sucRxIndex = 0;
	static uint8_t sucRxCount = 0;
	static uint8_t sucRxDataLength = 0;

	static uint8_t sucaCommandBuffer[RECEIVE_BUFFER_SIZE];

	static uint32_t suiFirstByteReceivedTime = 0;

	uint16_t lucCalculatedCRC = 0;
	uint16_t lucReceivedCRC = 0;

	uint16_t lucResponseCRC = 0;

	static uint8_t sucaResponseBuffer[TRANSMIT_BUFFER_SIZE];
	static uint8_t sucResponsePacketLength = 0;
	static uint8_t sucResponseDataLength = 0;

	static uint8_t sucCmdiDHBlockInitiatedByCMDi = 0;
	/*
		0 = CMDi to DH block is initiated by Logic Solver since LogicSolver to Database Handler block dose not exist
		1 = CMDI to DH block initiated by CMDi itself
	*/

	union ui16TempData lunWord16;
	union ui32TempData lunWord32;

	uint32_t tp_i=0;

	uint8_t lucCommandSupportReponseAck = 0;

	switch (sucCommunicationModuleDisplayState)
	{
	case eIdle:
		//
		//	Check whether there is data received in global receive buffer for Display Board
		//
		if(
				(0 == uartCheckNotFreeRxBuffer(UART_display,&lucTemp)) &&
				(eINACTIVE == gstCMDitoDH.commandRequestStatus)
		)
		{
			while(lucTemp)

			{

				uint8_t lucGetRxBuffData = 0;


				//
				//	Copy data from global receive buffer for display into local buffer
				//
				//uartGetRxBuffer(UART_display,&sucaCommandBuffer[sucRxIndex++]);
				uartGetRxBuffer(UART_display,&lucGetRxBuffData);

				if (((sucRxCount != 0) ||(sucRxCount == 0 &&(lucGetRxBuffData == CONTROL_BOARD_ADDRESS ||lucGetRxBuffData == DRIVE_BOARD_ADDRESS))) &&(sucRxCount < RECEIVE_BUFFER_SIZE))
				{

					if (sucRxCount == 0)
					{
					//
					//	A first byte is received at this time
					//
					suiFirstByteReceivedTime = g_ui32TickCount;
					}

					// Copy valid data in local static buffer
					sucaCommandBuffer[sucRxIndex++] = lucGetRxBuffData;

					//
					//	Increment "Received Bytes" count
					//
					sucRxCount++;

					//
					//	Compute data length
					//
					if(sucRxIndex == FRAME_LENGTH_POSITION + 1)
					{
						//
						//	Copy received data byte length into local buffer
						//
						sucRxDataLength = sucaCommandBuffer[FRAME_LENGTH_POSITION];

						//validate length possible min and max length. If not found correct then capturing the packet from start
						if (sucRxDataLength < MIN_COMMAND_LENGTH || sucRxDataLength > MAX_COMMAND_LENGTH)
						{
							sucRxCount = 0;
							sucRxIndex = 0;
							sucRxDataLength = 0;
						}

					} //if(sucRxIndex == FRAME_LENGTH_POSITION + 1)

				} //

				//
				//	Check whether the next byte is available
				//
				uartCheckNotFreeRxBuffer(UART_display,&lucTemp);
			}

			//
			//	Check whether valid data is received or receive line idle for n byte receive time
			//
			if(
					(
							(sucRxCount) &&
							(
									sucRxCount == sucRxDataLength  	/*||*/	//	Complete packet received
									//sucaCommandBuffer[0] == ACK 	||		//	ACK received
									//sucaCommandBuffer[0] == NACK 	||		//	NACK received
									//(g_ui32TickCount - suiByteReceivedTime) >= (ONE_BYTE_RECEIVE_TIME * 5) //	No data byte received for 5 bytes receive time
							)
					) /*&&
					// All Destination intertask module to whom command need to sent is free
					(gstCMDitoLS.commandRequestStatus == eINACTIVE && gstCMDitoDH.commandRequestStatus == eINACTIVE && gstCMDitoCMDr.commandRequestStatus == eINACTIVE)*/
			)
			{
				if(sucaCommandBuffer[0] == ACK ||
				   sucaCommandBuffer[0] ==  NACK)
				{
					if(sucaCommandBuffer[0] == ACK)
					{
						gstCMDitoDH.commandResponseACK_Status= eResponseAcknowledgement_ACK;
					}
					else
					{
						gstCMDitoDH.commandResponseACK_Status = eResponseAcknowledgement_NACK;
					}
					//
					//	Update function state
					//
					sucCommunicationModuleDisplayState = eCommandACK_ReceivedFromDisplayWaitingForReply;
					//
					//	Reset receive buffer write index
					//
					sucRxIndex = 0;
				}
				else if((sucRxCount == sucRxDataLength) && 	(sucRxDataLength > 1))	//Added - 22 Jul 14- Girish //	Validate received packet with CRC check
				{
					//
					//	Copy received CRC to local union
					//
					/*lucReceivedCRC = sucaCommandBuffer[sucRxCount - 2];
					lucReceivedCRC <<= 8;
					lucReceivedCRC |= sucaCommandBuffer[sucRxCount - 1];*/
					lunWord16.byte[1] = sucaCommandBuffer[sucRxCount - 2];
					lunWord16.byte[0] = sucaCommandBuffer[sucRxCount - 1];

					//
					//	Copy received CRC from local union to local 2 Byte buffer
					//
					lucReceivedCRC = lunWord16.halfWord.val;

					//
					//	Calculate CRC of the received packet
					//
					lucCalculatedCRC = Crc16(0,sucaCommandBuffer,sucRxCount-2);

					if(lucCalculatedCRC == lucReceivedCRC)	//	CRC passed
					{
						//
						// Clear display to control communication CRC error flag
						//
						if(gstControlCommunicationFault.bits.crcErrorDisplay == 1)
						{
							gstControlCommunicationFault.bits.crcErrorDisplay = 0;
						}

						if(gstControlCommunicationFault.bits.uartErrorDisplay == 1)
						{
							gstControlCommunicationFault.bits.uartErrorDisplay = 0;
						}

						if(0 == gstControlCommunicationFault.val)
						{
							gstControlBoardFault.bits.controlCommunication = 0;
						}

						if(0 == gstControlBoardFault.val)
						{
							gstControlBoardStatus.bits.controlFault = 0;
						}

						//
						//	Check destination ID and append command to respective board
						//
						if(sucaCommandBuffer[DESTINATION_ADDRESS_POSITION] == CONTROL_BOARD_ADDRESS)	// Command for control board
						{
							switch(sucaCommandBuffer[COMMAND_ID_POSITION])
							{
							case AUTO_MAN_SELECT:
								gstCMDitoLS.commandDisplayBoardLS.bits.autoManSel = 1;
								//flag_addlogin=1;
								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case RUN_CONTROL_BOARD:
								gstCMDitoLS.commandDisplayBoardLS.bits.runControlBoard = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case STOP_CONTROL_BOARD:
								gstCMDitoLS.commandDisplayBoardLS.bits.stopControlBoard = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case UP_PRESSED:
								gstCMDitoLS.commandDisplayBoardLS.bits.upPressed = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case UP_RELEASED:
								gstCMDitoLS.commandDisplayBoardLS.bits.upReleased = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case DOWN_PRESSED:
								gstCMDitoLS.commandDisplayBoardLS.bits.downPressed = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case DOWN_RELEASED:
								gstCMDitoLS.commandDisplayBoardLS.bits.downReleased = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case OPEN_PRESSED:

								gstCMDitoLS.commandDisplayBoardLS.bits.openPressed = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case OPEN_RELEASED:

								gstCMDitoLS.commandDisplayBoardLS.bits.openReleased = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case CLOSE_PRESSED:

								gstCMDitoLS.commandDisplayBoardLS.bits.closePressed = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case CLOSE_RELEASED:
								gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case STOP_PRESSED:
								gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed = 1;
								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case STOP_RELEASED:
								gstCMDitoLS.commandDisplayBoardLS.bits.stopReleased = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case ENTER_PRESSED:
								gstCMDitoLS.commandDisplayBoardLS.bits.enterPressed = 1;
								uartSendTxBuffer(UART_drive,Tp_cywsend,6);
								if(gstControlApplicationFault.bits.operationRestrictionTimer == 1)
								{
								gstControlApplicationFault.bits.operationRestrictionTimer = 0;
								gstControlBoardFault.bits.controlApplication = 0;
								gstControlBoardStatus.bits.controlFault = 0;
								Clear_E111_FLAG = 1;
								}

								//for(tp_i=0;tp_i<100;tp_i++);
								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case ENTER_RELEASED:
								gstCMDitoLS.commandDisplayBoardLS.bits.enterReleased = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case GET_PARAMETER_CMD_FROM_DISPLAY:
								gstCMDitoDH.commandDisplayBoardDH.bits.getParameter = 1;
								/*gstCMDitoDH.commandDataCMDiDH.parameterNumber = sucaCommandBuffer[COMMAND_ID_POSITION+1];
								gstCMDitoDH.commandDataCMDiDH.parameterNumber <<= 8;
								gstCMDitoDH.commandDataCMDiDH.parameterNumber |= sucaCommandBuffer[COMMAND_ID_POSITION+2];*/
								lunWord16.byte[1] = sucaCommandBuffer[COMMAND_ID_POSITION+1];
								lunWord16.byte[0] = sucaCommandBuffer[COMMAND_ID_POSITION+2];

								gstCMDitoDH.commandDataCMDiDH.parameterNumber = lunWord16.halfWord.val;

								sucCmdiDHBlockInitiatedByCMDi = 1;

								gstCMDitoDH.commandRequestStatus = eACTIVE;

								break;
							case SET_PARAMETER_CMD_FROM_DISPLAY:
								// Set flag indicating setParameter command is received
								if(gstControlBoardStatus.bits.runStop == 1)
								{
									//	Commented on 3 Nov 2014 to avoid control board going into stop mode during execution of
									//	setParameter command
//									gucSetParameterCommandFlag = 1;
								}
								gstCMDitoDH.commandDisplayBoardDH.bits.setParameter = 1;
								/*gstCMDitoDH.commandDataCMDiDH.parameterNumber = sucaCommandBuffer[COMMAND_ID_POSITION+1];
								gstCMDitoDH.commandDataCMDiDH.parameterNumber <<= 8;
								gstCMDitoDH.commandDataCMDiDH.parameterNumber |= sucaCommandBuffer[COMMAND_ID_POSITION+2];*/

								lunWord16.byte[1] = sucaCommandBuffer[COMMAND_ID_POSITION+1];
								lunWord16.byte[0] = sucaCommandBuffer[COMMAND_ID_POSITION+2];

								gstCMDitoDH.commandDataCMDiDH.parameterNumber = lunWord16.halfWord.val;

								sucCmdiDHBlockInitiatedByCMDi = 1;

								lunWord32.byte[3] = sucaCommandBuffer[COMMAND_ID_POSITION+3];
								lunWord32.byte[2] = sucaCommandBuffer[COMMAND_ID_POSITION+4];
								lunWord32.byte[1] = sucaCommandBuffer[COMMAND_ID_POSITION+5];
								lunWord32.byte[0] = sucaCommandBuffer[COMMAND_ID_POSITION+6];

								gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = lunWord32.word.val;

								sucCmdiDHBlockInitiatedByCMDi = 1;

								gstCMDitoDH.commandRequestStatus = eACTIVE;
								break;
							case SET_TIMESTAMP:
								gstCMDitoDH.commandDisplayBoardDH.bits.setTimeStamp = 1;

								/*gstCMDitoDH.commandDataCMDiDH.commandData.timeStamp = sucaCommandBuffer[COMMAND_ID_POSITION+1];
								gstCMDitoDH.commandDataCMDiDH.parameterNumber <<= 8;
								gstCMDitoDH.commandDataCMDiDH.parameterNumber |= sucaCommandBuffer[COMMAND_ID_POSITION+2];
								gstCMDitoDH.commandDataCMDiDH.parameterNumber <<= 8;
								gstCMDitoDH.commandDataCMDiDH.parameterNumber |= sucaCommandBuffer[COMMAND_ID_POSITION+3];
								gstCMDitoDH.commandDataCMDiDH.parameterNumber <<= 8;
								gstCMDitoDH.commandDataCMDiDH.parameterNumber |= sucaCommandBuffer[COMMAND_ID_POSITION+4];
								gstCMDitoDH.commandDataCMDiDH.parameterNumber <<= 8;
								 */

								lunWord32.byte[3] = sucaCommandBuffer[COMMAND_ID_POSITION+1];
								lunWord32.byte[2] = sucaCommandBuffer[COMMAND_ID_POSITION+2];
								lunWord32.byte[1] = sucaCommandBuffer[COMMAND_ID_POSITION+3];
								lunWord32.byte[0] = sucaCommandBuffer[COMMAND_ID_POSITION+4];

								gstCMDitoDH.commandDataCMDiDH.commandData.timeStamp = lunWord32.word.val;

								sucCmdiDHBlockInitiatedByCMDi = 1;

								gstCMDitoDH.commandRequestStatus = eACTIVE;
								break;
							case FIRMWARE_UPGRADECMD_FROM_DISPLAY:
								gstCMDitoDH.commandDisplayBoardDH.bits.firmwareUpgrade = 1;

								sucCmdiDHBlockInitiatedByCMDi = 1;

								gstCMDitoDH.commandRequestStatus = eACTIVE;
								break;
							case START_INSTALLATION_CMD_FROM_DISPLAY:
								gstCMDitoLS.commandDisplayBoardLS.bits.startInstallation = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;

								break;
							case START_APERTUREHEIGHT_CMD_FROM_DISPLAY:
								gstCMDitoLS.commandDisplayBoardLS.bits.start_apertureHeight = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;

								break;
							case MONITOR_LED_CONTROL:
								gstCMDitoMLH.commandDisplayBoardMLH.bits.monitorLEDControl = 1;
								gstCMDitoMLH.additionalCommandData = sucaCommandBuffer[COMMAND_ID_POSITION+1];

								gstCMDitoMLH.commandRequestStatus = eACTIVE;
								break;
							case GET_ERROR_LIST_CMD_FROM_DISPLAY:
								//memset((uint8_t *)gsActiveAnomalylist,0,40);
								//gsActiveAnomalylist_count_cyw  = 0;

								gstCMDitoDH.commandDisplayBoardDH.bits.getErrorList = 1;

								sucCmdiDHBlockInitiatedByCMDi = 1;

								gstCMDitoDH.commandRequestStatus = eACTIVE;
								break;

							case SYSTEM_INIT_COMPLETE:

								gucSystemInitComplete = 1;

								break;

							case WIRELESS_MODE_CHANGE_PRESSED:	//	Added on 04 Dec for new requirement from client
								gstCMDitoLS.commandDisplayBoardLS.bits.wirelessModeChangePressed = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case WIRELESS_MODE_CHANGE_RELEASED:	//	Added on 04 Dec for new requirement from client
								gstCMDitoLS.commandDisplayBoardLS.bits.wirelessModeChangeReleased = 1;

								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;

							//	Added this command to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
							case SETTINGS_MODE_STATUS:
								gstCMDitoLS.commandDisplayBoardLS.bits.settingsModeStatus = 1;
								gstCMDitoLS.additionalCommandData = sucaCommandBuffer[COMMAND_ID_POSITION+1];
								gstCMDitoLS.commandRequestStatus = eACTIVE;
								break;
							case ADD_LOGIN:
							//	gstCMDitoLS.commandRequestStatus = eACTIVE;
								if(sucaCommandBuffer[COMMAND_ID_POSITION+1]==0x01)
								{
								      flag_addlogin=1;
								      his_addlogin_step = flag_addlogin;
								      his_rise_time = g_ui32TickCount;
								      gstCMDitoLS.commandResponseStatus = eSUCCESS ;
								      gstCMDitoLS.acknowledgementReceived = eACK;
								      flag_loginok = 0;
								}
								if(sucaCommandBuffer[COMMAND_ID_POSITION+1]==0x02)
								{
									   flag_addlogin=14;
									   his_addlogin_step = flag_addlogin;
									   his_rise_time = g_ui32TickCount;
									   gstCMDitoLS.commandResponseStatus = eSUCCESS ;
									   gstCMDitoLS.acknowledgementReceived = eACK;
									   flag_loginok = 0;
								}
								if(sucaCommandBuffer[COMMAND_ID_POSITION+1]==0x03)
								{
									   flag_addlogin=17;
									   gstCMDitoLS.commandResponseStatus = eSUCCESS ;
									   gstCMDitoLS.acknowledgementReceived = eACK;
								}
								if(sucaCommandBuffer[COMMAND_ID_POSITION+1]==0x04)
								{
									   if(flag_loginok == 1)
									   {
										   flag_loginok = 0;
										   gstCMDitoLS.commandResponseStatus = eSUCCESS ;
										   gstCMDitoLS.acknowledgementReceived = eACK;
									   }
									   else
									   {
										   flag_loginok = 0;
										   gstCMDitoLS.commandResponseStatus = eSUCCESS ;
										   gstCMDitoLS.acknowledgementReceived = eNACK;
										  // cyw_jump = 1;

									   }
									  if( get_timego(his_rise_time) >3500)
									  {
										  flag_addlogin = his_addlogin_step;
										  his_rise_time = g_ui32TickCount;
									  }
								}
							//	uartSendTxBuffer(UART_drive,Tp_cywsend,6);

								break;
							default:
								break;
							}
						}	// Command for control board
						else if(sucaCommandBuffer[DESTINATION_ADDRESS_POSITION] == DRIVE_BOARD_ADDRESS)	// Pass through command
						{
							//
							//	Update command buffer of CMDiToCMDr block
							//
							for(lucTemp=0;lucTemp<sucRxCount;lucTemp++)
							{
								gstCMDitoCMDr.transmitCommandPacket[lucTemp] = sucaCommandBuffer[lucTemp];
							}

							//
							//	Update transmit buffer length of CMDiToCMDr block
							//
							gstCMDitoCMDr.transmitCommandPacketLen = sucRxCount;

#ifndef SOURCE_AND_DESTINATION_ADDRESS_CHANGE
							//
							//	Code added on 17 Jul to enable drive board code to handle getParameter or setParameter commands
							//	coming from display board. This code changes the source address of the command coming from display
							//	board. When command arrives from display, the source address of the packet is of display board (0x02).
							//	It is changed to control board address (0x01) and then CRC of the complete command packet is recomputed.
							//

							//
							// change source address
							//
							gstCMDitoCMDr.transmitCommandPacket[SOURCE_ADDRESS_POSITION] = CONTROL_BOARD_ADDRESS;

							//
							//	change getParameter and setParameter command ID
							//
							if(gstCMDitoCMDr.transmitCommandPacket[COMMAND_ID_POSITION] == GET_PARAMETER_CMD_FROM_DISPLAY)
							{
								gstCMDitoCMDr.transmitCommandPacket[COMMAND_ID_POSITION] = GET_PARAMETER_CMD_FROM_CONTROL;
							}

							if(gstCMDitoCMDr.transmitCommandPacket[COMMAND_ID_POSITION] == SET_PARAMETER_CMD_FROM_DISPLAY)
							{
								gstCMDitoCMDr.transmitCommandPacket[COMMAND_ID_POSITION] = SET_PARAMETER_CMD_FROM_CONTROL;

								// Set flag indicating setParameter command is received for drive board
								if(gstControlBoardStatus.bits.runStop == 1)
								{
									//	Commented on 3 Nov 2014 to avoid control board going into stop mode during execution of
									//	setParameter command
//									gucSetParameterCommandFlag = 1;
								}
							}

							//
							// Recompute the checksum
							//
							lucCalculatedCRC = Crc16(0,gstCMDitoCMDr.transmitCommandPacket,gstCMDitoCMDr.transmitCommandPacketLen-2);
							lunWord16.halfWord.val = lucCalculatedCRC;
							gstCMDitoCMDr.transmitCommandPacket[gstCMDitoCMDr.transmitCommandPacketLen-2] = lunWord16.byte[1];
							gstCMDitoCMDr.transmitCommandPacket[gstCMDitoCMDr.transmitCommandPacketLen-1] = lunWord16.byte[0];
#endif	// SOURCE_AND_DESTINATION_ADDRESS_CHANGE

							//
							//	Set CMDitoCMDr command request status to be active
							//
							if(gstCMDitoCMDr.commandRequestStatus == eINACTIVE)	// Check if a previous command is pending
							{
								gstCMDitoCMDr.commandRequestStatus = eACTIVE;
							}
						}	// Pass through command
						//
						//	Command data processed
						//	Reset receive index
						//
						sucRxIndex = 0;

						//
						//	Command data processed
						//	Change function state
						//
						slui32CaptureTimeAtCommandReceive = g_ui32TickCount;
						sucCommunicationModuleDisplayState = eWaitBeforeSendingResponse;//eCommandReceivedFromDisplayWaitingForReply;

						if((gstCMDitoCMDr.transmitCommandPacket[5]==0x19)&&(gstCMDitoCMDr.transmitCommandPacket[4]==0x02))    //A537-->shutter type(BEAD/M1/M2)     //20170421   201703_No.39
						{
						    if(gstControlProcessorFault.bits.watchdog==1)
						    {
							   gstControlProcessorFault.bits.watchdog = 0;
							   gstControlBoardFault.bits.controlProcessor = 0;
							   gstControlBoardStatus.bits.controlFault = 0;
						    }
						}
					}	// CRC passed
					else
					{	//	CRC failed

						//
						// Set display to control communication CRC error flag
						//
						gstControlCommunicationFault.bits.crcErrorDisplay = 1;
						gstControlBoardFault.bits.controlCommunication = 1;
						gstControlBoardStatus.bits.controlFault = 1;


						guchCMDiCRC_ErrorOccurrence[guchUARTMapToGlobalBuffer[UART_display]] = 1;
						//
						//	Reset receive index
						//
						sucRxIndex = 0;
					}	//	CRC failed
				}	//	Validate received packet with CRC check
				else
				{
					sucRxIndex = 0; // Added - Girish 22 Jul 2014 - Reset Index if none condition matches
				}


				//
				//	Clear receive count
				//
				sucRxCount = 0;
				sucRxDataLength = 0;

			}	// Complete packet received or ACK or NACK or receive line idle for n byte receive time
			// check for command receive timeout
			else if (
					(sucRxCount != 0) &&
					(get_timego( suiFirstByteReceivedTime) >= COMMAND_RxTIME_OUT)
					)
			{

				sucRxCount = 0;
				sucRxIndex = 0;
				sucRxDataLength = 0;

			}

		}
		break;	//	eIdle

	case eWaitBeforeSendingResponse:
		{
			if(get_timego( slui32CaptureTimeAtCommandReceive) >= 2)
			{
				sucCommunicationModuleDisplayState = eCommandReceivedFromDisplayWaitingForReply;
			}
		}
			break;
	case eCommandReceivedFromDisplayWaitingForReply:
		//
		//	A command flag from CMDitoCMDr or CMDitoLS or CMDitoEM or CMDitoDH set
		//	Waiting for reply form the corresponding module
		//

		if(		gstCMDitoLS.commandResponseStatus == eSUCCESS 	||
				gstCMDitoLS.commandResponseStatus == eTIME_OUT 	||
				gstCMDitoLS.commandResponseStatus == eFAIL		||
				gucSystemInitComplete == 1
		  )
		{

			// On 28 April added following check to send ACK to System Init command directly as it is not refer to any task
			if (gucSystemInitComplete == 1)
			{

						gucSystemInitComplete = 2;
						sucaResponseBuffer[sucResponsePacketLength++] = ACK;
						uartSendTxBuffer(UART_display,sucaResponseBuffer,sucResponsePacketLength);

			}
			else if(gstCMDitoLS.commandResponseStatus == eSUCCESS)
			{
				if(gstCMDitoLS.acknowledgementReceived == eACK)
				{
					//
					//	Respond to display board with ACK
					//

					sucaResponseBuffer[sucResponsePacketLength++] = ACK;

				}
				else if(gstCMDitoLS.acknowledgementReceived == eNACK)
				{
					//
					//	Respond to display board with NACK
					//
					sucaResponseBuffer[sucResponsePacketLength++] = NACK;
				}

				//
				//	Send response to display board
				//
				uartSendTxBuffer(UART_display,sucaResponseBuffer,sucResponsePacketLength);
			}
			else if(gstCMDitoLS.commandResponseStatus == eTIME_OUT ||
					gstCMDitoLS.commandResponseStatus == eFAIL)
			{
				//
				//	Respond to display board with NACK
				//
				sucaResponseBuffer[sucResponsePacketLength++] = NACK;

				//
				//	Send response to display board
				//
				uartSendTxBuffer(UART_display,sucaResponseBuffer,sucResponsePacketLength);
			}
			//
			//	Reset transmit packet length
			//
			sucResponsePacketLength = 0;

			//
			//	Response_ACK not supported
			//	Release gstCMDitoLS
			//
			gstCMDitoLS.commandRequestStatus = eINACTIVE;
			gstCMDitoLS.commandResponseStatus = eNO_STATUS;

			//
			//	Clear command register
			//
			gstCMDitoLS.commandDisplayBoardLS.val = 0;

			//
			//	Reset function state to eIdle
			//
			sucCommunicationModuleDisplayState = eIdle;

		}	//	if(gstCMDitoLS.commandResponseStatus == eSUCCESS)

		if(gstCMDitoMLH.commandResponseStatus == eSUCCESS)	// eFAIL not expected from DH
		{
			if(gstCMDitoMLH.acknowledgmentReceived == eACK)
			{
				//
				//	Respond to display board with ACK
				//
				sucaResponseBuffer[sucResponsePacketLength++] = ACK;
			}
			if(gstCMDitoMLH.acknowledgmentReceived == eNACK)
			{
				//
				//	Respond to display board with NACK
				//
				sucaResponseBuffer[sucResponsePacketLength++] = NACK;
			}
			//
			//	Send response to display board
			//
			uartSendTxBuffer(UART_display,sucaResponseBuffer,sucResponsePacketLength);

			//
			//	Reset Response packet length
			//
			sucResponsePacketLength = 0;
			//
			//	Response_ACK not supported
			//	Release gstCMDitoMLH
			//
			gstCMDitoMLH.commandRequestStatus = eINACTIVE;
			gstCMDitoMLH.commandResponseStatus = eNO_STATUS;

			//
			//	Clear command register
			//
			gstCMDitoMLH.commandDisplayBoardMLH.val = 0;

			//
			//	Reset function state to eIdle
			//
			sucCommunicationModuleDisplayState = eIdle;

		}	//	if(gstCMDitoMLH.commandResponseStatus == eSUCCESS)

		if(gstCMDitoCMDr.commandResponseStatus == eSUCCESS)
		{
			//
			//	Copy response packet for Display board from
			//	gstCMDitoCMDr.receiveCommandPacket to local
			//	buffer
			//
			memcpy((void*)sucaResponseBuffer,(const void*)gstCMDitoCMDr.receiveCommandPacket,gstCMDitoCMDr.receiveCommandPacketLen);

#ifndef SOURCE_AND_DESTINATION_ADDRESS_CHANGE
			//
			//	Code added on 17 Jul to enable drive board code to handle getParameter or setParameter commands
			//	coming from display board. This code changes the destination address of the response coming from drive
			//	board. When response arrives from drive, the destination address of the packet is of control board (0x01).
			//	It is changed to display board address (0x02) and then CRC of the complete response packet is recomputed.
			//

			if(gstCMDitoCMDr.receiveCommandPacketLen > 1 &&		// Length of received packet is more than 1
					sucaResponseBuffer[0] != ACK && 			// Received byte is not ACK
					sucaResponseBuffer[0] != NACK)				// Received byte is not NACK
			{
				//
				// change destination address
				//
				sucaResponseBuffer[DESTINATION_ADDRESS_POSITION] = DISPLAY_BOARD_ADDRESS;

				//
				// Recompute the checksum
				//
				lucCalculatedCRC = Crc16(0,sucaResponseBuffer,gstCMDitoCMDr.receiveCommandPacketLen-2);
				lunWord16.halfWord.val = lucCalculatedCRC;
				sucaResponseBuffer[gstCMDitoCMDr.receiveCommandPacketLen-2]=lunWord16.byte[1];
				sucaResponseBuffer[gstCMDitoCMDr.receiveCommandPacketLen-1]=lunWord16.byte[0];
			}

			//
			//	Set parameter command response received for parameters A020 (Operation count reset) and
			//	A028 (operation count input) reset Variable for sub-state 'Handle Counter' of 'Logic_Solver_Drive_Run'
			//
			if(gstCMDitoCMDr.transmitCommandPacket[COMMAND_ID_POSITION] == SET_PARAMETER_CMD_FROM_CONTROL)
			{
				lunWord16.byte[1] = gstCMDitoCMDr.transmitCommandPacket[COMMAND_DATA_POSITION];
				lunWord16.byte[0] = gstCMDitoCMDr.transmitCommandPacket[COMMAND_DATA_POSITION + 1];

				if(
					(lunWord16.halfWord.val == OPERATION_COUNT_RESET) ||
					(lunWord16.halfWord.val == OPERATION_COUNT_INPUT)
				)
				{
					geHandleCounter = HandleCounterInitGetCounter;
				}
			}
#endif

			//
			//	Copy response packet length to the local buffer
			//
			sucResponsePacketLength = gstCMDitoCMDr.receiveCommandPacketLen;

			//
			//	Send response to display board
			//
			uartSendTxBuffer(UART_display,sucaResponseBuffer,sucResponsePacketLength);

			//
			//	Added on 17 Nov 2014 to implement drive board firmware upgrade functionality.
			//
			//	Drive board firmware upgrade was requested from display application code. Drive board processed
			//	the request and sent ACK.
			//
			if(gstCMDitoCMDr.transmitCommandPacket[COMMAND_ID_POSITION] == FIRMWARE_UPGRADECMD_FROM_DISPLAY)
			{
				//	Set the flag to indicate that drive firmware upgrade was initiated by display board application
				//	and drive processed the request and responded with ACK

				gui8DriveFirwareUpgradeInitiated = 1;
			}
			//
			//	Reset response packet length
			//
			sucResponsePacketLength = 0;
			//
			//	Response_ACK not supported
			//	Release gstCMDitoCMDr
			//
			gstCMDitoCMDr.commandRequestStatus = eINACTIVE;
			gstCMDitoCMDr.commandResponseStatus = eNO_STATUS;

			//
			//	Reset function state to eIdle
			//
			sucCommunicationModuleDisplayState = eIdle;

		}	//	if(gstCMDitoCMDr.commandResponseStatus == eSUCCESS)

		if(gstCMDitoCMDr.commandResponseStatus == eFAIL || gstCMDitoCMDr.commandResponseStatus == eTIME_OUT)
		{
			//
			//	Do not respond to Display board
			//
			//	Release gstCMDitoCMDr
			//
			gstCMDitoCMDr.commandRequestStatus = eINACTIVE;
			gstCMDitoCMDr.commandResponseStatus = eNO_STATUS;

			//
			//	Set parameter command response received for parameters A020 (Operation count reset) and
			//	A028 (operation count input) reset Variable for sub-state 'Handle Counter' of 'Logic_Solver_Drive_Run'
			//
			if(gstCMDitoCMDr.transmitCommandPacket[COMMAND_ID_POSITION] == SET_PARAMETER_CMD_FROM_CONTROL)
			{
				lunWord16.byte[1] = gstCMDitoCMDr.transmitCommandPacket[COMMAND_DATA_POSITION];
				lunWord16.byte[0] = gstCMDitoCMDr.transmitCommandPacket[COMMAND_DATA_POSITION + 1];

				if(
					(lunWord16.halfWord.val == OPERATION_COUNT_RESET) ||
					(lunWord16.halfWord.val == OPERATION_COUNT_INPUT)
				)
				{
					geHandleCounter = HandleCounterInitGetCounter;
				}
			}

			//
			//	Reset function state to eIdle
			//
			sucCommunicationModuleDisplayState = eIdle;
		}	//	if(gstCMDitoCMDr.commandResponseStatus == eFAIL)

		if(sucCmdiDHBlockInitiatedByCMDi == 1)
		{
			sucCmdiDHBlockInitiatedByCMDi = 0;

			if(gstCMDitoDH.commandResponseStatus == eSUCCESS)
			{
				if(gstCMDitoDH.commandDisplayBoardDH.bits.setParameter == 1 ||
						gstCMDitoDH.commandDisplayBoardDH.bits.setTimeStamp == 1)
				{
					//if(gstCMDitoDH.acknowledgementReceived == eACK)		//	DH not required to update ACK/NACK
					{
						//
						//	Respond to display board with ACK
						//
						sucaResponseBuffer[sucResponsePacketLength++] = ACK;
					}
					/*if(gstCMDitoDH.acknowledgementReceived == eNACK)	//	DH not required to update ACK/NACK
				{
					//
					//	Respond to display board with NACK
					//
					sucaResponseBuffer[sucResponsePacketLength++] = NACK;
				}*/

					//
					//	Response_ACK not required
					//
					lucCommandSupportReponseAck = 0;
				}

				if(gstCMDitoDH.commandDisplayBoardDH.bits.getParameter == 1)
				{
					//
					//	Form response packet to display with parameter value
					//

					//	Destination address
					sucaResponseBuffer[DESTINATION_ADDRESS_POSITION] = DISPLAY_BOARD_ADDRESS;
					sucResponsePacketLength++;

					//	Source address
					sucaResponseBuffer[SOURCE_ADDRESS_POSITION] = CONTROL_BOARD_ADDRESS;
					sucResponsePacketLength++;

					//
					//	Feel response data into response packet
					//

					//	Copy 4 Byte parameter value (getParameter command response) into a local union
					lunWord32.word.val = gstCMDitoDH.getParameterValue;

					//
					//	Copy 4 Bytes of parameter value from local union to response packet
					//
					/*				sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord32.byte[3];
				sucResponsePacketLength++;

				sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord32.byte[2];
				sucResponsePacketLength++;

				sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord32.byte[1];
				sucResponsePacketLength++;

				sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord32.byte[0];
				sucResponsePacketLength++;*/

					sucaResponseBuffer[RESPONSE_DATA_POSITION] = lunWord32.byte[3];
					sucResponsePacketLength++;
					sucaResponseBuffer[RESPONSE_DATA_POSITION + 1] = lunWord32.byte[2];
					sucResponsePacketLength++;
					sucaResponseBuffer[RESPONSE_DATA_POSITION + 2] = lunWord32.byte[1];
					sucResponsePacketLength++;
					sucaResponseBuffer[RESPONSE_DATA_POSITION + 3] = lunWord32.byte[0];
					sucResponsePacketLength++;

					//	Compute data length
					sucResponsePacketLength += NUMBER_OF_CRC_BYTES + 1;		//	+1 for data byte length field

					//	Copy data length
					sucaResponseBuffer[FRAME_LENGTH_POSITION] = sucResponsePacketLength;
					/*sucResponsePacketLength++;*/

					//
					//	Calculate CRC
					//
					lucResponseCRC = Crc16(0,sucaResponseBuffer,sucResponsePacketLength - 2);	//	-2 to exclude CRC

					//	Copy 2 Byte CRC to local union
					lunWord16.halfWord.val = lucResponseCRC;

					//	Copy CRC bytes from local union to response packet
					sucaResponseBuffer[sucResponsePacketLength -2] = lunWord16.byte[1];
					sucaResponseBuffer[sucResponsePacketLength -1] = lunWord16.byte[0];

					//
					//	Response_ACK not required
					//
					lucCommandSupportReponseAck = 0;

				}

				if(gstCMDitoDH.commandDisplayBoardDH.bits.getErrorList == 1)
				{
					//
					//	Form response packet to display with error list
					//

					//	Destination address
					sucaResponseBuffer[DESTINATION_ADDRESS_POSITION] = DISPLAY_BOARD_ADDRESS;
					sucResponsePacketLength++;

					//	Source address
					sucaResponseBuffer[SOURCE_ADDRESS_POSITION] = CONTROL_BOARD_ADDRESS;
					sucResponsePacketLength++;

					//
					//	Feel response data into response frame
					//

					//	Copy time-stamp into local union
					lunWord32.word.val = gstCMDitoDH.errorFromControl.timeStamp;

					//
					//	Copy 4 Bytes of time-stamp value from local union to response packet
					//
					sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord32.byte[3];
					sucResponsePacketLength++;

					sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord32.byte[2];
					sucResponsePacketLength++;

					sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord32.byte[1];
					sucResponsePacketLength++;

					sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord32.byte[0];
					sucResponsePacketLength++;

					//	Copy 2 Bytes of Anomaly code into local union

					lunWord16.halfWord.val = gstCMDitoDH.errorFromControl.anomalyCode;
				//	gsActiveAnomalylist[gsActiveAnomalylist_count_cyw++]=lunWord16.halfWord.val;
				//	if(gsActiveAnomalylist_count_cyw >= 20)
				//		gsActiveAnomalylist_count_cyw = 0;
					//
					//	Copy 2 Bytes of Anomaly code value from local union to response packet
					//
					/*sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord16.byte[1];
				sucResponsePacketLength++;

				sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord16.byte[0];
				sucResponsePacketLength++;*/
					sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord16.byte[1];
					sucResponsePacketLength++;

					sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = lunWord16.byte[0];
					sucResponsePacketLength++;

					//	Copy 15 Bytes of error details to response packet
					for(lucTemp = 0;lucTemp<15;lucTemp++)
					{
						sucaResponseBuffer[RESPONSE_DATA_POSITION + sucResponseDataLength++] = gstCMDitoDH.errorFromControl.errorDetails[lucTemp];
						sucResponsePacketLength++;
					}

					//	Compute data length
					sucResponsePacketLength += NUMBER_OF_CRC_BYTES + 1;		// +1 for data byte length

					//	Copy data length
					sucaResponseBuffer[FRAME_LENGTH_POSITION] = sucResponsePacketLength;
					/*sucResponsePacketLength++;*/

					//
					//	Calculate CRC
					//
					lucResponseCRC = Crc16(0,sucaResponseBuffer,sucResponsePacketLength -2);	//	-2 to exclude 2 CRC bytes from calculations

					//	Copy 2 Byte CRC to local union
					lunWord16.halfWord.val = lucResponseCRC;

					//	Copy CRC bytes from local union to response packet
					sucaResponseBuffer[sucResponsePacketLength -2] = lunWord16.byte[1];
					sucaResponseBuffer[sucResponsePacketLength -1] = lunWord16.byte[0];

					//
					//	Response_ACK required
					//
					lucCommandSupportReponseAck = 1;

				}

				//
				//	Added on 06 Nov 2014 to handle firmware upgrade
				//
				if(gstCMDitoDH.commandDisplayBoardDH.bits.firmwareUpgrade == 1)
				{
					sucaResponseBuffer[sucResponsePacketLength++] = ACK;

					//
					//	Response_ACK not required
					//
					lucCommandSupportReponseAck = 0;
				}

				//
				//	Send response to display board
				//
				uartSendTxBuffer(UART_display,sucaResponseBuffer,sucResponsePacketLength);

				sucResponsePacketLength = 0;
				sucResponseDataLength = 0;
				//
				//	If response_ACK not supported release gstCMDitoDH
				//
				if(lucCommandSupportReponseAck == 0)
				{
					//
					//	Release CMDi to DH
					//
					gstCMDitoDH.commandRequestStatus = eINACTIVE;
					gstCMDitoDH.commandResponseStatus = eNO_STATUS;

					//
					//	Clear command buffer
					//
					gstCMDitoDH.commandDisplayBoardDH.val = 0;

					//
					//	Reset function state to eIdle
					//
					sucCommunicationModuleDisplayState = eIdle;

				}
				else
				{

					//
					//	Change function state to eIdle
					//
					sucCommunicationModuleDisplayState = eIdle;

				}
			}	//	if(gstCMDitoDH.commandResponseStatus == eSUCCESS)


			if(gstCMDitoDH.commandResponseStatus == eFAIL ||
					gstCMDitoDH.commandResponseStatus == eTIME_OUT)
			{
				//
				//	Respond to display board with NACK
				//
				sucaResponseBuffer[sucResponsePacketLength++] = NACK;

				//
				//	Send response to display board
				//
				uartSendTxBuffer(UART_display,sucaResponseBuffer,sucResponsePacketLength);

				//
				//	Reset transmit packet length
				//
				sucResponsePacketLength = 0;

				//
				//	Release CMDi to DH
				//
				gstCMDitoDH.commandRequestStatus = eINACTIVE;
				gstCMDitoDH.commandResponseStatus = eNO_STATUS;

				//
				//	Clear command buffer
				//
				gstCMDitoDH.commandDisplayBoardDH.val = 0;

				//
				//	Reset function state to eIdle
				//
				sucCommunicationModuleDisplayState = eIdle;
			}

		}//sucCmdiDHBlockInitiatedByCMDi = 1;

		break;

	case eCommandACK_ReceivedFromDisplayWaitingForReply:
		//
		//	ACK received from Display board for CMDitoDH command
		//
		if(gstCMDitoDH.commandResponseACK_Status == eResponseAcknowledgementProcessed)
		{
			//
			//	Release gstCMDitoDH
			//
			gstCMDitoDH.commandRequestStatus = eINACTIVE;
			gstCMDitoDH.commandResponseStatus = eNO_STATUS;
			gstCMDitoDH.commandResponseACK_Status = eNO_StatusAcknowledgement;

			//
			//	Clear command buffer
			//
			gstCMDitoDH.commandDisplayBoardDH.val = 0;

			//
			//	Reset function state to eIdle
			//
			sucCommunicationModuleDisplayState = eIdle;


			clear_uart_buff_cyw(UART_display);

		}
		break;
	}
}

/********************************************************************************/
