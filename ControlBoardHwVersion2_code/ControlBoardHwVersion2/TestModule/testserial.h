/*********************************************************************************
* FileName: test_serial.h
* Description:
* This source file contains the prototype definition of all the services of ....
* Version: 0.1D
*
*
**********************************************************************************/
#ifndef __SERIAL_H__
#define __SERIAL_H__

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
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       Initial Creation
****************************************************************************/


/****************************************************************************
 *  Macro definitions:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global variables:
****************************************************************************/


/****************************************************************************/


/******************************************************************************
 * testConfigrueUART
 *
 * Function Description:

 *
 * Function Parameters:
 *
 * Function Returns:
 *
 ********************************************************************************/

unsigned char testConfigrueUART(unsigned char lucUartNumber);

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

unsigned char testUartCheckFreeTxBuffer(unsigned char lucUartNumber, unsigned char* lucPendingNoCharToTx);

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

unsigned char testUartSendTxBuffer (unsigned char lucUartNumber);

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

unsigned char testUartCheckRxBufferOverflowState (unsigned char lucUartNumber);

/******************************************************************************
 * testUartCheckNotFreeRxBuffer
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

unsigned char testUartCheckNotFreeRxBuffer (unsigned char lucUartNumber);

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
testUartGetRxBuffer (unsigned char lucUartNumber, unsigned char lucBytesToRx);


#endif __SERIAL_H__
