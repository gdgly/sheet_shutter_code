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
 *  	0.4D	09/07/2014			iGATE Offshore team				Updated for prototype board from board bring up code
 *  	0.3D	03/07/2014			iGATE Offshore team 			Receive Timeout disabled
 *  	0.2D	06/06/2014			iGATE Offshore team				UART1, UART2, UART3 code added
 *  	0.1D	27/03/2014      	iGATE Offshore team       		Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Include:
 ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "Drivers/ustdlib.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "Middleware/system.h"
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
//#include "inc/hw_types.h"
#include "driverlib/uart.h"
#include "driverlib/rom_map.h"
#include "utils/uartstdio.h"
#include "driverlib/interrupt.h"


//#include "inc/tm4c123gh6pge.h"		//added this line for GPIO
#include "serial.h"
#include "Application/intertaskcommunication.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/

/****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/


/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/
uint8_t guchTxBuffer [UART_CHANNEL_COUNT] [TRANSMIT_BUFFER_SIZE];		// Transmit Buffer Array
uint8_t guchTxBufferByteCount [UART_CHANNEL_COUNT];						// Length of the buffer to be transmitted
uint8_t guchTxBufferIndex [UART_CHANNEL_COUNT];							// Index to the current byte in transmission

uint8_t guchRxBuffer [UART_CHANNEL_COUNT] [RECEIVE_BUFFER_SIZE];		// Receive Buffer Array
uint8_t guchRxBufferByteCount [UART_CHANNEL_COUNT];						// Number of bytes received
uint8_t guchRxBufferRdIndex [UART_CHANNEL_COUNT];						// Index to the byte currently being read
uint8_t guchRxBufferWrIndex [UART_CHANNEL_COUNT];						// Index to the byte position where received
																		// byte is to be written
uint8_t guchRxBufferOverflowState[UART_CHANNEL_COUNT];					// Receive buffer overflow state
uint8_t guchUARTMapToGlobalBuffer[MAX_UART_AVAILABLE] = {FREE,FREE, FREE, FREE, FREE, FREE, FREE};	// Index buffer
uint8_t guchUARTGlobalBufferAssigned[MAX_UART_AVAILABLE] = {UNASSIGNED,UNASSIGNED,UNASSIGNED,UNASSIGNED,UNASSIGNED,UNASSIGNED,UNASSIGNED};

/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
 ****************************************************************************/

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

void UART0_RS485_DirectionControl(uint8_t lucDir)
{
	//
	//	Write lucDir to UART0 direction control pin
	//
	ROM_GPIOPinWrite(GPIO_PORTC_BASE, UART0_RS485_DIR_CONTROL, lucDir);
}
/****************************************************************************/

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

void UART1_RS485_DirectionControl(uint8_t lucDir)
{
	//
	//	Write lucDir to UART0 direction control pin
	//
	ROM_GPIOPinWrite(GPIO_PORTB_BASE, UART1_RS485_DIR_CONTROL, lucDir);
}
/****************************************************************************/
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

void UART4_RS485_DirectionControl(uint8_t lucDir)
{
	//
	//	Write lucDir to UART0 direction control pin
	//
	ROM_GPIOPinWrite(GPIO_PORTA_BASE, UART4_RS485_DIR_CONTROL, lucDir);
}
/****************************************************************************/

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

void UART7_RS485_DirectionControl(uint8_t lucDir)
{
	//
	//	Write lucDir to UART0 direction control pin
	//
	ROM_GPIOPinWrite(GPIO_PORTD_BASE, UART7_RS485_DIR_CONTROL, lucDir);
}
/****************************************************************************/

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
configureUART(uint8_t lucUartNumber)
{
	uint8_t lucReturnValue = SUCCESS;
	uint8_t lucLocalCount = 0;

	switch(lucUartNumber)
	{
	case UART0:		//	DISPLAY PORT
		if(UART0_Available)		//UART0 is enabled
		{
			//
			// Enable the GPIO Peripheral used by the UART.
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

			//
			// Enable UART0
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

			//
			// Set GPIO PA0 and PA1 as UART pins.
			//
			ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

#ifdef VERSION_1HARDWARE
			//
			//	Set GPIO PC6 as output for UART0 direction control
			//
			MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
			ROM_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, UART0_RS485_DIR_CONTROL);
#endif
#ifdef VERSION_2HARDWARE
#ifndef VER2_TEST_WITH_VER1_HARDWARE
			//
			//	Set GPIO PA6 as output for UART0 direction control
			//
			MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
			ROM_GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, UART4_RS485_DIR_CONTROL);
#endif
#ifdef VER2_TEST_WITH_VER1_HARDWARE
			//
			//	Set GPIO PC6 as output for UART0 direction control
			//
			MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
			ROM_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, UART0_RS485_DIR_CONTROL);
#endif
#endif
			//
			//	Enable UART0 to read data
			//

#ifdef VERSION_1HARDWARE
				UART0_RS485_DirectionControl(READ_PORT);
#endif
#ifdef VERSION_2HARDWARE
				//	For client testing in China with new h/w where display transmit enable line is changed
#ifndef VER2_TEST_WITH_VER1_HARDWARE
				UART4_RS485_DirectionControl(READ_PORT);
#endif
#ifdef VER2_TEST_WITH_VER1_HARDWARE
				UART0_RS485_DirectionControl(READ_PORT);
