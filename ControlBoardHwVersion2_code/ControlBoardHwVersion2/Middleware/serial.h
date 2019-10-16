/***************************************************************************************
* FileName: serial.h
* Description:
* This source file contains the prototype definition of all the services of UART
* Version: 0.1D
*
*
****************************************************************************************/
#ifndef __SERIAL_H__
#define __SERIAL_H__

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
 *  Revision	Date				Name						Comments
 *  0.2D		09/07/2014			iGATE Offshore team			Updated for prototype board from board bring up code
 *  0.1D		27/03/2014			iGATE Offshore team			Initial Creation
****************************************************************************************/

/****************************************************************************
 *  Include:
 ****************************************************************************/

#include <stdint.h>
#include "Middleware/system.h"

/***************************************************************************************
 *  Macro definitions:
****************************************************************************************/
//#define DEBUG_DISABLED

#define DRIVE_BOARD_ADDRESS			0x00
#define CONTROL_BOARD_ADDRESS		0x01
#define DISPLAY_BOARD_ADDRESS		0x02

#define ACK					0x06
#define NACK				0x15

#define DESTINATION_ADDRESS_POSITION		0
#define SOURCE_ADDRESS_POSITION				1
#define FRAME_LENGTH_POSITION			2
#define COMMAND_ID_POSITION				3
#define COMMAND_DATA_POSITION			4

#define RESPONSE_DATA_POSITION			3

#define NUMBER_OF_CRC_BYTES				2

#define	SUCCESS		0
#define	FREE		0xFF

#define UNASSIGNED	0
#define ASSIGNED	1

#define	ENABLE		1
#define	DISABLE		0

#define	UART0		0
#define	UART1		1
#define	UART2		2
#define	UART3		3
#define	UART4		4
#define	UART5		5
#define	UART6		6
#define	UART7		7

#ifdef VERSION_1HARDWARE
#define UART_drive		UART4  //		// shall be UART4 in final code
//#define UART_control	UART0
#define UART_display	UART0 //		// shall be UART0 in final code
#define UART_relay		UART7 //
#define UART_debug		UART6 //		// shall be UART6 in final code

#define	UART0_RS485				ENABLE		// Display
#define	UART1_RS485				DISABLE
#define	UART2_RS485				DISABLE
#define	UART3_RS485				DISABLE
#define	UART4_RS485				ENABLE 		// Drive
#define	UART5_RS485				DISABLE
#define	UART6_RS485				ENABLE		// Debug
#define	UART7_RS485				DISABLE 	//Relay

#define MAX_UART_AVAILABLE			8		// Maximum number of UARTs available in controller
#define UART_CHANNEL_COUNT			4		// Channel 0, Channel 1 and Channel 2

#define	UART0_Available				ENABLE		// Display
#define	UART1_Available				DISABLE
#define	UART2_Available				DISABLE
#define	UART3_Available				DISABLE
#define	UART4_Available				ENABLE 		// Drive
#define	UART5_Available				DISABLE
#define	UART6_Available				ENABLE		// Debug
#define	UART7_Available				DISABLE 	// Relay
#endif	//	VERSION_1HARDWARE


#ifdef VERSION_2HARDWARE

#ifndef VER2_TEST_WITH_VER1_HARDWARE
#define UART_drive		UART1  //		// shall be UART1 in final code
#endif
#ifdef VER2_TEST_WITH_VER1_HARDWARE
#define UART_drive		UART4  //		// shall be UART1 in final code
#endif
//#define UART_control	UART0
#define UART_display	UART0 //		// shall be UART0 in final code
#define UART_relay		UART7 //
#define UART_debug		UART6 //		// shall be UART6 in final code

#define	UART0_RS485				ENABLE		// Display
#define	UART1_RS485				ENABLE		// Drive
#define	UART2_RS485				DISABLE
#define	UART3_RS485				DISABLE
#define	UART4_RS485				DISABLE
#define	UART5_RS485				DISABLE
#define	UART6_RS485				ENABLE		// Debug
#define	UART7_RS485				DISABLE 	//Relay

#define MAX_UART_AVAILABLE			8		// Maximum number of UARTs available in controller
#define UART_CHANNEL_COUNT			4		// Channel 0, Channel 1 and Channel 2

