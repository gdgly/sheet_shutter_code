/*********************************************************************************
 * FileName: dbhandler.c
 * Description:
 * This header file contains definitions for Database Handler module.
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
 *  	0.3D	30/06/2014									If no param found, fail the response
 *  	0.2D	25/06/2014									Reset logic for A020, A120 added
 *  	0.1D	21/05/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Include:
 ****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <inc/hw_types.h>
#include <inc/hw_nvic.h>
#include <driverlib/flash.h>
#include "dbhandler.h"
#include "interTaskCommunication.h"
#include "Middleware/paramdatabase.h"
#include "Middleware/rtc.h"
#include "Drivers/ustdlib.h"
#include "cmdi.h"
#include "flashmemory.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/
#define FLASH_BOOT_LOADER_AREA_STARTING_ADDRESS			0x00000

/****************************************************************************
 *  Global variables
 ****************************************************************************/
// Anomaly History Log Empty Index
uint8_t gui8_AnomalyHistory_ParamIdx;

// Data Structure for Flags for Internal modules
typedef union {
	uint8_t val;
	struct {
		//ReadErrorList module flag
		uint8_t readErrorlist :1;
		uint8_t bit1 :1;
		uint8_t bit2 :1;
		uint8_t bit3 :1;
		uint8_t bit4 :1;
		uint8_t bit5 :1;
		uint8_t bit6 :1;
		uint8_t bit7 :1;
	} flags;
} _DBHandlerModules;

// Define Flags for Internal modules
_DBHandlerModules DBHandlerFlags;

/****************************************************************************/

/****************************************************************************
 *  Function definitions for this file:
 ****************************************************************************/


/******************************************************************************
 * Function Name: jumpToControlBoardBootLoader
 *
 * Function Description:
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void jumpToControlBoardBootLoader(void)
{
	HWREG(NVIC_VTABLE) = (uint32_t)FLASH_BOOT_LOADER_AREA_STARTING_ADDRESS;

	/*__asm(" ldr r1, [r0]\n"
			" mov sp, r1");*/
	__asm(" ldr r1, [r0]");
	__asm(" mov sp, r1");

	/*__asm(" ldr r0, [r0, #4]\n"
			" bx r0\n");*/
	__asm(" ldr r0, [r0, #4]");
	__asm(" bx r0\n");
}