#endif
#endif

			//
			// Configure the UART for 9600, 8-N-1 operation.
			//
			ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 9600,
					(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
							UART_CONFIG_PAR_NONE));

			//
			//Disable Receive and Transmit FIFO
			//
			ROM_UARTFIFODisable(UART0_BASE);

			//
			// Enable the UART interrupt.
			//
			ROM_IntEnable(INT_UART0);

			//
			//	Enable interrupt for end of transmission (EOT) detection
			//
			ROM_UARTTxIntModeSet(UART0_BASE, UART_TXINT_MODE_EOT);

			//
			//	Enable receive, receive time out, transmit interrupt
			//
			ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | /*UART_INT_RT |*/ UART_INT_TX);
		}
		else		//UART0 is disabled
		{
			lucReturnValue = 2;
		}
		break;
	case UART1: //Not updated. Not required
		if(UART1_Available)		//UART1 available
		{
			//
			// Enable the GPIO Peripheral used by the UART.
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

			//
			// Enable UART1
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

			//
			// Set GPIO B0 and B1 as UART pins.
			//

			ROM_GPIOPinConfigure(/*GPIO_PC4_U1RX*/GPIO_PB0_U1RX);
			ROM_GPIOPinConfigure(/*GPIO_PC5_U1TX*/GPIO_PB1_U1TX);
			ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
#ifdef VERSION_2HARDWARE
			//
			//	Set GPIO PB2 as output for UART4 direction control
			//
			MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
			ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, UART1_RS485_DIR_CONTROL);

			//
			//	Enable UART4 to read data
			//
			UART1_RS485_DirectionControl(READ_PORT);
