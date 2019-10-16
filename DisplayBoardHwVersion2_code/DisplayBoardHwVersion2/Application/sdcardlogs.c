/*********************************************************************************
* FileName: sdcardlogs.c
* Description:
* This header file contains definitions for SDCard Logs module.
* Version: 0.4D
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
 *  	0.5D	18/08/2014									Handled 'dumpresult' for Empty logs
 *  	0.4D	17/07/2014									Patch for ERROR text in Log
 *  	0.3D	02/07/2014									Remove Paramno if no logs. Show "NULL" instead
 *  	0.2D	10/06/2014									  Handled Odd Timestamp
 *  	0.1D	03/06/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  ToDo
 *  1) Add Control Param No handling
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "Middleware/rtc.h"
#include "grlib/grlib.h"
#include "Middleware/bolymindisplay.h"
#include "userinterface.h"

#include "interTaskCommunication.h"
#include "Middleware/paramdatabase.h"
#include "Middleware/sdcard.h"
#include "sdcardlogs.h"
#include "logger.h"
#include "Middleware/eeprom.h"
#include "Application/errormodule.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define MAX_LOG_LINES 4

/****************************************************************************
 *  Global variables
****************************************************************************/
extern const _stErrorList gstErrorList[TOTAL_ERRORS];
unsigned char gucfilewritebuff[MAX_CHARS_IN_LINE * MAX_LOG_LINES];
char gcfilenamestr[35];
/****************************************************************************/


/****************************************************************************
 *  Function definitions for this file:
****************************************************************************/


/******************************************************************************
 * Function Name: dumpLogsToSDCard
 *
 * Function Description:
 *
 * Function Parameters: ANOMHIST_START_IDX or CHGSETHIST_START_IDX
 *
 * Function Returns: 0 Success, 1 Dump Failed, 2 Empty Logs
 *
 ********************************************************************************/
