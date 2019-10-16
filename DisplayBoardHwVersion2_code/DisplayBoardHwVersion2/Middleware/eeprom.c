/*********************************************************************************
* FileName: rtc.c
* Description:
* This source file contains the definitions for EEPROM functionality.
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
 *  	0.2D	18/08/2014									Handled Error Code
 *  	0.1D	01/04/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <driverlib/eeprom.h>
#include "inc/hw_types.h"
#include "eeprom.h"
#include "Application/intertaskcommunication.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define	 MAX_SIZE  256
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

/****************************************************************************/

/******************************************************************************
 * FunctionName: EEPROMRead
 *
 * Function Description:
 * Reads data from the EEPROM.
 *
 * Function Parameters:
 * pui32Data 	:  It is a pointer to storage for the data read from the EEPROM.
 * 				   This pointer must point to at least \e ui32Count bytes of available memory.
 * ui32Address 	:  It is the byte address within the EEPROM from which data is to be read.
 * 				   This value must be between 0 to 2048.
 * ui32Count 	:  It is the number of bytes of data to read from the EEPROM.
 * 				   Minimum value is 1 and maximum value is 256.
 *
 * Function Returns:
 * 0	:	Success
 * 1	:	Incorrect address or count value
 *
 ********************************************************************************/
uint8_t EEPROMReadByte(uint8_t *pui8Data, uint32_t ui32Address,
                       uint8_t ui8Count)
{
#if 1

	uint32_t ui32WordStartAddress, ui32LastWordStartAddress;
	uint32_t ui8StartIndex;
	DataWordType dataWord;
	uint8_t ui8ByteCount;
	uint32_t *pui32ReadDataPtr = NULL;
	uint8_t *pui8ReadPtr = pui8Data;

	if( ( ui8Count > MAX_SIZE ) || ( ui32Address > 0x800 ) )
	{
		gstDisplayProcessorFault.bits.eepromRead = 1;
		return 1;
	}

	//
	// Calculate start address
	//
	ui32WordStartAddress = ( ui32Address / 4 ) * 4;
	ui8StartIndex = ui32Address % 4;

	//
	// Check whether last byte address is valid
	//
	if( (ui32Address +  ui8Count) > 0x800 )
	{
		gstDisplayProcessorFault.bits.eepromRead = 1;
		return 1;
	}

	//
	// Calculate Last word start address
	//
	ui32LastWordStartAddress = ( (ui32Address +  ui8Count) / 4 ) * 4;

	//
	// Read word from ui32WordStartAddress
	//
	EEPROMRead(&dataWord.ui32DataWord, ui32WordStartAddress, 4);

	//
	// Check whether number of bytes to be read can be accomodated in first word.
	//
	if((ui32Address + ui8Count) < (ui32WordStartAddress + 4))
	{
		memcpy(pui8ReadPtr,&dataWord.ui8DataBytes[ui8StartIndex], ui8Count);
	}
	else
	{
		memcpy(pui8ReadPtr,&dataWord.ui8DataBytes[ui8StartIndex], 4 - ui8StartIndex);
	}

	//
	// Check whether only one word read is required
	//
	if( 0 == (ui32LastWordStartAddress - ui32WordStartAddress) )
	{
		gstDisplayProcessorFault.bits.eepromRead = 0;
		return 0;
	}

	//
	// Check whether we need to read intermediate bytes
	//
	if((ui32LastWordStartAddress - ui32WordStartAddress) > 4)
	{
		//
		// Modify word start address
		//
		ui32WordStartAddress += 4;

		//
		// Calculate intermediate byte count
		//
		ui8ByteCount = ui32LastWordStartAddress - ui32WordStartAddress;

		//
		// Modify read pointer
		//
		pui8ReadPtr += (4 - ui8StartIndex);
		pui32ReadDataPtr = (uint32_t *)pui8ReadPtr;

		//
		// Read intermediate bytes
		//
		EEPROMRead(pui32ReadDataPtr, ui32WordStartAddress, ui8ByteCount);
	}

	//
	// Calculate start address again
	//
	ui32WordStartAddress = ( ui32Address / 4 ) * 4;

	//
	// Modify read pointer to read last word
	//
	if((ui32LastWordStartAddress - ui32WordStartAddress) > 4)
		pui8ReadPtr += ui8ByteCount;
	else
		pui8ReadPtr += (4 - ui8StartIndex);

	//
	// Read last word
	//
	EEPROMRead(&dataWord.ui32DataWord, ui32LastWordStartAddress, 4);
	memcpy(pui8ReadPtr, dataWord.ui8DataBytes, ui32Address + ui8Count - ui32LastWordStartAddress);

#endif


		gstDisplayProcessorFault.bits.eepromRead = 0;

	return 0;
}
/********************************************************************************/