#endif	//	VERSION_2HARDWARE
			//
			// Configure the UART for 9600, 8-N-1 operation.
			//
			ROM_UARTConfigSetExpClk(UART1_BASE, ROM_SysCtlClockGet(), 9600,
					(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
							UART_CONFIG_PAR_NONE));

			//
			//Disable Receive and Transmit FIFO
			//
			ROM_UARTFIFODisable(UART1_BASE);

			//
			// Enable the UART interrupt.
			//
			ROM_IntEnable(INT_UART1);

			//
			//	Enable interrupt for end of transmission (EOT) detection
			//
			ROM_UARTTxIntModeSet(UART1_BASE, UART_TXINT_MODE_EOT);

			//
			//	Enable receive, receive time out, transmit interrupt
			//
			ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | /*UART_INT_RT |*/ UART_INT_TX);
		}
		else	//UART1 is disabled
		{
			lucReturnValue = 2;
		}
		break;
	case UART2:  //Not updated. Not required
		if(UART2_Available)		//UART2 available
		{
			//
			// Enable the GPIO Peripheral used by the UART.
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

			//
			// Enable UART2
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

			//
			// Set GPIO D6 and D7 as UART pins.
			//

			ROM_GPIOPinConfigure(GPIO_PD6_U2RX);
			ROM_GPIOPinConfigure(GPIO_PD7_U2TX);
			ROM_GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_6 | GPIO_PIN_7);

			//
			// Configure the UART for 115200, 8-N-1 operation.
			//
			ROM_UARTConfigSetExpClk(UART2_BASE, ROM_SysCtlClockGet(), 38400,
					(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
							UART_CONFIG_PAR_NONE));

			//
			//Disable Receive and Transmit FIFO
			//
			ROM_UARTFIFODisable(UART2_BASE);

			//
			// Enable the UART interrupt.
			//
			ROM_IntEnable(INT_UART2);

			//
			//	Enable interrupt for end of transmission (EOT) detection
			//
			ROM_UARTTxIntModeSet(UART2_BASE, UART_TXINT_MODE_EOT);

			//
			//	Enable receive, receive time out, transmit interrupt
			//
			ROM_UARTIntEnable(UART2_BASE, UART_INT_RX | /*UART_INT_RT |*/ UART_INT_TX);
		}
		else	//UART2 is disabled
		{
			lucReturnValue = 2;
		}
		break;
	case UART3:  //Not updated. Not required
		if(UART3_Available)		//UART3 available
		{
			//
			// Enable the GPIO Peripheral used by the UART.
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

			//
			// Enable UART1
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART3);

			//
			// Set GPIO A0 and A1 as UART pins.
			//

			ROM_GPIOPinConfigure(GPIO_PC6_U3RX);
			ROM_GPIOPinConfigure(GPIO_PC7_U3TX);
			ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_7);

			//
			// Configure the UART for 115200, 8-N-1 operation.
			//
			ROM_UARTConfigSetExpClk(UART3_BASE, ROM_SysCtlClockGet(), 38400,
					(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
							UART_CONFIG_PAR_NONE));

			//
			//Disable Receive and Transmit FIFO
			//
			ROM_UARTFIFODisable(UART3_BASE);

			//
			// Enable the UART interrupt.
			//
			ROM_IntEnable(INT_UART3);

			//
			//	Enable interrupt for end of transmission (EOT) detection
			//
			ROM_UARTTxIntModeSet(UART3_BASE, UART_TXINT_MODE_EOT);

			//
			//	Enable receive, receive time out, transmit interrupt
			//
			ROM_UARTIntEnable(UART3_BASE, UART_INT_RX | /*UART_INT_RT |*/ UART_INT_TX);
		}
		else	//UART3 is disabled
		{
			lucReturnValue = 2;
		}
		break;
	case UART4: // Drive UART
		if(UART4_Available)		//UART4 available
		{
			//
			// Enable the GPIO Peripheral used by the UART.
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

			//
			// Enable UART4
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART4);

			//
			// Set GPIO PC4 and PC5 as UART pins.
			//

			ROM_GPIOPinConfigure(GPIO_PC4_U4RX);
			ROM_GPIOPinConfigure(GPIO_PC5_U4TX);
			ROM_GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

			//
			//	Set GPIO PA2 as output for UART4 direction control
			//
			MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
			ROM_GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, UART4_RS485_DIR_CONTROL);

			//
			//	Enable UART4 to read data
			//
			UART4_RS485_DirectionControl(READ_PORT);

			//
			// Configure the UART for 9600, 8-N-1 operation.
			//
			ROM_UARTConfigSetExpClk(UART4_BASE, ROM_SysCtlClockGet(), 9600,
					(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
							UART_CONFIG_PAR_NONE));

			//
			//Disable Receive and Transmit FIFO
			//
			ROM_UARTFIFODisable(UART4_BASE);

			//
			// Enable the UART interrupt.
			//
			ROM_IntEnable(INT_UART4);

			//
			//	Enable interrupt for end of transmission (EOT) detection
			//
			ROM_UARTTxIntModeSet(UART4_BASE, UART_TXINT_MODE_EOT);

			//
			//	Enable receive, receive time out, transmit interrupt
			//
			ROM_UARTIntEnable(UART4_BASE, UART_INT_RX | /*UART_INT_RT |*/ UART_INT_TX);
		}
		else	//UART4 is disabled
		{
			lucReturnValue = 2;
		}
		break;
	case UART5:  //Not updated. Not required
		if(UART5_Available)		//UART5 available
		{

		}
		else	//UART5 is disabled
		{
			lucReturnValue = 2;
		}
		break;
	case UART6: // Debug Port RS232
		if(UART6_Available)		//UART6 available
		{
			//
			// Enable the GPIO Peripheral used by the UART.
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

			//
			// Enable UART1
			//
			ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART6);

			//
			// Set GPIO PD4 and PD5 as UART pins.
			//

			ROM_GPIOPinConfigure(GPIO_PD4_U6RX);
			ROM_GPIOPinConfigure(GPIO_PD5_U6TX);
			ROM_GPIOPinTypeUART(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);

			//
			// Configure the UART for 38400, 8-N-1 operation.
			//
			ROM_UARTConfigSetExpClk(UART6_BASE, ROM_SysCtlClockGet(), 38400,
					(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
							UART_CONFIG_PAR_NONE));

			//
			//Disable Receive and Transmit FIFO
			//
			ROM_UARTFIFODisable(UART6_BASE);

			//
			// Enable the UART interrupt.
			//
			ROM_IntEnable(INT_UART6);

			//
			//	Enable interrupt for end of transmission (EOT) detection
			//
			ROM_UARTTxIntModeSet(UART6_BASE, UART_TXINT_MODE_EOT);

			//
			//	Enable receive, receive time out, transmit interrupt
			//
			ROM_UARTIntEnable(UART6_BASE, UART_INT_RX | /*UART_INT_RT |*/ UART_INT_TX);
		}
		else	//UART6 is disabled
		{
			lucReturnValue = 2;
		}
		break;
	case UART7: // Relay. Not updated. Not required
			if(UART7_Available)		//UART7 available
			{
				//
				// Enable the GPIO Peripheral used by the UART.
				//
				ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

				//
				// Enable UART7
				//
				ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);

				//
				// Set GPIO PE0 and PE1 as UART pins.
				//

				ROM_GPIOPinConfigure(GPIO_PE0_U7RX);
				ROM_GPIOPinConfigure(GPIO_PE1_U7TX);
				ROM_GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1);

				//
				//	Set GPIO PD7 as output for UART0 direction control
				//
				MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
				ROM_GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, UART7_RS485_DIR_CONTROL);

				//
				//	Enable UART7 to read data
				//
				UART7_RS485_DirectionControl(READ_PORT);

				//
				// Configure the UART for 38400, 8-N-1 operation.
				//
				ROM_UARTConfigSetExpClk(UART7_BASE, ROM_SysCtlClockGet(), 38400,
						(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
								UART_CONFIG_PAR_NONE));

				//
				//Disable Receive and Transmit FIFO
				//
				ROM_UARTFIFODisable(UART7_BASE);

				//
				// Enable the UART interrupt.
				//
				ROM_IntEnable(INT_UART7);

				//
				//	Enable interrupt for end of transmission (EOT) detection
				//
				ROM_UARTTxIntModeSet(UART7_BASE, UART_TXINT_MODE_EOT);

				//
				//	Enable receive, receive time out, transmit interrupt
				//
				ROM_UARTIntEnable(UART7_BASE, UART_INT_RX | /*UART_INT_RT |*/ UART_INT_TX);
			}
			else	//UART6 is disabled
			{
				lucReturnValue = 2;
			}
			break;
	default:	//Specified UART number is not in valid range
		lucReturnValue = 1;
		break;
	}

	if(lucReturnValue == SUCCESS)	//UART configured successfully
	{
		//
		// Assign transmit buffer map number for UART0
		//
#ifdef APP
		for(lucLocalCount=0;lucLocalCount<MAX_UART_AVAILABLE;lucLocalCount++)
		{
			if(guchUARTMapToGlobalBuffer[lucLocalCount]==FREE && (
					(UART0_Available && 0 == lucUartNumber) ||
					(UART1_Available && 1 == lucUartNumber) ||
					(UART2_Available && 2 == lucUartNumber) ||
					(UART3_Available && 3 == lucUartNumber) ||
					(UART4_Available && 4 == lucUartNumber) ||
					(UART5_Available && 5 == lucUartNumber) ||
					(UART6_Available && 6 == lucUartNumber)
			)
			)
			{
				guchUARTMapToGlobalBuffer[lucUartNumber] = lucLocalCount;
			}
		}
#endif
		if(guchUARTMapToGlobalBuffer[lucUartNumber] == FREE)	//Array index for specified UART not mapped?
		{
			for(lucLocalCount=0;lucLocalCount<MAX_UART_AVAILABLE;lucLocalCount++)
			{
				if(/*guchUARTMapToGlobalBuffer[lucLocalCount]==FREE &&*/ guchUARTGlobalBufferAssigned[lucLocalCount]==UNASSIGNED)
				{
					guchUARTMapToGlobalBuffer[lucUartNumber] = lucLocalCount;	//Assign array index number
					guchUARTGlobalBufferAssigned[lucLocalCount] = ASSIGNED;
					break;
				}
			}
		}
		//
		//	Initialize transmit byte count, transmit index, receive byte count,
		//	receive read and write index, receive overflow state
		//
		guchTxBufferByteCount [guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
		guchTxBufferIndex [guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
		guchRxBufferByteCount [guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
		guchRxBufferRdIndex [guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
		guchRxBufferWrIndex [guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
		guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
	}
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
 *    0 = Success
 *    1 = lucUartNumber not in valid range
 *    2 = Specified UART is disabled
 *
 ***************************************************************************************/
uint8_t
uartCheckFreeTxBuffer (uint8_t lucUartNumber, uint8_t* lucPendingNoCharToTx)
{
	uint8_t lucReturnValue = SUCCESS;

	switch(lucUartNumber)	//	Check if specified UART is disabled
	{
	case UART0:
		if(!UART0_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART1:
		if(!UART1_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART2:
		if(!UART2_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART3:
		if(!UART3_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART4:
		if(!UART4_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART5:
		if(!UART5_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART6:
		if(!UART6_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART7:
		if(!UART7_Available)
		{
			lucReturnValue = 2;
		}
		break;
	default:
		lucReturnValue = 1;
		break;
	}	//Switch ends here

	if (lucReturnValue == SUCCESS)	//	Specified UART is enabled
	{
		//
		//	Read pending number of bytes to transmit from global buffer
		//
		*lucPendingNoCharToTx = guchTxBufferByteCount[guchUARTMapToGlobalBuffer[lucUartNumber]];
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
 *		2 = Specified UART is disabled
 *		3 = Channel Busy
 *
 ***************************************************************************************/
uint8_t
uartSendTxBuffer (uint8_t lucUartNumber, const uint8_t *lpu8Buffer, uint8_t lucCount)
{
	uint8_t lucReturnValue = SUCCESS;
	uint8_t lucPendingNoCharToTx = 0;
	uint8_t copyCount;

	// Local variable for delay after transmit enable
	uint32_t luintDelay;

	//
	//	Default UART
	//
	uint32_t UART_BASE = UART0_BASE;

	//
	//	Check whether specified UART is busy in transmission
	//
	while(lucPendingNoCharToTx != 0)
	{
		lucReturnValue = uartCheckFreeTxBuffer(lucUartNumber, &lucPendingNoCharToTx);
		if(lucReturnValue != 0)
		{
			lucPendingNoCharToTx = 0;
		}
	}
	//
	//	uartCheckFreeTxBuffer unsuccessful or UART busy in transmission
	//
	if(lucReturnValue || lucPendingNoCharToTx)
	{
		if(lucPendingNoCharToTx)	// UART busy
			lucReturnValue = 3;
	}
	else	//else transmit
	{
		switch(lucUartNumber)	//	Check if specified UART is disabled
		{
		case UART0:
			UART_BASE = UART0_BASE;
//			UART0_RS485_DirectionControl(WRITE_PORT);

			// wait for enable line to stabilize
//			for (luintDelay= 0;luintDelay < 0x2492;luintDelay++);

			if(!UART0_Available)
			{
				lucReturnValue = 2;
			}
			break;
		case UART1:
			UART_BASE = UART1_BASE;
			if(!UART1_Available)
			{
				lucReturnValue = 2;
			}
			break;
		case UART2:
			UART_BASE = UART2_BASE;
			if(!UART2_Available)
			{
				lucReturnValue = 2;
			}
			break;
		case UART3:
			UART_BASE = UART3_BASE;
			if(!UART3_Available)
			{
				lucReturnValue = 2;
			}
			break;
		case UART4:
			UART_BASE = UART4_BASE;
//			UART4_RS485_DirectionControl(WRITE_PORT);

			// wait for enable line to stabilize
//			for (luintDelay= 0;luintDelay < 0x2492;luintDelay++);

			if(!UART4_Available)
			{
				lucReturnValue = 2;
			}
			break;
		case UART5:
			UART_BASE = UART5_BASE;
			if(!UART5_Available)
			{
				lucReturnValue = 2;
			}
			break;
		case UART6:
			UART_BASE = UART6_BASE;
			if(!UART6_Available)
			{
				lucReturnValue = 2;
			}
			break;
		case UART7:
			UART_BASE = UART7_BASE;
//			UART7_RS485_DirectionControl(WRITE_PORT);

			// wait for enable line to stabilize
//			for (luintDelay= 0;luintDelay < 0x2492;luintDelay++);

			if(!UART7_Available)
			{
				lucReturnValue = 2;
			}
			break;
		default:
			lucReturnValue = 1;
			break;
		}	//	switch ends here

		if (lucReturnValue == SUCCESS)	//	Specified UART is enabled
		{
			//
			//	Clear earlier data
			//
			memset(guchTxBuffer,'\0',sizeof((const char*)guchTxBuffer[guchUARTMapToGlobalBuffer[lucUartNumber]][0]));
			//
			//	Copy data buffer to be transmitted pointed by lpu8Buffer into global buffer
			//
			//usprintf((char *)&guchTxBuffer[guchUARTMapToGlobalBuffer[lucUartNumber]][0],"%s",lpu8Buffer);
			for(copyCount=0;copyCount<lucCount;copyCount++)
			{
				guchTxBuffer[guchUARTMapToGlobalBuffer[lucUartNumber]][copyCount] = *lpu8Buffer++;
			}
			//
			//	Copy number of bytes to be transmitted into global buffer
			//
			guchTxBufferByteCount[guchUARTMapToGlobalBuffer[lucUartNumber]] = lucCount;
			//
			//	Clear transmit buffer index
			//
			guchTxBufferIndex[guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
			//
			//	Transmit the first byte in global buffer to initiate transmit interrupt.
			//	Transmission of remaining bytes would be handled by corresponding UART interrupt
			//
			if(lucUartNumber == UART0)
			{
#ifdef VERSION_1HARDWARE
				UART0_RS485_DirectionControl(WRITE_PORT);
#endif
#ifdef VERSION_2HARDWARE
				//	For client testing in China with new h/w where display transmit enable line is changed
#ifndef VER2_TEST_WITH_VER1_HARDWARE
				UART4_RS485_DirectionControl(WRITE_PORT);
#endif
#ifdef VER2_TEST_WITH_VER1_HARDWARE
				UART0_RS485_DirectionControl(WRITE_PORT);
#endif
#endif
			}
			if(lucUartNumber == UART1)
			{
				UART1_RS485_DirectionControl(WRITE_PORT);
			}
			if(lucUartNumber == UART4)
			{
				UART4_RS485_DirectionControl(WRITE_PORT);
			}
			if(lucUartNumber == UART7)
			{
				UART7_RS485_DirectionControl(WRITE_PORT);
			}
			ROM_UARTCharPut(UART_BASE, *&guchTxBuffer[guchUARTMapToGlobalBuffer[lucUartNumber]][0]);
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
uartCheckRxBufferOverflowState (uint8_t lucUartNumber, uint8_t* lucRxBufferOverflowState)
{
	uint8_t lucReturnValue = SUCCESS;

	switch(lucUartNumber)	//	Check if specified UART is disabled
	{
	case UART0:
		if(!UART0_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART1:
		if(!UART1_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART2:
		if(!UART2_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART3:
		if(!UART3_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART4:
		if(!UART4_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART5:
		if(!UART5_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART6:
		if(!UART6_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART7:
		if(!UART7_Available)
		{
			lucReturnValue = 2;
		}
		break;
	default:
		lucReturnValue = 1;
		break;
	}	//	switch ends here

	if(lucReturnValue == SUCCESS)	//	Specified UART is enabled
		//
		//	Read Rx buffer overflow state from global buffer
		//
		*lucRxBufferOverflowState = guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[lucUartNumber]];

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
uint8_t cywcywcyw=0;
uint8_t
uartCheckNotFreeRxBuffer (uint8_t lucUartNumber, uint8_t* lucPendingNoCharInRx)
{
	uint8_t lucReturnValue = SUCCESS;

	switch(lucUartNumber)	//	Check if specified UART is disabled
	{
	case UART0:
		if(!UART0_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART1:
		if(!UART1_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART2:
		if(!UART2_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART3:
		if(!UART3_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART4:
		if(!UART4_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART5:
		if(!UART5_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART6:
		if(!UART6_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART7:
		if(!UART7_Available)
		{
			lucReturnValue = 2;
		}
		break;
	default:
		lucReturnValue = 1;
		break;
	}

	if(lucReturnValue == SUCCESS)
		*lucPendingNoCharInRx = guchRxBufferByteCount[guchUARTMapToGlobalBuffer[lucUartNumber]];


	cywcywcyw = *lucPendingNoCharInRx;

	if(cywcywcyw!=0)
	{
		return lucReturnValue;
	}

	return lucReturnValue;
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
 * 		2 = Specified UART is disabled
 * 		3 = Global receive buffer is empty
 *
 ***************************************************************************************/
uint8_t
uartGetRxBuffer (uint8_t lucUartNumber, uint8_t* lucReadData)
{
	uint8_t lucReturnValue = SUCCESS;

	switch(lucUartNumber)
	{
	case UART0:
		if(!UART0_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART1:
		if(!UART1_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART2:
		if(!UART2_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART3:
		if(!UART3_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART4:
		if(!UART4_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART5:
		if(!UART5_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART6:
		if(!UART6_Available)
		{
			lucReturnValue = 2;
		}
		break;
	case UART7:
		if(!UART7_Available)
		{
			lucReturnValue = 2;
		}
		break;
	default:
		lucReturnValue = 1;
		break;
	}

	if(lucReturnValue == SUCCESS)
	{
		if(guchRxBufferByteCount[guchUARTMapToGlobalBuffer[lucUartNumber]] == 0)	//	Global receive buffer is empty
		{
			lucReturnValue = 3;
		}
		else	//	Data available in global receive buffer
		{
			//
			//	Copy data from global buffer corresponding to specified UART
			//
			*lucReadData = guchRxBuffer[guchUARTMapToGlobalBuffer[lucUartNumber]][guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[lucUartNumber]]];
			//
			//	Increment read index
			//
			guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[lucUartNumber]]++;
			//
			//	Reset read index if it exceeds maximum receive buffer size
			//
			if(guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[lucUartNumber]] >= RECEIVE_BUFFER_SIZE)
				guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;

			//
			//	Decrement read byte count
			//
			if(guchRxBufferByteCount[guchUARTMapToGlobalBuffer[lucUartNumber]])
				guchRxBufferByteCount[guchUARTMapToGlobalBuffer[lucUartNumber]]--;
		}
	}

	return lucReturnValue;
}


//*****************************************************************************
//
// The UART0 interrupt handler.
//
//*****************************************************************************
void
UART0IntHandler(void)
{
	uint32_t lui32Status;

	//
	// Get the interrupt status.
	//
	lui32Status = ROM_UARTIntStatus(UART0_BASE, true);

	//
	// Clear the asserted interrupts.
	//
	ROM_UARTIntClear(UART0_BASE, lui32Status);

	if(lui32Status & UART_INT_TX)
	{
		if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART0]])
		{
			guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART0]]--;	//Decremented for byte sent previously
			guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART0]]++;		//Incremented for byte sent previously
			if(guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART0]] >= TRANSMIT_BUFFER_SIZE)
				guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART0]] = 0;
			if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART0]])
			{
				ROM_UARTCharPut(UART0_BASE,
						*&guchTxBuffer[guchUARTMapToGlobalBuffer[UART0]][guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART0]]]);
			}
			else
			{
#ifdef VERSION_1HARDWARE
				UART0_RS485_DirectionControl(READ_PORT);
#endif
#ifdef VERSION_2HARDWARE
				//	For client testing in China with new h/w where display transmit enable line is changed
#ifndef VER2_TEST_WITH_VER1_HARDWARE
				UART4_RS485_DirectionControl(READ_PORT);
#endif
#ifdef VER2_TEST_WITH_VER1_HARDWARE
				UART0_RS485_DirectionControl(READ_PORT);
#endif
#endif
			}
		}
	}
	if(lui32Status & UART_INT_RX)
	{
		//
		// Loop while there are characters in the receive FIFO.
		//
		while(ROM_UARTCharsAvail(UART0_BASE))
		{
			//
			// Read the next character from the UART
			//

			guchRxBuffer[guchUARTMapToGlobalBuffer[UART0]][guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART0]]] = ROM_UARTCharGetNonBlocking(UART0_BASE);

			//
			// Increment write index
			//
			guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART0]]++;
			if(guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART0]] >= RECEIVE_BUFFER_SIZE)
			{
				//
				// Reset write index
				//
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART0]] = 0;
			}

			//
			// Increment received byte count
			//
			guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART0]]++;
			if(guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART0]] >= RECEIVE_BUFFER_SIZE)
			{
				//
				// Set overflow flag
				//
				guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART0]] = 1;

				//
				// Set overflow flag in global status register
				//
				gstControlBoardStatus.bits.controlFault = 1;
				gstControlBoardFault.bits.controlCommunication = 1;
				gstControlCommunicationFault.bits.uartErrorDisplay = 1;

				//
				// Reset received byte count
				//
				guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART0]] = 0;
				//
				// Reset Read index
				//
				guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[UART0]] = 0;
				//
				// Reset Write index
				//
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART0]] = 0;
			}

		} // while(ROM_UARTCharsAvail(UART0_BASE))
	}
}

//*****************************************************************************
//
// The UART1 interrupt handler.
//
//*****************************************************************************
void
UART1IntHandler(void)
{
	uint32_t lui32Status;

	//
	// Get the interrupt status.
	//
	lui32Status = ROM_UARTIntStatus(UART1_BASE, true);

	//
	// Clear the asserted interrupts.
	//
	ROM_UARTIntClear(UART1_BASE, lui32Status);

	if(lui32Status & UART_INT_TX)
	{
		if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART1]])
		{
			guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART1]]--;	//Decremented for byte sent previously
			guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART1]]++;		//Incremented for byte sent previously
			if(guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART1]] >= TRANSMIT_BUFFER_SIZE)
				guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART1]] = 0;
			if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART1]])
			{
				ROM_UARTCharPut(UART1_BASE, *&guchTxBuffer[guchUARTMapToGlobalBuffer[UART1]][guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART1]]]);
			}
			else
			{
				//
				//	Enable RS485 receive
				//
				UART1_RS485_DirectionControl(READ_PORT);
			}
		}
	}
	if(lui32Status & UART_INT_RX)
	{
		//
		// Loop while there are characters in the receive FIFO.
		//
		while(ROM_UARTCharsAvail(UART1_BASE))
		{
			//
			// Read the next character from the UART
			//

			guchRxBuffer[guchUARTMapToGlobalBuffer[UART1]][guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART1]]] = ROM_UARTCharGetNonBlocking(UART1_BASE);
			//
			// Increment write index
			//
			guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART1]]++;
			if(guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART1]] >= RECEIVE_BUFFER_SIZE)
			{
				//
				// Reset write index
				//
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART1]] = 0;
			}

			//
			// Increment receive byte count
			//
			guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART1]]++;
			if(guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART1]] >= RECEIVE_BUFFER_SIZE)
			{
				//
				// Set overflow flag in global status register
				//
				gstControlBoardStatus.bits.controlFault = 1;
				gstControlBoardFault.bits.controlCommunication = 1;
				gstControlCommunicationFault.bits.uartErrorDrive = 1;

				// set overflow flag
				guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART1]] = 1;
				// reset received byte count
				guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART1]] = 0;
				// reset read index
				guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[UART1]] = 0;
				// reset write index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART1]] = 0;
			}


		}
	}
}


