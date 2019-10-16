/*********************************************************************************
* FileName: flashmemory.c
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
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdint.h>
#include <string.h>
#include <driverlib/rom.h>
#include <driverlib/sw_crc.h>
#include <inc/hw_types.h>
#include <inc/hw_nvic.h>

#include "flashmemory.h"


#include "intertaskcommunication.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
****************************************************************************/


/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
****************************************************************************/




/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
****************************************************************************/


void jumpToApplicationCode(void)
{
	HWREG(NVIC_VTABLE) = (uint32_t)FLASH_APPLICATION_AREA_STARTING_ADDRESS;
	//
	// Load the stack pointer from the application's vector table.
	//
	/*__asm(" ldr r1, [r0]\n"
			" mov sp, r1");*/
	__asm(" ldr r1, [r0]");
	__asm(" mov sp, r1");

	//
	// Load the initial PC from the application's vector table and branch to
	// the application's entry point.
	//
	/*__asm(" ldr r0, [r0, #4]\n"
			" bx r0\n");*/
	__asm(" ldr r0, [r0, #4]");
	__asm(" bx r0\n");
}

/****************************************************************************/


/******************************************************************************
 * FunctionName
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

int8_t verifyApplicationImageInFlash(void)
{
	int8_t li8ReturnValue = SUCCESS;

	uint32_t lui32BlockCount = 0;
	uint8_t luiaFlashData[FLASH_ONE_BLOCK_SIZE];
	uint16_t lui16CRC16 = 0x0;

	uint32_t lui32ApplicationImageLength = 0;
	uint32_t lui32ApplicationImageCRC = 0;

	uint16_t lui16BlocksOccupiedByApplicationImage = 0;
	uint16_t lui16BytesInLastBlockOfApplicationImage = 0;

	uint32_t lui32FlashMemoryAddress = 0;
	
	union ui32TempData lun32TempWord;

	//	Copy existing application image CRC value
	lui32FlashMemoryAddress = (FLASH_NUMBER_OF_BLOCKS_FOR_APPLICATION * FLASH_ONE_BLOCK_SIZE) +
			FLASH_APPLICATION_AREA_STARTING_ADDRESS - FLASH_ONE_WORD_SIZE;
	lun32TempWord.word.val = 0;
	memcpy(lun32TempWord.byte,(const void*)lui32FlashMemoryAddress,FLASH_ONE_WORD_SIZE);

	lui32ApplicationImageCRC = lun32TempWord.word.val;

	//	Copy existing application image length value
	lui32FlashMemoryAddress = (FLASH_NUMBER_OF_BLOCKS_FOR_APPLICATION * FLASH_ONE_BLOCK_SIZE) +
			FLASH_APPLICATION_AREA_STARTING_ADDRESS - (2 * FLASH_ONE_WORD_SIZE);
	memcpy(lun32TempWord.byte,(const void*)lui32FlashMemoryAddress,FLASH_ONE_WORD_SIZE);

	lui32ApplicationImageLength = lun32TempWord.word.val;


	//	Calculate how many 1kB blocks are occupied by application image
	lui16BlocksOccupiedByApplicationImage = lui32ApplicationImageLength / FLASH_ONE_BLOCK_SIZE;

	//	Calculate how many bytes are present in last block of application image. If number of bytes of
	//	application image are not in multiples of 1024 the last block containing application image bytes
	//	will have bytes less than 1024
	lui16BytesInLastBlockOfApplicationImage = lui32ApplicationImageLength % FLASH_ONE_BLOCK_SIZE;


	for(lui32BlockCount = 0; lui32BlockCount < lui16BlocksOccupiedByApplicationImage; lui32BlockCount++)
	{
		//	Calculate flash block starting address
		lui32FlashMemoryAddress = (lui32BlockCount * FLASH_ONE_BLOCK_SIZE) + FLASH_APPLICATION_AREA_STARTING_ADDRESS;

		memset(luiaFlashData,'\0',sizeof(luiaFlashData));
		//	Copy data from flash
		memcpy(luiaFlashData,(const void*)lui32FlashMemoryAddress,FLASH_ONE_BLOCK_SIZE);

		//	Calculate CRC
		lui16CRC16 = Crc16(lui16CRC16,luiaFlashData,FLASH_ONE_BLOCK_SIZE);
	}
	//	If application image size
	if(0 != lui16BytesInLastBlockOfApplicationImage)
	{
		//	Calculate flash block starting address
		lui32FlashMemoryAddress = (lui16BlocksOccupiedByApplicationImage * FLASH_ONE_BLOCK_SIZE) + FLASH_APPLICATION_AREA_STARTING_ADDRESS;

		memset(luiaFlashData,'\0',sizeof(luiaFlashData));
		//	Copy data from flash
		memcpy(luiaFlashData,(const void*)lui32FlashMemoryAddress,lui16BytesInLastBlockOfApplicationImage);

		//	Calculate CRC
		lui16CRC16 = Crc16(lui16CRC16,luiaFlashData,lui16BytesInLastBlockOfApplicationImage);
	}

	if(lui16CRC16 != lui32ApplicationImageCRC)
	{
		li8ReturnValue = FLASH_IMAGE_VERIFICATION_FAILED;
	}
	return li8ReturnValue;
}

/******************************************************************************
 * FunctionName
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

int8_t eraseApplicationAreaInFlash(void)
{
	int8_t li8ReturnValue = SUCCESS;
	uint32_t lui32StartingAddressOfBlock = 0;
	uint8_t lui8BlockCount = 0;

	for(lui8BlockCount = 0;lui8BlockCount < FLASH_NUMBER_OF_BLOCKS_FOR_APPLICATION; lui8BlockCount++)
	{
		lui32StartingAddressOfBlock = (lui8BlockCount * FLASH_ONE_BLOCK_SIZE) + FLASH_APPLICATION_AREA_STARTING_ADDRESS;
		li8ReturnValue = ROM_FlashErase(lui32StartingAddressOfBlock);
		if(SUCCESS != li8ReturnValue)
		{
			// Erase operation failed
			return li8ReturnValue;
		}
	}
	return li8ReturnValue;
}

/******************************************************************************
 * FunctionName
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

int8_t writeApplicationAreaInFlash(uint32_t *pui32Data, uint32_t uiBlockAddress, uint32_t uiNumberOfBytes)
{
	int8_t li8ReturnValue = SUCCESS;

	li8ReturnValue = ROM_FlashProgram(pui32Data, uiBlockAddress, uiNumberOfBytes);

	return li8ReturnValue;
}

/********************************************************************************/



/******************************************************************************
 * FunctionName
 *
 * Function Description:
 *
 *
 * Function Parameter: void
 *
 * Function Returns: void
 *
 ********************************************************************************/

/*
//int8_t displayBoardFirmwareUpgrade(uint8_t *pui8Filemame)
//{
//	int8_t li8ReturnValue = SUCCESS;
//	uint32_t lui32BlockNumber = 0;
//	uint32_t lui32BlockStartAddress = 0;
//	uint8_t luca8FileData[1024];
//
//	//
//	//	Erase application area in flash
//	//
//	li8ReturnValue = eraseApplicationAreaInFlash();
//
//	if(SUCCESS != li8ReturnValue)
//	{
//		// Flash erase failed
//		li8ReturnValue = FLASH_ERASE_FAILED;
//
//		return li8ReturnValue;
//	}
//
//
//
//	return li8ReturnValue;
//}
*/
