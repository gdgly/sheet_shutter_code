/*********************************************************************************
* FileName: errormodule.c
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
 *  	0.1D	08/08/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <string.h>
#include "inc/hw_types.h"
#include "errormodule.h"
#include "ledhandler.h"
#include "userinterface.h"

#include "intertaskcommunication.h"
#include "Middleware/serial.h"
#include "logger.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define ERROR_LED_INDICATION_OVERRIDE_INSTALLATION	0
#define UPDATE_COMM_ERROR_TIME 	200

/****************************************************************************/

/****************************************************************************
 *  enum definitions:
****************************************************************************/
enum functionReturnValue
{
	eInit = 0,
	eWaitingForReply,
};

enum functionState
{
	eUpdate = 0,
	eWait,
};
/****************************************************************************
 *  Global variables for other files:
****************************************************************************/
// New variable added to inform error module to stop displaying the error - YG - NOV 15
/******************************************
 * Variable Name: gucStopErrorsDisplayn
 *
 * Variable Description:
 * This variable is used to inform "error module" to stop  displaying error
 * This feature will used in condition like "reset all parameter",
 * where there are chances the Drive board not responding to Control board
 * and displaying "CT-DR" error is not the recommended

 * Variable Values: 0 = Regular error display, 1 = Stop displaying error code
 *

 ******************************************/
uint8_t gucStopErrorsDisplay = 0;

/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
uint32_t guiControlCommunicationFaultCopy = 0;
uint32_t guiControlApplicationFaultCopy = 0;
uint32_t guiControlProcessorFaultCopy = 0;

uint32_t guiDisplayCommunicationFaultCopy = 0;
uint32_t guiDisplayBoardHwFaultCopy = 0;
uint32_t guiDisplayApplicationFaultCopy = 0;
uint32_t guiDisplayProcessorFaultCopy = 0;

uint32_t guiDriveCommunicationFaultCopy = 0;
uint32_t guiDriveMotorFaultCopy = 0;
uint32_t guiDriveApplicationFaultCopy = 0;
uint32_t guiDriveProcessorfaultCopy = 0;
uint32_t guiDriveInstallationCopy = 0;

uint8_t luiFaultLED_Value = 0;

uint32_t luiLastControlCommunicationFault = 0;
uint32_t luiLastControlApplicationFault = 0;
uint32_t luiLastControlProcessorFault = 0;

uint32_t luiLastDisplayCommunicationFault = 0;
uint32_t luiLastDisplayBoardHwFault = 0;
uint32_t luiLastDisplayApplicationFault = 0;
uint32_t luiLastDisplayProcessorFault = 0;

uint32_t luiLastDriveCommunicationFault = 0;
uint32_t luiLastDriveMotorFault = 0;
uint32_t luiLastDriveApplicationFault = 0;
uint32_t luiLastDriveProcessorfault = 0;
uint32_t luiLastDriveInstallation = 0;


uint8_t gucCallMonitorLED_Handler = 0;
uint8_t gucCallEMtoLM_Module = 0;