//*****************************************************************************
//
// The UART2 interrupt handler.
//
//*****************************************************************************
void
UART2IntHandler(void)
{
	uint32_t lui32Status;

	//
	// Get the interrupt status.
	//
	lui32Status = ROM_UARTIntStatus(UART2_BASE, true);

	//
	// Clear the asserted interrupts.
	//
	ROM_UARTIntClear(UART2_BASE, lui32Status);

	if(lui32Status & UART_INT_TX)
	{
		if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART2]])
		{
			guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART2]]--;	//Decremented for byte sent previously
			guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART2]]++;		//Incremented for byte sent previously
			if(guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART2]] >= TRANSMIT_BUFFER_SIZE)
				guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART2]] = 0;
			if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART2]])
				ROM_UARTCharPut(UART2_BASE, *&guchTxBuffer[guchUARTMapToGlobalBuffer[UART2]][guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART2]]]);
		}
	}
	if(lui32Status & UART_INT_RX)
	{
		//
		// Loop while there are characters in the receive FIFO.
		//
		while(ROM_UARTCharsAvail(UART2_BASE))
		{
			//
			// Read the next character from the UART
			//

			guchRxBuffer[guchUARTMapToGlobalBuffer[UART2]][guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART2]]] = ROM_UARTCharGetNonBlocking(UART2_BASE);
			// Increment write index
			guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART2]]++;
			if(guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART2]] >= RECEIVE_BUFFER_SIZE)
			{
				// Reset write index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART2]] = 0;
			}
			// Increment received byte count
			guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART2]]++;
			if(guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART2]] >= RECEIVE_BUFFER_SIZE)
			{
				guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART2]] = 1;
				// reset received byte count
				guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART2]] = 0;
				// reset read index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART2]] = 0;
				// reset write index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART2]] = 0;
			}
		}
	}
}


