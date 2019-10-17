/*********************************************************************************
 * FileName: serial.c
 * Description:
 * This source file contains the definition of all the services of UART
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
 *  	0.1D	27/03/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Include:
 ****************************************************************************/
#include <p33Exxxx.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "./Drivers/UART/UART.h"
#include "./Drivers/Timer/Timer.h"
#include "./Middleware/ParameterDatabase/eeprom.h"
#include "./Common/UserDefinition/Userdef.h"
#include "./Application/Application.h"
#include "./Application/CommandHandler.h"
#include "serial.h"
#include "./Common/Extern/Extern.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/
#define COMM_UART_PRIORITY	1


// UART Configuration data
//#define FCY	 	70000000	//This define has to be the system operating freq, this 
							//value is used to calculate the value of the BRG register
#define BAUDRATE 	9600//38400 // 115200  //57600	//This is the desired baudrate for the UART modules 
//#define BRG	(FCY/(16*BAUDRATE))-1
#define BRG	((FCY/BAUDRATE)/16)-1//(FCY/(16*BAUDRATE))-1
#define DEBUG_UART


#define CMD_FIRST_BYTE_INDEX 	0
#define CMD_SECOND_BYTE_INDEX 	1
#define CMD_THIRD_BYTE_INDEX 	2

/****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/
UINT8 packetStartIndex; // This indicates the start-of-command index in RX buffer 


/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/
enum {
	no_error = 0, 
	UART_channel_disabled,
	invalid_UART_channel, 
	UART_busy, 
	receive_buffer_is_empty, 
	number_of_channel_errors
}eUARTChannelCheck; // error list - used when checking if UART channel number is valid 



enum{
	no_command = number_of_channel_errors, 
	command_recv_in_progress, 
	new_command_recd
}eUARTCommandRxStatus;   // used by - stTxRxBuffer[].uchCommandRXStatus 





_stTxRxBuffer	stTxRxBuffer[UART_AVAILABLE_CHANNEL_COUNT];  


/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
 ****************************************************************************/


/****************************************************************************/

// initialization of UART buffers 
void initUARTBuffers(void)
{
	UINT8 index = 0; 
	for(index = 0; index<UART_AVAILABLE_CHANNEL_COUNT; index++)
	{
		memset(stTxRxBuffer[index].uchTxBuffer,'\0',TRANSMIT_BUFFER_SIZE);
		memset(stTxRxBuffer[index].uchRxBuffer,'\0',RECEIVE_BUFFER_SIZE);
		
		stTxRxBuffer[index].uchTxBufferByteCount = 0;
		stTxRxBuffer[index].uchTxBufferIndex = 0;
		stTxRxBuffer[index].uchRxBufferByteCount = 0;
		stTxRxBuffer[index].uchRxBufferRdIndex = 0;
		stTxRxBuffer[index].uchRxBufferWrIndex = 0;
		stTxRxBuffer[index].uchRxBufferOverflowState = 0;
		stTxRxBuffer[index].uchCommandRXStatus = no_command; 
		stTxRxBuffer[index].uchAssignedUARTChannel = FREE; 
	}

	packetStartIndex = 0;
}



/***************************************************************************************
 * configureUART
 *
 * Function Description:
 * Configure the UART and its pins.
 *
 * Function Parameters:
 * UINT8 lucUartNumber
 * lucUartNumber 	= 0 for UART1
 * 					= 1 for UART2
 * 					= 2 for UART3
 * 					= 3 for UART4
 *
 * Function Returns:
 * 		0 = Success
 *		1 = lucUartNumber not in valid range
 *		2 = UART is disabled
 *
 ***************************************************************************************/
