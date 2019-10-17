/*********************************************************************************
 * FileName: anomalyhistory.c
 * Description: Code for displaying 'Display Anomaly Logs' screen
 * Version: 0.1D
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
 *  	0.1D	09/06/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/gpio.h>
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "grlib/grlib.h"
//#include "Middleware/cfal96x64x16.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "userinterface.h"

#include "Middleware/paramdatabase.h"
#include "sdcardlogs.h"
#include "logger.h"
#include "ledhandler.h"
#include "Application/intertaskcommunication.h"
#include "communicationmodule.h"
#include "errormodule.h"


extern _CommunicationModuleInnerTaskComm lstCommunicationModuleInnerTaskComm;
/****************************************************************************/

/****************************************************************************
 *  Macros:
****************************************************************************/
#define ANOMALY_LIST_LEN 20
#define MAX_STRING_LEN	 30

// Error Codes
#define	_ERR_LIST_EMPTY				1
#define	_ERR_ENTRY_NOT_FOUND		2
#define	_ERR_ENTRY_ALREADY_EXISTS	3

bool bFatalErrorOccurred = false;
extern uint8_t KEY_PRESS_3SEC_ENT_FLAG_CYW;
extern const _stErrorList gstErrorList[TOTAL_ERRORS];
extern uint8_t KEY_PRESS_3SEC_STOP_FLAG_CYW_1; ////20170414      201703_No.27
/****************************************************************************
 *  Structures:
****************************************************************************/
struct errorDB gsActiveAnomalyList[20] = {0};
/*
{
		{100, "DRIVE COMM FAIL", eRECOVERABLE_ERROR},
		{101, "UART ERR", eRECOVERABLE_ERROR},
		{102, "MTR OPEN PHASE", eRECOVERABLE_ERROR},
		{001, "PACKET CRC ERR", eRECOVERABLE_ERROR},
		{002, "DRIVE COMM FAIL", eRECOVERABLE_ERROR},
		{003, "RELAY COMM FAIL", eRECOVERABLE_ERROR},
		{107, "DRV PFC SHUTDWN", eRECOVERABLE_ERROR},
		{004, "CONTR UART ERR", eRECOVERABLE_ERROR},
		{005, "OPR RESTRICT", eRECOVERABLE_ERROR},
		{006, "STARTUP ERR", eRECOVERABLE_ERROR}
};
*/

/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
uint8_t gui8ActiveAnomalyWriteIndex = 0;
uint8_t open_disable_enable_cyw = 0;//0-enable 1-disable
uint8_t close_disable_enable_cyw = 0;//0-enable 1-disable

/****************************************************************************/

/******************************************************************************
 * FunctionName: displayAnomalies
 *
 * Function Description:
 * This function display recoverable and non-recoverable errors on the fourth
 * line of display. It also displays a fatal error message when a non-recoverable
 * error is encountered. This function should be called from functional block's
 * runtime function.
 *
 * Function Parameters: None
 *
 * Function Returns: void
 *
 ********************************************************************************/
