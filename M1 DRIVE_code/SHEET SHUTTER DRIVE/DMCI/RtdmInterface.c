/*********************************************************************************
* FileName: RtdmInterface.c
* Description:
* This source file contains the definition of all the functions for RtdmInterface.
* It gives interface of DMCI to other modules.
**********************************************************************************/

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
#include <p33Exxxx.h>
#include "./DMCI/RtdmInterface.h"
#include "./DMCI/RTDMUSER.h"
#include "./DMCI/RTDM.h"
#include "./Common/Extern/Extern.h"

#define DATA_BUFFER_SIZE 100  /* Size in 16-bit Words */
#define SNAPDELAY	0//500 /* In number of PWM Interrupts */

#define	SNAP1		refSpeed//rampCurrentPosition//iTotalADCCnt//currentState//iTotalADCCnt//sector
#define	SNAP2		currentSectorNo//rampCurrentPosition//rampCurrentSpeed//ADC1BUF1//controlOutput//Ia//period//speedSnap//controlOutput//currentAverage//phase
#define SNAP3		measuredSpeed//refSpeed//measurediTotal//Ib//measurediTotal//measuredSpeed1//measurediTotal//PDC1
#define SNAP4		controlOutput//measurediTotal//hallCounts//measuredSpeed//Ic//measuredSpeed//iTotalInstFilter//measuredSpeed

SHORT recorderBuffer1[DATA_BUFFER_SIZE];  /* Buffer to store the data samples for the DMCI data viewer Graph1 */
SHORT recorderBuffer2[DATA_BUFFER_SIZE];	/* Buffer to store the data samples for the DMCI data viewer Graph2 */
SHORT recorderBuffer3[DATA_BUFFER_SIZE];	/* Buffer to store the data samples for the DMCI data viewer Graph3 */
SHORT recorderBuffer4[DATA_BUFFER_SIZE];	/* Buffer to store the data samples for the DMCI data viewer Graph4 */

SHORT * ptrRecBuffer1 = &recorderBuffer1[0];	/* Tail pointer for the DMCI Graph1 */
SHORT * ptrRecBuffer2 = &recorderBuffer2[0];	/* Tail pointer for the DMCI Graph2 */
SHORT * ptrRecBuffer3 = &recorderBuffer3[0];	/* Tail pointer for the DMCI Graph3 */
SHORT * ptrRecBuffer4 = &recorderBuffer4[0];	/* Tail pointer for the DMCI Graph4 */
SHORT * ptrRecBuffUpperLimit = recorderBuffer4 + DATA_BUFFER_SIZE -1;	/* Buffer Recorder Upper Limit */

typedef struct DMCIFlags{
    unsigned Recorder : 1;	/* Flag needs to be set to start buffering data */
    unsigned StartStop : 1;
    unsigned unused : 14;  
} DMCIFLAGS;

DMCIFLAGS DMCIFlags;

SHORT	snapCount;
SHORT snapShotDelayCnt;
SHORT snapShotDelay;

/******************************************************************************
 * updateRTDMData
 *
 * The updateRTDMData function updates the application data in DMCI data buffer.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/ 
VOID updateRTDMData(VOID)
{
    if(DMCIFlags.Recorder)
    {
        if(snapShotDelayCnt++ == snapShotDelay)
        {
            snapShotDelayCnt = 0;
            *ptrRecBuffer1++ 	= SNAP1;
            *ptrRecBuffer2++	= SNAP2;
            *ptrRecBuffer3++	= SNAP3;
            *ptrRecBuffer4++	= SNAP4;
            
            if(ptrRecBuffer4 > ptrRecBuffUpperLimit)
            {
                ptrRecBuffer1 = recorderBuffer1;
                ptrRecBuffer2 = recorderBuffer2;
                ptrRecBuffer3 = recorderBuffer3;
                ptrRecBuffer4 = recorderBuffer4;
                DMCIFlags.Recorder = 0;
            }   
        }
    }
}

/******************************************************************************
 * initRTDMD
 *
 * The initRTDMD inititializes all the variables used by DMCI.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/ 
VOID initRTDMD(VOID)
{
    SHORT i;  
    snapCount = 0;
    snapShotDelayCnt = 0;
    snapShotDelay = SNAPDELAY;
    for(i = 0; i < DATA_BUFFER_SIZE; i++) /* Clear DMCI data buffer */
        recorderBuffer1[i] = recorderBuffer2[i] = recorderBuffer3[i] = recorderBuffer4[i] = 0;

    RTDM_Start();  	/* Configure the UART module used by RTDM */
}