#if 0
//	This function is commented as the logs dumped into SD card does not have the same sequence
//	as that is displayed in anomaly history.
//	Corrected function is below
int dumpLogsToSDCard(PARAM_DISP AHorCSH_Idx)
{
	uint8_t psize = guc_Paramsize_Disp[AHorCSH_Idx];
	uint8_t iSize, mSize;
	uint8_t param_EEP_Read_Val[_MAX_EEPROM_READVALSIZE];
	struct stControlAnomaly anomhistlog;
	struct stChangeSettingHistory chgsetglog;
	struct tm currDateTime, logDateTime;

	unsigned int retbytes;
	unsigned char writeResult;

	// Generate filename
	RTCGet(&currDateTime);
//	time_t testtime = umktime(&currDateTime);
	memset(gcfilenamestr, 0, sizeof(gcfilenamestr));
	if(AHorCSH_Idx == ANOMHIST_START_IDX)
		ustrncpy(gcfilenamestr, "log_anom\0", 9);
	else
		ustrncpy(gcfilenamestr, "log_chst\0", 9);

	usnprintf(gcfilenamestr, sizeof(gcfilenamestr), "%s%02u%02u%04u%02u%02u.txt\0", gcfilenamestr, currDateTime.tm_mday, currDateTime.tm_mon + 1, currDateTime.tm_year + 1900, currDateTime.tm_hour, currDateTime.tm_min);

	for (iSize = 0; iSize < _MAX_ANOMALY_LOGS; iSize++)
	{
		memset(param_EEP_Read_Val, 0, sizeof(param_EEP_Read_Val));
		if(!EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Disp[AHorCSH_Idx + iSize], psize))
		{
			for(mSize = 0; mSize < psize; mSize++)
			{
				// Check if Data present
				if(param_EEP_Read_Val[mSize])
				{
					// Data is present. So read again in structure in proper format.
					if(AHorCSH_Idx == ANOMHIST_START_IDX) {
						memset(&anomhistlog, 0, sizeof(anomhistlog));
						if(!(readParameterFromDB((PARAM_DISP)(AHorCSH_Idx + iSize), (uint8_t*)&anomhistlog) == _SUCCESS))
							return 1;
					}
					else {
						memset(&chgsetglog, 0, sizeof(chgsetglog));
						if(!(readParameterFromDB((PARAM_DISP)(AHorCSH_Idx + iSize), (uint8_t*)&chgsetglog) == _SUCCESS))
							return 1;
					}

					// After fetching AnomHist/Change Setting, write this to SDCard

					//----//Write First Line with Time information in txt file
					if(!iSize) {
						memset(gucfilewritebuff, 0, sizeof(gucfilewritebuff));
						usnprintf((char *)gucfilewritebuff, sizeof(gucfilewritebuff), "*** %02u-%02u-%04u %02u:%02u:%02u ***\r\n", currDateTime.tm_mday, currDateTime.tm_mon + 1, currDateTime.tm_year + 1900, currDateTime.tm_hour, currDateTime.tm_min, currDateTime.tm_sec);
						writeResult = write_SDCard(gcfilenamestr, gucfilewritebuff, &retbytes,FILE_OVERWRITE);

						if(writeResult != FR_OK)
							return 1;
						/*
						unsigned char testbuff[25] = "This is inside writeanom";
						fres = write_SDCard("panduwrite.txt", testbuff,&retbytes,FILE_OVERWRITE);
						 */
					}

					// Process Log Event timestamp
					memset(&logDateTime, 0, sizeof(logDateTime));
					if(AHorCSH_Idx == ANOMHIST_START_IDX) {
						ulocaltime(anomhistlog.timeStamp, &logDateTime);
					}
					else {
						ulocaltime(chgsetglog.timeStamp, &logDateTime);
					}

					memset(gucfilewritebuff, 0, sizeof(gucfilewritebuff));
					if(AHorCSH_Idx == ANOMHIST_START_IDX) {
//						usnprintf(gucfilewritebuff, sizeof(gucfilewritebuff), "%u,%u,%s,%02u%02u%04u%02u%02u%02u\r\n", iSize + 1, anomhistlog.anomalyCode, anomhistlog.errorDetails, logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, logDateTime.tm_hour, logDateTime.tm_min, logDateTime.tm_sec);

						usnprintf((char *)gucfilewritebuff, sizeof(gucfilewritebuff), "A%03u (LOG%u)\r\nAnomaly Code: E%03u\r\n%s\r\n%02u/%02u/%04u %02u:%02u:%02u\r\n\r\n", gui_ParamNo_Disp[(AHorCSH_Idx + iSize)], iSize + 1, anomhistlog.anomalyCode, anomhistlog.errorDetails, logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, logDateTime.tm_hour, logDateTime.tm_min, logDateTime.tm_sec);
					}
					else {
//						usnprintf(gucfilewritebuff, sizeof(gucfilewritebuff), "%u,%u,%02u%02u%04u%02u%02u%02u,%u,%u\r\n", iSize + 1, chgsetglog.parameterNumber, logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, logDateTime.tm_hour, logDateTime.tm_min, logDateTime.tm_sec, chgsetglog.oldValue, chgsetglog.newValue);
						usnprintf((char *)gucfilewritebuff, sizeof(gucfilewritebuff), "A%03u (LOG%u) A%03u\r\nOLD VALUE: %u\r\nNEW VALUE: %u\r\n%02u/%02u/%04u %02u:%02u:%02u,\r\n\r\n", gui_ParamNo_Disp[(AHorCSH_Idx + iSize)], iSize + 1, chgsetglog.parameterNumber, chgsetglog.oldValue, chgsetglog.newValue, logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, logDateTime.tm_hour, logDateTime.tm_min, logDateTime.tm_sec);
					}
					writeResult = write_SDCard(gcfilenamestr, gucfilewritebuff, &retbytes,FILE_APPEND_WRITE);

					if(writeResult != FR_OK)
						return 1;
					//Proceed for next Log
					break;
				}
			} //for loop

			//Break if empty log found. Dont proceed to next log since logs are entered sequentially.
			if(mSize == psize)
				break;
		} //if readParameter
	} //outer for loop

	if((mSize == psize) || (iSize == _MAX_ANOMALY_LOGS))
	{
		if(iSize == 0)
			return 2;
		else
			return 0;
	}
	else
		return 1;
}
#endif
#if 1
int dumpLogsToSDCard(PARAM_DISP AHorCSH_Idx)
{
	PARAM_DISP readIndex = AHorCSH_Idx;
	uint8_t psize = guc_Paramsize_Disp[AHorCSH_Idx];
	uint8_t iSize, mSize;
	uint8_t param_EEP_Read_Val[_MAX_EEPROM_READVALSIZE];
	struct stControlAnomaly anomhistlog;
	struct stChangeSettingHistory chgsetglog;
	struct tm currDateTime, logDateTime;

	PARAM_DISP Tp_ANOMHIST_START_IDX[20]={A030ANOMHIST_1,A031ANOMHIST_2,A032ANOMHIST_3,A033ANOMHIST_4,A034ANOMHIST_5,
		                                       A035ANOMHIST_6,A036ANOMHIST_7,A037ANOMHIST_8,A038ANOMHIST_9,A039ANOMHIST_10,
		                                       A900ANOMHIST_11,A901ANOMHIST_12,A902ANOMHIST_13,A903ANOMHIST_14,A904ANOMHIST_15,
					                           A905ANOMHIST_16,A906ANOMHIST_17,A907ANOMHIST_18,A908ANOMHIST_19,A909ANOMHIST_20};


	unsigned int retbytes;
	unsigned char writeResult;

	if(AHorCSH_Idx == ANOMHIST_START_IDX)
	{
	// Generate filename
	RTCGet(&currDateTime);
//	time_t testtime = umktime(&currDateTime);
	memset(gcfilenamestr, 0, sizeof(gcfilenamestr));
	if(AHorCSH_Idx == ANOMHIST_START_IDX)
	{
		ustrncpy(gcfilenamestr, "log_anom\0", 9);

		if(gui8_AnomalyHistory_ParamIdx > 1)
		{
			readIndex = (PARAM_DISP)Tp_ANOMHIST_START_IDX[gui8_AnomalyHistory_ParamIdx - 2];
			//readIndex --;
		}
		else
		{
			readIndex = (PARAM_DISP)Tp_ANOMHIST_START_IDX[ANOMHIST_END_IDX];
		}
	}
	else
	{
		ustrncpy(gcfilenamestr, "log_chst\0", 9);

		if(gui8_ChgSettHistory_ParamIdx > 1)
		{
			readIndex = (PARAM_DISP)(AHorCSH_Idx + gui8_ChgSettHistory_ParamIdx - 1);
			readIndex--;
		}
		else
		{
			readIndex = (PARAM_DISP)(CHGSETHIST_END_IDX);
		}
	}

	usnprintf(gcfilenamestr, sizeof(gcfilenamestr), "%s%02u%02u%04u%02u%02u.txt\0", gcfilenamestr, currDateTime.tm_mday, currDateTime.tm_mon + 1, currDateTime.tm_year + 1900, currDateTime.tm_hour, currDateTime.tm_min);


	for (iSize = 0; iSize < _MAX_ANOMALY_LOGS; iSize++)
	{
		/*if(readIndex >= (_MAX_ANOMALY_LOGS))
		{
			readIndex = AHorCSH_Idx;
		}*/

		memset(param_EEP_Read_Val, 0, sizeof(param_EEP_Read_Val));
		if(!EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Disp[readIndex], psize))
		{
			for(mSize = 0; mSize < psize; mSize++)
			{
				// Check if Data present
				if(param_EEP_Read_Val[mSize])
				{
					// Data is present. So read again in structure in proper format.
					if(AHorCSH_Idx == ANOMHIST_START_IDX) {
						memset(&anomhistlog, 0, sizeof(anomhistlog));
						if(!(readParameterFromDB(readIndex, (uint8_t*)&anomhistlog) == _SUCCESS))
							return 1;
					}
					else {
						memset(&chgsetglog, 0, sizeof(chgsetglog));
						if(!(readParameterFromDB(readIndex, (uint8_t*)&chgsetglog) == _SUCCESS))
							return 1;
					}

					// After fetching AnomHist/Change Setting, write this to SDCard

					//----//Write First Line with Time information in txt file
					if(!iSize) {
						memset(gucfilewritebuff, 0, sizeof(gucfilewritebuff));
						usnprintf((char *)gucfilewritebuff, sizeof(gucfilewritebuff),
								"*** %02u-%02u-%04u %02u:%02u:%02u ***\r\n", currDateTime.tm_mday,
								currDateTime.tm_mon + 1, currDateTime.tm_year + 1900, currDateTime.tm_hour,
								currDateTime.tm_min, currDateTime.tm_sec);
						writeResult = write_SDCard(gcfilenamestr, gucfilewritebuff, &retbytes,FILE_OVERWRITE);

						if(writeResult != FR_OK)
							return 1;
						/*
						unsigned char testbuff[25] = "This is inside writeanom";
						fres = write_SDCard("panduwrite.txt", testbuff,&retbytes,FILE_OVERWRITE);
						 */
					}

					// Process Log Event timestamp
					memset(&logDateTime, 0, sizeof(logDateTime));
					if(AHorCSH_Idx == ANOMHIST_START_IDX) {
						ulocaltime(anomhistlog.timeStamp, &logDateTime);
					}
					else {
						ulocaltime(chgsetglog.timeStamp, &logDateTime);
					}

					memset(gucfilewritebuff, 0, sizeof(gucfilewritebuff));
					if(AHorCSH_Idx == ANOMHIST_START_IDX) {
//						usnprintf(gucfilewritebuff, sizeof(gucfilewritebuff), "%u,%u,%s,%02u%02u%04u%02u%02u%02u\r\n", iSize + 1, anomhistlog.anomalyCode, anomhistlog.errorDetails, logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, logDateTime.tm_hour, logDateTime.tm_min, logDateTime.tm_sec);

						usnprintf((char *)gucfilewritebuff, sizeof(gucfilewritebuff),
								"A%03u (LOG%u)\r\nAnomaly Code: E%03u\r\n%s\r\n%02u/%02u/%04u %02u:%02u:%02u\r\n\r\n",
								gui_ParamNo_Disp[readIndex], iSize + 1, anomhistlog.anomalyCode, anomhistlog.errorDetails,
								logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, logDateTime.tm_hour,
								logDateTime.tm_min, logDateTime.tm_sec);
					}
					else {
//						usnprintf(gucfilewritebuff, sizeof(gucfilewritebuff), "%u,%u,%02u%02u%04u%02u%02u%02u,%u,%u\r\n", iSize + 1, chgsetglog.parameterNumber, logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, logDateTime.tm_hour, logDateTime.tm_min, logDateTime.tm_sec, chgsetglog.oldValue, chgsetglog.newValue);
						usnprintf((char *)gucfilewritebuff, sizeof(gucfilewritebuff),
								"A%03u (LOG%u) A%03u\r\nOLD VALUE: %u\r\nNEW VALUE: %u\r\n%02u/%02u/%04u %02u:%02u:%02u,\r\n\r\n",
								gui_ParamNo_Disp[readIndex], iSize + 1, chgsetglog.parameterNumber, chgsetglog.oldValue,
								chgsetglog.newValue, logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900,
								logDateTime.tm_hour, logDateTime.tm_min, logDateTime.tm_sec);
					}
					writeResult = write_SDCard(gcfilenamestr, gucfilewritebuff, &retbytes,FILE_APPEND_WRITE);

					if(AHorCSH_Idx == ANOMHIST_START_IDX)
					{
						if(readIndex == ANOMHIST_START_IDX)
						{
							readIndex = (PARAM_DISP)Tp_ANOMHIST_START_IDX[ANOMHIST_END_IDX];
						}
						else
						{
							if(readIndex == A900ANOMHIST_11)
							{
								readIndex = A039ANOMHIST_10;
							}
							else
							{
							readIndex--;
							}
						}
					}
					else
					{
						if(readIndex == CHGSETHIST_START_IDX)
						{
							readIndex = (PARAM_DISP)(CHGSETHIST_END_IDX);
						}
						else
						{
							readIndex--;
						}
					}
					//readIndex++;

					/*if(readIndex == (ANOMHIST_START_IDX))
					{
						readIndex = ANOMHIST_START_IDX + _MAX_ANOMALY_LOGS - 1;
					}*/

					if(writeResult != FR_OK)
						return 1;
					//Proceed for next Log
					break;
				}
			} //for loop

			//Break if empty log found. Dont proceed to next log since logs are entered sequentially.
			if(mSize == psize)
				break;
		} //if readParameter
	} //outer for loop

	if((mSize == psize) || (iSize == _MAX_ANOMALY_LOGS))
	{
		if(iSize == 0)
			return 2;
		else
			return 0;
	}
	else
		return 1;
	}

	if(AHorCSH_Idx == CHGSETHIST_START_IDX)
	{
		RTCGet(&currDateTime);
	//	time_t testtime = umktime(&currDateTime);
		memset(gcfilenamestr, 0, sizeof(gcfilenamestr));
		if(AHorCSH_Idx == ANOMHIST_START_IDX)
		{
			ustrncpy(gcfilenamestr, "log_anom\0", 9);

			if(gui8_AnomalyHistory_ParamIdx > 1)
			{
				readIndex = (PARAM_DISP)(gui8_AnomalyHistory_ParamIdx - 1);
				readIndex--;
			}
			else
			{
				readIndex = (PARAM_DISP)(ANOMHIST_END_IDX);
			}
		}
		else
		{
			ustrncpy(gcfilenamestr, "log_chst\0", 9);

			if(gui8_ChgSettHistory_ParamIdx > 1)
			{
				readIndex = (PARAM_DISP)(AHorCSH_Idx + gui8_ChgSettHistory_ParamIdx - 1);
				readIndex--;
			}
			else
			{
				readIndex = (PARAM_DISP)(CHGSETHIST_END_IDX);
			}
		}

		usnprintf(gcfilenamestr, sizeof(gcfilenamestr), "%s%02u%02u%04u%02u%02u.txt\0", gcfilenamestr, currDateTime.tm_mday, currDateTime.tm_mon + 1, currDateTime.tm_year + 1900, currDateTime.tm_hour, currDateTime.tm_min);


		for (iSize = 0; iSize < _MAX_ANOMALY_LOGS; iSize++)
		{
			/*if(readIndex >= (_MAX_ANOMALY_LOGS))
			{
				readIndex = AHorCSH_Idx;
			}*/

			memset(param_EEP_Read_Val, 0, sizeof(param_EEP_Read_Val));
			if(!EEPROMReadByte(param_EEP_Read_Val, gui_ParamAddr_Disp[readIndex], psize))
			{
				for(mSize = 0; mSize < psize; mSize++)
				{
					// Check if Data present
					if(param_EEP_Read_Val[mSize])
					{
						// Data is present. So read again in structure in proper format.
						if(AHorCSH_Idx == ANOMHIST_START_IDX) {
							memset(&anomhistlog, 0, sizeof(anomhistlog));
							if(!(readParameterFromDB(readIndex, (uint8_t*)&anomhistlog) == _SUCCESS))
								return 1;
						}
						else {
							memset(&chgsetglog, 0, sizeof(chgsetglog));
							if(!(readParameterFromDB(readIndex, (uint8_t*)&chgsetglog) == _SUCCESS))
								return 1;
						}

						// After fetching AnomHist/Change Setting, write this to SDCard

						//----//Write First Line with Time information in txt file
						if(!iSize) {
							memset(gucfilewritebuff, 0, sizeof(gucfilewritebuff));
							usnprintf((char *)gucfilewritebuff, sizeof(gucfilewritebuff),
									"*** %02u-%02u-%04u %02u:%02u:%02u ***\r\n", currDateTime.tm_mday,
									currDateTime.tm_mon + 1, currDateTime.tm_year + 1900, currDateTime.tm_hour,
									currDateTime.tm_min, currDateTime.tm_sec);
							writeResult = write_SDCard(gcfilenamestr, gucfilewritebuff, &retbytes,FILE_OVERWRITE);

							if(writeResult != FR_OK)
								return 1;
							/*
							unsigned char testbuff[25] = "This is inside writeanom";
							fres = write_SDCard("panduwrite.txt", testbuff,&retbytes,FILE_OVERWRITE);
							 */
						}

						// Process Log Event timestamp
						memset(&logDateTime, 0, sizeof(logDateTime));
						if(AHorCSH_Idx == ANOMHIST_START_IDX) {
							ulocaltime(anomhistlog.timeStamp, &logDateTime);
						}
						else {
							ulocaltime(chgsetglog.timeStamp, &logDateTime);
						}

						memset(gucfilewritebuff, 0, sizeof(gucfilewritebuff));
						if(AHorCSH_Idx == ANOMHIST_START_IDX) {
	//						usnprintf(gucfilewritebuff, sizeof(gucfilewritebuff), "%u,%u,%s,%02u%02u%04u%02u%02u%02u\r\n", iSize + 1, anomhistlog.anomalyCode, anomhistlog.errorDetails, logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, logDateTime.tm_hour, logDateTime.tm_min, logDateTime.tm_sec);

							usnprintf((char *)gucfilewritebuff, sizeof(gucfilewritebuff),
									"A%03u (LOG%u)\r\nAnomaly Code: E%03u\r\n%s\r\n%02u/%02u/%04u %02u:%02u:%02u\r\n\r\n",
									gui_ParamNo_Disp[readIndex], iSize + 1, anomhistlog.anomalyCode, anomhistlog.errorDetails,
									logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, logDateTime.tm_hour,
									logDateTime.tm_min, logDateTime.tm_sec);
						}
						else {
	//						usnprintf(gucfilewritebuff, sizeof(gucfilewritebuff), "%u,%u,%02u%02u%04u%02u%02u%02u,%u,%u\r\n", iSize + 1, chgsetglog.parameterNumber, logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, logDateTime.tm_hour, logDateTime.tm_min, logDateTime.tm_sec, chgsetglog.oldValue, chgsetglog.newValue);
							usnprintf((char *)gucfilewritebuff, sizeof(gucfilewritebuff),
									"A%03u (LOG%u) A%03u\r\nOLD VALUE: %u\r\nNEW VALUE: %u\r\n%02u/%02u/%04u %02u:%02u:%02u,\r\n\r\n",
									gui_ParamNo_Disp[readIndex], iSize + 1, chgsetglog.parameterNumber, chgsetglog.oldValue,
									chgsetglog.newValue, logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900,
									logDateTime.tm_hour, logDateTime.tm_min, logDateTime.tm_sec);
						}
						writeResult = write_SDCard(gcfilenamestr, gucfilewritebuff, &retbytes,FILE_APPEND_WRITE);

						if(AHorCSH_Idx == ANOMHIST_START_IDX)
						{
							if(readIndex == ANOMHIST_START_IDX)
							{
								readIndex = (PARAM_DISP)(ANOMHIST_END_IDX);
							}
							else
							{
								readIndex--;
							}
						}
						else
						{
							if(readIndex == CHGSETHIST_START_IDX)
							{
								readIndex = (PARAM_DISP)(CHGSETHIST_END_IDX);
							}
							else
							{
								readIndex--;
							}
						}
						//readIndex++;

						/*if(readIndex == (ANOMHIST_START_IDX))
						{
							readIndex = ANOMHIST_START_IDX + _MAX_ANOMALY_LOGS - 1;
						}*/

						if(writeResult != FR_OK)
							return 1;
						//Proceed for next Log
						break;
					}
				} //for loop

				//Break if empty log found. Dont proceed to next log since logs are entered sequentially.
				if(mSize == psize)
					break;
			} //if readParameter
		} //outer for loop

		if((mSize == psize) || (iSize == _MAX_ANOMALY_LOGS))
		{
			if(iSize == 0)
				return 2;
			else
				return 0;
		}
		else
			return 1;

	}

}
#endif

