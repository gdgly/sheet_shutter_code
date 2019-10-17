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
 *  	0.1D	06/06/2014      	iGATE Offshore team       Initial Creation. Temporary functions written to
 *  														  test CMdi and CMDr modules.
 *  	0.2D	14/08/2014			iGATE Offshore team		  Actual error module function added.
 ****************************************************************************/

/****************************************************************************
 *  Include:
 ****************************************************************************/
#include <string.h>
//#include <inc/hw_memmap.h>
#include "errormodule.h"
#include "monitorledhandler.h"
#include <driverlib/pin_map.h>
#include <driverlib/rom_map.h>
#include <driverlib/rom.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include "intertaskcommunication.h"
#include "Middleware/serial.h"
#include "Middleware/sensorsdebounce.h"
#include "Drivers/extern.h"
#include "Drivers/systicktimer.h"
uint32_t snow_timer_cyw = 0;//modify 20160920 snow mode
const uint16_t para_snow_timer_cyw[3]={0,250,500};//modify 20160920 snow mode
extern uint8_t gu8_snow_mode;//modify 2016092 snow mode
uint32_t get_timego(uint32_t x_data_his);//modify 20160920 snow mode
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/
#define ERROR_LED_INDICATION_OVERRIDE_INSTALLATION	1

/****************************************************************************/

/****************************************************************************
 *  enum definitions:
 ****************************************************************************/
enum functionReturnValue
{
	eInit = 0,
	eWaitingForReply,
};

/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/


/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/
uint32_t guiControlCommunicationFaultCopy = 0;
uint32_t guiControlApplicationFaultCopy = 0;
uint32_t guiControlProcessorFaultCopy = 0;
/*

uint32_t guiDisplayCommunicationFaultCopy = 0;
uint32_t guiDisplayBoardHwFaultCopy = 0;
uint32_t guiDisplayApplicationFaultCopy = 0;
uint32_t guiDisplayProcessorFaultCopy = 0;
 */
uint8_t OB_Status = 0;

uint32_t	guiDriveCommunicationFaultCopy = 0;
uint32_t	guiDriveMotorFaultCopy = 0;
uint32_t	guiDriveApplicationFaultCopy = 0;
uint32_t	guiDriveProcessorfaultCopy = 0;
uint32_t	guiDriveInstallationCopy = 0;

uint8_t luiFaultLED_Value = 0;

uint32_t luiLastControlCommunicationFault = 0;
uint32_t luiLastControlApplicationFault = 0;
uint32_t luiLastControlProcessorFault = 0;
/*

uint32_t luiLastDisplayCommunicationFault = 0;
uint32_t luiLastDisplayBoardHwFault = 0;
uint32_t luiLastDisplayApplicationFault = 0;
uint32_t luiLastDisplayProcessorFault = 0;
 */

uint32_t	luiLastDriveCommunicationFault = 0;
uint32_t	luiLastDriveMotorFault = 0;
uint32_t	luiLastDriveApplicationFault = 0;
uint32_t	luiLastDriveProcessorfault = 0;
uint32_t	luiLastDriveInstallation = 0;


uint8_t gucCallMonitorLED_Handler = 0;