UINT8
configureUART(UINT8 lucUartNumber)
{
	UINT8 lucReturnValue = no_error;
	UINT8 index = 0; 

	UINT16 UART_MODE_VALUE; // Holds the value of uart config reg
	UINT16 UART_STA_VALUE; // Holds the information regarding uart	TX & RX interrupt modes

	/* Configure UART modules to transmit 8 bit data with one stopbit.	*/ 
	UART_MODE_VALUE = UART_EN & UART_IDLE_CON & UART_IrDA_DISABLE &
				UART_MODE_FLOW & UART_UEN_00 & UART_DIS_WAKE & 
				UART_DIS_LOOPBACK & UART_DIS_ABAUD & UART_UXRX_IDLE_ONE & 
				UART_BRGH_SIXTEEN & UART_NO_PAR_8BIT & UART_1STOPBIT;

	//UART_STA_VALUE	= UART_INT_TX_BUF_EMPTY  & UART_IrDA_POL_INV_ZERO & 				 
	//			UART_SYNC_BREAK_DISABLED & UART_TX_ENABLE & UART_INT_RX_CHAR & 
	//			UART_ADR_DETECT_DIS & UART_RX_OVERRUN_CLEAR;
            
            
    UART_STA_VALUE	= UART_INT_TX_LAST_CH  & UART_IrDA_POL_INV_ZERO & 				 
                UART_SYNC_BREAK_DISABLED & UART_TX_ENABLE & UART_INT_RX_CHAR & 
                UART_ADR_DETECT_DIS & UART_RX_OVERRUN_CLEAR;

            
	switch(lucUartNumber)
	{
	case UART_CHANNEL_1:
		if(UART1_Available)		//UART1 is enabled
		{
			CloseUART1(); // Turn off UART1 module
			ConfigIntUART1(UART_RX_INT_EN 
						& (UART_RX_INT_PR1+COMM_UART_PRIORITY)
						& UART_TX_INT_EN & UART_TX_INT_PR1); // Configure UART1 receive and transmit interrupt 
			
			OpenUART1(UART_MODE_VALUE, UART_STA_VALUE, BRG); // Configure UART1 module to transmit 8 bit data, no parity and one stopbit at 38400 baud
		}
		else		//UART1 is disabled
		{
			lucReturnValue = UART_channel_disabled;
		}
		break;
	case UART_CHANNEL_2:
		if(UART2_Available)		//UART2 available
		{
			CloseUART2(); // Turn off UART2 module
			ConfigIntUART2(UART_RX_INT_EN 
						& (UART_RX_INT_PR1+COMM_UART_PRIORITY)
						& UART_TX_INT_EN & UART_TX_INT_PR1); // Configure UART2 receive and transmit interrupt 
			
			OpenUART2(UART_MODE_VALUE, UART_STA_VALUE, BRG); // Configure UART2 module to transmit 8 bit data, no parity and one stopbit at 38400 baud
		}
		else	//UART2 is disabled
		{
			lucReturnValue = UART_channel_disabled;
		}
		break;
	case UART_CHANNEL_3:
		if(UART3_Available)		//UART3 available
		{
		}
		else	//UART3 is disabled
		{
			lucReturnValue = UART_channel_disabled;
		}
		break;
	case UART_CHANNEL_4:
		if(UART4_Available)		//UART4 available
		{
		}
		else	//UART4 is disabled
		{
			lucReturnValue = UART_channel_disabled;
		}
		break;
	default:	//Specified UART number is not in valid range
		lucReturnValue = invalid_UART_channel;
		break;
	}

	
	if(no_error == lucReturnValue)
	{		
		for(index = 0; index<UART_AVAILABLE_CHANNEL_COUNT; index++)
		{
			if(FREE == stTxRxBuffer[index].uchAssignedUARTChannel) 
			{
				stTxRxBuffer[index].uchAssignedUARTChannel = lucUartNumber; 
				break;
			}
			 
		}
	}
	return lucReturnValue;
}



// gets the index of the buffer array which is in use by the specified UART 
UINT8 getBufferIndex(UINT8 lucUartNumber)
{

	UINT8 status = NOT_FOUND; 
	UINT8 index = 0; 	

	for(index = 0; index<UART_AVAILABLE_CHANNEL_COUNT; index++)
	{
		if(lucUartNumber == stTxRxBuffer[index].uchAssignedUARTChannel) 
		{
			return index; 
		}
	}

	return status; 	
}



// Function to check if specified UART number is valid 
UINT8 checkUARTChannel(UINT8 lucUartNumber)
{
	UINT8 lucReturnValue = no_error;

	switch(lucUartNumber)	//	Check if specified UART is disabled
	{
	case UART_CHANNEL_1:
		if(!UART1_Available)
		{
			lucReturnValue = UART_channel_disabled;
		}
		break;
	case UART_CHANNEL_2:
		if(!UART2_Available)
		{
			lucReturnValue = UART_channel_disabled;
		}
		break;
	case UART_CHANNEL_3:
		if(!UART3_Available)
		{
			lucReturnValue = UART_channel_disabled;
		}
		break;
	case UART_CHANNEL_4:
		if(!UART4_Available)
		{
			lucReturnValue = UART_channel_disabled;
		}
		break;
	default:
		lucReturnValue = invalid_UART_channel;
		break;
	}	//Switch ends here

	return lucReturnValue; 
}


