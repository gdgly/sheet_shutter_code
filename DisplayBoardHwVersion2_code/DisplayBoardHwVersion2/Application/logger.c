/*********************************************************************************
* FileName: logger.c
* Description:
* This header file contains definitions for Logger module.
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
 *  	0.1D	13/05/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdint.h>
#include "Middleware/paramdatabase.h"
#include "logger.h"
#include "interTaskCommunication.h"
#include "Middleware/serial.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define AH_PARAM_SIZE	28
#define CHS_PARAM_SIZE	16

/****************************************************************************
 *  Global variables
****************************************************************************/
//gstUMtoLM_write;
//gstEMtoLM;
//gstUMtoLM_read;
uint8_t gui8_AnomalyHistory_ParamIdx;
uint8_t gui8_ChgSettHistory_ParamIdx;
extern uint32_t ui32OperationCount;  //20170414      201703_No.31
/****************************************************************************/


/****************************************************************************
 *  Function definitions for this file:
****************************************************************************/

void initLogger(void)
{
	gui8_AnomalyHistory_ParamIdx = computeIndex(ANOMHIST_START_IDX);
	gui8_ChgSettHistory_ParamIdx = computeIndex(CHGSETHIST_START_IDX);
}
/******************************************************************************
 * Function Name: writeChangeSettingsHistory
 *
 * Function Description: Writes Change Settings History data to EEPROM and
 * updates Request and Response status.
 * Possible Status: eSUCCESS, eFAIL, eINACTIVE
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void writeChangeSettingsHistory(void)
{
	if((gstUMtoLM_write.commandRequestStatus == eACTIVE) && (gstUMtoLM_write.commandResponseStatus == eNO_STATUS))
	{
		if(gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory)
		{
			//15 May 2014 - UM will reset the flag
//			gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory = 0;

			if(writeParameterUpdateInDB((PARAM_DISP)(gui8_ChgSettHistory_ParamIdx -1 + CHGSETHIST_START_IDX), //Since
					(uint8_t*)&gstUMtoLM_write.changeSetting) == _SUCCESS)
			{
				gstUMtoLM_write.commandResponseStatus = eSUCCESS;
				gui8_ChgSettHistory_ParamIdx++;

				if(gui8_ChgSettHistory_ParamIdx > _MAX_CHGSETHIST_LOGS)
					gui8_ChgSettHistory_ParamIdx = 1;
			}
			else
				gstUMtoLM_write.commandResponseStatus = eFAIL;

			//15 May 2014 - UM will control the flag
//			gstUMtoLM_write.commandRequestStatus = eINACTIVE;
		}
	}
}

/******************************************************************************
 * Function Name: writeAnomalyHistory
 *
 * Function Description: Writes Anomaly History data to EEPROM and
 * updates Request and Response status.
 * Possible Status: eSUCCESS, eFAIL, eINACTIVE
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void writeAnomalyHistory(void)
{
	uint8_t Tp_ANOMHIST_START_IDX[20]={A030ANOMHIST_1,A031ANOMHIST_2,A032ANOMHIST_3,A033ANOMHIST_4,A034ANOMHIST_5,
                                       A035ANOMHIST_6,A036ANOMHIST_7,A037ANOMHIST_8,A038ANOMHIST_9,A039ANOMHIST_10,
                                       A900ANOMHIST_11,A901ANOMHIST_12,A902ANOMHIST_13,A903ANOMHIST_14,A904ANOMHIST_15,
			                           A905ANOMHIST_16,A906ANOMHIST_17,A907ANOMHIST_18,A908ANOMHIST_19,A909ANOMHIST_20};

	if((gstEMtoLM.commandRequestStatus == eACTIVE) && (gstEMtoLM.commandResponseStatus == eNO_STATUS))
	{
		   gstEMtoLM.errorToLM.operationCount = ui32OperationCount;
		   //20170414      201703_No.31 start
		  if((writeParameterUpdateInDB((PARAM_DISP)(Tp_ANOMHIST_START_IDX[gui8_AnomalyHistory_ParamIdx - 1]),
					(uint8_t*)&gstEMtoLM.errorToLM) == _SUCCESS)&&
		    (writeParameterUpdateInDB((PARAM_DISP)(gui8_AnomalyHistory_ParamIdx-1+A910ANOMHIST_OP1),
				  					(uint8_t*)&gstEMtoLM.errorToLM.operationCount)==_SUCCESS))
			  //20170414      201703_No.31 end
			{
				gstEMtoLM.commandResponseStatus = eSUCCESS;
				gui8_AnomalyHistory_ParamIdx++;

				if(gui8_AnomalyHistory_ParamIdx > _MAX_ANOMALY_LOGS)
					gui8_AnomalyHistory_ParamIdx = 1;
			}
			else
			{
				gstEMtoLM.commandResponseStatus = eFAIL;
			}


			;

			//15 May 2014 - UM will control the flag
//			gstEMtoLM.commandRequestStatus = eINACTIVE;
	}
}

/******************************************************************************
 * Function Name: readChangeSettingsHistory
 *
 * Function Description: Reads Anomaly History data to EEPROM and
 * updates Request and Response status.
 * Possible Status: eSUCCESS, eFAIL, eINACTIVE
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void readChangeSettingsHistory(void)
{
	if((gstUMtoLM_read.commandRequestStatus == eACTIVE) && (gstUMtoLM_read.commandResponseStatus == eNO_STATUS))
	{
		if(gstUMtoLM_read.commandToLMread.bits.readChangeSettingsHistory)
		{
			//15 May 2014 - UM will control the flag
//			gstUMtoLM_read.commandToLMread.bits.readChangeSettingsHistory = 0;

			uint8_t logReadIndex;
			logReadIndex = (gstUMtoLM_read.historyOrAnomalyIndex > (gui8_ChgSettHistory_ParamIdx-1)) ? (_MAX_CHGSETHIST_LOGS - (gstUMtoLM_read.historyOrAnomalyIndex - gui8_ChgSettHistory_ParamIdx)) : (gui8_ChgSettHistory_ParamIdx - gstUMtoLM_read.historyOrAnomalyIndex);

			if(readParameterFromDB((PARAM_DISP)(logReadIndex - 1 + CHGSETHIST_START_IDX), (uint8_t*)&gstUMtoLM_read.changeSettingHistory) == _SUCCESS)
				gstUMtoLM_read.commandResponseStatus = eSUCCESS;
			else
				gstUMtoLM_read.commandResponseStatus = eFAIL;

			//15 May 2014 - UM will control the flag
//			gstUMtoLM_read.commandRequestStatus = eINACTIVE;
		}
	}
}

/******************************************************************************
 * Function Name: readAnomalyHistory
 *
 * Function Description: Reads Anomaly History data to EEPROM and
 * updates Request and Response status.
 * Possible Status: eSUCCESS, eFAIL, eINACTIVE
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void readAnomalyHistory(void)
{
	uint8_t Tp_ANOMHIST_IDX[20]={A030ANOMHIST_1,A031ANOMHIST_2,A032ANOMHIST_3,A033ANOMHIST_4,A034ANOMHIST_5,
			                     A035ANOMHIST_6,A036ANOMHIST_7,A037ANOMHIST_8,A038ANOMHIST_9,A039ANOMHIST_10,
			                     A900ANOMHIST_11,A901ANOMHIST_12,A902ANOMHIST_13,A903ANOMHIST_14,A904ANOMHIST_15,
								 A905ANOMHIST_16,A906ANOMHIST_17,A907ANOMHIST_18,A908ANOMHIST_19,A909ANOMHIST_20};

	if((gstUMtoLM_read.commandRequestStatus == eACTIVE) && (gstUMtoLM_read.commandResponseStatus == eNO_STATUS))
	{
		if(gstUMtoLM_read.commandToLMread.bits.readAnomalyHistory)
		{
			//15 May 2014 - UM will control the flag
//			gstUMtoLM_read.commandToLMread.bits.readAnomalyHistory = 0;

			uint8_t logReadIndex;
			logReadIndex = (gstUMtoLM_read.historyOrAnomalyIndex > (gui8_AnomalyHistory_ParamIdx-1)) ? (_MAX_ANOMALY_LOGS - (gstUMtoLM_read.historyOrAnomalyIndex - gui8_AnomalyHistory_ParamIdx)) : (gui8_AnomalyHistory_ParamIdx - gstUMtoLM_read.historyOrAnomalyIndex);
			//20170414      201703_No.31 start
			if((readParameterFromDB((PARAM_DISP)(Tp_ANOMHIST_IDX[logReadIndex - 1]), (uint8_t*)&gstUMtoLM_read.anomalyHistory) == _SUCCESS)&&
					(readParameterFromDB((PARAM_DISP)(logReadIndex - 1 + A910ANOMHIST_OP1),(uint8_t*)&gstUMtoLM_read.anomalyHistory.operationCount)==_SUCCESS))
				//20170414      201703_No.31 end
				gstUMtoLM_read.commandResponseStatus = eSUCCESS;
			else
				gstUMtoLM_read.commandResponseStatus = eFAIL;



			//15 May 2014 - UM will control the flag
//			gstUMtoLM_read.commandRequestStatus = eINACTIVE;
		}
	}
}


/******************************************************************************
 * Function Name: monitorResetLogParam
 *
 * Function Description: Reads
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
uint8_t monitorResetLogParam(void)
{
	if(RESET_CHGSET_PARAM)
	{
		//Reset all 10 Change Setting history logs
		if(clearLogs(CHGSETHIST_START_IDX) != _SUCCESS)
			return 1;

		//Reset Change Setting Param Relative Index
		gui8_ChgSettHistory_ParamIdx = 1;

		//Reset the CSH Reset Parameter
		RESET_CHGSET_PARAM = 0;
		if(writeParameterUpdateInDB(RESETCHGSET_PARAM_IDX, &RESET_CHGSET_PARAM) != _SUCCESS)
			return 1;
	}

	else if(RESET_ANOM_PARAM)
	{
		//Reset all 10 Anomaly History logs
		if(clearLogs(ANOMHIST_START_IDX) != _SUCCESS)
				return 1;

		//Reset Anomaly History Param Relative Index
		gui8_AnomalyHistory_ParamIdx = 1;

		//Reset the CSH Reset Parameter
		RESET_ANOM_PARAM = 0;
		if(writeParameterUpdateInDB(RESETANOM_PARAM_IDX, &RESET_ANOM_PARAM) != _SUCCESS)
			return 1;
	}
	return 0;
}


/******************************************************************************
 * Function Name: computeEEPROMLogsWriteIndex
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:
 *
 ********************************************************************************/
