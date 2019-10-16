/*
 * testhomescreen.c
 *
 *  Created on: Apr 17, 2014
 *      Author: rk803609
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <driverlib/gpio.h>
#include "Application/ustdlib.h"
#include "Application/userinterface.h"
#include "Application/intertaskcommunication.h"
#include "Middleware/display.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"

//extern uint32_t g_ui32TickCount;
uint32_t currentCount = 1234;
extern stAnomalyError gsActiveAnomalyList[10];

void installationTestFunction()
{
//	static uint8_t localFlag = 0;
//
//	if(localFlag == 0)
//	{
//		// for sending installation command
//		//gstDriveStatus.bits.driveInstallation = 1;
//
//		// for installation status
//		//gstDriveInstallation.bits.installA101 = 1;
//
//		gstDriveInstallation.bits.installationFailed = 1;
//		gstDriveStatus.bits.driveFault = 1;
//
//		localFlag = 1;
//	}

	if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
	{
		if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == 129)
		{
			memcpy(&gstUMtoCMdatabase.getParameterValue, &currentCount, sizeof(uint32_t));
			gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
			gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
		}
	}

	if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
	{
		if(gstUMtoCMoperational.commandToControlBoard.bits.startInstallation == 1)
			gstDriveBoardStatus.bits.driveInstallation = 1;

		gstUMtoCMoperational.commandResponseStatus = eSUCCESS;
	}

}


void installationRuntimeTestFunction()
{
	static uint32_t lsDelay3secTest = 0;
	static uint8_t lsTestDelay3secStart = 0;

	if(lsTestDelay3secStart == 0)
	{
		gstDriveInstallation.bits.installA100 = 1;

		//
		// Capture time
		//
		lsDelay3secTest = g_ui32TickCount;

		//
		// Start delay
		//
		lsTestDelay3secStart = 1;
	}

	//
	// Check whether delay is achieved
	//
	if( ((g_ui32TickCount - lsDelay3secTest) > 300) &&
		(lsTestDelay3secStart == 1)
	  )
	{
		//
		// Reset delay start flag
		//
		lsTestDelay3secStart = 2;
	}

	if( (lsTestDelay3secStart == 2) &&
		(gstDriveInstallation.bits.installationValid == 1)
	  )
	{
		gsActiveAnomalyList[0].anomalyCode = 501;
		memcpy(gsActiveAnomalyList[0].description, "NEW ERROR 1", 11);

		gsActiveAnomalyList[1].anomalyCode = 502;
		memcpy(gsActiveAnomalyList[1].description, "NEW ERROR 2", 11);

		gsActiveAnomalyList[2].anomalyCode = 503;
		memcpy(gsActiveAnomalyList[2].description, "NEW ERROR 3", 11);

		//
		// Capture time
		//
		lsDelay3secTest = g_ui32TickCount;

		//
		// Start delay
		//
		lsTestDelay3secStart = 3;
	}

	//
	// Check whether delay is achieved
	//
	if( ((g_ui32TickCount - lsDelay3secTest) > 300) &&
		(lsTestDelay3secStart == 3)
	  )
	{
		//
		// Reset delay start flag
		//
		lsTestDelay3secStart = 4;
	}

	if(lsTestDelay3secStart == 4)
	{
		gstDriveInstallation.bits.installationFailed = 1;
		gstDriveBoardStatus.bits.driveFault = 1;
		gstDriveInstallation.bits.installationValid = 0;

		//
		// Capture time
		//
		lsDelay3secTest = g_ui32TickCount;

		//
		// Start delay
		//
		lsTestDelay3secStart = 5;
	}

	//
	// Check whether delay is achieved
	//
	if( ((g_ui32TickCount - lsDelay3secTest) > 1500) &&
		(lsTestDelay3secStart == 5)
	  )
	{
		//
		// Reset delay start flag
		//
		lsTestDelay3secStart = 6;
	}

	if(lsTestDelay3secStart == 6)
	{
		gstDriveBoardStatus.bits.driveFault = 0;
		gstDriveInstallation.bits.installationValid = 1;
		gstDriveInstallation.bits.installationFailed = 0;

		//
		// Capture time
		//
		lsDelay3secTest = g_ui32TickCount;

		//
		// Start delay
		//
		lsTestDelay3secStart = 7;
	}

	//
	// Check whether delay is achieved
	//
	if( ((g_ui32TickCount - lsDelay3secTest) > 300) &&
		(lsTestDelay3secStart == 7)
	  )
	{
		//
		// Reset delay start flag
		//
		lsTestDelay3secStart = 8;
	}

	if(lsTestDelay3secStart == 8)
	{
		gstDriveInstallation.bits.installationSuccess = 1;
		gstDriveInstallation.bits.installationValid = 0;

		//
		// Capture time
		//
		lsDelay3secTest = g_ui32TickCount;

		//
		// Start delay
		//
		lsTestDelay3secStart = 9;
	}

	//
	// Check whether delay is achieved
	//
	if( ((g_ui32TickCount - lsDelay3secTest) > 300) &&
		(lsTestDelay3secStart == 9)
	  )
	{
		//
		// Reset delay start flag
		//
		lsTestDelay3secStart = 10;

	}

}
