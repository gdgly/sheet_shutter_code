/*********************************************************************************
* FileName: sdcard.c
* Description:
* This source file contains the definition for SDCard functionality.
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
 *  	0.3D	18/08/2014									Error Code handled
 *  	0.2D	11/08/2014									DeleteFile functionality added
 *  	0.1D	21/04/2014      	iGATE Offshore team       Initial Creation
 *  	Patch added in mmc-ek-lm4f232h5qd.c for enabling RTC timing
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inc/hw_memmap.h>
#include <driverlib/fpu.h>
#include <driverlib/gpio.h>
#include <driverlib/interrupt.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom.h>
#include <driverlib/sysctl.h>
#include <driverlib/systick.h>
#include <inc/hw_nvic.h>
#include <inc/hw_types.h>

#include "Middleware/fatfs/src/ff.h"
#include "Middleware/fatfs/src/diskio.h"
#include "Application/ustdlib.h"
#include "Middleware/uartstdio.h"
#include "Middleware/sdcard.h"
#include "Application/intertaskcommunication.h"
#include "Application/userinterface.h"
/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/

// Defines the size of the buffers that hold the path, or temporary data from
// the SD card.  There are two buffers allocated of this size.  The buffer size
// must be large enough to hold the longest expected full path name, including
// the file name, and a trailing null character.
#define PATH_BUF_SIZE           80

// Defines the size of temporary data buffer when manipulating file paths, or
// reading/writing data on SD card.
#define TEMP_BUF_SIZE			64

// A macro that holds the number of result codes.
#define NUM_FRESULT_CODES       (sizeof(g_psFResultStrings) /                 \
                                 sizeof(tFResultString))

// Defines the SSI Peripheral No. to use for SDCard
// Currently Used: SSI No.0 - SYSCTL_PERIPH_SSI0
#define SDCARD_SSI_PERIPHERAL SYSCTL_PERIPH_SSI0

// A macro to make it easy to add result codes to the table.
#define FRESULT_ENTRY(f)        { (f), (#f) }
//*****************************************************************************


/****************************************************************************
 *  Global variables
****************************************************************************/

// This buffer holds the full path to the current working directory.  Initially
// it is root ("/").
static char g_pcCwdBuf[PATH_BUF_SIZE] = "/";

// A temporary data buffer used when manipulating file paths, or reading data
// from the SD card.
static char g_pcTmpBuf[TEMP_BUF_SIZE];

// The following are data structures used by FatFs.
static FATFS g_sFatFs;
static DIR g_sDirObject;
static FILINFO g_sFileInfo;
static FIL g_sFileObject;

// A structure that holds a mapping between an FRESULT numerical code, and a
// string representation.  FRESULT codes are returned from the FatFs FAT file
// system driver.
typedef struct
{
    FRESULT iFResult;
    char *pcResultStr;
}
tFResultString;

// A table that holds a mapping between the numerical FRESULT code and it's
// name as a string.  This is used for looking up error codes for printing to
// the console.
tFResultString g_psFResultStrings[] =
{
    FRESULT_ENTRY(FR_OK),
    FRESULT_ENTRY(FR_DISK_ERR),
    FRESULT_ENTRY(FR_INT_ERR),
    FRESULT_ENTRY(FR_NOT_READY),
    FRESULT_ENTRY(FR_NO_FILE),
    FRESULT_ENTRY(FR_NO_PATH),
    FRESULT_ENTRY(FR_INVALID_NAME),
    FRESULT_ENTRY(FR_DENIED),
    FRESULT_ENTRY(FR_EXIST),
    FRESULT_ENTRY(FR_INVALID_OBJECT),
    FRESULT_ENTRY(FR_WRITE_PROTECTED),
    FRESULT_ENTRY(FR_INVALID_DRIVE),
    FRESULT_ENTRY(FR_NOT_ENABLED),
    FRESULT_ENTRY(FR_NO_FILESYSTEM),
    FRESULT_ENTRY(FR_MKFS_ABORTED),
    FRESULT_ENTRY(FR_TIMEOUT),
    FRESULT_ENTRY(FR_LOCKED),
    FRESULT_ENTRY(FR_NOT_ENOUGH_CORE),
    FRESULT_ENTRY(FR_TOO_MANY_OPEN_FILES),
    FRESULT_ENTRY(FR_INVALID_PARAMETER),
};

FRESULT iFResult;

/****************************************************************************/