/***************************************************************************************
 * uartCheckFreeTxBuffer
 *
 * Function Description:
 * Check free UART transmit buffer
 * If UART transmit buffer is free, then only user should call 'uartSendTxBuffer'
 *
 * Function Parameters:
 * UINT8 lucUartNumber
 * lucUartNumber 	= 0 for UART1
 * 					= 1 for UART2
 * 					= 2 for UART3
 * 					= 3 for UART4
 *
 * UINT8 lucPendingNoCharToTx
 * Number of characters remaining to transmit.
 *
 * Function Returns:
 * Pending number of characters to be transmitted.
 *    0 = Success
 *    1 = lucUartNumber not in valid range
 *    2 = Specified UART is disabled
 *
 ***************************************************************************************/
UINT8
uartCheckFreeTxBuffer (UINT8 lucUartNumber, UINT8* lucPendingNoCharToTx)
{
	UINT8 lucReturnValue = no_error;

	UINT8 index = getBufferIndex(lucUartNumber); 

	lucReturnValue = checkUARTChannel(lucUartNumber); 
	if (lucReturnValue == no_error)	//	Specified UART is enabled
	{
		//	Read pending number of bytes to transmit from global buffer
		*lucPendingNoCharToTx = stTxRxBuffer[index].uchTxBufferByteCount;
	}

	return lucReturnValue;
}


/***************************************************************************************
 * uartSendTxBuffer
 *
 * Function Description:
 * Function will be used to send transmit buffer on respective serial channel.
 * Function will carry out following activities
 * i.   Copy the data pointed by *pui8Buffer into a global buffer
 * ii.  Copy number of bytes to be transmitted specified by uiCount into global variable
 * iii. Send first byte, initialize global transmit buffer index to zero and start transmit interrupt.
 *
 * Byte transmit interrupt routine will carry out following activities
 * i.   Decrement the global variable which hold number of bytes to be transmitted
 * ii.  If above global variable is not zero, then increment the global transmit buffer index
 *      a.  Transmit the single byte pointed by global transmit buffer index
 * iii. If above global variable is equal to zero, then stop transmit interrupt
 *
 * Function Parameters:
 * UINT8 lucUartNumber
 * lucUartNumber 	= 0 for UART1
 * 					= 1 for UART2
 * 					= 2 for UART3
 * 					= 3 for UART4
 *
 * const UINT8 *lpu8Buffer
 * *pui8Buffer	= pointer to the buffer to be transmitted
 *
 * UINT8 lucCount
 * uiCount	= number of bytes to be transmitted
 *
 * Function Returns:
 *		0 = Transmission success
 *		1 = lucUartNumber not in valid range
 *		2 = Specified UART is disabled
 *		3 = Channel Busy
 *
 ***************************************************************************************/
UINT8
uartSendTxBuffer (UINT8 lucUartNumber, const UINT8 *lpu8Buffer, UINT8 lucCount)
{
    //WORD delayCnt = 0xFFFF;
	UINT8 lucReturnValue = no_error;
	UINT8 lucPendingNumOfCharToTx = 0;
	UINT8 index = getBufferIndex(lucUartNumber); 

	//	Check whether specified UART is busy in transmission
	lucReturnValue = uartCheckFreeTxBuffer(lucUartNumber, &lucPendingNumOfCharToTx);

	//	uartCheckFreeTxBuffer unsuccessful or UART busy in transmission
	if(lucReturnValue || lucPendingNumOfCharToTx)
	{
		if(lucPendingNumOfCharToTx)	// UART busy
			lucReturnValue = UART_busy;
	}
	else	//else transmit
	{
		lucReturnValue = checkUARTChannel(lucUartNumber); 
		if (lucReturnValue == no_error)	//	Specified UART is enabled
		{
			//	Clear earlier data
			memset(stTxRxBuffer[index].uchTxBuffer,'\0',TRANSMIT_BUFFER_SIZE);

			//	Copy data buffer to be transmitted pointed by lpu8Buffer into global buffer
			//usprintf((char *)&uchTxBuffer[uchUARTMapToGlobalBuffer[lucUartNumber]][0],"%s",lpu8Buffer);
			memcpy((UINT8 *)&stTxRxBuffer[index].uchTxBuffer[0], lpu8Buffer, lucCount); 

			//	Copy number of bytes to be transmitted into global buffer
			stTxRxBuffer[index].uchTxBufferByteCount = lucCount;

			//	Clear transmit buffer index
			stTxRxBuffer[index].uchTxBufferIndex = 0;

            PORTCbits.RC4 = 1;            
            txInProgress = TRUE; 

			//	Transmit the first byte in global buffer to initiate transmit interrupt.
			//	Transmission of remaining bytes would be handled by corresponding UART interrupt
            txResetCount = 0;
            writeCharToUART(lucUartNumber, *&stTxRxBuffer[index].uchTxBuffer[0]); 
		}
	}	//	else transmit ends here

	return lucReturnValue;
	
}


