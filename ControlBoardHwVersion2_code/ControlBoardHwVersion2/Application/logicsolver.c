/*********************************************************************************
 * FileName: logicsolver.c
 * Description:
 * This source file contains the definition of all the services of ...
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
 *  	0.2D	10/07/2014			iGATE Offshore team			Added logic for auto manual and run stop select
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       	Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Include:
 ****************************************************************************/
//#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "logicsolver.h"
#include "intertaskcommunication.h"
#include "cmdr.h"
#include "cmdi.h"
#include "Middleware/debounce.h"
#include "Middleware/sensorsdebounce.h"
#include "Middleware/paramdatabase.h"
#include "Drivers/extern.h"
#include "errormodule.h"

//#include "Middleware/serial.h"
//#include "Middleware/rtc.h"
//#include "Drivers/ustdlib.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/
#define INTERLOCK_DELAY_TIMER_FOR_CLOSE_OPEN		1
// 0 == Interlock delay timer enabled for only relay close operation
// 1 == Interlock delay timer enabled for close and relay open operation


#define ENTER_CMD_ONLY_ONCE_PER_INSTALLATION_STATE	0        //20160914   bug_No。99
// 0 = Enter command can go multiple times per installation state
// 1 = Logic added to make sure Enter command to go single times per installation state

/****************************************************************************/


/****************************************************************************
 *  Type definitions:
 ****************************************************************************/
typedef union unMultifunctionOutput
{
	uint16_t val;
	struct stMultifunctionOutput
	{
		uint16_t UpperLimit					: 1;
		uint16_t LowerLimit					: 1;
		uint16_t InterlockOutput			: 1;
		uint16_t Operating					: 1;
		uint16_t Rising						: 1;
		uint16_t Dropping					: 1;
		uint16_t GreenSignalLamp 			: 1;
		uint16_t RedSignalLamp				: 1;
		uint16_t AutomaticModeOutput		: 1;
		uint16_t ManaualModeOutput			: 1;
		uint16_t ErrorOutput				: 1;
		uint16_t unused						: 5;
	} bits;
} _MultifunctionOutput;

#define relay_delay_cyw 6000
uint8_t runing_a009_dir_cyw=0;

uint8_t OpenCmdForDistinguish = 0;//20160808 AOYAGI STT ; 1-open cmd、0-other cmd
uint8_t select_apertureHeight_Instalation=0;
/****************************************************************************/

//20160906 item104
uint8_t season_cyw=0;
static unsigned int time_ObstacleSensor;   //add 20161020
/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/


/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/
//static _MultifunctionOutput	gstBitwiseMultifuncOutputLastState;

/****************************************************************************/

/****************************************************************************
 *  Function prototypes for this file:
 ****************************************************************************/
/******************************************************************************
 * ValidateInterlockInput
 *
 * Function Description:
 * Function will validate the Interlock Output from other shutter and
 * will decide whether it is allowed to operate the shutter or not
 * Function Parameter: void
 *
 * Function Returns:
 * 0 = do not operate shutter
 * 1 = operate shutter
 ********************************************************************************/
unsigned char ValidateInterlockInput(void);
extern uint8_t flag_addlogin;
/****************************************************************************/
bool UpEnabled;
bool OnePBSEnabled = false;
bool OnePBSEnableStatus = false;
extern unsigned int suiTimeStampForOnePBS;
/****************************************************************************
 *  Enum for this file:
 ****************************************************************************/
enum Logic_Solver_State {

	Logic_Solver_Init = 0,
	Logic_Solver_Init_Delay,
	Logic_Solver_Power_ON_Init,
	Logic_Solver_Drive_Instalation,
	Logic_Solver_Drive_apertureHeight,
	Logic_Solver_Drive_Run

};

enum Logic_Solver_State eLogic_Solver_State = Logic_Solver_Init;
extern uint8_t  Clear_E111_FLAG ;
uint32_t get_timego(uint32_t x_data_his);

/****************************************************************************/

/******************************************************************************
 * logicSolver
 *
 * Function Description:
 * This function handles the Core Logic of the Shutter
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void logicSolver(void) {
	// ***********************************************************************************************************
	// ***********************************************************************************************************
	// Variable for 'Logic_Solver_Drive_Run'
	// ***********************************************************************************************************
	// ***********************************************************************************************************

	// *********************************************************************************************
	// Variable for sub-state 'Handle Keys and Startup' of 'Logic_Solver_Drive_Run'
	// *********************************************************************************************

	UpEnabled = 0;
	// Following variable is used to indicate the command sent through specific sub-state of Logic_Solver_Drive_Run
	enum HandleKeysStartupState {

		CmdNotSent = 0, CmdSentWaitingForReply,
	};
	static enum HandleKeysStartupState seHandleKeysStartupState = CmdNotSent;

	// Following variable hold the state of any as press or released
	// Actual key event will get reseted after handling
	static unsigned char sucOpenKeyDisplay = 0;
	static unsigned char sucOpenKeyControl = 0;
	static unsigned char sucWirelessOpenKeyControl = 0;

	static unsigned char sucCloseKeyDisplay = 0;
	static unsigned char sucCloseKeyControl = 0;
	static unsigned char sucWirelessCloseKeyControl = 0;

	static unsigned char sucStopKeyDisplay = 0;
	static unsigned char sucStopKeyControl = 0;
	static unsigned char sucWirelessStopKeyControl = 0;

	static unsigned char suc1PBSControl = 0;
	static unsigned char sucWireless1PBSControl = 0;
	static unsigned char sucStartupControl = 0;

	// Following variable hold the state of the Open / Close command sending activity
	// The variable mainly used in situation like "Open Shutter, Open Shutter Aperture like command"
	// which physically need to sent to drive board after go up operation delay
	enum ShutterOpenCloseCmdState {

		CmdNotDetected = 0,
		CmdUpDetectedWaitUpDelay,
		CmdDownDetectedWaitDownDelay

	};
	static enum ShutterOpenCloseCmdState seShutterOpenCloseCmdState =
			CmdNotDetected;

	// Following variable hold the command to be sent
	// The variable mainly used in situation like "Open Shutter, Open Shutter Aperture like command"
	// which physically need to sent to drive board after go up operation delay
	static _LStoCMDr sstLStoCMDrCmdToBeSent;

	// Following variable hold the last command sent
	// The variable used to avoid sending of the same command one after another
	static _LStoCMDr sstLStoCMDrCmdSent;

	// It is used to generate delay
	static unsigned int suiTimeStamp;
	// *********************************************************************************************

	// *********************************************************************************************
	// Variable for sub-state 'Handle Upper Limit Stoppage Time' of 'Logic_Solver_Drive_Run'
	// *********************************************************************************************
	enum HandleUpperLimitStopTimeState {

		HandleUpperLimitStopTimeInit = 0,
		UpperLimitStopTimeStarted,
		UpperLimitStopTimeExpiredDnCmdSentWaitingReply,
		WaitingShutterMoveUpperPos

	};

	static enum HandleUpperLimitStopTimeState seHandleUpperLimitStopTimeState =
			HandleUpperLimitStopTimeInit;

	// *********************************************************************************************

	// *********************************************************************************************

	// *********************************************************************************************
	// Variable for sub-state 'Handle Auto/Manual Run/Stop' of 'Logic_Solver_Drive_Run'
	// *********************************************************************************************
	enum HandleAutoManualRunStop {

		HandleAutoManualRunStopInit = 0,
		HandleAutoManualRunStopCmdSentToDH_WaitingReply

	};
	static enum HandleAutoManualRunStop seHandleAutoManualRunStop =
			HandleAutoManualRunStopInit;

	// *********************************************************************************************

	// *********************************************************************************************
	// Variable for sub-state 'Handle Safety Signal' of 'Logic_Solver_Drive_Run'
	// *********************************************************************************************
	enum HandleSafetySignal {

		HandleSafetySignalInit = 0,
		HandleSafetySignalWaitingReply,	// Safety signal detected and open shutter jog command sent and waiting for reply
		HandleSafetySignalWaitingShutterReachUpperLimit,
		HandleSafetySignalWaitingSafetySigToReset

	};
	static enum HandleSafetySignal seHandleSafetySignal = HandleSafetySignalInit;


	// *********************************************************************************************
	// Variable for sub-state 'Handle Interlock Input' of 'Logic_Solver_Drive_Run'
	// *********************************************************************************************
	enum HandleInterlockInput {

		HandleInterlockInputInit = 0,
		HandleInterlockWaitingReply,
		// Interlock input signal de-activated under following condition
		// i)  Interlock input is Valid
		// ii) Non priority set
		// iii) shutter is moving
		// and open shutter stop command sent and waiting for reply
		HandleInterlockWaitingShutterToStop,
		HandleInterlockWaitingInterlockInputToReset

	};
	static enum HandleInterlockInput seHandleInterlockInput = HandleInterlockInputInit;

	// *********************************************************************************************

	// *********************************************************************************************
	// Variable for sub-state 'Handle Installation' of 'Logic_Solver_Drive_Run'
	// *********************************************************************************************
	enum HandleInstallation {

		HandleInstallationInit = 0,
		HandleInstallationWaitingReply,
		HandleInstallationWaitingShutterStateInstallation,
		HandleInstallationWaitDbHandlerToFreeSendStopShutterCmdToDbHandler,
		HandleInstallationSentStopShutterCmdToDbHandlerWaitReply,

	};
	static enum HandleInstallation seHandleInstallation = HandleInstallationInit;

#if 0
	// *********************************************************************************************
	// Variable for sub-state 'Handle Counter' of 'Logic_Solver_Drive_Run'
	// *********************************************************************************************
	enum HandleCounter {

		HandleCounterInitGetCounter = 0,
		HandleCounterGetCounterWaitingReply,
		HandleCounterWaitLowerLimitGetCounter,
		HandleCounterLowerLimitGetCounterWaitingReply,
		HandleCounterWaitNonLowerLimit

	};
	static enum HandleCounter geHandleCounter = HandleCounterInitGetCounter;

	static uint32_t gulCounterValue;

	// *********************************************************************************************
#endif

	// *********************************************************************************************
	// Variable for sub-state 'Interlock Output' of 'Logic_Solver_Drive_Run'
	// *********************************************************************************************
	enum HandleInterlockOutput {

		HandleInterlockOP_WaitLowerLimit = 0,
		HandleInterlockOP_DelayBeforeCloseRelay,
		HandleInterlockOP_CloseRelay,
		HandleInterlockOP_WaitNonLowerLimit,
		HandleInterlockOP_DelayBeforeOpenRelay,
		HandleInterlockOP_OpenRelay,
	};
	static enum HandleInterlockOutput sHandleInterlockOutput =
			HandleInterlockOP_WaitLowerLimit;

	static uint32_t suiTimeStampInterlockOutput;

	static unsigned char sucInterlockOutputStatus = 0;
	// 0 = Interlock output replay OFF
	// 1 = Interlock output replay ON

	static unsigned char sucLastOpenCommandType = 0; // 0 == Open command,1 == Open Aperture height command

	// *********************************************************************************************

	// *********************************************************************************************
	// Variable for sub-state 'Monitor continuous Operation' of 'Logic_Solver_Drive_Run'
	// *********************************************************************************************
	enum HandleContinuousOperation {

		HandleContinuousOpr_WaitNonStoppedStateStrtDelay = 0,
		HandleContinuousOpr_MonitorStoppedStateForDelay,
		HandleContinuousOpr_ConfirmNonStoppedState,
		HandleContinuousOpr_SendStopShutterCmd,
		HandleContinuousOpr_SendStopShutterReply,
		HandleContinuousOpr_WaitPowerReset,
	};
	static enum HandleContinuousOperation sHandleContinuousOperation = HandleContinuousOpr_WaitNonStoppedStateStrtDelay;

	static uint32_t suiTimeStampContinuousOperation;
	static uint32_t suiTimeStampConfirmNonStoppedState;

	// *********************************************************************************************


	// *********************************************************************************************
	// Variable for sub-state 'Power ON INit' of 'Logic_Solver_Drive_Run'
	// *********************************************************************************************
	enum HandlePowerON_Init {

		HandlePowerOnInit_Home = 0,
		HandlePowerOnInit_InstallationCmdSentToDrive,
		HandlePowerOnInit_StopInstrucSentToDH,
	};
	static enum HandlePowerON_Init sHandlePowerON_Init = HandlePowerOnInit_Home;



	// *********************************************************************************************


	// *********************************************************************************************
	// Variable for sub-state 'update snow mode setting of drive board' of 'Logic_Solver_Drive_Run'
	// *********************************************************************************************

	enum HandleupdateSnowModeDrive {

		HandleSnowDrive_CheckForUpdate = 0,
		HandleSnowDrive_SendSnowModeCmd,
		HandleSnowDrive_WaitSnowModeReply,
	};
	static enum HandleupdateSnowModeDrive sHandleupdateSnowModeDrive = HandleSnowDrive_CheckForUpdate;

	static uint8_t su8_drive_snow_mode = 0xFF;

	// *********************************************************************************************


	// *********************************************************************************************

#if 0
	static unsigned char sucContineousOperation = 0;
	static unsigned int suiTimeStampContineousOperation;
#define ContineousOperationDelay 5000 // 5 Sec
#endif

//#if ENTER_CMD_ONLY_ONCE_PER_INSTALLATION_STATE    //20160914   bug_No。99

		static unsigned char sucConfSubStateCmdAllowedFlag = 0;
		static unsigned char sucConfSubStateCmdAllowedFlagCopy = 0;
		// 1 = Confirm substate allowed to sent
		// 0 = Confirm substate not allowed to sent
//#endif                //20160914   bug_No。99

	switch (eLogic_Solver_State)
	{

	case Logic_Solver_Init:

		if (gu8_mode_auto_man == 1)
		{
			gstControlBoardStatus.bits.autoManual = 1;
			gstDriveStatusMenu.bits.Auto_Mode_Status = 1;
			gstDriveStatusMenu.bits.Manual_Mode_Status = 0;
		}
		else
		{
			gstControlBoardStatus.bits.autoManual = 0;
			gstDriveStatusMenu.bits.Auto_Mode_Status = 0;
			gstDriveStatusMenu.bits.Manual_Mode_Status = 1;
		}

		if (gu8_run_stop_ctrl == 1)
		{
			gstControlBoardStatus.bits.runStop = 1;
		}
		else
		{
			gstControlBoardStatus.bits.runStop = 0;
		}

		// Capture time stamp to generate power on delay
		suiTimeStamp = g_ui32TickCount;

		eLogic_Solver_State = Logic_Solver_Init_Delay;

		break; // Logic_Solver_Init

	case Logic_Solver_Init_Delay:

		//if ((g_ui32TickCount - suiTimeStamp) >= 20000) // wait for 20 sec before start the logic solver
		if (get_timego( suiTimeStamp) >= 2000) // wait for 2 sec before start the logic solver
		{
			eLogic_Solver_State = Logic_Solver_Power_ON_Init;
		}

		break; // Logic_Solver_Init_Delay

	case Logic_Solver_Power_ON_Init:

		if (
				(
						(
						 gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandResponseStatus == eNO_STATUS &&
						 gstCMDitoLS.commandDisplayBoardLS.bits.startInstallation == 1/*gstCMDitoLS.commandDisplayBoardLS.val == START_INSTALLATION_CMD_FROM_DISPLAY*/
						 ) ||
						 (gKeysStatus.bits.Keys3_3secOpStCl_pressed)
				) &&
				gstLStoCMDr.commandRequestStatus == eINACTIVE &&
				sHandlePowerON_Init == HandlePowerOnInit_Home
			)
		{

			gstLStoCMDr.commandRequestStatus = eACTIVE;
			gstLStoCMDr.commandToDriveBoard.val = 0;
			gstLStoCMDr.commandToDriveBoard.bits.startInstallation = 1;

			// Update last command sent
			sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

			sHandlePowerON_Init = HandlePowerOnInit_InstallationCmdSentToDrive;

		}
