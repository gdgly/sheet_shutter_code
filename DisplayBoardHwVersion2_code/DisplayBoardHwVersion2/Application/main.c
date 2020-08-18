/*********************************************************************************
 * FileName: main.c
 * Description:
 * This source file is main file for Bx Shutter Control Board
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
 *  	0.2D	11/04/2014			iGATE Offshore team		  Addition of Serial Module
 *  	0.1D	27/03/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Include:
 ****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/sysctl.h>
#include <driverlib/eeprom.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include "driverlib/sysctl.h"
#include <driverlib/uart.h>
#include <inc/hw_memmap.h>

#include "Middleware/display.h"
#include "Middleware/eeprom.h"
//#include "Middleware/cfal96x64x16.h"
#include "Middleware/bolymindisplay.h"
#include "Drivers/systicktimer.h"
#include "Middleware/debounce.h"
#include "Middleware/serial.h"
#include "Middleware/rtc.h"
#include "userinterface.h"
//#include "intertaskcommunication.h"
#include "communicationmodule.h"

#include "Test_Module/testdisplay.h"
#include "Test_Module/testrtc.h"
#include "Test_Module/testeeprom.h"
#include "Test_Module/testdebounce.h"
#include "Test_Module/testserial.h"
#include "Test_Module/testledhandler.h"

#include "Middleware/uartstdio.h"

#include "Middleware/sdcard.h"
#include "Drivers/watchdogtimer.h"
#include "Middleware/paramdatabase.h"
#include "Application/userinterface.h"
#include "Application/ledhandler.h"
#include "ustdlib.h"
#include "sdcardlogs.h"
#include "logger.h"
#include "errormodule.h"
#include "backlight_control.h"

#include "Gesture_Sensor/GP2AP054A_cyw.h"//add cyw
#include "Gesture_Sensor/ram_cyw.h"//add cyw
#include "Middleware/eeprom_cyw.h"//add cyw
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/
//#define TEST_DEBOUNCE
//#define DISABLE_WATCHDOG
//#define TEST_PARAMDB1
//#define TESTING_UART
//#define TEST_EEPROM
//#define TEST_RTC
//#define TEST_MENUS
//#define TEST_DATE_TIME
//#define TEST_HOME_SCREEN
//#define TEST_DATE_TIME
//#define TEST_SDCARD

// Power on state for initiating "START_INSTALLATION" command
#define START_INSTALLATION		4

#define HEART_BEAT_DURATION		200		//	2 Sec using 10mS tick counter

#define PORTF_POWERLEDPIN	(GPIO_PIN_4)
#define PORTE_AUTOMANUALLEDPIN (GPIO_PIN_0)
#define PORTE_FAULTLEDPIN (GPIO_PIN_1)
/****************************************************************************/

/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/

uint32_t ui32OperationCount;
uint32_t gTickCount3Seconds = 0;



//
// This variable contains the address of the currently active functional block
//
stInternalFunctions *psActiveFunctionalBlock = &gsPowerOnFunctionalBlock;

//
// This variable contains the address of the currently active menu.
//
stMenu *psActiveMenu = &gsMainMenu;

//
//	Variable for inner task communication in communication module
//
extern _CommunicationModuleInnerTaskComm lstCommunicationModuleInnerTaskComm;
extern uint8_t guchRxBufferOverflowState[UART_CHANNEL_COUNT];// Receive buffer overflow state
extern uint8_t guchUARTMapToGlobalBuffer[MAX_UART_AVAILABLE];

// Control to drive communication error occurrence count
extern uint8_t guchCommunicationCRC_ErrorOccurrence[UART_CHANNEL_COUNT];

/****************************************************************************/

/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/

/****************************************************************************/

/****************************************************************************
 *  Function prototypes for this file:
 ****************************************************************************/
void initDisplayBoard(void);
void userInterfaceModule(void);
void runTimeFunction(void);
void logWatchDogTimerError(void);

/****************************************************************************/