void displayAnomalies(void)
{
	static uint8_t lsAnomalyIndex = 0;
	static uint8_t lsDelay3SecStart = 0;
	static uint32_t lsTickCount3Seconds = 0;

	unsigned char lbuff[31];
	unsigned char CCCC[6]={0};
	uint8_t tp_i = 0;
//	unsigned char  lbuff_cyw[41];

	if(lsAnomalyIndex < 20)
	{
		if(0 == lsDelay3SecStart)
		{

			if(gsActiveAnomalyList[lsAnomalyIndex].errorCode != 0)
			{
				//
				// Check for fatal error.
				//
				if( (false == bFatalErrorOccurred ) &&
					(  (1 == gstDriveBoardStatus.bits.driveFaultUnrecoverable)	||
					   (1 == gstControlBoardStatus.bits.controlFaultUnrecoverable)	||
					   (1 == gstDisplayBoardStatus.bits.displayFaultUnrecoverable)
					)
				  )
				{
					//
					// Clear screen
					//
					//GrRectFIllBolymin(0, 126, 0, 63, true, true);
					GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);



					if(gu8_language == Japanese_IDX)
					{
					//
					// Paint screen title
					//
					displayText("エラーハッセイ!", 2, 0, true, true, false, false, false, false);
					GrLineDrawHorizontalBolymin(0, 126, 15, false);	// line draw function

					//
					// Paint fatal error message
					//
					//displayText(" S H U T T E R   S Y S T E M   S T O P", 2, 16, false, false, false, false);
					displayText("シャッターシステム STOP", 2, 16, false, false, false, false,false, false);
					//displayText("RESTART THE SYSTEM", 2, 32, false, false, false, false, false, true);
					displayText("リスタートシテクダサイ", 2, 32, false, false, false, false, false, false);
					//
					// Set fatal error occurred flag
					//

					 }
					else
					{
						//
						// Paint screen title
						//
						displayText("FATAL ERRORS!", 20, 0, true, true, false, false, false,true);
						GrLineDrawHorizontalBolymin(0, 126, 15, false);	// line draw function

						//
						// Paint fatal error message
						//
						displayText("SHUTTER SYSTEM STOP", 2, 16, false, false, false, false, false,true);
						displayText("RESTART THE SYSTEM", 2, 32, false, false, false, false, false,true);

					}
					bFatalErrorOccurred = true;
				}

				//
				// Clear fourth line of display.
				//
				//GrRectFIllBolymin(0, 126, 48, 63, true, true);
				GrRectFIllBolymin(0, 127, 48, 63, 0x00, true);

				//
				// Display anomaly log on fourth line.
				//

				//20170414      201703_other
			   if(gu8_language == Japanese_IDX)
			   {
				usnprintf((char*)lbuff, sizeof(lbuff), "E%03d:%s", gsActiveAnomalyList[lsAnomalyIndex].errorCode,
						gsActiveAnomalyList[lsAnomalyIndex].errordescription_jananese);
			   }
			   else
			   {
			   	usnprintf((char*)lbuff, sizeof(lbuff), "E%03d:%s", gsActiveAnomalyList[lsAnomalyIndex].errorCode,
			   			gsActiveAnomalyList[lsAnomalyIndex].errordescripition_english);
			   }
			//	memset(lbuff_cyw,0x20,sizeof(lbuff_cyw));
				//usnprintf_E03_cyw((char*)lbuff_cyw,sizeof(lbuff_cyw),(char*)lbuff);
				//memcpy((char*)(lbuff+5),(char*)(gsActiveAnomalyList[lsAnomalyIndex].errordescription),26);
			   //20170414      201703_No.25 start
				if((gsActiveAnomalyList[lsAnomalyIndex].errorCode==27)||(gsActiveAnomalyList[lsAnomalyIndex].errorCode==21)||
						(gsActiveAnomalyList[lsAnomalyIndex].errorCode==112)||(gsActiveAnomalyList[lsAnomalyIndex].errorCode==113))
				{

			       memset(lbuff,0,sizeof(lbuff));
				   if(gu8_language == Japanese_IDX)
				   {
					   	memcpy(lbuff,gsActiveAnomalyList[lsAnomalyIndex].errordescription_jananese,strlen(gsActiveAnomalyList[lsAnomalyIndex].errordescription_jananese));
				   		displayText(lbuff, 1, 48, false, false, false, false, true, false);
				   	}
				  	else
				   {
				  		memcpy(lbuff,gsActiveAnomalyList[lsAnomalyIndex].errordescripition_english,strlen(gsActiveAnomalyList[lsAnomalyIndex].errordescripition_english));
				  		displayText(lbuff, 1, 48, false, false, false, false, false, true);
				   }

				}
				else
				{



				   memcpy(CCCC,lbuff,5);
				   	if(gu8_language == Japanese_IDX)
				   	{
				   		memcpy(lbuff +5,gsActiveAnomalyList[lsAnomalyIndex].errordescription_jananese,strlen(gsActiveAnomalyList[lsAnomalyIndex].errordescription_jananese));
				   		displayText(CCCC, 1, 48, false, false, false, false, true, true);
				   		displayText(lbuff+5, 30, 48, false, false, false, false, true, false);
				   	}
				   	else
				   	{
				   		memcpy(lbuff +5,gsActiveAnomalyList[lsAnomalyIndex].errordescripition_english,strlen(gsActiveAnomalyList[lsAnomalyIndex].errordescripition_english));
				   		displayText(CCCC, 1, 48, false, false, false, false, false, true);
				   	   displayText(lbuff+5, 30, 48, false, false, false, false, false, true);
				   	}
				}
				//20170414      201703_No.25 end

				//
				// Capture time.
				//
				lsTickCount3Seconds = g_ui32TickCount;

				//
				// Set 3 seconds delay start flag.
				//
				lsDelay3SecStart = 1;

			}

			//
			// Increment anomaly index
			//
			lsAnomalyIndex++;
		}
	}
	else
	{
		lsAnomalyIndex = 0;
	}

	//
	// Check whether delay is achieved.
	//
	if( ( get_timego( lsTickCount3Seconds) > 300 ) &&
		(lsDelay3SecStart == 1)
	  )
	{
		//
		// Clear fourth line of display.
		//
		//GrRectFIllBolymin(0, 126, 48, 63, true, true);
		GrRectFIllBolymin(0, 127, 48, 63, 0x00, true);

		//
		// Reset 3 seconds delay start flag
		//
		lsDelay3SecStart = 0;
	}

}


