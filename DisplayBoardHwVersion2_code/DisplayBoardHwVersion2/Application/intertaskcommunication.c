/*********************************************************************************
* FileName: gVariables.h
* Description:
* This source file contains the prototype definition of all the global variables.
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
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include "interTaskCommunication.h"
/****************************************************************************/


/****************************************************************************
 *  Macro definitions:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
****************************************************************************/
/*
	Drive board status and fault registers declaration
*/
_DriveBoardStatus			gstDriveBoardStatus;
_DriveBoardFault			gstDriveBoardFault;
_DriveCommunicationFault	gstDriveCommunicationFault;
_DriveMotorFault			gstDriveMotorFault;
_DriveApplicationFault		gstDriveApplicationFault;
_DriveProcessorFault		gstDriveProcessorfault;
_DriveInstallation			gstDriveInstallation;
//_DriveApertureheight        gstDriveApertureheight;

/*
	Control board status and fault registers declaration
*/
_ControlBoardStatus			gstControlBoardStatus;
_ControlBoardFault			gstControlBoardFault;
_ControlCommunicationFault	gstControlCommunicationFault;
_ControlApplicationFault	gstControlApplicationFault;
_ControlProcessorFault		gstControlProcessorFault;

/*
	Display board status and fault registers
*/
_DisplayBoardStatus			gstDisplayBoardStatus;
_DisplayBoardFault			gstDisplayBoardFault;
_DisplayCommunicationFault	gstDisplayCommunicationFault;
_DisplayBoardHwFault		gstDisplayBoardHwFault;
_DisplayApplicationFault	gstDisplayApplicationFault;
_DisplayProcessorFault		gstDisplayProcessorFault;

/*
	Inter-module communication
*/
_LEDcontrolRegister		gstLEDcontrolRegister;

_UMtoCMoperational		gstUMtoCMoperational;
_UMtoCMdatabase			gstUMtoCMdatabase;
_UMtoLM_write			gstUMtoLM_write;
_EMtoUM					gstEMtoUM;
_UMtoEM					gstUMtoEM;
_EMtoCM_errorList		gstEMtoCM_errorList;
_EMtoLM					gstEMtoLM;
_EMtoCM_monitorLED		gstEMtoCM_monitorLED;
_UMtoLM_read			gstUMtoLM_read;


/*
 * Communication error indicator for error module.
 * Added on 15 Sep 2014 to avoid communication error logging during power fail.
*/

uint8_t gucDItoCT_CommError;
uint8_t gucCTtoDR_CommError;

/****************************************************************************/

/****************************************************************************
 *  Function definitions:
 ****************************************************************************/
void initDispBoardGlobalRegisters(void)
{
	/*
		Drive board status and fault registers initialization
	*/
	gstDriveBoardStatus.val = 0;
	gstDriveBoardFault.val = 0;
	gstDriveCommunicationFault.val = 0;
	gstDriveMotorFault.val = 0;
	gstDriveApplicationFault.val = 0;
	gstDriveProcessorfault.val = 0;
	gstDriveInstallation.val = 0;

	/*
		Control board status and fault registers initialization
	*/
	gstControlBoardStatus.val = 0;
	gstControlBoardFault.val = 0;
	gstControlCommunicationFault.val = 0;
	gstControlApplicationFault.val = 0;
	gstControlProcessorFault.val = 0;

	/*
		Display board status and fault registers initialization
	*/
	gstDisplayBoardStatus.val = 0;
	gstDisplayBoardFault.val = 0;
	gstDisplayCommunicationFault.val = 0;
	gstDisplayBoardHwFault.val = 0;
	//gstDisplayBoardHwFault.bits.powerOnIndication = 1;		// //20170414      201703_No.31 	Added to log system power on event as requested by Bx on 21Apr2015
	gstDisplayApplicationFault.val = 0;
	gstDisplayProcessorFault.val = 0;

	/*
		Inter-module communication initialization
	*/
	gstLEDcontrolRegister.autoManualLED = 0;
	gstLEDcontrolRegister.faultLED = 0;
	gstLEDcontrolRegister.monitorLED = 0;
	gstLEDcontrolRegister.powerLED = 1;


	gstUMtoCMoperational.commandRequestStatus = eINACTIVE;
	gstUMtoCMoperational.commandToControlBoard.val = 0;
	gstUMtoCMoperational.commandResponseStatus = eNO_STATUS;
	gstUMtoCMoperational.acknowledgementReceived = eNO_ACK;

	gstUMtoCMdatabase.commandRequestStatus = eINACTIVE;
	gstUMtoCMdatabase.commandToControlBoard.val = 0;
	gstUMtoCMdatabase.commandResponseStatus = eNO_STATUS;
	gstUMtoCMdatabase.acknowledgementReceived = eNO_ACK;

	gstUMtoLM_write.commandRequestStatus = eINACTIVE;
	gstUMtoLM_write.commandToLMwrite.val = 0;
	gstUMtoLM_write.commandResponseStatus = eNO_STATUS;
	gstUMtoLM_write.changeSetting_index = 0;

	gstEMtoUM.faultLEDstatus = 0;

//	gstUMtoEM.commandRequestStatus = eINACTIVE;
	gstUMtoEM.commandToEM.val = 0;
//	gstUMtoEM.commandResponseStatus = eNO_STATUS;

	gstEMtoCM_errorList.commandRequestStatus = eINACTIVE;
	gstEMtoCM_errorList.commandToCM.val = 0;
	gstEMtoCM_errorList.commandResponseStatus = eNO_STATUS;

	gstEMtoLM.commandRequestStatus = eINACTIVE;
	gstEMtoLM.anomaly_index = 0;
	gstEMtoLM.errorToLM.anomalyLED_blinkRate = 0;
	gstEMtoLM.errorToLM.monitorLED_blinkRate = 0;
	gstEMtoLM.commandResponseStatus = eNO_STATUS;
	gstEMtoLM.commandResponseACK = eNO_StatusAcknowledgement;

	gstEMtoCM_monitorLED.commandRequestStatus = eINACTIVE;
	gstEMtoCM_monitorLED.commandToControlBoard.val = 0;
	gstEMtoCM_monitorLED.commandResponseStatus = eNO_STATUS;
	gstEMtoCM_monitorLED.acknowledgementReceived = eNO_ACK;

	gstUMtoLM_read.commandRequestStatus = eINACTIVE;
	gstUMtoLM_read.historyOrAnomalyIndex = 0;
	gstUMtoLM_read.commandToLMread.val = 0;
	gstUMtoLM_read.commandResponseStatus = eNO_STATUS;
	gstUMtoLM_read.acknowledgementReceived = eNO_ACK;

	gucDItoCT_CommError = 0;
	gucCTtoDR_CommError = 0;
}