/******************************************************************************
 * main
 *
 * Function Description:
 * main function
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
//char test_test[]="傊";
//#define DISABLE_WATCHDOG 0 //jingzhikanmengou


void main(void) {
	uint8_t ui8ButtonChanged; 		//Debounce mod
	uint32_t ui32LastTickCount = 0; //Debounce mod
	uint32_t ui32TickCount5Seconds = 0;

	uint64_t ui64LastTickCount = 0; //Debounce mod

#ifndef DEBUG_DISABLED
	uint32_t ui32HeartBeatCount = 0;
#endif

	initDisplayBoard();

#ifndef	TEST_COMM_MODULE
	configureUART(UART_debug);
	configureUART(UART_control);
	initCommunicationModuleGlobalRegisters();

	//	Log watchdog error
	logWatchDogTimerError();

#ifndef DISABLE_WATCHDOG
	doWatchdogReset();
#endif

#ifdef WATCHDOG_TEST

#ifndef DEBUG_DISABLED
	ui32HeartBeatCount = g_ui32TickCount;
#endif
//    UARTprintf("\n************* START ***************\n");
	while(1)
	{
#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif
		userInterfaceModule();
		communicationModuleControlBoard();
		displayBoardLEDHandler();
		errorModule();

#ifndef DEBUG_DISABLED
		if((g_ui32TickCount - ui32HeartBeatCount) >= HEART_BEAT_DURATION)
		{
			ui32HeartBeatCount = g_ui32TickCount;

			uartSendTxBuffer(UART_debug,"H",1);
		}
#endif
	}

#endif	//	WATCHDOG_TEST

#ifdef TEST_HANDLE_CONTROL_CMD_RESP
	lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE;
	lstCommunicationModuleInnerTaskComm.commandResponseACK_Status = eNO_StatusAcknowledgement;

#ifdef UM_CM_OPERATIONAL
	lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.autoManualSelect = 1;
#endif	//	UM_CM_OPERATIONAL

#ifdef UM_CM_getParameter_control
	lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 1;
	lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;

	lstCommunicationModuleInnerTaskComm.parameterNumber = 602;
#endif	//	UM_CM_getParameter_control

#ifdef UM_CM_getParameter_drive
	lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.getParameter = 1;
	lstCommunicationModuleInnerTaskComm.destination = eDestDriveBoard;

	lstCommunicationModuleInnerTaskComm.parameterNumber = 640;
#endif	//	UM_CM_getParameter_drive

#ifdef UM_CM_setParameter_conrtol
	lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.setParameter = 1;
	lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;

	lstCommunicationModuleInnerTaskComm.parameterNumber = 412;

	lstCommunicationModuleInnerTaskComm.parameterValue = 999999;
#endif	//	UM_CM_setParameter_conrtol

#ifndef UM_CM_setParameter_drive
	lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.setParameter = 1;
	lstCommunicationModuleInnerTaskComm.destination = eDestDriveBoard;

	lstCommunicationModuleInnerTaskComm.parameterNumber = 512;

	lstCommunicationModuleInnerTaskComm.parameterValue = 88888888;
#endif	//	UM_CM_setParameter_drive

	while(1)
	{
		handleControlCmdResp();
		if(lstCommunicationModuleInnerTaskComm.commandResponseStatus == eSUCCESS)
		{
			lstCommunicationModuleInnerTaskComm.commandToControlBoard.val = 0;
			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
		}
	}
#endif	//	TEST_HANDLE_CONTROL_CMD_RESP

#ifdef TEST_POLL_DRIVE_CONTROL_STATUS
	while(1)
	{
		pollDriveControlStatusFault();
		handleControlCmdResp();
	}
#endif	//	TEST_POLL_DRIVE_CONTROL_STATUS

#ifdef TEST_COMMNAD_UM_EM_LH

#ifndef UMtoCMoperational
	gstUMtoCMoperational.commandRequestStatus = eACTIVE;
	gstUMtoCMoperational.commandToControlBoard.bits.stopPressed= 1;
#endif	//	UMtoCMoperational

#ifndef UMtoCMdatabase

#ifdef SETTIMESTAMP
	gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
	gstUMtoCMdatabase.commandToControlBoard.bits.setTimeStamp = 1;
	gstUMtoCMdatabase.dataToControlBoard.commandData.timeStamp = 0x23456789;
#endif	//	SETTIMESTAMP

#ifdef GETPARAMETER
	gstUMtoCMdatabase.commandRequestStatus = eACTIVE;
	gstUMtoCMdatabase.commandToControlBoard.bits.getParameter = 1;
	gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 600;
#endif //	GETPARAMETER

#endif	//	UMtoCMdatabase

	while(1)
	{
		communicationModuleControlBoard();
		userModuleToTestCM();
	}
#endif //	TEST_COMMNAD_UM_EM_LH

#endif	//	TEST_COMM_MODULE

#ifdef TEST_DEBOUNCE
	testDebounceSetup();

	while(1)
	{
		if(g_ui32TickCount != ui32LastTickCount)
		{
			// Remember last tick count
			ui32LastTickCount = g_ui32TickCount;

			// Read the debounced state of the buttons.
			//keysPoll(&ui8ButtonChanged, 0);

#ifdef TEST_DEBOUNCE
			testKeyDebounceModule();
#endif

			//
			// Handle menu key press events
			//
			//userInterfaceModule();

			//communicationModule();
		}

		keysPoll_F(&ui8ButtonChanged, 0);
		keysPoll_D(&ui8ButtonChanged, 0);
		keysPoll_E(&ui8ButtonChanged, 0);
	}
#endif

#ifdef TEST_MENUS
	homeScreenPaint();
	homeScreenMode();
	//retVal = menuPaintFirstScreen();
	//retVal = menuEnter();
	retVal = menuMode();
	//retVal = menuMode();
	//retVal = menuDown();
	/*menuDown(&gsMainMenu);
	 menuDown(&gsMainMenu);
	 menuDown(&gsMainMenu);
	 menuDown(&gsMainMenu);
	 menuDown(&gsMainMenu);
	 menuDown(&gsMainMenu);
	 menuDown(&gsMainMenu);
	 menuDown(&gsMainMenu);*/
	//retVal = menuUp();
	/*menuUp(&gsMainMenu);
	 menuUp(&gsMainMenu);
	 menuUp(&gsMainMenu);
	 menuUp(&gsMainMenu);
	 menuUp(&gsMainMenu);
	 menuUp(&gsMainMenu);
	 menuUp(&gsMainMenu);*/