/***************************************************************************************
 * uartCheckRxBufferOverflowState
 *
 * Function Description:
 * Check not free state of UART receive buffer
 * If UART transmit buffer is not free, then only user should call 'uartGetRxBuffer'
 *
 * Function Parameters:
 * UINT8 lucUartNumber
 * lucUartNumber 	= 0 for UART1
 * 					= 1 for UART2
 * 					= 2 for UART3
 * 					= 3 for UART4
 *
 * Function Returns:
 * Number of characters received
 *    0 = Success
 *    1 = lucUartNumber not in valid range
 *    2 = Specified UART is disabled
 ***************************************************************************************/
UINT8
uartCheckRxBufferOverflowState (UINT8 lucUartNumber, UINT8* lucRxBufferOverflowState)
{
	UINT8 lucReturnValue = no_error;

	UINT8 index = getBufferIndex(lucUartNumber); 

	lucReturnValue = checkUARTChannel(lucUartNumber); 
	
	if(lucReturnValue == no_error)	//	Specified UART is enabled
	{	
		//	Read Rx buffer overflow state from global buffer
		*lucRxBufferOverflowState = stTxRxBuffer[index].uchRxBufferOverflowState;
	}

	return lucReturnValue;
}


/***************************************************************************************
 * uartCheckNotFreeRxBuffer
 *
 * Function Description:
 * Check not free state of UART receive buffer
 * If UART transmit buffer is not free, then only user should call 'uartGetRxBuffer'
 *
 * Function Parameters:
 * UINT8 lucUartNumber
 * lucUartNumber 	= 0 for UART1
 * 					= 1 for UART2
 * 					= 2 for UART3
 * 					= 3 for UART4
 *
 * Function Returns:
 * Number of characters received
 *    0 = Success
 *    1 = lucUartNumber not in valid range
 *    2 = Specified UART is disabled
 ***************************************************************************************/
UINT8
uartCheckNotFreeRxBuffer (UINT8 lucUartNumber, UINT8* lucPendingNoCharInRx)
{
	UINT8 lucReturnValue = no_error;

	UINT8 index = getBufferIndex(lucUartNumber); 
	lucReturnValue = checkUARTChannel(lucUartNumber); 
	
	
	if(lucReturnValue == no_error)
		*lucPendingNoCharInRx = stTxRxBuffer[index].uchRxBufferByteCount;

	return lucReturnValue;
}


/***************************************************************************************
 * uartCheckNewCommandReceived
 *
 * Function Description:
 * Will return with status TRUE if a new command is received from CB
 * The command handler should get the command from receive buffer using 'uartGetRxBuffer'
 * only when this function returns TRUE, with this check, the command handler need not 
 * check for duplicate commands from CB, that would be responsibility of CB? 
 * 
 * Function Parameters:
 * UINT8 lucUartNumber
 * lucUartNumber 	= 0 for UART1
 * 					= 1 for UART2
 * 					= 2 for UART3
 * 					= 3 for UART4
 *
 * Function Returns:
 * Status 
 *    1 = Invalid UART channel 
 * 	2 = UART channel error 
 *    3 = No new command from CB 
 *    4 = Command receive in progress
 *    5 = New command is received in buffer and ready to be read 
 ***************************************************************************************/
