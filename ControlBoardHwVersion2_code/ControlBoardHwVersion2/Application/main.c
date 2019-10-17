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
 *  	0.1D	27/03/2014      	iGATE Offshore team       Initial Creation
 ****************************************************************************/



/****************************************************************************
 *  Include:
 ****************************************************************************/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "Drivers/watchdogtimer.h"
#include "Middleware/system.h"

#include "Middleware/serial.h"
#include "Middleware/eeprom.h"
#include "Middleware/rtc.h"
#include "Middleware/debounce.h"
#include "Drivers/systicktimer.h"
#include "cmdr.h"
#include "cmdi.h"
#include "logicsolver.h"
#include "errormodule.h"
#include "intertaskcommunication.h"
#include "monitorledhandler.h"
//#include <utils/ustdlib.h>
#include <stdbool.h>
#include <inc/hw_ints.h>
#include <inc/hw_memmap.h>
//#include "inc/hw_types.h"
#include <driverlib/debug.h>
#include <driverlib/fpu.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/sysctl.h>
#include <driverlib/uart.h>
#include <driverlib/rom.h>

#include <driverlib/eeprom.h>

//#include "Application/logicsolver.h"
#include "Middleware/sensorsdebounce.h"

#ifndef TEST_DBHANDLER_CMDi
#include "Middleware/paramdatabase.h"
#include "Application/dbhandler.h"
#endif

#ifdef TEST_UART
#include "TestModule/test_serial.h"
#endif

#ifdef TEST_EEPROM
#include "TestModule/test_eeprom.h"
#endif

#ifdef TEST_RTC
#include "TestModule/testrtc.h"
#endif

#ifdef TEST_DEBOUNCE
#include "TestModule/testdebounce.h"
#endif

#ifdef TEST_CMDr
#include "driverlib/sw_crc.h"
#endif

/****************************************************************************/


/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/
#define TEST_PARAMDB
#define TEST_DBHANDLER
#define HEART_BEAT_DURATION		200		//	2 Sec using 10mS tick counter
/****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/
extern uint8_t guchRxBufferByteCount [UART_CHANNEL_COUNT];					// Number of bytes received
extern uint8_t guchRxBufferRdIndex [UART_CHANNEL_COUNT];						// Index to the byte currently being read
extern uint8_t guchRxBufferWrIndex [UART_CHANNEL_COUNT];						// Index to the byte position where received
extern uint8_t guchUARTMapToGlobalBuffer[MAX_UART_AVAILABLE];
extern uint8_t guchRxBufferOverflowState[UART_CHANNEL_COUNT];					// Receive buffer overflow state

// Control to drive communication error occurrence count
extern uint8_t guchCMDrCRC_ErrorOccurrence[UART_CHANNEL_COUNT];

// Display to Control communication error occurrence count
extern uint8_t guchCMDiCRC_ErrorOccurrence[UART_CHANNEL_COUNT];

extern _CMDrInnerTaskComm lstCMDrInnerTaskComm;
uint32_t get_timego10ms(uint32_t x_data_his);
uint32_t get_timego(uint32_t x_data_his);
/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
 ****************************************************************************/
void counterHandler(void);
void logWatchDogTimerError(void);


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