//		else if (
//						(
//						 gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandResponseStatus == eNO_STATUS &&
//						 gstCMDitoLS.commandDisplayBoardLS.bits.start_apertureHeight == 1/*gstCMDitoLS.commandDisplayBoardLS.val == START_APERTUREHEIGHT_CMD_FROM_DISPLAY*/
//						 ) &&
//				gstLStoCMDr.commandRequestStatus == eINACTIVE //&&
//				//sHandlePowerON_Init == HandlePowerOnInit_Home
//			)
//		{
//
//			gstLStoCMDr.commandRequestStatus = eACTIVE;
//			gstLStoCMDr.commandToDriveBoard.val = 0;
//			gstLStoCMDr.commandToDriveBoard.bits.start_apertureHeight = 1;
//
//			// Update last command sent
//			sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;
//
//			//sHandlePowerON_Init = HandlePowerOnInit_InstallationCmdSentToDrive;
//
//		}

		else if (
				(
						(
						 gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandResponseStatus == eNO_STATUS &&
						 	 (
						 			 gstCMDitoLS.commandDisplayBoardLS.bits.openPressed == 1 ||

						 			 gstCMDitoLS.commandDisplayBoardLS.bits.closePressed == 1
						 	 )
						 ) ||
						 (gKeysStatus.bits.Key_Open_pressed == 1 || gKeysStatus.bits.Key_Close_pressed == 1 ||
						  gKeysStatus.bits.Wireless_Open_pressed == 1 || gKeysStatus.bits.Wireless_Close_pressed == 1 ||     //20161201
						  gSensorStatus.bits.Sensor_Wireless_1PBS_active == 1 ||   //20161202
						  gSensorStatus.bits.Sensor_1PBS_active == 1 ||(gSensorStatus.bits.Sensor_Obstacle_active && gstControlBoardStatus.bits.autoManual == 1))
				) &&
				(gstLStoCMDr.commandRequestStatus == eINACTIVE) &&
				(sHandlePowerON_Init == HandlePowerOnInit_Home) &&
				//	Power on calibration command shall be initiated only when drivePowerOnCalibration bit is set - Added - Feb 2016
				(gstDriveStatus.bits.drivePowerOnCalibration)
			)
		{

			gstLStoCMDr.commandRequestStatus = eACTIVE;
			gstLStoCMDr.commandToDriveBoard.val = 0;
			gstLStoCMDr.commandToDriveBoard.bits.startPowerOnCalibration = 1;

			// Update last command sent
			sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

			sHandlePowerON_Init = HandlePowerOnInit_InstallationCmdSentToDrive;
		}
		else if (
					(
							(
							 gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandResponseStatus == eNO_STATUS &&
							 gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed == 1
							 ) ||
							 (gKeysStatus.bits.Key_Stop_pressed)
					) &&
					gstLStoCMDr.commandRequestStatus == eINACTIVE &&
					sHandlePowerON_Init == HandlePowerOnInit_Home
				)
		{

			gstLStoCMDr.commandRequestStatus = eACTIVE;
			gstLStoCMDr.commandToDriveBoard.val = 0;
			gstLStoCMDr.commandToDriveBoard.bits.stopPowerOnCalibration = 1;

			// Update last command sent
			sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

			sHandlePowerON_Init = HandlePowerOnInit_InstallationCmdSentToDrive;

		}
		else if (
					(
							(
							 gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandResponseStatus == eNO_STATUS &&
							 	 (
							 			 gstCMDitoLS.commandDisplayBoardLS.bits.openReleased == 1  ||
							 			 gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased == 1 ||
							 			 gstCMDitoLS.commandDisplayBoardLS.bits.stopReleased == 1  ||
							 			 gstCMDitoLS.commandDisplayBoardLS.bits.autoManSel == 1
							 	 )
							 ) /*||
							 (gKeysStatus.bits.Keys3_3secOpStCl_pressed)*/
					) &&
					sHandlePowerON_Init == HandlePowerOnInit_Home
				)
		{

			gstCMDitoLS.commandResponseStatus = eSUCCESS;
			gstCMDitoLS.acknowledgementReceived = eNACK;

		}
		else if ((gstLStoCMDr.commandRequestStatus == eACTIVE) && (sHandlePowerON_Init == HandlePowerOnInit_InstallationCmdSentToDrive))
		{

			if (
					(gstLStoCMDr.commandResponseStatus == eSUCCESS  || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL) &&

					// Check CMDi --> DH block for Inactive, as in case of SUCCESS logic need to change the state of the shutter as Stop
					(gstCMDitoDH.commandRequestStatus == eINACTIVE)
				)
			{

				if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				{

					if (gstCMDitoLS.commandRequestStatus == eACTIVE)
					{

						gstCMDitoLS.commandResponseStatus = eSUCCESS;
						gstCMDitoLS.acknowledgementReceived = eACK;

					} //if (gstCMDitoLS.commandRequestStatus == eACTIVE)

					// Commented as it is not require to stop shutter during installtion and change the local state to 'HandlePowerOnInit_Home' instead 'HandlePowerOnInit_StopInstrucSentToDH'
					/*
					// Change "Run/Stop" status bit of Control board to "Stop" using CMDi --> DH
					gstCMDitoDH.commandRequestStatus = eACTIVE;
					gstCMDitoDH.commandDisplayBoardDH.bits.setParameter = 1;
					gstCMDitoDH.commandDataCMDiDH.parameterNumber = 602;
					// Set Stop
					gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 0;

					sHandlePowerON_Init = HandlePowerOnInit_StopInstrucSentToDH;
					*/

					sHandlePowerON_Init = HandlePowerOnInit_Home;

				} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
				{

					if (gstCMDitoLS.commandRequestStatus == eACTIVE)
					{

						gstCMDitoLS.commandResponseStatus = eSUCCESS;
						gstCMDitoLS.acknowledgementReceived = eNACK;

					} //if (gstCMDitoLS.commandRequestStatus == eACTIVE)

				} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

				if (gKeysStatus.bits.Keys3_3secOpStCl_pressed)
				{
					gKeysStatus.bits.Keys3_3secOpStCl_pressed = 0; // reset key press
				}

				// Release CMDr Block
				gstLStoCMDr.commandRequestStatus = eINACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;
				gstLStoCMDr.commandResponseStatus = eNO_STATUS;

			} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

		} //else if (gstLStoCMDr.commandRequestStatus == eACTIVE)
		  // Handle the response from CMDi --> DH block for Set Drive Status as Stop
		else if ((gstCMDitoDH.commandRequestStatus == eACTIVE) && (sHandlePowerON_Init == HandlePowerOnInit_StopInstrucSentToDH))
		{

			if (gstCMDitoDH.commandResponseStatus == eSUCCESS || gstCMDitoDH.commandResponseStatus == eTIME_OUT || gstCMDitoDH.commandResponseStatus == eFAIL)
			{

				if (gstCMDitoDH.commandResponseStatus == eSUCCESS)
				{

				} //if (gstCMDitoDH.commandResponseStatus == eSUCCESS)
				else if (gstCMDitoDH.commandResponseStatus == eTIME_OUT || gstCMDitoDH.commandResponseStatus == eFAIL)
				{

				} //else if (gstCMDitoDH.commandResponseStatus == eTIME_OUT || gstCMDitoDH.commandResponseStatus == eFAIL)

				// Release CMDi --> DH Block
				gstCMDitoDH.commandRequestStatus = eINACTIVE;
				gstCMDitoDH.commandResponseStatus = eNO_STATUS;

				gstCMDitoDH.commandDisplayBoardDH.bits.setParameter = 0;
				gstCMDitoDH.commandDataCMDiDH.parameterNumber = 0;
				gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 0;

				sHandlePowerON_Init = HandlePowerOnInit_Home;

			} //if (gstCMDitoDH.commandResponseStatus == eSUCCESS || gstCMDitoDH.commandResponseStatus == eTIME_OUT || gstCMDitoDH.commandResponseStatus == eFAIL)

		} // Handle the response from CMDi --> DH block for Set Drive Status as Stop
		else if (gstDriveStatus.bits.driveInstallation && gucSystemInitComplete == 2)
		{
			eLogic_Solver_State = Logic_Solver_Drive_Instalation;
#if 0
					//	Commented on 3 Nov 2014 to avoid control board going into stop mode after
					//	installation
			gstControlBoardStatus.bits.runStop = 0;
#endif

			// De-activate multi-function output and inter-lock output
			DEACTIVATE_MULTI_FUNC_OUT_1;
			DEACTIVATE_MULTI_FUNC_OUT_2;
			DEACTIVATE_MULTI_FUNC_OUT_3;
			DEACTIVATE_MULTI_FUNC_OUT_4;
			DEACTIVATE_MULTI_FUNC_OUT_5;
			RELAY_OPEN;

			/**************************add 20161017 start***********************************/
			    gstLStoCMDr.commandRequestStatus = eACTIVE;
			    gstLStoCMDr.commandToDriveBoard.val = 0;
			    gstLStoCMDr.commandToDriveBoard.bits.stopShutter = 1;

			    // Update last command sent
			    sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;
			/**************************add 20161017 end***********************************/

		}
		else if (gstDriveStatus.bits.drive_apertureHeight && gucSystemInitComplete == 2)
		{
			eLogic_Solver_State = Logic_Solver_Drive_apertureHeight;

			/**************************add 20161017 start***********************************/
			    gstLStoCMDr.commandRequestStatus = eACTIVE;
			    gstLStoCMDr.commandToDriveBoard.val = 0;
			    gstLStoCMDr.commandToDriveBoard.bits.stopShutter = 1;

			    // Update last command sent
			    sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;
			/**************************add 20161017 end***********************************/
		}
		else if (gstDriveStatus.bits.driveReady && gucSystemInitComplete == 2)
		{
			eLogic_Solver_State = Logic_Solver_Drive_Run;
		}

		break; //case Logic_Solver_Power_ON_Init:



	case Logic_Solver_Drive_apertureHeight:
		if (

				(gstCMDitoLS.commandRequestStatus == eACTIVE)     &&
				(gstCMDitoLS.commandResponseStatus == eNO_STATUS) &&
				(gstCMDitoLS.commandDisplayBoardLS.bits.settingsModeStatus)

		)
		{
			guiSettingsModeStatus = gstCMDitoLS.additionalCommandData;

			gstCMDitoLS.commandDisplayBoardLS.val = 0;

			gstCMDitoLS.commandResponseStatus = eSUCCESS;
			gstCMDitoLS.acknowledgementReceived = eACK;

		}
		// *********************************************************************************************
		// End for sub-state 'disable shutter operations while in settings mode' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************


		if (

			(

				 (gstCMDitoLS.commandRequestStatus == eACTIVE  && gstCMDitoLS.commandResponseStatus == eNO_STATUS) ||

						 (gKeysStatus.bits.Key_Open_pressed) 		||
						 (gKeysStatus.bits.Key_Open_released) 		||
						 (gKeysStatus.bits.Key_Close_pressed) 		||
						 (gKeysStatus.bits.Key_Close_released) 		||
						 (gKeysStatus.bits.Wireless_Stop_pressed)

			 )

			 && (gstLStoCMDr.commandRequestStatus == eINACTIVE) &&
			 //
			 //	Added check to see whether system is in healthy state. If a fatal error has occurred then
			 //	don't process operation keys - Jan 2016
			 //
			 //(gstDriveStatus.bits.driveInstallation) &&
			 (gstDriveStatus.bits.driveFaultUnrecoverable == 0) &&
			 (gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)

			)
		{

			// Validate the acceptable key input from display board for installation
			if (
					(
							(gstCMDitoLS.commandRequestStatus == eACTIVE &&

									(
												(
														gstCMDitoLS.commandDisplayBoardLS.bits.upPressed 		||
														gstCMDitoLS.commandDisplayBoardLS.bits.upReleased 		||
														gstCMDitoLS.commandDisplayBoardLS.bits.downPressed 		||
														gstCMDitoLS.commandDisplayBoardLS.bits.downReleased 	||
														gstCMDitoLS.commandDisplayBoardLS.bits.openPressed 		||
														gstCMDitoLS.commandDisplayBoardLS.bits.openReleased 	||
														gstCMDitoLS.commandDisplayBoardLS.bits.closePressed 	||
														gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased	||
														gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed
												) ||

//#if ENTER_CMD_ONLY_ONCE_PER_INSTALLATION_STATE
//											(gstCMDitoLS.commandDisplayBoardLS.bits.enterPressed && sucConfSubStateCmdAllowedFlag != 0)
//#else
											gstCMDitoLS.commandDisplayBoardLS.bits.enterPressed
//#endif

											/*gstCMDitoLS.commandDisplayBoardLS.bits.enterReleased*/
									)

							)

					) && //(gstDriveStatus.bits.driveInstallation) &&
					(
							gstDriveStatus.bits.driveFaultUnrecoverable == 0 &&
							gstControlBoardStatus.bits.controlFaultUnrecoverable == 0
					)
			)
			{

				// Process Up key Pressed, Open key Pressed from Display board and Open Key Pressed from Control board
				if (
						(
								gstCMDitoLS.commandRequestStatus == eACTIVE &&
								(gstCMDitoLS.commandDisplayBoardLS.bits.upPressed || gstCMDitoLS.commandDisplayBoardLS.bits.openPressed)
						)
						|| gKeysStatus.bits.Key_Open_pressed
					) //&& (gstDriveInstallation.bits.installA100 == 1 || gstDriveInstallation.bits.installA101 == 1 || gstDriveInstallation.bits.installA102 == 1))
				{
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.openShutterJog = 1;
					gstLStoCMDr.additionalCommandData = 10;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

				}
				// Process Down key Pressed, Close key Pressed from Display board and Close Key Pressed from Control board
				else if (
							(
							gstCMDitoLS.commandRequestStatus == eACTIVE &&
							(gstCMDitoLS.commandDisplayBoardLS.bits.downPressed || gstCMDitoLS.commandDisplayBoardLS.bits.closePressed)
							)
							|| gKeysStatus.bits.Key_Close_pressed)
						 //&& (gstDriveInstallation.bits.installA100 == 1 || gstDriveInstallation.bits.installA101 == 1 || gstDriveInstallation.bits.installA102 == 1))
				{
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.closeShutterJog = 1;
					gstLStoCMDr.additionalCommandData = 10;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

				}
				// Process Up key Released, Open key Released,Down key Released, Close key Released from Display board and Open Key Released, Close key Released from Control board
				else if (
							(
							 gstCMDitoLS.commandRequestStatus == eACTIVE &&
							(gstCMDitoLS.commandDisplayBoardLS.bits.upReleased || gstCMDitoLS.commandDisplayBoardLS.bits.openReleased || gstCMDitoLS.commandDisplayBoardLS.bits.downReleased || gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased || gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed)
							) ||
							(gKeysStatus.bits.Key_Open_released || gKeysStatus.bits.Key_Close_released || gKeysStatus.bits.Key_Stop_pressed)
						) //&& (gstDriveInstallation.bits.installA100 == 1 || gstDriveInstallation.bits.installA101 == 1 || gstDriveInstallation.bits.installA102 == 1))
				{
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.stopShutter = 1;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

				}
				// Process Enter key Pressed from Display board and Control board
				else if (gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandDisplayBoardLS.bits.enterPressed)
						 //&& (gstDriveInstallation.bits.installA100 == 1 || gstDriveInstallation.bits.installA101 == 1 || gstDriveInstallation.bits.installA102 == 1))
				{
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.confirmSubstateInstallation = 1;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

					#if ENTER_CMD_ONLY_ONCE_PER_INSTALLATION_STATE
					sucConfSubStateCmdAllowedFlag = 0;
					#endif

				}

			} // Validate the acceptable key input from display board for installation
			else
			{

				if (gstCMDitoLS.commandRequestStatus == eACTIVE)
				{
				// For not acceptable key input from display board for installation, send NACK to display board
				gstCMDitoLS.commandResponseStatus = eSUCCESS;
				gstCMDitoLS.acknowledgementReceived = eNACK;
				}

				if (gKeysStatus.bits.Key_Open_pressed)
				{
					gKeysStatus.bits.Key_Open_pressed = 0;
				}

				if (gKeysStatus.bits.Key_Close_pressed)
				{
					gKeysStatus.bits.Key_Close_pressed = 0;
				}

				if (gKeysStatus.bits.Key_Open_released)
				{
					gKeysStatus.bits.Key_Open_released = 0;
				}

				if (gKeysStatus.bits.Key_Close_released)
				{
					gKeysStatus.bits.Key_Close_released = 0;
				}

				if (gKeysStatus.bits.Key_Stop_pressed)
				{
					gKeysStatus.bits.Key_Stop_pressed = 0;
				}

			}

		} // Scan key input from display and control board
		else if (gstLStoCMDr.commandRequestStatus == eACTIVE)
		{
			if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
			{
				if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				{

					if (gstCMDitoLS.commandRequestStatus == eACTIVE)
					{

						gstCMDitoLS.commandResponseStatus = eSUCCESS;
						gstCMDitoLS.acknowledgementReceived = eACK;

					} //if (gstCMDitoLS.commandRequestStatus == eACTIVE)

				} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
				{

					if (gstCMDitoLS.commandRequestStatus == eACTIVE)
					{

						gstCMDitoLS.commandResponseStatus = eSUCCESS;
						gstCMDitoLS.acknowledgementReceived = eNACK;

					} //if (gstCMDitoLS.commandRequestStatus == eACTIVE)

				} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

				// Release Control board key pressed or released basis the command sent
				if (gstLStoCMDr.commandToDriveBoard.bits.openShutterJog)
				{

					if (gKeysStatus.bits.Key_Open_pressed)
					{
						gKeysStatus.bits.Key_Open_pressed = 0;
					}

				} //if (gstLStoCMDr.commandToDriveBoard.bits.openShutterJog)
				else if (gstLStoCMDr.commandToDriveBoard.bits.closeShutterJog)
				{

					if (gKeysStatus.bits.Key_Close_pressed)
					{
						gKeysStatus.bits.Key_Close_pressed = 0;
					}

				} //else if (gstLStoCMDr.commandToDriveBoard.bits.closeShutterJog)
				else if (gstLStoCMDr.commandToDriveBoard.bits.stopShutter)
				{

					if (gKeysStatus.bits.Key_Open_released)
					{
						gKeysStatus.bits.Key_Open_released = 0;
					}

					if (gKeysStatus.bits.Key_Close_released)
					{
						gKeysStatus.bits.Key_Close_released = 0;
					}

					if (gKeysStatus.bits.Key_Stop_pressed)
					{
						gKeysStatus.bits.Key_Stop_pressed = 0;
					}

				} //else if (gstLStoCMDr.commandToDriveBoard.bits.stopShutter)
				else if (gstLStoCMDr.commandToDriveBoard.bits.confirmSubstateInstallation)
				{

					if (gKeysStatus.bits.Keys2_3secStpOpn_pressed)
					{
						gKeysStatus.bits.Keys2_3secStpOpn_pressed = 0;
					}

					if (gKeysStatus.bits.Keys2_3secStpCls_pressed)
					{
						gKeysStatus.bits.Keys2_3secStpCls_pressed = 0;
					}

					if (gKeysStatus.bits.Key_3secStp_pressed)
					{
						gKeysStatus.bits.Key_3secStp_pressed = 0;
					}

				} // else if (gstLStoCMDr.commandToDriveBoard.bits.confirmSubstateInstallation)

				// Release CMDr Block
				gstLStoCMDr.commandRequestStatus = eINACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;
				gstLStoCMDr.commandResponseStatus = eNO_STATUS;

			} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

		} //else if (gstLStoCMDr.commandRequestStatus == eACTIVE)
		else if (gstDriveStatus.bits.driveReady == 1)
		{
			eLogic_Solver_State = Logic_Solver_Drive_Run;

			sucConfSubStateCmdAllowedFlag = 0;
			sucConfSubStateCmdAllowedFlagCopy = 0;
		}

		break; //case Logic_Solver_Drive_apertureHeight:



	case Logic_Solver_Drive_Instalation:


		// *********************************************************************************************
		// Start for sub-state 'disable shutter operations while in settings mode' of 'Logic_Solver_Drive_Run'
		//	RN - DEC 2015
		// *********************************************************************************************

		if (

				(gstCMDitoLS.commandRequestStatus == eACTIVE)     &&
				(gstCMDitoLS.commandResponseStatus == eNO_STATUS) &&
				(gstCMDitoLS.commandDisplayBoardLS.bits.settingsModeStatus)

		)
		{
			guiSettingsModeStatus = gstCMDitoLS.additionalCommandData;

			gstCMDitoLS.commandDisplayBoardLS.val = 0;

			gstCMDitoLS.commandResponseStatus = eSUCCESS;
			gstCMDitoLS.acknowledgementReceived = eACK;

		}
		// *********************************************************************************************
		// End for sub-state 'disable shutter operations while in settings mode' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************




#if ENTER_CMD_ONLY_ONCE_PER_INSTALLATION_STATE

		if (sucConfSubStateCmdAllowedFlag == 0 && sucConfSubStateCmdAllowedFlagCopy == 0)
		{

			if (gstDriveInstallation.bits.installA100 == 1)
			{
				sucConfSubStateCmdAllowedFlag = 1;
				sucConfSubStateCmdAllowedFlagCopy = 1;
			}
			else if (gstDriveInstallation.bits.installA101 == 1)
			{
				sucConfSubStateCmdAllowedFlag = 2;
				sucConfSubStateCmdAllowedFlagCopy = 2;
			}
			else if (gstDriveInstallation.bits.installA102 == 1)
			{
				sucConfSubStateCmdAllowedFlag = 3;
				sucConfSubStateCmdAllowedFlagCopy = 3;
			}
			/*else if (gstDriveInstallation.bits.installationValid)
			{
				sucConfSubStateCmdAllowedFlag = 5;
				sucConfSubStateCmdAllowedFlagCopy = 5;
			}*/
			else if (gstDriveInstallation.bits.installationSuccess == 1)
			{
				sucConfSubStateCmdAllowedFlag = 4;
				sucConfSubStateCmdAllowedFlagCopy = 4;
			}


		} //if (sucConfSubStateCmdAllowedFlag == 0)


		if (sucConfSubStateCmdAllowedFlagCopy != 0)
		{

			if (sucConfSubStateCmdAllowedFlagCopy == 1 && gstDriveInstallation.bits.installA100 == 0)
			{
				sucConfSubStateCmdAllowedFlagCopy = 0;
			}
			else if (sucConfSubStateCmdAllowedFlagCopy == 2 && gstDriveInstallation.bits.installA101 == 0)
			{
				sucConfSubStateCmdAllowedFlagCopy = 0;
			}
			else if (sucConfSubStateCmdAllowedFlagCopy == 3 && gstDriveInstallation.bits.installA102 == 0)
			{
				sucConfSubStateCmdAllowedFlagCopy = 0;
			}
			else if (sucConfSubStateCmdAllowedFlagCopy == 4 && gstDriveInstallation.bits.installationSuccess == 0)
			{
				sucConfSubStateCmdAllowedFlagCopy = 0;
			}
			/*else if(sucConfSubStateCmdAllowedFlagCopy == 5 && gstDriveInstallation.bits.installationValid == 0)
			{
				sucConfSubStateCmdAllowedFlagCopy = 0;
			}*/

		} //if (sucConfSubStateCmdAllowedFlag == 0)

#endif


		if (

			(

				 (gstCMDitoLS.commandRequestStatus == eACTIVE  && gstCMDitoLS.commandResponseStatus == eNO_STATUS) ||

				 (
//						 (gKeysStatus.bits.Key_Open_pressed) 		||     //20170330_2
//						 (gKeysStatus.bits.Key_Open_released) 		||
//						 (gKeysStatus.bits.Key_Close_pressed) 		||
//						 (gKeysStatus.bits.Key_Close_released) 		||
						 (gKeysStatus.bits.Wireless_Stop_pressed) 	||

					(
							(gstDriveInstallation.bits.installA100 && gKeysStatus.bits.Keys2_3secStpOpn_pressed) 	||
							(gstDriveInstallation.bits.installA101 && gKeysStatus.bits.Keys2_3secStpCls_pressed) 	||
							(gstDriveInstallation.bits.installA102 && gKeysStatus.bits.Key_3secStp_pressed) 		||
							(gstDriveInstallation.bits.installationSuccess && gKeysStatus.bits.Key_3secStp_pressed)
					)

				  )

			 )

			 && (gstLStoCMDr.commandRequestStatus == eINACTIVE) &&
			 //
			 //	Added check to see whether system is in healthy state. If a fatal error has occurred then
			 //	don't process operation keys - Jan 2016
			 //
			 //(gstDriveStatus.bits.driveInstallation) &&
			 (gstDriveStatus.bits.driveFaultUnrecoverable == 0) &&
			 (gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)

			)
		{

			// Validate the acceptable key input from display board for installation
			if (
					(
							(gstCMDitoLS.commandRequestStatus == eACTIVE &&

									(
											(
												(gstDriveInstallation.bits.installA100 == 1 || gstDriveInstallation.bits.installA101 == 1 || gstDriveInstallation.bits.installA102 == 1  ) &&
												(
														gstCMDitoLS.commandDisplayBoardLS.bits.upPressed 		||
														gstCMDitoLS.commandDisplayBoardLS.bits.upReleased 		||
														gstCMDitoLS.commandDisplayBoardLS.bits.downPressed 		||
														gstCMDitoLS.commandDisplayBoardLS.bits.downReleased 	||
														gstCMDitoLS.commandDisplayBoardLS.bits.openPressed 		||
														gstCMDitoLS.commandDisplayBoardLS.bits.openReleased 	||
														gstCMDitoLS.commandDisplayBoardLS.bits.closePressed 	||
														gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased	||
														gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed
												)
											) ||

#if ENTER_CMD_ONLY_ONCE_PER_INSTALLATION_STATE
											(gstCMDitoLS.commandDisplayBoardLS.bits.enterPressed && sucConfSubStateCmdAllowedFlag != 0)
#else
											gstCMDitoLS.commandDisplayBoardLS.bits.enterPressed
#endif

											/*gstCMDitoLS.commandDisplayBoardLS.bits.enterReleased*/
									)

							) ||
							(gstCMDitoLS.commandRequestStatus == eACTIVE &&
																		(
																			(gstDriveInstallation.bits.installationValid == 1) &&
																			(
																					gstCMDitoLS.commandDisplayBoardLS.bits.upPressed 		||
																					gstCMDitoLS.commandDisplayBoardLS.bits.downPressed 		||
																					gstCMDitoLS.commandDisplayBoardLS.bits.openPressed 		||
																					gstCMDitoLS.commandDisplayBoardLS.bits.closePressed 	||
																					gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed
																			)
																		)

							)||
							((gstDriveInstallation.bits.installationValid == 1) &&
									                                            (
																						(gKeysStatus.bits.Key_Open_pressed) 	||
																						(gKeysStatus.bits.Key_Close_pressed) 	||
																						(gKeysStatus.bits.Key_Stop_pressed)
																				)
							)||


							(
									(
											(gstDriveInstallation.bits.installA100 == 1 || gstDriveInstallation.bits.installA101 == 1 || gstDriveInstallation.bits.installA102 == 1) &&

											(
													(gKeysStatus.bits.Key_Open_pressed) 	||
													(gKeysStatus.bits.Key_Open_released) 	||
													(gKeysStatus.bits.Key_Close_pressed) 	||
													(gKeysStatus.bits.Key_Close_released)	||
													(gKeysStatus.bits.Key_Stop_pressed)
											)
									) ||

									(

#if ENTER_CMD_ONLY_ONCE_PER_INSTALLATION_STATE
											(sucConfSubStateCmdAllowedFlag != 0) &&
#endif

											(
													(gstDriveInstallation.bits.installA100 && gKeysStatus.bits.Keys2_3secStpOpn_pressed) 	||
													(gstDriveInstallation.bits.installA101 && gKeysStatus.bits.Keys2_3secStpCls_pressed) 	||
													(gstDriveInstallation.bits.installA102 && gKeysStatus.bits.Key_3secStp_pressed) 		||
													(gstDriveInstallation.bits.installationSuccess && gKeysStatus.bits.Key_3secStp_pressed)
											)
									)

							)

					) && //(gstDriveStatus.bits.driveInstallation) &&
					(
							gstDriveStatus.bits.driveFaultUnrecoverable == 0 &&
							gstControlBoardStatus.bits.controlFaultUnrecoverable == 0
					)
			)
			{

				// Process Up key Pressed, Open key Pressed from Display board and Open Key Pressed from Control board
				if ((
						(
								gstCMDitoLS.commandRequestStatus == eACTIVE &&
								(gstCMDitoLS.commandDisplayBoardLS.bits.upPressed || gstCMDitoLS.commandDisplayBoardLS.bits.openPressed)
						)
						|| gKeysStatus.bits.Key_Open_pressed
					) && (gstDriveInstallation.bits.installA100 == 1 || gstDriveInstallation.bits.installA101 == 1 || gstDriveInstallation.bits.installA102 == 1))
				{
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.openShutterJog = 1;
					gstLStoCMDr.additionalCommandData = 10;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

				}
				// Process Down key Pressed, Close key Pressed from Display board and Close Key Pressed from Control board
				else if ((
							(
							gstCMDitoLS.commandRequestStatus == eACTIVE &&
							(gstCMDitoLS.commandDisplayBoardLS.bits.downPressed || gstCMDitoLS.commandDisplayBoardLS.bits.closePressed)
							)
							|| gKeysStatus.bits.Key_Close_pressed)
						  &&(gstDriveInstallation.bits.installA100 == 1 || gstDriveInstallation.bits.installA101 == 1 || gstDriveInstallation.bits.installA102 == 1))
				{
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.closeShutterJog = 1;
					gstLStoCMDr.additionalCommandData = 10;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

				}
				// Process Up key Released, Open key Released,Down key Released, Close key Released from Display board and Open Key Released, Close key Released from Control board
				else if ((
							(
							 gstCMDitoLS.commandRequestStatus == eACTIVE &&
							(gstCMDitoLS.commandDisplayBoardLS.bits.upReleased || gstCMDitoLS.commandDisplayBoardLS.bits.openReleased || gstCMDitoLS.commandDisplayBoardLS.bits.downReleased || gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased || gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed)
							) ||
							(gKeysStatus.bits.Key_Open_released || gKeysStatus.bits.Key_Close_released || gKeysStatus.bits.Key_Stop_pressed)
						) && (gstDriveInstallation.bits.installA100 == 1 || gstDriveInstallation.bits.installA101 == 1 || gstDriveInstallation.bits.installA102 == 1))
				{
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.stopShutter = 1;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

				}
				// Process Enter key Pressed from Display board and Control board
				else if (
						(gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandDisplayBoardLS.bits.enterPressed) 	||
						(
								(gstDriveInstallation.bits.installA100 && gKeysStatus.bits.Keys2_3secStpOpn_pressed) 			||
								(gstDriveInstallation.bits.installA101 && gKeysStatus.bits.Keys2_3secStpCls_pressed) 			||
								(gstDriveInstallation.bits.installA102 && gKeysStatus.bits.Key_3secStp_pressed) 				||
								(gstDriveInstallation.bits.installationSuccess && gKeysStatus.bits.Key_3secStp_pressed)
						)
						)
				{
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.confirmSubstateInstallation = 1;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

					#if ENTER_CMD_ONLY_ONCE_PER_INSTALLATION_STATE
					sucConfSubStateCmdAllowedFlag = 0;
					#endif

				}
				// add by st aoyagi 20160812  START
				else if(((gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed)
						||(gKeysStatus.bits.Key_Stop_pressed))
					  && gstDriveInstallation.bits.installationValid )
				{
					/*gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.stopShutter = 1;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;*/

					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.stopPowerOnCalibration = 1;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

				}
				else if((
						(gstCMDitoLS.commandRequestStatus == eACTIVE &&
						    (gstCMDitoLS.commandDisplayBoardLS.bits.closePressed || gstCMDitoLS.commandDisplayBoardLS.bits.openPressed || gstCMDitoLS.commandDisplayBoardLS.bits.upPressed || gstCMDitoLS.commandDisplayBoardLS.bits.downPressed)
					    )|| gKeysStatus.bits.Key_Open_pressed || gKeysStatus.bits.Key_Close_pressed)
					  && gstDriveInstallation.bits.installationValid  && gstDriveStatus.bits.shutterStopped)
				{
					/*gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.openShutterJog = 1;
					gstLStoCMDr.additionalCommandData = 50;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;*/
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.startPowerOnCalibration = 1;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

				}



			} // Validate the acceptable key input from display board for installation
			else
			{

				if (gstCMDitoLS.commandRequestStatus == eACTIVE)
				{
				// For not acceptable key input from display board for installation, send NACK to display board
				gstCMDitoLS.commandResponseStatus = eSUCCESS;
				gstCMDitoLS.acknowledgementReceived = eNACK;
				}

				if (gKeysStatus.bits.Key_Open_pressed)
				{
					gKeysStatus.bits.Key_Open_pressed = 0;
				}

				if (gKeysStatus.bits.Key_Close_pressed)
				{
					gKeysStatus.bits.Key_Close_pressed = 0;
				}

				if (gKeysStatus.bits.Key_Open_released)
				{
					gKeysStatus.bits.Key_Open_released = 0;
				}

				if (gKeysStatus.bits.Key_Close_released)
				{
					gKeysStatus.bits.Key_Close_released = 0;
				}

				if (gKeysStatus.bits.Key_Stop_pressed)
				{
					gKeysStatus.bits.Key_Stop_pressed = 0;
				}

			}

		} // Scan key input from display and control board
		else if (gstLStoCMDr.commandRequestStatus == eACTIVE)
		{
			if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
			{
				if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				{

					if (gstCMDitoLS.commandRequestStatus == eACTIVE)
					{

						gstCMDitoLS.commandResponseStatus = eSUCCESS;
						gstCMDitoLS.acknowledgementReceived = eACK;

					} //if (gstCMDitoLS.commandRequestStatus == eACTIVE)

				} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
				{

					if (gstCMDitoLS.commandRequestStatus == eACTIVE)
					{

						gstCMDitoLS.commandResponseStatus = eSUCCESS;
						gstCMDitoLS.acknowledgementReceived = eNACK;

					} //if (gstCMDitoLS.commandRequestStatus == eACTIVE)

				} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

				// Release Control board key pressed or released basis the command sent
				if (gstLStoCMDr.commandToDriveBoard.bits.openShutterJog)
				{

					if (gKeysStatus.bits.Key_Open_pressed)
					{
						gKeysStatus.bits.Key_Open_pressed = 0;
					}

				} //if (gstLStoCMDr.commandToDriveBoard.bits.openShutterJog)
				else if (gstLStoCMDr.commandToDriveBoard.bits.closeShutterJog)
				{

					if (gKeysStatus.bits.Key_Close_pressed)
					{
						gKeysStatus.bits.Key_Close_pressed = 0;
					}

				} //else if (gstLStoCMDr.commandToDriveBoard.bits.closeShutterJog)
				else if (gstLStoCMDr.commandToDriveBoard.bits.stopShutter)
				{

					if (gKeysStatus.bits.Key_Open_released)
					{
						gKeysStatus.bits.Key_Open_released = 0;
					}

					if (gKeysStatus.bits.Key_Close_released)
					{
						gKeysStatus.bits.Key_Close_released = 0;
					}

					if (gKeysStatus.bits.Key_Stop_pressed)
					{
						gKeysStatus.bits.Key_Stop_pressed = 0;
					}

				} //else if (gstLStoCMDr.commandToDriveBoard.bits.stopShutter)
				else if (gstLStoCMDr.commandToDriveBoard.bits.confirmSubstateInstallation)
				{

					if (gKeysStatus.bits.Keys2_3secStpOpn_pressed)
					{
						gKeysStatus.bits.Keys2_3secStpOpn_pressed = 0;
					}

					if (gKeysStatus.bits.Keys2_3secStpCls_pressed)
					{
						gKeysStatus.bits.Keys2_3secStpCls_pressed = 0;
					}

					if (gKeysStatus.bits.Key_3secStp_pressed)
					{
						gKeysStatus.bits.Key_3secStp_pressed = 0;
					}

				} // else if (gstLStoCMDr.commandToDriveBoard.bits.confirmSubstateInstallation)

				// Release CMDr Block
				gstLStoCMDr.commandRequestStatus = eINACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;
				gstLStoCMDr.commandResponseStatus = eNO_STATUS;

			} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

		} //else if (gstLStoCMDr.commandRequestStatus == eACTIVE)
		else if (gstDriveStatus.bits.driveReady == 1)
		{
			eLogic_Solver_State = Logic_Solver_Drive_Run;

			sucConfSubStateCmdAllowedFlag = 0;
			sucConfSubStateCmdAllowedFlagCopy = 0;
		}

		break; //case Logic_Solver_Drive_Instalation:

	case Logic_Solver_Drive_Run:

#if 0
#if 0
		if (sucContineousOperation == 0)
		{

			if (gstDriveStatus.bits.shutterUpperLimit == 1)
			{

				sucContineousOperation = 1;

			}

		}
		else if (sucContineousOperation == 1)
		{

			gKeysStatus.bits.Key_Close_pressed = 1;
			sucContineousOperation = 2;

		}
		else if (sucContineousOperation == 2)
		{

			if (gstDriveStatus.bits.shutterLowerLimit == 1)
			{
				sucContineousOperation = 3;
			}

		}
		else if (sucContineousOperation == 3)
		{

			gKeysStatus.bits.Key_Open_pressed = 1;
			sucContineousOperation = 0;

		}
#endif

#if 1

		if (sucContineousOperation == 0)
		{
			suiTimeStampContineousOperation = g_ui32TickCount;
			sucContineousOperation = 1;
		}
		else if (sucContineousOperation == 1)
		{

			if ((g_ui32TickCount - suiTimeStampContineousOperation) > ContineousOperationDelay)
			{
				gKeysStatus.bits.Key_Open_pressed = 1;
				sucContineousOperation = 2;
			}

		}
		else if (sucContineousOperation == 2)
		{
			suiTimeStampContineousOperation = g_ui32TickCount;
			sucContineousOperation = 3;
		}
		else if (sucContineousOperation == 3)
		{

			if ((g_ui32TickCount - suiTimeStampContineousOperation) > ContineousOperationDelay)
			{
				gKeysStatus.bits.Key_Close_pressed = 1;
				sucContineousOperation = 0;
			}

		}

#endif