UINT8
uartCheckNewCommandReceived (UINT8 lucUartNumber)
{
	UINT8 lucReturnValue = no_error;
//	UINT32 currentTimestamp = 0; 
	UINT8 index = 0;
	static BOOL startPacketDetected = FALSE; 

	UINT8 bufferIndex = getBufferIndex(lucUartNumber); 
	lucReturnValue = checkUARTChannel(lucUartNumber);
	if(lucReturnValue == no_error)
	{	
		if(stTxRxBuffer[bufferIndex].uchCommandRXStatus == command_recv_in_progress)
		{
			//currentTimestamp = getSystemTick(); 
			//if((currentTimestamp - stTxRxBuffer[bufferIndex].uchRxFirstByteTimeStamp) >= 1000) // 1000 milliseconds have lapsed since the last RX ISR 

			//if((ASCII_ACK == stTxRxBuffer[bufferIndex].uchRxBuffer[CMD_FIRST_BYTE_INDEX]) 
			//	|| (ASCII_NACK == stTxRxBuffer[bufferIndex].uchRxBuffer[CMD_FIRST_BYTE_INDEX]))
			//{
			//	stTxRxBuffer[bufferIndex].uchCommandRXStatus = new_command_recd; 
			//}
			//else 
				// 1. detect start-of-command-frame, 
				// 2. if detected, read number of bytes expected, 
				// 3. keep reading bytes till all expected bytes are received, 
				// 4. when done flag "new-command-recd"
			{
				if((!startPacketDetected) )// && (CMD_THIRD_BYTE_INDEX <= stTxRxBuffer[bufferIndex].uchRxBufferByteCount))
				{
					for(index = 0; index < RECEIVE_BUFFER_SIZE; index++)
					{
						if((stTxRxBuffer[bufferIndex].uchRxBuffer[getSafeRxBufferIndex(index)] == DRIVE_BOARD_ADDRESS)
								&& (CONTROL_BOARD_ADDRESS == stTxRxBuffer[bufferIndex].uchRxBuffer[getSafeRxBufferIndex(index+CMD_SECOND_BYTE_INDEX)]))
						{
							startPacketDetected = TRUE; 
							packetStartIndex = index; 
							break; 
						}
					}
				}

				if(startPacketDetected )//&& stTxRxBuffer[bufferIndex].uchRxBufferByteCount) 
				{
					if((DRIVE_BOARD_ADDRESS == stTxRxBuffer[bufferIndex].uchRxBuffer[getSafeRxBufferIndex(packetStartIndex)])
						&& (CONTROL_BOARD_ADDRESS == stTxRxBuffer[bufferIndex].uchRxBuffer[getSafeRxBufferIndex(packetStartIndex+CMD_SECOND_BYTE_INDEX)]))
					{	
						// if(packetStartIndex > (RECEIVE_BUFFER_SIZE - MAX_CMND_LENGTH)) // buffer overflow is expected -- TBD 

						if(getRxBufferBytesRecdFrmStartFrame(bufferIndex, packetStartIndex) > CMD_THIRD_BYTE_INDEX)
						//if(stTxRxBuffer[bufferIndex].uchRxBufferWrIndex > (packetStartIndex+CMD_THIRD_BYTE_INDEX)) 
							// third byte containing command data-length has also been received 
						{
							// continue to read bytes till expected data length 
							//if((stTxRxBuffer[bufferIndex].uchRxBufferByteCount - packetStartIndex) 
							//			== stTxRxBuffer[bufferIndex].uchRxBuffer[getSafeRxBufferIndex(packetStartIndex+CMD_THIRD_BYTE_INDEX)])
							if(getRxBufferBytesRecdFrmStartFrame(bufferIndex, packetStartIndex)
									>= stTxRxBuffer[bufferIndex].uchRxBuffer[getSafeRxBufferIndex(packetStartIndex+CMD_THIRD_BYTE_INDEX)])
							{
								stTxRxBuffer[bufferIndex].uchCommandRXStatus = new_command_recd; 
								startPacketDetected = FALSE;
							}
							else if(MAX_CMND_LENGTH < (stTxRxBuffer[bufferIndex].uchRxBuffer[getSafeRxBufferIndex(packetStartIndex+CMD_THIRD_BYTE_INDEX)]))
							//else if(getRxBufferBytesRecdFrmStartFrame(bufferIndex, packetStartIndex) > MAX_CMND_LENGTH)
							{
								// reset the start pattern which has come with invalid data bytes, 
								// this will allow detection of next valid command in buffer 
								stTxRxBuffer[bufferIndex].uchRxBuffer[getSafeRxBufferIndex(packetStartIndex)] = 0; 
								stTxRxBuffer[bufferIndex].uchRxBuffer[getSafeRxBufferIndex(packetStartIndex+CMD_SECOND_BYTE_INDEX)] = 0; 
								
								packetStartIndex = 0;
								startPacketDetected = FALSE;
							}
						}
					
					}
					else 
					{
						packetStartIndex = 0;
						startPacketDetected = FALSE;
					}
				}
			}

			//if(MAX_CMND_LENGTH <= (stTxRxBuffer[bufferIndex].uchRxBufferByteCount - packetStartIndex))
			//		|| (stTxRxBuffer[bufferIndex].uchRxBufferOverflowState))
			//{
			//	packetStartIndex = 0;
			//	startPacketDetected = FALSE;
			//}
		}
		
		lucReturnValue = stTxRxBuffer[index].uchCommandRXStatus; 	
	}

	return lucReturnValue;
}


UINT8 getSafeRxBufferIndex(UINT8 currentIndex)
{
	UINT8 result = currentIndex; 
	if(RECEIVE_BUFFER_SIZE <= currentIndex)	// need to consider overflow and bytes being in multiples of Rx buffer size 
	{
		result = (currentIndex % RECEIVE_BUFFER_SIZE); // wraps around to the start of the array 
	}
	
	return result; 
}