void main(void)
{
	uint8_t ui8ButtonChanged = 0; 			//Needed for key Debounce module
	uint8_t ui8SensorChanged = 0; 			//Needed for sensor Debounce module
	uint32_t ui32LastTickCount = 0;			//Needed for Debounce module
#ifndef DEBUG_DISABLED
	uint32_t ui32HeartBeatCount = 0;
#endif
	//uint32_t tp_i;
//	uint8_t lucTemp = 0;
#if 0
	uint8_t lucReturnValue;
	uint8_t lucaLocalBuff[40];
	uint8_t lucRxByte, lucByteCount;
	uint8_t lucaTestString[40]	= {"UART0 transmit string\n\r"};

#endif	//	0

#ifndef TEST_DBHANDLER
	uint8_t ui8ButtonState;			//Needed for Debounce module
	uint8_t ui8SensorChanged, ui8ButtonChanged; 		//Needed for Debounce module
	uint32_t ui32LastTickCount = 0;	//Needed for Debounce module
#endif	//	TEST_DBHANDLER_CMDi


#ifdef TEST_UART
	uint8_t lucPendingCharacters = 0;
#endif

	//	Initialize controller
	systemInit();

	//*************************************************************
	// Set following variable for Hardware version of Control Board
	// Format:
	// First byte = Not used, 		Second Byte = Not used
	// Third byte = Not Used, 		Fourth byte = Version

	gu32_ctrl_hwver = 2;

	//*************************************************************

	initControlBoardGlobalRegisters();
#ifndef DISABLE_WATCHDOG
	doWatchdogReset();
#endif

#ifndef TEST_PARAMDB
	uint32_t readvaltmp1, p6temp;
	readParameterFromDB(A450CTRL_FWVER, (uint8_t*)&readvaltmp1);

	p6temp = 0x12003400;
	writeParameterUpdateInDB(A450CTRL_FWVER, (uint8_t*)&p6temp);

	readParameterFromDB(A450CTRL_FWVER, (uint8_t*)&readvaltmp1);
	if(readvaltmp1 == 0x12003400)
		readvaltmp1 = 0;
	else
		readvaltmp1 = 5;
#endif	//	TEST_PARAMDB

#ifdef TEST_DBHANDLER_CMDi
	union ui32TempData lun32Word;
	extern uint32_t gu32_stat_drive_brdc;
	gu32_stat_drive_brdc = 0x12345678;

	configureUART(UART_debug);
	configureUART(UART_display);

	readParameterFromDB(A000UPPLIM_STPTIME,&lun32Word.byte[0]);
	uartSendTxBuffer(UART_debug,&lun32Word.byte[0],1);

	readParameterFromDB(A007EN_SWITCHPAN,&lun32Word.byte[0]);
	uartSendTxBuffer(UART_debug,&lun32Word.byte[0],1);

	while(1)
	{
		DBHandler_Module();
		communicationModuleDisplay();
	}
#endif	//	TEST_DBHANDLER_CMDi

#ifndef TEST_DBHANDLER

	while(1)
	{
		DBHandler_Module();
		if(g_ui32TickCount != ui32LastTickCount)
		{
			//
			// Remember last tick count
			//
			ui32LastTickCount = g_ui32TickCount;

			//
			// Read the debounced state of the buttons.
			//
			ui8ButtonState = keysPoll(&ui8ButtonChanged, 0);
			testDBHandler();
		}
	}



	/*
	DBHandler_Module();
	gstEMtoDH.commandRequestStatus = eACTIVE;
	gstEMtoDH.commandResponseStatus = eNO_STATUS;
	gstEMtoDH.errorToDH.anomalyCode = 0x010A;
	uint8_t teststr[15] = "TestAnomaly";
	ustrncpy(gstEMtoDH.errorToDH.errorDetails, teststr, ustrlen((const char*)teststr));
//	gstEMtoLM.errorToLM.errorDetails = teststr;
	gstEMtoDH.errorToDH.timeStamp = 0x0A0B0C0D;
	writeAnomalyHistory();

	gstCMDitoDH.commandRequestStatus = eACTIVE;
	gstCMDitoDH.commandResponseStatus = eNO_STATUS;
	gstCMDitoDH.commandDisplayBoardDH.bits.getParameter = 1;
	gstCMDitoDH.CommandDataCMDiDH.parameterNumber = 6;
	handleCMDi();
	gstCMDitoDH.commandDisplayBoardDH.val = 0;

	gstCMDitoDH.commandRequestStatus = eACTIVE;
	gstCMDitoDH.commandResponseStatus = eNO_STATUS;
	gstCMDitoDH.commandDisplayBoardDH.bits.setParameter = 1;
	gstCMDitoDH.CommandDataCMDiDH.parameterNumber = 5;
	gstCMDitoDH.CommandDataCMDiDH.commandData.setParameterValue = 0x34;
	handleCMDi();

	gstCMDitoDH.commandDisplayBoardDH.val = 0;
	gstCMDitoDH.commandRequestStatus = eACTIVE;
	gstCMDitoDH.commandResponseStatus = eNO_STATUS;
	gstCMDitoDH.commandDisplayBoardDH.bits.setTimeStamp = 1;
	gstCMDitoDH.CommandDataCMDiDH.commandData.timeStamp = 0x001E4568;
	handleCMDi();

	gstCMDitoDH.commandDisplayBoardDH.val = 0;
	gstCMDitoDH.commandRequestStatus = eACTIVE;
	gstCMDitoDH.commandResponseStatus = eNO_STATUS;
	gstCMDitoDH.commandResponseACK_Status = eNO_StatusAcknowledgement;
	gstCMDitoDH.commandDisplayBoardDH.bits.getErrorList = 1;
	handleCMDi();
	handleCMDi();
	gstCMDitoDH.commandResponseACK_Status = eResponseeAcknowledgement_ACK;
	handleCMDi();
	handleCMDi();

	gstEMtoDH.commandRequestStatus = eACTIVE;
	gstEMtoDH.commandResponseStatus = eNO_STATUS;
	gstEMtoDH.errorToDH.anomalyCode = 0x010A;
	uint8_t teststr1[15] = "Test1Anomaly1";
	ustrncpy(gstEMtoDH.errorToDH.errorDetails, teststr1, ustrlen((const char*)teststr1));
//	gstEMtoLM.errorToLM.errorDetails = teststr;
	gstEMtoDH.errorToDH.timeStamp = 0x01230E0E;
	writeAnomalyHistory();
*/
#endif	//	TEST_DBHANDLER


#ifndef TEST_CMDr
	configureUART(UART_drive);
	initCMDrGlobalRegisters();

	/*
	 * inserted for testing
	*/
#ifndef handleCmdLS_CMDr
	gstLStoCMDr.commandRequestStatus = eACTIVE;
	gstLStoCMDr.commandToDriveBoard.bits.openShutter = 1;
#endif	// handleCmdLS_CMDr

#ifdef handleCmdEM_CMDr
	gstEMtoCMDr.commandRequestStatus = eACTIVE;
	gstEMtoCMDr.commandToDriveBoard.bits.getError = 1;
#endif	// handleCmdEM_CMDr

#ifdef handleCmdCMDi_CMDr
	gstCMDitoCMDr.commandRequestStatus = eACTIVE;

	gstCMDitoCMDr.transmitCommandPacket[0] = 0x00;
	gstCMDitoCMDr.transmitCommandPacket[1] = 0x02;
	gstCMDitoCMDr.transmitCommandPacket[2] = 0x05;
	gstCMDitoCMDr.transmitCommandPacket[3] = 0x14;
	gstCMDitoCMDr.transmitCommandPacket[4] = 0x12;
	gstCMDitoCMDr.transmitCommandPacket[5] = 0xFC;
	gstCMDitoCMDr.transmitCommandPacket[6] = 0xE9;
	gstCMDitoCMDr.transmitCommandPacket[7] = 0x35;

	gstCMDitoCMDr.transmitCommandPacketLen = 8;
#endif	// handleCmdCMdi_CMDr

#ifdef handleDrvCmdRes
	lstCMDrInnerTaskComm.commandRequestStatus = eACTIVE;
	lstCMDrInnerTaskComm.commandResponseACK_Status = eNO_StatusAcknowledgement;

	lstCMDrInnerTaskComm.commandType = DIRECT;
#ifdef direct
	lstCMDrInnerTaskComm.uchTxBuffer[0] = 0x01;
	lstCMDrInnerTaskComm.uchTxBuffer[1] = 0x00;
	lstCMDrInnerTaskComm.uchTxBuffer[2] = 0x04;
	lstCMDrInnerTaskComm.uchTxBuffer[3] = 0x02;
	lstCMDrInnerTaskComm.uchTxBuffer[4] = 0x41;
	lstCMDrInnerTaskComm.uchTxBuffer[5] = 0x00;

	lstCMDrInnerTaskComm.uchTxBuffer[6] = 0x00;
	lstCMDrInnerTaskComm.uchTxBuffer[7] = 0x00;
	lstCMDrInnerTaskComm.uchTxBuffer[8] = 0x00;

	lucTempCRC = Crc16(0,lstCMDrInnerTaskComm.uchTxBuffer,lstCMDrInnerTaskComm.uchTxBuffer [2]+1);
	lstCMDrInnerTaskComm.uchTxBuffer[5] = (lucTempCRC & 0xFF00) >> 8;
	lstCMDrInnerTaskComm.uchTxBuffer[6] = lucTempCRC & 0xFF;

	lstCMDrInnerTaskComm.uchTxBufferLen = 7;
#endif	// direct
	lstCMDrInnerTaskComm.commandToDriveBoard.bits.openShutter = 1;
	lstCMDrInnerTaskComm.additionalCommandData = 0x90;
	lstCMDrInnerTaskComm.parameterNumber = 130;
	lstCMDrInnerTaskComm.parameterValue = 0x12131418;
#endif	// handleDrvCmdRes
	/*
	 * remove above code after testing
	*/
//	ui32TxInstance = GetTickms();
	while(1)
	{
//		ui32TxTimeElapsed += GetTickms();
//		ui32TxTimeOut = ui32TxTimeElapsed - ui32TxInstance;
		communicationModuleDrive();
		communicationModuleDisplayToTestCMDr();
		logicSolverToTestCMDrCMDi();
		errorModuleToTestCMDr();
	}
#endif	// TEST_CMDr

#ifdef TEST_CMDi
	configureUART(UART_display);
	configureUART(UART_drive);
	initCMDrGlobalRegisters();

//	uartSendTxBuffer(UART_display,"This is display board",21);
//	while(1)
//	{
//		uartSendTxBuffer(UART_drive,"This is drive board",19);
//
//		uartCheckFreeTxBuffer(UART_drive,&lucTemp);
//		while(lucTemp)
//		{
//			uartCheckFreeTxBuffer(UART_drive,&lucTemp);
//		}
//	}
	while(1)
	{
		communicationModuleDisplay();
		logicSolverToTestCMDrCMDi();
		communicationModuleDrive();
		dataHandlerToTestCMDi();
		errorModuleToTestCMDi();
	}
#endif	//	TEST_CMDi

#ifdef TEST_CMDi_CMDr_with_DISPLAY_BOARD

	configureUART(UART_drive);
	configureUART(UART_display);
	configureUART(UART_debug);
	initCMDrGlobalRegisters();

	while(1)
	{
		communicationModuleDisplay();
		logicSolverToTestCMDrCMDi();
		communicationModuleDrive();
		dataHandlerToTestCMDi();
		errorModuleToTestCMDi();
	}
#endif	//	TEST_CMDi_CMDr_with_DISPLAY_BOARD

#ifndef TEST_DBHANDLER_CMDi_CMDr_with_DISPLAY_BOARD
//	union ui32TempData lun32Word;
/*	extern uint32_t gu32_stat_drive_brdc;
	gu32_stat_drive_brdc = 0x12345678;*/
#ifndef DEBUG_DISABLED
	configureUART(UART_debug);
#endif
	configureUART(UART_display);
	// Clear global register for CRC error occurrence count for display UART port
	guchCMDiCRC_ErrorOccurrence[guchUARTMapToGlobalBuffer[UART_display]] = 0;

	configureUART(UART_drive);
	// Clear global register for CRC error occurrence count for drive UART port
	guchCMDiCRC_ErrorOccurrence[guchUARTMapToGlobalBuffer[UART_drive]] = 0;

	//	Log watchdog error
	logWatchDogTimerError();

#ifndef DISABLE_WATCHDOG
	doWatchdogReset();
#endif

	initCMDrGlobalRegisters();

//	ROM_GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, 0); //ON

	uint8_t initPressedState = 0;
	//	Added on 3 Nov 2014 to scan interlock input state at power-on, so that if
	//	interlock input is active at power on it can be serviced
	uint8_t initSensorState = 0;

#ifdef WATCHDOG_TEST

#ifndef DEBUG_DISABLED
	ui32HeartBeatCount = g_ui32TickCount10ms;
#endif
	while(1)
	{
#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif
		DBHandler_Module();
		communicationModuleDisplay();
		communicationModuleDrive();
		errorModule();
		logicSolver();
		counterHandler();

#ifndef DEBUG_DISABLED
		if(g_ui32TickCount10ms - ui32HeartBeatCount >= HEART_BEAT_DURATION)
		{
			ui32HeartBeatCount = g_ui32TickCount10ms;

			uartSendTxBuffer(UART_debug,"H",1);
		}
#endif	// DEBUG_DISABLED
	}
#endif	// WATCHDOG_TEST

	while(1) {
		// Each time the timer tick occurs, process any button events.
		if(g_ui32TickCount != ui32LastTickCount)
		{
#ifndef DISABLE_WATCHDOG
			doWatchdogReset();
#endif
			// Remember last tick count
			ui32LastTickCount = g_ui32TickCount;

			// Read the debounced state of the buttons.
			keysPoll(&ui8ButtonChanged, &initPressedState);
			if(gKeysStatus.bits.Keys3_3secOpStCl_pressed) {
//				gKeysStatus.bits.Keys3_3secOpStCl_pressed = 0;
				//UARTprintf("Open + Stop + Close + 3sec INSTALLATION\n");
//				uartSendTxBuffer(UART0,"OPEN +CLOSE +STOP 3sec\n\r",24);

				//Call Installation sequence here
//				eLogic_Solver_State = Logic_Solver_Drive_Instalation;
				break;
			}

			if(ui32LastTickCount > SYSTICK_3SEC_KEYPRESS)
				break;

			if((INSTALLATION_KEYS & initPressedState) == INSTALLATION_KEYS)
				continue;
			else
				break;

		}
#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif
	}

	/*readParameterFromDB(A000UPPLIM_STPTIME,&lun32Word.byte[0]);
	uartSendTxBuffer(UART_debug,&lun32Word.byte[0],1);

	readParameterFromDB(A007EN_SWITCHPAN,&lun32Word.byte[0]);
	uartSendTxBuffer(UART_debug,&lun32Word.byte[0],1);*/

#ifndef DEBUG_DISABLED
	ui32HeartBeatCount = g_ui32TickCount10ms;
#endif




	while(1)
	{
#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif

#ifndef DEBUG_DISABLED
		if(get_timego10ms(ui32HeartBeatCount) >= HEART_BEAT_DURATION)
		{
			ui32HeartBeatCount = g_ui32TickCount10ms;

//			uartSendTxBuffer(UART_debug,"H",1);
		}
#endif

		// Call database handler
		DBHandler_Module();

#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif
		// Call communication handler to handle query-response between display and control boards
		communicationModuleDisplay();

#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif
		// Call functional logic decision making routine
		logicSolver();
//		logicSolverToTestCMDrCMDi();

#ifdef Enable_WATCHDOG_CHECK
		if(GPIOPinRead(WATCHDOG_CHECK_GPIO_BASE, WATCHDOG_CHECK_PE)==0)
			while(1);
#endif

#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif
		// Call communication handler to handle query-response between control and drive boards
		communicationModuleDrive();

#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif

		errorModule();

#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif

		monitorLEDHandler();

#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif
		counterHandler();


		wirelesslogin_cyw();
		// Scan for change of state on operational buttons and sensors, every 10mS
		// if(g_ui32TickCount10ms != ui32LastTickCount)

		// Scan for change of state on operational buttons and sensors, every 1mS
		if(g_ui32TickCount != ui32LastTickCount)
		{
			//
			// Remember last tick count
			//
			//ui32LastTickCount = g_ui32TickCount10ms;
			ui32LastTickCount = g_ui32TickCount;

			//
			// Read the debounced state of the buttons.
			//
			keysPoll(&ui8ButtonChanged, 0);
#ifndef DISABLE_WATCHDOG
			doWatchdogReset();
#endif
			sensorsPoll(&ui8SensorChanged, &initSensorState);
		}

#ifndef DISABLE_WATCHDOG
		doWatchdogReset();
#endif

		// Check if overflow flag or CRC error flag is set on display or drive port and log the occurrence
		if(guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART_display]] == 1)
		{
			gu16_commbuffof_ct2di++;
			writeParameterUpdateInDB(A805COMMBUFFOF_CT2DI, (uint8_t*)&gu16_commbuffof_ct2di);
			guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART_display]] = 0;
		}
		if(guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART_drive]] == 1)
		{
			gu16_commbuffof_ct2dr++;
			writeParameterUpdateInDB(A806COMMBUFFOF_CT2DR, (uint8_t*)&gu16_commbuffof_ct2dr);
			guchRxBufferOverflowState[guchUARTMapToGlobalBuffer[UART_drive]] = 0;
		}
		if(guchCMDrCRC_ErrorOccurrence[guchUARTMapToGlobalBuffer[UART_drive]] == 1)
		{
			gu16_crcfail_ct2dr++;
			writeParameterUpdateInDB(A802CRCFAIL_CT2DR, (uint8_t*)&gu16_crcfail_ct2dr);
			guchCMDrCRC_ErrorOccurrence[guchUARTMapToGlobalBuffer[UART_drive]] = 0;
		}
		if(guchCMDiCRC_ErrorOccurrence[guchUARTMapToGlobalBuffer[UART_display]] == 1)
		{
			gu16_crcfail_ct2dr++;
			writeParameterUpdateInDB(A801CRCFAIL_CT2DI, (uint8_t*)&gu16_crcfail_ct2di);
			guchCMDrCRC_ErrorOccurrence[guchUARTMapToGlobalBuffer[UART_display]] = 0;
		}
	}	// main while ends here