#endif


		// *********************************************************************************************
		// Start for sub-state 'disable shutter operations while in settings mode' of 'Logic_Solver_Drive_Run'
		//	RN - DEC 2015
		// *********************************************************************************************

		if (

				(gstCMDitoLS.commandRequestStatus == eACTIVE)     &&
				(gstCMDitoLS.commandResponseStatus == eNO_STATUS) &&
				(gstCMDitoLS.commandDisplayBoardLS.bits.settingsModeStatus)

		)
		{
			guiSettingsModeStatus = gstCMDitoLS.additionalCommandData;

			gstCMDitoLS.commandDisplayBoardLS.val = 0;

			gstCMDitoLS.commandResponseStatus = eSUCCESS;
			gstCMDitoLS.acknowledgementReceived = eACK;

		}
		// *********************************************************************************************
		// End for sub-state 'disable shutter operations while in settings mode' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		// *********************************************************************************************
		// Start sub-state 'Handle Keys and Startup' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************
		// Check for active command from display board or control board
		if (
				(
						(
								(gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandResponseStatus == eNO_STATUS) &&
								(		gstCMDitoLS.commandDisplayBoardLS.bits.openPressed 		||
										gstCMDitoLS.commandDisplayBoardLS.bits.openReleased 	||
										gstCMDitoLS.commandDisplayBoardLS.bits.closePressed 	||
										gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased 	||
										gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed 		||
										gstCMDitoLS.commandDisplayBoardLS.bits.stopReleased
								)
						) ||
						(
						 gKeysStatus.bits.Key_Open_pressed 			||
						 gKeysStatus.bits.Key_Open_released 		||
						 gKeysStatus.bits.Key_Close_pressed 		||
						 gKeysStatus.bits.Key_Close_released 		||
						 gKeysStatus.bits.Key_Stop_pressed 			||
						 gKeysStatus.bits.Key_Stop_released 		||
						 gSensorStatus.bits.Sensor_1PBS_active		||
						 gSensorStatus.bits.Sensor_1PBS_inactive	||
						 // 20 Oct Added check for auto mode for startup sensor by yogesh
						 (gSensorStatus.bits.Sensor_Obstacle_active && gstControlBoardStatus.bits.autoManual == 1)		||
						 (gSensorStatus.bits.Sensor_Obstacle_inactive && gstControlBoardStatus.bits.autoManual == 1)	||

						 // 17 Dec 14 Added to support wireless 3PBS & 1PBS functionality
						 gKeysStatus.bits.Wireless_Open_pressed 			||
						 gKeysStatus.bits.Wireless_Open_released 			||
						 gKeysStatus.bits.Wireless_Close_pressed 			||
						 gKeysStatus.bits.Wireless_Close_released 			||
						 gKeysStatus.bits.Wireless_Stop_pressed 			||
						 gKeysStatus.bits.Wireless_Stop_released 			||
						 gSensorStatus.bits.Sensor_Wireless_1PBS_active		||
						 gSensorStatus.bits.Sensor_Wireless_1PBS_inactive

						 )
				) &&
				(gstLStoCMDr.commandRequestStatus == eINACTIVE) 	&&
				(gstDriveStatus.bits.driveReady)					&&
				(gstControlBoardStatus.bits.runStop == 1) 			&&

				// Check for Emergency Signal Not Trigger
				(gstDriveApplicationFault.bits.emergencyStop == 0) 	&&

				// Check intelock input
				(ValidateInterlockInput() == 1) &&

				//	Added this check to implement "disable shutter functionality while we are in settings mode" -RN - Dec 2015
				(guiSettingsModeStatus == 0) &&
				//
				//	Added check to see whether system is in healthy state. If a fatal error has occurred then
				//	then don't process operation keys - Jan 2016
				//
				(gstDriveStatus.bits.driveFaultUnrecoverable == 0) &&
				(gstControlBoardStatus.bits.controlFaultUnrecoverable == 0) &&
				//(gstControlApplicationFault.bits.operationRestrictionTimer == 0) &&
				//
				//	Power fail error category changed to recoverable. Do not send operational commands
				//	when power fail error is present. Added - Jan 2016.
				//
				(gstDriveApplicationFault.bits.powerFail == 0)
		)
		{

			// Open button pressed either from display or control and same send it to Drive when no error present and shutter is not at upper limit
			if (
					// check Open pressed from display board
					(gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandDisplayBoardLS.bits.openPressed) ||

					 gKeysStatus.bits.Key_Open_pressed 		||

					 // Added 17 Dec 14 to support wireless 3PBS & 1PBS
					 gKeysStatus.bits.Wireless_Open_pressed ||

					 (
							 (
									 gSensorStatus.bits.Sensor_1PBS_active ||

									 // Added 17 Dec 14 to support wireless 3PBS & 1PBS
									 gSensorStatus.bits.Sensor_Wireless_1PBS_active
							 ) &&
							 /*(gstDriveStatus.bits.shutterUpperLimit == 0 && gstDriveStatus.bits.shutterApertureHeight == 0)*/
							 //	Check added for 1PBS as it was observed that 1PBS was not getting processed during aperture height mode - Feb 2016
							 //stt ((gu8_1pb_in == 0 && gstDriveStatus.bits.shutterUpperLimit == 0) || (gu8_1pb_in == 1 && gstDriveStatus.bits.shutterApertureHeight == 0))
							 (gstDriveStatus.bits.shutterUpperLimit == 0)
					 ) ||
					 //(gSensorStatus.bits.Sensor_Obstacle_active && gstControlBoardStatus.bits.autoManual == 1 &&
					 (gSensorStatus.bits.Sensor_Obstacle_active && gstControlBoardStatus.bits.autoManual == 1 &&  sucStopKeyControl==0 &&       //20161208
							 //OpenCmdForDistinguish == 0 && gstDriveStatus.bits.shutterMovingUp == 0 && (get_timego(time_ObstacleSensor)>500) && sucStopKeyDisplay==0)   //add 20161020
							 OpenCmdForDistinguish == 0 && gstDriveStatus.bits.shutterMovingUp == 0 && (get_timego(time_ObstacleSensor)>500) && sucStopKeyDisplay==0 &&(((gstDriveStatus.bits.shutterUpperLimit != 1)&&(gu8_en_apheight_ctl==0))||((gstDriveStatus.bits.shutterApertureHeight != 1)&&(gu8_en_apheight_ctl==1))) )   //20161204
				)
			{

				if (gstCMDitoLS.commandDisplayBoardLS.bits.openPressed)
				{
					sucOpenKeyDisplay = 1;
				}

				if (gKeysStatus.bits.Key_Open_pressed)
				{
					sucOpenKeyControl = 1;
				}

				if (gSensorStatus.bits.Sensor_1PBS_active)
				{
					suc1PBSControl = 1;
				}

				// Added 17 Dec 14 to support wireless 3PBS & 1PBS
				if (gKeysStatus.bits.Wireless_Open_pressed)
				{
					sucWirelessOpenKeyControl = 1;
				}

				// Added 17 Dec 14 to support wireless 3PBS & 1PBS
				if (gSensorStatus.bits.Sensor_Wireless_1PBS_active)
				{
					sucWireless1PBSControl = 1;
				}

				// 20 oct added check for automation mode to set the local static flag
				if (gSensorStatus.bits.Sensor_Obstacle_active && gstControlBoardStatus.bits.autoManual == 1)
				{
					sucStartupControl = 1;
				}

					UpEnabled = 1;

				// Check for error / valid condition before sending the command and switch on panels are enabled
				if (
						// Shutter is not at upper limit
						// Upper limit check commented as it is observed while shutter coming down from upper limit,
						// if user press "open' button
						// sometimes shutter is not going up immediately
						// One of the reason, the control board will get new status after 500msecond
						//(gstDriveStatus.bits.shutterUpperLimit == 0) 	&&

						// Check shutter is not at aperture height and command received for opening shutter till aperture height
						(
								(gu8_en_apheight_ctl == 0) ||//半开模式未使能
								((gu8_en_apheight_ctl == 1) && (gstControlBoardStatus.bits.autoManual == 0))||              //20160906   bug_No。87

								(

										gstDriveStatus.bits.shutterApertureHeight == 0 &&//不在半开高度

										(
												(
														gu8_sensor_in == 1 &&	//传感器设置使能

														gSensorStatus.bits.Sensor_Obstacle_active//障碍物传感器触发
												) ||
												(
														gu8_1pb_in == 1 &&//1pbs设置使能
														(
																gSensorStatus.bits.Sensor_1PBS_active ||//控制板1pbs触发

																// Added 17 Dec 14 to support wireless 3PBS & 1PBS
																gSensorStatus.bits.Sensor_Wireless_1PBS_active//无线遥控1pbs触发
														)
												) ||
												(
														gu8_3pb_in == 1 &&//3pbs设置使能
														(
																gstCMDitoLS.commandDisplayBoardLS.bits.openPressed 	||//显示板open按键触发
																gKeysStatus.bits.Key_Open_pressed					||//控制板open按键触发

																// Added 17 Dec 14 to support wireless 3PBS & 1PBS
																gKeysStatus.bits.Wireless_Open_pressed//无线遥控open按键使能
														)
												)
										)
								) ||
								(
										(
												gu8_sensor_in == 0 && gSensorStatus.bits.Sensor_Obstacle_active//传感器设置未使能 &&障碍物传感器触发
										) ||
										(
												gu8_1pb_in == 0 &&//1pbs设置不使能 && (控制板1pbs使能 || 无线遥控1pbs触发）
												(gSensorStatus.bits.Sensor_1PBS_active || gSensorStatus.bits.Sensor_Wireless_1PBS_active)
										) ||
										(
												gu8_3pb_in == 0 &&//3pbs设置不使能 && (显示板open按键使能 || 控制板open按键使能 || 无线遥控open按键使能）
												(
														gstCMDitoLS.commandDisplayBoardLS.bits.openPressed 	||
														gKeysStatus.bits.Key_Open_pressed 					||
														gKeysStatus.bits.Wireless_Open_pressed
												)
										)

								)

						) &&

						// Stop and close keys are not pressed
						(
//								sucCloseKeyDisplay == 0 && sucCloseKeyControl == 0 	&&
								//sucStopKeyDisplay == 0 &&
								sucStopKeyControl == 0   //20161205
//								&&  sucWirelessCloseKeyControl == 0    //20161204
						) &&

						// Check Open key on display panel not pressed OR Open key on display panel pressed,then it should be enabled
						(
								(gstCMDitoLS.commandDisplayBoardLS.bits.openPressed == 0) ||
								(gstCMDitoLS.commandDisplayBoardLS.bits.openPressed	&& gu8_en_switchpan == 0 /*open & close buttons on the Display panel enabled*/)
						) &&
#if 0	                //	Commented to disable startup sensor check during open close operation on 21 Oct 2014
						//	as Yogesh found issue during onsite testing
						// check startup sensor not active OR startup sensor active then mode should be auto mode
						(
								// 20 Oct : Commented below line and added new check for checking manual mode,
								// in order to allow open in manual mode even startup sensor is active
								//(gSensorStatus.bits.Sensor_Obstacle_active == 0) ||
								(gstControlBoardStatus.bits.autoManual == 0) ||
								(gSensorStatus.bits.Sensor_Obstacle_active==1 && gstControlBoardStatus.bits.autoManual == 1)
						) &&
#endif

						// Check any fault is not active on drive board
						(
								(gstDriveStatus.bits.driveFault == 0) || (gstDriveStatus.bits.driveFaultUnrecoverable == 0)
						) &&

						// Check any fault is not active on control board
						((gstControlBoardStatus.bits.controlFault == 0) || (gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)) &&

						//(gstControlApplicationFault.bits.operationRestrictionTimer == 0) &&
						// Check none of the safety signal is triggered
						(gstDriveApplicationFault.bits.microSwitch == 0 && gstDriveApplicationFault.bits.peObstacle == 0 /*&& gstControlApplicationFault.bits.startupSafetySensor == 0*/)  //20160906 item 104

						// Check last command sent not the same as the one which we are going to sent
						//(sstLStoCMDrCmdSent.commandToDriveBoard.bits.openShutter == 0 && sstLStoCMDrCmdSent.commandToDriveBoard.bits.openShutterApperture == 0)
						)
				{


					if (
							// Check Aperture setting to disabled
							// 23 Oct, Added additional check to disable "Aperture feature" in Manual Mode
							((gu8_en_apheight_ctl == 0) || (gstControlBoardStatus.bits.autoManual == 0)) ||
							//半开模式不使能 || 手动模式
							(
								(gu8_en_apheight_ctl == 1) &&//半开模式使能
								(
									(gu8_sensor_in == 0 && gSensorStatus.bits.Sensor_Obstacle_active ) 	||//传感器设置未使能 &&障碍物传感器触发
									(
											gu8_1pb_in == 0 &&//1pbs设置不使能 && (控制板1pbs触发 || 无线遥控1pbs触发）
											(gSensorStatus.bits.Sensor_1PBS_active || gSensorStatus.bits.Sensor_Wireless_1PBS_active)
									) ||
									(
											gu8_3pb_in == 0 &&//3pbs设置不使能 && (显示板open按键触发 || 控制板open按键触发 || 无线遥控open按键触发）
											(gstCMDitoLS.commandDisplayBoardLS.bits.openPressed || gKeysStatus.bits.Key_Open_pressed || gKeysStatus.bits.Wireless_Open_pressed)
									)
								)
							)
						)
					{

						sstLStoCMDrCmdToBeSent.commandToDriveBoard.val = 0;
						sstLStoCMDrCmdToBeSent.commandToDriveBoard.bits.openShutter = 1;
						sucLastOpenCommandType = 0;
						OpenCmdForDistinguish = 1;
						static unsigned char sucLastOpenCommandType = 0; // 0 == Open command,1 == Open Aperture height command

					} else
					{
						//if(gstControlApplicationFault.bits.startupSafetySensor == 1)  //20160906 item104
						if((gstControlApplicationFault.bits.startupSafetySensor == 1)&&(gstDriveStatus.bits.shutterUpperLimit != 1))  //20161205
						{
							season_cyw = 1;
						}
						else if(gstDriveStatus.bits.shutterBetweenUplmtAphgt)
						{
							sstLStoCMDrCmdToBeSent.commandToDriveBoard.val = 0;
							sstLStoCMDrCmdToBeSent.commandToDriveBoard.bits.openShutter = 1;
							sucLastOpenCommandType = 0;
							OpenCmdForDistinguish = 1;
						}
						else
						{
							sstLStoCMDrCmdToBeSent.commandToDriveBoard.val = 0;
							sstLStoCMDrCmdToBeSent.commandToDriveBoard.bits.openShutterApperture = 1;
							sucLastOpenCommandType = 1;
							OpenCmdForDistinguish = 0;
						}
					}

					// Check whether Go up operation delay need to start or not. Also check operating mode to activate Go up operation delay.
					// A005 - Mode setting for enabling the Go-up and Go-Down delay timers = 0: Enabled only in auto mode, 1: Enabled only in manual mode,2: Enabled in both auto and manual modes
					if (
							(
									((gu8_en_oprdelay == 0 || gu8_en_oprdelay == 2) && gstControlBoardStatus.bits.autoManual == 1) ||
									((gu8_en_oprdelay == 1 || gu8_en_oprdelay == 2) && gstControlBoardStatus.bits.autoManual == 0)) &&
									gu8_goup_oprdelay != 0 	&& gstDriveStatus.bits.shutterLowerLimit
							)
					{

						seShutterOpenCloseCmdState = CmdUpDetectedWaitUpDelay;
						suiTimeStamp = g_ui32TickCount;

						// In case of go up operation delay, the actual command will sent after delay,
						// so it is require to released the pressed Key/signal immediately with out waiting for actual reply
						if (gstCMDitoLS.commandDisplayBoardLS.bits.openPressed)
						{

							gstCMDitoLS.commandResponseStatus = eSUCCESS;
							gstCMDitoLS.acknowledgementReceived = eACK;

						}

						if (gKeysStatus.bits.Key_Open_pressed)
						{
							gKeysStatus.bits.Key_Open_pressed = 0;
						}

						if (gSensorStatus.bits.Sensor_1PBS_active)
						{
							gSensorStatus.bits.Sensor_1PBS_active = 0;
						}

						if (gSensorStatus.bits.Sensor_Obstacle_active)   //20161202pm   //20170330_4
						{
							gSensorStatus.bits.Sensor_Obstacle_active = 0;
						}

						if (gKeysStatus.bits.Wireless_Open_pressed)
						{
							gKeysStatus.bits.Wireless_Open_pressed = 0;
						}

						if (gSensorStatus.bits.Sensor_Wireless_1PBS_active)
						{
							gSensorStatus.bits.Sensor_Wireless_1PBS_active = 0;
						}

					}
					else
					{

						// Reset the "UP or Down command detected and wait for delay before sending actual command" flag
						seShutterOpenCloseCmdState = CmdNotDetected;

						gstLStoCMDr.commandRequestStatus = eACTIVE;
						gstLStoCMDr.commandToDriveBoard.val = sstLStoCMDrCmdToBeSent.commandToDriveBoard.val;
						seHandleKeysStartupState = CmdSentWaitingForReply;

						// Update last command sent
						sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

					}

				} //// Check for error before sending the command
				else
				{

					// Reject key event....
					if (gstCMDitoLS.commandDisplayBoardLS.bits.openPressed)
					{

						gstCMDitoLS.commandResponseStatus = eSUCCESS;
						gstCMDitoLS.acknowledgementReceived = eNACK;

					}

					if (gKeysStatus.bits.Key_Open_pressed)
					{
						gKeysStatus.bits.Key_Open_pressed = 0;
					}

					if (gSensorStatus.bits.Sensor_1PBS_active)
					{
					gSensorStatus.bits.Sensor_1PBS_active = 0;
					}

//					if (gSensorStatus.bits.Sensor_Obstacle_active)    //20161202pm
//					{
//						gSensorStatus.bits.Sensor_Obstacle_active = 0;
//					}

					if (gKeysStatus.bits.Wireless_Open_pressed)
					{
						gKeysStatus.bits.Wireless_Open_pressed = 0;
					}

					if (gSensorStatus.bits.Sensor_Wireless_1PBS_active)
					{
						gSensorStatus.bits.Sensor_Wireless_1PBS_active = 0;
					}

				} //

			} // Open button pressed either from display or control and same send it to Drive when no error present and shutter is not at upper limit
			  // Close button pressed either from display or control and same send it to Drive when no error present and shutter is not at lower limit
			else if (
						// check Close pressed from display board
						(gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandDisplayBoardLS.bits.closePressed) ||
						(gKeysStatus.bits.Key_Close_pressed) 	||
						(gSensorStatus.bits.Sensor_1PBS_active) ||

						(gKeysStatus.bits.Wireless_Close_pressed) 	||
						(gSensorStatus.bits.Sensor_Wireless_1PBS_active)

					)
			{
				updateControlBoardApplicationFaults();
				if (gstCMDitoLS.commandDisplayBoardLS.bits.closePressed)
				{
					sucCloseKeyDisplay = 1;
				}

				if (gKeysStatus.bits.Key_Close_pressed)
				{
					sucCloseKeyControl = 1;
				}

				if (gSensorStatus.bits.Sensor_1PBS_active)
				{
					suc1PBSControl = 1;
				}

				if (gKeysStatus.bits.Wireless_Close_pressed)
				{
					sucWirelessCloseKeyControl = 1;
				}

				if (gSensorStatus.bits.Sensor_Wireless_1PBS_active)
				{
					sucWireless1PBSControl = 1;
				}

				// Check for error / valid condition before sending the command and switch on panels are enabled
				if (
						// Check for 1PBS not pressed OR 1PBS pressed, then shutter should be at upper limit
						(
								(gSensorStatus.bits.Sensor_1PBS_active == 0 && gSensorStatus.bits.Sensor_Wireless_1PBS_active == 0) ||
								(
										(gSensorStatus.bits.Sensor_1PBS_active == 1 || gSensorStatus.bits.Sensor_Wireless_1PBS_active == 1 ) &&
										(gstDriveStatus.bits.shutterUpperLimit == 1 || gstDriveStatus.bits.shutterApertureHeight == 1) 		&&

										// 23 Oct Added condition to allow 1 PBS to close shutter from Upper Limit only in case of Manual mode
										// In Auto mode closing the shutter from upper limit using 1 PBS not required
										(gstControlBoardStatus.bits.autoManual == 0)
								)
						) &&
#if 0					//	Commented to disable startup sensor check during open close operation on 21 Oct 2014
						//	as Yogesh found issue during onsite testing
						// check startup sensor not active OR startup sensor active then mode should be auto mode
						// 20 Oct : Commented below line and added new check for checking manual mode,
						// in order to allow open in manual mode even startup sensor is active
						//((sucStartupControl == 0) || (sucStartupControl == 1 && gstControlBoardStatus.bits.autoManual == 1)) &&
						((gstControlBoardStatus.bits.autoManual == 0) || (sucStartupControl == 1 && gstControlBoardStatus.bits.autoManual == 1)) &&
#endif
						// Shutter is not at lower limit
						gstDriveStatus.bits.shutterLowerLimit == 0 &&

						(sucStopKeyDisplay == 0 && sucStopKeyControl == 0 && sucWirelessStopKeyControl == 0) &&

						// Check OPEN key not pressed OR if OPEN key pressed, Close operation setting should be enabled
						//((sucOpenKeyDisplay == 0 && sucOpenKeyControl == 0) || ((sucOpenKeyDisplay != 0 || sucOpenKeyControl != 0) && gu8_close_oprset == 1)) &&

						// Check OPEN key not pressed
						(sucOpenKeyDisplay == 0 && sucOpenKeyControl == 0 && sucStartupControl == 0 && sucWirelessOpenKeyControl == 0) &&

						// Check close key on display panel not pressed OR Close key on display panel pressed,then it should be enabled
						(
								(gstCMDitoLS.commandDisplayBoardLS.bits.closePressed == 0) ||
								(gstCMDitoLS.commandDisplayBoardLS.bits.closePressed && gu8_en_switchpan == 0 /*open & close buttons on the Display panel enabled*/)
						) &&

						//Check for Emergency Signal Not Trigger
						(gstDriveApplicationFault.bits.emergencyStop == 0) &&

						//(gstControlApplicationFault.bits.operationRestrictionTimer == 0) &&

						//Check for Safety Signal Not Trigger OR if trigger then close operation setting should be enabled
						(
								(
										gstDriveApplicationFault.bits.microSwitch == 0 &&
										gstDriveApplicationFault.bits.peObstacle == 0 &&
										gstControlApplicationFault.bits.startupSafetySensor == 0
										//(gstDriveStatus.bits.shutterUpperLimit == 1 && gSensorStatus.bits.Sensor_Safety_active == true))
								) ||
								 (
										 (
												 gu8_close_oprset == 1 										&&
												 gSensorStatus.bits.Sensor_1PBS_active == 0					&&
												 gSensorStatus.bits.Sensor_Wireless_1PBS_active == 0
										 ) &&
										 (
												 gstDriveApplicationFault.bits.microSwitch == 1 			||
												 gstDriveApplicationFault.bits.peObstacle == 1 				||
												 gstControlApplicationFault.bits.startupSafetySensor == 1
										)
								)
						)

						//check other drive and control board errors are not active

						// Check last command sent not the same as the one which we are going to sent
						// (sstLStoCMDrCmdSent.commandToDriveBoard.bits.closeShutter == 0 && sstLStoCMDrCmdSent.commandToDriveBoard.bits.closeShutterApperture == 0 && sstLStoCMDrCmdSent.commandToDriveBoard.bits.closeShutterJog == 0)

						)
				{

					// if Close operation setting should be enabled, then trigger JOG Close command
					if (
							(gu8_close_oprset == 1) /*&&                             //20161012
							(gstDriveApplicationFault.bits.microSwitch == 1 ||
							 gstDriveApplicationFault.bits.wraparound == 1 ||
							 gstDriveApplicationFault.bits.peObstacle == 1  ||
							 gstControlApplicationFault.bits.startupSafetySensor == 1)*/
					   )
					{

						sstLStoCMDrCmdToBeSent.commandToDriveBoard.val = 0;
						sstLStoCMDrCmdToBeSent.commandToDriveBoard.bits.closeShutterJog = 1;
						sstLStoCMDrCmdToBeSent.additionalCommandData = 50;
						OpenCmdForDistinguish = 0;

					}
					else
					{

						// Check Aperture setting to disabled
						if (
								(gu8_en_apheight_ctl == 0) ||

								(
									(gu8_en_apheight_ctl == 1) &&
									(
										(gu8_1pb_in == 0 && (gSensorStatus.bits.Sensor_1PBS_active || gSensorStatus.bits.Sensor_Wireless_1PBS_active)) ||
										(
												gu8_3pb_in == 0 &&
												(
														gstCMDitoLS.commandDisplayBoardLS.bits.closePressed ||
														gKeysStatus.bits.Key_Close_pressed
												)
										)
									)
								)
						   )
						{

							sstLStoCMDrCmdToBeSent.commandToDriveBoard.val = 0;
							sstLStoCMDrCmdToBeSent.commandToDriveBoard.bits.closeShutter = 1;
							OpenCmdForDistinguish = 0;

						} else
						{

							sstLStoCMDrCmdToBeSent.commandToDriveBoard.val = 0;
							sstLStoCMDrCmdToBeSent.commandToDriveBoard.bits.closeShutterApperture = 1;
							OpenCmdForDistinguish = 0;

						}
						time_ObstacleSensor = g_ui32TickCount;  //20161202pm

					}

					// Check whether Go down operation delay need to start or not. Also check operating mode to activate Go down operation delay.
					// A005 - Mode setting for enabling the Go-up and Go-Down delay timers = 0: Enabled only in auto mode, 1: Enabled only in manual mode,2: Enabled in both auto and manual modes

					// Also check the condition of go down operation delay command to drive is applicable only for command "Close Shutter" and not "Close Shutter Jog"
					if (
							(
									((gu8_en_oprdelay == 0 || gu8_en_oprdelay == 2) && gstControlBoardStatus.bits.autoManual == 1) ||
									((gu8_en_oprdelay == 1 || gu8_en_oprdelay == 2) && gstControlBoardStatus.bits.autoManual == 0)
							) &&
							(gu8_godn_oprdelay != 0) &&
							(gstDriveStatus.bits.shutterUpperLimit) &&
							(sstLStoCMDrCmdToBeSent.commandToDriveBoard.bits.closeShutter == 1)
					  )
					{

						seShutterOpenCloseCmdState = CmdDownDetectedWaitDownDelay;
						suiTimeStamp = g_ui32TickCount;

						// In case of go up operation delay, the actual command will sent after delay,
						// so it is require to released the pressed Key/signal immediately with out waiting for actual reply
						if (gstCMDitoLS.commandDisplayBoardLS.bits.closePressed)
						{

							gstCMDitoLS.commandResponseStatus = eSUCCESS;
							gstCMDitoLS.acknowledgementReceived = eACK;

						}

						if (gKeysStatus.bits.Key_Close_pressed)
						{
							gKeysStatus.bits.Key_Close_pressed = 0;
						}

						if (gSensorStatus.bits.Sensor_1PBS_active)
						{
							gSensorStatus.bits.Sensor_1PBS_active = 0;
						}

						// Added on 17 Dec 14 to support wireless functionality
						if (gKeysStatus.bits.Wireless_Close_pressed)
						{
							gKeysStatus.bits.Wireless_Close_pressed = 0;
						}

						if (gSensorStatus.bits.Sensor_Wireless_1PBS_active)
						{
							gSensorStatus.bits.Sensor_Wireless_1PBS_active = 0;
						}

					} else
					{

						// Reset the "UP or Down command detected and wait for delay before sending actual command" flag
						seShutterOpenCloseCmdState = CmdNotDetected;

						gstLStoCMDr.commandRequestStatus = eACTIVE;
						gstLStoCMDr.commandToDriveBoard.val = sstLStoCMDrCmdToBeSent.commandToDriveBoard.val;
						gstLStoCMDr.additionalCommandData = sstLStoCMDrCmdToBeSent.additionalCommandData;
						seHandleKeysStartupState = CmdSentWaitingForReply;

						// Update last command sent
						sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

					}

				} //// Check for error before sending the command
				else
				{

					// Reject key event....
					if (gstCMDitoLS.commandDisplayBoardLS.bits.closePressed)
					{

						gstCMDitoLS.commandResponseStatus = eSUCCESS;
						gstCMDitoLS.acknowledgementReceived = eNACK;

					}

					if (gKeysStatus.bits.Key_Close_pressed)
					{
						gKeysStatus.bits.Key_Close_pressed = 0;
					}

					if (gSensorStatus.bits.Sensor_1PBS_active)
					{
						gSensorStatus.bits.Sensor_1PBS_active = 0;
					}

					// Added on 17 Dec 14 to support wireless functionality
					if (gKeysStatus.bits.Wireless_Close_pressed)
					{
						gKeysStatus.bits.Wireless_Close_pressed = 0;
					}

					if (gSensorStatus.bits.Sensor_Wireless_1PBS_active)
					{
						gSensorStatus.bits.Sensor_Wireless_1PBS_active = 0;
					}

				} //

			} // Close button pressed either from display or control and same send it to Drive when no error present and shutter is not at lower limit
			  // Stop button pressed either from display or control and same send it to Drive when no error present
			else if (
						// check Stop pressed from display board
						(gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed) ||
						(gKeysStatus.bits.Key_Stop_pressed)																	||
						(gKeysStatus.bits.Wireless_Stop_pressed)
					)
			{

				if (gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed)
				{
					sucStopKeyDisplay = 1;
				}

				if (gKeysStatus.bits.Key_Stop_pressed)
				{
					sucStopKeyControl = 1;
					// Reset the flag which indicate either Up or Down  and Go UP or Go Down delay is in progress
					if ((seShutterOpenCloseCmdState == CmdUpDetectedWaitUpDelay)||(seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay))seShutterOpenCloseCmdState = CmdNotDetected;      //20170328
				}

				if (gKeysStatus.bits.Wireless_Stop_pressed)
				{
					sucWirelessStopKeyControl = 1;
				}

				// Check last command sent not the same as the one which we are going to sent AND Safety and Emergency signals are inactive
				if (
						// Check last command sent not the same as the one which we are going to sent
						(sstLStoCMDrCmdSent.commandToDriveBoard.bits.stopShutter == 0) &&

						// Check any fault is not active on drive board
						((gstDriveStatus.bits.driveFault == 0) || (gstDriveStatus.bits.driveFaultUnrecoverable == 0)) &&

						// Check any fault is not active on control board
						((gstControlBoardStatus.bits.controlFault == 0) || (gstControlBoardStatus.bits.controlFaultUnrecoverable == 0))

						)
				{
					time_ObstacleSensor = g_ui32TickCount;  //add 20161020
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.stopShutter = 1;
					OpenCmdForDistinguish = 0;
					seHandleKeysStartupState = CmdSentWaitingForReply;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val =
							gstLStoCMDr.commandToDriveBoard.val;

					// Reset the "UP or Down command detected and wait for delay before sending actual command" flag
					seShutterOpenCloseCmdState = CmdNotDetected;

				} else
				{

					// Reject key event....
					if (gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed)
					{

						gstCMDitoLS.commandResponseStatus = eSUCCESS;
						gstCMDitoLS.acknowledgementReceived = eNACK;

					}

					if (gKeysStatus.bits.Key_Stop_pressed)
					{
						gKeysStatus.bits.Key_Stop_pressed = 0;
					}

					if (gKeysStatus.bits.Wireless_Stop_pressed)
					{
						gKeysStatus.bits.Wireless_Stop_pressed = 0;
					}

				}

			} // Stop button pressed either from display or control and same send it to Drive when no error present
			  // Open button released
			else if
			(
			// check Open released from display board
			(gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandDisplayBoardLS.bits.openReleased) ||
			 gKeysStatus.bits.Key_Open_released || gSensorStatus.bits.Sensor_1PBS_inactive || gSensorStatus.bits.Sensor_Obstacle_inactive ||

			 // Added by Yogesh on 17 Dec 14 for Wireless functionality
			 gKeysStatus.bits.Wireless_Open_released || gSensorStatus.bits.Sensor_Wireless_1PBS_inactive)
			{

				if (gstCMDitoLS.commandDisplayBoardLS.bits.openReleased)
				{

					sucOpenKeyDisplay = 0;
					gstCMDitoLS.commandResponseStatus = eSUCCESS;
					gstCMDitoLS.acknowledgementReceived = eNACK;

				}

				if (gKeysStatus.bits.Key_Open_released)
				{
					sucOpenKeyControl = 0;
					gKeysStatus.bits.Key_Open_released = 0;
				}

				if (gSensorStatus.bits.Sensor_1PBS_inactive)
				{
					suc1PBSControl = 0;
					gSensorStatus.bits.Sensor_1PBS_inactive = 0;
				}

				if (gSensorStatus.bits.Sensor_Obstacle_inactive)
				{
					sucStartupControl = 0;
					gSensorStatus.bits.Sensor_Obstacle_inactive = 0;
				}

				// Added by Yogesh on 17 Dec 14 for Wireless functionality
				if (gKeysStatus.bits.Wireless_Open_released)
				{
					sucWirelessOpenKeyControl = 0;
					gKeysStatus.bits.Wireless_Open_released = 0;
				}

				if (gSensorStatus.bits.Sensor_Wireless_1PBS_inactive)
				{
					sucWireless1PBSControl = 0;
					gSensorStatus.bits.Sensor_Wireless_1PBS_inactive = 0;
				}

			} // Open button released
			  // Close button released with last command sent is "Close Shutter Jog 10%"
			else if (
						(
							// check Close released from display board
							(gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased) ||
							gKeysStatus.bits.Key_Close_released ||

							// Added by Yogesh on 17 Dec 14 for Wireless functionality
							gKeysStatus.bits.Wireless_Close_released

						) &&
						(gstDriveStatus.bits.shutterLowerLimit == 0) &&
						(sstLStoCMDrCmdSent.commandToDriveBoard.bits.closeShutterJog == 1)
					)
			{

				gstLStoCMDr.commandRequestStatus = eACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;
				gstLStoCMDr.commandToDriveBoard.bits.stopShutter = 1;
				OpenCmdForDistinguish = 0;
				seHandleKeysStartupState = CmdSentWaitingForReply;

				// Update last command sent
				sstLStoCMDrCmdSent.commandToDriveBoard.val =
						gstLStoCMDr.commandToDriveBoard.val;

				if (gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased)
				{

					sucCloseKeyDisplay = 0;

				}

				if (gKeysStatus.bits.Key_Close_released)
				{
					sucCloseKeyControl = 0;

				}

				// Added by Yogesh on 17 Dec 14 for Wireless functionality
				if (gKeysStatus.bits.Wireless_Close_released)
				{
					sucWirelessCloseKeyControl = 0;

				}

			} // Close button released with last command sent is "Close Shutter Jog 10%"
			  // Close button released without last command sent is "Close Shutter Jog 10%"
			else if (
					// check Close released from display board
					(gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased) ||
					(gKeysStatus.bits.Key_Close_released) ||

					// Added by Yogesh on 17 Dec 14 for Wireless functionality
					(gKeysStatus.bits.Wireless_Close_released)
					)
			{

				if (gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased)
				{

					sucCloseKeyDisplay = 0;
					gstCMDitoLS.commandResponseStatus = eSUCCESS;
					gstCMDitoLS.acknowledgementReceived = eNACK;

				}

				if (gKeysStatus.bits.Key_Close_released)
				{
					sucCloseKeyControl = 0;
					gKeysStatus.bits.Key_Close_released = 0;

				}

				// Added by Yogesh on 17 Dec 14 for Wireless functionality
				if (gKeysStatus.bits.Wireless_Close_released)
				{
					sucWirelessCloseKeyControl = 0;
					gKeysStatus.bits.Wireless_Close_released = 0;

				}

			} // Close button released without last command sent is "Close Shutter Jog 10%"
			  // Stop button released
			else if (
						// check Stop released from display board
						(gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandDisplayBoardLS.bits.stopReleased) ||
						(gKeysStatus.bits.Key_Stop_released) ||
						(gKeysStatus.bits.Wireless_Stop_released)
					)
			{

				if (gstCMDitoLS.commandDisplayBoardLS.bits.stopReleased)
				{

					sucStopKeyDisplay = 0;
					gstCMDitoLS.commandResponseStatus = eSUCCESS;
					gstCMDitoLS.acknowledgementReceived = eNACK;

				}

				if (gKeysStatus.bits.Key_Stop_released)
				{
					sucStopKeyControl = 0;
					gKeysStatus.bits.Key_Stop_released = 0;

				}

				if (gKeysStatus.bits.Wireless_Stop_released)
				{
					sucWirelessStopKeyControl = 0;
					gKeysStatus.bits.Wireless_Stop_released = 0;

				}

			} // Stop button released

		} // Check for active command from display board or control board
		  // check waiting "Open Shutter" command for "Go Up Delay" and Close Shutter" command for "Go Down Delay"
		else if (seShutterOpenCloseCmdState == CmdUpDetectedWaitUpDelay
				|| seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay)
		{

			if (((seShutterOpenCloseCmdState == CmdUpDetectedWaitUpDelay
					&& (get_timego( suiTimeStamp)
							>= ((uint32_t) gu8_goup_oprdelay * 1000)))
					|| (seShutterOpenCloseCmdState
							== CmdDownDetectedWaitDownDelay
							&& (get_timego( suiTimeStamp)
									>= ((uint32_t) gu8_godn_oprdelay * 1000))))
					&& gstLStoCMDr.commandRequestStatus == eINACTIVE) {
				gstLStoCMDr.commandRequestStatus = eACTIVE;
				gstLStoCMDr.commandToDriveBoard.val =
						sstLStoCMDrCmdToBeSent.commandToDriveBoard.val;
				seHandleKeysStartupState = CmdSentWaitingForReply;

				// Update last command sent
				sstLStoCMDrCmdSent.commandToDriveBoard.val =
						gstLStoCMDr.commandToDriveBoard.val;

				// Reset the flag which indicate either Up or Down key detected and Go UP or Go Down delay is in progress
				seShutterOpenCloseCmdState = CmdNotDetected;

			}

		} // check waiting "Open Shutter" command for "Go Up Delay" and Close Shutter" command for "Go Down Delay"
		  // handle the response for command sent through 'HandleKeysStartup'
		else if (gstLStoCMDr.commandRequestStatus == eACTIVE
				&& seHandleKeysStartupState == CmdSentWaitingForReply)
		{

			if (gstLStoCMDr.commandResponseStatus == eSUCCESS
					|| gstLStoCMDr.commandResponseStatus == eTIME_OUT
					|| gstLStoCMDr.commandResponseStatus == eFAIL) {

				if (gstLStoCMDr.commandResponseStatus == eSUCCESS) {

					if (gstCMDitoLS.commandRequestStatus == eACTIVE) {

						gstCMDitoLS.commandResponseStatus = eSUCCESS;
						gstCMDitoLS.acknowledgementReceived = eACK;

					} //if (gstCMDitoLS.commandRequestStatus == eACTIVE)

				} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT
						|| gstLStoCMDr.commandResponseStatus == eFAIL) {

					if (gstCMDitoLS.commandRequestStatus == eACTIVE) {

						gstCMDitoLS.commandResponseStatus = eSUCCESS;
						gstCMDitoLS.acknowledgementReceived = eNACK;

					} //if (gstCMDitoLS.commandRequestStatus == eACTIVE)

				} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

				// Release Control board key pressed or released basis the command sent
				if (gstLStoCMDr.commandToDriveBoard.bits.openShutter || gstLStoCMDr.commandToDriveBoard.bits.openShutterApperture)
				{

					if (gKeysStatus.bits.Key_Open_pressed)
					{
						gKeysStatus.bits.Key_Open_pressed = 0;
					}

					if (gSensorStatus.bits.Sensor_1PBS_active)
					{
						gSensorStatus.bits.Sensor_1PBS_active = 0;
					}

//					if (gSensorStatus.bits.Sensor_Obstacle_active)     //add 20161020
//					{
//						gSensorStatus.bits.Sensor_Obstacle_active = 0;
//					}

					//Added on 17 Dec by Yogesh to support wireless functionalities
					if (gKeysStatus.bits.Wireless_Open_pressed)
					{
						gKeysStatus.bits.Wireless_Open_pressed = 0;
					}

					//Added on 17 Dec by Yogesh to support wireless functionalities
					if (gSensorStatus.bits.Sensor_Wireless_1PBS_active)
					{
						gSensorStatus.bits.Sensor_Wireless_1PBS_active = 0;
					}

				} //if (gstLStoCMDr.commandToDriveBoard.bits.openShutter || gstLStoCMDr.commandToDriveBoard.bits.openShutterApperture)
				else if (gstLStoCMDr.commandToDriveBoard.bits.closeShutter ||
						 gstLStoCMDr.commandToDriveBoard.bits.closeShutterApperture ||
						 gstLStoCMDr.commandToDriveBoard.bits.closeShutterJog)
				{

					if (gKeysStatus.bits.Key_Close_pressed)
					{
						gKeysStatus.bits.Key_Close_pressed = 0;
					}

					if (gSensorStatus.bits.Sensor_1PBS_active)
					{
						gSensorStatus.bits.Sensor_1PBS_active = 0;
					}

					//Added on 17 Dec by Yogesh to support wireless functionalities
					if (gKeysStatus.bits.Wireless_Close_pressed)
					{
						gKeysStatus.bits.Wireless_Close_pressed = 0;
					}

					//Added on 17 Dec by Yogesh to support wireless functionalities
					if (gSensorStatus.bits.Sensor_Wireless_1PBS_active)
					{
						gSensorStatus.bits.Sensor_Wireless_1PBS_active = 0;
					}

				} //else if (gstLStoCMDr.commandToDriveBoard.bits.closeShutter || gstLStoCMDr.commandToDriveBoard.bits.closeShutterApperture || gstLStoCMDr.commandToDriveBoard.bits.closeShutterJog)
				else if (gstLStoCMDr.commandToDriveBoard.bits.stopShutter)
				{

					if (gKeysStatus.bits.Key_Stop_pressed)
					{
						gKeysStatus.bits.Key_Stop_pressed = 0;
					}

					//Added on 17 Dec by Yogesh to support wireless functionalities
					if (gKeysStatus.bits.Wireless_Stop_pressed)
					{
						gKeysStatus.bits.Wireless_Stop_pressed = 0;
					}

				} //else if (gstLStoCMDr.commandToDriveBoard.bits.stopShutter)

				// Release CMDr Block
				gstLStoCMDr.commandRequestStatus = eINACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;
				gstLStoCMDr.commandResponseStatus = eNO_STATUS;

				// Reset flag which indicate command sent by "Handle Keys and Startup"
				seHandleKeysStartupState = CmdNotSent;

			} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

		} // handle the response for command sent through 'HandleKeysStartup'
		  // If Shutter System state is Stop, then do reject all key evenets
		else if (
				 (gstControlBoardStatus.bits.runStop == 0) ||
				 (gstDriveApplicationFault.bits.emergencyStop == 1) ||
				 (ValidateInterlockInput() == 0) ||
				 (gstLStoCMDr.commandRequestStatus != eINACTIVE) ||
				 // 20 Oct startup sensor event reject in case of manual mode
				 (gSensorStatus.bits.Sensor_Obstacle_active && gstControlBoardStatus.bits.autoManual == 0)	||
				 (gSensorStatus.bits.Sensor_Obstacle_inactive && gstControlBoardStatus.bits.autoManual == 0) ||
				 //
				 //	If an operational command is received from display board during fatal error condition then discard it.
				 //	Added - Jan 2016
				 //
				 (gstDriveStatus.bits.driveFaultUnrecoverable == 1) ||
				 (gstControlBoardStatus.bits.controlFaultUnrecoverable == 1) ||
				 //
				 //	Power fail error category changed to recoverable. Do not process operational commands
				 //	when power fail error is present. Added - Jan 2016.
				 //
				 (gstDriveApplicationFault.bits.powerFail == 1)
				)
		{

			// Reject key event....
			if (gstCMDitoLS.commandDisplayBoardLS.bits.openPressed)
			{
				gstCMDitoLS.commandResponseStatus = eSUCCESS;
				gstCMDitoLS.acknowledgementReceived = eNACK;
			}

			if (gstCMDitoLS.commandDisplayBoardLS.bits.closePressed)
			{
				gstCMDitoLS.commandResponseStatus = eSUCCESS;
				gstCMDitoLS.acknowledgementReceived = eNACK;
			}

			if (gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed)
			{
				gstCMDitoLS.commandResponseStatus = eSUCCESS;
				gstCMDitoLS.acknowledgementReceived = eNACK;
			}

			if (gKeysStatus.bits.Key_Open_pressed)
			{
				gKeysStatus.bits.Key_Open_pressed = 0;
			}

			if (gKeysStatus.bits.Key_Close_pressed)
			{
				gKeysStatus.bits.Key_Close_pressed = 0;
			}

			if (gKeysStatus.bits.Key_Stop_pressed)
			{
				gKeysStatus.bits.Key_Stop_pressed = 0;
			}

			if (gSensorStatus.bits.Sensor_1PBS_active)
			{
				gSensorStatus.bits.Sensor_1PBS_active = 0;
			}
			//	Removed to service startup sensor in manual mode so that if startup sensor is active during
			//	manual mode and user changes shutter mode to auto, then shutter will work as per startup sensor
			//	functionality - Jan 2016
/*
			if (gSensorStatus.bits.Sensor_Obstacle_active)
			{
				gSensorStatus.bits.Sensor_Obstacle_active = 0;
			}
*/
			if (gstCMDitoLS.commandDisplayBoardLS.bits.openReleased)
			{
				sucOpenKeyDisplay = 0;
				gstCMDitoLS.commandResponseStatus = eSUCCESS;
				gstCMDitoLS.acknowledgementReceived = eNACK;
			}

			if (gstCMDitoLS.commandDisplayBoardLS.bits.closeReleased)
			{

				sucCloseKeyDisplay = 0;
				gstCMDitoLS.commandResponseStatus = eSUCCESS;
				gstCMDitoLS.acknowledgementReceived = eNACK;

			}

			if (gstCMDitoLS.commandDisplayBoardLS.bits.stopReleased)
			{

				sucStopKeyDisplay = 0;
				gstCMDitoLS.commandResponseStatus = eSUCCESS;
				gstCMDitoLS.acknowledgementReceived = eNACK;

			}

			if (gKeysStatus.bits.Key_Open_released)
			{
				sucOpenKeyControl = 0;
				gKeysStatus.bits.Key_Open_released = 0;
			}

			if (gKeysStatus.bits.Key_Close_released)
			{
				sucCloseKeyControl = 0;
				gKeysStatus.bits.Key_Close_released = 0;

			}

			if (gKeysStatus.bits.Key_Stop_released)
			{
				sucStopKeyControl = 0;
				gKeysStatus.bits.Key_Stop_released = 0;

			}

			if (gSensorStatus.bits.Sensor_1PBS_inactive)
			{
				suc1PBSControl = 0;
				gSensorStatus.bits.Sensor_1PBS_inactive = 0;
			}

			if (gSensorStatus.bits.Sensor_Obstacle_inactive)
			{
				sucStartupControl = 0;
				gSensorStatus.bits.Sensor_Obstacle_inactive = 0;
			}

			//Added on 17 Dec by Yogesh to support wireless functionalities
			if (gKeysStatus.bits.Wireless_Open_released)
			{
				sucWirelessOpenKeyControl = 0;
				gKeysStatus.bits.Wireless_Open_released = 0;
			}

			if (gKeysStatus.bits.Wireless_Close_released)
			{
				sucWirelessCloseKeyControl = 0;
				gKeysStatus.bits.Wireless_Close_released = 0;

			}

			if (gKeysStatus.bits.Wireless_Stop_released)
			{
				sucWirelessStopKeyControl = 0;
				gKeysStatus.bits.Wireless_Stop_released = 0;

			}

			if (gSensorStatus.bits.Sensor_Wireless_1PBS_inactive)
			{
				sucWireless1PBSControl = 0;
				gSensorStatus.bits.Sensor_Wireless_1PBS_inactive = 0;
			}


		} //else if (gstControlBoardStatus.bits.runStop == 0)
		//	Added to handle display screen hang issue at "INSTALLATION SUCCESSFUL" when a key is pressed during installation success - Dec 2015
		else if(
				(gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandResponseStatus == eNO_STATUS) &&
				(
						gstCMDitoLS.commandDisplayBoardLS.bits.upPressed ||
						gstCMDitoLS.commandDisplayBoardLS.bits.upReleased ||
						gstCMDitoLS.commandDisplayBoardLS.bits.downPressed ||
						gstCMDitoLS.commandDisplayBoardLS.bits.downReleased ||
						gstCMDitoLS.commandDisplayBoardLS.bits.enterPressed ||
						gstCMDitoLS.commandDisplayBoardLS.bits.enterReleased
				)
		)
		{

		gstCMDitoLS.commandResponseStatus = eSUCCESS;
		gstCMDitoLS.acknowledgementReceived = eNACK;

		}

		// *********************************************************************************************
		// End sub-state 'Handle Keys and Startup' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		// *********************************************************************************************
		// Start sub-state 'Handle Upper Limit Stoppage Time' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		// Validate the the exact situation to start "Upper Limit Stoppage Time"
		if (

				(seHandleUpperLimitStopTimeState == HandleUpperLimitStopTimeInit) &&

				// Check system is in Auto Mode
				(gstControlBoardStatus.bits.autoManual == 1) &&

				// Check shutter is at upper limit
				//static unsigned char sucLastOpenCommandType = 0; // 0 == Open command,1 == Open Aperture height command
				(
						(gstDriveStatus.bits.shutterUpperLimit == 1 /*&& sucLastOpenCommandType == 0*/) ||    //20160913   bug_No。108
						(gstDriveStatus.bits.shutterApertureHeight == 1 && sucLastOpenCommandType == 1)
				) &&

				// Check all keys at released state
				(sucOpenKeyDisplay == 0 && sucOpenKeyControl == 0 &&
				 sucCloseKeyDisplay == 0 && sucCloseKeyControl == 0 &&
				 sucStopKeyDisplay == 0 && sucStopKeyControl == 0 &&
				  sucStartupControl == 0) &&

				// Check Going down delay is not in progress after detecting the Close key/command
				(seShutterOpenCloseCmdState == CmdNotDetected) &&

				// Check shutter is in RUN state
				(gstControlBoardStatus.bits.runStop == 1) &&

				// Check for Safety Signal Not Trigger
				(gstDriveApplicationFault.bits.microSwitch == 0 && gstDriveApplicationFault.bits.peObstacle == 0 && gstControlApplicationFault.bits.startupSafetySensor == 0) &&

				//Check for Emergency Signal Not Trigger
				(gstDriveApplicationFault.bits.emergencyStop == 0) &&

				// Check any fault is not active on drive board
				((gstDriveStatus.bits.driveFault == 0) || (gstDriveStatus.bits.driveFaultUnrecoverable == 0)) &&

				// Check any fault is not active on control board
				((gstControlBoardStatus.bits.controlFault == 0) || (gstControlBoardStatus.bits.controlFaultUnrecoverable == 0)) &&

				// 23 Oct Check Interlock invalid OR interlock valid with priority OR interlock valid with non priority and interlock input is active (other shutter at lower limit)
				(
					(gu8_intlck_valid == 1) /*Interlock input invalid*/ 								||

					(gu8_intlck_valid == 0 && gu8_intlck_prior == 0) /*interlock valid with priority*/	||

					(
							(gu8_intlck_valid == 0) /*Interlock input valid*/ &&
							(gu8_intlck_prior == 1) /*Interlock Priority*/ &&
							(gSensorStatus.bits.Sensor_InterlockIP_active == true)
					)
				) &&
				//	Added this check to "reset stop at upper limit functionality while we are in settings mode" -RN - Dec 2015
				(guiSettingsModeStatus == 0)

		)
		{

			suiTimeStampForOnePBS = g_ui32TickCount;
			seHandleUpperLimitStopTimeState = UpperLimitStopTimeStarted;

		} // Validate the the exact situation to start "Upper Limit Stoppage Time"
		  // Keep on monitoring the condition to stop the "Upper Limit Stoppage Time"
		else if (
					(seHandleUpperLimitStopTimeState == UpperLimitStopTimeStarted) &&

					(

					// Check system is not in Auto Mode
					(gstControlBoardStatus.bits.autoManual == 0) ||

					// Check shutter is not at upper limit
					(gstDriveStatus.bits.shutterUpperLimit == 0 && gstDriveStatus.bits.shutterApertureHeight == 0) ||

					// Check any one keys at not released state
					(sucOpenKeyDisplay != 0 || sucOpenKeyControl != 0 ||
					 sucCloseKeyDisplay != 0 || sucCloseKeyControl != 0 ||
					 sucStopKeyDisplay != 0 || sucStopKeyControl != 0 ||
					 OnePBSEnabled == true || sucStartupControl != 0) ||

					 ((gSensorStatus.bits.Sensor_Obstacle_active==1)&&(gstControlBoardStatus.bits.autoManual == 1))||   //add 20161021

					// Check Going down delay is in progress after detecting the Close key/command
					(seShutterOpenCloseCmdState != CmdNotDetected) ||

					// Check shutter is not in RUN state
					(gstControlBoardStatus.bits.runStop != 1) ||

					// 23 Oct If any one of the safety signal trigger during upper stoppage time count down, then it is require reset the timer
					(gstDriveApplicationFault.bits.microSwitch == 1 || gstDriveApplicationFault.bits.peObstacle == 1 || gstControlApplicationFault.bits.startupSafetySensor == 1) ||

					// 23 Oct Check interlock valid with priority and interlock input is inactive (other shutter not at lower limit)
						(
								(gu8_intlck_valid == 0) /*Interlock input valid*/ &&
								(gu8_intlck_prior == 1) /*Interlock Priority*/ &&
								(gSensorStatus.bits.Sensor_InterlockIP_active == false)
						) ||

						//	Added this check to "reset stop at upper limit functionality while we are in settings mode" -RN - Dec 2015
						(guiSettingsModeStatus == 1)

					)
				)
		{
			OnePBSEnabled = false;
			seHandleUpperLimitStopTimeState = HandleUpperLimitStopTimeInit;

		} // Keep on monitoring the condition to stop the "Upper Limit Stoppage Time"
		  // Wait till "Upper Limit Stoppage Time" to expired
		else if (
					(seHandleUpperLimitStopTimeState == UpperLimitStopTimeStarted ) &&
					//	Add gu8_godn_oprdelay to gu8_upplim_stptime before shutter go down command - Jan 2016
					//(get_timego(suiTimeStampForOnePBS) > ((gu8_upplim_stptime  + gu8_godn_oprdelay)* 1000)) &&
					(get_timego(suiTimeStampForOnePBS) > (((uint32_t)gu8_upplim_stptime  + (uint32_t)gu8_godn_oprdelay)* 1000)) &&    //20161206_1
					(gstLStoCMDr.commandRequestStatus == eINACTIVE))
		{

			gstLStoCMDr.commandRequestStatus = eACTIVE;
			gstLStoCMDr.commandToDriveBoard.val = 0;

			if (gstDriveStatus.bits.shutterUpperLimit == 1)
			{
				gstLStoCMDr.commandToDriveBoard.bits.closeShutter = 1;
				OpenCmdForDistinguish = 0;
			} else
			{
				gstLStoCMDr.commandToDriveBoard.bits.closeShutterApperture = 1;
				OpenCmdForDistinguish = 0;
			}

			//seHandleKeysStartupState = CmdSentWaitingForReply;

			// Update last command sent
			sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

			seHandleUpperLimitStopTimeState = UpperLimitStopTimeExpiredDnCmdSentWaitingReply;

		} // Wait till "Upper Limit Stoppage Time" to expired
		  // handle the response for command sent through 'Handle Upper Limit Stoppage Time'
		else if (gstLStoCMDr.commandRequestStatus == eACTIVE && seHandleUpperLimitStopTimeState == UpperLimitStopTimeExpiredDnCmdSentWaitingReply)
		{

			if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
			{

				if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				{
					OnePBSEnableStatus = false;
					seHandleUpperLimitStopTimeState = WaitingShutterMoveUpperPos;

				} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
				{

					seHandleUpperLimitStopTimeState = HandleUpperLimitStopTimeInit;

				} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

				// Release CMDr Block
				gstLStoCMDr.commandRequestStatus = eINACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;
				gstLStoCMDr.commandResponseStatus = eNO_STATUS;

			} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

		} // handle the response for command sent through 'HandleKeysStartup'
		  // Waiting for shutter to move from upper limit
		else if (seHandleUpperLimitStopTimeState == WaitingShutterMoveUpperPos)
		{

			if (gstDriveStatus.bits.shutterUpperLimit == 0)
			{

				seHandleUpperLimitStopTimeState = HandleUpperLimitStopTimeInit;

			}

			// 23 Oct
			// Microswitch and Photoelectric error set only after close shutter command received from control board
			// If close shutter command received from control board due to upper stoppage timer elapsed and Microswitch or Photoelectric error present,
			// then shutter will not come down even after error removed
			// To avoid same state need to reset after if any one of the error present and shutter is at upper limit
			if (
					(gstDriveStatus.bits.shutterUpperLimit == 1) &&
					((gstDriveApplicationFault.bits.microSwitch == 1) || (gstDriveApplicationFault.bits.peObstacle == 1))

			   )
			{

				seHandleUpperLimitStopTimeState = HandleUpperLimitStopTimeInit;

			}


		} //// Waiting for shutter to move from upper limit

		// *********************************************************************************************
		// End sub-state 'Handle Upper Limit Stoppage Time' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		// *********************************************************************************************
		// Start sub-state 'Handle Safety Signal- Drive board' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		// Keep on monitoring the Safety Signal Triggered situation
		//20160906 item 104
					if (

									(season_cyw == 1) &&

									// Check for Safety Signal Trigger
									(
											gstDriveApplicationFault.bits.microSwitch == 1 								||
											gstDriveApplicationFault.bits.peObstacle == 1  								||

											// Check added for start sensor not to work below PE limit similar to PE Sensor
											// 8 April 2015 - YPG
											(
													gstControlApplicationFault.bits.startupSafetySensor == 1 		&&

														(
																gstDriveStatus.bits.ignPhotoElecSensLimRchd == 0 &&
																gstDriveStatus.bits.shutterLowerLimit == 0
														)

											)																			||
											gstDriveApplicationFault.bits.airSwitch == 1
									) &&

									// Check shutter is not at upper limit
									// Commented and replaced with below statement by YPG on 1 Sep 14 as shutter needs to be open till Upper Limit
									//(gstDriveStatus.bits.shutterUpperLimit != 1 && gstDriveStatus.bits.shutterApertureHeight != 1) &&
									(gstDriveStatus.bits.shutterUpperLimit != 1) &&

									// Check shutter is moving not down
									// ov  2014 - Shutter moving down check bypass for startup safety by YPG
									((gstDriveStatus.bits.shutterMovingDown == 0) || (gstControlApplicationFault.bits.startupSafetySensor == 1)) &&

									// Check shutter is in RUN state
									(gstControlBoardStatus.bits.runStop == 1) &&

									// Check LS to CMDr block is Free
									(gstLStoCMDr.commandRequestStatus == eINACTIVE) &&

									//Check for Emergency Signal Not Trigger
									(gstDriveApplicationFault.bits.emergencyStop == 0)

									//Check other fault bits are reset

									)

				{
					season_cyw = 0;
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;

					gstLStoCMDr.commandToDriveBoard.bits.openShutterJog = 1;
					gstLStoCMDr.additionalCommandData = 50;
					OpenCmdForDistinguish = 0;

										// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

					seHandleSafetySignal = HandleSafetySignalWaitingReply;
				}

		if (

				(seHandleSafetySignal == HandleSafetySignalInit) &&

				// Check for Safety Signal Trigger
				(
						gstDriveApplicationFault.bits.microSwitch == 1 								||
						gstDriveApplicationFault.bits.peObstacle == 1  								||

						// Check added for start sensor not to work below PE limit similar to PE Sensor
						// 8 April 2015 - YPG
						(
								gstControlApplicationFault.bits.startupSafetySensor == 1 		&&

									(
											gstDriveStatus.bits.ignPhotoElecSensLimRchd == 0 &&
											gstDriveStatus.bits.shutterLowerLimit == 0
									)

						)																			||
						gstDriveApplicationFault.bits.airSwitch == 1
				) &&

				// Check shutter is not at upper limit
				// Commented and replaced with below statement by YPG on 1 Sep 14 as shutter needs to be open till Upper Limit
				//(gstDriveStatus.bits.shutterUpperLimit != 1 && gstDriveStatus.bits.shutterApertureHeight != 1) &&
				(gstDriveStatus.bits.shutterUpperLimit != 1) &&

				// Check shutter is moving not down
				//�ov� 2014 - Shutter moving down check bypass for startup safety by YPG
				((gstDriveStatus.bits.shutterMovingDown == 0) || (gstControlApplicationFault.bits.startupSafetySensor == 1)) &&

				// Check shutter is in RUN state
				(gstControlBoardStatus.bits.runStop == 1) &&

				// Check LS to CMDr block is Free
				(gstLStoCMDr.commandRequestStatus == eINACTIVE) &&

				//Check for Emergency Signal Not Trigger
				(gstDriveApplicationFault.bits.emergencyStop == 0)

				//Check other fault bits are reset

				)
		{
				if(//sstLStoCMDrCmdSent.commandToDriveBoard.val == 0x000F0000 &&//last cmd is stopshutter
					gstDriveStatus.bits.shutterStopped && gstDriveStatus.bits.shutterLowerLimit == 0 &&
							gstDriveStatus.bits.shutterUpperLimit == 0 )
				{
					if(UpEnabled)
					{
						gstLStoCMDr.commandRequestStatus = eACTIVE;
						gstLStoCMDr.commandToDriveBoard.val = 0;

						gstLStoCMDr.commandToDriveBoard.bits.openShutterJog = 1;
						gstLStoCMDr.additionalCommandData = 50;
						OpenCmdForDistinguish = 0;

						// Update last command sent
						sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;
						seHandleSafetySignal = HandleSafetySignalWaitingReply;
					}
				}
				else if(gstDriveStatus.bits.shutterMovingDown)
				{
					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;

					gstLStoCMDr.commandToDriveBoard.bits.openShutterJog = 1;
					gstLStoCMDr.additionalCommandData = 50;
					OpenCmdForDistinguish = 0;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

					seHandleSafetySignal = HandleSafetySignalWaitingReply;
				}

		} // Keep on monitoring the Safety Signal Triggered situation
		  // Handle the response for command sent through 'Handle Safety Signal'
		else if (gstLStoCMDr.commandRequestStatus == eACTIVE && seHandleSafetySignal == HandleSafetySignalWaitingReply)
		{

			if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
			{

				if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				{

					seHandleSafetySignal = HandleSafetySignalWaitingShutterReachUpperLimit;

				} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
				{

					seHandleSafetySignal = HandleSafetySignalInit;

				} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

				// Release CMDr Block
				gstLStoCMDr.commandRequestStatus = eINACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;
				gstLStoCMDr.commandResponseStatus = eNO_STATUS;

			} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

		} // Handle the response for command sent through 'Handle Safety Signal'
		  // Waiting for shutter to reach upper limit
		else if (seHandleSafetySignal == HandleSafetySignalWaitingShutterReachUpperLimit)
		{

			if (gstDriveStatus.bits.shutterUpperLimit == 1)
			{

				//seHandleSafetySignal = HandleSafetySignalInit;
				seHandleSafetySignal = HandleSafetySignalWaitingSafetySigToReset;

			}
			if(gstDriveStatus.bits.shutterMovingDown==1)//20160906 item for 79
			{
				seHandleSafetySignal = HandleSafetySignalWaitingSafetySigToReset;
			}

		} // Waiting for shutter to reach upper limit
		else if (seHandleSafetySignal == HandleSafetySignalWaitingSafetySigToReset)
		{

			if (gstDriveApplicationFault.bits.microSwitch == 0 && gstDriveApplicationFault.bits.peObstacle == 0 && gstControlApplicationFault.bits.startupSafetySensor == 0 && gstDriveApplicationFault.bits.airSwitch == 0)
			{

				seHandleSafetySignal = HandleSafetySignalInit;

			}

		}

		// *********************************************************************************************
		// End sub-state 'Handle Safety Signal' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************


		/*

				// *********************************************************************************************
				// Variable for sub-state 'Handle Interlock Input' of 'Logic_Solver_Drive_Run'
				// *********************************************************************************************
				enum HandleInterlockInput {

				HandleInterlockInputInit = 0,
				HandleInterlockWaitingReply,
				// Interlock input signal de-activated under following condition
				// i)  Interlock input is Valid
				// ii) Non priority set
				// iii) shutter is moving
				// and open shutter stop command sent and waiting for reply
				HandleInterlockWaitingShutterToStop,
				HandleInterlockWaitingInterlockInputToReset

				};
				static enum HandleInterlockInput seHandleInterlockInput = HandleInterlockInputInit;

		 */


		// *********************************************************************************************
		// Start sub-state 'Handle Interlock Input Signal' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		// Reset the flag which indicate either Up or Down  and Go UP or Go Down delay is in progress
		if (
			(gSensorStatus.bits.Sensor_InterlockIP_active == false)&&
			((seShutterOpenCloseCmdState == CmdUpDetectedWaitUpDelay)||(seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay))&&
			((gu8_intlck_prior == 1)||((gu8_intlck_prior==0)&&(gstDriveStatus.bits.shutterLowerLimit == 1)))
			)
			seShutterOpenCloseCmdState = CmdNotDetected;      //20170330_1
		// Keep on monitoring the Interlock Input Signal De-activated when shutter is moving and Interlock is valid and Non Priority set
		if (

				(seHandleInterlockInput == HandleInterlockInputInit) &&

				// Check shutter is moving
				(gstDriveStatus.bits.shutterStopped == 0) &&

				// Check shutter is in RUN state
				(gstControlBoardStatus.bits.runStop == 1) &&

				// Check LS to CMDr block is Free
				(gstLStoCMDr.commandRequestStatus == eINACTIVE) &&

				//Check for Emergency Signal Not Trigger
				(gstDriveApplicationFault.bits.emergencyStop == 0) &&


				(
						(gu8_intlck_valid == 0) /*Interlock input valid*/ &&
						(gu8_intlck_prior == 1) /*Interlock Priority*/ &&
						(gSensorStatus.bits.Sensor_InterlockIP_active == false)
				)

			)
		{

			gstLStoCMDr.commandRequestStatus = eACTIVE;
			gstLStoCMDr.commandToDriveBoard.val = 0;

			gstLStoCMDr.commandToDriveBoard.bits.stopShutter = 1;
			OpenCmdForDistinguish = 0;

			// Update last command sent
			sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

			seHandleInterlockInput = HandleInterlockWaitingReply;


		} // Keep on monitoring the Safety Signal Triggered situation
		  // Handle the response for command sent through 'Handle Safety Signal'
		else if (gstLStoCMDr.commandRequestStatus == eACTIVE && seHandleInterlockInput == HandleInterlockWaitingReply)
		{

			if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
			{

				if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				{

					seHandleInterlockInput = HandleInterlockWaitingShutterToStop;

				} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
				{

					seHandleInterlockInput = HandleInterlockInputInit;

				} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

				// Release CMDr Block
				gstLStoCMDr.commandRequestStatus = eINACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;
				gstLStoCMDr.commandResponseStatus = eNO_STATUS;

			} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

		} // Handle the response for command sent through 'Handle Safety Signal'
		  // Waiting for shutter to reach upper limit
		else if (seHandleInterlockInput == HandleInterlockWaitingShutterToStop)
		{

			if (gstDriveStatus.bits.shutterStopped == 1)
			{

				seHandleInterlockInput = HandleInterlockInputInit;

			}

		} // Waiting for shutter to reach upper limit
		else if (seHandleInterlockInput == HandleInterlockWaitingInterlockInputToReset)
		{


		}

		// *********************************************************************************************
		// End sub-state 'Handle Interlock Input' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************


		// *********************************************************************************************
		// Start sub-state 'Handle Auto/Manual Run/Stop' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		// Monitor Auto/Manual Run/Stop event
		if (

				(seHandleAutoManualRunStop == HandleAutoManualRunStopInit) &&
				(
						// Check for Auto Manual mode select from Display board, Run /Stop Mode selected by Display board
						(
								(
										gstCMDitoLS.commandRequestStatus == eACTIVE	&&
										gstCMDitoLS.commandResponseStatus == eNO_STATUS
								) &&
								(
										gstCMDitoLS.commandDisplayBoardLS.bits.autoManSel == 1 		||
										gstCMDitoLS.commandDisplayBoardLS.bits.runControlBoard == 1 ||
										gstCMDitoLS.commandDisplayBoardLS.bits.stopControlBoard == 1
								)
						) ||
						// Check for Fix Auto or Manual mode selected by the Menu A006 - Auto Manual Fixing
						(
								(
										gu8_modefix == 1 /*Only fix mode is Auto */ &&
										gstControlBoardStatus.bits.autoManual == 0
								) ||
								(gu8_modefix == 2 /*Only fix mode is Manual */ && gstControlBoardStatus.bits.autoManual == 1)
						) ||
						// check set parameter command initiated by User interface and system mode is Run
						(gucSetParameterCommandFlag == 1 && gstControlBoardStatus.bits.runStop == 1)
				) &&

				// To set Auto/Manual Run/Stop mode using DH, no dedicated intertask interface available for logic solver
				// For same Logic Solver will use the CMDI --> DH module
				(gstCMDitoDH.commandRequestStatus == eINACTIVE))
		{

			seHandleAutoManualRunStop = HandleAutoManualRunStopCmdSentToDH_WaitingReply;

			gstCMDitoDH.commandRequestStatus = eACTIVE;
			gstCMDitoDH.commandDisplayBoardDH.bits.setParameter = 1;

			// Check for Auto Manual mode select from Display board
			if (gstCMDitoLS.commandDisplayBoardLS.bits.autoManSel == 1)
			{

				gstCMDitoDH.commandDataCMDiDH.parameterNumber = 601;

				// Check for Manual mode
				if (gstControlBoardStatus.bits.autoManual == 0)
				{

					// Set Auto mode
					gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 1;

				} else
				{

					// Set Manual mode
					gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 0;

				}

			}
			// Check for Fix Auto or Manual mode selected by the Menu A006 - Auto Manual Fixing
			else if (gu8_modefix == 1 /*Only fix mode is Auto */ || gu8_modefix == 2 /*Only fix mode is Manual */)
			{

				gstCMDitoDH.commandDataCMDiDH.parameterNumber = 601;

				// Check for Only fix mode is Auto mode
				if (gu8_modefix == 1 /*Only fix mode is Auto */)
				{

					// Set Auto mode
					gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 1;

				} else  // Check for Only fix mode is Manual mode
				{

					// Set Manual mode
					gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 0;

				}

			} else if (gstCMDitoLS.commandDisplayBoardLS.bits.runControlBoard == 1)
			{
				gstCMDitoDH.commandDataCMDiDH.parameterNumber = 602;
				gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 1;

			} else if (gstCMDitoLS.commandDisplayBoardLS.bits.stopControlBoard == 1 || gucSetParameterCommandFlag == 1)
			{
				gstCMDitoDH.commandDataCMDiDH.parameterNumber = 602;
				gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 0;

			}

		} // Monitor Auto/Manual Run/Stop event
		// Handle reply for command sent for Auto/Manual Run/Stop event to DH using CMDi --> DH block
		else if (gstCMDitoDH.commandRequestStatus == eACTIVE &&
				seHandleAutoManualRunStop == HandleAutoManualRunStopCmdSentToDH_WaitingReply)
		{

			if (gstCMDitoDH.commandResponseStatus == eSUCCESS
					|| gstCMDitoDH.commandResponseStatus == eTIME_OUT
					|| gstCMDitoDH.commandResponseStatus == eFAIL)
			{

				seHandleAutoManualRunStop = HandleAutoManualRunStopInit;

				// Check for Auto Manual mode select from Display board, Run /Stop Mode selected by Display board
				if (gstCMDitoLS.commandRequestStatus == eACTIVE)
				{

					// Send reply to Display board
					gstCMDitoLS.commandResponseStatus = eSUCCESS;

					if (gstCMDitoDH.commandResponseStatus == eSUCCESS)
					{

						gstCMDitoLS.acknowledgementReceived = eACK;

						if (gstCMDitoLS.commandDisplayBoardLS.bits.autoManSel == 1)
						{

							// Check for Manual mode
							if (gstControlBoardStatus.bits.autoManual == 0)
							{

								// Set Auto mode
								gstControlBoardStatus.bits.autoManual = 1;

								gSensorStatus.bits.autoManual_switch_first=1;    //add 20161026

								gstDriveStatusMenu.bits.Auto_Mode_Status = 1;
								gstDriveStatusMenu.bits.Manual_Mode_Status = 0;

							} else
							{
								// Set Manual mode
								gstControlBoardStatus.bits.autoManual = 0;

								gSensorStatus.bits.Sensor_Obstacle_inactive=0;     //add 20161026
								gSensorStatus.bits.Sensor_Obstacle_active=0;
								sucStartupControl = 0;

								gstDriveStatusMenu.bits.Auto_Mode_Status = 0;
								gstDriveStatusMenu.bits.Manual_Mode_Status = 1;

							}

						} else if (gstCMDitoLS.commandDisplayBoardLS.bits.runControlBoard == 1)
						{
							gstControlBoardStatus.bits.runStop = 1;
						} else if (gstCMDitoLS.commandDisplayBoardLS.bits.stopControlBoard == 1)
						{
							gstControlBoardStatus.bits.runStop = 0;
						}

					} //if (gstCMDitoDH.commandResponseStatus == eSUCCESS)
					else if (gstCMDitoDH.commandResponseStatus == eTIME_OUT
							|| gstCMDitoDH.commandResponseStatus == eFAIL)
					{

						gstCMDitoLS.acknowledgementReceived = eNACK;

					} //else if (gstCMDitoDH.commandResponseStatus == eTIME_OUT || gstCMDitoDH.commandResponseStatus == eFAIL)

				} // if (gstCMDitoLS.commandRequestStatus == eACTIVE)
				else // Check for Fix Auto or Manual mode selected by the Menu A006 - Auto Manual Fixing
				{

					// Check for Only fix mode is Auto mode
					if (gu8_modefix == 1 /*Only fix mode is Auto */)
					{

						// Set Auto mode
						gstControlBoardStatus.bits.autoManual = 1;

						gstDriveStatusMenu.bits.Auto_Mode_Status = 1;
						gstDriveStatusMenu.bits.Manual_Mode_Status = 0;

					}
					else if (gu8_modefix == 2 /*Only fix mode is Manual */) // Check for Only fix mode is Manual mode
					{

						// Set Manual mode
						gstControlBoardStatus.bits.autoManual = 0;

						gstDriveStatusMenu.bits.Auto_Mode_Status = 0;
						gstDriveStatusMenu.bits.Manual_Mode_Status = 1;

					}
#if 0
					//	Commented on 3 Nov 2014 to avoid control board going into stop mode during execution of
					//	setParameter command
					if (gucSetParameterCommandFlag == 1)
					{

						gstControlBoardStatus.bits.runStop = 0;
						gucSetParameterCommandFlag = 0;
					}
#endif

				}

				// Release CMDi--> DH Block
				gstCMDitoDH.commandRequestStatus = eINACTIVE;
				gstCMDitoDH.commandResponseStatus = eNO_STATUS;

				gstCMDitoDH.commandDisplayBoardDH.bits.setParameter = 0;
				gstCMDitoDH.commandDataCMDiDH.parameterNumber = 0;
				gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 0;

			} //if (gstCMDitoDH.commandResponseStatus == eSUCCESS || gstCMDitoDH.commandResponseStatus == eTIME_OUT || gstCMDitoDH.commandResponseStatus == eFAIL)

		} // Handle reply for command sent for Auto/Manual Run/Stop event to DH using CMDi --> DH block

		// *********************************************************************************************
		// End sub-state 'Handle Auto/Manual Run/Stop' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		// *********************************************************************************************
		// Start sub-state 'Handle Installation' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		// Keep on monitoring the Installation triggered by the User Interface
		if (
				(seHandleInstallation == HandleInstallationInit) &&

				((gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandResponseStatus == eNO_STATUS) && (gstCMDitoLS.commandDisplayBoardLS.bits.startInstallation)) &&

				// Check LS to CMDr block is Free
				(gstLStoCMDr.commandRequestStatus == eINACTIVE)

				)
		{

			gstLStoCMDr.commandRequestStatus = eACTIVE;
			gstLStoCMDr.commandToDriveBoard.val = 0;

			gstLStoCMDr.commandToDriveBoard.bits.startInstallation = 1;
			select_apertureHeight_Instalation=0;

			// Update last command sent
			sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

			seHandleInstallation = HandleInstallationWaitingReply;

		} // Keep on monitoring the Installation triggered by the User Interface
		else if (
				//(seHandleInstallation == HandleInstallationInit) &&

				((gstCMDitoLS.commandRequestStatus == eACTIVE && gstCMDitoLS.commandResponseStatus == eNO_STATUS) && (gstCMDitoLS.commandDisplayBoardLS.bits.start_apertureHeight)) &&

				// Check LS to CMDr block is Free
				(gstLStoCMDr.commandRequestStatus == eINACTIVE)

				)
		{

			gstLStoCMDr.commandRequestStatus = eACTIVE;
			gstLStoCMDr.commandToDriveBoard.val = 0;

			gstLStoCMDr.commandToDriveBoard.bits.start_apertureHeight = 1;
			select_apertureHeight_Instalation=1;

			// Update last command sent
			sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

			seHandleInstallation = HandleInstallationWaitingReply;

		}
		  // Handle the response for command sent through 'Handle Installation'
		else if (gstLStoCMDr.commandRequestStatus == eACTIVE && seHandleInstallation == HandleInstallationWaitingReply)
		{

			if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
			{

				// Send reply to Display board
				gstCMDitoLS.commandResponseStatus = eSUCCESS;

				if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				{

					seHandleInstallation = HandleInstallationWaitingShutterStateInstallation;
					gstCMDitoLS.acknowledgementReceived = eACK;

				} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
				{

					seHandleInstallation = HandleInstallationInit;
					gstCMDitoLS.acknowledgementReceived = eNACK;

				} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

				// Release CMDr Block
				gstLStoCMDr.commandRequestStatus = eINACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;
				gstLStoCMDr.commandResponseStatus = eNO_STATUS;

			} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

		} // Handle the response for command sent through 'Handle Installation'
		  // Waiting for shutter change its state to Installation
		else if (seHandleInstallation == HandleInstallationWaitingShutterStateInstallation)
		{

			if ((gstDriveStatus.bits.driveInstallation == 1)||(gstDriveStatus.bits.drive_apertureHeight == 1))
			{

				// commented on 9 Sep
				//seHandleInstallation = HandleInstallationInit;
				//eLogic_Solver_State = Logic_Solver_Drive_Instalation;

				// Added on 9 Sep
				seHandleInstallation = HandleInstallationWaitDbHandlerToFreeSendStopShutterCmdToDbHandler;

			}

		} // Waiting for shutter change its state to Installation
		  // Check for drive board directly entered into Installation
		else if ((seHandleInstallation == HandleInstallationInit) && ((gstDriveStatus.bits.driveInstallation == 1)||(gstDriveStatus.bits.drive_apertureHeight == 1)))
		{

			// commented on 9 Sep
			//eLogic_Solver_State = Logic_Solver_Drive_Instalation;

			// Added on 9 Sep
			seHandleInstallation = HandleInstallationWaitDbHandlerToFreeSendStopShutterCmdToDbHandler;

		}
		else if (seHandleInstallation == HandleInstallationWaitDbHandlerToFreeSendStopShutterCmdToDbHandler)
		{

			if (gstCMDitoDH.commandRequestStatus == eINACTIVE)
			{

				// Change "Run/Stop" status bit of Control board to "Run" using CMDi --> DH
				gstCMDitoDH.commandRequestStatus = eACTIVE;
				gstCMDitoDH.commandDisplayBoardDH.bits.setParameter = 1;
				gstCMDitoDH.commandDataCMDiDH.parameterNumber = 602;
				// Set Run
				//	Changed to run on 3 Nov 2014 to set shutter mode as RUN after installation
				gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 1;

				seHandleInstallation = HandleInstallationSentStopShutterCmdToDbHandlerWaitReply;

			}

		}
		else if (seHandleInstallation == HandleInstallationSentStopShutterCmdToDbHandlerWaitReply)
		{

			if (gstCMDitoDH.commandRequestStatus == eACTIVE)
			{

				if (
						(gstCMDitoDH.commandResponseStatus == eSUCCESS) ||
						(gstCMDitoDH.commandResponseStatus == eTIME_OUT) ||
						(gstCMDitoDH.commandResponseStatus == eFAIL)
				)
				{

					// in case of 'timeout' or 'fail' condition, lower will set the appropriate flag which will result system to display "fatal error" message on display

					// Release CMDi --> DH Block
					gstCMDitoDH.commandRequestStatus = eINACTIVE;
					gstCMDitoDH.commandResponseStatus = eNO_STATUS;

					gstCMDitoDH.commandDisplayBoardDH.bits.setParameter = 0;
					gstCMDitoDH.commandDataCMDiDH.parameterNumber = 0;
					gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 0;

					seHandleInstallation = HandleInstallationInit;

                    if(select_apertureHeight_Instalation==1)eLogic_Solver_State = Logic_Solver_Drive_apertureHeight;
                    else eLogic_Solver_State = Logic_Solver_Drive_Instalation;
#if 0
					//	Commented on 3 Nov 2014 to avoid control board going into stop mode after
					//	installation
					gstControlBoardStatus.bits.runStop = 0;
#endif

					// De-activate multi-function output and inter-lock output
					DEACTIVATE_MULTI_FUNC_OUT_1;
					DEACTIVATE_MULTI_FUNC_OUT_2;
					RELAY_OPEN;

				} //if (gstCMDitoDH.commandResponseStatus == eSUCCESS || gstCMDitoDH.commandResponseStatus == eTIME_OUT || gstCMDitoDH.commandResponseStatus == eFAIL)

			} // Handle the response from CMDi --> DH block for Set Drive Status as Stop

		}

		// *********************************************************************************************
		// End sub-state 'Handle Installation' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

