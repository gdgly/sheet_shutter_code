/*
 * testhomescreen.c
 *
 *  Created on: Apr 17, 2014
 *      Author: rk803609
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "Application/ustdlib.h"
#include "Application/userinterface.h"
#include "Application/intertaskcommunication.h"
#include "Middleware/display.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/paramdatabase.h"

uint32_t opCount = 1234567;
uint32_t shutterType = 1;
uint32_t controlHWVersion = 1;
uint32_t controlFWVersion = 10;
uint32_t driveHWVersion = 2;
uint32_t driveFWVersion = 20;
uint32_t shutterStates = 0x00044444;

int flag = 0;

void communicationModule()
{
	gstDriveBoardStatus.bits.driveReady = 1;

	if((psActiveFunctionalBlock == &gsPowerOnFunctionalBlock) ||
			(psActiveFunctionalBlock == &gsHomeScreenFunctionalBlock))
	{
		if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		{
			if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == A600_OPERATION_COUNT)
			{
				memcpy(&gstUMtoCMdatabase.getParameterValue, &opCount, sizeof(uint32_t));
				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
			}

			else if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == A537_shutter_type)
			{
				memcpy(&gstUMtoCMdatabase.getParameterValue, &shutterType, sizeof(uint32_t));
				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
			}

			else if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == 451)
			{
				memcpy(&gstUMtoCMdatabase.getParameterValue, &controlHWVersion, sizeof(uint32_t));
				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
			}

			else if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == 450)
			{
				memcpy(&gstUMtoCMdatabase.getParameterValue, &controlFWVersion, sizeof(uint32_t));
				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
			}

			else if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == 550)
			{
				memcpy(&gstUMtoCMdatabase.getParameterValue, &driveHWVersion, sizeof(uint32_t));
				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
			}

			else if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == 549)
			{
				memcpy(&gstUMtoCMdatabase.getParameterValue, &driveFWVersion, sizeof(uint32_t));
				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
			}

			else
				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
		}

		if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
		{
			if(gstUMtoCMoperational.commandToControlBoard.bits.startInstallation == 1)
			{
				gstDriveBoardStatus.bits.driveInstallation = 1;
				gstUMtoCMoperational.commandResponseStatus = eSUCCESS;

			}
		}

	//	if(flag == 0)
	//	{
	//		gstDriveStatus.bits.driveRunTimeCalibration = 1;
	//		flag = 1;
	//	}
	//	else if(flag == 1)
	//	{
	//		gstDriveStatus.bits.driveRunTimeCalibration = 0;
	//	}
	}

	if(psActiveFunctionalBlock == &gsShutterRunFunctionalBlock)
	{
//		if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
//		{
//			if(gstUMtoCMoperational.commandToControlBoard.bits.runControlBoard == 1)
//			{
//				gstUMtoCMoperational.commandResponseStatus = eSUCCESS;
//
//			}
//		}

		gstControlBoardStatus.bits.runStop = 1;
	}

	if(psActiveFunctionalBlock == &gsShutterStopFunctionalBlock)
	{
		if(gstUMtoCMoperational.commandRequestStatus == eACTIVE)
		{
			if(gstUMtoCMoperational.commandToControlBoard.bits.stopControlBoard == 1)
			{
				gstUMtoCMoperational.commandResponseStatus = eSUCCESS;

			}
		}

		//gstControlBoardStatus.bits.runStop = 0;
	}

	if(psActiveFunctionalBlock == &gsShutterStatusFunctionalBlock)
	{
		if(gstUMtoCMdatabase.dataToControlBoard.parameterNumber == 200)
		{
			memcpy(&gstUMtoCMdatabase.getParameterValue, &shutterStates, sizeof(uint32_t));
			gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
			gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
		}
	}

	//Test
	if((psActiveFunctionalBlock == &gsDownloadParametersFunctionalBlock))
	{
		if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		{
			if(gstUMtoCMdatabase.commandToControlBoard.bits.getParameter)
			{
				gstUMtoCMdatabase.getParameterValue = 45;
				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
//				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
				gstUMtoCMdatabase.acknowledgementReceived = eACK;
			}
			else if(gstUMtoCMdatabase.commandToControlBoard.bits.setParameter)
			{
				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
			}
		}
	}

	//Test
	if((psActiveFunctionalBlock == &gsSDUpParamCTRLFunctionalBlock) || (psActiveFunctionalBlock == &gsSDUpParamDRVFunctionalBlock))
	{
		if(gstUMtoCMdatabase.commandToControlBoard.bits.setParameter)
		{
			gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
		}

	}
}