#endif	//	TEST_DBHANDLER_CMDi_CMDr_with_DISPLAY_BOARD




#ifdef TEST_UART			// UART test code begins here
//	lucReturnValue = testConfigrueUART(UART1);
//	lucReturnValue = testUartCheckFreeTxBuffer(UART0, &lucPendingCharacters);
	lucReturnValue = testUartSendTxBuffer(UART0);
//	lucReturnValue = testUartCheckRxBufferOverflowState(UART0);
//	lucReturnValue = testUartCheckNotFreeRxBuffer(7);
	lucReturnValue = testUartGetRxBuffer(UART0, 8);
	while(1);
#endif				//UART test code ends here

#ifdef	TEST_EEPROM			//	EEPROM test code begins here
	testEEPROMWrite_OneByte();
	testEEPROMWrite_OneByteLast();
	testEEPROMWrite_TwoBytesMiddle();
	testEEPROMWrite_FourWordsComplete();
	while(1);
#endif						//	EEPROM test code ends here

#ifdef	TEST_RTC			//	RTC test code begins here
//	testRTCSet();
	testRTCGet();
	while(1);
#endif						//	RTC test code ends here

#ifdef	TEST_DEBOUNCE			//	DEBOUNCE test code begins here
	while(1)
	{
		if(g_ui32TickCount != ui32LastTickCount)
		{
			//
			// Remember last tick count
			//
			ui32LastTickCount = g_ui32TickCount;

			//
			// Read the debounced state of the buttons.
			//
			keysPoll(&ui8ButtonChanged, 0);
			sensorsPoll(&ui8SensorChanged, 0);
			testKeyDebounceModule();
		}
	}
