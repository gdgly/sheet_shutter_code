/*
 * testledhandler.c
 *
 *  Created on: Aug 7, 2014
 *      Author: rk803609
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/gpio.h>
#include <inc/hw_memmap.h>
#include "Drivers/systicktimer.h"
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "Middleware/paramdatabase.h"
#include "Application/userinterface.h"
#include "Application/intertaskcommunication.h"
#include "Application/ledhandler.h"

void testLEDHandler()
{
	gstLEDcontrolRegister.autoManualLED = 0x42;
	gstLEDcontrolRegister.faultLED = 0x32;
	//gunLEDcontrolRegister.powerLED = 0xA3;

	//gstDriveBoardStatus.bits.driveInstallation = 0;


}