#endif

#ifdef TEST_DATE_TIME
	dateTimeFirstScreen();
	dateTimeEnter();
	dateTimeUp();
	dateTimeUp();
	dateTimeDown();
	dateTimeDown();
	dateTimeEnter();
	dateTimeUp();
	dateTimeUp();
	dateTimeDown();
	dateTimeDown();
	dateTimeEnter();
	dateTimeUp();
	dateTimeUp();
	dateTimeDown();
	dateTimeDown();
	dateTimeEnter();
	dateTimeUp();
	dateTimeUp();
	dateTimeDown();
	dateTimeDown();
	dateTimeEnter();
	dateTimeUp();
	dateTimeUp();
	dateTimeDown();
	dateTimeDown();
	dateTimeEnter();
	dateTimeUp();
	dateTimeUp();
	dateTimeDown();
	dateTimeDown();
	dateTimeEnter();
	dateTimeUp();
	dateTimeUp();
	dateTimeDown();
	dateTimeDown();
	dateTimeEnter();
	dateTimeUp();
	dateTimeUp();
	dateTimeDown();
	dateTimeDown();
	dateTimeEnter();
	dateTimeUp();
	dateTimeUp();
	dateTimeDown();
	dateTimeDown();
	dateTimeMode();
	dateTimeMode();
	dateTimeMode();
	dateTimeMode();
	dateTimeMode();
	dateTimeMode();
	dateTimeMode();
	dateTimeMode();
	dateTimeMode();
//	displayText("DATE AND TIME", 0, 0, false, false, false, false);
//	displayText("01", 0 , 16, false, false, false, false);
//	displayText("JAN", 18 , 16, false, false, false, false);
//	displayText("1", 42 , 16, false, false, false, false);
//	displayText("9", 51 , 16, false, false, false, false);
//	displayText("7", 60 , 16, false, false, false, false);
//	displayText("0", 69 , 16, false, false, false, false);
//	displayText("00", 0 , 32, false, false, false, false);
//	displayText(":", 15 , 32, false, false, false, false);
//	displayText("00",24 , 32, false, false, false, false);
//	displayText("AM",45 , 32, false, false, false, false);
#endif

#ifdef TEST_HOME_SCREEN
	//homeScreenRunTime();
	//communicationModule();
	//homeScreenRunTime();
	//homeScreenPaint();
	retval = displayText("OPERATION", 2, 0, false, false, false, true);