#if 0
		// *********************************************************************************************
		// Start sub-state 'Handle Counter' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		if (
				(geHandleCounter == HandleCounterInitGetCounter) &&

				// Check LS to CMDr block is Free
				(gstLStoCMDr.commandRequestStatus == eINACTIVE)

			)
		{

			gstLStoCMDr.commandRequestStatus = eACTIVE;
			gstLStoCMDr.commandToDriveBoard.val = 0;

			gstLStoCMDr.commandToDriveBoard.bits.getOperationCount = 1;

			// Update last command sent
			sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

			geHandleCounter = HandleCounterGetCounterWaitingReply;

		}
		else if (gstLStoCMDr.commandRequestStatus == eACTIVE
				&& geHandleCounter == HandleCounterGetCounterWaitingReply)
		{

			if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
			{

				if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				{

					geHandleCounter = HandleCounterWaitLowerLimitGetCounter;
					gulCounterValue = gstLStoCMDr.getParameterValue;

				} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
				{

					geHandleCounter = HandleCounterInitGetCounter;

				} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

				// Release CMDr Block
				gstLStoCMDr.commandRequestStatus = eINACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;
				gstLStoCMDr.commandResponseStatus = eNO_STATUS;

			} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

		}
		// Wait shutter to reach at lower limit and send "get operation count" command
		else if ((geHandleCounter == HandleCounterWaitLowerLimitGetCounter) && (gstLStoCMDr.commandRequestStatus == eINACTIVE))
		{

			if (gstDriveStatus.bits.shutterLowerLimit == 1)
			{
				gstLStoCMDr.commandRequestStatus = eACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;

				gstLStoCMDr.commandToDriveBoard.bits.getOperationCount = 1;

				// Update last command sent
				sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

				geHandleCounter = HandleCounterLowerLimitGetCounterWaitingReply;

			}

		} else if (gstLStoCMDr.commandRequestStatus == eACTIVE && geHandleCounter == HandleCounterLowerLimitGetCounterWaitingReply)
		{

			if (gstLStoCMDr.commandResponseStatus == eSUCCESS  || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
			{

				if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				{

					geHandleCounter = HandleCounterWaitNonLowerLimit;

					if (gstLStoCMDr.getParameterValue >= gulCounterValue)
					{
					gstElectroMechanicalCounter.value = gstLStoCMDr.getParameterValue - gulCounterValue;
					}
					else
					{
						gstElectroMechanicalCounter.value = 1;
					}

					gulCounterValue = gstLStoCMDr.getParameterValue;

				} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
				else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
				{

					geHandleCounter = HandleCounterWaitLowerLimitGetCounter;

				} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

				// Release CMDr Block
				gstLStoCMDr.commandRequestStatus = eINACTIVE;
				gstLStoCMDr.commandToDriveBoard.val = 0;
				gstLStoCMDr.commandResponseStatus = eNO_STATUS;

			} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

		}
		// Waiting for shutter change its state to other than lower limit
		else if (geHandleCounter == HandleCounterWaitNonLowerLimit)
		{

			if (gstDriveStatus.bits.shutterLowerLimit == 0)
			{

				geHandleCounter = HandleCounterWaitLowerLimitGetCounter;

			}

		} // Waiting for shutter change its state to Installation

		// *********************************************************************************************
		// End sub-state 'Handle Counter' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************
#endif

		// *********************************************************************************************
		// Start sub-state 'Interlock Output' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		if (sHandleInterlockOutput == HandleInterlockOP_WaitLowerLimit)
		{

			if (gstDriveStatus.bits.shutterLowerLimit == 1)
			{
				// Start delay provide "interlock delay timer (A010)" is not equal to zero OR directly closed relay
				if (gu32_intlck_deltmr != 0)
				{
					sHandleInterlockOutput = HandleInterlockOP_DelayBeforeCloseRelay;
					suiTimeStampInterlockOutput = g_ui32TickCount;
				}
				else
				{
					sHandleInterlockOutput = HandleInterlockOP_CloseRelay;
				}
			}

		} else if (sHandleInterlockOutput == HandleInterlockOP_DelayBeforeCloseRelay)
		{

			if (get_timego( suiTimeStampInterlockOutput) >= (gu32_intlck_deltmr * 1000))
			{

				sHandleInterlockOutput = HandleInterlockOP_CloseRelay;

			}

		} else if (sHandleInterlockOutput == HandleInterlockOP_CloseRelay)
		{

			// close the relay
			RELAY_CLOSE;
			sucInterlockOutputStatus = 0;

			//
			sHandleInterlockOutput = HandleInterlockOP_WaitNonLowerLimit;

		} else if (sHandleInterlockOutput == HandleInterlockOP_WaitNonLowerLimit)
		{
			if (gstDriveStatus.bits.shutterLowerLimit == 0)
			{

#if INTERLOCK_DELAY_TIMER_FOR_CLOSE_OPEN == 1
				// Start delay provide "interlock delay timer (A010)" is not equal to zero OR directly closed relay
				if (gu32_intlck_deltmr != 0)
				{
					sHandleInterlockOutput = HandleInterlockOP_DelayBeforeOpenRelay;
					suiTimeStampInterlockOutput = g_ui32TickCount;
				} else
				{
					sHandleInterlockOutput = HandleInterlockOP_OpenRelay;
				}
#else
				sHandleInterlockOutput = HandleInterlockOP_OpenRelay;
#endif

			}
		} else if (sHandleInterlockOutput == HandleInterlockOP_DelayBeforeOpenRelay)
		{

			if (get_timego( suiTimeStampInterlockOutput) >= gu32_intlck_deltmr)
			{

				sHandleInterlockOutput = HandleInterlockOP_OpenRelay;

			}

		} else if (sHandleInterlockOutput == HandleInterlockOP_OpenRelay)
		{

			// Open the relay
			RELAY_OPEN;
			sucInterlockOutputStatus = 1;

			//
			sHandleInterlockOutput = HandleInterlockOP_WaitLowerLimit;

		}

		// *********************************************************************************************
		// End sub-state 'Interlock Output' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

#if 1
		// *********************************************************************************************
		// Start for sub-state 'Monitor continuous Operation' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************
        if(Clear_E111_FLAG == 1)
        {
        	Clear_E111_FLAG = 0;
        	sHandleContinuousOperation = HandleContinuousOpr_WaitNonStoppedStateStrtDelay;
        	gstDriveStatus.bits.shutterStopped = 0;
        }


		if (gu8_opr_rsrttmr != 0)
		{

			if (sHandleContinuousOperation == HandleContinuousOpr_WaitNonStoppedStateStrtDelay)
			{

				if (gstDriveStatus.bits.shutterStopped == 0)
				//if (gstDriveStatus.bits.shutterMovingDown == 1 || gstDriveStatus.bits.shutterMovingUp == 1)
				{
					suiTimeStampContinuousOperation = g_ui32TickCount;
					sHandleContinuousOperation = HandleContinuousOpr_MonitorStoppedStateForDelay;
					runing_a009_dir_cyw = 0;
				}

			}
			else if (sHandleContinuousOperation == HandleContinuousOpr_MonitorStoppedStateForDelay)
			{

				if (
						(gstDriveStatus.bits.shutterLowerLimit == 1) ||

						(
								(gstDriveStatus.bits.shutterUpperLimit == 1 && sucLastOpenCommandType == 0) 	||
								(gstDriveStatus.bits.shutterApertureHeight == 1 && sucLastOpenCommandType == 1)
						) 											 ||
						(gstDriveStatus.bits.driveRunTimeCalibration == 1)

				   )
				{

					sHandleContinuousOperation = HandleContinuousOpr_WaitNonStoppedStateStrtDelay;

				}
				else if (gstDriveStatus.bits.shutterStopped == 1)
				//if (gstDriveStatus.bits.shutterMovingDown == 0 && gstDriveStatus.bits.shutterMovingUp == 0)
				{

					sHandleContinuousOperation = HandleContinuousOpr_ConfirmNonStoppedState;
					suiTimeStampConfirmNonStoppedState = g_ui32TickCount;
				}
				else
				{
					if(gstDriveStatus.bits.shutterMovingDown == 1)
					{
						if(runing_a009_dir_cyw != 1)  suiTimeStampContinuousOperation = g_ui32TickCount;
						runing_a009_dir_cyw = 1;
					}
					if(gstDriveStatus.bits.shutterMovingUp == 1)
					{
						if(runing_a009_dir_cyw != 2)  suiTimeStampContinuousOperation = g_ui32TickCount;
						runing_a009_dir_cyw = 2;
					}

					if (get_timego(suiTimeStampContinuousOperation) >= (gu8_opr_rsrttmr * 1000))
					{

						sHandleContinuousOperation = HandleContinuousOpr_SendStopShutterCmd;

						// Set error flag as fatal error
						gstControlApplicationFault.bits.operationRestrictionTimer = 1;
						gstControlBoardFault.bits.controlApplication = 1;
						gstControlBoardStatus.bits.controlFault = 1;
					//	gstControlBoardStatus.bits.controlFaultUnrecoverable = 1;//cyw

					}

				}

			}
			else if (sHandleContinuousOperation == HandleContinuousOpr_ConfirmNonStoppedState)
			{

				// confirm stop condition for 1.5 second, then reset the state machine
				if (get_timego( suiTimeStampConfirmNonStoppedState) >= 1500)
				{
				sHandleContinuousOperation = HandleContinuousOpr_WaitNonStoppedStateStrtDelay;
				}
				else if (gstDriveStatus.bits.shutterStopped == 0)
				//else if (gstDriveStatus.bits.shutterMovingDown == 1 || gstDriveStatus.bits.shutterMovingUp == 1)
				{
				sHandleContinuousOperation = HandleContinuousOpr_MonitorStoppedStateForDelay;
				}

			}
			else if (sHandleContinuousOperation == HandleContinuousOpr_SendStopShutterCmd)
			{

				if (gstLStoCMDr.commandRequestStatus == eINACTIVE)
				{

					gstLStoCMDr.commandRequestStatus = eACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandToDriveBoard.bits.stopShutter = 1;
					OpenCmdForDistinguish = 0;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

					sHandleContinuousOperation = HandleContinuousOpr_SendStopShutterReply;

				}

			}
			else if (sHandleContinuousOperation == HandleContinuousOpr_SendStopShutterReply)
			{

				if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
				{

					if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
					{

						sHandleContinuousOperation = HandleContinuousOpr_WaitPowerReset;

						// Set error flag as fatal error
						gstControlApplicationFault.bits.operationRestrictionTimer = 1;
						gstControlBoardFault.bits.controlApplication = 1;
						gstControlBoardStatus.bits.controlFault = 1;
					//	gstControlBoardStatus.bits.controlFaultUnrecoverable = 1;//cyw

					} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
					else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
					{

						sHandleContinuousOperation = HandleContinuousOpr_SendStopShutterCmd;

					} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

					// Release CMDr Block
					gstLStoCMDr.commandRequestStatus = eINACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandResponseStatus = eNO_STATUS;

				} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

			}
			else if (sHandleContinuousOperation == HandleContinuousOpr_WaitPowerReset)
			{

			}

		} // if (gu8_opr_rsrttmr != 0)

		// *********************************************************************************************
		// End for sub-state 'Monitor continuous Operation' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************
#endif


#if 0
		// *********************************************************************************************
		// Start for sub-state 'Multi-Function Output' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************
		_MultifunctionOutput	gstMultifuncOutput;

		static _MultifunctionOutput	gstMultifuncOutputLastState = {0};

		static uint8_t gu8_multi_fn_out1LastState = 0;
		static uint8_t gu8_multi_fn_out2LastState = 0;

		uint8_t lucmulti_fn_out1_pin,lucmulti_fn_out2_pin;

		// Update gstMultifuncOutput with latest status
		// Upper Limit
		if (gstDriveStatus.bits.shutterUpperLimit == 1)
		{
			gstMultifuncOutput.bits.UpperLimit = 1;
		}
		else
		{
			gstMultifuncOutput.bits.UpperLimit = 0;
		}

		// Lower Limit
		if (gstDriveStatus.bits.shutterLowerLimit == 1)
		{
			gstMultifuncOutput.bits.LowerLimit = 1;
		}
		else
		{
			gstMultifuncOutput.bits.LowerLimit = 0;
		}

		// Interlock Output
		if (sucInterlockOutputStatus == 0)
		{
			gstMultifuncOutput.bits.InterlockOutput = 1;
		}
		else
		{
			gstMultifuncOutput.bits.InterlockOutput = 0;
		}

		// Operating
		if (
				(seShutterOpenCloseCmdState == CmdUpDetectedWaitUpDelay) 	 ||
				(seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay) ||
				(gstDriveStatus.bits.shutterMovingUp == 1) 					 ||
				(gstDriveStatus.bits.shutterMovingDown == 1)
		   )
		{
			gstMultifuncOutput.bits.Operating = 1;
		}
		else
		{
			gstMultifuncOutput.bits.Operating = 0;
		}


		// Rising
		if (gstDriveStatus.bits.shutterMovingUp == 1)
		{
			gstMultifuncOutput.bits.Rising = 1;
		}
		else
		{
			gstMultifuncOutput.bits.Rising = 0;
		}

		// Dropping
		if (gstDriveStatus.bits.shutterMovingDown == 1)
		{
			gstMultifuncOutput.bits.Dropping = 1;
		}
		else
		{
			gstMultifuncOutput.bits.Dropping = 0;
		}


		// Green signal lamp
		if (gstDriveStatus.bits.shutterUpperLimit == 1)
		{
			gstMultifuncOutput.bits.GreenSignalLamp = 1;
		}
		else
		{
			gstMultifuncOutput.bits.GreenSignalLamp = 0;
		}

		// Red signal lamp
		if (gstDriveStatus.bits.shutterUpperLimit == 1)
		{
			gstMultifuncOutput.bits.RedSignalLamp = 0;
		}
		else
		{
			gstMultifuncOutput.bits.RedSignalLamp = 1;
		}


		// Automatic Mode & Manual Mode
		if (gstControlBoardStatus.bits.autoManual == 1)
		{
			gstMultifuncOutput.bits.AutomaticModeOutput = 1;
			gstMultifuncOutput.bits.ManaualModeOutput = 0;
		}
		else
		{
			gstMultifuncOutput.bits.AutomaticModeOutput = 0;
			gstMultifuncOutput.bits.ManaualModeOutput = 1;
		}


		// Error Output
		if ((gstDriveStatus.bits.driveFault == 1) || (gstDriveStatus.bits.driveFaultUnrecoverable == 1) || (gstControlBoardStatus.bits.controlFault == 1) || (gstControlBoardStatus.bits.controlFaultUnrecoverable == 1))
		{
			gstMultifuncOutput.bits.ErrorOutput = 1;
		}
		else
		{
			gstMultifuncOutput.bits.ErrorOutput = 0;
		}


		// Check change in Multi Function Output Setting (071,072)
		if ((gu8_multi_fn_out1LastState != gu8_multi_fn_out1) || (gu8_multi_fn_out2LastState != gu8_multi_fn_out2))
		{

			gu8_multi_fn_out1LastState = gu8_multi_fn_out1;
			gu8_multi_fn_out2LastState = gu8_multi_fn_out2;

			gstMultifuncOutputLastState.val = 0;

		}

		// compare gstMultifuncOutput with gstMultifuncOutputLastState and update Multi-function output
		if (gstMultifuncOutput.val != gstMultifuncOutputLastState.val)
		{

			gstMultifuncOutputLastState = gstMultifuncOutput;

			// Update Multi-function output 1
			if (gu8_multi_fn_out1 == 0)
			{

				if (gstMultifuncOutput.bits.UpperLimit == 1)
				{
					lucmulti_fn_out1_pin = 1;
				}
				else
				{
					lucmulti_fn_out1_pin = 0;
				}
			}
			else if (gu8_multi_fn_out1 == 1)
			{

				if (gstMultifuncOutput.bits.LowerLimit == 1)
				{
					lucmulti_fn_out1_pin = 1;
				}
				else
				{
					lucmulti_fn_out1_pin = 0;
				}
			}
			else if (gu8_multi_fn_out1 == 2)
			{

				if (gstMultifuncOutput.bits.InterlockOutput == 1)
				{
					lucmulti_fn_out1_pin = 1;
				}
				else
				{
					lucmulti_fn_out1_pin = 0;
				}
			}
			else if (gu8_multi_fn_out1 == 3)
			{

				if (gstMultifuncOutput.bits.Operating == 1)
				{
					lucmulti_fn_out1_pin = 1;
				}
				else
				{
					lucmulti_fn_out1_pin = 0;
				}
			}
			else if (gu8_multi_fn_out1 == 4)
			{

				if (gstMultifuncOutput.bits.Rising == 1)
				{
					lucmulti_fn_out1_pin = 1;
				}
				else
				{
					lucmulti_fn_out1_pin = 0;
				}
			}
			else if (gu8_multi_fn_out1 == 5)
			{

				if (gstMultifuncOutput.bits.Dropping == 1)
				{
					lucmulti_fn_out1_pin = 1;
				}
				else
				{
					lucmulti_fn_out1_pin = 0;
				}
			}
			else if (gu8_multi_fn_out1 == 6)
			{

				if (gstMultifuncOutput.bits.GreenSignalLamp == 1)
				{
					lucmulti_fn_out1_pin = 1;
				}
				else
				{
					lucmulti_fn_out1_pin = 0;
				}
			}
			else if (gu8_multi_fn_out1 == 7)
			{

				if (gstMultifuncOutput.bits.RedSignalLamp == 1)
				{
					lucmulti_fn_out1_pin = 1;
				}
				else
				{
					lucmulti_fn_out1_pin = 0;
				}
			}
			else if (gu8_multi_fn_out1 == 8)
			{

				if (gstMultifuncOutput.bits.AutomaticModeOutput == 1)
				{
					lucmulti_fn_out1_pin = 1;
				}
				else
				{
					lucmulti_fn_out1_pin = 0;
				}
			}
			else if (gu8_multi_fn_out1 == 9)
			{

				if (gstMultifuncOutput.bits.ManaualModeOutput == 1)
				{
					lucmulti_fn_out1_pin = 1;
				}
				else
				{
					lucmulti_fn_out1_pin = 0;
				}
			}
			else if (gu8_multi_fn_out1 == 10)
			{

				if (gstMultifuncOutput.bits.ErrorOutput == 1)
				{
					lucmulti_fn_out1_pin = 1;
				}
				else
				{
					lucmulti_fn_out1_pin = 0;
				}
			}

			// Update Multi-function output 1 pin basis lucmulti_fn_out1_pin
			if(lucmulti_fn_out1_pin == 1)
			{
				ACTIVATE_MULTI_FUNC_OUT_1;
			}
			else
			{
				DEACTIVATE_MULTI_FUNC_OUT_1;
			}


			// Update Multi-function output 2
			if (gu8_multi_fn_out2 == 0)
			{

				if (gstMultifuncOutput.bits.UpperLimit == 1)
				{
					lucmulti_fn_out2_pin = 1;
				}
				else
				{
					lucmulti_fn_out2_pin = 0;
				}
			}
			else if (gu8_multi_fn_out2 == 1)
			{

				if (gstMultifuncOutput.bits.LowerLimit == 1)
				{
					lucmulti_fn_out2_pin = 1;
				}
				else
				{
					lucmulti_fn_out2_pin = 0;
				}
			}
			else if (gu8_multi_fn_out2 == 2)
			{

				if (gstMultifuncOutput.bits.InterlockOutput == 1)
				{
					lucmulti_fn_out2_pin = 1;
				}
				else
				{
					lucmulti_fn_out2_pin = 0;
				}
			}
			else if (gu8_multi_fn_out2 == 3)
			{

				if (gstMultifuncOutput.bits.Operating == 1)
				{
					lucmulti_fn_out2_pin = 1;
				}
				else
				{
					lucmulti_fn_out2_pin = 0;
				}
			}
			else if (gu8_multi_fn_out2 == 4)
			{

				if (gstMultifuncOutput.bits.Rising == 1)
				{
					lucmulti_fn_out2_pin = 1;
				}
				else
				{
					lucmulti_fn_out2_pin = 0;
				}
			}
			else if (gu8_multi_fn_out2 == 5)
			{

				if (gstMultifuncOutput.bits.Dropping == 1)
				{
					lucmulti_fn_out2_pin = 1;
				}
				else
				{
					lucmulti_fn_out2_pin = 0;
				}
			}
			else if (gu8_multi_fn_out2 == 6)
			{

				if (gstMultifuncOutput.bits.GreenSignalLamp == 1)
				{
					lucmulti_fn_out2_pin = 1;
				}
				else
				{
					lucmulti_fn_out2_pin = 0;
				}
			}
			else if (gu8_multi_fn_out2 == 7)
			{

				if (gstMultifuncOutput.bits.RedSignalLamp == 1)
				{
					lucmulti_fn_out2_pin = 1;
				}
				else
				{
					lucmulti_fn_out2_pin = 0;
				}
			}
			else if (gu8_multi_fn_out2 == 8)
			{

				if (gstMultifuncOutput.bits.AutomaticModeOutput == 1)
				{
					lucmulti_fn_out2_pin = 1;
				}
				else
				{
					lucmulti_fn_out2_pin = 0;
				}
			}
			else if (gu8_multi_fn_out2 == 9)
			{

				if (gstMultifuncOutput.bits.ManaualModeOutput == 1)
				{
					lucmulti_fn_out2_pin = 1;
				}
				else
				{
					lucmulti_fn_out2_pin = 0;
				}
			}
			else if (gu8_multi_fn_out2 == 10)
			{

				if (gstMultifuncOutput.bits.ErrorOutput == 1)
				{
					lucmulti_fn_out2_pin = 1;
				}
				else
				{
					lucmulti_fn_out2_pin = 0;
				}
			}

			// Update Multi-function output 2 pin basis lucmulti_fn_out2_pin
			if(lucmulti_fn_out2_pin == 1)
			{
				ACTIVATE_MULTI_FUNC_OUT_2;
			}
			else
			{
				DEACTIVATE_MULTI_FUNC_OUT_2;
			}

		}

		// *********************************************************************************************
		// End for sub-state 'Multi-Function Output' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************
#endif



#if 1
		// *********************************************************************************************
		// Start for sub-state 'Multi-Function Output' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		// It hold the bitwise status of various multifucntion output
		_MultifunctionOutput gstBitwiseMultifuncOutput;

		// It hold the bitwise LAST status of various multifucntion output
		static _MultifunctionOutput	gstBitwiseMultifuncOutputLastState;

		// It hold the last settting of 071 - 075 multifunction output menu
		static uint8_t gucMultiFuncOutputMenuLastState[5] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		// It hold the temporary status of every multifunction output. It at the end use to update all multifunction outputs
		uint8_t lucNewMultiFuncOutputPin[5];

		uint8_t lucNeedToSetMultifuncOutputFlag = 0;

		uint8_t lucLoop;

		static uint32_t InterlockOutput_count_cyw=relay_delay_cyw;
		static uint32_t operationing_count_cyw=relay_delay_cyw;
		static uint32_t rasing_count_cyw=relay_delay_cyw;
		static uint32_t droping_count_cyw=relay_delay_cyw;
		static uint32_t reding_count_openall_cyw=relay_delay_cyw;
		static uint32_t reding_count_openhalf_cyw=relay_delay_cyw;

		gstBitwiseMultifuncOutput.val = 0;



		// Update gstBitwiseMultifuncOutput with latest status
		// Upper Limit
		if (gstDriveStatus.bits.shutterUpperLimit == 1)
		{
			gstBitwiseMultifuncOutput.bits.UpperLimit = 1;
		}
		else
		{
			gstBitwiseMultifuncOutput.bits.UpperLimit = 0;
		}

		// Lower Limit
		if (gstDriveStatus.bits.shutterLowerLimit == 1)
		{
			gstBitwiseMultifuncOutput.bits.LowerLimit = 1;
		}
		else
		{
			gstBitwiseMultifuncOutput.bits.LowerLimit = 0;
		}

		// Interlock Output
//		if (sucInterlockOutputStatus == 0)  //20161206
//		{
//			gstBitwiseMultifuncOutput.bits.InterlockOutput = 1;
//		}
//		else
//		{
//			gstBitwiseMultifuncOutput.bits.InterlockOutput = 0;
//		}

		// Operating
		if (
									(seShutterOpenCloseCmdState == CmdUpDetectedWaitUpDelay) 	 ||
									(seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay) ||
									(seHandleUpperLimitStopTimeState == UpperLimitStopTimeStarted )||   //20170330_3
									(gstDriveStatus.bits.shutterUpperLimit == 1 && gstControlBoardStatus.bits.autoManual == 1) ||  //20170330_3
								    (gstDriveStatus.bits.shutterMovingUp == 1) 					 ||
									(gstDriveStatus.bits.shutterMovingDown == 1)                 ||
									(operationing_count_cyw <relay_delay_cyw)
			)
		    {
					if((seShutterOpenCloseCmdState == CmdUpDetectedWaitUpDelay)||(seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay))
					{
						operationing_count_cyw=0;
					}
					if((seShutterOpenCloseCmdState!=CmdUpDetectedWaitUpDelay)&&(seShutterOpenCloseCmdState != CmdDownDetectedWaitDownDelay))
					{
						if(operationing_count_cyw < relay_delay_cyw)
						    operationing_count_cyw++;
					}
					gstBitwiseMultifuncOutput.bits.Operating = 1;
		    }
			else
			{
					operationing_count_cyw=relay_delay_cyw;
					gstBitwiseMultifuncOutput.bits.Operating = 0;
			}


		gstBitwiseMultifuncOutput.bits.InterlockOutput = 0;     //20161206
		// Rising
		if ((gstDriveStatus.bits.shutterMovingUp == 1)||(seShutterOpenCloseCmdState == CmdUpDetectedWaitUpDelay)||(rasing_count_cyw<relay_delay_cyw))
				{
					gstBitwiseMultifuncOutput.bits.Rising = 1;
					if(seShutterOpenCloseCmdState == CmdUpDetectedWaitUpDelay)
					{
						rasing_count_cyw = 0;
					}
					if(seShutterOpenCloseCmdState != CmdUpDetectedWaitUpDelay)
					{
						if(rasing_count_cyw < relay_delay_cyw)
						{
							rasing_count_cyw ++;
						}
					}
				}
				else
				{
					rasing_count_cyw=relay_delay_cyw;
					gstBitwiseMultifuncOutput.bits.Rising = 0;

					//if(gstDriveStatus.bits.shutterLowerLimit == 1)gstBitwiseMultifuncOutput.bits.InterlockOutput = 1;	//20161206
					if((gstDriveStatus.bits.shutterLowerLimit == 1)&&(sucInterlockOutputStatus == 0)&&(gstDriveStatus.bits.shutterMovingDown==0))gstBitwiseMultifuncOutput.bits.InterlockOutput = 1;	//20170330
				}
		// Dropping
		if ((gstDriveStatus.bits.shutterMovingDown == 1)||(seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay)||(droping_count_cyw<relay_delay_cyw)
			 ||((seHandleUpperLimitStopTimeState == UpperLimitStopTimeStarted ) &&(get_timego(suiTimeStampForOnePBS) > ((uint32_t)gu8_upplim_stptime* 1000)))    //20161206_1
		    )
				{
					gstBitwiseMultifuncOutput.bits.Dropping = 1;
					if((seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay)
					    ||((seHandleUpperLimitStopTimeState == UpperLimitStopTimeStarted ) &&(get_timego(suiTimeStampForOnePBS) > ((uint32_t)gu8_upplim_stptime* 1000)))    //20161206_1
					  )
					{
						droping_count_cyw=0;
					}
					if(seShutterOpenCloseCmdState != CmdDownDetectedWaitDownDelay)
					{
						if(droping_count_cyw < relay_delay_cyw)
						{
							droping_count_cyw ++;
						}
					}
				}
				else
				{
					droping_count_cyw=relay_delay_cyw;
					gstBitwiseMultifuncOutput.bits.Dropping = 0;
				}


		// Green signal lamp
		if (gstDriveStatus.bits.shutterUpperLimit == 1)
		{
			gstBitwiseMultifuncOutput.bits.GreenSignalLamp = 1;
		}
		else if (gstDriveStatus.bits.shutterApertureHeight == 1 && sucLastOpenCommandType == 1)
		{
			gstBitwiseMultifuncOutput.bits.GreenSignalLamp = 1;
		}
		else if (seShutterOpenCloseCmdState == CmdUpDetectedWaitUpDelay || seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay)
		{

			gstBitwiseMultifuncOutput.bits.GreenSignalLamp = 0;

		}
		else
		{
			gstBitwiseMultifuncOutput.bits.GreenSignalLamp = 0;
		}

		// Red signal lamp
		if (gstDriveStatus.bits.shutterUpperLimit == 1)
				{
					if((seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay)||( reding_count_openall_cyw<relay_delay_cyw))
					{
						gstBitwiseMultifuncOutput.bits.RedSignalLamp = 1;
						if(seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay)
						{
							reding_count_openall_cyw = 0;
						}
						if(seShutterOpenCloseCmdState != CmdDownDetectedWaitDownDelay)
						{
							if(reding_count_openall_cyw<relay_delay_cyw)
								reding_count_openall_cyw ++;
						}
					}
					else
					{
						reding_count_openall_cyw=relay_delay_cyw;
						gstBitwiseMultifuncOutput.bits.RedSignalLamp = 0;
					}
				}
				else if (gstDriveStatus.bits.shutterApertureHeight == 1 && sucLastOpenCommandType == 1)
				{

					if((seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay)||( reding_count_openhalf_cyw<relay_delay_cyw))
					{
					    gstBitwiseMultifuncOutput.bits.RedSignalLamp = 1;
					    if(seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay)
					    {
					    	reding_count_openhalf_cyw = 0;
					    }
					    if(seShutterOpenCloseCmdState != CmdDownDetectedWaitDownDelay)
					    {
					    	if(reding_count_openhalf_cyw<relay_delay_cyw)
					    		reding_count_openhalf_cyw ++;
					    }
					}
					else
					{
						reding_count_openhalf_cyw=relay_delay_cyw;
						gstBitwiseMultifuncOutput.bits.RedSignalLamp = 0;
					}
				}
				else if (seShutterOpenCloseCmdState == CmdUpDetectedWaitUpDelay || seShutterOpenCloseCmdState == CmdDownDetectedWaitDownDelay)
				{

					gstBitwiseMultifuncOutput.bits.RedSignalLamp = 1;

				}
				else
				{
					gstBitwiseMultifuncOutput.bits.RedSignalLamp = 1;
				}


		// Automatic Mode & Manual Mode
		if (gstControlBoardStatus.bits.autoManual == 1)
		{
			gstBitwiseMultifuncOutput.bits.AutomaticModeOutput = 1;
			gstBitwiseMultifuncOutput.bits.ManaualModeOutput = 0;
		}
		else
		{
			gstBitwiseMultifuncOutput.bits.AutomaticModeOutput = 0;
			gstBitwiseMultifuncOutput.bits.ManaualModeOutput = 1;
		}


		// Error Output
		if ((gstDriveStatus.bits.driveFault == 1) || (gstDriveStatus.bits.driveFaultUnrecoverable == 1) || (gstControlBoardStatus.bits.controlFault == 1) || (gstControlBoardStatus.bits.controlFaultUnrecoverable == 1))
		{
			gstBitwiseMultifuncOutput.bits.ErrorOutput = 1;
		}
		else
		{
			gstBitwiseMultifuncOutput.bits.ErrorOutput = 0;
		}


		// Check change in Multi Function Output Setting (071,072)
		if (
				(gucMultiFuncOutputMenuLastState[0] != gu8_multi_fn_out1) ||
				(gucMultiFuncOutputMenuLastState[1] != gu8_multi_fn_out2) ||
				(gucMultiFuncOutputMenuLastState[2] != gu8_multi_fn_out3) ||
				(gucMultiFuncOutputMenuLastState[3] != gu8_multi_fn_out4) ||
				(gucMultiFuncOutputMenuLastState[4] != gu8_multi_fn_out5) ||

				(gstBitwiseMultifuncOutput.val != gstBitwiseMultifuncOutputLastState.val)
		   )
		{

			gucMultiFuncOutputMenuLastState[0] = gu8_multi_fn_out1;
			gucMultiFuncOutputMenuLastState[1] = gu8_multi_fn_out2;
			gucMultiFuncOutputMenuLastState[2] = gu8_multi_fn_out3;
			gucMultiFuncOutputMenuLastState[3] = gu8_multi_fn_out4;
			gucMultiFuncOutputMenuLastState[4] = gu8_multi_fn_out5;

			gstBitwiseMultifuncOutputLastState.val = gstBitwiseMultifuncOutput.val;

			lucNeedToSetMultifuncOutputFlag = 1;

		}

		// update Multi-function output
		if (lucNeedToSetMultifuncOutputFlag == 1)
		{

			lucNeedToSetMultifuncOutputFlag = 0;

			for (lucLoop = 0; lucLoop < 5; lucLoop++)
			{

				// Update Multi-function output 1
				if (gucMultiFuncOutputMenuLastState[lucLoop] == 0)
				{

					if (gstBitwiseMultifuncOutput.bits.UpperLimit == 1)
					{
						lucNewMultiFuncOutputPin[lucLoop] = 1;
					}
					else
					{
						lucNewMultiFuncOutputPin[lucLoop] = 0;
					}
				}
				else if (gucMultiFuncOutputMenuLastState[lucLoop] == 1)
				{

					if (gstBitwiseMultifuncOutput.bits.LowerLimit == 1)
					{
						lucNewMultiFuncOutputPin[lucLoop] = 1;
					}
					else
					{
						lucNewMultiFuncOutputPin[lucLoop] = 0;
					}
				}
				else if (gucMultiFuncOutputMenuLastState[lucLoop] == 2)
				{

					if (gstBitwiseMultifuncOutput.bits.InterlockOutput == 1)
					{
						lucNewMultiFuncOutputPin[lucLoop] = 1;
					}
					else
					{
						lucNewMultiFuncOutputPin[lucLoop] = 0;
					}
				}
				else if (gucMultiFuncOutputMenuLastState[lucLoop] == 3)
				{

					if (gstBitwiseMultifuncOutput.bits.Operating == 1)
					{
						lucNewMultiFuncOutputPin[lucLoop] = 1;
					}
					else
					{
						lucNewMultiFuncOutputPin[lucLoop] = 0;
					}
				}
				else if (gucMultiFuncOutputMenuLastState[lucLoop] == 4)
				{

					if (gstBitwiseMultifuncOutput.bits.Rising == 1)
					{
						lucNewMultiFuncOutputPin[lucLoop] = 1;
					}
					else
					{
						lucNewMultiFuncOutputPin[lucLoop] = 0;
					}
				}
				else if (gucMultiFuncOutputMenuLastState[lucLoop] == 5)
				{

					if (gstBitwiseMultifuncOutput.bits.Dropping == 1)
					{
						lucNewMultiFuncOutputPin[lucLoop] = 1;
					}
					else
					{
						lucNewMultiFuncOutputPin[lucLoop] = 0;
					}
				}
				else if (gucMultiFuncOutputMenuLastState[lucLoop] == 6)
				{

					if (gstBitwiseMultifuncOutput.bits.GreenSignalLamp == 1)
					{
						lucNewMultiFuncOutputPin[lucLoop] = 1;
					}
					else
					{
						lucNewMultiFuncOutputPin[lucLoop] = 0;
					}
				}
				else if (gucMultiFuncOutputMenuLastState[lucLoop] == 7)
				{

					if (gstBitwiseMultifuncOutput.bits.RedSignalLamp == 1)
					{
						lucNewMultiFuncOutputPin[lucLoop] = 1;
					}
					else
					{
						lucNewMultiFuncOutputPin[lucLoop] = 0;
					}
				}
				else if (gucMultiFuncOutputMenuLastState[lucLoop] == 8)
				{

					if (gstBitwiseMultifuncOutput.bits.AutomaticModeOutput == 1)
					{
						lucNewMultiFuncOutputPin[lucLoop] = 1;
					}
					else
					{
						lucNewMultiFuncOutputPin[lucLoop] = 0;
					}
				}
				else if (gucMultiFuncOutputMenuLastState[lucLoop] == 9)
				{

					if (gstBitwiseMultifuncOutput.bits.ManaualModeOutput == 1)
					{
						lucNewMultiFuncOutputPin[lucLoop] = 1;
					}
					else
					{
						lucNewMultiFuncOutputPin[lucLoop] = 0;
					}
				}
				else if (gucMultiFuncOutputMenuLastState[lucLoop] == 10)
				{

					if (gstBitwiseMultifuncOutput.bits.ErrorOutput == 1)
					{
						lucNewMultiFuncOutputPin[lucLoop] = 1;
					}
					else
					{
						lucNewMultiFuncOutputPin[lucLoop] = 0;
					}
				}

			} // for (lucLoop == 0; lucLoop  < 5; lucLoop++)

			// Update Multi-function output pins basis lucNewMultiFuncOutputPin
			if(lucNewMultiFuncOutputPin[0] == 1)
			{
				ACTIVATE_MULTI_FUNC_OUT_1;
			}
			else
			{
				DEACTIVATE_MULTI_FUNC_OUT_1;
			}

			if(lucNewMultiFuncOutputPin[1] == 1)
			{
				ACTIVATE_MULTI_FUNC_OUT_2;
			}
			else
			{
				DEACTIVATE_MULTI_FUNC_OUT_2;
			}

			if(lucNewMultiFuncOutputPin[2] == 1)
			{
				ACTIVATE_MULTI_FUNC_OUT_3;
			}
			else
			{
				DEACTIVATE_MULTI_FUNC_OUT_3;
			}

			if(lucNewMultiFuncOutputPin[3] == 1)
			{
				ACTIVATE_MULTI_FUNC_OUT_4;
			}
			else
			{
				DEACTIVATE_MULTI_FUNC_OUT_4;
			}

			if(lucNewMultiFuncOutputPin[4] == 1)
			{
				ACTIVATE_MULTI_FUNC_OUT_5;
			}
			else
			{
				DEACTIVATE_MULTI_FUNC_OUT_5;
			}

		} //if (lucNeedToSetMultifuncOutputFlag == 1)

		// *********************************************************************************************
		// End for sub-state 'Multi-Function Output' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************
#endif


		// *********************************************************************************************
		// Start for sub-state 'Wireless Mode Change' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

		if (

				gstCMDitoLS.commandRequestStatus == eACTIVE     &&
				gstCMDitoLS.commandResponseStatus == eNO_STATUS &&
				(gstCMDitoLS.commandDisplayBoardLS.bits.wirelessModeChangePressed || gstCMDitoLS.commandDisplayBoardLS.bits.wirelessModeChangeReleased)

		)
		{

			if (gstCMDitoLS.commandDisplayBoardLS.bits.wirelessModeChangePressed)
			{
				DEACTIVATE_WIRELESS_MODE_CHANGE;
			}
			else if (gstCMDitoLS.commandDisplayBoardLS.bits.wirelessModeChangeReleased)
			{
				ACTIVATE_WIRELESS_MODE_CHANGE;
			}

			gstCMDitoLS.commandResponseStatus = eSUCCESS;
			gstCMDitoLS.acknowledgementReceived = eACK;

		}
		// *********************************************************************************************
		// End for sub-state 'Wireless Mode Change' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

#if 1
		// *********************************************************************************************
		// Start for sub-state 'Update Snow Mode Setting of Drive Board' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************

			if (sHandleupdateSnowModeDrive == HandleSnowDrive_CheckForUpdate)
			{

				if (su8_drive_snow_mode != gu8_snow_mode)
				{
					sHandleupdateSnowModeDrive = HandleSnowDrive_SendSnowModeCmd;
				}

			}
			else if (sHandleupdateSnowModeDrive == HandleSnowDrive_SendSnowModeCmd)
			{

				if (gstLStoCMDr.commandRequestStatus == eINACTIVE)
				{

					gstLStoCMDr.commandRequestStatus = eACTIVE;

					gstLStoCMDr.commandToDriveBoard.bits.setParameter = 1;
					gstLStoCMDr.dataToDriveBoard.parameterNumber = SNOW_MODE_PARAMETER;
					gstLStoCMDr.dataToDriveBoard.commandData.setParameterValue = gu8_snow_mode;

					// Update last command sent
					sstLStoCMDrCmdSent.commandToDriveBoard.val = gstLStoCMDr.commandToDriveBoard.val;

					sHandleupdateSnowModeDrive = HandleSnowDrive_WaitSnowModeReply;

				}

			}
			else if (sHandleupdateSnowModeDrive == HandleSnowDrive_WaitSnowModeReply)
			{

				if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
				{

					if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
					{

						sHandleupdateSnowModeDrive = HandleSnowDrive_CheckForUpdate;

						su8_drive_snow_mode = gu8_snow_mode;

					} //if (gstLStoCMDr.commandResponseStatus == eSUCCESS)
					else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)
					{

						sHandleupdateSnowModeDrive = HandleSnowDrive_SendSnowModeCmd;

					} //else if (gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

					// Release CMDr Block
					gstLStoCMDr.commandRequestStatus = eINACTIVE;
					gstLStoCMDr.commandToDriveBoard.val = 0;
					gstLStoCMDr.commandResponseStatus = eNO_STATUS;

				} // if (gstLStoCMDr.commandResponseStatus == eSUCCESS || gstLStoCMDr.commandResponseStatus == eTIME_OUT || gstLStoCMDr.commandResponseStatus == eFAIL)

			}


		// *********************************************************************************************
		// End for sub-state 'Update Snow Mode Setting of Drive Board' of 'Logic_Solver_Drive_Run'
		// *********************************************************************************************