UINT8 getRxBufferBytesRecdFrmStartFrame(UINT8 uartBufferIndex, UINT8 packetStartIndex)
{
	UINT8 result = 0; 
		
	if(stTxRxBuffer[uartBufferIndex].uchRxBufferWrIndex > packetStartIndex)
	{
		result = stTxRxBuffer[uartBufferIndex].uchRxBufferWrIndex - packetStartIndex; 
	}
	else if(stTxRxBuffer[uartBufferIndex].uchRxBufferOverflowState) // if overflow 
	{
		result = stTxRxBuffer[uartBufferIndex].uchRxBufferWrIndex 
						+ (RECEIVE_BUFFER_SIZE - packetStartIndex); 
	}
	// else start packet detected is wrong, return 0 
	
	// overflow needs to be considered here - multiples of entire buffer will need to
	// be considered, else Rd buffer pointer goes haywire 
	//if(stTxRxBuffer[uartBufferIndex].uchRxBufferOverflowState) // if overflow 
	//{
	//	result = result + (stTxRxBuffer[uartBufferIndex].uchRxBufferOverflowCount * RECEIVE_BUFFER_SIZE);
	//}
	
	return result; 
}


/***************************************************************************************
 * uartGetRxBuffer
 *
 * Function Description:
 * Byte received interrupt will carry out following activities
 * i.  Store received byte on "circular global buffer" at index pointed by "write circular buffer index"
 * ii. increment the "Rx buffer count"
 * iii. Increment the "write circular buffer index". if it reaches to "maximum buffer size", then same should reset to zero

 * Function will be used to fetch one byte at a time from "circular global buffer"
 * Function will carry out following activities
 * i.   Read single byte from "circular global buffer" at index pointed by "read circular buffer index"
 * ii.  Increment the "read circular buffer index". if it reaches to "maximum buffer size", then same should reset to zero
 * iii. decrement the "Rx buffer count"
 * iv.  Return the single byte from step one in lucReadData
 *
 * Function Parameters:
 * UINT8 lucUartNumber
 * lucUartNumber 	= 0 for UART1
 * 					= 1 for UART2
 * 					= 2 for UART3
 * 					= 3 for UART4
 *
 * Function Returns:
 * 		0 = Success
 * 		1 = lucUartNumber not in valid range
 * 		2 = Specified UART is disabled
 * 		3 = Global receive buffer is empty
 *
 ***************************************************************************************/
UINT8
uartGetRxBuffer (UINT8 lucUartNumber, UINT8* lucReadData)
{
	UINT8 lucReturnValue = no_error;
	UINT8 index = getBufferIndex(lucUartNumber); 
	
	lucReturnValue = checkUARTChannel(lucUartNumber);

	if(lucReturnValue == no_error)
	{
		if(stTxRxBuffer[index].uchRxBufferByteCount == 0)	//	Global receive buffer is empty
		{
			lucReturnValue = 3;
		}
		else	//	Data available in global receive buffer
		{
			//	Copy data from global buffer corresponding to specified UART
			*lucReadData = stTxRxBuffer[index].uchRxBuffer[stTxRxBuffer[index].uchRxBufferRdIndex];

			//	Increment read index
			stTxRxBuffer[index].uchRxBufferRdIndex++;

			//	Reset read index if it exceeds maximum receive buffer size
			if(stTxRxBuffer[index].uchRxBufferRdIndex >= RECEIVE_BUFFER_SIZE)
				stTxRxBuffer[index].uchRxBufferRdIndex = 0;

			//	Decrement read byte count
			if(stTxRxBuffer[index].uchRxBufferByteCount)
				stTxRxBuffer[index].uchRxBufferByteCount--;
		}
	}

	return lucReturnValue;
}