#endif

	// Wait for 2 second before scanning the key

	uint8_t lucWaitforDelay = 1;

	ui32LastTickCount = g_ui32TickCount;
	while (lucWaitforDelay == 1) {

		if (get_timego( ui32LastTickCount) > 200) {

			lucWaitforDelay = 0;
			//keysPoll(&ui8ButtonChanged, &initPressedState);

		}

	}

	uint8_t initPressedState;

	while (1) {

		// Each time the timer tick occurs, process any button events.
		if (g_ui32TickCount != ui32LastTickCount) {
#ifndef DISABLE_WATCHDOG
			doWatchdogReset();
#endif
			// Remember last tick count
			ui32LastTickCount = g_ui32TickCount;

			// Read the debounced state of the buttons.
			keysPoll(&ui8ButtonChanged, &initPressedState);
			if (gKeysStatus.bits.Keys3_3secOpStCl_pressed) {
				//gKeysStatus.bits.Keys3_3secOpStCl_pressed = 0;
				//gUserModuleState = START_INSTALLATION;
				break;
			}

			if (ui32LastTickCount > SYSTICK_3SEC_KEYPRESS)
				break;

			if ((INSTALLATION_KEYS & initPressedState) == INSTALLATION_KEYS)
				continue;
			else
				break;

		}
#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif
	}


#ifdef TEST_RTC
	//testRTCSet(); //Set only once
	//testRTCGet();
#endif

#ifdef TEST_SDCARD
	gstEMtoLM.commandRequestStatus = eACTIVE;
	gstEMtoLM.commandResponseStatus = eNO_STATUS;
	gstEMtoLM.errorToLM.anomalyCode = 0x010A;
	gstEMtoLM.errorToLM.anomalyLED_blinkRate = 0x0B;
	uint8_t teststr[15] = "TestAnomaly1";
	ustrncpy(gstEMtoLM.errorToLM.errorDetails, teststr, ustrlen((const char*)teststr));
	gstEMtoLM.errorToLM.monitorLED_blinkRate = 0x1B;
	gstEMtoLM.errorToLM.operationCount = 0x00010F47;
	gstEMtoLM.errorToLM.timeStamp = 0x0A0B0C0D;
	writeAnomalyHistory();

	gstEMtoLM.commandRequestStatus = eACTIVE;
	gstEMtoLM.commandResponseStatus = eNO_STATUS;
	gstEMtoLM.errorToLM.anomalyCode = 0x000E;
	gstEMtoLM.errorToLM.anomalyLED_blinkRate = 0x0B;
	ustrncpy(teststr, "TestAnomaly2\0", 13);
	ustrncpy(gstEMtoLM.errorToLM.errorDetails, teststr, ustrlen((const char*)teststr));
	gstEMtoLM.errorToLM.monitorLED_blinkRate = 0x1B;
	gstEMtoLM.errorToLM.operationCount = 0x00010F47;
	gstEMtoLM.errorToLM.timeStamp = 0x000D1C0D;
	writeAnomalyHistory();

	gstEMtoLM.commandRequestStatus = eACTIVE;
	gstEMtoLM.commandResponseStatus = eNO_STATUS;
	gstEMtoLM.errorToLM.anomalyCode = 0x002E;
	gstEMtoLM.errorToLM.anomalyLED_blinkRate = 0x0B;
	ustrncpy(teststr, "TestAnomaly3\0", 13);
	ustrncpy(gstEMtoLM.errorToLM.errorDetails, teststr, ustrlen((const char*)teststr));
	gstEMtoLM.errorToLM.monitorLED_blinkRate = 0x1B;
	gstEMtoLM.errorToLM.operationCount = 0x00010F47;
	gstEMtoLM.errorToLM.timeStamp = 0x000F670E;
	writeAnomalyHistory();

	/*
	 gstUMtoLM_read.commandRequestStatus = eACTIVE;
	 gstUMtoLM_read.commandResponseStatus = eNO_STATUS;
	 gstUMtoLM_read.commandToLMread.bits.readAnomalyHistory = 1;
	 gstUMtoLM_read.historyOrAnomalyIndex = 1;
	 readAnomalyHistory();
	 //	showLogs(ANOMHIST_START_IDX);
	 */
//
	gstUMtoLM_read.commandRequestStatus = eACTIVE;
	gstUMtoLM_read.commandResponseStatus = eNO_STATUS;
	gstUMtoLM_read.commandToLMread.bits.readAnomalyHistory = 1;
	gstUMtoLM_read.historyOrAnomalyIndex = 2;
	readAnomalyHistory();
