/********************************************************************************
* FileName: RtdmInterface.h
* Description:  
* This header file contains the decleration of all the attributes and 
* services for RtdmInterface.c file. It gives interface of DMCI to other modules.
*********************************************************************************/

/****************************************************************************
 * Copyright 2014 Bunka Shutters.
 * This program is the property of the Bunka Shutters
 * Company, Inc.and it shall not be reproduced, distributed or used
 * without permission of an authorized company official.This is an
 * unpublished work subject to Trade Secret and Copyright
 * protection.
*****************************************************************************/

/****************************************************************************
 *  Modification History
 *  
 *  Date                  Name          Comments 
 *  09/04/2014            iGate          Initial Creation                                                               
*****************************************************************************/
#ifndef RTDM_INTERFACE_H
#define RTDM_INTERFACE_H
#include "./Common/Typedefs/Typedefs.h"

/* This function updates data in DMCI buffer */
VOID updateRTDMData(VOID);

/* This function initializes DMCI data buffer */
VOID initRTDMD(VOID);

#endif  /* RTDM_INTERFACE_H */
