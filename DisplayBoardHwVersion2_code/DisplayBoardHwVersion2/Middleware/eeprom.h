/*********************************************************************************
* FileName: eeprom.h
* Description: 
* This source file contains the prototype definitions for EEPROM operations.
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
 *  	0.1D	31/03/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

#ifndef EEPROM_H
#define EEPROM_H

#ifdef __cplusplus
extern "C"
{
#endif


/****************************************************************************
 *  Macro definitions:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global variables:
****************************************************************************/


/****************************************************************************/

/****************************************************************************
 *  Structures:
****************************************************************************/
typedef union _DataWordType
{
	uint8_t   ui8DataBytes[4];
	uint32_t  ui32DataWord;
} DataWordType;
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
 * 				   Minimum value is 1.
 *
 * Function Returns:
 * 0	:	Success
 * 1	:	Incorrect address or count value
 *
 ********************************************************************************/
extern uint8_t EEPROMReadByte(uint8_t *pui8Data, uint32_t ui32Address,
                       uint8_t ui8Count);
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
 * 					Minimum value is 1.
 *
 * Function Returns	:
 * 0	:	Success
 * 1	:	Incorrect address or count value
 *
 ********************************************************************************/
extern uint32_t EEPROMProgramByte(uint8_t *pui8Data,
                              uint32_t ui32Address,
                              uint8_t ui8Count);
/********************************************************************************/


/********************************************************************************/

#endif /*EEPROM_H*/