//	showLogs(ANOMHIST_START_IDX);
//	int res = dumpLogsToSDCard(ANOMHIST_START_IDX);

	gstUMtoLM_write.commandRequestStatus = eACTIVE;
	gstUMtoLM_write.commandResponseStatus = eNO_STATUS;
	gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory = 1;
	gstUMtoLM_write.changeSetting_index = 1;//Not to be used
	gstUMtoLM_write.changeSetting.newValue = 45;
	gstUMtoLM_write.changeSetting.oldValue = 78;
	gstUMtoLM_write.changeSetting.parameterNumber = 107;
	gstUMtoLM_write.changeSetting.timeStamp = 12345;
	writeChangeSettingsHistory();

	gstUMtoLM_write.commandRequestStatus = eACTIVE;
	gstUMtoLM_write.commandResponseStatus = eNO_STATUS;
	gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory = 1;
	gstUMtoLM_write.changeSetting_index = 1;//Not to be used
	gstUMtoLM_write.changeSetting.newValue = 0x08;
	gstUMtoLM_write.changeSetting.oldValue = 0x05;
	gstUMtoLM_write.changeSetting.parameterNumber = 0xA1;
	gstUMtoLM_write.changeSetting.timeStamp = 0x01020304;
	writeChangeSettingsHistory();

	gstUMtoLM_write.commandRequestStatus = eACTIVE;
	gstUMtoLM_write.commandResponseStatus = eNO_STATUS;
	gstUMtoLM_write.commandToLMwrite.bits.changeSettingHistory = 1;
	gstUMtoLM_write.changeSetting_index = 1;//Not to be used
	gstUMtoLM_write.changeSetting.newValue = 0x0B;
	gstUMtoLM_write.changeSetting.oldValue = 0x01;
	gstUMtoLM_write.changeSetting.parameterNumber = 0xB1;
	gstUMtoLM_write.changeSetting.timeStamp = 0x0A0B0C0D;
	writeChangeSettingsHistory();
	*/
//
	gstUMtoLM_read.commandRequestStatus = eACTIVE;
	gstUMtoLM_read.commandResponseStatus = eNO_STATUS;
	gstUMtoLM_read.commandToLMread.bits.readChangeSettingsHistory = 1;
	gstUMtoLM_read.historyOrAnomalyIndex = 1;
	readChangeSettingsHistory();
	showLogs(CHGSETHIST_START_IDX);
//

//
	gstUMtoLM_read.commandRequestStatus = eACTIVE;
	gstUMtoLM_read.commandResponseStatus = eNO_STATUS;
	gstUMtoLM_read.commandToLMread.bits.readChangeSettingsHistory = 1;
	gstUMtoLM_read.historyOrAnomalyIndex = 3;
	readChangeSettingsHistory();
	showLogs(CHGSETHIST_START_IDX);
	*/
#endif

#ifdef TEST_PARAMDB1
	uint32_t readvaltmp1, p6temp;
	readParameterFromDB(A452DISP_FWVER, (uint8_t*)&readvaltmp1);

	p6temp = 0x23C67EF9;
	writeParameterUpdateInDB(A452DISP_FWVER, (uint8_t*)&p6temp);

	while(1);
//	uint8_t t1;
//	uint16_t t2;
//	uint32_t t3;

//	t1 = gui8_Param1;
//	t2 = gui16_Param2;
//	t3 = gui32_Param3;

//	uint32_t readvaltmp1, p6temp;
//	readParameterFromDB(PARAM3, (uint8_t*)&readvaltmp1);
//
//	p6temp = 0x23C67EF9;
//	writeParameterUpdateInDB(PARAM3, (uint8_t*)&p6temp);
//
//	readParameterFromDB(PARAM3, (uint8_t*)&readvaltmp1);
//	if(readvaltmp1 == 0x23C67EF9)
//		readvaltmp1 = 0;
//	else
//		readvaltmp1 = 5;

//	uint8_t readvaltmp2[2];
//	uint16_t *valtmp2 = (uint16_t*)readvaltmp2;
//	readParameterFromDB(PARAM2, readvaltmp2);
////	uint16_t *valtmp2 = (uint16_t*)readvaltmp2;
//	if(*valtmp2 == 0xD41B)
//		*valtmp2 = 0x1234;
//	else
//		*valtmp2 = 0x5678;