/****************************************************************************
 *  Function Definations for this file:
****************************************************************************/
uint8_t initSdCard(void)
{
    // Enable the SSI Peripheral
    ROM_SysCtlPeripheralEnable(SDCARD_SSI_PERIPHERAL);

    // Mount the file system, using logical disk 0.
    iFResult = f_mount(0, &g_sFatFs);
    if(iFResult != FR_OK);
//        UARTprintf("f_mount error: %s\n", StringFromFResult(iFResult));

    return (uint8_t)iFResult;
}

uint8_t write_SDCard(char *filename, const void *writebuff, unsigned int *bw, char fmode)
{
	FRESULT iFResult;

	memset(g_pcTmpBuf, 0, sizeof(g_pcTmpBuf));

	// First, check to make sure that the current path (CWD), plus the file
	// name, plus a separator and trailing null, will all fit in the temporary
	// buffer that will be used to hold the file name.  The file name must be
	// fully specified, with path, to FatFs.
	//
	if((ustrlen(g_pcCwdBuf) + ustrlen(filename) + 1 + 1) > sizeof(g_pcTmpBuf))
	{
//		UARTprintf("Resulting path name is too long\n");
		return ERR_CODE_FNAME_LONG;
	}

	// Copy the current path to the temporary buffer so it can be manipulated.
	ustrncpy(g_pcTmpBuf, g_pcCwdBuf, ustrlen(g_pcCwdBuf) + 1);

	// If not already at the root level, then append a separator.
	if(ustrcmp("/", g_pcCwdBuf))
	{
		strcat(g_pcTmpBuf, "/");
	}

	// Now finally, append the file name to result in a fully specified file.
	strcat(g_pcTmpBuf, filename);

	// Open the file for writing.
	iFResult = f_open(&g_sFileObject, g_pcTmpBuf,(FA_OPEN_ALWAYS |
			FA_WRITE |
			FA_READ));

	// If there was some problem opening the file, then return an error.
	if(iFResult != FR_OK)
	{
//		UARTprintf("Error in Opening file - %s\n", StringFromFResult(iFResult));
		gstDisplayApplicationFault.bits.sdWrite = 1;
		return ((uint8_t)iFResult);
	}

//	UARTprintf("Open Success - %u bytes\n", g_sFileObject.fsize);

	//Move File Pointer to the end of file if Append mode selected
	if(fmode == FILE_APPEND_WRITE)
		f_lseek(&g_sFileObject, g_sFileObject.fsize);

	iFResult = f_write(&g_sFileObject, writebuff, ustrlen(writebuff),
			bw);

	if(iFResult != FR_OK) {
//		UARTprintf("Error Code: %s\n",  StringFromFResult(iFResult));
		gstDisplayApplicationFault.bits.sdWrite = 1;
		return ((int)iFResult);
	}

//	UARTprintf("Bytes Written - %u\n", *bw);

	//If 0 bytes read or
	if((*bw == 0) || (*bw !=ustrlen(writebuff)))
	{
//		UARTprintf("\n");
//		UARTprintf("Zero bytes written");
		return ERR_CODE_ZEROBYTES;
	}

	iFResult = f_close(&g_sFileObject);
//	UARTprintf("Result Close Code%s\n",  StringFromFResult(iFResult));

	// Return success.
	gstDisplayApplicationFault.bits.sdWrite = 0;
	return ((int)iFResult);
}