void RecoveredAnomalies(void)
{
	uint8_t Tp_i =0,Tp_j=0,Tp_flag=0,Tp_installation=0 ;
	//uint8_t Tp_cyedata[6]={0x01,0x02,0x06,0x38,0x4e,0xa2};
	//uint8_t Tp_cyedata[6]={0x01,0x02,0x06,0x13,0x51,0xe2};
	//uint8_t Tp_cyedata[6]={0x01,0x02,0x06,0x55,0xa3,0x63};
	uint8_t Tp_cyedata[6]={0x01,0x02,0x06,0x20,0x44,0xa2};

	open_disable_enable_cyw=0;
	close_disable_enable_cyw=0;
	//if(gKeysStatus.bits.Key_3secEnter_pressed == true)
	if((KEY_PRESS_3SEC_ENT_FLAG_CYW == true)||(KEY_PRESS_3SEC_STOP_FLAG_CYW_1==true))//20170414      201703_No.27
	{
		//gKeysStatus.bits.Key_3secEnter_pressed = false;
		KEY_PRESS_3SEC_ENT_FLAG_CYW = false;
		KEY_PRESS_3SEC_STOP_FLAG_CYW_1 = false;            //20170414      201703_No.27
	//	uartSendTxBuffer(UART_control,Tp_cyedata,6);
		for(Tp_i =0;Tp_i<20;Tp_i++)
		{
			if((gsActiveAnomalyList[Tp_i].errorCode ==  14)||(gsActiveAnomalyList[Tp_i].errorCode ==  16)||(gsActiveAnomalyList[Tp_i].errorCode ==  18)||(gsActiveAnomalyList[Tp_i].errorCode ==  19))
			{
				Tp_flag = 1;
			}
			if(gsActiveAnomalyList[Tp_i].errorCode == 111)
			{
				Tp_flag = 1;
			}
			if((gsActiveAnomalyList[Tp_i].errorCode ==  35)||(gsActiveAnomalyList[Tp_i].errorCode ==  36)||(gsActiveAnomalyList[Tp_i].errorCode ==  37)||(gsActiveAnomalyList[Tp_i].errorCode ==  38)||(gsActiveAnomalyList[Tp_i].errorCode ==  39))
			{
				Tp_installation = 1;
				Tp_flag = 1;
			}
		}
		if(Tp_flag==1)
		{
			uartSendTxBuffer(UART_control,Tp_cyedata,6);
			//lstCommunicationModuleInnerTaskComm.commandToControlBoard.val=0x10000000;
			//lstCommunicationModuleInnerTaskComm.destination = eDestDriveBoard;
			//lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE ;
			//lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
			//lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.recover_anomaly=1;
		}
	    if(Tp_installation ==1)
	    {
	    	psActiveFunctionalBlock = &gsInstallationFunctionalBlock;
	    	psActiveFunctionalBlock->pfnPaintFirstScreen();
	    }
	}

	for(Tp_i =0;Tp_i<20;Tp_i++)
	{
		if(gsActiveAnomalyList[Tp_i].errorCode != 0)
		{
			for(Tp_j=0;Tp_j<TOTAL_ERRORS;Tp_j++)
			{
				if(gstErrorList[Tp_j].errorCode ==gsActiveAnomalyList[Tp_i].errorCode )
				{
					if(gstErrorList[Tp_j].stop_shutter_move_comm==NONE_OPEN_CLOSE)
					{
						open_disable_enable_cyw=1;
						close_disable_enable_cyw=1;
					}
					if(gstErrorList[Tp_j].stop_shutter_move_comm==OPEN_ONLY)
					{
						close_disable_enable_cyw=1;
					}
				}
			}
		}
	}
 //  if(gstDisplayCommunicationFault.bits.commFailControl == 1)
 // {
	// gstDisplayBoardStatus.bits.displayFault = 0;
	// gstDisplayBoardFault.bits.displayCommunication = 0;
	// gstDisplayCommunicationFault.bits.commFailControl = 0;
  //}
	//open_disable_enable_cyw=0;
	//close_disable_enable_cyw=0;

}

/******************************************************************************
 * FunctionName: addToActiveAnomaly
 *
 * Function Description:
 * Add anomaly entry into active anomaly list an index given by active anomaly
 * write index.
 *
 * Function Parameters:
 * pstActiveAnomalyData : pointer to errorDB type object
 *
 * Function Returns:
 * 0 							: On Success
 * _ERR_ENTRY_ALREADY_EXISTS	: Error already present
 *
 ********************************************************************************/