#if 0
uint8_t computeIndex(PARAM_DISP AHorCSH_Idx)
{
	uint8_t i = 0;
	uint8_t lui8ParamSize = guc_Paramsize_Disp[AHorCSH_Idx];
	uint8_t lui8LogWriteIndex = 0;
	time_t lCurrentLogTimeStamp = 0, lNextLogTimeStamp = 0;

	for (i = 1; i <= _MAX_ANOMALY_LOGS; i++)
	{
		//
		// index to read from anomaly history or change setting history
		//
		gstUMtoLM_read.historyOrAnomalyIndex = i;

		//
		// Calculate Anomaly History write index.
		//
		if(lui8ParamSize == AH_PARAM_SIZE)
		{
			//
			// Read anomaly history
			//
			gstUMtoLM_read.commandToLMread.bits.readAnomalyHistory = 1;
			readAnomalyHistory();

			//
			// Check whether anomaly log exists
			//
			if( 0 == gstUMtoLM_read.anomalyHistory.anomalyCode )
			{
				lui8LogWriteIndex = gstUMtoLM_read.historyOrAnomalyIndex;
				return lui8LogWriteIndex;
			}

			//
			// Store current log's time stamp
			//
			lCurrentLogTimeStamp = gstUMtoLM_read.anomalyHistory.timeStamp;

			//
			// Calculate next log's time stamp
			//
			if(i < 10)
				gstUMtoLM_read.historyOrAnomalyIndex = i+1;
			else
				gstUMtoLM_read.historyOrAnomalyIndex = 1;
			readAnomalyHistory();
			lNextLogTimeStamp = gstUMtoLM_read.anomalyHistory.timeStamp;

			//
			// Check whether next log is obsolete
			//
			if(lCurrentLogTimeStamp > lNextLogTimeStamp)
			{
				lui8LogWriteIndex = gstUMtoLM_read.historyOrAnomalyIndex;
				return lui8LogWriteIndex;
			}
		}

		//
		// Calculate Change Setting History write index
		//
		else if(lui8ParamSize == CHS_PARAM_SIZE)
		{
			gstUMtoLM_read.commandToLMread.bits.readChangeSettingsHistory = 1;
			readChangeSettingsHistory();

			if( 0 == gstUMtoLM_read.changeSettingHistory.parameterNumber )
			{
				lui8LogWriteIndex = gstUMtoLM_read.historyOrAnomalyIndex;
				return lui8LogWriteIndex;
			}

			//
			// Store current log's time stamp
			//
			lCurrentLogTimeStamp = gstUMtoLM_read.changeSettingHistory.timeStamp;

			//
			// Calculate next log's time stamp
			//
			if(i < 10)
				gstUMtoLM_read.historyOrAnomalyIndex = i+1;
			else
				gstUMtoLM_read.historyOrAnomalyIndex = 1;
			readChangeSettingsHistory();
			lNextLogTimeStamp = gstUMtoLM_read.changeSettingHistory.timeStamp;

			//
			// Check whether next log is obsolete
			//
			if(lCurrentLogTimeStamp > lNextLogTimeStamp)
			{
				lui8LogWriteIndex = gstUMtoLM_read.historyOrAnomalyIndex;
				return lui8LogWriteIndex;
			}
		}
	}

	return 0;
}
#endif

/******************************************************************************
 * Function Name: Logger_Module
 *
 * Function Description: void
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void Logger_Module(void)
{
	writeChangeSettingsHistory();
	writeAnomalyHistory();
	readAnomalyHistory();
	readChangeSettingsHistory();
	monitorResetLogParam();
}


uint8_t getLogIndex(PARAM_DISP AHorCSH_Idx)
{
	if(AHorCSH_Idx == ANOMHIST_START_IDX)
		return gui8_AnomalyHistory_ParamIdx;
	else
		return gui8_ChgSettHistory_ParamIdx;
}