//const _stErrorList gstErrorList [TOTAL_ERRORS] =
//{
//		/*DISPLAY BOARD ERROR LIST*/
//		{	200	,	"�p�P�b�g�G���[2"	    	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DISP	,	0	,	&guiDisplayCommunicationFaultCopy	,	0	,	&luiLastDisplayCommunicationFault	,	0	,"PACKET CRC ERR"}	,
//	//	{	201	,	"DI-CT COMM ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayCommunicationFaultCopy	,	1	,	&luiLastDisplayCommunicationFault	,	1	}	,
//		{	201	,	"D-C�c�E�V���G���["		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayCommunicationFaultCopy	,	1	,	&luiLastDisplayCommunicationFault	,	1	,"DI-CT COMM ERR"}	,
//		//{	203	,	"UART ERR"			,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DISP	,	0	,	&guiDisplayCommunicationFaultCopy	,	3	,	&luiLastDisplayCommunicationFault	,	3	}	,
//		{	203	,	"UART�G���[2"			,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DISP	,	0	,	&guiDisplayCommunicationFaultCopy	,	3	,	&luiLastDisplayCommunicationFault	,	3	,"UART ERR"}	,
//		//{	211	,	"CARD DETECT ERR"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DISP	,	0	,	&guiDisplayBoardHwFaultCopy			,	0	,	&luiLastDisplayBoardHwFault			,	0	}	,
//		{	211	,	"SD�P���V���c�G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DISP	,	0	,	&guiDisplayBoardHwFaultCopy			,	0	,	&luiLastDisplayBoardHwFault			,	0	,"CARD DETECT ERR"}	,
//		{	212	,	"SD�z�S�@�J�L�R�~�t�J"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DISP	,	0	,	&guiDisplayBoardHwFaultCopy			,	1	,	&luiLastDisplayBoardHwFault			,	1	,"SD WRITE PROTECT"}	,
//		//	Added to log system power on event as requested by Bx on 21Apr2015
//		{	213	,	"�f���Q��ON/OFF"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DISP	,	0	,	&guiDisplayBoardHwFaultCopy			,	3	,	&luiLastDisplayBoardHwFault			,	3	,"POWER ON EVENT"}	,
//		//{	221	,	"SD READ ERR"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayApplicationFaultCopy		,	0	,	&luiLastDisplayApplicationFault		,	0	}	,
//		{	221	,	"SD���~�R�~�G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayApplicationFaultCopy		,	0	,	&luiLastDisplayApplicationFault		,	0	,"SD READ ERR"}	,
//		{	222	,	"SD�J�L�R�~�G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayApplicationFaultCopy		,	1	,	&luiLastDisplayApplicationFault		,	1	,"SD WRITE ERR"}	,
//		{	231	,	"�p�����[�^���[�h�G���[2"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	0	,	&luiLastDisplayProcessorFault		,	0	,"PARAM DB CRC"}	,
//		{	232	,	"EEP�����G���[3"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	2	,	&luiLastDisplayProcessorFault		,	2	,"EEPROM PROGRAM"}	,
//		{	233	,	"EEP�����G���[4"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	3	,	&luiLastDisplayProcessorFault		,	3	,"EEPROM ERASE"}	,
//		{	234	,	"�E�H�b�`�h�b�O�G���[2"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	6	,	&luiLastDisplayProcessorFault		,	6	,"WATCHDOG TRIP"}	,
//	//  {	234	,	"WATCHDOG TRIP"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	6	,	&luiLastDisplayProcessorFault		,	6	}	,
//		{	235	,	"EEP�����G���[5"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	7	,	&luiLastDisplayProcessorFault		,	7	,"EEPROM READ"}	,
//		/*CONTROL BOARD ERROR LIST*/
//		//{	101	,	"PACKET CRC ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	CTRL	,	0	,	&guiControlCommunicationFaultCopy	,	0	,	&luiLastControlCommunicationFault	,	0	}	,
//		{	101	,	"�p�P�b�g�G���[1"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	CTRL	,	0	,	&guiControlCommunicationFaultCopy	,	0	,	&luiLastControlCommunicationFault	,	0	,"PACKET CRC ERR"}	,
//		//{	102	,	"CT-DR COMM ERR"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlCommunicationFaultCopy	,	1	,	&luiLastControlCommunicationFault	,	1	}	,
//		{	102	,	"M-C�c�E�V���G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlCommunicationFaultCopy	,	1	,	&luiLastControlCommunicationFault	,	1	,"CT-DR COMM ERR"}	,
//		//{	103	,	"CONTR UART ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	CTRL	,	0	,	&guiControlCommunicationFaultCopy	,	3	,	&luiLastControlCommunicationFault	,	3	}	,
//		{	103	,	"UART�G���[�P"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	CTRL	,	0	,	&guiControlCommunicationFaultCopy	,	3	,	&luiLastControlCommunicationFault	,	3	,"CONTR UART ERR"}	,
//		//{	111	,	"OPR RESTRICT"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlApplicationFaultCopy		,	0	,	&luiLastControlApplicationFault		,	0	}	,
//		{	111	,	"�h�E�T�Z�C�Q���^�C�}"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlApplicationFaultCopy		,	0	,	&luiLastControlApplicationFault		,	0	,"OPR RESTRICT"}	,
//		//{	112	,	"STARTUP ERR"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlApplicationFaultCopy		,	1	,	&luiLastControlApplicationFault		,	1	}	,
//		{	112	,	"�L�h�E�Z���TON"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlApplicationFaultCopy		,	1	,	&luiLastControlApplicationFault		,	1	,"STARTUP ERR"}	,
//		//{	121	,	"PARAM DB CRC"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	0	,	&luiLastControlProcessorFault		,	0	}	,
//		{	121	,	"�p�����[�^���[�h�G���[1"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	0	,	&luiLastControlProcessorFault		,	0	,"PARAM DB CRC"}	,
//		//{	122	,	"EEPROM PROGRAM"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	2	,	&luiLastControlProcessorFault		,	2	}	,
//		{	122	,	"EEP�����G���[1"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	2	,	&luiLastControlProcessorFault		,	2	,"EEPROM PROGRAM"}	,
//		{	123	,	"EEP�����G���[2"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	3	,	&luiLastControlProcessorFault		,	3	,"EEPROM ERASE"}	,
//		//{	124	,	"WATCHDOG"			,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	6	,	&luiLastControlProcessorFault		,	6	}	,
//		{	124	,	"�E�H�b�`�h�b�O�G���[�P"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	6	,	&luiLastControlProcessorFault		,	6	,"WATCHDOG"}	,
//		{	125	,	"EEP�����G���[3"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	7	,	&luiLastControlProcessorFault		,	7	,"EEPROM READ"}	,
//		/*DRIVE BOARD ERROR LIST*/
///*		{	1	,	"PACKET CRC ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy		,	0	,	&luiLastDriveCommunicationFault		,	0	}	,
//		{	2	,	"UART ERR"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy		,	1	,	&luiLastDriveCommunicationFault		,	1	}	,
//		{	3	,	"COMD FRAME ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy		,	2	,	&luiLastDriveCommunicationFault		,	2	}	,
//		{	11	,	"MTR OPEN PHASE"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_5	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	0	,	&luiLastDriveMotorFault				,	0	}	,
//		{	12	,	"DRV DC OVER VTG"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	1	,	&luiLastDriveMotorFault				,	1	}	,
//		{	13	,	"DRV OVER CURENT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_3	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	2	,	&luiLastDriveMotorFault				,	2	}	,
//		{	14	,	"DRV TORQ 2 SEC"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	3	,	&luiLastDriveMotorFault				,	3	}	,
//		{	15	,	"DRV PFC SHUTDWN"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	4	,	&luiLastDriveMotorFault				,	4	}	,
//		{	16	,	"DRV MTR STALL"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	5	,	&luiLastDriveMotorFault				,	5	}	,
//		{	17	,	"DRV OVERHEAT"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	6	,	&luiLastDriveMotorFault				,	6	}	,
//		{	21	,	"PE OBSTACLE"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	0	,	&luiLastDriveApplicationFault		,	0	}	,
//		{	22	,	"LOW INPUT VTG"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_6	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	1	,	&luiLastDriveApplicationFault		,	1	}	,
//		{	23	,	"HIGH INPUT VTG"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_8	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	2	,	&luiLastDriveApplicationFault		,	2	}	,
//		{	24	,	"HALL SENSOR"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_7	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	3	,	&luiLastDriveApplicationFault		,	3	}	,
//		{	25	,	"ORIGIN SENSOR"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_2	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	4	,	&luiLastDriveApplicationFault		,	4	}	,
//		{	26	,	"WRAP AROUND"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	5	,	&luiLastDriveApplicationFault		,	5	}	,
//		{	27	,	"MICRO SWITCH"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	6	,	&luiLastDriveApplicationFault		,	6	}	,
//		{	28	,	"AIR SW TRIGGER"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	7	,	&luiLastDriveApplicationFault		,	7	}	,
//		{	29	,	"EMERGENCY STOP"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	8	,	&luiLastDriveApplicationFault		,	8	}	,
//		{	30	,	"CALLIB FAIL"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	9	,	&luiLastDriveApplicationFault		,	9	}	,
//		{	31	,	"MICRO SW LIMIT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	_BLINK_STATUS_50_MSEC	,	_BLINK_STATUS_50_MSEC	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	10	,	&luiLastDriveApplicationFault		,	10	}	,
//		{	32	,	"OPR COUNT LIMIT"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	11	,	&luiLastDriveApplicationFault		,	11	}	,
//		{	41	,	"FLASH IMG CRC"		,	NONRECOVERABLE	,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveProcessorfaultCopy			,	0	,	&luiLastDriveProcessorfault			,	0	}	,
//		{	42	,	"PARAM DB CRC"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	1	,	&luiLastDriveProcessorfault			,	1	}	,
//		{	43	,	"WATCHDOG TRIP"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	2	,	&luiLastDriveProcessorfault			,	2	}	,
//		{	44	,	"EEPROM PROGRAM"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	3	,	&luiLastDriveProcessorfault			,	3	}	,
//		{	45	,	"EEPROM ERASE"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	4	,	&luiLastDriveProcessorfault			,	4	}	,
//		{	46	,	"IGBT OVERHEAT"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	12	,	&luiLastDriveApplicationFault		,	12	}	,
//*/
//		{	1	,	"�p�P�b�g�G���[3"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy		,	0	,	&luiLastDriveCommunicationFault	,	0	,"PACKET CRC ERR"}	,
//		//{	2	,	"UART ERR"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy		,	1	,	&luiLastDriveCommunicationFault	,	1	}	,
//		{	2	,	"UART�G���[3"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy		,	1	,	&luiLastDriveCommunicationFault	,	1	,"UART ERR"}	,
//		{	3	,	"�R�}���h�G���["			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy		,	2	,	&luiLastDriveCommunicationFault	,	2	,"COMMAND FRAME ERR"}	,
//		{	11	,	"���[�^�P�b�\ �E�G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_5	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	0	,	&luiLastDriveMotorFault			,	0	,"MTR OPEN PHASE"}	,
//		{	12	,	"�J�f���A�c"			,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	1	,	&luiLastDriveMotorFault			,	1	,"DRV DC OVER VTG"}	,
//		{	13	,	"�J�f�������E"			,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_3	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	2	,	&luiLastDriveMotorFault			,	2	,"DRV OVER CURENT"}	,
//		{	14	,	"�J�t�J�G���["			,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	3	,	&luiLastDriveMotorFault			,	3	,"DRV TORQ 2 SEC"}	,
//		{	15	,	"DRV PFC SHUTDWN"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	4	,	&luiLastDriveMotorFault			,	4	,"DRV PFC SHUTDWN"}	,
//		//{	16	,	"DRV MTR STALL"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	5	,	&luiLastDriveMotorFault			,	5	}	,
//		{	16	,	"���[�^�X�g�[���G���[1"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	5	,	&luiLastDriveMotorFault			,	5	,"DRV MTR STALL"}	,
//		//{	17	,	"DRV OVERHEAT"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	6	,	&luiLastDriveMotorFault			,	6	}	,
//		{	17	,	"�I�[�o�[�q�[�g�G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	6	,	&luiLastDriveMotorFault			,	6	,"DRV OVERHEAT"}	,
//		{	18	,	"���[�^�X�g�[���G���[2"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	7	,	&luiLastDriveMotorFault			,	7	,"MTR SUSTAIN"}	,
//		{	19	,	"���[�^�X�g�[���G���[3"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	8	,	&luiLastDriveMotorFault			,	8	,"MTR PWM COAST"}	,
//		{	20	,	"���[�^�G���["			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	9	,	&luiLastDriveMotorFault			,	9	,"MTR OTHER ERROR"}	,
//		{	21	,	"�R�E�f���Z���TON"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DRIV	,	0	,	&guiDriveApplicationFaultCopy		,	0	,	&luiLastDriveApplicationFault	,	0	,"PE OBSTACLE"}	,
//		{	22	,	"�f���A�c�@�u�\ �N"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_6	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	1	,	&luiLastDriveApplicationFault	,	1	,"LOW INPUT VTG"}	,
//		{	23	,	"�J�f���A�c"			,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_8	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	2	,	&luiLastDriveApplicationFault	,	2	,"HIGH INPUT VTG"}	,
//		//{	24	,	"HALL SENSOR"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_7	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	3	,	&luiLastDriveApplicationFault	,	3	}	,
//		{	24	,	"�z�[��IC�G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_7	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	3	,	&luiLastDriveApplicationFault	,	3	,"HALL SENSOR"}	,
//		{	25	,	"�Q���e���G���["		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_2	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	4	,	&luiLastDriveApplicationFault		,	4	,"ORIGIN SENSOR"}	,
//		{	26	,	"WRAP AROUND"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DRIV	,	0	,	&guiDriveApplicationFaultCopy		,	4	,	&luiLastDriveApplicationFault	,	4	,"WRAP AROUND"}	,
//		//{	27	,	"MICRO SWITCH"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy		,	5	,	&luiLastDriveApplicationFault	,	5	}	,
//		{	27	,	"�}�C�N���Z���TON"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy		,	5	,	&luiLastDriveApplicationFault	,	5	,"MICRO SWITCH"}	,
//		//{	28	,	"AIR SW TRIGGER"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy		,	6	,	&luiLastDriveApplicationFault	,	6	}	,
//		{	28	,	"�G�A�X�C�b�`ON"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy		,	6	,	&luiLastDriveApplicationFault	,	6	,"AIR SW TRIGGER"}	,
//		{	29	,	"�G�}�[���X�C�b�`ON"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	7	,	&luiLastDriveApplicationFault	,	7	,"EMERGENCY STOP"}	,
//		//{	30	,	"CALLIB FAIL"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	13	,	&luiLastDriveApplicationFault	,	13	}	,
//		{	30	,	"�z�Z�C�G���["			,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	13	,	&luiLastDriveApplicationFault	,	13	,"CALLIB FAIL"}	,
//		//{	31	,	"MICRO SW LIMIT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	_BLINK_STATUS_50_MSEC	,	_BLINK_STATUS_50_MSEC	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	14	,	&luiLastDriveApplicationFault	,	14	}	,
//		{	31	,	"�}�C�N���J�E���^�g�E�^�c"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	_BLINK_STATUS_50_MSEC	,	_BLINK_STATUS_50_MSEC	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	14	,	&luiLastDriveApplicationFault	,	14	,"MICRO SW LIMIT"}	,
//		//{	32	,	"OPR COUNT LIMIT"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy		,	15	,	&luiLastDriveApplicationFault	,	15	}	,
//		{	32	,	"�����e�J�E���g�g�E�^�c"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy		,	15	,	&luiLastDriveApplicationFault	,	15	,"OPR COUNT LIMIT"}	,
//		// "POWER FAIL" will use to indicate the situation during power fail - YG - NOV 15
//		//{	33	,	"POWER FAIL"		,	NONRECOVERABLE	,	NONRECORDABLE_ANOMALY	,	OFF						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	17	,	&luiLastDriveApplicationFault	,	17	}	,
//		{	33	,	"�f���A�c�u�\ �N"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	17	,	&luiLastDriveApplicationFault	,	17	,"POWER FAIL"}	,
//		{	50	,	"�t���b�V���C���[�W�G���["	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveProcessorfaultCopy			,	0	,	&luiLastDriveProcessorfault		,	0	,"IGBT OVERHEAT"}	,
//		{	51	,	"�p�����[�^���[�h�G���[3"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	1	,	&luiLastDriveProcessorfault		,	1	,"PARAM DB CRC"}	,
//		{	52	,	"�E�H�b�`�h�b�O�G���[3"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	2	,	&luiLastDriveProcessorfault		,	2	,"WATCHDOG TRIP"}	,
//		{	53	,	"EEP�����G���[5"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	3	,	&luiLastDriveProcessorfault		,	3	,"EEPROM PROGRAM"}	,
//		{	54	,	"EEP�����G���[6"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	4	,	&luiLastDriveProcessorfault		,	4	,"EEPROM ERASE"}	,
//		//	Error numbers updated as per error category - YG - NOV 2015
//		//{	34	,	"IGBT OVERHEAT"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	16	,	&luiLastDriveApplicationFault	,	16	}	,
//		{	34	,	"IGBT�T�[�}��"			,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	16	,	&luiLastDriveApplicationFault	,	16	,"IGBT OVERHEAT"}	,
//		//{	35	,	"OS NOT DTCT UP"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	8	,	&luiLastDriveApplicationFault	,	8	}	,
//		{	35	,	"�Q���e��_�G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	8	,	&luiLastDriveApplicationFault	,	8	,"OS NOT DTCT UP"}	,
//		//{	36	,	"OS NOT DTCT DWN"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	9	,	&luiLastDriveApplicationFault	,	9	}	,
//		{	36	,	"�Q���e��_�G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	9	,	&luiLastDriveApplicationFault	,	9	,"OS NOT DTCT DWN"}	,
//		//{	37	,	"OS ON UPPER LMT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	10	,	&luiLastDriveApplicationFault	,	10	}	,
//		{	37	,	"�Q���e��_�G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	10	,	&luiLastDriveApplicationFault	,	10	,"OS ON UPPER LMT"}	,
//		//{	38	,	"OS OFF LWR LMT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	11	,	&luiLastDriveApplicationFault	,	11	}	,
//		{	38	,	"�Q���e��_�G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	11	,	&luiLastDriveApplicationFault	,	11	,"OS OFF LWR LMT"}	,
//		//{	39	,	"OS VALIDT FAIL"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	12	,	&luiLastDriveApplicationFault	,	12	}	,
//		{	39	,	"�Q���e��_�G���["		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	12	,	&luiLastDriveApplicationFault	,	12	,"OS VALIDT FAIL"}	,
//		//	Two new errors added to monitor false shutter movement - YG - NOV 2015
//		{	40	,	"�C�W���E�@�W���E�V���E"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	18	,	&luiLastDriveApplicationFault	,	18	,""}	,
//		{	41	,	"�C�W���E�@�J�R�E"			,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	19	,	&luiLastDriveApplicationFault	,	19	,""}	,
//		{	42	,	"MTR CABLE FAULT"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	20	,	&luiLastDriveApplicationFault	,	20	,""}	,
//};