//*****************************************************************************
//
// The UART3 interrupt handler.
//
//*****************************************************************************
void
UART3IntHandler(void)
{
	uint32_t lui32Status;

	//
	// Get the interrupt status.
	//
	lui32Status = ROM_UARTIntStatus(UART3_BASE, true);

	//
	// Clear the asserted interrupts.
	//
	ROM_UARTIntClear(UART3_BASE, lui32Status);

	if(lui32Status & UART_INT_TX)
	{
		if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART3]])
		{
			guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART3]]--;	//Decremented for byte sent previously
			guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART3]]++;		//Incremented for byte sent previously
			if(guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART3]] >= TRANSMIT_BUFFER_SIZE)
				guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART3]] = 0;
			if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART3]])
				ROM_UARTCharPut(UART3_BASE, *&guchTxBuffer[guchUARTMapToGlobalBuffer[UART3]][guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART3]]]);
		}
	}
	if(lui32Status & UART_INT_RX)
	{
		//
		// Loop while there are characters in the receive FIFO.
		//
		while(ROM_UARTCharsAvail(UART3_BASE))
		{
			//
			// Read the next character from the UART
			//

			guchRxBuffer[guchUARTMapToGlobalBuffer[UART3]][guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART3]]] = ROM_UARTCharGetNonBlocking(UART3_BASE);
			// Increment write index
			guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART3]]++;
			if(guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART3]] >= RECEIVE_BUFFER_SIZE)
			{
				// Reset write index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART3]] = 0;
			}
			// Increment received byte count
			guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART3]]++;
			if(guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART3]] >= RECEIVE_BUFFER_SIZE)
			{
				guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART3]] = 1;
				// Reset received byte count
				guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART3]] = 0;
				// Reset read index
				guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[UART3]] = 0;
				// Reset write index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART3]] = 0;
			}

		}
	}
}


