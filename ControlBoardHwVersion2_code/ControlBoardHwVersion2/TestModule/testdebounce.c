#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom_map.h>
#include <driverlib/rom.h>
#include <inc/hw_types.h>
#include <driverlib/uart.h>
#include "Middleware/debounce.h"
#include "Middleware/serial.h"

#ifdef UART_CONSOLE	// Use uart as console. Need to include uartstdio.c
void testDebounceSetup(void) {
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);

    UARTprintf("\n************* START ***************\n");
}
#endif	//UART_CONSOLE

void testKeyDebounceModule(void)
{
	//Open test
    if(gKeysStatus.bits.Key_Open_pressed) {
    	gKeysStatus.bits.Key_Open_pressed = 0;

    	uartSendTxBuffer(UART0,"OPEN PRESSED\n\r",14);
    }
    if(gKeysStatus.bits.Key_Open_released) {
    	gKeysStatus.bits.Key_Open_released = 0;
    	uartSendTxBuffer(UART0,"OPEN RELEASED\n\r",15);
    }

    //Stop test
    if(gKeysStatus.bits.Key_Stop_pressed) {
    	gKeysStatus.bits.Key_Stop_pressed = 0;
    	uartSendTxBuffer(UART0,"STOP PRESSED\n\r",14);
    }
    if(gKeysStatus.bits.Key_Stop_released) {
    	gKeysStatus.bits.Key_Stop_released = 0;
    	uartSendTxBuffer(UART0,"STOP RELEASED\n\r",15);
    }

    //Close test
    if(gKeysStatus.bits.Key_Close_pressed) {
    	gKeysStatus.bits.Key_Close_pressed = 0;
    	uartSendTxBuffer(UART0,"Close PRESSED\n\r",15);
    }
    if(gKeysStatus.bits.Key_Close_released) {
    	gKeysStatus.bits.Key_Close_released = 0;
    	uartSendTxBuffer(UART0,"Close RELEASED\n\r",16);
    }

    //Open + Stop + 3sec
    if(gKeysStatus.bits.Keys2_3secStpOpn_pressed) {
    	gKeysStatus.bits.Keys2_3secStpOpn_pressed = 0;
    	uartSendTxBuffer(UART0,"Open + Stop + 3sec\n\r",20);
    }

    //Close + Stop + 3sec
    if(gKeysStatus.bits.Keys2_3secStpCls_pressed) {
    	gKeysStatus.bits.Keys2_3secStpCls_pressed = 0;
    	uartSendTxBuffer(UART0,"Close + Stop + 3sec\n\r",21);
    }

    //Open + Stop + Close+ 3sec
    if(gKeysStatus.bits.Keys3_3secOpStCl_pressed) {
    	gKeysStatus.bits.Keys3_3secOpStCl_pressed = 0;
    	uartSendTxBuffer(UART0,"Open + Stop + Close + 3sec\n\r",28);
    }

    //Stop + 3sec
    if(gKeysStatus.bits.Key_3secCls_pressed) {
    	gKeysStatus.bits.Key_3secCls_pressed = 0;
    	uartSendTxBuffer(UART0,"Open + Stop + Close + 3sec\n\r",24);
    }
}