const _stErrorList gstErrorList [TOTAL_ERRORS] =
{
		/*DISPLAY BOARD ERROR LIST*/
/*
		{	200	,	"PACKET CRC ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DISP	,	0	,	&guiDisplayCommunicationFaultCopy	,	0	,	&luiLastDisplayCommunicationFault	,	0	}	,
		{	201	,	"DI-CT COMM ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayCommunicationFaultCopy	,	1	,	&luiLastDisplayCommunicationFault	,	1	}	,
		{	203	,	"UART ERR"			,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DISP	,	0	,	&guiDisplayCommunicationFaultCopy	,	3	,	&luiLastDisplayCommunicationFault	,	3	}	,
		{	211	,	"CARD DETECT ERR"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DISP	,	0	,	&guiDisplayBoardHwFaultCopy			,	0	,	&luiLastDisplayBoardHwFault			,	0	}	,
		{	212	,	"SD WRITE PRTCT"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DISP	,	0	,	&guiDisplayBoardHwFaultCopy			,	1	,	&luiLastDisplayBoardHwFault			,	1	}	,
		//	Added to log system power on event as requested by Bx on 21Apr2015
		{	213	,	"POWER ON EVENT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DISP	,	0	,	&guiDisplayBoardHwFaultCopy			,	3	,	&luiLastDisplayBoardHwFault			,	3	}	,
		{	221	,	"SD READ ERR"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayApplicationFaultCopy		,	0	,	&luiLastDisplayApplicationFault		,	0	}	,
		{	222	,	"SD WRITE ERR"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayApplicationFaultCopy		,	1	,	&luiLastDisplayApplicationFault		,	1	}	,
		{	231	,	"PARAM DB CRC"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	0	,	&luiLastDisplayProcessorFault		,	0	}	,
		{	232	,	"EEPROM PROGRAM"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	2	,	&luiLastDisplayProcessorFault		,	2	}	,
		{	233	,	"EEPROM ERASE"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	3	,	&luiLastDisplayProcessorFault		,	3	}	,
		{	234	,	"WATCHDOG TRIP"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	6	,	&luiLastDisplayProcessorFault		,	6	}	,
		{	235	,	"EEPROM READ"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DISP	,	1	,	&guiDisplayProcessorFaultCopy		,	7	,	&luiLastDisplayProcessorFault		,	7	}	,
*/
		/*CONTROL BOARD ERROR LIST*/
		{	101	,	"PACKET CRC ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	CTRL	,	0	,	&guiControlCommunicationFaultCopy	,	0	,	&luiLastControlCommunicationFault	,	0,NONE_OPEN_CLOSE	}	,
		{	102	,	"CT-DR COMM ERR"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlCommunicationFaultCopy	,	1	,	&luiLastControlCommunicationFault	,	1,NONE_OPEN_CLOSE	}	,
		{	103	,	"CONTR UART ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	CTRL	,	0	,	&guiControlCommunicationFaultCopy	,	3	,	&luiLastControlCommunicationFault	,	3,NONE_OPEN_CLOSE	}	,
		{	111	,	"OPR RESTRICT"		,	RECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlApplicationFaultCopy		,	0	,	&luiLastControlApplicationFault		,	0,NONE_OPEN_CLOSE	}	,
		{	113	,	"OBSTACLE"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlApplicationFaultCopy		,	1	,	&luiLastControlApplicationFault		,	1,OPEN_ONLY	}	,
		{	112	,	"STARTUP ERR"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlApplicationFaultCopy		,	2	,	&luiLastControlApplicationFault		,	2,OPEN_ONLY	}	,
		{	121	,	"PARAM DB CRC"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	0	,	&luiLastControlProcessorFault		,	0,NONE_OPEN_CLOSE	}	,
		{	122	,	"EEPROM PROGRAM"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	2	,	&luiLastControlProcessorFault		,	2,NONE_OPEN_CLOSE	}	,
		{	123	,	"EEPROM ERASE"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	3	,	&luiLastControlProcessorFault		,	3,NONE_OPEN_CLOSE	}	,
		{	124	,	"WATCHDOG"			,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	6	,	&luiLastControlProcessorFault		,	6,NONE_OPEN_CLOSE	}	,
		{	125	,	"EEPROM READ"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	CTRL	,	1	,	&guiControlProcessorFaultCopy		,	7	,	&luiLastControlProcessorFault		,	7,NONE_OPEN_CLOSE	}	,
		/*DRIVE BOARD ERROR LIST*/
/*		{	1	,	"PACKET CRC ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF	,	OFF	,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy	,	0	,	&luiLastDriveCommunicationFault	,	0	}	,
		{	2	,	"UART ERR"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF	,	OFF	,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy	,	1	,	&luiLastDriveCommunicationFault	,	1	}	,
		{	3	,	"COMD FRAME ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF	,	OFF	,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy	,	2	,	&luiLastDriveCommunicationFault	,	2	}	,
		{	11	,	"MTR OPEN PHASE"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_5	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveMotorFaultCopy	,	0	,	&luiLastDriveMotorFault	,	0	}	,
		{	12	,	"DRV DC OVER VTG"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveMotorFaultCopy	,	1	,	&luiLastDriveMotorFault	,	1	}	,
		{	13	,	"DRV OVER CURENT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_3	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveMotorFaultCopy	,	2	,	&luiLastDriveMotorFault	,	2	}	,
		{	14	,	"DRV TORQ 2 SEC"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveMotorFaultCopy	,	3	,	&luiLastDriveMotorFault	,	3	}	,
		{	15	,	"DRV PFC SHUTDWN"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveMotorFaultCopy	,	4	,	&luiLastDriveMotorFault	,	4	}	,
		{	16	,	"DRV MTR STALL"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveMotorFaultCopy	,	5	,	&luiLastDriveMotorFault	,	5	}	,
		{	17	,	"DRV OVERHEAT"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveMotorFaultCopy	,	6	,	&luiLastDriveMotorFault	,	6	}	,
		{	21	,	"PE OBSTACLE"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	0	,	&luiLastDriveApplicationFault	,	0	}	,
		{	22	,	"LOW INPUT VTG"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_6	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	1	,	&luiLastDriveApplicationFault	,	1	}	,
		{	23	,	"HIGH INPUT VTG"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_8	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	2	,	&luiLastDriveApplicationFault	,	2	}	,
		{	24	,	"HALL SENSOR"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_7	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	3	,	&luiLastDriveApplicationFault	,	3	}	,
		{	25	,	"ORIGIN SENSOR"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_2	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	4	,	&luiLastDriveApplicationFault	,	4	}	,
		{	26	,	"WRAP AROUND"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	5	,	&luiLastDriveApplicationFault	,	5	}	,
		{	27	,	"MICRO SWITCH"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	6	,	&luiLastDriveApplicationFault	,	6	}	,
		{	28	,	"AIR SW TRIGGER"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	7	,	&luiLastDriveApplicationFault	,	7	}	,
		{	29	,	"EMERGENCY STOP"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	8	,	&luiLastDriveApplicationFault	,	8	}	,
		{	30	,	"CALLIB FAIL"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	9	,	&luiLastDriveApplicationFault	,	9	}	,
		{	31	,	"MICRO SW LIMIT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	_BLINK_STATUS_50_MSEC	,	_BLINK_STATUS_50_MSEC	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	10	,	&luiLastDriveApplicationFault	,	10	}	,
		{	32	,	"OPR COUNT LIMIT"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	11	,	&luiLastDriveApplicationFault	,	11	}	,
		{	41	,	"FLASH IMG CRC"		,	NONRECOVERABLE	,	NONRECORDABLE_ANOMALY	,	OFF	,	OFF	,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveProcessorfaultCopy	,	0	,	&luiLastDriveProcessorfault	,	0	}	,
		{	42	,	"PARAM DB CRC"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveProcessorfaultCopy	,	1	,	&luiLastDriveProcessorfault	,	1	}	,
		{	43	,	"WATCHDOG TRIP"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveProcessorfaultCopy	,	2	,	&luiLastDriveProcessorfault	,	2	}	,
		{	44	,	"EEPROM PROGRAM"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveProcessorfaultCopy	,	3	,	&luiLastDriveProcessorfault	,	3	}	,
		{	45	,	"EEPROM ERASE"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON	,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveProcessorfaultCopy	,	4	,	&luiLastDriveProcessorfault	,	4	}	,
		{	46	,	"IGBT OVERHEAT"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION	,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	12	,	&luiLastDriveApplicationFault	,	12	}	,
*/
		{	1	,	"PACKET CRC ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy	,	0	,	&luiLastDriveCommunicationFault	,	0,NONE_OPEN_CLOSE	}	,
		{	2	,	"UART ERR"			,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy	,	1	,	&luiLastDriveCommunicationFault	,	1,NONE_OPEN_CLOSE	}	,
		{	3	,	"COMD FRAME ERR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveCommunicationFaultCopy	,	2	,	&luiLastDriveCommunicationFault	,	2,NONE_OPEN_CLOSE	}	,
		{	11	,	"MTR OPEN PHASE"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_5	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	0	,	&luiLastDriveMotorFault			,	0,NONE_OPEN_CLOSE	}	,
		{	12	,	"DRV DC OVER VTG"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	1	,	&luiLastDriveMotorFault			,	1,NONE_OPEN_CLOSE	}	,
		{	13	,	"DRV OVER CURENT"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_3	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	2	,	&luiLastDriveMotorFault			,	2,NONE_OPEN_CLOSE	}	,
		{	14	,	"DRV TORQ 2 SEC"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	3	,	&luiLastDriveMotorFault			,	3,NONE_OPEN_CLOSE	}	,
		{	15	,	"DRV PFC SHUTDWN"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	4	,	&luiLastDriveMotorFault			,	4,NONE_OPEN_CLOSE	}	,
		{	16	,	"DRV MTR STALL"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	5	,	&luiLastDriveMotorFault			,	5,NONE_OPEN_CLOSE	}	,
		{	17	,	"DRV OVERHEAT"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	6	,	&luiLastDriveMotorFault			,	6,NONE_OPEN_CLOSE	}	,
		{	18	,	"MTR SUSTAIN"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	7	,	&luiLastDriveMotorFault			,	7,NONE_OPEN_CLOSE	}	,
		{	19	,	"MTR PWM COAST"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	8	,	&luiLastDriveMotorFault			,	8,NONE_OPEN_CLOSE	}	,
		{	20	,	"MTR OTHER ERROR"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveMotorFaultCopy			,	9	,	&luiLastDriveMotorFault			,	9,NONE_OPEN_CLOSE	}	,
		{	21	,	"PE OBSTACLE"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DRIV	,	0	,	&guiDriveApplicationFaultCopy	,	0	,	&luiLastDriveApplicationFault	,	0,OPEN_ONLY	}	,
		{	22	,	"LOW INPUT VTG"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_6	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	1	,	&luiLastDriveApplicationFault	,	1,NONE_OPEN_CLOSE	}	,
		{	23	,	"HIGH INPUT VTG"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_8	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	2	,	&luiLastDriveApplicationFault	,	2,NONE_OPEN_CLOSE	}	,
		{	24	,	"HALL SENSOR"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_7	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	3	,	&luiLastDriveApplicationFault	,	3,NONE_OPEN_CLOSE	}	,
		{	26	,	"WRAP AROUND"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	OFF						,	OFF						,	DISPLAY_INDICATION		,	DRIV	,	0	,	&guiDriveApplicationFaultCopy	,	4	,	&luiLastDriveApplicationFault	,	4,NONE_OPEN_CLOSE	}	,
		{	27	,	"MICRO SWITCH"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy	,	5	,	&luiLastDriveApplicationFault	,	5,OPEN_ONLY	}	,
		{	28	,	"AIR SW TRIGGER"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy	,	6	,	&luiLastDriveApplicationFault	,	6,NONE_OPEN_CLOSE	}	,
		{	29	,	"EMERGENCY STOP"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	7	,	&luiLastDriveApplicationFault	,	7,NONE_OPEN_CLOSE	}	,
		{	30	,	"CALLIB FAIL"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	13	,	&luiLastDriveApplicationFault	,	13,NONE_OPEN_CLOSE	}	,
		{	31	,	"MICRO SW LIMIT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	_BLINK_STATUS_50_MSEC	,	_BLINK_STATUS_50_MSEC	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	14	,	&luiLastDriveApplicationFault	,	14,ENABLE_OPEN_CLOSE	}	,
		{	32	,	"OPR COUNT LIMIT"	,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveApplicationFaultCopy	,	15	,	&luiLastDriveApplicationFault	,	15,ENABLE_OPEN_CLOSE	}	,
		// "POWER FAIL" will use to indicate the situation during power fail - YG - NOV 15
		{	33	,	"POWER FAIL"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	17	,	&luiLastDriveApplicationFault	,	17,NONE_OPEN_CLOSE	}	,
		{	50	,	"FLASH IMG CRC"		,	RECOVERABLE		,	NONRECORDABLE_ANOMALY	,	OFF						,	OFF						,	NO_DISPLAY_INDICATION	,	DRIV	,	0	,	&guiDriveProcessorfaultCopy		,	0	,	&luiLastDriveProcessorfault		,	0,NONE_OPEN_CLOSE	}	,
		{	51	,	"PARAM DB CRC"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy		,	1	,	&luiLastDriveProcessorfault		,	1,NONE_OPEN_CLOSE	}	,
		{	52	,	"WATCHDOG TRIP"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy		,	2	,	&luiLastDriveProcessorfault		,	2,NONE_OPEN_CLOSE	}	,
		{	53	,	"EEPROM PROGRAM"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy		,	3	,	&luiLastDriveProcessorfault		,	3,NONE_OPEN_CLOSE	}	,
		{	54	,	"EEPROM ERASE"		,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveProcessorfaultCopy		,	4	,	&luiLastDriveProcessorfault		,	4,NONE_OPEN_CLOSE	}	,
		//	Error numbers updated as per error category - YG - NOV 2015
		{	34	,	"IGBT OVERHEAT"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_4	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	16	,	&luiLastDriveApplicationFault	,	16,NONE_OPEN_CLOSE	}	,
		{	35	,	"OS NOT DTCT UP"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	8	,	&luiLastDriveApplicationFault	,	8,NONE_OPEN_CLOSE	}	,
		{	36	,	"OS NOT DTCT DWN"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	9	,	&luiLastDriveApplicationFault	,	9,NONE_OPEN_CLOSE	}	,
		{	37	,	"OS ON UPPER LMT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	10	,	&luiLastDriveApplicationFault	,	10,NONE_OPEN_CLOSE}	,
		{	38	,	"OS OFF LWR LMT"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	11	,	&luiLastDriveApplicationFault	,	11,NONE_OPEN_CLOSE	}	,
		{	39	,	"OS VALIDT FAIL"	,	RECOVERABLE		,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	12	,	&luiLastDriveApplicationFault	,	12,NONE_OPEN_CLOSE	}	,
		//	Two new errors added to monitor false shutter movement - YG - NOV 2015
		{	40	,	"WRONG MOVE UP"		,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	18	,	&luiLastDriveApplicationFault	,	18,NONE_OPEN_CLOSE	}	,
		{	41	,	"WRONG MOVE DOWN"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	19	,	&luiLastDriveApplicationFault	,	19,NONE_OPEN_CLOSE	}	,
		{	42	,	"MTR CABLE FAULT"	,	NONRECOVERABLE	,	RECORDABLE_ANOMALY		,	ON						,	_FLASH_STATUS_COUNT_1	,	DISPLAY_INDICATION		,	DRIV	,	1	,	&guiDriveApplicationFaultCopy	,	20	,	&luiLastDriveApplicationFault	,	20,NONE_OPEN_CLOSE	}	,
};

/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
 ****************************************************************************/
/******************************************************************************
 * updateMonitorLEDstatus
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
uint8_t updateMonitorLEDstatus(void);

/******************************************************************************
 * updateControlBoardApplicationFault
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Function definitions for this file:
 ****************************************************************************/


/******************************************************************************
 * errorModuleToTestCMDr
 *
 * Function Description:
 * This function is temporarily created to test CMDr. It clears the EM to CMDr
 * command being activated in main.c file for functional testing.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void errorModuleToTestCMDr(void);

/******************************************************************************
 * errorModuleToTestCMDi
 *
 * Function Description:
 * This function is temporarily created to test CMDi. It checks if a command is
 * received from CMDi and responds accordingly.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void errorModuleToTestCMDi(void);


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

	static uint8_t lsucLastErrorPriority = 0;
	static uint8_t lsucLastMonitorLED_Status = 0;

	uint8_t lucCount = 0;

	updateControlBoardApplicationFaults();

	guiControlCommunicationFaultCopy = (uint32_t) gstControlCommunicationFault.val;
	guiControlApplicationFaultCopy = (uint32_t) gstControlApplicationFault.val;
	guiControlProcessorFaultCopy = (uint32_t) gstControlProcessorFault.val;
	/*

	guiDisplayCommunicationFaultCopy = (uint32_t) gstDisplayCommunicationFault.val;
	guiDisplayBoardHwFaultCopy = (uint32_t) gstDisplayBoardHwFault.val;
	guiDisplayApplicationFaultCopy = (uint32_t) gstDisplayApplicationFault.val;
	guiDisplayProcessorFaultCopy = (uint32_t) gstDisplayProcessorFault.val;
	 */

	guiDriveCommunicationFaultCopy = (uint32_t) gstDriveCommunicationFault.val;
	guiDriveMotorFaultCopy = (uint32_t) gstDriveMotorFault.val;
	guiDriveApplicationFaultCopy = (uint32_t) gstDriveApplicationFault.val;
	guiDriveProcessorfaultCopy = (uint32_t) gstDriveProcessorfault.val;
	guiDriveInstallationCopy = (uint32_t) gstDriveInstallation.val;



	for(lucCount=0; lucCount<TOTAL_ERRORS; lucCount++)
	{
		// Check whether a fault bit in global fault register is set
		if(
				// Check whether last value of anomaly bit at gstErrorList[lucCount].bitPositionInLocalFaultRegister position was 0
				(((*gstErrorList[lucCount].localFaultRegister) & (1 << gstErrorList[lucCount].bitPositionInLocalFaultRegister)) == 0) &&
				// Check whether new value of that bit is 1
				(((*gstErrorList[lucCount].globalFaultRegister) & (1 << gstErrorList[lucCount].bitPositionInGlobalFaultRegister)) != 0)
		)
		{
			/*
			// Set flag to call function to inform control board about monitor LED state in case a higher
			// priority error has occurred
			if((gstErrorList[lucCount].controlBoardMonitorLED_State != lsucLastMonitorLED_Status) &&
					(gstErrorList[lucCount].errorPriority > lsucLastErrorPriority))
			{
				gucCallMonitorLED_Handler = 1;

				gstMonitorLEDControlRegister.monitorLEDstatus = gstErrorList[lucCount].controlBoardMonitorLED_State;
			}
			 */

			// Update the error in local register
			*gstErrorList[lucCount].localFaultRegister |= ((*gstErrorList[lucCount].globalFaultRegister) & (1 << gstErrorList[lucCount].bitPositionInGlobalFaultRegister));
			lsucLastMonitorLED_Status = updateMonitorLEDstatus();

			/*
			// Update current value of error priority
			if(lsucLastErrorPriority < gstErrorList[lucCount].errorPriority)
			{
				lsucLastErrorPriority = gstErrorList[lucCount].errorPriority;
			}
			 */

			// Break FOR Loop
			lucCount = TOTAL_ERRORS;

		}// Check whether a fault bit in global fault register is set

		// Check whether lucCount is equated to TOTAL_ERRORS in above if loop
		// to break for loop after a fault bit is found set.
		if(lucCount < TOTAL_ERRORS)
		{
			// Check whether fault bit in global fault register is cleared and clear that in
			// local fault register if it was previously set
			if(
					// Check whether an anomaly bit is set previously
					(((*gstErrorList[lucCount].localFaultRegister) & (1 << gstErrorList[lucCount].bitPositionInLocalFaultRegister)) != 0) &&
					// Check whether current value of that bit in global register is 0
					(((*gstErrorList[lucCount].globalFaultRegister) & (1 << gstErrorList[lucCount].bitPositionInGlobalFaultRegister)) == 0)
			)
			{
				// Update local fault register with current value of global register
				*gstErrorList[lucCount].localFaultRegister &= ~(1 << gstErrorList[lucCount].bitPositionInGlobalFaultRegister);
				lsucLastMonitorLED_Status = updateMonitorLEDstatus();
			}
		}

	}	// for(lucCount=0; lucCount<TOTAL_ERRORS; lucCount++)





	if (
			(gstDriveStatus.bits.driveReady == 1) &&
			((gstDriveStatus.bits.driveFault == 0) && (gstDriveStatus.bits.driveFaultUnrecoverable == 0))
		)
	{
		gstMonitorLEDControlRegister.monitorLEDstatus = LED_ON;
	}
	else if (
			(gstDriveStatus.bits.driveInstallation == 1) &&
			(
					((gstDriveStatus.bits.driveFault == 0) && (gstDriveStatus.bits.driveFaultUnrecoverable == 0)) ||
					(((gstDriveStatus.bits.driveFault == 1) || (gstDriveStatus.bits.driveFaultUnrecoverable == 1)) && (ERROR_LED_INDICATION_OVERRIDE_INSTALLATION == 0))
			)
	)
	{

		if(gstDriveInstallation.bits.installA100 == 1)
		{
			gstMonitorLEDControlRegister.monitorLEDstatus = _BLINK_STATUS_500_MSEC;
		}
		else if(gstDriveInstallation.bits.installA101 == 1)
		{
			gstMonitorLEDControlRegister.monitorLEDstatus = _BLINK_STATUS_250_MSEC;
		}
		else if(gstDriveInstallation.bits.installA102 == 1)
		{
			gstMonitorLEDControlRegister.monitorLEDstatus = _BLINK_STATUS_100_MSEC;
		}
		else
		{
			gstMonitorLEDControlRegister.monitorLEDstatus = LED_ON;
		}

	}
	else if (
			(gstDriveStatus.bits.drivePowerOnCalibration == 1) &&
				(
						((gstDriveStatus.bits.driveFault == 0) && (gstDriveStatus.bits.driveFaultUnrecoverable == 0)) ||
						(((gstDriveStatus.bits.driveFault == 1) || (gstDriveStatus.bits.driveFaultUnrecoverable == 1)) && (ERROR_LED_INDICATION_OVERRIDE_INSTALLATION == 0))
				)
	)
	{

		gstMonitorLEDControlRegister.monitorLEDstatus = _BLINK_STATUS_50_MSEC;

	}
	else
	{
		gstMonitorLEDControlRegister.monitorLEDstatus = lsucLastMonitorLED_Status;
	}




} //void errorModule(void)

/********************************************************************************/

/******************************************************************************
 * updateMonitorLEDstatus
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
uint8_t updateMonitorLEDstatus(void)
{

	uint8_t lsucErrorPriority = 0;
	static uint8_t luiFaultLED_Value = LED_ON;
	uint8_t lucErrorOccurrenceFlag = 0;

	uint8_t lucCount = 0;

	for(lucCount=0; lucCount<TOTAL_ERRORS; lucCount++)
	{
		// Check whether a fault bit in global fault register is set
		if(
				(((*gstErrorList[lucCount].globalFaultRegister) & (1 << gstErrorList[lucCount].bitPositionInGlobalFaultRegister)) != 0)
		)
		{
			lucErrorOccurrenceFlag = 1;
			// Update display board fault LED status in case current error has higher priority than the previous error
			if(	gstErrorList[lucCount].errorPriority > lsucErrorPriority)
			{
				luiFaultLED_Value = gstErrorList[lucCount].controlBoardMonitorLED_State;
				lsucErrorPriority = gstErrorList[lucCount].errorPriority;
			}
		}// Check whether a fault bit in global fault register is set

	}	// for(lucCount=0; lucCount<TOTAL_ERRORS; lucCount++)

	if(0 == lucErrorOccurrenceFlag)
	{
		//
		//	If there is no error change fault LED state to ON
		//
		luiFaultLED_Value = LED_ON;
	}

	//
	// Update fault LED status
	//

/*
	if (luiFaultLED_Value != 0xFF)
	{
		gstMonitorLEDControlRegister.monitorLEDstatus = luiFaultLED_Value;
	}

	return lsucErrorPriority;
*/

	return luiFaultLED_Value;
}

/******************************************************************************
 * updateControlBoardApplicationFault
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void updateControlBoardApplicationFaults(void)
{
	OB_Status = MAP_GPIOPinRead(0x40007000, GPIO_INT_PIN_2) >> 2;
	//gstControlApplicationFault.bits.obstacleSensor = gSensorStatus.bits.Sensor_Safety_active;//add cyw
	if(OB_Status == 0)
	{

		gSensorStatus.bits.Sensor_Safety_active = true;
		gSensorStatus.bits.Sensor_Safety_inactive = false;

		// Set drive status menu parameter
		gstDriveStatusMenu.bits.Startup_Safety_Status = 1;
	}
	else
	{
		snow_timer_cyw = g_ui32TickCount;//modify 20160920 snow mode
		gSensorStatus.bits.Sensor_Safety_active = false;

		gSensorStatus.bits.Sensor_Safety_active = false;
		// Reset drive status menu parameter
		gstDriveStatusMenu.bits.Startup_Safety_Status = 0;
	}

	if(
			gSensorStatus.bits.Sensor_Safety_active == true		&&

			(
					gstDriveStatus.bits.ignPhotoElecSensLimRchd == 0 &&
					gstDriveStatus.bits.shutterLowerLimit == 0		 &&
					((gstDriveStatus.bits.shutterMovingDown == 1||gstDriveStatus.bits.shutterUpperLimit==1) ||
						(gstDriveStatus.bits.shutterStopped && gstDriveStatus.bits.shutterLowerLimit == 0 &&
							gstDriveStatus.bits.shutterUpperLimit == 0 ))
			)

	  )
	{
		if(get_timego(snow_timer_cyw)  > para_snow_timer_cyw[gu8_snow_mode])//modify 20160920 snow mode
		{
		gstControlApplicationFault.bits.startupSafetySensor = 1;
		gstControlBoardFault.bits.controlApplication = 1;
		gstControlBoardStatus.bits.controlFault = 1;
		}
	}
	else if(gSensorStatus.bits.Sensor_Safety_active == false)
	{
		gstControlApplicationFault.bits.startupSafetySensor = 0;
		if(gstControlApplicationFault.val == 0)
		{
			gstControlBoardFault.bits.controlApplication = 0;
		}
		if(gstControlBoardFault.val == 0)
		{
			gstControlBoardStatus.bits.controlFault = 0;
		}
	}
}


/******************************************************************************
 * errorModuleToTestCMDr
 *
 * Function Description:
 * This function is temporarily created to test CMDr. It clears the LS to CMDr
 * command being activated in main.c file for functional testing.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void errorModuleToTestCMDr(void)
{
	if(gstEMtoCMDr.commandResponseStatus == eSUCCESS || gstEMtoCMDr.commandResponseStatus == eTIME_OUT)	// Active command processed
	{
		if(gstEMtoCMDr.commandResponseStatus == eSUCCESS)
		{
			//
			//	include code to write error data on EEPROM
			//
			gstEMtoCMDr.commandResponseACK_Status = eResponseAcknowledgement_ACK;
		}
	}
	if(gstEMtoCMDr.commandResponseACK_Status == eResponseAcknowledgementProcessed)
	{
		gstEMtoCMDr.commandRequestStatus = eINACTIVE;
		gstEMtoCMDr.commandToDriveBoard.val = 0;	// Clear the processed command
		gstEMtoCMDr.commandResponseACK_Status = eNO_StatusAcknowledgement;
	}
}

/********************************************************************************/

/******************************************************************************
 * errorModuleToTestCMDi
 *
 * Function Description:
 * This function is temporarily created to test CMDi. It checks if a command is
 * received from CMDi and responds accordingly.
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

void errorModuleToTestCMDi(void)
{
	//	uint8_t lucMonitrorLED_blinkRate = 0;
	if(gstCMDitoMLH.commandRequestStatus == eACTIVE)
	{
		if(gstCMDitoMLH.commandDisplayBoardMLH.bits.monitorLEDControl == 1)
		{
			//			lucMonitrorLED_blinkRate = gstCMDitoMLH.additionalCommandData;

			gstCMDitoMLH.acknowledgmentReceived = eACK;
			gstCMDitoMLH.commandResponseStatus = eSUCCESS;
		}
	}
}