#define	UART0_Available				ENABLE		// Display
#ifndef VER2_TEST_WITH_VER1_HARDWARE
#define	UART1_Available				ENABLE 		// Drive
#endif
#ifdef VER2_TEST_WITH_VER1_HARDWARE
#define	UART1_Available				DISABLE
#endif
#define	UART2_Available				DISABLE
#define	UART3_Available				DISABLE
#ifndef VER2_TEST_WITH_VER1_HARDWARE
#define	UART4_Available				DISABLE
#endif
#ifdef VER2_TEST_WITH_VER1_HARDWARE
#define	UART4_Available				ENABLE 		// Drive
#endif
#define	UART5_Available				DISABLE
#define	UART6_Available				ENABLE		// Debug
#define	UART7_Available				DISABLE 	// Relay

#endif	//	VERSION_2HARDWARE

#define TRANSMIT_BUFFER_SIZE		30
#define RECEIVE_BUFFER_SIZE			30

#define UART0_RS485_DIR_CONTROL		(GPIO_PIN_6)
#define UART4_RS485_DIR_CONTROL		(GPIO_PIN_2)
#define UART7_RS485_DIR_CONTROL		(GPIO_PIN_7)

#ifdef VERSION_2HARDWARE
#define UART1_RS485_DIR_CONTROL		(GPIO_PIN_2)
#endif

#define READ_PORT	0x00
#define WRITE_PORT	0xFF

/***************************************************************************************/


/***************************************************************************************
 *  Global variables:
****************************************************************************************/


/***************************************************************************************/

/***************************************************************************************
 * UART0_RS485_DirectionControl
 *
 * Function Description:
 * Read/Write Direction control for UART0.
 *
 * Function Parameters:
 * uint8_t lucDir
 * lucUartNumber 	= 0x00 for Read
 * 					= 0xFF for Write
 *
 * Function Returns:
 * void
 *
 ***************************************************************************************/

void UART0_RS485_DirectionControl(uint8_t lucDir);

#ifdef VERSION_2HARDWARE
/***************************************************************************************
 * UART1_RS485_DirectionControl
 *
 * Function Description:
 * Read/Write Direction control for UART1.
 *
 * Function Parameters:
 * uint8_t lucDir
 * lucUartNumber 	= 0x00 for Read
 * 					= 0xFF for Write
 *
 * Function Returns:
 * void
 *
 ***************************************************************************************/

void UART1_RS485_DirectionControl(uint8_t lucDir);

#endif
/***************************************************************************************
 * UART4_RS485_DirectionControl
 *
 * Function Description:
 * Read/Write Direction control for UART0.
 *
 * Function Parameters:
 * uint8_t lucDir
 * lucUartNumber 	= 0x00 for Read
 * 					= 0xFF for Write
 *
 * Function Returns:
 * void
 *
 ***************************************************************************************/

void UART4_RS485_DirectionControl(uint8_t lucDir);

/***************************************************************************************
 * UART7_RS485_DirectionControl
 *
 * Function Description:
 * Read/Write Direction control for UART0.
 *
 * Function Parameters:
 * uint8_t lucDir
 * lucUartNumber 	= 0x00 for Read
 * 					= 0xFF for Write
 *
 * Function Returns:
 * void
 *
 ***************************************************************************************/

void UART7_RS485_DirectionControl(uint8_t lucDir);


/***************************************************************************************
 * configureUART
 *
 * Function Description:
 * Configure the UART and its pins.
 *
 * Function Parameters: 
 * uint8_t lucUartNumber
 * lucUartNumber 	= 0 for UART0
 * 					= 1 for UART1
 * 					= 2 for UART2
 * 					= 3 for UART3
 * 					= 4 for UART4
 * 					= 5 for UART5
 * 					= 6 for UART6
 *
 * Function Returns:
 * 		0 = Success
 *		1 = lucUartNumber not in valid range
 *		2 = UART is disabled
 *
 ***************************************************************************************/
uint8_t
configureUART(uint8_t lucUartNumber);