uint8_t read_SDCard(char *filename, const void *readbuff, uint8_t lineno)
{

	FRESULT iFResult;
	unsigned int bytesRead;
	unsigned char tempbyteread, tcount2;
	unsigned char tcount=0;
	unsigned char linecount = 0;
	unsigned char buff[MAX_CHARS_IN_LINE];
	unsigned char *buffptr = buff;

	memset(buff, 0, sizeof(buff));
	memset(g_pcTmpBuf, 0, sizeof(g_pcTmpBuf));

	//
	// First, check to make sure that the current path (CWD), plus the file
	// name, plus a separator and trailing null, will all fit in the temporary
	// buffer that will be used to hold the file name.  The file name must be
	// fully specified, with path, to FatFs.
	//
	if(ustrlen(g_pcCwdBuf) + ustrlen(filename) + 1 + 1 > sizeof(g_pcTmpBuf))
	{
//		UARTprintf("Resulting path name is too long\n");
		return ERR_CODE_FNAME_LONG;
	}

	// Copy the current path to the temporary buffer so it can be manipulated.
	ustrncpy(g_pcTmpBuf, g_pcCwdBuf, ustrlen(g_pcCwdBuf));

	// If not already at the root level, then append a separator.
	if(ustrcmp("/", g_pcCwdBuf))
	{
		strcat(g_pcTmpBuf, "/");
	}

	// Now finally, append the file name to result in a fully specified file.
	strcat(g_pcTmpBuf, filename);

	// Open the file for reading.
	iFResult = f_open(&g_sFileObject, g_pcTmpBuf, FA_READ);

	// If there was some problem opening the file, then return an error.
	if(iFResult != FR_OK)
	{
		return ((uint8_t)iFResult);
	}

	// Enter a loop to repeatedly read data from the file and display it, until
	// the end of the file is reached.
	do
	{
		// Read a byte of data from the file
		iFResult = f_read(&g_sFileObject, &tempbyteread, sizeof(tempbyteread),
				&bytesRead);

		// If there was an error reading, then print a newline and return the
		// error to the user.
		if(iFResult != FR_OK)
		{
//			UARTprintf("\n");
			gstDisplayApplicationFault.bits.sdRead = 1;
			return ((uint8_t)iFResult);
		}

		if(bytesRead == 0)
		{
//			UARTprintf("\n");
//			UARTprintf("Zero bytes read");
			return ERR_CODE_ZEROBYTES;
		}

		tcount++;

		if(tempbyteread == 0x0D) {
			tcount2 = tcount;
			continue;
		}

		if(tempbyteread == 0x0A) {
			if(tcount == (tcount2 + 1)) {
				//	    			UARTprintf("-EOL-\n");
				tcount2 = 0;
				tcount = 0;
				linecount++;
				if(linecount == lineno) {
					*buffptr = 0;
					break;
				}
				else {
					memset(buff, 0, sizeof(buff));
					buffptr = buff;
					continue;
				}
			}
		}

		*buffptr = tempbyteread;
		*buffptr++;

		// Print the last chunk of the file that was received.
		//	    	UARTprintf("%c", tempbyteread);

	}
	while(bytesRead);

//	UARTprintf("BUFF - %s", buff);
	ustrncpy((char *)readbuff, (const char*)buff, ustrlen((const char*)buff));

	// Return success.
	gstDisplayApplicationFault.bits.sdRead = 0;
	return ((int)iFResult);
}

extern unsigned char uploadParamFilenames[MAX_FILES_FOR_SDPARAM][MAX_CHARS_IN_LINE];

uint8_t list_SDCard_cyw(void)
{
	unsigned char buff_filename[MAX_CHARS_IN_LINE];
	FRESULT iFResult;
	uint8_t ui8MatchFileCount = 0;
	uint8_t Tp_i=0,Tp_i_min=0;
	uint32_t Tp_timenow,Tp_timemin,Tp_time_his;
	int tp_ret;
	//	    uint8_t ui8TotalFileCount = 0;

#if _USE_LFN
	char pucLfn[_MAX_LFN + 1];
	g_sFileInfo.lfname = pucLfn;
	g_sFileInfo.lfsize = sizeof(pucLfn);
#endif

	memset(buff_filename, 0, sizeof(buff_filename));
	//
	// Open the current directory for access.
	//
	iFResult = f_opendir(&g_sDirObject, g_pcCwdBuf);

	//
	// Check for error and return if there is a problem.
	//
	if(iFResult != FR_OK)
	{
		gstDisplayApplicationFault.bits.sdRead = 1;
		//return((int)iFResult);
		return 0;
	}


	memset((uint8_t *)uploadParamFilenames,0,MAX_CHARS_IN_LINE*(MAX_FILES_FOR_SDPARAM - 1));

	//
	// Enter loop to enumerate through all directory entries.
	//
	for(;;)
	{
		//
		// Read an entry from the directory.
		//
		iFResult = f_readdir(&g_sDirObject, &g_sFileInfo);

		//
		// Check for error and return if there is a problem.
		//
		if(iFResult != FR_OK)
		{
			gstDisplayApplicationFault.bits.sdRead = 1;
			//return((int)iFResult);
			return 0;
		}


			if((*g_sFileInfo.lfname)!=0)
			{

				if(g_sFileInfo.lfname[0]=='P'&&g_sFileInfo.lfname[1]=='A'&&g_sFileInfo.lfname[2]=='R')
				{
					//for(ui8MatchFileCount=MAX_FILES_FOR_SDPARAM-1;ui8MatchFileCount>=1;)
					//{
					//ui8MatchFileCount--;
					//memcpy((uint8_t *)(uploadParamFilenames[ui8MatchFileCount+1]),(uint8_t *)(uploadParamFilenames[ui8MatchFileCount]),20);
					//}

					//memcpy((uint8_t *)(uploadParamFilenames[0]),(uint8_t *)g_sFileInfo.lfname,20);
					memcpy((uint8_t *)(buff_filename),(uint8_t *)g_sFileInfo.lfname,20);
					Tp_i_min = 0;
					if(ui8MatchFileCount >= MAX_FILES_FOR_SDPARAM)
					{
						ui8MatchFileCount = 11;
						for(Tp_i=0;Tp_i<10;Tp_i++)
						{
							tp_ret = strcmp((char *)(uploadParamFilenames[Tp_i_min]),(char *)(uploadParamFilenames[Tp_i+1]));
							if(tp_ret>0)
							{
								Tp_i_min = Tp_i;
							}
						}
						tp_ret = strcmp((char *)(uploadParamFilenames[Tp_i_min]),(char *)(buff_filename));
						if(tp_ret<0)
						{
							memcpy((uint8_t *)(uploadParamFilenames[Tp_i_min]),(uint8_t *)g_sFileInfo.lfname,20);
						}

					}
					else
					{
						memcpy((uint8_t *)(uploadParamFilenames[ui8MatchFileCount]),(uint8_t *)g_sFileInfo.lfname,20);
						ui8MatchFileCount++;
					}

				}
			}
			else
			{
				break;
			}





	} // End of for loop

	//Check for conditions FILE END

	gstDisplayApplicationFault.bits.sdRead = 0;
	return ui8MatchFileCount; // At this step, it returns FR_OK
}

