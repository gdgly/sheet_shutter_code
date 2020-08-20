/*********************************************************************************
* FileName: sdcard.h
* Description:
* This header file contains interfaces for the SDCard operations.
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
 *  	0.1D	21/04/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

#ifndef SDCARD_H_
#define SDCARD_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "Middleware/fatfs/src/ff.h"
/****************************************************************************
 *  Macro definitions:
****************************************************************************/
//Macros for File Write method
#define	FILE_APPEND_WRITE		0x10
#define FILE_OVERWRITE			0x20

//Error Code to be thrown if filename is long
#define ERR_CODE_FNAME_LONG		0x45

//Error code to be thrown if Zero bytes read or write
#define ERR_CODE_ZEROBYTES		0x46

//Error Code to be thrown if no files found in list
#define ERR_CODE_NOFILE			0x47

//Max 40 characters allowed in line
#define MAX_CHARS_IN_LINE		21



#define param_all_list_long 111
/****************************************************************************
 *  Global variables:
****************************************************************************/


//*****************************************************************************
// Function prototypes.
//*****************************************************************************

extern int8_t closeFile(void);

extern int8_t openFile(uint8_t *pui8FileName);

extern int8_t readBinaryFileFromSDcard(uint8_t *pui32FileData, uint32_t ui32NumberOfBytesToRead);

/******************************************************************************
 * Function Name: initSdCard
 *
 * Function Description: This function must be called during application
 * initialization to configure the communication pins for SDCard. It also
 * mounts the File System.
 *
 * Function Parameters: void
 *
 * Function Returns: Mount OK(FR_OK) or Not successful(FR_INVALID_DRIVE)
 *
 ********************************************************************************/
extern uint8_t initSdCard(void);

/******************************************************************************
 * Function Name: write_SDCard
 *
 * Function Description: This function must be called for writing into the
 * SDCard.
 *
 * Function Parameters:
 * char *filename 	- Filename
 * const void *writebuff - Pointer to the data to be written
 * unsigned int *bw	- Pointer to number of bytes written
 * char fmode 		- File Append or Overwrite mode. Possible values are
 * 					  FILE_APPEND_WRITE or FILE_OVERWRITE
 *
 * Function Returns:  File function return code (FRESULT)
 *
 ********************************************************************************/
extern uint8_t write_SDCard(char *filename, const void *writebuff, unsigned int *bw, char fmode);

/******************************************************************************
 * Function Name: read_SDCard
 *
 * Function Description: This function must be called for reading from SDCard.
 *
 * Function Parameters:
 * char *filename - Filename
 * const void *readbuff - Pointer to the buffer in which file is read
 * 						  (Clear the buffer before sending)
 * uint8_t lineno	- Line number to read(1,2,3,...)
 *
 * Function Returns:  File function return code (FRESULT)
 *
 ********************************************************************************/
extern uint8_t read_SDCard(char *filename, const void *readbuff, uint8_t lineno);

/******************************************************************************
 * Function Name: list_SDCard
 *
 * Function Description: This function must be called for listing of files
 *
 * Function Parameters:
 * char *startwith - Search pattern in filename
 * uint8_t listno - Fetch item from the list according to list number(1,2,..)
 * char *fnamebuff - Buffer for the fetched filename item
 *
 * Function Returns: void
 *
 ********************************************************************************/
extern uint8_t list_SDCard(char *startwith, uint8_t listno, char *fnamebuff);
extern uint8_t list_SDCard_cyw(void);

extern uint8_t delete_SDCard(char *filename);
//*****************************************************************************
//
// This function returns a string representation of an error code that was
// returned from a function call to FatFs.  It can be used for printing human
// readable error messages.
//
//*****************************************************************************
const char *StringFromFResult(FRESULT iFResult);

#endif /* SDCARD_H_ */
