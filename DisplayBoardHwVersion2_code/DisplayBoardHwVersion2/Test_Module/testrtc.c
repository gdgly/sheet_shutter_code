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
#include "driverlib/hibernate.h"
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/rtc.h"



void testRTCGet()
{
	struct tm currentDateTime;
	unsigned char displayBuff[15];
	int retVal;

	bolyminDisplayInit();
	RTCEnable();
	retVal = RTCGet(&currentDateTime);
	usnprintf((char*)displayBuff, sizeof(displayBuff), "return : %u",retVal);
	displayText(displayBuff,0,0,false,false,false,false,false,false);
	memset(displayBuff, 0, sizeof(displayBuff));
	usnprintf((char*)displayBuff, sizeof(displayBuff), "%2u/%2u/%4u",currentDateTime.tm_mday,
			currentDateTime.tm_mon+1, currentDateTime.tm_year + 1900);
	displayText(displayBuff,0,16,false,false,false,false,false,false);
	memset(displayBuff, 0, sizeof(displayBuff));
	usnprintf((char*)displayBuff, sizeof(displayBuff), "%2u:%2u:%2u",currentDateTime.tm_hour,
			currentDateTime.tm_min, currentDateTime.tm_sec);
	displayText(displayBuff,0,33,false,false,false,false,false,false);
	//RTCDisable();
}

void testRTCSet()
{
	struct tm currentDateTime;
	currentDateTime.tm_mday = 8;
	currentDateTime.tm_mon = 7;
	currentDateTime.tm_year = 114;
	currentDateTime.tm_hour = 12;
	currentDateTime.tm_min = 47;
	currentDateTime.tm_sec = 3;
	RTCEnable();
	RTCSet(&currentDateTime);
}