uint8_t addToActiveAnomaly(struct errorDB* pstActiveAnomalyData)
{
	int i = 0;

	//
	// Check whether anomaly code exists
	// if yes, then return
	//
	if((pstActiveAnomalyData->errorCode==52)||(pstActiveAnomalyData->errorCode==124))       //20170421  201703_No.39
	{
		doWatchdogReset_powerON();
		while(1);
	}
	for( i = 0; i < ANOMALY_LIST_LEN; i++ )
	{
		if( gsActiveAnomalyList[i].errorCode == pstActiveAnomalyData->errorCode )
		{
			return _ERR_ENTRY_ALREADY_EXISTS;
		}
	}

	//
	// Add new value in active anomaly list at current write index
	//
	gsActiveAnomalyList[gui8ActiveAnomalyWriteIndex].errorCode = pstActiveAnomalyData->errorCode;
	gsActiveAnomalyList[gui8ActiveAnomalyWriteIndex].errorType = pstActiveAnomalyData->errorType;
	//memcpy(gsActiveAnomalyList[gui8ActiveAnomalyWriteIndex].errordescription,
	//		pstActiveAnomalyData->errordescription, 15);
	//20170414      201703_other
	memcpy(gsActiveAnomalyList[gui8ActiveAnomalyWriteIndex].errordescripition_english,
				pstActiveAnomalyData->errordescripition_english, 30);
	memcpy(gsActiveAnomalyList[gui8ActiveAnomalyWriteIndex].errordescription_jananese,
					pstActiveAnomalyData->errordescription_jananese, 30);
//	memcpy(gsActiveAnomalyList[gui8ActiveAnomalyWriteIndex].errordescription,
//			pstActiveAnomalyData->errordescription,
//			(MAX_STRING_LEN)*(sizeof(char)) );

	//
	// Increment active anomaly write index
	//
	gui8ActiveAnomalyWriteIndex++;
	if(gui8ActiveAnomalyWriteIndex == ANOMALY_LIST_LEN)
		gui8ActiveAnomalyWriteIndex = 0;

	return 0;
}

/******************************************************************************
 * FunctionName: deleteFromActiveAnomoly
 *
 * Function Description:
 * 1. Delete an anomaly entry w.r.t. to error code if bClearList parameter is set to false.
 * 2. Delete entire active anomaly list if bClearList parameter is set to true.
 *
 * Function Parameters:
 * lui16ErrorCode :  Anomaly entry error code that is to be removed
 * bClearList : Enable clearing active anomaly list
 *
 * Function Returns:
 * 0 					: Success
 * _ERR_LIST_EMPTY		: No item in the list
 * _ERR_ENTRY_NOT_FOUND	: Error entry not found
 *
 ********************************************************************************/
uint8_t deleteFromActiveAnomaly(uint16_t lui16ErrorCode, bool bClearList)
{
	int i = 0;
	bool bErrorCodeExists = false;
	bool bActiveAnomalyListEmpty = true;
	uint8_t lui8AnomalyIndex = 0;

	if(bClearList == true)
	{
		for(i = 0; i < ANOMALY_LIST_LEN; i++ )
		{
			if( gsActiveAnomalyList[i].errorCode != 0 )
			{
				bActiveAnomalyListEmpty = false;
				break;
			}
		}

		if(bActiveAnomalyListEmpty == true)
		{
			return _ERR_LIST_EMPTY;
		}

		//
		// Delete entire active anomaly list
		//
		memset(gsActiveAnomalyList, 0 , (ANOMALY_LIST_LEN)*(sizeof(struct errorDB)) );
	}
	else
	{
		//
		// Check whether anomaly code exists
		//
		for(i = 0; i < ANOMALY_LIST_LEN; i++ )
		{
			if( gsActiveAnomalyList[i].errorCode == lui16ErrorCode )
			{
				bErrorCodeExists = true;
				lui8AnomalyIndex = i;
				break;
			}
		}

		if(bErrorCodeExists == false)
		{
			return _ERR_ENTRY_NOT_FOUND;
		}

		//
		// Delete active anomaly at lui8Index
		//
		memset(&gsActiveAnomalyList[lui8AnomalyIndex], 0 , sizeof(struct errorDB));
		//
		// Shift active anomaly data to remove gaps
		//
		for(i = lui8AnomalyIndex; i < ANOMALY_LIST_LEN - 1; i++)
		{
			gsActiveAnomalyList[i].errorCode = gsActiveAnomalyList[i+1].errorCode;
			gsActiveAnomalyList[i].errorType = gsActiveAnomalyList[i+1].errorType;
			//20170414      201703_other
			memcpy(&gsActiveAnomalyList[i].errordescripition_english,
					&gsActiveAnomalyList[i+1].errordescripition_english,
					(MAX_STRING_LEN)*(sizeof(char)) );
			memcpy(&gsActiveAnomalyList[i].errordescription_jananese,
					&gsActiveAnomalyList[i+1].errordescription_jananese,
					(MAX_STRING_LEN)*(sizeof(char)) );
		}
		memset(&gsActiveAnomalyList[ANOMALY_LIST_LEN - 1], 0 , sizeof(struct errorDB));

		//
		// Decrement active anomaly write index
		//
		if(gui8ActiveAnomalyWriteIndex > lui8AnomalyIndex)
			gui8ActiveAnomalyWriteIndex--;
	}

	return 0;
}

