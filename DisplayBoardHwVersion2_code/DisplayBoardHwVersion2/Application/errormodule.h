/*********************************************************************************
* FileName: errormodule.h
* Description:
* This source file contains the prototype definition of all the services of ....
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
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define TOTAL_ERRORS			63

#define RECOVERABLE				1
#define NONRECOVERABLE			2
#define RECORDABLE_ANOMALY		1
#define NONRECORDABLE_ANOMALY	0
#define ON						1
#define OFF						0
#define DISPLAY_INDICATION		1
#define NO_DISPLAY_INDICATION	0

#define DRIV			0
#define CTRL		1
#define DISP		2

#define  NONE_OPEN_CLOSE  0
#define  ENABLE_OPEN_CLOSE 1
#define  OPEN_ONLY 2
/****************************************************************************/

/****************************************************************************
 *  Structures
****************************************************************************/
typedef struct stErrorList
{
	uint16_t errorCode;
	//uint8_t errorName[20];
	uint8_t errorName_english[30];
	uint8_t recoveryType;
	// 0: NONRECOVERABLE
	// 1: RECOVERABLE
	bool recordType;
	// 0: NONRECORDABLE_ANOMALY
	// 1: RECORDABLE_ANOMALY
	uint8_t displayFaultLED_State;
	// 0: OFF
	// 1: ON
	uint8_t controlBoardMonitorLED_State;
	// 0: OFF
	// 1: ON
	bool indicationOnDisplay;
	// 0: NO
	// 1: YES
	uint8_t errorCategroy;
	// 0: DRIVE
	// 1: CONTROL
	// 2: DISPLAY
	uint8_t errorPriority;
	// 0 is lowest priority
	uint32_t *globalFaultRegister;
	// array of pointers to global fault registers
	uint8_t bitPositionInGlobalFaultRegister;
	// error bit position in global fault register
	uint32_t *localFaultRegister;
	// pointer to local fault register
	uint8_t bitPositionInLocalFaultRegister;
	// error bit position in local fault register
	uint8_t errorName_japanese[30];
	uint8_t stop_shutter_move_comm;
	// 0:NONE_OPEN_CLOSE
	// 1:ENABLE_OPEN_CLOSE
	// 2:OPEN_ONLY
} _stErrorList;


/****************************************************************************/

/****************************************************************************
 *  Global variables:
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
extern uint8_t gucStopErrorsDisplay;

/****************************************************************************/


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

void errorModule(void);

/********************************************************************************/
