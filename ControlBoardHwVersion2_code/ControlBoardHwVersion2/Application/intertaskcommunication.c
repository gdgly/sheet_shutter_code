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
#include "monitorledhandler.h"

/****************************************************************************/


/****************************************************************************
 *  Macro definitions:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
****************************************************************************/

/*
	Monitor LED control register declaration
*/
_MonitorLED gstMonitorLEDControlRegister;

_ElectroMechanicalCounter gstElectroMechanicalCounter;

/*
	Drive board status and fault registers declaration
*/
_DriveStatus				gstDriveStatus;
_DriveFault					gstDriveBoardFault;
_DriveCommunicationFault	gstDriveCommunicationFault;
_DriveMotorFault			gstDriveMotorFault;
_DriveApplicationFault		gstDriveApplicationFault;
_DriveProcessorFault		gstDriveProcessorfault;
_DriveInstallation			gstDriveInstallation;

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
_CMDitoCMDr		gstCMDitoCMDr;
_EMtoDH			gstEMtoDH;
_CMDrtoLS		gstCMDrtoLS;
_LStoCMDr		gstLStoCMDr;
_EMtoCMDr		gstEMtoCMDr;
_CMDitoLS		gstCMDitoLS;
_CMDitoDH		gstCMDitoDH;
_CMDitoMLH		gstCMDitoMLH;


/*
	Drive Status Menu
*/
_DriveStatusMenu	gstDriveStatusMenu;

// Following flag is used to change system state from run to stop.
// CMDi will set this flag  when setParameter command is received from display.
// Logic solver will change the system state from run to stop and it
// won't send any operation command to drive when this flag is set.

uint8_t gucSetParameterCommandFlag;

//
//	Added on 17 Nov 2014 to implement drive board firmware upgrade functionality.
//
//	Following flag is used to indicate that drive firmware upgrade was initiated by
//	display board and drive board has responded to the request with ACK.

uint8_t gui8DriveFirwareUpgradeInitiated;

//	Added this global variable to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
uint8_t guiSettingsModeStatus;
/****************************************************************************/

/****************************************************************************
 *  Function definitions:
 ****************************************************************************/
void initControlBoardGlobalRegisters(void)
{
	/*
		Drive board status and fault registers initialization
	*/
	gstDriveStatus.val = 0;
	gstDriveStatus.bits.driveReady = 0;
	gstDriveStatus.bits.shutterStopped = 1;
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
		Monitor LED control register initialization
	*/
	gstMonitorLEDControlRegister.monitorLEDstatus = LED_ON;

	gstElectroMechanicalCounter.value = 0;

	/*
		Inter-module communication initialization
	*/
	gstCMDitoCMDr.commandRequestStatus = eINACTIVE;
	gstCMDitoCMDr.receiveCommandPacketLen = 0;
	gstCMDitoCMDr.transmitCommandPacketLen = 0;

	gstEMtoDH.commandRequestStatus = eINACTIVE;
	gstEMtoDH.controlAnomalyIndex = 0;
	gstEMtoDH.commandResponseStatus = eNO_STATUS;

	gstCMDrtoLS.commandRequestStatus = eINACTIVE;
	gstCMDrtoLS.commandFromDriveBoard.val = 0;
	gstCMDrtoLS.commandResponseStatus = eNO_STATUS;
	gstCMDrtoLS.acknowledgementReceived = eNO_ACK;

	gstLStoCMDr.commandRequestStatus = eINACTIVE;
	gstLStoCMDr.commandToDriveBoard.val = 0;
	gstLStoCMDr.commandResponseStatus = eNO_STATUS;
	gstLStoCMDr.acknowledgementReceived = eNO_ACK;

	gstEMtoCMDr.commandRequestStatus = eINACTIVE;
	gstEMtoCMDr.commandToDriveBoard.val = 0;
	gstEMtoCMDr.commandResponseStatus = eNO_STATUS;
	gstEMtoCMDr.commandResponseACK_Status = eNO_StatusAcknowledgement;
//	gstEMtoCMDr.commandResponseACK = eNO_ACK;

	gstCMDitoLS.commandRequestStatus = eINACTIVE;
	gstCMDitoLS.commandDisplayBoardLS.val = 0;
	gstCMDitoLS.commandResponseStatus = eNO_STATUS;
	gstCMDitoLS.acknowledgementReceived = eNO_ACK;
	//	Added this initialization while implementing "disable shutter functionality while we are in settings mode" -RN - Dec 2015
	gstCMDitoLS.additionalCommandData = 0;

	gstCMDitoDH.commandRequestStatus = eINACTIVE;
	gstCMDitoDH.commandDisplayBoardDH.val = 0;
	gstCMDitoDH.commandResponseStatus = eNO_STATUS;
	gstCMDitoDH.acknowledgementReceived = eNO_ACK;
	gstCMDitoDH.commandResponseACK_Status = eNO_StatusAcknowledgement;
//	gstCMDitoDH.commandResponseACK = eNO_ACK;

	gstCMDitoMLH.commandRequestStatus = eINACTIVE;
	gstCMDitoMLH.commandDisplayBoardMLH.val = 0;
	gstCMDitoMLH.commandResponseStatus = eNO_STATUS;
	gstCMDitoMLH.acknowledgmentReceived = eNO_ACK;

	// Clear setParameter command flag
	gucSetParameterCommandFlag = 0;

	// Clear 'drive firmware upgrade initiated' indication flag
	gui8DriveFirwareUpgradeInitiated = 0;

	//	Added this variable to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
	guiSettingsModeStatus = 0;
}

// *********************************************************************************************
// Variable for sub-state 'Handle Counter' of 'Logic_Solver_Drive_Run'
// *********************************************************************************************
enum HandleCounter geHandleCounter = HandleCounterInitGetCounter;

uint32_t gulCounterValue;

// *********************************************************************************************


// *********************************************************************************************
// Variable to indicate "System Init Complete" command received from Display board
// *********************************************************************************************

uint8_t gucSystemInitComplete = 0;
// 0 = System init command not received
// 1 = System init commnad received by CMDI, but not sent reply
// 2 = System init commnad received by CMDI, and sent reply

// *********************************************************************************************