//*****************************************************************************
//
// The UART4 interrupt handler.
//
//*****************************************************************************
void
UART4IntHandler(void)
{
	uint32_t lui32Status;

	//
	// Get the interrupt status.
	//
	lui32Status = ROM_UARTIntStatus(UART4_BASE, true);

	//
	// Clear the asserted interrupts.
	//
	ROM_UARTIntClear(UART4_BASE, lui32Status);

	if(lui32Status & UART_INT_TX)
	{
		if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART4]])
		{
			guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART4]]--;	//Decremented for byte sent previously
			guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART4]]++;		//Incremented for byte sent previously
			if(guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART4]] >= TRANSMIT_BUFFER_SIZE)
				guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART4]] = 0;
			if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART4]])
			{
				ROM_UARTCharPut(UART4_BASE,
						*&guchTxBuffer[guchUARTMapToGlobalBuffer[UART4]][guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART4]]]);
			}
			else
			{
#ifndef DEBUG_DISABLED
				//
				//	Debug comment added to verify that complete command frame is transmitted to drive (19 Jul 14)
				//
#endif

				//
				//	Enable RS485 receive
				//
				UART4_RS485_DirectionControl(READ_PORT);
			}
		}
	}
	if(lui32Status & UART_INT_RX)
	{
		//
		// Loop while there are characters in the receive FIFO.
		//
		while(ROM_UARTCharsAvail(UART4_BASE))
		{
			//
			// Read the next character from the UART
			//

			guchRxBuffer[guchUARTMapToGlobalBuffer[UART4]][guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART4]]]
			                                               = ROM_UARTCharGetNonBlocking(UART4_BASE);

			// Increment write index
			guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART4]]++;
			if(guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART4]] >= RECEIVE_BUFFER_SIZE)
			{
				// Reset write index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART4]] = 0;
			}
			// Increment received byte count
			guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART4]]++;
			if(guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART4]] >= RECEIVE_BUFFER_SIZE)
			{
				//
				// Set overflow flag in global status register
				//
				gstControlBoardStatus.bits.controlFault = 1;
				gstControlBoardFault.bits.controlCommunication = 1;
				gstControlCommunicationFault.bits.uartErrorDrive = 1;

				guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART4]] = 1;
				// Reset received byte count
				guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART4]] = 0;
				// Reset read index
				guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[UART4]] = 0;
				// Reset write index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART4]] = 0;
			}

		}
	}
}