//	uint16_t readvaltmp3;
//	readParameterFromDB(PARAM4, (uint8_t*)&readvaltmp3);
//	if(readvaltmp3 == 0x123B)
//		readvaltmp3 = 0;
//	else
//		readvaltmp3 = 5;
//
//	uint8_t iTemp, z=1;
//	for(iTemp = 0; iTemp < 10; iTemp++)
//		z = z+1;

	/*
	 //	writeParameterUpdateInDB(PARAM6,&p5temp);
	 p5temp = 0x34;
	 writeParameterUpdateInDB(PARAM7,&p5temp);
	 p5temp = 0x56;
	 writeParameterUpdateInDB(PARAM8,&p5temp);
	 p5temp = 0x78;
	 writeParameterUpdateInDB(PARAM9,&p5temp);
	 p5temp = 0x90;
	 writeParameterUpdateInDB(PARAM10,&p5temp);

	 uint8_t idx = computeIndex(gui8_AnomalyHistory_ParamIdx);

	 */
//	readvaltmp3 = 0x123B;
//	writeParameterUpdateInDB(PARAM4, (uint8_t*)&readvaltmp3);
//
//	readParameterFromDB(PARAM4, (uint8_t*)&readvaltmp3);
//	if(readvaltmp3 == 0x123B)
//		readvaltmp3 = 0;
//	else
//		readvaltmp3 = 5;
//	struct stChangeSettingHistory chgsetread_tmp;
//	readParameterFromDB(PARAM6, (uint8_t*)&chgsetread_tmp);
#endif

	uint32_t testsystick = getTickms();

#ifdef TEST_PARAMDB
	uint32_t readvaltmp1, p6temp;
	readParameterFromDB(A452DISP_FWVER, (uint8_t*)&readvaltmp1);

	p6temp = 0x23C67EF9;
	writeParameterUpdateInDB(A452DISP_FWVER, (uint8_t*)&p6temp);

	readParameterFromDB(A452DISP_FWVER, (uint8_t*)&readvaltmp1);
	if(readvaltmp1 == 0x23C67EF9)
	readvaltmp1 = 0;
	else
	readvaltmp1 = 5;
#endif

#ifdef TEST_EEPROM
	uint32_t pui32Data[2];
	uint32_t pui32Read[2];
	//
	// Program some data into the EEPROM at address 0x400.
	//
	pui32Data[0] = 0x12345678;
	pui32Data[1] = 0x56789abc;
	EEPROMProgram(pui32Data, 0x400, sizeof(pui32Data));
	//
	// Read it back.
	//
	EEPROMRead(pui32Read, 0x400, sizeof(pui32Read));
#endif

#ifndef DEBUG_DISABLED
	ui32HeartBeatCount = g_ui32TickCount;