uint8_t list_SDCard(char *startwith, uint8_t listno, char *fnamebuff)
{
	unsigned char buff_filename[MAX_CHARS_IN_LINE];
	FRESULT iFResult;
	uint8_t ui8MatchFileCount = 0;
	//	    uint8_t ui8TotalFileCount = 0;

#if _USE_LFN
	char pucLfn[_MAX_LFN + 1];
	g_sFileInfo.lfname = pucLfn;
	g_sFileInfo.lfsize = sizeof(pucLfn);
#endif

	memset(buff_filename, 0, sizeof(buff_filename));
	//
	// Open the current directory for access.
	//
	iFResult = f_opendir(&g_sDirObject, g_pcCwdBuf);

	//
	// Check for error and return if there is a problem.
	//
	if(iFResult != FR_OK)
	{
		gstDisplayApplicationFault.bits.sdRead = 1;
		return((int)iFResult);
	}


	//
	// Enter loop to enumerate through all directory entries.
	//
	for(;;)
	{
		//
		// Read an entry from the directory.
		//
		iFResult = f_readdir(&g_sDirObject, &g_sFileInfo);

		//
		// Check for error and return if there is a problem.
		//
		if(iFResult != FR_OK)
		{
			gstDisplayApplicationFault.bits.sdRead = 1;
			return((int)iFResult);
		}

		//
		// If the file name is blank, then this is the end of the listing.
		//
		if(!g_sFileInfo.fname[0])
		{
			//Total file count available here
			return ERR_CODE_NOFILE;
		}
		//	    	else
		//	    		ui8TotalFileCount++;

		//
		// If the attribute is directory, then increment the directory count.
		//
		if(g_sFileInfo.fattrib & AM_DIR)
		{
			continue;
		}

		//
		// Otherwise, it is a file.  Increment the file count, and add in the
		// file size to the total.
		//
		else
		{
			//Check if pattern matches
			if(!ustrncasecmp(startwith, g_sFileInfo.fname, ustrlen(startwith))) {
				ui8MatchFileCount++;
			}
			else
				continue;

		}

		if(ui8MatchFileCount == listno) {
#if _USE_LFN
//			buff_filename = ((*g_sFileInfo.lfname)?g_sFileInfo.lfname:g_sFileInfo.fname);
			if(*g_sFileInfo.lfname)
				ustrncpy((char *)buff_filename, (const char *)g_sFileInfo.lfname, ustrlen((const char *)g_sFileInfo.lfname));
//				usnprintf((char *)buff_filename, sizeof(buff_filename), "%s", (const char *)g_sFileInfo.lfname);
			else
				ustrncpy((char *)buff_filename, (const char *)&g_sFileInfo.fname, ustrlen((const char *)g_sFileInfo.fname));
			//CHECK ADDRESS AND VALUE HERE//
#else
			buff_filename = g_sFileInfo.fname;
#endif
			ustrncpy(fnamebuff, (const char *)buff_filename, ustrlen((const char *)buff_filename) + 1);
//			UARTprintf("s");
			break;
		}

	} // End of for loop

	//Check for conditions FILE END

	gstDisplayApplicationFault.bits.sdRead = 0;
	return ((int)iFResult); // At this step, it returns FR_OK
}

