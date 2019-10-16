/*********************************************************************************
 * FileName: test_serial.c
 * Description:
 * This source file is main file for Bx Shutter Control Board
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
#include <stdint.h>
#include <string.h>
#include "Middleware/serial.h"
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include <driverlib/gpio.h>
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"
/****************************************************************************/


/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/
extern unsigned char guchTxBufferIndex [UART_CHANNEL_COUNT];						// Index to the current byte in transmission
extern unsigned char guchRxBufferByteCount [UART_CHANNEL_COUNT];					// Number of bytes received
extern unsigned char guchRxBufferRdIndex [UART_CHANNEL_COUNT];						// Index to the byte currently being read
extern unsigned char guchRxBufferWrIndex [UART_CHANNEL_COUNT];						// Index to the byte position where received
extern unsigned char guchUARTMapToGlobalBuffer[MAX_UART_AVAILABLE];
extern unsigned char guchRxBufferOverflowState[UART_CHANNEL_COUNT];
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
 * testConfigrueUART
 *
 * Function Description:
 *
 *
 * Function Parameter:
 * unsigned char lucUartNumber
 * lucUartNumber 	= 0 for UART0
 * 					= 1 for UART1
 * 					= 2 for UART2
 * 					= 3 for UART3
 * 					= 4 for UART4
 * 					= 5 for UART5
 * 					= 6 for UART6
 *
 * Function Returns:
 * unsigned char lucReturnValue
 *
 ********************************************************************************/

unsigned char testConfigrueUART(unsigned char lucUartNumber)
{
	unsigned char lucReturnValue = SUCCESS;

	lucReturnValue = configureUART(lucUartNumber);
	return lucReturnValue;
}

/******************************************************************************
 * testConfigrueUART
 *
 * Function Description:
 *
 *
 * Function Parameter:
 * unsigned char lucUartNumber
 * lucUartNumber 	= 0 for UART0
 * 					= 1 for UART1
 * 					= 2 for UART2
 * 					= 3 for UART3
 * 					= 4 for UART4
 * 					= 5 for UART5
 * 					= 6 for UART6
 *
 * Function Returns:
 * unsigned char lucReturnValue
 *
 ********************************************************************************/

unsigned char testUartCheckFreeTxBuffer(unsigned char lucUartNumber, unsigned char* lucPendingNoCharToTx)
{
	unsigned char lucReturnValue = SUCCESS;
	unsigned char lucaString[TRANSMIT_BUFFER_SIZE] = "Check Free Buffer\n\r";
	//unsigned char lucPendingCharacters = 0;

	uartSendTxBuffer (lucUartNumber, lucaString, strlen ((const char*)lucaString));

	lucReturnValue = uartCheckFreeTxBuffer(lucUartNumber,lucPendingNoCharToTx);
	if(*lucPendingNoCharToTx)
		while(*lucPendingNoCharToTx)
			uartCheckFreeTxBuffer(lucUartNumber,lucPendingNoCharToTx);

	return lucReturnValue;
}


/******************************************************************************
 * testUartSendTxBuffer
 *
 * Function Description:
 *
 *
 * Function Parameter:
 * unsigned char lucUartNumber
 * lucUartNumber 	= 0 for UART0
 * 					= 1 for UART1
 * 					= 2 for UART2
 * 					= 3 for UART3
 * 					= 4 for UART4
 * 					= 5 for UART5
 * 					= 6 for UART6
 *
 * Function Returns:
 * unsigned char lucReturnValue
 *
 ********************************************************************************/
unsigned char testUartSendTxBuffer (unsigned char lucUartNumber)
{
	unsigned char lucReturnValue = SUCCESS;
	unsigned char lucaString[TRANSMIT_BUFFER_SIZE] = "uartSendTxBuffer Function\n\r";

	lucReturnValue = uartSendTxBuffer(lucUartNumber,lucaString, strlen ((const char*)lucaString));

	return lucReturnValue;
}


/******************************************************************************
 * testUartCheckRxBufferOverflowState
 *
 * Function Description:
 *
 *
 * Function Parameter:
 * unsigned char lucUartNumber
 * lucUartNumber 	= 0 for UART0
 * 					= 1 for UART1
 * 					= 2 for UART2
 * 					= 3 for UART3
 * 					= 4 for UART4
 * 					= 5 for UART5
 * 					= 6 for UART6
 *
 * Function Returns:
 * unsigned char lucReturnValue
 *
 ********************************************************************************/

unsigned char
testUartCheckRxBufferOverflowState (unsigned char lucUartNumber)
{
	unsigned char lucReturnValue = SUCCESS;
	unsigned char lucRxBufferOverflowState = 0;

	lucReturnValue = uartCheckRxBufferOverflowState(lucUartNumber, &lucRxBufferOverflowState);
	if(lucReturnValue == SUCCESS)
		while(!lucRxBufferOverflowState)
			lucReturnValue = uartCheckRxBufferOverflowState(lucUartNumber, &lucRxBufferOverflowState);

	return lucReturnValue;
}


/******************************************************************************
 * testUartCheckRxBufferOverflowState
 *
 * Function Description:
 *
 *
 * Function Parameter:
 * unsigned char lucUartNumber
 * lucUartNumber 	= 0 for UART0
 * 					= 1 for UART1
 * 					= 2 for UART2
 * 					= 3 for UART3
 * 					= 4 for UART4
 * 					= 5 for UART5
 * 					= 6 for UART6
 *
 * Function Returns:
 * unsigned char lucReturnValue
 *
 ********************************************************************************/

unsigned char
testUartCheckNotFreeRxBuffer (unsigned char lucUartNumber)
{
	unsigned char lucReturnValue = SUCCESS;
	unsigned char lucPendingNoCharInRx = 0;

	lucReturnValue = uartCheckNotFreeRxBuffer(lucUartNumber, &lucPendingNoCharInRx);
	if(lucReturnValue == SUCCESS)
		while(!lucPendingNoCharInRx)
			lucReturnValue = uartCheckNotFreeRxBuffer(lucUartNumber, &lucPendingNoCharInRx);

	return lucReturnValue;
}

/******************************************************************************
 * testUartGetRxBuffer
 *
 * Function Description:
 *
 *
 * Function Parameter:
 * unsigned char lucUartNumber
 * lucUartNumber 	= 0 for UART0
 * 					= 1 for UART1
 * 					= 2 for UART2
 * 					= 3 for UART3
 * 					= 4 for UART4
 * 					= 5 for UART5
 * 					= 6 for UART6
 *
 * Function Returns:
 * unsigned char lucReturnValue
 *
 ********************************************************************************/

unsigned char
testUartGetRxBuffer (unsigned char lucUartNumber, unsigned char lucBytesToRx)
{
	unsigned char lucReturnValue = SUCCESS;
	unsigned char lucaReadData[10];
	unsigned char lucReadDataIndex = 0;

	while(lucReadDataIndex < lucBytesToRx)
	{
		lucReturnValue = uartGetRxBuffer(lucUartNumber, &lucaReadData[lucReadDataIndex]);
		if(lucReturnValue == SUCCESS || lucReturnValue == 3)
		{
			while(!guchRxBufferByteCount[guchUARTMapToGlobalBuffer[lucUartNumber]]);
			lucReturnValue = uartGetRxBuffer(lucUartNumber, &lucaReadData[lucReadDataIndex++]);
			if(lucReturnValue != SUCCESS)
				break;
		}
	}
	return lucReturnValue;
}