const _stErrorList gstErrorList[TOTAL_ERRORS] =
{
		/*DISPLAY BOARD ERROR LIST*/
		{	200	,	"PACKET CRC ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DISP	,	0	,	&guiDisplayCommunicationFaultCopy	,	0	,	&luiLastDisplayCommunicationFault	,	0	,"�p�P�b�g�G���[2"		,NONE_OPEN_CLOSE}	,
		{	201	,	"DI-CT COMM ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayCommunicationFaultCopy	,	1	,	&luiLastDisplayCommunicationFault	,	1	,"D-C�c�E�V���G���["		,NONE_OPEN_CLOSE}	,
		{	203	,	"UART ERR"			,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DISP	,	0	,	&guiDisplayCommunicationFaultCopy	,	3	,	&luiLastDisplayCommunicationFault	,	3	,"UART�G���[2"			,NONE_OPEN_CLOSE}	,
		{	211	,	"CARD DETECT ERR"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DISP	,	0	,	&guiDisplayBoardHwFaultCopy			,	0	,	&luiLastDisplayBoardHwFault			,	0	,"SD�P���V���c�G���["		,ENABLE_OPEN_CLOSE}	,
		{	212	,	"SD WRITE PRTCT"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DISP	,	0	,	&guiDisplayBoardHwFaultCopy			,	1	,	&luiLastDisplayBoardHwFault			,	1	,"SD�z�S�@�J�L�R�~�t�J"	,ENABLE_OPEN_CLOSE}	,
		//	Added to log system power on event as requested by Bx on 21Apr2015
		{	213	,	"POWER ON EVENT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DISP	,	0	,	&guiDisplayBoardHwFaultCopy			,	3	,	&luiLastDisplayBoardHwFault			,	3	,"�f���Q��ON/OFF"		,NONE_OPEN_CLOSE}	,
		{	221	,	"SD READ ERR"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayApplicationFaultCopy		,	0	,	&luiLastDisplayApplicationFault		,	0	,"SD���~�R�~�G���["		,ENABLE_OPEN_CLOSE}	,
		{	222	,	"SD WRITE ERR"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayApplicationFaultCopy		,	1	,	&luiLastDisplayApplicationFault		,	1	,"SD�J�L�R�~�G���["		,ENABLE_OPEN_CLOSE}	,
		{	231	,	"PARAM DB CRC"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	0	,	&luiLastDisplayProcessorFault		,	0	,"�p�����[�^���[�h�G���[2"	,NONE_OPEN_CLOSE}	,
		{	232	,	"EEPROM PROGRAM"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	2	,	&luiLastDisplayProcessorFault		,	2	,"EEP�����G���[3"		,NONE_OPEN_CLOSE}	,
		{	233	,	"EEPROM ERASE"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	3	,	&luiLastDisplayProcessorFault		,	3	,"EEP�����G���[4"		,NONE_OPEN_CLOSE}	,
		{	234	,	"WATCHDOG TRIP"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	6	,	&luiLastDisplayProcessorFault		,	6	,"�E�H�b�`�h�b�O�G���[2"	,NONE_OPEN_CLOSE}	,
		{	235	,	"EEPROM READ"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	7	,	&luiLastDisplayProcessorFault		,	7	,"EEP�����G���[5"		,NONE_OPEN_CLOSE}	,
		/*CONTROL BOARD ERROR LIST*/
		{	101	,	"PACKET CRC ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	CTRL	,	0	,	&guiControlCommunicationFaultCopy	,	0	,	&luiLastControlCommunicationFault	,	0	,"�p�P�b�g�G���[1"		,NONE_OPEN_CLOSE}	,
		{	102	,	"CT-DR COMM ERR"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlCommunicationFaultCopy	,	1	,	&luiLastControlCommunicationFault	,	1	,"M-C�c�E�V���G���["		,NONE_OPEN_CLOSE}	,
		{	103	,	"CONTR UART ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	CTRL	,	0	,	&guiControlCommunicationFaultCopy	,	3	,	&luiLastControlCommunicationFault	,	3	,"UART�G���[1"			,NONE_OPEN_CLOSE}	,
		{	111	,	"OPR RESTRICT"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlApplicationFaultCopy		,	0	,	&luiLastControlApplicationFault		,	0	,"�h�E�T�Z�C�Q���^�C�}"	,NONE_OPEN_CLOSE}	,
		{	113	,	"OBSTACLE"		    ,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlApplicationFaultCopy		,	1	,	&luiLastControlApplicationFault		,	1	,"�A���[���Z���TON"		,OPEN_ONLY}	,
		{	112	,	"STARTUP ERR"	    ,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlApplicationFaultCopy		,	2	,	&luiLastControlApplicationFault		,	2	,"�L�h�E�Z���TON"		,OPEN_ONLY}	,
		{	121	,	"PARAM DB CRC"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	0	,	&luiLastControlProcessorFault		,	0	,"�p�����[�^���[�h�G���[1"	,NONE_OPEN_CLOSE}	,
		{	122	,	"EEPROM PROGRAM"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	2	,	&luiLastControlProcessorFault		,	2	,"EEP�����G���[1"		,NONE_OPEN_CLOSE}	,
		{	123	,	"EEPROM ERASE"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	3	,	&luiLastControlProcessorFault		,	3	,"EEP�����G���[2"		,NONE_OPEN_CLOSE}	,
		{	124	,	"WATCHDOG"			,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	6	,	&luiLastControlProcessorFault		,	6	,"�E�H�b�`�h�b�O�G���[1"	,NONE_OPEN_CLOSE}	,
		{	125	,	"EEPROM READ"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	7	,	&luiLastControlProcessorFault		,	7	,"EEP�����G���[3"		,NONE_OPEN_CLOSE}	,
		/*DRIVE BOARD ERROR LIST*/
/*		{	1	,	"PACKET CRC ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy		,	0	,	&luiLastDriveCommunicationFault		,	0	}	,
		{	2	,	"UART ERR"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy		,	1	,	&luiLastDriveCommunicationFault		,	1	}	,
		{	3	,	"COMD FRAME ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy		,	2	,	&luiLastDriveCommunicationFault		,	2	}	,
		{	11	,	"MTR OPEN PHASE"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_5	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	0	,	&luiLastDriveMotorFault				,	0	}	,
		{	12	,	"DRV DC OVER VTG"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	1	,	&luiLastDriveMotorFault				,	1	}	,
		{	13	,	"DRV OVER CURENT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_3	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	2	,	&luiLastDriveMotorFault				,	2	}	,
		{	14	,	"DRV TORQ 2 SEC"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	3	,	&luiLastDriveMotorFault				,	3	}	,
		{	15	,	"DRV PFC SHUTDWN"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	4	,	&luiLastDriveMotorFault				,	4	}	,
		{	16	,	"DRV MTR STALL"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	5	,	&luiLastDriveMotorFault				,	5	}	,
		{	17	,	"DRV OVERHEAT"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy				,	6	,	&luiLastDriveMotorFault				,	6	}	,
		{	21	,	"PE OBSTACLE"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	0	,	&luiLastDriveApplicationFault		,	0	}	,
		{	22	,	"LOW INPUT VTG"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_6	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	1	,	&luiLastDriveApplicationFault		,	1	}	,
		{	23	,	"HIGH INPUT VTG"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_8	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	2	,	&luiLastDriveApplicationFault		,	2	}	,
		{	24	,	"HALL SENSOR"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_7	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	3	,	&luiLastDriveApplicationFault		,	3	}	,
		{	25	,	"ORIGIN SENSOR"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_2	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	4	,	&luiLastDriveApplicationFault		,	4	}	,
		{	26	,	"WRAP AROUND"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	5	,	&luiLastDriveApplicationFault		,	5	}	,
		{	27	,	"MICRO SWITCH"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	6	,	&luiLastDriveApplicationFault		,	6	}	,
		{	28	,	"AIR SW TRIGGER"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	7	,	&luiLastDriveApplicationFault		,	7	}	,
		{	29	,	"EMERGENCY STOP"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	8	,	&luiLastDriveApplicationFault		,	8	}	,
		{	30	,	"CALLIB FAIL"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	9	,	&luiLastDriveApplicationFault		,	9	}	,
		{	31	,	"MICRO SW LIMIT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	_BLINK_STATUS_50_MSEC	,	_BLINK_STATUS_50_MSEC	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	10	,	&luiLastDriveApplicationFault		,	10	}	,
		{	32	,	"OPR COUNT LIMIT"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	11	,	&luiLastDriveApplicationFault		,	11	}	,
		{	41	,	"FLASH IMG CRC"		,	NONRECOVERABLE	,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveProcessorfaultCopy			,	0	,	&luiLastDriveProcessorfault			,	0	}	,
		{	42	,	"PARAM DB CRC"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	1	,	&luiLastDriveProcessorfault			,	1	}	,
		{	43	,	"WATCHDOG TRIP"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	2	,	&luiLastDriveProcessorfault			,	2	}	,
		{	44	,	"EEPROM PROGRAM"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	3	,	&luiLastDriveProcessorfault			,	3	}	,
		{	45	,	"EEPROM ERASE"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy			,	4	,	&luiLastDriveProcessorfault			,	4	}	,
		{	46	,	"IGBT OVERHEAT"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy		,	12	,	&luiLastDriveApplicationFault		,	12	}	,
*/
		{	1	,	"PACKET CRC ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy	,	0	,	&luiLastDriveCommunicationFault	,	0	,"�p�P�b�g�G���[3"				,NONE_OPEN_CLOSE}	,
		{	2	,	"UART ERR"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy	,	1	,	&luiLastDriveCommunicationFault	,	1	,"UART�G���[3"					,NONE_OPEN_CLOSE}	,
		{	3	,	"COMD FRAME ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy	,	2	,	&luiLastDriveCommunicationFault	,	2	,"�R�}���h�G���["					,NONE_OPEN_CLOSE}	,
		{	11	,	"MTR OPEN PHASE"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_5	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	0	,	&luiLastDriveMotorFault			,	0	,"���[�^�P�b�\ �E�G���["			,NONE_OPEN_CLOSE}	,
		{	12	,	"DRV DC OVER VTG"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	1	,	&luiLastDriveMotorFault			,	1	,"�J�f���A�c"					,NONE_OPEN_CLOSE}	,
		{	13	,	"DRV OVER CURENT"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_3	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	2	,	&luiLastDriveMotorFault			,	2	,"�J�f�������E"					,NONE_OPEN_CLOSE}	,
		{	14	,	"DRV TORQ 2 SEC"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	3	,	&luiLastDriveMotorFault			,	3	,"�J�t�J�G���["					,NONE_OPEN_CLOSE}	,
		{	15	,	"DRV PFC SHUTDWN"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	4	,	&luiLastDriveMotorFault			,	4	,"DRV PFC SHUTDWN"			,NONE_OPEN_CLOSE}	,
		{	16	,	"DRV MTR STALL"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	5	,	&luiLastDriveMotorFault			,	5	,"���[�^�X�g�[���G���[1"			,NONE_OPEN_CLOSE}	,
		{	17	,	"DRV OVERHEAT"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	6	,	&luiLastDriveMotorFault			,	6	,"�I�[�o�[�q�[�g�G���["			,NONE_OPEN_CLOSE}	,
		{	18	,	"MTR SUSTAIN"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	7	,	&luiLastDriveMotorFault			,	7	,"���[�^�X�g�[���G���[2"			,NONE_OPEN_CLOSE}	,
		{	19	,	"MTR PWM COAST"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	8	,	&luiLastDriveMotorFault			,	8	,"���[�^�X�g�[���G���[3"			,NONE_OPEN_CLOSE}	,
		{	20	,	"MTR OTHER ERROR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	9	,	&luiLastDriveMotorFault			,	9	,"���[�^�G���["					,NONE_OPEN_CLOSE}	,
		{	21	,	"PE OBSTACLE"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	0	,	&luiLastDriveApplicationFault	,	0	,"�R�E�f���Z���TON"				,OPEN_ONLY}	,
		{	22	,	"LOW INPUT VTG"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_6	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	1	,	&luiLastDriveApplicationFault	,	1	,"�f���A�c�@�u�\ �N"				,NONE_OPEN_CLOSE}	,
		{	23	,	"HIGH INPUT VTG"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_8	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	2	,	&luiLastDriveApplicationFault	,	2	,"�J�f���A�c"					,NONE_OPEN_CLOSE}	,
		{	24	,	"HALL SENSOR"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_7	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	3	,	&luiLastDriveApplicationFault	,	3	,"�z�[��IC�G���["				,NONE_OPEN_CLOSE}	,
		{	26	,	"WRAP AROUND"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DRIV	,	0	,	&guiDriveApplicationFaultCopy	,	4	,	&luiLastDriveApplicationFault	,	4	,"WRAP AROUND"				,NONE_OPEN_CLOSE}	,
		{	27	,	"MICRO SWITCH"		,	RECOVERABLE		,	RECORDABLE_ANOMALY	    ,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy	,	5	,	&luiLastDriveApplicationFault	,	5	,"�}�C�N���Z���TON"				,OPEN_ONLY}	,
		{	28	,	"AIR SW TRIGGER"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy	,	6	,	&luiLastDriveApplicationFault	,	6	,"�G�A�X�C�b�`ON"				,NONE_OPEN_CLOSE}	,
		{	29	,	"EMERGENCY STOP"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	7	,	&luiLastDriveApplicationFault	,	7	,"�G�}�[���X�C�b�`ON"				,NONE_OPEN_CLOSE}	,
		{	30	,	"CALLIB FAIL"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	13	,	&luiLastDriveApplicationFault	,	13	,"�z�Z�C�G���["					,NONE_OPEN_CLOSE}	,
		{	31	,	"MICRO SW LIMIT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	_BLINK_STATUS_50_MSEC	,	_BLINK_STATUS_50_MSEC	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	14	,	&luiLastDriveApplicationFault	,	14	,"�}�C�N���J�E���^�g�E�^�c"			,ENABLE_OPEN_CLOSE}	,
		{	32	,	"OPR COUNT LIMIT"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy	,	15	,	&luiLastDriveApplicationFault	,	15	,"�����e�J�E���g�g�E�^�c"			,ENABLE_OPEN_CLOSE}	,
		// "POWER FAIL" will use to indicate the situation during power fail - YG - NOV 15
		{	33	,	"POWER FAIL"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	17	,	&luiLastDriveApplicationFault	,	17	,"�f���A�c�u�\ �N"				,NONE_OPEN_CLOSE}	,
		{	50	,	"FLASH IMG CRC"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveProcessorfaultCopy		,	0	,	&luiLastDriveProcessorfault		,	0	,"�t���b�V���C���[�W�G���["			,NONE_OPEN_CLOSE}	,
		{	51	,	"PARAM DB CRC"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy		,	1	,	&luiLastDriveProcessorfault		,	1	,"�p�����[�^���[�h�G���[3"			,NONE_OPEN_CLOSE}	,
		{	52	,	"WATCHDOG TRIP"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy		,	2	,	&luiLastDriveProcessorfault		,	2	,"�E�H�b�`�h�b�O�G���[3"			,NONE_OPEN_CLOSE}	,
		{	53	,	"EEPROM PROGRAM"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy		,	3	,	&luiLastDriveProcessorfault		,	3	,"EEP�����G���[5"				,NONE_OPEN_CLOSE}	,
		{	54	,	"EEPROM ERASE"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy		,	4	,	&luiLastDriveProcessorfault		,	4	,"EEP�����G���[6"				,NONE_OPEN_CLOSE}	,
		//	Error numbers updated as per error category - YG - NOV 2015
		{	34	,	"IGBT OVERHEAT"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	16	,	&luiLastDriveApplicationFault	,	16	,"�I�[�o�[�q�[�g�G���["			,NONE_OPEN_CLOSE}	,
		{	35	,	"OS NOT DTCT UP"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	8	,	&luiLastDriveApplicationFault	,	8	,"�Q���e���G���[1"				,NONE_OPEN_CLOSE}	,
		{	36	,	"OS NOT DTCT DWN"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	9	,	&luiLastDriveApplicationFault	,	9	,"�Q���e���G���[2"				,NONE_OPEN_CLOSE}	,
		{	37	,	"OS ON UPPER LMT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	10	,	&luiLastDriveApplicationFault	,	10	,"�Q���e���G���[3"				,NONE_OPEN_CLOSE}	,
		{	38	,	"OS OFF LWR LMT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	11	,	&luiLastDriveApplicationFault	,	11	,"�Q���e���G���[4"				,NONE_OPEN_CLOSE}	,
		{	39	,	"OS VALIDT FAIL"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	12	,	&luiLastDriveApplicationFault	,	12	,"�Q���e���G���[5"				,NONE_OPEN_CLOSE}	,
		//	Two new errors added to monitor false shutter movement - YG - NOV 2015
		{	40	,	"WRONG MOVE UP"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	18	,	&luiLastDriveApplicationFault	,	18	,"�C�W���E�@�W���E�V���E"			,NONE_OPEN_CLOSE}	,
		{	41	,	"WRONG MOVE DOWN"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	19	,	&luiLastDriveApplicationFault	,	19	,"�C�W���E�@�J�R�E"				,NONE_OPEN_CLOSE}	,
		{	42	,	"MTR CABLE FAULT"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	20	,	&luiLastDriveApplicationFault	,	20	,"MTR CABLE FAULT"			,NONE_OPEN_CLOSE}	,
};
/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
****************************************************************************/

/******************************************************************************
 * controlBoardMonitorLED_Handler
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
enum functionReturnValue controlBoardMonitorLED_Handler(void);

/******************************************************************************
 * getControlBoardError
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
enum functionReturnValue logError(void);

/******************************************************************************
 * updateFaultLEDstatus
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
uint8_t updateFaultLEDstatus(void);


/******************************************************************************
 * updateCTtoDR_CommErrorBit
 *
 * Function Description:
 * This function will be called when gucCTtoDR_CommError value is 1.
 * It will set the gstControlCommunicationFault.bits.commFailDrive only when
 * gucDItoCT_CommError remains high for a specified time.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void updateCTtoDR_CommErrorBit(void);

/******************************************************************************
 * updateDItoCT_CommErrorBit
 *
 * Function Description:
 * This function will be called when gucDItoCT_CommError value is 1.
 * It will set the gstDisplayCommunicationFault.bits.commFailControl only when
 * gucDItoCT_CommError remains high for a specified time.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void updateDItoCT_CommErrorBit(void);

/********************************************************************************/


/******************************************************************************
 * errorModule
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void errorModule(void)
{
	enum functionReturnValue returnValue = eInit;

	struct errorDB errorDetails;

	static uint8_t lsucLastErrorPriority = 0;
	static uint8_t lsucLastMonitorLED_Status = 0;

	uint8_t lucCount = 0;
//	uint8_t lucTemp = 0;

	//
	// Added on 15 Sep 2014 to avoid communication error logging
	// during power fail condition
	//
	updateDItoCT_CommErrorBit();

	//
	// Added on 15 Sep 2014 to avoid communication error logging
	// during power fail condition
	//
	updateCTtoDR_CommErrorBit();


	guiControlCommunicationFaultCopy = (uint32_t) gstControlCommunicationFault.val;
	guiControlApplicationFaultCopy = (uint32_t) gstControlApplicationFault.val;
	guiControlProcessorFaultCopy = (uint32_t) gstControlProcessorFault.val;

	guiDisplayCommunicationFaultCopy = (uint32_t) gstDisplayCommunicationFault.val;
	guiDisplayBoardHwFaultCopy = (uint32_t) gstDisplayBoardHwFault.val;
	guiDisplayApplicationFaultCopy = (uint32_t) gstDisplayApplicationFault.val;
	guiDisplayProcessorFaultCopy = (uint32_t) gstDisplayProcessorFault.val;

	guiDriveCommunicationFaultCopy = (uint32_t) gstDriveCommunicationFault.val;
	guiDriveMotorFaultCopy = (uint32_t) gstDriveMotorFault.val;
	guiDriveApplicationFaultCopy = (uint32_t) gstDriveApplicationFault.val;
	guiDriveProcessorfaultCopy = (uint32_t) gstDriveProcessorfault.val;
	guiDriveInstallationCopy = (uint32_t) gstDriveInstallation.val;

	// Check "Database handler" and "Communication" module are free, before start scanning the error
	// New variable 'gucStopErrorsDisplay' used to inform error module to stop displaying the error - YG - NOV 15
	if(
			gucCallEMtoLM_Module == 0 && gucCallMonitorLED_Handler == 0 && gucStopErrorsDisplay == 0 &&
			(eINACTIVE == gstEMtoLM.commandRequestStatus && eNO_STATUS == gstEMtoLM.commandResponseStatus) &&
			(eINACTIVE == gstEMtoCM_monitorLED.commandRequestStatus && eNO_STATUS == gstEMtoCM_monitorLED.commandResponseStatus)
	  )
	{

		for(lucCount=0; lucCount<TOTAL_ERRORS; lucCount++)
		{
			// Check whether a fault bit in global fault register is set
			if(
					// Check whether last value of anomaly bit at gstErrorList[lucCount].bitPositionInLocalFaultRegister position was 0
					((*gstErrorList[lucCount].localFaultRegister & (1 << gstErrorList[lucCount].bitPositionInLocalFaultRegister)) == 0) &&
					// Check whether new value of that bit is 1
					((*gstErrorList[lucCount].globalFaultRegister & (1 << gstErrorList[lucCount].bitPositionInGlobalFaultRegister)) != 0)
			)
			{
				// Log the anomaly if it is a recordable anomaly
				if(RECORDABLE_ANOMALY == gstErrorList[lucCount].recordType)
				{
					gucCallEMtoLM_Module = 1;
					gstEMtoLM.commandRequestStatus = eACTIVE;

					gstEMtoLM.errorToLM.anomalyCode = gstErrorList[lucCount].errorCode;
					if(gu8_language == Japanese_IDX)
					{
					memcpy(gstEMtoLM.errorToLM.errorDetails,gstErrorList[lucCount].errorName_japanese,sizeof(gstEMtoLM.errorToLM.errorDetails));
					}
					else
					{
				    memcpy(gstEMtoLM.errorToLM.errorDetails,gstErrorList[lucCount].errorName_english,sizeof(gstEMtoLM.errorToLM.errorDetails));
					}
					//
					//	Copy current timestamp value from HW register
					//
					gstEMtoLM.errorToLM.timeStamp = (HWREG(0x400FC000));
				}

				// Display error on OLED if indication on display is to be given
				if(DISPLAY_INDICATION == gstErrorList[lucCount].indicationOnDisplay)
				{
					errorDetails.errorCode = gstErrorList[lucCount].errorCode;
					errorDetails.errorType = gstErrorList[lucCount].recoveryType;
					if(gu8_language == Japanese_IDX)
					{
						memcpy(errorDetails.errordescription,gstErrorList[lucCount].errorName_japanese,sizeof(errorDetails.errordescription));
					}
					else
					{
						memcpy(errorDetails.errordescription,gstErrorList[lucCount].errorName_english,sizeof(errorDetails.errordescription));
					}
					//
					// Add anomaly to display active anomaly list
					//
					addToActiveAnomaly(&errorDetails);
				}

				// Update display board fault LED status in case it does not match its previous value and
				// current error has higher priority than the previous error
				if(
						(gstErrorList[lucCount].displayFaultLED_State != gstEMtoUM.faultLEDstatus) &&
						(gstErrorList[lucCount].displayFaultLED_State != OFF) &&
						(gstErrorList[lucCount].errorPriority > lsucLastErrorPriority)
				)
				{
//					luiFaultLED_Value = gstErrorList[lucCount].displayFaultLED_State;
					updateFaultLEDstatus();
				}

				// Set flag to call function to inform control board about monitor LED state in case a higher
				// priority error has occurred
				if(gstErrorList[lucCount].controlBoardMonitorLED_State != lsucLastMonitorLED_Status &&
						gstErrorList[lucCount].errorPriority > lsucLastErrorPriority)
				{
					//gucCallMonitorLED_Handler = 1;
					//gstEMtoCM_monitorLED.commandRequestStatus = eACTIVE;

					gstLEDcontrolRegister.monitorLED = gstErrorList[lucCount].controlBoardMonitorLED_State;
				}

				// Update the error in local register
				//*gstErrorList[lucCount].localFaultRegister = *gstErrorList[lucCount].globalFaultRegister;
				*gstErrorList[lucCount].localFaultRegister |= (1 << gstErrorList[lucCount].bitPositionInGlobalFaultRegister);

				// Update current value of error priority
				if(lsucLastErrorPriority < gstErrorList[lucCount].errorPriority)
				{
					lsucLastErrorPriority = gstErrorList[lucCount].errorPriority;
				}

				// Break FOR Loop
				lucCount = TOTAL_ERRORS;

			}// Check whether a fault bit in global fault register is set

			if(lucCount < TOTAL_ERRORS)
			{
				// Check whether fault bit in global fault register is cleared and clear that in
				// local fault register if it is previously set
				if(
						// Check whether an anomaly bit is set previously
						((*gstErrorList[lucCount].localFaultRegister & (1 << gstErrorList[lucCount].bitPositionInLocalFaultRegister)) != 0) &&
						// Check whether current value of that bit in global register is 0
						((*gstErrorList[lucCount].globalFaultRegister & (1 << gstErrorList[lucCount].bitPositionInGlobalFaultRegister)) == 0)
				)
				{
					// Update local fault register with current value of global register
//					*gstErrorList[lucCount].localFaultRegister &= (~(*gstErrorList[lucCount].globalFaultRegister & (1 << gstErrorList[lucCount].bitPositionInGlobalFaultRegister)));
					*gstErrorList[lucCount].localFaultRegister &= (~(1 << gstErrorList[lucCount].bitPositionInGlobalFaultRegister));

//					lucTemp = (*gstErrorList[lucCount].localFaultRegister & 0xFF);

					// In case of power-on calibration and installation remove the error code from active anomaly list
					/*if(gstDriveBoardStatus.bits.driveInstallation == 1 ||
							gstDriveBoardStatus.bits.drivePowerOnCalibration == 1 ||
							gstDriveBoardStatus.bits.driveReady == 1)*/
					{
						deleteFromActiveAnomaly(gstErrorList[lucCount].errorCode,false);
						lsucLastErrorPriority = updateFaultLEDstatus();
					}
				}
			}

		}	// for(lucCount=0; lucCount<TOTAL_ERRORS; lucCount++)

	} // if(gucCallEMtoLM_Module == 0 && gucCallMonitorLED_Handler == 0)

	// Log anomaly
	if(1 == gucCallEMtoLM_Module)
	{
		// logError() function is called only when gstEMtoLM.commandRequestStatus = eACTIVE
		returnValue = logError();

		if(eInit == returnValue)
		{
			gucCallEMtoLM_Module = 0;
		}

	}

#if 0
	// Monitor Anomaly History access on UI
	if(gstUMtoEM.commandToEM.bits.anomalyHistoryAccessed == 1)
	{
		// Delete complete active anomaly list
		deleteFromActiveAnomaly(0,true);
		gstEMtoUM.faultLEDstatus = 0xFF;
		gstUMtoEM.commandToEM.bits.anomalyHistoryAccessed = 0;
		lsucLastErrorPriority = 0;
	}
#endif

	// Send new monitor LED status to control board
/*
	if(1 == gucCallMonitorLED_Handler)
	{
		// controlBoardMonitorLED_Handler() function is called only when gstEMtoCM_monitorLED.commandRequestStatus = eACTIVE
		returnValue = controlBoardMonitorLED_Handler();
		if(eInit == returnValue)
		{
			gucCallMonitorLED_Handler = 0;
		}
	}
*/



/*

	if (gstDriveBoardStatus.bits.driveReady == 1 && (gstDriveBoardStatus.bits.driveFault == 0 && gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0))
	{
		gstLEDcontrolRegister.faultLED = LED_ON;
	}
	else if (
			(gstDriveBoardStatus.bits.driveReady == 1 || gstDriveBoardStatus.bits.drivePowerOnCalibration == 1 || gstDriveBoardStatus.bits.driveInstallation == 1) &&
			(
					(gstDriveBoardStatus.bits.driveFault == 0 && gstDriveBoardStatus.bits.driveFaultUnrecoverable == 0) ||
					((gstDriveBoardStatus.bits.driveFault == 1 || gstDriveBoardStatus.bits.driveFaultUnrecoverable == 1) && ERROR_LED_INDICATION_OVERRIDE_INSTALLATION == 0)
			)
			)
	{

		if(gstDriveInstallation.bits.installA100 == 1)
		{
			gstLEDcontrolRegister.faultLED = _BLINK_STATUS_500_MSEC;
		}
		else if(gstDriveInstallation.bits.installA101 == 1)
		{
			gstLEDcontrolRegister.faultLED = _BLINK_STATUS_250_MSEC;
		}
		else if(gstDriveInstallation.bits.installA102 == 1)
		{
			gstLEDcontrolRegister.faultLED = _BLINK_STATUS_100_MSEC;
		}
		else if (gstDriveBoardStatus.bits.drivePowerOnCalibration == 1)
		{
			gstLEDcontrolRegister.faultLED = _BLINK_STATUS_50_MSEC;
		}
	}
	else
	{
		gstLEDcontrolRegister.faultLED = luiFaultLED_Value;
	}
*/


} //void errorModule(void)

/********************************************************************************/

/******************************************************************************
 * logError
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
enum functionReturnValue logError(void)
{
	static enum functionReturnValue logErrorState = eInit;

	switch(logErrorState)
	{
	case eInit:
		if(eNO_STATUS == gstEMtoLM.commandResponseStatus)
		{
			// Set command flag
			//gstEMtoLM.commandRequestStatus = eACTIVE;
			logErrorState = eWaitingForReply;
			writeAnomalyHistory();
		}
		break;
	case eWaitingForReply:
		if(eSUCCESS == gstEMtoLM.commandResponseStatus)
		{
			gstEMtoLM.commandRequestStatus = eINACTIVE;
			gstEMtoLM.commandResponseStatus = eNO_STATUS;

			logErrorState = eInit;
		}
		else if(eFAIL == gstEMtoLM.commandResponseStatus || eTIME_OUT == gstEMtoLM.commandResponseStatus)
		{
			gstEMtoLM.commandRequestStatus = eINACTIVE;
			gstEMtoLM.commandResponseStatus = eNO_STATUS;

			logErrorState = eInit;
		}

		break;
	}

	return logErrorState;
}

/******************************************************************************
 * controlBoardMonitorLED_Handler
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
enum functionReturnValue controlBoardMonitorLED_Handler(void)
{
	static enum functionReturnValue monitoLED_HandlerState = eInit;

	switch(monitoLED_HandlerState)
	{
	case eInit:
		if(eNO_STATUS == gstEMtoCM_monitorLED.commandResponseStatus)
		{
			// Set monitor LED command flag
			gstEMtoCM_monitorLED.commandToControlBoard.bits.monitorLEDControl = 1;

			// Assign monitor LED control value
			gstEMtoCM_monitorLED.additionalCommandData = gstLEDcontrolRegister.monitorLED;

			gstEMtoCM_monitorLED.commandRequestStatus = eACTIVE;

			monitoLED_HandlerState = eWaitingForReply;
		}
		break;
	case eWaitingForReply:
		if(eSUCCESS == gstEMtoCM_monitorLED.commandResponseStatus)
		{
			gstEMtoCM_monitorLED.commandRequestStatus = eINACTIVE;
			gstEMtoCM_monitorLED.commandToControlBoard.val = 0;

			monitoLED_HandlerState = eInit;
		}
		else if(eFAIL == gstEMtoCM_monitorLED.commandResponseStatus || eTIME_OUT == gstEMtoCM_monitorLED.commandResponseStatus)
		{/*
			gstEMtoCM_monitorLED.commandRequestStatus = eINACTIVE;
			gstEMtoCM_monitorLED.commandToControlBoard.val = 0;

			monitoLED_HandlerState = eInit;*/
		}
		break;
	}

	return monitoLED_HandlerState;
}
/********************************************************************************/

/******************************************************************************
 * updateFaultLEDstatus
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
#if 0
void updateFaultLEDstatus(void)
{
	/*if(
			(gstDriveBoardStatus.bits.driveInstallation == 1) &&
			(gstDriveBoardStatus.bits.driveFault == 0)
	)
	{
		if(gstDriveInstallation.bits.installA100 == 1)
		{
			gstEMtoUM.faultLEDstatus = _BLINK_STATUS_500_MSEC;
		}
		else if(gstDriveInstallation.bits.installA101 == 1)
		{
			gstEMtoUM.faultLEDstatus = _BLINK_STATUS_250_MSEC;
		}
		else if(gstDriveInstallation.bits.installA102 == 1)
		{
			gstEMtoUM.faultLEDstatus = _BLINK_STATUS_100_MSEC;
		}
	}
	else */if(
			(gstDriveBoardStatus.bits.driveFault == 1) &&
			(gstDriveBoardStatus.bits.drivePowerOnCalibration == 0)
	)
	{
		if(
				(
						(gstDriveBoardFault.bits.driveApplication == 1) &&
						(
								(gstDriveApplicationFault.bits.originSensor == 1) ||
								(gstDriveApplicationFault.bits.hallSensor == 1) ||
								(gstDriveApplicationFault.bits.highInputVoltage == 1) ||
								(gstDriveApplicationFault.bits.lowInputVoltage == 1)
						) &&
						(
								(gstDriveApplicationFault.bits.microSwitchSensorLimit == 0)
						)
				) ||
				(
						(gstDriveBoardFault.bits.driveMotor == 1) &&
						(
								(gstDriveMotorFault.bits.motorOverheat == 1) ||
								(gstDriveMotorFault.bits.motorOpenphase == 1)
						)
				)
		)
		{
			gstEMtoUM.faultLEDstatus = ON;
		}
		else if(
				(gstDriveBoardFault.bits.driveApplication == 1) &&
				(gstDriveApplicationFault.bits.microSwitchSensorLimit == 1)
		)
		{
			gstEMtoUM.faultLEDstatus = _BLINK_STATUS_50_MSEC;;
		}
	}
}

#endif


uint8_t updateFaultLEDstatus(void)
{

	uint8_t lsucErrorPriority = 0;
	uint8_t luiFaultLED_Value = LED_OFF;

	uint8_t lucCount = 0;

	for(lucCount=0; lucCount<TOTAL_ERRORS; lucCount++)
	{
		// Check whether a fault bit in global fault register is set
		if(
				((*gstErrorList[lucCount].globalFaultRegister & (1 << gstErrorList[lucCount].bitPositionInGlobalFaultRegister)) != 0)
		)
		{
			// Update display board fault LED status in case current error has higher priority than the previous error
			if(	gstErrorList[lucCount].errorPriority > lsucErrorPriority)
			{
				luiFaultLED_Value = gstErrorList[lucCount].displayFaultLED_State;
				lsucErrorPriority = gstErrorList[lucCount].errorPriority;

			}
		}// Check whether a fault bit in global fault register is set

	}	// for(lucCount=0; lucCount<TOTAL_ERRORS; lucCount++)

	//
	// Update fault LED status
	//

	if (luiFaultLED_Value != LED_OFF)
	{
		gstEMtoUM.faultLEDstatus = luiFaultLED_Value;
	}
	else
	{
		gstEMtoUM.faultLEDstatus = LED_OFF;
	}
	return lsucErrorPriority;
}

/******************************************************************************
 * updateDItoCT_CommErrorBit
 *
 * Function Description:
 * This function will be called when gucDItoCT_CommError value is 1.
 * It will set the gstDisplayCommunicationFault.bits.commFailControl only when
 * gucDItoCT_CommError remains high for a specified time.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
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

void updateDItoCT_CommErrorBit(void)
{
	static enum functionState updateDItoCT_CommErrorFunctionState = eUpdate;
	static uint32_t luiDItoCT_FunctionEntryTime = 0;

	switch(updateDItoCT_CommErrorFunctionState)
	{
	case eUpdate:
		if(1 == gucDItoCT_CommError)
		{
			luiDItoCT_FunctionEntryTime = g_ui32TickCount;
			updateDItoCT_CommErrorFunctionState = eWait;
		}
		break;
	case eWait:
		if(get_timego(luiDItoCT_FunctionEntryTime) >= UPDATE_COMM_ERROR_TIME)
		{
			if(1 == gucDItoCT_CommError)
			{

				gstDisplayBoardStatus.bits.displayFault = 1;
				gstDisplayBoardFault.bits.displayCommunication = 1;
				gstDisplayCommunicationFault.bits.commFailControl = 1;
				//configureUART(UART_control);


				gucDItoCT_CommError = 0;

				updateDItoCT_CommErrorFunctionState = eUpdate;
			}
			else
			{
				updateDItoCT_CommErrorFunctionState = eUpdate;
			}

		}
		else if(0 == gucDItoCT_CommError)
		{
			updateDItoCT_CommErrorFunctionState = eUpdate;
		}
		break;
	}
}


/******************************************************************************
 * updateCTtoDR_CommErrorBit
 *
 * Function Description:
 * This function will be called when gucCTtoDR_CommError value is 1.
 * It will set the gstControlCommunicationFault.bits.commFailDrive only when
 * gucDItoCT_CommError remains high for a specified time.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void updateCTtoDR_CommErrorBit(void)
{
	static enum functionState updateCTtoDR_CommErrorFunctionState = eUpdate;
	static uint32_t luiCTtoDR_FunctionEntryTime = 0;

	switch(updateCTtoDR_CommErrorFunctionState)
	{
	case eUpdate:
		if(1 == gucCTtoDR_CommError)
		{
			luiCTtoDR_FunctionEntryTime = g_ui32TickCount;
			updateCTtoDR_CommErrorFunctionState = eWait;
		}
		break;
	case eWait:
		if(get_timego(luiCTtoDR_FunctionEntryTime) >= UPDATE_COMM_ERROR_TIME)
		{
			if(1 == gucCTtoDR_CommError)
			{
				gstControlCommunicationFault.bits.commFailDrive = 1;
				gucCTtoDR_CommError = 0;

				updateCTtoDR_CommErrorFunctionState = eUpdate;
			}
		}
		else if(0 == gucCTtoDR_CommError)
		{
			updateCTtoDR_CommErrorFunctionState = eUpdate;
		}
		break;
	}
}
