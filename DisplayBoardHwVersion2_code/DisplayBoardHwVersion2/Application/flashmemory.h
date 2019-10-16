/*********************************************************************************
* FileName: flashmemory.h
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

#ifndef __FLASHMEMORY_H__
#define __FLASHMEMORY_H__

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define SUCCESS						0
#define FLASH_ERASE_FAILED			1
#define SD_CARD_FILE_OPEN_FAILED	2
#define SD_CARD_FILE_READ_FAILED	3
#define FLASH_WRITE_FAILED			4
#define SD_CARD_FILE_CLOSE_FAILED	5


#define FLASH_APPLICATION_AREA_STARTING_ADDRESS			0x13000
#define BOOT_LOADER_AREA_STARTING_ADDRESS				0x00000
#define FLASH_STARTING_ADDRESS							0x0
#define FLASH_ONE_BLOCK_SIZE							1024
#define FLASH_ONE_WORD_SIZE								4
#define FLASH_NUMBER_OF_BLOCKS_FOR_APPLICATION			150

#define FLASH_FIRMWARE_UPGRADE_FROM_APP_FLAG		(FLASH_NUMBER_OF_BLOCKS_FOR_APPLICATION * FLASH_ONE_BLOCK_SIZE) +	\
												FLASH_APPLICATION_AREA_STARTING_ADDRESS

/****************************************************************************/


/****************************************************************************
 *  Global variables:
****************************************************************************/

extern uint8_t gui8FirmwareUpgradeCommandResponseReceived;
/****************************************************************************/


/******************************************************************************
 * FunctionName
 *
 * Function Description:

 *
 * Function Parameters:
 *
 * Function Returns:
 *
 ********************************************************************************/

int8_t eraseApplicationAreaInFlash(void);
int8_t writeApplicationAreaInFlash(uint32_t *pui32Data, uint32_t uiBlockAddress, uint32_t uiNumberOfBytes);
int8_t displayBoardSelfFirmwareUpgrade(uint8_t *pui8Filemame);
int8_t verifyApplicationImageInFlash(void);
void jumpToApplicationCode(void);

/********************************************************************************/

#endif /*__FLASHMEMORY_H__*/