#endif
//    UARTprintf("\n************* START ***************\n");
	logWatchDogTimerError();
	while (1) {

#ifndef DEBUG_DISABLED
		if (get_timego( ui32HeartBeatCount) >= HEART_BEAT_DURATION) {
			ui32HeartBeatCount = g_ui32TickCount;

			//uartSendTxBuffer(UART_debug,"H",1);
		}
#endif

		/*		if( g_ui32TickCount == 500 + ui32TickCount5Seconds)
		 {
		 #ifndef DISABLE_WATCHDOG
		 doWatchdogReset();
		 #endif
		 ui32TickCount5Seconds = g_ui32TickCount;

		 runTimeFunction();
		 }*/

		// Each time the timer tick occurs, process any button events.
		//
		// Check whether control board firmware upgrade is required
		//
		if (1 == gstControlBoardStatus.bits.controlBoardBootLoader) {
			psActiveFunctionalBlock = &gsDisplayUpgradeFirmwareFunctionalBlock;
		}

		if (ui64LastTickCount != gui64_TickCount1ms) {

			ui64LastTickCount = gui64_TickCount1ms;

#ifdef DISP_TARGET_BOARD
			keysPoll(&ui8ButtonChanged, 0);
#endif

		}

		if (g_ui32TickCount != ui32LastTickCount) {
#ifndef DISABLE_WATCHDOG
			doWatchdogReset();
#endif
			// Remember last tick count
			ui32LastTickCount = g_ui32TickCount;

#ifdef DISP_LAUNCHPAD_BOARD
			keysPoll_F(&ui8ButtonChanged, 0);
			keysPoll_D(&ui8ButtonChanged, 0);
			keysPoll_E(&ui8ButtonChanged, 0);
#endif

			/*
			 #ifdef DISP_TARGET_BOARD
			 keysPoll(&ui8ButtonChanged, 0);
			 #endif
			 */

			userInterfaceModule();

#ifndef DISABLE_WATCHDOG
			doWatchdogReset();
#endif

#ifdef TEST_DEBOUNCE
			testKeyDebounceModule();
#endif

			//
			// Handle menu key press events
			//
			//userInterfaceModule();

			//communicationModule();
		}

//      communicationModule();
		Set_lcdlightOFF();
		Out_of_settingmode_cyw();
		communicationModuleControlBoard();
		if((menu_gesture_flag_cyw == 0)&&(menu_gesture_flag_A007==0))//菜单使能了手势cyw add    //201806_Bug_No.10
	     {//cyw add
				            shoushi_cyw++;//cyw add
				            if(shoushi_cyw>=5)//=6约2.4ms//cyw add
				            {//cyw add
				                GP2A054A_calculation_cyw();//cyw add
				                shoushi_cyw = 0;//cyw add
				                // ROM_GPIOPinWrite(GPIO_PORTE_BASE, PORTE_AUTOMANUALLEDPIN, PORTE_AUTOMANUALLEDPIN);
				                // ROM_GPIOPinWrite(GPIO_PORTE_BASE, PORTE_AUTOMANUALLEDPIN, 0);
				            }//cyw add
	     }//cyw add
#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif

		displayBoardLEDHandler();

#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif

#ifndef ENABLE_ERROR_DISPLAY
		errorModule();//	Commented for client's testing in China without drive board on 08 Dec to hide CT-DR comm error
#endif

#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif

		if (guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART_control]]
				== 1) {
			gu16_commbuffof_di2ct++;
			writeParameterUpdateInDB(A804COMMBUFFOF_DI2CT,
					(uint8_t*) &gu16_commbuffof_di2ct);
			guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART_control]] =
					0;
		}

		if (guchCommunicationCRC_ErrorOccurrence[guchUARTMapToGlobalBuffer[UART_control]]
				== 1) {
			gu16_crcfail_di2ct++;
			writeParameterUpdateInDB(A800CRCFAIL_DI2CT,
					(uint8_t*) &gu16_crcfail_di2ct);
			guchCommunicationCRC_ErrorOccurrence[guchUARTMapToGlobalBuffer[UART_control]] =
					0;
		}
//        testLEDHandler();

//        if(psActiveFunctionalBlock == &gsInstallationFunctionalBlock)
//        {
//        	installationTestFunction();
//        	installationRuntimeTestFunction();
//        }
	}
}

/********************************************************************************/

void initDisplayBoard(void) {
	/*
	 ROM_FPULazyStackingEnable();
	 ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
	 SYSCTL_XTAL_16MHZ);

	 initSysTick();
	 //	initKeys();
	 //	initRTC();
	 //    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
	 //    EEPROMInit();
	 //    bolyminDisplayInit();
	 */

	ROM_FPULazyStackingEnable();
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
	SYSCTL_XTAL_16MHZ);
	 init054A();//add cyw
    GP2AP054A_Init_cyw_debug();//add cyw
//---------- LEDs -----------
	//Turn Power LED ON - Added 5th Jul 2014
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, PORTF_POWERLEDPIN);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, PORTF_POWERLEDPIN, 0);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, PORTF_POWERLEDPIN, PORTF_POWERLEDPIN);

	//Configure AutoMan LED
	MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, PORTE_AUTOMANUALLEDPIN);
	ROM_GPIOPinWrite(GPIO_PORTE_BASE, PORTE_AUTOMANUALLEDPIN, 0); //Default OFF

	// Configure fault LED
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, PORTE_FAULTLEDPIN);
	ROM_GPIOPinWrite(GPIO_PORTE_BASE, PORTE_FAULTLEDPIN, 0); //Default OFF
//------------------------------



	initSysTick();
	initKeys();
	initRTC();
#ifndef DISPLAY_BOARD_CONNECTED
	bolyminDisplayInit();
