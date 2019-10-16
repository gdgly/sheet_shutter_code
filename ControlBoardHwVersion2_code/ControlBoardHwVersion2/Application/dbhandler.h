/*********************************************************************************
* FileName: dbhandler.h
* Description:
* This header file contains interfaces for the Database Handler Module.
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
 *  	0.1D	21/05/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/
#ifndef __DBHANDLER_H__
#define __DBHANDLER_H__

#ifdef __cplusplus
extern "C"
{
#endif


/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/
#define INIT_VALUE_PARAM 				gu8_init_valueset
#define INIT_VALUE_PARAM_IDX 			A021INIT_VALUESET

//Added 25 Jun 14
#define OPRCNTRES_PARAM 				gu8_opr_cntres
#define OPRCNTRES_PARAM_IDX 			A020OPR_CNTRES
#define INITSHEETPOSPARAMS_PARAM 		gu8_init_sheetpospar
#define INITSHEETPOSPARAMS_PARAM_IDX 	A120INIT_SHEETPOSPAR
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

void initdbhandler(void);
void writeAnomalyHistory(void);
void DBHandler_Module(void);
void handleCMDi(void);
void monitorResetParam(void);
/****************************************************************************/
#endif /* __DBHANDLER_H__ */