#endif


		break; //case Logic_Solver_Drive_Run:

	default:
		eLogic_Solver_State = Logic_Solver_Power_ON_Init;

	} // switch(eLogic_Solver_State)

} //void logicSolver(void)

/******************************************************************************/

/******************************************************************************
 * logicSolverToTestCMDr
 *
 * Function Description:
 * This function is temporarily created to test CMDr. It checks whether a command
 * is generated from CMDi to LS and whether that command is processed by CMDr
 * and clears the LS to CMDr accordingly.
 * Command to CMDr may be activated in main.c file for functional testing.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void logicSolverToTestCMDrCMDi(void) {
	/************************Needed for testing CMDi*******************************/
	if (gstCMDitoLS.commandRequestStatus == eACTIVE) {
		if (gstCMDitoLS.commandDisplayBoardLS.bits.upPressed == 1
				&& gstLStoCMDr.commandRequestStatus == eINACTIVE) {
			gstLStoCMDr.commandToDriveBoard.bits.openShutter = 1;
			gstLStoCMDr.commandRequestStatus = eACTIVE;
		}
		if (gstCMDitoLS.commandDisplayBoardLS.bits.openPressed == 1
				&& gstLStoCMDr.commandRequestStatus == eINACTIVE) {
			gstLStoCMDr.commandToDriveBoard.bits.openShutter = 1;
			gstLStoCMDr.commandRequestStatus = eACTIVE;
		}
		if (gstCMDitoLS.commandDisplayBoardLS.bits.downPressed == 1
				&& gstLStoCMDr.commandRequestStatus == eINACTIVE) {
			gstLStoCMDr.commandToDriveBoard.bits.closeShutter = 1;
			gstLStoCMDr.commandRequestStatus = eACTIVE;
		}
		if (gstCMDitoLS.commandDisplayBoardLS.bits.closePressed == 1
				&& gstLStoCMDr.commandRequestStatus == eINACTIVE) {
			gstLStoCMDr.commandToDriveBoard.bits.closeShutter = 1;
			gstLStoCMDr.commandRequestStatus = eACTIVE;
		}
		if (gstCMDitoLS.commandDisplayBoardLS.bits.stopPressed == 1
				&& gstLStoCMDr.commandRequestStatus == eINACTIVE) {
			gstLStoCMDr.commandToDriveBoard.bits.stopShutter = 1;
			gstLStoCMDr.commandRequestStatus = eACTIVE;
		}
		if (gstCMDitoLS.commandDisplayBoardLS.bits.startInstallation == 1
				&& gstLStoCMDr.commandRequestStatus == eINACTIVE) {
			gstLStoCMDr.commandToDriveBoard.bits.startInstallation = 1;
			gstLStoCMDr.commandRequestStatus = eACTIVE;
		}

		if (gstLStoCMDr.commandResponseStatus == eSUCCESS
				|| eFAIL == gstLStoCMDr.commandResponseStatus
				|| eTIME_OUT == gstLStoCMDr.commandResponseStatus) {
			gstCMDitoLS.commandResponseStatus =
					gstLStoCMDr.commandResponseStatus;
		}
	}
	/************************Needed for testing CMDi*******************************/

	/************************Needed for testing CMDr and CMDi*******************************/
	if (gstLStoCMDr.commandResponseStatus == eSUCCESS
			|| gstLStoCMDr.commandResponseStatus == eTIME_OUT
			|| gstLStoCMDr.commandResponseStatus == eFAIL)// Active command processed
					{
		//
		//	Update gstCMDitoLS.commandResponseStatus
		//
		gstCMDitoLS.commandResponseStatus = gstLStoCMDr.commandResponseStatus;

		if (gstLStoCMDr.commandResponseStatus == eSUCCESS) {
			//
			//	Clear control board communication error flag
			//
			if (gstControlBoardFault.bits.controlCommunication == 1) {
				gstControlBoardFault.bits.controlCommunication = 0;
			}
			//
			//	Clear control-drive communication error flag
			//
			if (gstControlCommunicationFault.bits.commFailDrive == 1) {
				gstControlCommunicationFault.bits.commFailDrive = 0;
			}
			//
			//	Clear control board CRC error flag
			//
			if (gstControlCommunicationFault.bits.crcErrorDrive == 1) {
				gstControlCommunicationFault.bits.crcErrorDrive = 0;
			}

			gstCMDitoLS.acknowledgementReceived =
					gstLStoCMDr.acknowledgementReceived;
		}

		if (gstLStoCMDr.commandResponseStatus == eTIME_OUT) {
			gstControlBoardFault.bits.controlCommunication = 1;
			gstControlCommunicationFault.bits.commFailDrive = 1;
		}

		if (gstLStoCMDr.commandResponseStatus == eFAIL) {
			gstControlBoardFault.bits.controlCommunication = 1;
			gstControlCommunicationFault.bits.crcErrorDrive = 1;
		}

		gstLStoCMDr.commandRequestStatus = eINACTIVE;
		gstLStoCMDr.commandToDriveBoard.val = 0;// Clear the processed command
		gstLStoCMDr.commandResponseStatus = eNO_STATUS;
	}
	/************************Needed for testing CMDr and CMDi*******************************/
}

