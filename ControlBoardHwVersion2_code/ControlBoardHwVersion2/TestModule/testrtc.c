/*
 * testrtc.c
 *
 *  Created on: Apr 1, 2014
 *      Author: rk803609
 */

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <driverlib/hibernate.h>
#include "Drivers/ustdlib.h"
#include "Middleware/rtc.h"



void testRTCGet()
{
	struct tm currentDateTime;

	RTCEnable();
	RTCGet(&currentDateTime);
	//RTCDisable();
}

void testRTCSet()
{
	struct tm currentDateTime;
	currentDateTime.tm_mday = 3;
	currentDateTime.tm_mon = 6;
	currentDateTime.tm_year = 114;
	currentDateTime.tm_hour = 14;
	currentDateTime.tm_min = 24;
	currentDateTime.tm_sec = 20;
	RTCEnable();
	RTCSet(&currentDateTime);
}