//*****************************************************************************
//
// The UART6 interrupt handler.
//
//*****************************************************************************
void
UART6IntHandler(void)
{
	uint32_t lui32Status;

	//
	// Get the interrupt status.
	//
	lui32Status = ROM_UARTIntStatus(UART6_BASE, true);

	//
	// Clear the asserted interrupts.
	//
	ROM_UARTIntClear(UART6_BASE, lui32Status);

	if(lui32Status & UART_INT_TX)
	{
		if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART6]])
		{
			guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART6]]--;	//Decremented for byte sent previously
			guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART6]]++;		//Incremented for byte sent previously
			if(guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART6]] >= TRANSMIT_BUFFER_SIZE)
				guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART6]] = 0;
			if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART6]])
				ROM_UARTCharPut(UART6_BASE,
						*&guchTxBuffer[guchUARTMapToGlobalBuffer[UART6]][guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART6]]]);
		}
	}
	if(lui32Status & UART_INT_RX)
	{
		//
		// Loop while there are characters in the receive FIFO.
		//
		while(ROM_UARTCharsAvail(UART6_BASE))
		{
			//
			// Read the next character from the UART
			//

			guchRxBuffer[guchUARTMapToGlobalBuffer[UART6]][guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART6]]] = ROM_UARTCharGetNonBlocking(UART6_BASE);
			// increment write index
			guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART6]]++;
			if(guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART6]] >= RECEIVE_BUFFER_SIZE)
			{
				// reset write index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART6]] = 0;
			}
			// increment received byte count
			guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART6]]++;
			if(guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART6]] >= RECEIVE_BUFFER_SIZE)
			{
				guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART6]] = 1;
				// reset received byte count
				guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART6]] = 0;
				// reset read index
				guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[UART6]] = 0;
				//reset write index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART6]] = 0;
			}
		}
	}
}

