/*********************************************************************************
* FileName: ledhandler.h
* Description:
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
 *  	0.1D	05/08/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/
#ifndef LEDHANDLER_H_
#define LEDHANDLER_H_

/****************************************************************************
 *  Macro definitions:
****************************************************************************/

// LED Status
#define LED_OFF		0x00
#define LED_ON		0x01
#define LED_FLASH	0x02
#define LED_BLINK	0x03

// LED Flash States
#define _3_SEC_OFF_1	0
#define _500_MSEC_ON_1	1
#define _500_MSEC_OFF	2
#define _500_MSEC_ON_2	3
#define _3_SEC_OFF_2	4

// LED blink status
#define _BLINK_STATUS_50_MSEC		0x13
#define _BLINK_STATUS_100_MSEC		0x23
#define _BLINK_STATUS_150_MSEC		0x33
#define _BLINK_STATUS_200_MSEC		0x43
#define _BLINK_STATUS_250_MSEC		0x53
#define _BLINK_STATUS_300_MSEC		0x63
#define _BLINK_STATUS_350_MSEC		0x73
#define _BLINK_STATUS_400_MSEC		0x83
#define _BLINK_STATUS_450_MSEC		0x93
#define _BLINK_STATUS_500_MSEC		0xA3

// LED flash count status
#define _FLASH_STATUS_COUNT_1		0x12
#define _FLASH_STATUS_COUNT_2		0x22
#define _FLASH_STATUS_COUNT_3		0x32
#define _FLASH_STATUS_COUNT_4		0x42
#define _FLASH_STATUS_COUNT_5		0x52
#define _FLASH_STATUS_COUNT_6		0x62
#define _FLASH_STATUS_COUNT_7		0x72
#define _FLASH_STATUS_COUNT_8		0x82

/******************************************************************************
 * FunctionName: displayBoardLEDHandler
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
void displayBoardLEDHandler(void);


#endif /* LEDHANDLER_H_ */