/******************************************************************************
 * Function Name: initdbhandler
 *
 * Function Description: Initializes the Anomaly History Log Empty Index.
 * This is required to be called in every application initialization.
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void initdbhandler(void) {
	DBHandlerFlags.val = 0;
	gui8_AnomalyHistory_ParamIdx = computeIndex(ANOMHIST_START_IDX);
}

/******************************************************************************
 * Function Name: writeAnomalyHistory
 *
 * Function Description: Writes Anomaly History data to EEPROM and
 * updates Request and Response status.
 * Skip this function if currently Error List is being read.
 * EM's responsibility to set its commandRequestStatus flag.
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void writeAnomalyHistory(void) {
	//
	if ((gstEMtoDH.commandRequestStatus == eACTIVE)
			&& (gstEMtoDH.commandResponseStatus == eNO_STATUS)
			&& (!DBHandlerFlags.flags.readErrorlist)) {
		if (writeParameterUpdateInDB(
				(PARAM_CTRL) (gui8_AnomalyHistory_ParamIdx - 1
						+ ANOMHIST_START_IDX),
				(uint8_t*) &gstEMtoDH.errorToDH) == _SUCCESS) {
			gstEMtoDH.commandResponseStatus = eSUCCESS;
			gui8_AnomalyHistory_ParamIdx++;

			// Check Anomaly History Log Empty Index range
			if (gui8_AnomalyHistory_ParamIdx > _MAX_ANOMALY_LOGS)
				gui8_AnomalyHistory_ParamIdx = 1;
		} else
			gstEMtoDH.commandResponseStatus = eFAIL;
		//15 May 2014 - UM will control the flag
//			gstEMtoDH.commandRequestStatus = eINACTIVE;
	}
}

/******************************************************************************
 * Function Name: deleteAnomalyHistory
 *
 * Function Description: This function is called if ACK is received for the Log
 * sent to the Display board. Loads default parameter for Log which is zero
 * defined. Also updates Anomaly History Log Empty Index.
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void deleteAnomalyHistory(void) {
	uint8_t logParamIdx = ANOMHIST_START_IDX;

	// Check Anomaly History Log Empty Index range
	if (gui8_AnomalyHistory_ParamIdx == 1) {
		logParamIdx = _MAX_ANOMALY_LOGS;
	} else {
		logParamIdx = gui8_AnomalyHistory_ParamIdx - 1;
	}

	//Load Default Log which is defined zero.
	if (paramLoadDefault(
			(PARAM_CTRL) (logParamIdx + ANOMHIST_START_IDX - 1)) == _SUCCESS) {
		// Check Anomaly History Log Empty Index range
		if (gui8_AnomalyHistory_ParamIdx > 1)
			gui8_AnomalyHistory_ParamIdx--;
		else
			gui8_AnomalyHistory_ParamIdx = _MAX_ANOMALY_LOGS;
	}
}

/******************************************************************************
 * Function Name: DBHandler_Module
 *
 * Function Description: This module function to be called in main() while{}.
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void DBHandler_Module(void) {
	writeAnomalyHistory();
	handleCMDi();
	monitorResetParam();
}

/******************************************************************************
 * Function Name: handleCMDi
 *
 * Function Description: This module handles the CMDitoDH Block structure.
 * Handles following commands:
 * GETERRORLIST - Reads Error Log to be sent to Display Board. If ACK comes,
 * that Error log is to be deleted.
 * SETTIMESTAMP - ,
 * GETPARAMETER - ,
 * SETPARAMETER -
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void handleCMDi(void) {
	uint8_t iTemp;
	PARAM_CTRL getParamIdx = (PARAM_CTRL) 0;

	static uint8_t lsui8JumpToBootLoaderFlag = 0;
	uint32_t tempData = 0;

	if(	(gstCMDitoDH.commandRequestStatus == eINACTIVE) &&
		(gstCMDitoDH.commandResponseStatus == eNO_STATUS) &&
		(1 == lsui8JumpToBootLoaderFlag)
	)
	{
		tempData = 0x02;		//	Control board firmware upgrade

		FlashErase(FLASH_FIRWARE_UPGRADE_FROM_APP_FLAG);

		//	Set flag in flash memory to indicate to the control board that
		//	"Control board firmware upgrade" is being called from application
		writeApplicationAreaInFlash(&tempData, FLASH_FIRWARE_UPGRADE_FROM_APP_FLAG, 4);

		//
		// jump to control board boot loader
		//
		jumpToControlBoardBootLoader();
	}

	//
	//	Added on 17 Nov 2014 to implement drive board firmware upgrade functionality.
	//

	//	Check whether drive board firmware upgrade in initiated and drive board has jumped into bootloader
	if(
			(gui8DriveFirwareUpgradeInitiated == 1) &&
			(gstDriveStatus.bits.driveBoardBootloader == 1)
	)
	{
		tempData = 0x04;		//	Drive board firmware upgrade

		FlashErase(FLASH_FIRWARE_UPGRADE_FROM_APP_FLAG);

		//	Set flag in flash memory to indicate to the control board that
		//	"Control board firmware upgrade" is being called from application
		writeApplicationAreaInFlash(&tempData, FLASH_FIRWARE_UPGRADE_FROM_APP_FLAG, 4);

		//
		// jump to control board boot loader
		//
		jumpToControlBoardBootLoader();
	}

	if ((gstCMDitoDH.commandRequestStatus == eACTIVE)
			&& (gstCMDitoDH.commandResponseStatus == eNO_STATUS)) {
		// GET_ERROR_LIST & No Status ACK - Read to Structure
		if ((gstCMDitoDH.commandDisplayBoardDH.bits.getErrorList)
				&& (gstCMDitoDH.commandResponseACK_Status
						== eNO_StatusAcknowledgement)) {
			DBHandlerFlags.flags.readErrorlist = 1;

			uint8_t ReadIdx_ParamAnomHist = 1;

			// Check Anomaly History Log Empty Index range. Set Read Index to
			if (gui8_AnomalyHistory_ParamIdx == 1)
				ReadIdx_ParamAnomHist = _MAX_ANOMALY_LOGS;
			else
				ReadIdx_ParamAnomHist = gui8_AnomalyHistory_ParamIdx - 1;

			if (readParameterFromDB(
					(PARAM_CTRL) (ReadIdx_ParamAnomHist + ANOMHIST_START_IDX - 1),
					(uint8_t*) &gstCMDitoDH.errorFromControl) == _SUCCESS) {
				gstCMDitoDH.commandResponseStatus = eSUCCESS;
			} else
				gstCMDitoDH.commandResponseStatus = eFAIL;

		}

		// If ACK Received from Display Board, then delete that Anomaly Log and change to eResponseAcknowledgementProcessed
		else if ((gstCMDitoDH.commandDisplayBoardDH.bits.getErrorList)
				&& (gstCMDitoDH.commandResponseACK_Status
						== eResponseAcknowledgement_ACK)) {
			deleteAnomalyHistory();
			DBHandlerFlags.flags.readErrorlist = 0;
			gstCMDitoDH.commandResponseStatus = eSUCCESS;
			gstCMDitoDH.commandResponseACK_Status =
					eResponseAcknowledgementProcessed;
		}

		// GET_PARAMETER
		else if (gstCMDitoDH.commandDisplayBoardDH.bits.getParameter) {
			// Calculate Parameter Index from Parameter Number
			for (iTemp = 0; iTemp < _NO_OF_PARAMS; iTemp++) {
				if (gstCMDitoDH.commandDataCMDiDH.parameterNumber
						== gui_ParamNo_Ctrl[iTemp]) {
					getParamIdx = (PARAM_CTRL) iTemp;
					break;
				}
			}

			//Patch added-30Jun14- If no param found
			if (iTemp == _NO_OF_PARAMS) {
				gstCMDitoDH.commandResponseStatus = eFAIL;
				return;
			}

			//Get and save response in gstCMDitoDH.getParameterValue
			gstCMDitoDH.getParameterValue = 0;
			if (readParameterFromDB(getParamIdx,
					(uint8_t*) &gstCMDitoDH.getParameterValue) == _SUCCESS)
			{
				if(gstCMDitoDH.commandDataCMDiDH.parameterNumber == CONTROL_BOARD_FIRMWARE_VERSION)
				{
					//	Added on 3 Nov 2014
					//	Control board firmware version read from default value on RAM
					gstCMDitoDH.getParameterValue = gu32_ctrl_fwver_DEF;
				}
				gstCMDitoDH.commandResponseStatus = eSUCCESS;
			}
			else
			{
				gstCMDitoDH.commandResponseStatus = eFAIL;
			}

		}

		// SET_PARAMETER
		else if (gstCMDitoDH.commandDisplayBoardDH.bits.setParameter) {
			//Calculate Parameter Index from Parameter Number
			for (iTemp = 0; iTemp < _NO_OF_PARAMS; iTemp++) {
				if (gstCMDitoDH.commandDataCMDiDH.parameterNumber
						== gui_ParamNo_Ctrl[iTemp]) {
					getParamIdx = (PARAM_CTRL) iTemp;
					break;
				}
			}

			//Patch added-30Jun14- If no param found
			if (iTemp == _NO_OF_PARAMS) {
				gstCMDitoDH.commandResponseStatus = eFAIL;
				return;
			}

			// Save Parameter value in Database
			if (writeParameterUpdateInDB(getParamIdx,
					(uint8_t*) &gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue) == _SUCCESS)
				gstCMDitoDH.commandResponseStatus = eSUCCESS;
			else
				gstCMDitoDH.commandResponseStatus = eFAIL;

		}

		// SET_TIMESTAMP
		else if (gstCMDitoDH.commandDisplayBoardDH.bits.setTimeStamp) {
			struct tm tmset = {0,0,0,0,0,0,0,0,0};
			ulocaltime(gstCMDitoDH.commandDataCMDiDH.commandData.timeStamp,
					&tmset);
			if (!RTCSet(&tmset))
				gstCMDitoDH.commandResponseStatus = eSUCCESS;
			else
				gstCMDitoDH.commandResponseStatus = eFAIL;
		}

		// FIRMWARE_UPGRADE
		else if(gstCMDitoDH.commandDisplayBoardDH.bits.firmwareUpgrade)
		{
			gstCMDitoDH.commandResponseStatus = eSUCCESS;

			// set flag to allow switching to boot loader
			lsui8JumpToBootLoaderFlag = 1;
		}
	}
}

/******************************************************************************
 * Function Name: monitorResetParam
 *
 * Function Description: Checks for Reset parameter value
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void monitorResetParam(void) {
	PARAM_CTRL getParamIdx = (PARAM_CTRL) 0;
	uint8_t iTemp, jTemp;
	uint8_t noparamsreset;

	if (INIT_VALUE_PARAM) {
		noparamsreset = sizeof(gui_InitValSetParamNo_table)
				/ sizeof(gui_InitValSetParamNo_table[0]);

		//Calculate Parameter Index from Parameter Number
		for (iTemp = 0; iTemp < noparamsreset; iTemp++)
		{
			for (jTemp = 0; jTemp < _NO_OF_PARAMS; jTemp++)
			{
				if (gui_InitValSetParamNo_table[iTemp] == gui_ParamNo_Ctrl[jTemp])
				{
					getParamIdx = (PARAM_CTRL) jTemp;
					paramLoadDefault(getParamIdx);
					break;
				}
			}
		}
		//Reset the CSH Reset Parameter
		INIT_VALUE_PARAM = 0;
		writeParameterUpdateInDB(INIT_VALUE_PARAM_IDX, &INIT_VALUE_PARAM);
	}

	if (OPRCNTRES_PARAM) {
		paramLoadDefault(A600OPRCNT);

		//Reset the Operation Counter Reset Parameter
		OPRCNTRES_PARAM = 0;
		writeParameterUpdateInDB(OPRCNTRES_PARAM_IDX, &OPRCNTRES_PARAM);
	}

//	if(INITSHEETPOSPARAMS_PARAM)
//	{
//		noparamsreset = sizeof(gui_InitSheetPosParamNo_table)/sizeof(gui_InitSheetPosParamNo_table[0]);
//
//		//Calculate Parameter Index from Parameter Number
//		for(iTemp = 0; iTemp < noparamsreset; iTemp++)
//		{
//			for(jTemp = 0; jTemp < _NO_OF_PARAMS; jTemp++)
//				if(gui_InitSheetPosParamNo_table[iTemp] == gui_ParamNo_Ctrl[jTemp])
//				{
//					getParamIdx = (PARAM_CTRL)jTemp;
//					paramLoadDefault(getParamIdx);
//					break;
//				}
//		}
//		//Reset the InitSheetPosResetParam Parameter
//		INITSHEETPOSPARAMS_PARAM = 0;
//		writeParameterUpdateInDB(INITSHEETPOSPARAMS_PARAM_IDX, &INITSHEETPOSPARAMS_PARAM);
//	}
}