//*****************************************************************************
//
// The UART7 interrupt handler.
//
//*****************************************************************************
void
UART7IntHandler(void)
{
	uint32_t lui32Status;

	//
	// Get the interrupt status.
	//
	lui32Status = ROM_UARTIntStatus(UART7_BASE, true);

	//
	// Clear the asserted interrupts.
	//
	ROM_UARTIntClear(UART7_BASE, lui32Status);

	if(lui32Status & UART_INT_TX)
	{
		if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART7]])
		{
			guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART7]]--;	//Decremented for byte sent previously
			guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART7]]++;		//Incremented for byte sent previously
			if(guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART7]] >= TRANSMIT_BUFFER_SIZE)
				guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART7]] = 0;
			if(guchTxBufferByteCount[guchUARTMapToGlobalBuffer[UART7]])
			{
				ROM_UARTCharPut(UART7_BASE,
						*&guchTxBuffer[guchUARTMapToGlobalBuffer[UART7]][guchTxBufferIndex[guchUARTMapToGlobalBuffer[UART7]]]);
			}
			else
			{
				UART7_RS485_DirectionControl(READ_PORT);
			}
		}
	}
	if(lui32Status & UART_INT_RX)
	{
		//
		// Loop while there are characters in the receive FIFO.
		//
		while(ROM_UARTCharsAvail(UART7_BASE))
		{
			//
			// Read the next character from the UART
			//

			guchRxBuffer[guchUARTMapToGlobalBuffer[UART7]][guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART7]]]
			                                               = ROM_UARTCharGetNonBlocking(UART7_BASE);
			// Increment write index
			guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART7]]++;
			if(guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART7]] >= RECEIVE_BUFFER_SIZE)
			{
				// reset write index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART7]] = 0;
			}
			// increment received byte count
			guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART7]]++;
			if(guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART7]] >= RECEIVE_BUFFER_SIZE)
			{
				guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART7]] = 1;
				// reset received byte count
				guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART7]] = 0;
				// reset read index
				guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[UART7]] = 0;
				// reset write index
				guchRxBufferWrIndex[guchUARTMapToGlobalBuffer[UART7]] = 0;
			}

		}
	}
}


void clear_uart_buff_cyw(uint8_t lucUartNumber)
{
	guchTxBufferByteCount [guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
			guchTxBufferIndex [guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
			guchRxBufferByteCount [guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
			guchRxBufferRdIndex [guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
			guchRxBufferWrIndex [guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
			guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[lucUartNumber]] = 0;
}