/***********************************************************************************/

/******************************************************************************
 * Function Name: showLogs
 *
 * Function Description:
 *
 * Prerequisites: readAnomalyHistory() / readChangeSettingsHistory()
 *
 * Function Parameters: ANOMHIST_START_IDX or CHGSETHIST_START_IDX
 *
 * Function Returns: None
 *
 ********************************************************************************/
//void showLogs(PARAM_DISP AHorCSH_Idx)
//{
////	tRectangle sRect;
////	tContext sContextBlack;
//	uint8_t viewbuff[MAX_CHARS_IN_LINE];
//	//char lBuff_cyw[MAX_CHARS_IN_LINE];
//	struct tm logDateTime;
//	uint8_t logReadIndex;
////	uint8_t Tp_datalong;
//
//	//
//	// Clear Screen.
//	//
//	GrRectFIllBolymin(0, 126, 0, 63, true, true);
//
//	// Process Log Event timestamp
//	memset(&logDateTime, 0, sizeof(logDateTime));
//	if(AHorCSH_Idx == ANOMHIST_START_IDX) {
//		ulocaltime(gstUMtoLM_read.anomalyHistory.timeStamp, &logDateTime);
//
//		logReadIndex = (gstUMtoLM_read.historyOrAnomalyIndex > (getLogIndex(ANOMHIST_START_IDX)-1)) ? (_MAX_ANOMALY_LOGS - (gstUMtoLM_read.historyOrAnomalyIndex - getLogIndex(ANOMHIST_START_IDX))) : (getLogIndex(ANOMHIST_START_IDX) - gstUMtoLM_read.historyOrAnomalyIndex);
//
////		usnprintf((char *)viewbuff, sizeof(viewbuff), "A%03u (LOG %u)", gui_ParamNo_Disp[(logReadIndex - 1 + ANOMHIST_START_IDX)], gstUMtoLM_read.historyOrAnomalyIndex);
////		displayText(viewbuff, 0, 0, false, false, false, false);
//		usnprintf((char *)viewbuff, sizeof(viewbuff), "ANOMALY CODE:E%03u", gstUMtoLM_read.anomalyHistory.anomalyCode);
////		usnprintf(viewbuff, sizeof(viewbuff), "Anomaly1234567");
//	//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
//		//usnprintf_nU_cyw((char*)(lBuff_cyw),17,(char*)(viewbuff));
//		//displayText(viewbuff, 2, 16, false, false, false, false, false, true);
//		displayText(viewbuff, 2, 16, false, false, false, false, false, true);
//
//		//Handled Odd Timestamp
//		if(umktime(&logDateTime) != (unsigned long)-1) {
//			uint8_t disphour;
//			char ampm[3];
//
//			if(logDateTime.tm_hour > 11)
//				ustrncpy(ampm, "PM\0", 3);
//			else
//				ustrncpy(ampm, "AM\0", 3);
//
//			disphour = logDateTime.tm_hour;
//			if(logDateTime.tm_hour > 12)
//				disphour -= 12;
//			else if(logDateTime.tm_hour == 0)
//				disphour += 12;
//
//#if 1	// Display error details
//
//			usnprintf((char *)viewbuff, sizeof(viewbuff), "%s", gstUMtoLM_read.anomalyHistory.errorDetails);
//			displayText(viewbuff, 2, 32, false, false, false, false, false, false);
//#endif
//
//			usnprintf((char *)viewbuff, sizeof(viewbuff), "%02u/%02u/%04u %02u:%02u %s", logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, /*logDateTime.tm_hour*/disphour, logDateTime.tm_min, /*logDateTime.tm_sec*/ampm);
//			//		usnprintf(viewbuff, sizeof(viewbuff), "Timestamp");
//			//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
//			//usnprintf_nU_cyw((char*)(lBuff_cyw),19,(char*)(viewbuff));
//			//displayText(viewbuff, 2, 48, false, false, false, false, false, false);
//			displayText(viewbuff, 2, 48, false, false, false, false, false, true);
//
//			usnprintf((char *)viewbuff, sizeof(viewbuff), "A%03u (LOG %u)", gui_ParamNo_Disp[(logReadIndex - 1 + ANOMHIST_START_IDX)], gstUMtoLM_read.historyOrAnomalyIndex);
//			//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
//			//usnprintf_nU_cyw((char*)(lBuff_cyw),12,(char*)(viewbuff));
//			//displayText(viewbuff, 2, 0, false, false, false, false, false, false);
//			displayText(viewbuff, 2, 0, false, false, false, false, false, false);
//		}
//		// Added - 2 Jul14 - No log found
//		else {
//			usnprintf((char *)viewbuff, sizeof(viewbuff), "A%03u (LOG%u)-NULL", gui_ParamNo_Disp[(logReadIndex - 1 + ANOMHIST_START_IDX)], gstUMtoLM_read.historyOrAnomalyIndex);
//		//	memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
//			//usnprintf_nU_cyw((char*)(lBuff_cyw),16,(char*)(viewbuff));
//			displayText(viewbuff, 1, 0, false, false, false, false, false, false);
//		}
//
//	}
//	else {
//		ulocaltime(gstUMtoLM_read.changeSettingHistory.timeStamp, &logDateTime);
//
//		logReadIndex = (gstUMtoLM_read.historyOrAnomalyIndex > (getLogIndex(CHGSETHIST_START_IDX)-1)) ? (_MAX_ANOMALY_LOGS - (gstUMtoLM_read.historyOrAnomalyIndex - getLogIndex(CHGSETHIST_START_IDX))) : (getLogIndex(CHGSETHIST_START_IDX) - gstUMtoLM_read.historyOrAnomalyIndex);
//
////		usnprintf((char *)viewbuff, sizeof(viewbuff), "A%03u (LOG%u)-A%03u", gui_ParamNo_Disp[(logReadIndex - 1 + CHGSETHIST_START_IDX)], gstUMtoLM_read.historyOrAnomalyIndex, gstUMtoLM_read.changeSettingHistory.parameterNumber);
////		displayText(viewbuff, 0, 0, false, false, false, false);
//		//gstUMtoLM_read.changeSettingHistory.oldValue= 2333;
//
//		usnprintf((char *)viewbuff, sizeof(viewbuff), "OLD VALUE:%u", gstUMtoLM_read.changeSettingHistory.oldValue);
//		//		usnprintf(viewbuff, sizeof(viewbuff), "Anomaly1234567");
//
//
//		//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
//		//Tp_datalong = data_long_cyw(gstUMtoLM_read.changeSettingHistory.oldValue);
//		//memcpy((char*)lBuff_cyw,(char*)viewbuff,20);
//		//usnprintf_nU_cyw((char*)(lBuff_cyw+20),Tp_datalong,(char*)(viewbuff+20));
//
//		//displayText(viewbuff, 2, 16, false, false, false, false, false, false);
//		displayText(viewbuff, 2, 16, false, false, false, false, true, false);
//
//		//gstUMtoLM_read.changeSettingHistory.newValue = 2334;
//		usnprintf((char *)viewbuff, sizeof(viewbuff), "NEW VALUE:%u", gstUMtoLM_read.changeSettingHistory.newValue);
//		//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
//		//Tp_datalong = data_long_cyw(gstUMtoLM_read.changeSettingHistory.newValue);
//		//memcpy((char*)lBuff_cyw,(char*)viewbuff,20);
//		//usnprintf_nU_cyw((char*)(lBuff_cyw+20),Tp_datalong,(char*)(viewbuff+20));
//		//		usnprintf(viewbuff, sizeof(viewbuff), "Anomaly1234567");
//		//displayText(viewbuff, 2, 32, false, false, false, false, false, false);
//		displayText(viewbuff, 2, 32, false, false, false, false, true, false);
//		//Handled Odd Timestamp
//		if(umktime(&logDateTime) != (unsigned long)-1) {
//			int disphour;
//				char ampm[3];
//
//				if(logDateTime.tm_hour > 11)
//					ustrncpy(ampm, "PM\0", 3);
//				else
//					ustrncpy(ampm, "AM\0", 3);
//
//				disphour = logDateTime.tm_hour;
//				if(logDateTime.tm_hour > 12)
//					disphour -= 12;
//				else if(logDateTime.tm_hour == 0)
//					disphour += 12;
//
//			usnprintf((char *)viewbuff, sizeof(viewbuff), "%02u/%02u/%04u %02u:%02u %s", logDateTime.tm_mday, logDateTime.tm_mon + 1, logDateTime.tm_year + 1900, /*logDateTime.tm_hour*/disphour, logDateTime.tm_min, /*logDateTime.tm_sec*/ampm);
//			//		usnprintf(viewbuff, sizeof(viewbuff), "Timestamp");
//			//displayText(viewbuff, 2, 48, false, false, false, false, false, false);
//			//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
//			//usnprintf_nU_cyw((char*)(lBuff_cyw),19,(char*)(viewbuff));
//			displayText(viewbuff, 2, 48, false, false, false, false, false, true);
//
//			usnprintf((char *)viewbuff, sizeof(viewbuff), "A%03u (LOG%u)-A%03u", gui_ParamNo_Disp[(logReadIndex - 1 + CHGSETHIST_START_IDX)], gstUMtoLM_read.historyOrAnomalyIndex, gstUMtoLM_read.changeSettingHistory.parameterNumber);
//			//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
//			//			usnprintf_nU_cyw((char*)(lBuff_cyw),16,(char*)(viewbuff));
//
//			//displayText(viewbuff, 2, 0, false, false, false, false, ture, false);
//						displayText(viewbuff, 2, 0, false, false, false, false, false, true);
//		}
//		// Added - 2 Jul14 - No log found
//		else {
//			usnprintf((char *)viewbuff, sizeof(viewbuff), "A%03u (LOG%u)-NULL", gui_ParamNo_Disp[(logReadIndex - 1 + CHGSETHIST_START_IDX)], gstUMtoLM_read.historyOrAnomalyIndex);
//			//memset(lBuff_cyw,0x20,sizeof(lBuff_cyw));
//			//usnprintf_nU_cyw((char*)(lBuff_cyw),16,(char*)(viewbuff));
//			//displayText(viewbuff, 2, 0, false, false, false, false, false, false);
//			displayText(viewbuff, 1, 0, false, false, false, false, false, false);
//		}
//
//	}
//
//}
void showLogs(PARAM_DISP AHorCSH_Idx)
{

	uint8_t viewbuff[MAX_CHARS_IN_LINE];
	uint8_t lBuff_cyw[MAX_CHARS_IN_LINE]={0};
	const unsigned char jiantou[3] = "->";
	const unsigned char para_setting_japanese[17] = "<セッテイ_リレキ";
	const unsigned char fankuo[2] =">";
	const unsigned char canshu_japanese[12]="パラメータ:";
	const unsigned char canshu_null_japanese[16]="パラメータ:NULL";
	const unsigned char para_error_japanese[14] ="<エラー_リレキ";
	const unsigned char cuowu_japanese[14]="エラーコード:";
	const unsigned char cuowu_null_japanese[18]="エラーコード:NULL";

	const unsigned char para_setting_english[17] = "<SETTING HISTORY";

	const unsigned char canshu_english[14]="SETTING CODE:";
	const unsigned char canshu_null_english[18]="SETTING CODE:NULL";
	const unsigned char para_error_english[16] ="<ANOMALY HISTORY";
	const unsigned char cuowu_english[14]="ERROR CODE:";
	const unsigned char cuowu_null_english[18]="ERROR CODE:NULL";
	struct tm logDateTime;
	uint8_t logReadIndex;
	uint8_t Tp_ANOMHIST_START_IDX[20]={A030ANOMHIST_1,A031ANOMHIST_2,A032ANOMHIST_3,A033ANOMHIST_4,A034ANOMHIST_5,
	                                       A035ANOMHIST_6,A036ANOMHIST_7,A037ANOMHIST_8,A038ANOMHIST_9,A039ANOMHIST_10,
	                                       A900ANOMHIST_11,A901ANOMHIST_12,A902ANOMHIST_13,A903ANOMHIST_14,A904ANOMHIST_15,
				                           A905ANOMHIST_16,A906ANOMHIST_17,A907ANOMHIST_18,A908ANOMHIST_19,A909ANOMHIST_20};
	uint8_t Tp_error;
	//
	// Clear Screen.
	//
	GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);

	// Process Log Event timestamp
	memset(&logDateTime, 0, sizeof(logDateTime));
	if(AHorCSH_Idx == ANOMHIST_START_IDX) {
		ulocaltime(gstUMtoLM_read.anomalyHistory.timeStamp, &logDateTime);

		logReadIndex = (gstUMtoLM_read.historyOrAnomalyIndex > (getLogIndex(ANOMHIST_START_IDX)-1)) ? (_MAX_ANOMALY_LOGS - (gstUMtoLM_read.historyOrAnomalyIndex - getLogIndex(ANOMHIST_START_IDX))) : (getLogIndex(ANOMHIST_START_IDX) - gstUMtoLM_read.historyOrAnomalyIndex);
		  memset(lBuff_cyw,0,sizeof(lBuff_cyw));
		usnprintf((char *)viewbuff, sizeof(viewbuff), "ANOMALY CODE:E%03u", gstUMtoLM_read.anomalyHistory.anomalyCode);
       // displayText(viewbuff, 2, 16, false, false, false, false, false, true);
	 if(gu8_language == Japanese_IDX)
	   memcpy(lBuff_cyw,cuowu_japanese,14);
	 else
	   memcpy(lBuff_cyw,cuowu_english,14);

       memcpy(lBuff_cyw+strlen(lBuff_cyw),viewbuff+13,5);
       if(gu8_language == Japanese_IDX)
       displayText(lBuff_cyw, 2, 16, false, false, false, false, false, false);
       else
       displayText(lBuff_cyw, 2, 16, false, false, false, false, false, true);
		//Handled Odd Timestamp
		if(umktime(&logDateTime) != (unsigned long)-1) {
			uint8_t disphour;
			char ampm[3];

			if(logDateTime.tm_hour > 11)
				ustrncpy(ampm, "PM\0", 3);
			else
				ustrncpy(ampm, "AM\0", 3);

			disphour = logDateTime.tm_hour;
			if(logDateTime.tm_hour > 12)
				disphour -= 12;
			else if(logDateTime.tm_hour == 0)
				disphour += 12;

#if 1	// Display error details
			for(Tp_error = 0;Tp_error<TOTAL_ERRORS;Tp_error++)
			{
				if(gstErrorList[Tp_error].errorCode == gstUMtoLM_read.anomalyHistory.anomalyCode)
				{
					 if(gu8_language == Japanese_IDX)
					 {
						 memcpy(gstUMtoLM_read.anomalyHistory.errorDetails,gstErrorList[Tp_error].errorName_japanese,30);
					 }
					 else
					 {
						 memcpy(gstUMtoLM_read.anomalyHistory.errorDetails,gstErrorList[Tp_error].errorName_english,30);
					 }
					 break;
				}
			}

			usnprintf((char *)viewbuff, sizeof(viewbuff), "%s", gstUMtoLM_read.anomalyHistory.errorDetails);
			 if(gu8_language == Japanese_IDX)
			displayText(viewbuff, 2, 32, false, false, false, false, false, false);
			 else
			displayText(viewbuff, 2, 32, false, false, false, false, false, true);
#endif

			usnprintf((char *)viewbuff, sizeof(viewbuff), "%04u/%02u/%02u %s %02u:%02u", logDateTime.tm_year + 1900,logDateTime.tm_mon + 1, logDateTime.tm_mday,   /*logDateTime.tm_sec*/ampm,/*logDateTime.tm_hour*/disphour, logDateTime.tm_min);
			displayText(viewbuff, 2, 48, false, false, false, false, false, true);

			usnprintf((char *)viewbuff, sizeof(viewbuff), "A%03u (LOG %u)", gui_ParamNo_Disp[Tp_ANOMHIST_START_IDX[logReadIndex - 1]], gstUMtoLM_read.historyOrAnomalyIndex);
			//displayText(viewbuff, 2, 0, false, false, false, false, false, false);
			  memset(lBuff_cyw,0,sizeof(lBuff_cyw));
			  if(gu8_language == Japanese_IDX)
			  memcpy(lBuff_cyw,para_error_japanese,14);
			  else
			  memcpy(lBuff_cyw,para_error_english,16);
			  memcpy(lBuff_cyw+strlen(lBuff_cyw),viewbuff+10,strlen(viewbuff)-11);
			  memcpy(lBuff_cyw+strlen(lBuff_cyw),fankuo,1);
			  displayText(lBuff_cyw, 2, 0, false, false, false, false, false, false);
		}
		// Added - 2 Jul14 - No log found
		else {
			GrRectFIllBolymin(0, 126, 0, 63, 0, true);
			usnprintf((char *)viewbuff, sizeof(viewbuff), "A%03u (LOG%u)-NULL", gui_ParamNo_Disp[Tp_ANOMHIST_START_IDX[logReadIndex - 1]], gstUMtoLM_read.historyOrAnomalyIndex);
		//	displayText(viewbuff, 1, 0, false, false, false, false, false, false);
			 memset(lBuff_cyw,0,sizeof(lBuff_cyw));
			 if(gu8_language == Japanese_IDX)
			 memcpy(lBuff_cyw,para_error_japanese,14);
			 else
		     memcpy(lBuff_cyw,para_error_english,16);

			 memcpy(lBuff_cyw+strlen(lBuff_cyw),viewbuff+9,strlen(viewbuff)-15);
			 memcpy(lBuff_cyw+strlen(lBuff_cyw),fankuo,1);
			 if(gu8_language == Japanese_IDX)
			 displayText(lBuff_cyw, 2, 0, false, false, false, false, false, false);
			 else
			 displayText(lBuff_cyw, 2, 0, false, false, false, false, false, true);
			 memset(lBuff_cyw,0,sizeof(lBuff_cyw));
			 if(gu8_language == Japanese_IDX)
			 memcpy(lBuff_cyw,cuowu_null_japanese,18);
			 else
			 memcpy(lBuff_cyw,cuowu_null_english,18);
			 if(gu8_language == Japanese_IDX)
			 displayText(lBuff_cyw, 2, 16, false, false, false, false, false, false);
			 else
			 displayText(lBuff_cyw, 2, 16, false, false, false, false, false, true);
		}

	}
	else {
		ulocaltime(gstUMtoLM_read.changeSettingHistory.timeStamp, &logDateTime);

		logReadIndex = (gstUMtoLM_read.historyOrAnomalyIndex > (getLogIndex(CHGSETHIST_START_IDX)-1)) ? (_MAX_CHGSETHIST_LOGS - (gstUMtoLM_read.historyOrAnomalyIndex - getLogIndex(CHGSETHIST_START_IDX))) : (getLogIndex(CHGSETHIST_START_IDX) - gstUMtoLM_read.historyOrAnomalyIndex);

        memset(lBuff_cyw,0,sizeof(lBuff_cyw));
		usnprintf((char *)viewbuff, sizeof(viewbuff), "OLD VALUE:%u", gstUMtoLM_read.changeSettingHistory.oldValue);
		//displayText(viewbuff, 2, 16, false, false, false, false, true, false);
        memcpy(lBuff_cyw,viewbuff+10, strlen(viewbuff)-10);
        memcpy(lBuff_cyw+strlen(lBuff_cyw),jiantou, 2);
		usnprintf((char *)viewbuff, sizeof(viewbuff), "NEW VALUE:%u", gstUMtoLM_read.changeSettingHistory.newValue);
		//displayText(viewbuff, 2, 32, false, false, false, false, true, false);
		 memcpy(lBuff_cyw+strlen(lBuff_cyw),viewbuff+10, strlen(viewbuff)-10);
		 displayText(lBuff_cyw, 2, 32, false, false, false, false, false, true);
		//Handled Odd Timestamp
		if(umktime(&logDateTime) != (unsigned long)-1) {
			int disphour;
				char ampm[3];

				if(logDateTime.tm_hour > 11)
					ustrncpy(ampm, "PM\0", 3);
				else
					ustrncpy(ampm, "AM\0", 3);

				disphour = logDateTime.tm_hour;
				if(logDateTime.tm_hour > 12)
					disphour -= 12;
				else if(logDateTime.tm_hour == 0)
					disphour += 12;

			usnprintf((char *)viewbuff, sizeof(viewbuff), "%04u/%02u/%02u %s %02u:%02u",   logDateTime.tm_year + 1900,logDateTime.tm_mon + 1,logDateTime.tm_mday, ampm,/*logDateTime.tm_hour*/disphour, logDateTime.tm_min /*logDateTime.tm_sec*/);
			displayText(viewbuff, 2, 48, false, false, false, false, false, true);

			 memset(lBuff_cyw,0,sizeof(lBuff_cyw));
             memset(viewbuff,0,sizeof(viewbuff));
			usnprintf((char *)viewbuff, sizeof(viewbuff), "A%03u (LOG%u)-A%03u", gui_ParamNo_Disp[(logReadIndex - 1 + CHGSETHIST_START_IDX)], gstUMtoLM_read.historyOrAnomalyIndex, gstUMtoLM_read.changeSettingHistory.parameterNumber);
			 if(gu8_language == Japanese_IDX)
			memcpy(lBuff_cyw,para_setting_japanese,17);
			 else
			 memcpy(lBuff_cyw,para_setting_english,17);
			memcpy(lBuff_cyw+strlen(lBuff_cyw),viewbuff+9,strlen(viewbuff)-15);
			memcpy(lBuff_cyw+strlen(lBuff_cyw),fankuo,1);
			 if(gu8_language == Japanese_IDX)
			displayText(lBuff_cyw, 2, 0, false, false, false, false, false, false);
			 else
			displayText(lBuff_cyw, 2, 0, false, false, false, false, false, true);
			memset(lBuff_cyw,0,sizeof(lBuff_cyw));
			 if(gu8_language == Japanese_IDX)
			memcpy(lBuff_cyw,canshu_japanese,12);
			 else
			 memcpy(lBuff_cyw,canshu_english,14);
			memcpy(lBuff_cyw+strlen(lBuff_cyw),viewbuff+strlen(viewbuff)-4,4);
			 if(gu8_language == Japanese_IDX)
			displayText(lBuff_cyw, 2, 16, false, false, false, false, false, false);
			 else
		    displayText(lBuff_cyw, 2, 16, false, false, false, false, false, true);
		}
		// Added - 2 Jul14 - No log found
		else {
			GrRectFIllBolymin(0, 126, 0, 63, 0, true);
			usnprintf((char *)viewbuff, sizeof(viewbuff), "A%03u (LOG%u)-NULL", gui_ParamNo_Disp[(logReadIndex - 1 + CHGSETHIST_START_IDX)], gstUMtoLM_read.historyOrAnomalyIndex);
			//displayText(viewbuff, 1, 0, false, false, false, false, false, false);
			 memset(lBuff_cyw,0,sizeof(lBuff_cyw));
			 if(gu8_language == Japanese_IDX)
			 memcpy(lBuff_cyw,para_setting_japanese,17);
			 else
			 memcpy(lBuff_cyw,para_setting_english,17);
			 memcpy(lBuff_cyw+strlen(lBuff_cyw),viewbuff+9,strlen(viewbuff)-15);
			 memcpy(lBuff_cyw+strlen(lBuff_cyw),fankuo,1);
			 displayText(lBuff_cyw, 2, 0, false, false, false, false, false, false);
			 memset(lBuff_cyw,0,sizeof(lBuff_cyw));
			 if(gu8_language == Japanese_IDX)
			 memcpy(lBuff_cyw,canshu_null_japanese,16);
			 else
			 memcpy(lBuff_cyw,canshu_null_english,18);
			 if(gu8_language == Japanese_IDX)
			 displayText(lBuff_cyw, 2, 16, false, false, false, false, false, false);
			 else
			 displayText(lBuff_cyw, 2, 16, false, false, false, false, false, true);

		}

	}

}
