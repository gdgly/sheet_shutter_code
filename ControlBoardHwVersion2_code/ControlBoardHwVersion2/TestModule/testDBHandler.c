#include <stdbool.h>
#include <stdint.h>
#include <inc/hw_memmap.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom_map.h>
#include <driverlib/rom.h>
#include <inc/hw_types.h>
#include <driverlib/uart.h>
#include "Middleware/debounce.h"
#include "Middleware/serial.h"

#include "Middleware/paramdatabase.h"
#include "Application/dbhandler.h"
#include "Drivers/ustdlib.h"
#include "Application/intertaskcommunication.h"

void testDBHandler(void)
{
	static uint8_t pressstate = 'A';

	//Open test
    if(gKeysStatus.bits.Key_Open_pressed) {
    	gKeysStatus.bits.Key_Open_pressed = 0;

    	switch (pressstate) {
    	case 'A': {
    		pressstate = 'B';
    		gstEMtoDH.commandRequestStatus = eACTIVE;
    		gstEMtoDH.commandResponseStatus = eNO_STATUS;
    		gstEMtoDH.errorToDH.anomalyCode = 0x010A;
    		uint8_t teststr[15] = "TestAnomaly";
    		ustrncpy((char *)gstEMtoDH.errorToDH.errorDetails, (const char *)teststr, ustrlen((const char*)teststr));
    		gstEMtoDH.errorToDH.timeStamp = 0x0A0B0C0D;
    		break;
    	}
    	case 'B': {
    		pressstate = 'C';
    		gstCMDitoDH.commandDisplayBoardDH.val = 0;
    		gstCMDitoDH.commandRequestStatus = eACTIVE;
    		gstCMDitoDH.commandResponseStatus = eNO_STATUS;
    		gstCMDitoDH.commandDisplayBoardDH.bits.getParameter = 1;
    		gstCMDitoDH.commandDataCMDiDH.parameterNumber = 6;
    		break;
    	}
    	case 'C': {
    		pressstate = 'D';
    		gstCMDitoDH.commandDisplayBoardDH.val = 0;
    		gstCMDitoDH.commandRequestStatus = eACTIVE;
    		gstCMDitoDH.commandResponseStatus = eNO_STATUS;
    		gstCMDitoDH.commandDisplayBoardDH.bits.setParameter = 1;
    		gstCMDitoDH.commandDataCMDiDH.parameterNumber = 0;
    		gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 0x34;
    		break;
    	}
    	case 'D': {
    		pressstate = 'E';
//    		gstCMDitoDH.commandDisplayBoardDH.val = 0;
//    		gstCMDitoDH.commandRequestStatus = eACTIVE;
//    		gstCMDitoDH.commandResponseStatus = eNO_STATUS;
//    		gstCMDitoDH.commandDisplayBoardDH.bits.setTimeStamp = 1;
//    		gstCMDitoDH.commandDataCMDiDH.commandData.timeStamp = 0x001E4568;
    		gstCMDitoDH.commandDisplayBoardDH.val = 0;
    		gstCMDitoDH.commandRequestStatus = eACTIVE;
    		gstCMDitoDH.commandResponseStatus = eNO_STATUS;
    		gstCMDitoDH.commandDisplayBoardDH.bits.setParameter = 1;
    		gstCMDitoDH.commandDataCMDiDH.parameterNumber = 8;
    		gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 0x8A;
    		break;
    	}
    	case 'E': {
    		pressstate = 'F';
    		gstCMDitoDH.commandDisplayBoardDH.val = 0;
    		gstCMDitoDH.commandRequestStatus = eACTIVE;
    		gstCMDitoDH.commandResponseStatus = eNO_STATUS;
    		gstCMDitoDH.commandResponseACK_Status = eNO_StatusAcknowledgement;
    		gstCMDitoDH.commandDisplayBoardDH.bits.getErrorList = 1;
    		break;
    	}
    	case 'F': {
    		pressstate = 'G';
    		gstCMDitoDH.commandDisplayBoardDH.val = 0;
    		gstCMDitoDH.commandRequestStatus = eACTIVE;
    		gstCMDitoDH.commandResponseStatus = eNO_STATUS;
    		gstCMDitoDH.commandResponseACK_Status = eResponseAcknowledgement_ACK;
    		gstCMDitoDH.commandDisplayBoardDH.bits.getErrorList = 1;
    		break;
    	}
    	case 'G': {
    		pressstate = 'H';
    		gstEMtoDH.commandRequestStatus = eACTIVE;
    		gstEMtoDH.commandResponseStatus = eNO_STATUS;
    		gstEMtoDH.errorToDH.anomalyCode = 0x010A;
    		uint8_t teststr1[15] = "Test1Anomaly1";
    		ustrncpy((char *)gstEMtoDH.errorToDH.errorDetails, (const char *)teststr1, ustrlen((const char*)teststr1));
    		gstEMtoDH.errorToDH.timeStamp = 0x01230E0E;
    		break;
    	}
    	case 'H': {
    		pressstate = 'A';
    		gstCMDitoDH.commandDisplayBoardDH.val = 0;
    		gstCMDitoDH.commandRequestStatus = eACTIVE;
    		gstCMDitoDH.commandResponseStatus = eNO_STATUS;
    		gstCMDitoDH.commandDisplayBoardDH.bits.setParameter = 1;
    		gstCMDitoDH.commandDataCMDiDH.parameterNumber = 21;
    		gstCMDitoDH.commandDataCMDiDH.commandData.setParameterValue = 0x01;
    		break;
    	}
    	default:
    		break;
		}

    }

    if(gKeysStatus.bits.Key_Open_released) {
    	gKeysStatus.bits.Key_Open_released = 0;
    }
}
