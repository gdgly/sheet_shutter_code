/***************************************************************************************
* FileName: serial.h
* Description:
* This source file contains the prototype definition of all the services of UART
* Version: 0.1D
*
*
****************************************************************************************/

/***************************************************************************************
 * Copyright 2014 Bunka Shutters.
 * This program is the property of the Bunka Shutters
 * and it shall not be reproduced, distributed or used
 * without permission of an authorized company official.
 * This is an unpublished work subject to Trade Secret
 * and Copyright protection.
****************************************************************************************/


/***************************************************************************************
 *  Modification History
 *
 *  Revision  Date        Name          	   Comments
 *  0.1D	  27/03/2014  iGATE Offshore team  Initial Creation
****************************************************************************************/
//#ifndef SERIAL_H
//#define SERIAL_H

//#include <p33Exxxx.h>
//#include "./Common/UserDefinition/Userdef.h"

/***************************************************************************************
 *  Macro definitions:
****************************************************************************************/
#define	SUCCESS		0
#define	FREE		0xFF
#define NOT_FOUND	0xFF

#define	ENABLE		1
#define	DISABLE		0

#define	UART_CHANNEL_1		0
#define	UART_CHANNEL_2		1
#define	UART_CHANNEL_3		2
#define	UART_CHANNEL_4		3

#define	UART1_RS485				ENABLE	
#define	UART2_RS485				ENABLE	// Control board 
#define	UART3_RS485				DISABLE
#define	UART4_RS485				DISABLE	

#define MAX_UART_AVAILABLE			4		// Maximum number of UARTs available in controller
#define UART_AVAILABLE_CHANNEL_COUNT	2		// Channel 1 only for now 

//#define UART2_ALLOCATED_BUFFER_INDEX	0 // add indexes of any other UART channel to be used by the software 


#define APPL_ASSIGNED_UART_CHANNEL 	UART_CHANNEL_1
#define RTDM_ASSIGNED_UART_CHANNEL 	UART_CHANNEL_2


#define	UART1_Available				ENABLE	
#define	UART2_Available				ENABLE	// to Control board -- only this channel is avlbl on Eval board 
#define	UART3_Available				DISABLE
#define	UART4_Available				DISABLE	

#define TRANSMIT_BUFFER_SIZE		30
#define RECEIVE_BUFFER_SIZE			30

#define ASCII_ACK 	0x06
#define ASCII_NACK 	0x15
#define LENGTH_OF_ACK_PACKET 1

// TBD -- foll constants need to be verified with CB 
#define DRIVE_BOARD_ADDRESS 0x00
#define CONTROL_BOARD_ADDRESS 0x01
#define PC_ADDRESS 0xFF


typedef struct 
{
	UINT8 uchTxBuffer[TRANSMIT_BUFFER_SIZE];// Transmit Buffer Array
	UINT8 uchTxBufferByteCount; 			// Length of the buffer to be transmitted
	UINT8 uchTxBufferIndex; 				// Index to the current byte in transmission
    
	UINT8 uchRxBuffer[RECEIVE_BUFFER_SIZE]; // Receive Buffer Array
	UINT8 uchRxBufferByteCount;				// Number of bytes received
	UINT8 uchRxBufferRdIndex; 				// Index to the byte currently being read
	UINT8 uchRxBufferWrIndex; 				// Index to the byte position where received byte is to be written
	UINT8 uchRxFirstByteTimeStamp; 			// stores the timestamp in system ticks of when the first byte was RX
    
	UINT8 uchRxBufferOverflowState;			// Receive buffer overflow state
	//UINT8 uchRxBufferOverflowCount; 
	UINT8 uchCommandRXStatus;	// status - whether no command, receive in progress or command recd 	- used by command handler 
    
	UINT8 uchAssignedUARTChannel; 	// UART channel that is using this buffer 
    
}_stTxRxBuffer; 

/***************************************************************************************/

/***************************************************************************************
 *  Global variables:
****************************************************************************************/
EXTERN UINT8 packetStartIndex; // This indicates the start-of-command index in RX buffer 
EXTERN _stTxRxBuffer	stTxRxBuffer[UART_AVAILABLE_CHANNEL_COUNT];  

/***************************************************************************************/

// Configure the UART and initialize related buffers 
unsigned char configureUART(unsigned char lucUartNumber);  

// Check free UART transmit buffer - If UART transmit buffer is free, then only user should call 'uartSendTxBuffer'
unsigned char uartCheckFreeTxBuffer (unsigned char lucUartNumber, unsigned char* lucPendingNoCharToTx);

// Function to check if a new command is received from CB
UINT8 uartCheckNewCommandReceived (unsigned char lucUartNumber); 

// Function will be used to send transmit buffer on respective serial channel
unsigned char uartSendTxBuffer (unsigned char lucUartNumber, const unsigned char *lpu8Buffer, unsigned char lucCount);

// Check not free state of UART receive buffer
unsigned char uartCheckRxBufferOverflowState (unsigned char lucUartNumber, unsigned char* lucPendingNoCharInRx);

//Function to check not free state of UART receive buffer
unsigned char uartCheckNotFreeRxBuffer (unsigned char lucUartNumber, unsigned char* lucPendingNoCharInRx);

// Function will be used to fetch one byte at a time from "circular global buffer"
unsigned char uartGetRxBuffer (unsigned char lucUartNumber, unsigned char* lucReadData);

// Function to retrieve a received command from the RX buffer of the specified channel 
unsigned char uartGetCommand (unsigned char lucUartNumber, unsigned char* lucReadData, unsigned char* lucDataLength); 

// Function to check if specified UART number is valid 
unsigned char checkUARTChannel(unsigned char lucUartNumber); 

// Check if data received on channel 
BOOL checkDataReadyBit(UINT8 channelNumber); 

// RX interrupt handler method for all 4 UART channels 
void genericRXInterruptHandler(UINT8 channelNumber); 

// TX interrupt handler method for all 4 UART channels 
void genericTXInterruptHandler(UINT8 channelNumber); 

// fetch character received on the specified channel 
UINT8 readCharFromUART(UINT8 channelNumber); 

// write a character to the transmit buffer of the specified channel 
void writeCharToUART(UINT8 channelNumber, UINT8 character); 

// Initialize UART TX RX buffers
void initUARTBuffers(void); 


UINT8 getSafeRxBufferIndex(UINT8 currentIndex); 

UINT8 getRxBufferBytesRecdFrmStartFrame(UINT8 uartBufferIndex, UINT8 packetStartIndex); 



//#endif //SERIAL_H