#endif						//	DEBOUNCE test code ends here

#ifdef APPLICATION		//Application code begins here
	while(1)
	{
#ifdef DISABLE_WATCHDOG
		// Clear the watchdog interrupt to reload Watchdog
		ROM_WatchdogIntClear(WATCHDOG0_BASE);
#endif

		//
		//
		// Main looping code here
		//
		//
	}


	if(lucReturnValue == 1)
	{
		uartSendTxBuffer(UART0,"Invalid UART number specified\n", 31);
	}
	else if(lucReturnValue == 2)
	{
		uartSendTxBuffer(UART0,"Specified UART is disabled\n", 28);
	}
	else if(!lucReturnValue)
	{
		//UARTSend((const uint8_t*)lucaLocalBuff, strlen((const char *)lucaLocalBuff));
	}

	uartSendTxBuffer (UART0, lucaTestString, strlen ((const char*)lucaTestString));

	memset(lucaLocalBuff,'\0',sizeof((const char*)lucaLocalBuff));

	while(1)
	{
		if(guchRxBufferByteCount[guchUARTMapToGlobalBuffer[UART0]])
		{
			uartGetRxBuffer (UART0, &lucRxByte);
			lucaLocalBuff[guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[UART0]]] = lucRxByte;
			uartSendTxBuffer(UART0,&lucaLocalBuff[guchRxBufferRdIndex[guchUARTMapToGlobalBuffer[UART0]]],1);
		}
	}
