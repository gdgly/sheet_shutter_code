/*********************************************************************************
* FileName: logger.h
* Description:
* This header file contains interfaces for the Logger Module.
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
 *  	0.1D	13/05/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/
#ifndef __LOGGER_H__
#define __LOGGER_H__

#ifdef __cplusplus
extern "C"
{
#endif


/****************************************************************************/
#include "Middleware/paramdatabase.h"
/****************************************************************************
 *  Macro definitions:
 ****************************************************************************/
#define RESET_CHGSET_PARAM gu8_erase_chgsethist
#define RESET_ANOM_PARAM   gu8_erase_anomhist
#define RESETCHGSET_PARAM_IDX A060ERASE_CHGSETHIST
#define RESETANOM_PARAM_IDX A040ERASE_ANOMHIST

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
/******************************************************************************
 * Function Name: initLogger
 *
 * Function Description:
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void initLogger(void);

/******************************************************************************
 * Function Name: writeChangeSettingsHistory
 *
 * Function Description:
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void writeChangeSettingsHistory(void);

/******************************************************************************
 * Function Name: writeAnomalyHistory
 *
 * Function Description:
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void writeAnomalyHistory(void);

/******************************************************************************
 * Function Name: readChangeSettingsHistory
 *
 * Function Description:
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void readChangeSettingsHistory(void);

/******************************************************************************
 * Function Name: readAnomalyHistory
 *
 * Function Description:
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void readAnomalyHistory(void);

/******************************************************************************
 * Function Name: monitorResetLogParam
 *
 * Function Description:
 *
 * Function Parameters: void
 *
 * Function Returns: 0 success 1 fail
 *
 ********************************************************************************/
uint8_t monitorResetLogParam(void);

/******************************************************************************
 * Function Name: Logger_Module
 *
 * Function Description:
 *
 * Function Parameters: void
 *
 * Function Returns: void
 *
 ********************************************************************************/
void Logger_Module(void);
/****************************************************************************/

/******************************************************************************
 * Function Name: getLogIndex
 *
 * Function Description:
 *
 * Function Parameters: ANOMHIST_START_IDX or CHGSETHIST_START_IDX
 *
 * Function Returns: Log Index
 *
 ********************************************************************************/
uint8_t getLogIndex(PARAM_DISP AHorCSH_Idx);
/****************************************************************************/
#endif /* __LOGGER_H__ */