uint8_t delete_SDCard(char *filename)
{

	FRESULT iFResult;

	memset(g_pcTmpBuf, 0, sizeof(g_pcTmpBuf));

	//
	// First, check to make sure that the current path (CWD), plus the file
	// name, plus a separator and trailing null, will all fit in the temporary
	// buffer that will be used to hold the file name.  The file name must be
	// fully specified, with path, to FatFs.
	//
	if(ustrlen(g_pcCwdBuf) + ustrlen(filename) + 1 + 1 > sizeof(g_pcTmpBuf))
	{
//		UARTprintf("Resulting path name is too long\n");
		return ERR_CODE_FNAME_LONG;
	}

	// Copy the current path to the temporary buffer so it can be manipulated.
	ustrncpy(g_pcTmpBuf, g_pcCwdBuf, ustrlen(g_pcCwdBuf));

	// If not already at the root level, then append a separator.
	if(ustrcmp("/", g_pcCwdBuf))
	{
		strcat(g_pcTmpBuf, "/");
	}

	// Now finally, append the file name to result in a fully specified file.
	strcat(g_pcTmpBuf, filename);

	// Delete the file
	iFResult = f_unlink(g_pcTmpBuf);

	return ((int)iFResult); // At this step, it returns FR_OK if successful
}

int8_t closeFile(void)
{
	FRESULT iFResult;

	iFResult = f_close(&g_sFileObject);

	return iFResult;
}

int8_t openFile(uint8_t *pui8FileName)
{
	FRESULT iFResult;

	memset(g_pcTmpBuf, 0, sizeof(g_pcTmpBuf));

	//
	// First, check to make sure that the current path (CWD), plus the file
	// name, plus a separator and trailing null, will all fit in the temporary
	// buffer that will be used to hold the file name.  The file name must be
	// fully specified, with path, to FatFs.
	//
	if(ustrlen(g_pcCwdBuf) + ustrlen((char *)pui8FileName) + 1 + 1 > sizeof(g_pcTmpBuf))
	{
//		UARTprintf("Resulting path name is too long\n");
		return ERR_CODE_FNAME_LONG;
	}

	// Copy the current path to the temporary buffer so it can be manipulated.
	ustrncpy(g_pcTmpBuf, g_pcCwdBuf, ustrlen(g_pcCwdBuf));

	// If not already at the root level, then append a separator.
	if(ustrcmp("/", g_pcCwdBuf))
	{
		strcat(g_pcTmpBuf, "/");
	}

	// Now finally, append the file name to result in a fully specified file.
	strcat(g_pcTmpBuf, (char *)pui8FileName);

	// Open the file for reading.
	iFResult = f_open(&g_sFileObject, g_pcTmpBuf, FA_READ);

	return ((int8_t)iFResult);
}

int8_t readBinaryFileFromSDcard(uint8_t *pui32FileData, uint32_t ui32NumberOfBytesToRead)
{
	FRESULT iFResult;
	uint32_t luiByteCount = 0;
	unsigned char tempbyteread;
	unsigned int bytesRead;

	// Enter a loop to repeatedly read data from the file and display it, until
	// the end of the file is reached.
	for(luiByteCount = 0; luiByteCount < ui32NumberOfBytesToRead; luiByteCount++)
	{
		// Read a byte of data from the file
		iFResult = f_read(&g_sFileObject, &tempbyteread, sizeof(tempbyteread),
				&bytesRead);

		// If there was an error reading, then print a newline and return the
		// error to the user.
		if(iFResult != FR_OK)
		{
//			UARTprintf("\n");
			gstDisplayApplicationFault.bits.sdRead = 1;
			return ((int8_t)iFResult);
		}

		if(bytesRead == 0)
		{
			return ERR_CODE_ZEROBYTES;
		}

		*pui32FileData = tempbyteread;
		pui32FileData++;

	}


	// Return success.
	gstDisplayApplicationFault.bits.sdRead = 0;
	return ((int8_t)iFResult);
}


const char *
StringFromFResult(FRESULT iFResult)
{
    uint_fast8_t ui8Idx;

    //
    // Enter a loop to search the error code table for a matching error code.
    //
    for(ui8Idx = 0; ui8Idx < NUM_FRESULT_CODES; ui8Idx++)
    {
        //
        // If a match is found, then return the string name of the error code.
        //
        if(g_psFResultStrings[ui8Idx].iFResult == iFResult)
        {
            return(g_psFResultStrings[ui8Idx].pcResultStr);
        }
    }

    //
    // At this point no matching code was found, so return a string indicating
    // an unknown error.
    //
    return("UNKNOWN ERROR CODE");
}