/********************************************************************************/

#if 0
/******************************************************************************
 * ValidateInterlockInput
 *
 * Function Description:
 * Function will validate the Interlock Output from other shutter and
 * will decide whether it is allowed to operate the shutter or not
 * Function Parameter: void
 *
 * Function Returns:
 * 0 = do not operate shutter
 * 1 = operate shutter
 ********************************************************************************/
unsigned char ValidateInterlockInput(void)
{
	unsigned char lucFunctionReturn = 1;
//	unsigned char lucInterlockInputPin;


	if (gu8_intlck_valid != 0)  // Interlock output invalid
	{

		if (gu8_intlck_prior == 0)
		{

			if ((gstDriveStatus.bits.shutterLowerLimit == 1) && (gSensorStatus.bits.Sensor_InterlockIP_inactive == true))
			{
				lucFunctionReturn = 0;  //A001 = 0, Prioritize
			}

		}
		else 							//A001 = 1, Not Prioritize
		{

			if (gSensorStatus.bits.Sensor_InterlockIP_inactive == true)
			{
				lucFunctionReturn = 0;
			}

		}

	} //if (gu8_intlck_valid != 0)  // Interlock output invalid

	return lucFunctionReturn;

} //unsigned char ValidateInterlockOutput (void)
#endif

//	Code added on 21 Oct 2014. Code was sent by Yogesh on Linc