/***************************************************************************************
 * uartCheckFreeTxBuffer
 *
 * Function Description:
 * Check free UART transmit buffer
 * If UART transmit buffer is free, then only user should call 'uartSendTxBuffer'
 *
 * Function Parameters:
 * uint8_t lucUartNumber
 * lucUartNumber 	= 0 for UART0
 * 					= 1 for UART1
 * 					= 2 for UART2
 * 					= 3 for UART3
 * 					= 4 for UART4
 * 					= 5 for UART5
 * 					= 6 for UART6
 *
 * uint8_t lucPendingNoCharToTx
 * Number of characters remaining to transmit.
 *
 * Function Returns: 
 * Pending number of characters to be transmitted.
 *    	0 = Success
 *    	1 = lucUartNumber not in valid range
 *    	2 = Failure
 *
 ***************************************************************************************/
 uint8_t
 uartCheckFreeTxBuffer (uint8_t lucUartNumber, uint8_t* lucPendingNoCharToTx);



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
 * uint8_t lucUartNumber
 * lucUartNumber 	= 0 for UART0
 * 					= 1 for UART1
 * 					= 2 for UART2
 * 					= 3 for UART3
 * 					= 4 for UART4
 * 					= 5 for UART5
 * 					= 6 for UART6
 *
 * const uint8_t *lpu8Buffer
 * *pui8Buffer	= pointer to the buffer to be transmitted
 *
 * uint8_t lucCount
 * uiCount	= number of bytes to be transmitted
 *
 * Function Returns:
 *		0 = Transmission success
 *		1 = lucUartNumber not in valid range
 *		2 = Transmission failure
 *
 ***************************************************************************************/
 uint8_t
 uartSendTxBuffer (uint8_t lucUartNumber, const uint8_t *lpu8Buffer, uint8_t lucCount);


/***************************************************************************************
 * uartCheckRxBufferOverflowState
 *
 * Function Description:
 * Check not free state of UART receive buffer
 * If uart transmit buffer is not free, then only user should called 'uartGetRxBuffer'
 *
 * Function Parameters:
 * uint8_t lucUartNumber
 * lucUartNumber 	= 0 for UART0
 * 					= 1 for UART1
 * 					= 2 for UART2
 * 					= 3 for UART3
 * 					= 4 for UART4
 * 					= 5 for UART5
 * 					= 6 for UART6
 *
 *
 * Function Returns: 
 * Number of characters received
 *    	0 = Success
 *    	1 = lucUartNumber not in valid range
 *    	2 = Failure
 *
 ***************************************************************************************/
 uint8_t
 uartCheckRxBufferOverflowState (uint8_t lucUartNumber, uint8_t* lucPendingNoCharInRx);


 /***************************************************************************************
  * uartCheckNotFreeRxBuffer
  *
  * Function Description:
  * Check not free state of UART receive buffer
  * If UART transmit buffer is not free, then only user should call 'uartGetRxBuffer'
  *
  * Function Parameters:
  * uint8_t lucUartNumber
  * lucUartNumber 	= 0 for UART0
  * 					= 1 for UART1
  * 					= 2 for UART2
  * 					= 3 for UART3
  * 					= 4 for UART4
  * 					= 5 for UART5
  * 					= 6 for UART6
  *
  *
  * Function Returns:
  * Number of characters received
  *    0 = Success
  *    1 = lucUartNumber not in valid range
  *    2 = Specified UART is disabled
  ***************************************************************************************/
 uint8_t
 uartCheckNotFreeRxBuffer (uint8_t lucUartNumber, uint8_t* lucPendingNoCharInRx);

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
 * uint8_t lucUartNumber
 * lucUartNumber 	= 0 for UART0
 * 					= 1 for UART1
 * 					= 2 for UART2
 * 					= 3 for UART3
 * 					= 4 for UART4
 * 					= 5 for UART5
 * 					= 6 for UART6
 *
 * Function Returns:
 * 		0 = Success
 * 		1 = lucUartNumber not in valid range
 * 		2 = Failure
 *
 ***************************************************************************************/
 uint8_t
 uartGetRxBuffer (uint8_t lucUartNumber, uint8_t* lucReadData);




 void clear_uart_buff_cyw(uint8_t lucUartNumber);
#endif /*__SERIAL_H__*/