/******************************************************************************
 * FunctionName: anomalyHistoryRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t anomalyHistoryRunTime(void)
{
	updateFaultLEDStatus();

	return 0;
}
/******************************************************************************
 * FunctionName: anomalyHistoryPaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t anomalyHistoryPaint(void)
{
	//
	// This is first screen paint function
	//
	gstUMtoLM_read.commandRequestStatus = eACTIVE;
	gstUMtoLM_read.commandResponseStatus = eNO_STATUS;
	gstUMtoLM_read.commandToLMread.bits.readAnomalyHistory = 1;
	gstUMtoLM_read.historyOrAnomalyIndex = 1;
	readAnomalyHistory();
	showLogs(ANOMHIST_START_IDX);
	gstUMtoEM.commandToEM.bits.anomalyHistoryAccessed = 1;
	return 0;
}

/******************************************************************************
 * FunctionName: anomalyHistoryMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t anomalyHistoryMode(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Mode key press
	//
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
    	gKeysStatus.bits.Key_Mode_pressed = 0;

    	gstUMtoLM_read.historyOrAnomalyIndex = 0;
		psActiveFunctionalBlock = &gsMenuFunctionalBlock;
		psActiveFunctionalBlock->pfnPaintFirstScreen();

    }

	return 0;
}

/******************************************************************************
 * FunctionName: anomalyHistoryEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t anomalyHistoryEnter(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Enter key press
	//
	if(gKeysStatus.bits.Key_Enter_pressed)
	{
		gKeysStatus.bits.Key_Enter_pressed = 0;

		//
		// Write enter key functionality here
		//
	}

	return 0;
}

/******************************************************************************
 * FunctionName: anomalyHistoryUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t anomalyHistoryUp(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Up key press
	//
	if(gKeysStatus.bits.Key_Up_pressed)
	{
		gKeysStatus.bits.Key_Up_pressed = 0;

		gstUMtoLM_read.commandRequestStatus = eACTIVE;
		gstUMtoLM_read.commandResponseStatus = eNO_STATUS;
		gstUMtoLM_read.commandToLMread.bits.readAnomalyHistory = 1;
		if(gstUMtoLM_read.historyOrAnomalyIndex > 1)
			gstUMtoLM_read.historyOrAnomalyIndex--;
		else
			gstUMtoLM_read.historyOrAnomalyIndex = 20;
		readAnomalyHistory();
		showLogs(ANOMHIST_START_IDX);
	}

	return 0;
}

/******************************************************************************
 * FunctionName: anomalyHistoryDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t anomalyHistoryDown(void)
{
	//
	// This function is called periodically
	//

	//
	// Handle Down key press
	//
	if(gKeysStatus.bits.Key_Down_pressed)
	{
		gKeysStatus.bits.Key_Down_pressed = 0;

		gstUMtoLM_read.commandRequestStatus = eACTIVE;
		gstUMtoLM_read.commandResponseStatus = eNO_STATUS;
		gstUMtoLM_read.commandToLMread.bits.readAnomalyHistory = 1;
		if(gstUMtoLM_read.historyOrAnomalyIndex < 20)
			gstUMtoLM_read.historyOrAnomalyIndex++;
		else
			gstUMtoLM_read.historyOrAnomalyIndex = 1;
		readAnomalyHistory();
		showLogs(ANOMHIST_START_IDX);
	}

	return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/
stInternalFunctions gsAnomalyHistoryFunctionalBlock =
{
	0,
	&gsMenuFunctionalBlock,
	anomalyHistoryPaint,
	anomalyHistoryRunTime,
	anomalyHistoryUp,
	anomalyHistoryDown,
	anomalyHistoryMode,
	anomalyHistoryEnter
};