/******************************************************************************
� ValidateInterlockInput
�
� Function Description:
� Function will validate the�nterlock�utput from other shutter and
� will decide whether it is allowed to operate the shutter or not
� Function Parameter: void
�
� Function Returns:
� 0 = do not operate shutter
� 1 = operate shutter
�*******************************************************************************/
unsigned char ValidateInterlockInput(void)
{
	unsigned char lucFunctionReturn = 1;
	// unsigned char lucInterlockInputPin;

	if (gu8_intlck_valid == 0)	//	Interlock input valid
	{

		if(gu8_intlck_prior == 0)
		{
			// 23 Oct
			if((gstDriveStatus.bits.shutterLowerLimit== 1) && (gSensorStatus.bits.Sensor_InterlockIP_active == false))
			{
				lucFunctionReturn = 0;//A001 = 0, Prioritize
			}

		}
		else      //A001 = 1, Not Prioritize
		{
			// 23 Oct
			if(gSensorStatus.bits.Sensor_InterlockIP_active == false)
			{
				lucFunctionReturn = 0;
			}
		}

	}//if (gu8_intlck_valid != 0) //Interlock output invalid

	return lucFunctionReturn;
}//unsigned char ValidateInterlockOutput (void)

uint32_t get_timego10ms(uint32_t x_data_his)
{
	uint32_t time_pass=0;
	if(g_ui32TickCount10ms >= x_data_his)
	{
		time_pass = g_ui32TickCount10ms - x_data_his;
	}
	else
	{
		time_pass = g_ui32TickCount10ms + 0xffffffff - x_data_his;
	}
	return time_pass;
}

uint32_t get_timego(uint32_t x_data_his)
{
	uint32_t time_pass=0;
	if(g_ui32TickCount >= x_data_his)
	{
		time_pass = g_ui32TickCount - x_data_his;
	}
	else
	{
		time_pass = g_ui32TickCount + 0xffffffff - x_data_his;
	}
	return time_pass;
}



uint32_t his_flaglogintime=0;
#define key_time 3
#define key_time_3s  120
#define key_time_1s  20//150

void wirelesslogin_cyw(void)
{
   uint8_t tp_i = 0;
   switch(flag_addlogin)
   {
   case 0:
	   break;
   case 1:ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, MONIDENGLU_KEY);
          his_flaglogintime = g_ui32TickCount10ms;
          flag_addlogin = 2;
	      break;
   case 2:
	       if(get_timego10ms(his_flaglogintime) > key_time)
	     {
		     flag_addlogin = 3;
	      }
	      break;
   case 3: ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, 0);
           his_flaglogintime = g_ui32TickCount10ms;
          flag_addlogin = 4;
          break;
   case 4:
	      if(get_timego10ms(his_flaglogintime) > key_time)
	  	   {
	  		   flag_addlogin = 5;
	  	   }
	  	   break;
   case 5:ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, MONIDENGLU_KEY);
          his_flaglogintime = g_ui32TickCount10ms;
          flag_addlogin = 6;
	      break;
   case 6:
	      if(get_timego10ms(his_flaglogintime) > key_time)
	   	 {
	   	  	 flag_addlogin = 7;
	   	 }
	      break;
   case 7:ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, 0);
          his_flaglogintime = g_ui32TickCount10ms;
          flag_addlogin = 8;
          break;
   case 8:
	      if(get_timego10ms(his_flaglogintime) > key_time)
	 	   	 {
	 	   	  	 flag_addlogin = 9;
	 	   	 }
	 	      break;
   case 9:ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, MONIDENGLU_KEY);
          his_flaglogintime = g_ui32TickCount10ms;
           flag_addlogin = 10;
          break;
   case 10:
	          if(get_timego10ms(his_flaglogintime) > key_time)
	   	 	   	 {
	   	 	   	  	 flag_addlogin = 11;
	   	 	   	 }
	      break;
   case 11:ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, 0);
           his_flaglogintime = g_ui32TickCount10ms;
           flag_addlogin = 12;
            break;
   case 12 :
	   if(get_timego10ms(his_flaglogintime) > key_time)
	  	   	 	   	 {
	  	   	 	   	  	 flag_addlogin = 13;
	  	   	 	   	 }
	  	      break;
   case 13:
	   ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, MONIDENGLU_KEY);
	     his_flaglogintime = g_ui32TickCount10ms;
	      flag_addlogin = 0;

	               break;
   case 14:
	   ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, 0);
	              his_flaglogintime = g_ui32TickCount10ms;
	              flag_addlogin = 15;
	   break;
   case 15:
	   if(get_timego10ms(his_flaglogintime) > key_time_3s)
	   {
	   	  	   	 	   	  	 flag_addlogin = 16;
	   }
	   break;
   case 16:
	   ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, MONIDENGLU_KEY);
	   	     his_flaglogintime = g_ui32TickCount10ms;
	   	      flag_addlogin = 0;
	   	break;
   case 17:
  	   ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, 0);
  	              his_flaglogintime = g_ui32TickCount10ms;
  	              flag_addlogin = 18;
  	   break;
    case 18:
  	   if(get_timego10ms(his_flaglogintime) > key_time_1s)
  	   {
  	   	  	   	 	   	  	 flag_addlogin = 19;
  	   }
  	   break;
    case 19:
  	   ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, MONIDENGLU_KEY);
  	   	     his_flaglogintime = g_ui32TickCount10ms;
  	   	      flag_addlogin = 20;
  	   	    //flag_addlogin=0;
  	   	break;
    case 20:
    	if(get_timego10ms(his_flaglogintime) > key_time_1s)
    	  	   {
    	  	   	  	   	 	   	  	 flag_addlogin = 21;
    	  	   }
    	  	   break;
    case 21:
      	   ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, 0);
      	   	     his_flaglogintime = g_ui32TickCount10ms;
      	   	      flag_addlogin = 22;
      	   	break;
    case 22:
       	if(get_timego10ms(his_flaglogintime) > key_time_1s)
       	  	   {
       	  	   	  	   	 	   	  	 flag_addlogin = 23;
       	  	   }
       	  	   break;
    case 23:
    	  ROM_GPIOPinWrite(MULTI_FUNC_OUT_5_GPIO_BASE, MONIDENGLU_KEY, MONIDENGLU_KEY);
    	  	   	     his_flaglogintime = g_ui32TickCount10ms;
    	  	   	      flag_addlogin = 0;
    	  	 break;
   default:
	   break;
   }

}
