/*********************************************************************************
* FileName: sdcardlogs.h
* Description:
* This header file contains interfaces for the SDCard Logs Module.
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
 *  	0.1D	03/06/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/
#ifndef __SDCARDLOGS_H__
#define __SDCARDLOGS_H__

#ifdef __cplusplus
extern "C"
{
#endif

//#include "Middleware/paramdatabase.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/


/****************************************************************************/


/****************************************************************************/

/****************************************************************************
 *  Global variables for other files:
 ****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
 ****************************************************************************/



/****************************************************************************/

/****************************************************************************
 *  Function prototype declarations:
 ****************************************************************************/
int dumpLogsToSDCard(PARAM_DISP AHorCSH_Idx);
void showLogs(PARAM_DISP AHorCSH_Idx);
/****************************************************************************/
#endif /* __SDCARDLOGS_H__ */