// Function to retrieve a received command from the RX buffer of the specified channel 
UINT8
uartGetCommand (UINT8 lucUartNumber, UINT8* lucReadData, UINT8* lucDataLength)
{
	UINT8 lucReturnValue = no_error;
	UINT8 bufferIndex = getBufferIndex(lucUartNumber); 
	lucReturnValue = checkUARTChannel(lucUartNumber);
	UINT8 bytes, index = 0; 
	
	if(lucReturnValue == no_error)
	{
		if(stTxRxBuffer[bufferIndex].uchRxBufferByteCount == 0)	//	Global receive buffer is empty
		{
			lucReturnValue = receive_buffer_is_empty;
		}
		else if(new_command_recd == stTxRxBuffer[bufferIndex].uchCommandRXStatus)  //	Command completely received 
		{			
			//	Copy data from global buffer corresponding to specified UART
			//memcpy(lucReadData, (char *)&stTxRxBuffer[bufferIndex].uchRxBuffer[packetStartIndex], stTxRxBuffer[bufferIndex].uchRxBuffer[packetStartIndex+2]);
							
			// Copy data length 
			*lucDataLength = stTxRxBuffer[bufferIndex].uchRxBuffer[getSafeRxBufferIndex(packetStartIndex+CMD_THIRD_BYTE_INDEX)]; 

			bytes = *lucDataLength; 
			index = 0; 
			if(MAX_CMND_LENGTH >= bytes)
			{
				while(bytes)
				{
					*(lucReadData+index) = stTxRxBuffer[bufferIndex].uchRxBuffer[getSafeRxBufferIndex(packetStartIndex+index)];
					bytes--; 
					index++; 
				}
			}
			// else part not reqd for this if, let the data get erased anyways as it is invalid 
			
			// Reset data in the RX buffer 
			memset((char *)&stTxRxBuffer[bufferIndex].uchRxBuffer[0],'\0',RECEIVE_BUFFER_SIZE);

			//	Reset read index 
			stTxRxBuffer[bufferIndex].uchRxBufferRdIndex = 0;

			// Reset write index 
			stTxRxBuffer[bufferIndex].uchRxBufferWrIndex = 0;

			//	Reset recd byte count
			stTxRxBuffer[bufferIndex].uchRxBufferByteCount = 0;

			// Reset command Recd status 
			stTxRxBuffer[bufferIndex].uchCommandRXStatus = no_command; 

			packetStartIndex = 0;  
		}
	}

	return lucReturnValue;
}

void genericTXInterruptHandler(UINT8 channelNumber)
{
    //DWORD delayCnt = 0xFFFF;
    //DWORD delayCnt = 0x2492; //2ms delay
    
    //PORTAbits.RA7 = !PORTAbits.RA7; //Debug LED1
    
    UINT8 index = getBufferIndex(channelNumber); 
    
    LED_RED=0;
    if(stTxRxBuffer[index].uchTxBufferByteCount)
    {
        stTxRxBuffer[index].uchTxBufferByteCount--;  //Decremented for byte sent previously
        stTxRxBuffer[index].uchTxBufferIndex++;                          //Incremented for byte sent previously
        if(stTxRxBuffer[index].uchTxBufferIndex >= TRANSMIT_BUFFER_SIZE)
            stTxRxBuffer[index].uchTxBufferIndex = 0;
        if(stTxRxBuffer[index].uchTxBufferByteCount)
        {
            txResetCount = 0;
            writeCharToUART(channelNumber, stTxRxBuffer[index].uchTxBuffer[stTxRxBuffer[index].uchTxBufferIndex]);
        }
    }
    
    if(stTxRxBuffer[index].uchTxBufferByteCount == 0)
    {
        PORTCbits.RC4 = 0; //Disable transmitt for UART1 
    }
    
    //if(stTxRxBuffer[index].uchTxBufferByteCount == 0)
    //{
    //    while(delayCnt--);
    //    PORTCbits.RC4 = 0; //Disable transmitt for UART1 
    //    //TRISDbits.TRISD6 = 0; // TURN off DEBUG led to indicate Rx mode        
    //}
    
    switch(channelNumber)
    {
        case UART_CHANNEL_1: 
            _U1TXIF = 0; 
            break; 
        case UART_CHANNEL_2: 
            _U2TXIF = 0; 
            break; 
        case UART_CHANNEL_3: 
            break; 
        case UART_CHANNEL_4: 
            break; 
        default: 
            break; 
            
    }
    
    LED_RED=1;
}



void writeCharToUART(UINT8 channelNumber, UINT8 character)
{
	switch(channelNumber)
	{
		case UART_CHANNEL_1: 
				WriteUART1(character);
			break; 
		case UART_CHANNEL_2: 
				WriteUART2(character);
			break; 
		case UART_CHANNEL_3: 
			break; 
		case UART_CHANNEL_4: 
			break; 
		default: 
			break; 
			
	}
}


UINT8 readCharFromUART(UINT8 channelNumber)
{
	switch(channelNumber)
	{
		case UART_CHANNEL_1: 
				return ReadUART1();
			break; 
		case UART_CHANNEL_2: 
				return ReadUART2();
			break; 
		case UART_CHANNEL_3: 
			break; 
		case UART_CHANNEL_4: 
			break; 
		default: 
			break; 
			
	}
	return FALSE; 
}

//*****************************************************************************
// The UART1 TX interrupt handler.
//*****************************************************************************
void __attribute__((__interrupt__, no_auto_psv)) _U1TXInterrupt(void) 
{
	genericTXInterruptHandler(UART_CHANNEL_1); 

}