#endif	//Application code ends here
}

/********************************************************************************/


void counterHandler(void)
{

	static unsigned char lsCounterHandlerState = 0;
	static uint32_t lsulTickCounts = 0;

	#define COUNTER_ON_OFF_TIME		100

	if (lsCounterHandlerState == 0)
	{

		if (gstElectroMechanicalCounter.value != 0)
		{

			lsCounterHandlerState = 1;

		}

	}
	else if (lsCounterHandlerState == 1)  // Turn ON the PIN
	{
//		ROM_GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, 0); //ON
		ROM_GPIOPinWrite(COUNTER_GPIO_BASE, COUNTER_OUT, COUNTER_OUT_HIGH); //ON

		lsCounterHandlerState = 2;

	}
	else if (lsCounterHandlerState == 2 || lsCounterHandlerState == 5)  // Capture time
	{

		lsulTickCounts = g_ui32TickCount;
		lsCounterHandlerState++;

	}
	else if (lsCounterHandlerState == 3 || lsCounterHandlerState == 6)  // Wait for time delay of
	{

		if(get_timego ( lsulTickCounts) >= COUNTER_ON_OFF_TIME )
		{
			lsCounterHandlerState++;
		}

	}
	else if (lsCounterHandlerState == 4)  // Turn OFF the PIN
	{
//		ROM_GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4); //OFF
		ROM_GPIOPinWrite(COUNTER_GPIO_BASE, COUNTER_OUT, COUNTER_OUT_LOW); //ON

		lsCounterHandlerState = 5;

	}
	else if (lsCounterHandlerState == 7)  // Go back to initial state after decrementing the ticks to be given variable
	{

		gstElectroMechanicalCounter.value--;
		lsCounterHandlerState = 0;

	}

} // void counterHandler(void)


void logWatchDogTimerError(void)
{
	uint32_t lui32ProcessorResetCause = 0;

	lui32ProcessorResetCause = ROM_SysCtlResetCauseGet();

	//if(lui32ProcessorResetCause & SYSCTL_CAUSE_SW)
	if(lui32ProcessorResetCause & (SYSCTL_CAUSE_SW || SYSCTL_CAUSE_LDO ||SYSCTL_CAUSE_WDOG1 ||SYSCTL_CAUSE_WDOG0 ||SYSCTL_CAUSE_WDOG ||SYSCTL_CAUSE_BOR ||SYSCTL_CAUSE_EXT))
	{
		gstControlProcessorFault.bits.watchdog = 1;
		gstControlBoardFault.bits.controlProcessor = 1;
		gstControlBoardStatus.bits.controlFault = 1;
		ROM_SysCtlResetCauseClear(lui32ProcessorResetCause);
	}
	else
	{
		gstControlProcessorFault.bits.watchdog = 0;
		if(gstControlProcessorFault.val == 0)
		{
			gstControlBoardFault.bits.controlProcessor = 0;
		}
		if(gstControlBoardFault.val == 0)
		{
			gstControlBoardStatus.bits.controlFault = 0;
		}
	}
}
