#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <sysctl.h>
#include <gpio.h>
#include <pin_map.h>
#include <rom_map.h>
#include <rom.h>
#include <inc/hw_types.h>
#include <uart.h>
#include "Middleware/uartstdio.h"
#include "Middleware/debounce.h"
#include "Drivers/watchdogtimer.h"


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
}

void testKeyDebounceModule(void)
{

	//Open test
    if(gKeysStatus.bits.Key_Open_pressed) {
    	gKeysStatus.bits.Key_Open_pressed = 0;
    	UARTprintf("OPEN PRESSED\n");
    }
    if(gKeysStatus.bits.Key_Open_released) {
    	gKeysStatus.bits.Key_Open_released = 0;
    	UARTprintf("OPEN RELEASED\n");
//    	doWatchdogReset();
    }

    //Stop test
    if(gKeysStatus.bits.Key_Stop_pressed) {
    	gKeysStatus.bits.Key_Stop_pressed = 0;
    	UARTprintf("STOP PRESSED\n");
    }
    if(gKeysStatus.bits.Key_Stop_released) {
    	gKeysStatus.bits.Key_Stop_released = 0;
    	UARTprintf("STOP RELEASED\n");
    }

    //Close test
    if(gKeysStatus.bits.Key_Close_pressed) {
    	gKeysStatus.bits.Key_Close_pressed = 0;
    	UARTprintf("Close PRESSED\n");
    }
    if(gKeysStatus.bits.Key_Close_released) {
    	gKeysStatus.bits.Key_Close_released = 0;
    	UARTprintf("Close RELEASED\n");
    }

    //AutMan test
    if(gKeysStatus.bits.Key_AutMan_pressed) {
    	gKeysStatus.bits.Key_AutMan_pressed = 0;
    	UARTprintf("AutMan PRESSED\n");
    }
    if(gKeysStatus.bits.Key_AutMan_released) {
    	gKeysStatus.bits.Key_AutMan_released = 0;
    	UARTprintf("AutMan RELEASED\n");
    }

    //Up test
    if(gKeysStatus.bits.Key_Up_pressed) {
    	gKeysStatus.bits.Key_Up_pressed = 0;
    	UARTprintf("UP PRESSED\n");
    }
    if(gKeysStatus.bits.Key_Up_released) {
    	gKeysStatus.bits.Key_Up_released = 0;
    	UARTprintf("UP RELEASED\n");
    }

    //Down test
    if(gKeysStatus.bits.Key_Down_pressed) {
    	gKeysStatus.bits.Key_Down_pressed = 0;
    	UARTprintf("Down PRESSED\n");
    }
    if(gKeysStatus.bits.Key_Down_released) {
    	gKeysStatus.bits.Key_Down_released = 0;
    	UARTprintf("Down RELEASED\n");
    }

    //Mode test
    if(gKeysStatus.bits.Key_Mode_pressed) {
    	gKeysStatus.bits.Key_Mode_pressed = 0;
    	UARTprintf("Mode PRESSED\n");
    }
    if(gKeysStatus.bits.Key_Mode_released) {
    	gKeysStatus.bits.Key_Mode_released = 0;
    	UARTprintf("Mode RELEASED\n");
    }

    //Enter test
    if(gKeysStatus.bits.Key_Enter_pressed) {
    	gKeysStatus.bits.Key_Enter_pressed = 0;
    	UARTprintf("Enter PRESSED\n");
    }
    if(gKeysStatus.bits.Key_Enter_released) {
    	gKeysStatus.bits.Key_Enter_released = 0;
    	UARTprintf("Enter RELEASED\n");
    }

    //Open + Stop + 3sec
    if(gKeysStatus.bits.Keys2_3secStpOpn_pressed) {
    	gKeysStatus.bits.Keys2_3secStpOpn_pressed = 0;
    	UARTprintf("Open + Stop + 3sec\n");
    }

    //Close + Stop + 3sec
    if(gKeysStatus.bits.Keys2_3secStpCls_pressed) {
    	gKeysStatus.bits.Keys2_3secStpCls_pressed = 0;
    	UARTprintf("Close + Stop + 3sec\n");
    }

    //Open + Stop + Close+ 3sec
    if(gKeysStatus.bits.Keys3_3secOpStCl_pressed) {
    	gKeysStatus.bits.Keys3_3secOpStCl_pressed = 0;
    	UARTprintf("Open + Stop + Close + 3sec\n");
    }

    //Close + 3sec
    if(gKeysStatus.bits.Key_3secCls_pressed) {
    	gKeysStatus.bits.Key_3secCls_pressed = 0;
    	UARTprintf("Close + 3sec\n");
    }

    //Enter + 3sec
    if(gKeysStatus.bits.Key_3secEnter_pressed) {
    	gKeysStatus.bits.Key_3secEnter_pressed = 0;
    	UARTprintf("Enter + 3sec\n");
    }

}