void genericRXInterruptHandler(UINT8 channelNumber)
{
	UINT8 index = getBufferIndex(channelNumber); 
    
    
    
    
	//static UINT8 startupCount = 0; 
    
    //PORTCbits.RC0 = !PORTCbits.RC0; //Debug LED2

	//#define STARTUP_NUM_OF_CHARS_TO_REJECT 1

	// Loop while there are characters in the receive FIFO.
	while(checkDataReadyBit(channelNumber))
	{
		//if(bStopRecd && (STARTUP_NUM_OF_CHARS_TO_REJECT > startupCount))
		{
		//	startupCount++; 
		//	readCharFromUART(channelNumber); // read, but dont copy first 1 junk byte into buffer 
		}
		//else // start receiving 2nd character onwards - 1st character after stop is junk 
		{	
			// Read the next character from the UART
			stTxRxBuffer[index].uchRxBuffer[stTxRxBuffer[index].uchRxBufferWrIndex] 
						= (UINT8)readCharFromUART(channelNumber); // typecasting ok as we have selected 8 bit receive 

			if(!stTxRxBuffer[index].uchRxBufferByteCount)
			{
				// save the timestamp when we received the first bit - for cleanup operation 
				stTxRxBuffer[index].uchRxFirstByteTimeStamp = getSystemTick(); 
				stTxRxBuffer[index].uchCommandRXStatus = command_recv_in_progress; 
			}
					
			stTxRxBuffer[index].uchRxBufferByteCount++;
			if(stTxRxBuffer[index].uchRxBufferByteCount >= RECEIVE_BUFFER_SIZE)
			{
				stTxRxBuffer[index].uchRxBufferOverflowState = 1;
				stTxRxBuffer[index].uchRxBufferByteCount = 0;
			}
			stTxRxBuffer[index].uchRxBufferWrIndex++;
			if(stTxRxBuffer[index].uchRxBufferWrIndex >= RECEIVE_BUFFER_SIZE)
				stTxRxBuffer[index].uchRxBufferWrIndex = 0;
		}
		
	}

#define OERR_RESET 0xFFFD
#define FERR_RESET 0xFFFB

	// reset buffer overflow and framing error flags if set 
	// parity error does not happen here as parity checking is disabled 
	switch(channelNumber)
	{
		case UART_CHANNEL_1: 
				if(U1STA & _U1STA_OERR_MASK) //Buffer overflow error has occurred 
				{
					U1STA = (U1STA & OERR_RESET);  // reset the OERR flag 
					uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.commBufferOverflowCountA807++; 
				}
				if(U1STA & _U1STA_FERR_MASK) //Framing error has occurred 
				{
					U1STA = (U1STA & FERR_RESET);  // reset the FERR flag 
				}
			break; 
		case UART_CHANNEL_2: 
				if(U2STA & _U2STA_OERR_MASK) //Buffer overflow error has occurred 
				{
					U2STA = (U2STA & OERR_RESET);  // reset the OERR flag 
					uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.commBufferOverflowCountA807++; 
				}
				if(U2STA & _U2STA_FERR_MASK) //Framing error has occurred 
				{
					U2STA = (U2STA & FERR_RESET);  // reset the FERR flag 
				}
			break; 
		case UART_CHANNEL_3: 
			break; 
		case UART_CHANNEL_4: 
			break; 
		default: 
			break; 
	}

	// TBD - similar checks for framing error/ any other blocking flags needs to be in place 

    // reset Rx interrupt flag 
	switch(channelNumber)
	{
		case UART_CHANNEL_1: 
            _U1RXIF = 0; 
			break; 
		case UART_CHANNEL_2: 
            _U2RXIF = 0; 
			break; 
		case UART_CHANNEL_3: 
			break; 
		case UART_CHANNEL_4: 
			break; 
		default: 
			break; 
	}
}


BOOL checkDataReadyBit(UINT8 channelNumber)
{
	switch(channelNumber)
	{
		case UART_CHANNEL_1: 
				return DataRdyUART1(); 
			break; 
		case UART_CHANNEL_2: 
				return DataRdyUART2(); 
			break; 
		case UART_CHANNEL_3: 
			break; 
		case UART_CHANNEL_4: 
			break; 
		default: 
			break; 		
	}

	return FALSE; 
}
	

//*****************************************************************************
//
// The UART1 RX interrupt handler.
//
//*****************************************************************************
void __attribute__((__interrupt__, no_auto_psv)) _U1RXInterrupt(void) 
{
	genericRXInterruptHandler(UART_CHANNEL_1);	
}

//*****************************************************************************
//
// The UART1 Error interrupt handler.
//
//*****************************************************************************
void __attribute__((__interrupt__, no_auto_psv)) _U1ErrInterrupt(void) 
{
	_U1EIF = 0;

	// Update drive fault flag - uart error 
	uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveCommunicationFault.bits.uartError = TRUE;

	errorUARTTimeCount = 0; // reset uart error status hold counter 
	
}