/******************************************************************************
 * FunctionName: EEPROMProgram
 *
 * Function Description:
 * Writes data to the EEPROM.
 *
 * Function Parameters:
 * pui32Data	:	It points to the first word of data to write to the EEPROM.
 * ui32Address	: 	It defines the byte address within the EEPROM that the data
 * 					is to be written to.  This value must be between 0 to 2048.
 * ui32Count	:	It defines the number of bytes of data that is to be written.
 * 					Minimum value is 1 and maximum value is 256.
 *
 * Function Returns	:
 * 0	:	Success
 * 1	:	Incorrect address or count value
 *
 ********************************************************************************/
uint32_t EEPROMProgramByte(uint8_t *pui8Data,
                              uint32_t ui32Address,
                              uint8_t ui8Count)
{

#if 1

	uint32_t ui32WordStartAddress, ui32LastWordStartAddress;
	uint32_t ui8StartIndex;
	DataWordType dataWord;
	uint8_t ui8x, ui8y, ui8ByteCount;
	uint32_t *pui32DataPtr = NULL;

	if( ( ui8Count > MAX_SIZE ) || ( ui32Address > 0x800 ) )
	{
		gstDisplayProcessorFault.bits.eepromProgramming = 1;
		return 1;
	}

	//
	// Calculate start address
	//
	ui32WordStartAddress = ( ui32Address / 4 ) * 4;
	ui8StartIndex = ui32Address % 4;

	//
	// Check whether last byte address is valid
	//
	if( (ui32Address +  ui8Count) > 0x800 )
	{
		gstDisplayProcessorFault.bits.eepromProgramming = 1;
		return 1;
	}

	//
	// Calculate Last word start address
	//
	ui32LastWordStartAddress = ( (ui32Address +  ui8Count) / 4 ) * 4;

	//
	// Read word from Word Start Address
	//
	EEPROMRead(&dataWord.ui32DataWord, ui32WordStartAddress, 4);

	//
	// Modify first word with initial bytes with a check whether
	// number of values to be written can be accomodated in first word
	//
	if((ui32Address + ui8Count) < (ui32WordStartAddress + 4))
		for(ui8x = ui8StartIndex, ui8y = 0; ui8x <= ui8Count; ui8x++,ui8y++)
				dataWord.ui8DataBytes[ui8x] = *(pui8Data + ui8y );
	else
		for(ui8x = ui8StartIndex, ui8y = 0; ui8x < 4; ui8x++,ui8y++)
			dataWord.ui8DataBytes[ui8x] = *(pui8Data + ui8y );

	//
	// Write first word to EEPROM
	//
	EEPROMProgram(&dataWord.ui32DataWord, ui32WordStartAddress, 4);

	//
	// Check whether only one word write is required
	//
	if( 0 == (ui32LastWordStartAddress - ui32WordStartAddress) )
	{
		gstDisplayProcessorFault.bits.eepromProgramming = 0;
		return 0;
	}


	//
	// Check whether we need to write intermediate bytes
	//
	if((ui32LastWordStartAddress - ui32WordStartAddress) > 4)
	{
		//
		// Modify word start address
		//
		ui32WordStartAddress += 4;

		//
		// Calculate intermediate byte count
		//
		ui8ByteCount = ui32LastWordStartAddress - ui32WordStartAddress;

		//
		// Modify Data pointer
		//
		pui8Data = pui8Data + (4 - ui8StartIndex);
		pui32DataPtr = (uint32_t *)pui8Data;

		//
		// Write intermediate bytes defined by byte count
		//
		EEPROMProgram(pui32DataPtr, ui32WordStartAddress, ui8ByteCount);
	}

	//
	// Calculate start address again
	//
	ui32WordStartAddress = ( ui32Address / 4 ) * 4;

	//
	// Modify data pointer to write last word
	//
	if((ui32LastWordStartAddress - ui32WordStartAddress) > 4)
	{
		pui8Data += ui8ByteCount;
	}
	else
	{
		pui8Data = pui8Data + (4 - ui8StartIndex);
	}


	//
	// Program last word
	//
	memset(&dataWord,0,sizeof(dataWord));
	EEPROMRead(&dataWord.ui32DataWord, ui32LastWordStartAddress, 4);
	for(ui8x = 0; ui8x < ui32Address + ui8Count - ui32LastWordStartAddress; ui8x++)
		dataWord.ui8DataBytes[ui8x] = *(pui8Data + ui8x);
	EEPROMProgram(&dataWord.ui32DataWord, ui32LastWordStartAddress, 4);

#endif

	gstDisplayProcessorFault.bits.eepromProgramming = 0;
	return 0;
}
/********************************************************************************/