#endif

	SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
	EEPROMInit();
	initSdCard();
	initDispBoardGlobalRegisters();

	//
	// Enable processor interrupts.
	//
	ROM_IntMasterEnable();

	/*	uint8_t iTmp;
	 uint8_t tempwriteeep[108];
	 for(iTmp = 0; iTmp < 27; iTmp++)
	 {
	 int randno = urand();
	 tempwriteeep[iTmp*4] = (randno & 0x000000FF);
	 tempwriteeep[iTmp*4 + 1] = (randno & 0x0000FF00) >> 8;
	 tempwriteeep[iTmp*4 + 2] = (randno & 0x00FF0000) >> 16;
	 tempwriteeep[iTmp*4 + 3] = (randno & 0xFF000000) >> 24;
	 }
	 EEPROMProgramByte(tempwriteeep, 0, 108);
	 EEPROMProgramByte(tempwriteeep, 108, 108);
	 EEPROMProgramByte(tempwriteeep, 216, 108);
	 EEPROMProgramByte(tempwriteeep, 324, 108);
	 EEPROMProgramByte(tempwriteeep, 432, 64);

	 for(iTmp = 0; iTmp < _NO_OF_PARAMS; iTmp++)
	 {
	 initParameterDB((PARAM_DISP)iTmp);
	 }*/

	uint8_t iTmp;
	for (iTmp = 0; iTmp < _NO_OF_PARAMS; iTmp++) {
		initParameterDB((PARAM_DISP) iTmp);
	}

	//EEPROM_Gesture_Load_cyw();//add cyw

	initLogger();

	//Turn ON Power LED after all INIT complete
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, PORTF_POWERLEDPIN, PORTF_POWERLEDPIN);
#ifndef DISABLE_WATCHDOG
	initWatchdog();
#endif
}

/******************************************************************************
 * FunctionName: userInterfaceModule
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
void userInterfaceModule(void) {
	//
	// Call functional active block's runtime function
	//
	psActiveFunctionalBlock->pfnRunTimeOperation();

	//
	// Call active functional block's Up functionality
	//
	psActiveFunctionalBlock->pfnUp();

	//
	// Call active functional block's Down functionality
	//
	psActiveFunctionalBlock->pfnDown();

	//
	// Call active functional block's Mode functionality
	//
	psActiveFunctionalBlock->pfnMode();

	//
	// Call active functional block's Enter functionality
	//
	psActiveFunctionalBlock->pfnEnter();
}

/******************************************************************************
 * FunctionName: updateFaultLEDStatus
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
void updateFaultLEDStatus(void) {
	//
	// Update fault LED status
	//
	if (((1 == gstDriveBoardStatus.bits.driveFault)
			|| (1 == gstDriveBoardStatus.bits.driveFaultUnrecoverable)
			|| (1 == gstControlBoardStatus.bits.controlFault)
			|| (1 == gstControlBoardStatus.bits.controlFaultUnrecoverable)
			|| (1 == gstDisplayBoardStatus.bits.displayFault)
			|| (1 == gstDisplayBoardStatus.bits.displayFaultUnrecoverable)) //&& ( 0xFF != gstEMtoUM.faultLEDstatus )
	) {
		gstLEDcontrolRegister.faultLED = gstEMtoUM.faultLEDstatus;
	} else if (((1 != gstDriveBoardStatus.bits.driveFault)
			&& (1 != gstDriveBoardStatus.bits.driveFaultUnrecoverable)
			&& (1 != gstControlBoardStatus.bits.controlFault)
			&& (1 != gstControlBoardStatus.bits.controlFaultUnrecoverable)
			&& (1 != gstDisplayBoardStatus.bits.displayFault)
			&& (1 != gstDisplayBoardStatus.bits.displayFaultUnrecoverable)) //|| ( 0xFF == gstEMtoUM.faultLEDstatus )
	) {
		gstLEDcontrolRegister.faultLED = 0;
	}
}

void runTimeFunction(void) {
	dateTimeRunTime();
}

uint8_t defaultFunction() {
	return 0;
}

void logWatchDogTimerError(void) {
	uint32_t lui32ProcessorResetCause = 0;
                                                                 //20170421  201703_No.39
//	lui32ProcessorResetCause = ROM_SysCtlResetCauseGet();
//
//	if (lui32ProcessorResetCause & SYSCTL_CAUSE_SW) {
//		gstDisplayProcessorFault.bits.watchdog = 1;
//		gstDisplayBoardFault.bits.displayProcessor = 1;
//		gstDisplayBoardStatus.bits.displayFault = 1;
//		ROM_SysCtlResetCauseClear(lui32ProcessorResetCause);
//	} else {
//		gstDisplayProcessorFault.bits.watchdog = 0;
//		if (gstDisplayProcessorFault.val == 0) {
//			gstDisplayBoardFault.bits.displayProcessor = 0;
//		}
//		if (gstDisplayBoardFault.val == 0) {
//			gstDisplayBoardStatus.bits.displayFault = 0;
//		}
//	}
}

