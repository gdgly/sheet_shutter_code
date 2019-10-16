/********************************************************************************
* FileName: Application.h
* Description:  
* This header file contains the decleration of all the attributes and 
* services for Application.c file. It handles commands from the Control board acting as 
* an interface between communication module and ramp generator, Drive Fault checking, 
* System Counters, EEPROM and Anomaly history, updating Drive status and Fault registers. 
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
 *  22/04/2014            iGate          Initial Creation                                                               
*****************************************************************************/
#ifndef APPLICATION_H
#define APPLICATION_H
#include "./Common/Typedefs/Typedefs.h"

#define ZERO_TOLERANCE 0 
#define MS_TO_TIMER_TICKS(value) value // TBD - conversion now not required as we have a 1ms tick timer 

#define SET_STATUS_READY(value) 				((value & 0xFFF0) | 0x0001)
#define SET_STATUS_POWERON_CALIB(value) 		((value & 0xFFF0) | 0x0002)
#define SET_STATUS_RUNTIME_CALIB(value) 		((value & 0xFFF0) | 0x0004)
#define SET_STATUS_INSTALLATION(value) 			((value & 0xFFF0) | 0x0008)

#define SET_STATUS_SHUTTER_AT_UPPER_LIMIT(value) 		((value & 0xF83F) | 0x0040)
#define SET_STATUS_SHUTTER_AT_APERTURE_HEIGHT(value) 	((value & 0xF83F) | 0x0080)
#define SET_STATUS_SHUTTER_AT_LOWER_LIMIT(value) 		((value & 0xF83F) | 0x0100)
#define SET_UPPER_DECEL_POINT_REACHED(value)			((value & 0xF83F) | 0x0200)
#define SET_PHOTO_ELEC_IGNORE_REACHED(value)			((value & 0xF83F) | 0x0400)


#define SET_STATUS_SHUTTER_STOPPED(value) 			((value & 0x87FF) | 0x0800)
#define SET_STATUS_SHUTTER_MOVING_UP(value) 		((value & 0x87FF) | 0x1000) 
#define SET_STATUS_SHUTTER_MOVING_DOWN(value) 		((value & 0x87FF) | 0x2000) 
#define SET_STATUS_SHTTR_MOV_DOWN_IGN_SENS(value) 	((value & 0x87FF) | 0x6000) // both flags mov-dwn and mov-dwn-ign-sens are set 



// Application module externals 
// Up and Down aperture command received flag added separetly to avoid any chance of micounting of operation count - YG - NOV 15
extern BOOL bUpApertureCmdRecd;
extern BOOL bDownApertureCmdRecd;


// TBD - to link these to the hardware 
EXTERN BOOL emergencySensorTrigrd; 	// sensor input - TRUE - Emergency Stop sensor ON 
EXTERN BOOL microSwSensorTrigrd; 		// sensor input - TRUE - Micro Switch sensor ON 
EXTERN BOOL airSwitchTrigrd; 			// sensor input - TRUE - Air Switch sensor ON 
EXTERN BOOL wrapAroundSensor; 			// sensor input - TRUE - Wrap around sensor ON 
EXTERN BOOL photoElecObsSensTrigrd; 	// sensor input - TRUE - Photoelec Obs sensor ON 
EXTERN BOOL tempSensTrigrd; 			// sensor input - TRUE - Temperature sensor ON 
EXTERN BOOL fourPtLmtSwtchDetected;
EXTERN BOOL originSensorDetected;       
EXTERN BOOL powerFailSensorDetected;
EXTERN BOOL OvercurrentfaultTrigrd;
EXTERN UINT16 errorUARTTimeCount; 
EXTERN BOOL sysInitCompleted;

/* Enumaration for power up calibration and installation verification  */
typedef enum powerUpCalibState
{
	CALIB_STATE_START,	//	Added on 03 FEB 2015 to implement user control on power up calibration
    CALIB_SEARCH_ORG_UP_DIR,
    CALIB_SEARCH_ORG_DN_DIR,
    CALIB_MOVE_UP_50MM,
    CALIB_MOVE_DN_50MM,
    CALIB_MOVE_TO_UP_LIMIT,
    CALIB_MOVE_TO_DN_LIMIT,
    CALIB_MOVE_TO_APP_LIMIT,
    CALIB_STATE_END
}powerUpCalibState_en;

typedef enum installationState
{
    INSTALL_A100,
    INSTALL_A101,
    INSTALL_A102,
    INSTALL_VERIFY,
    INSTALL_SEARCH_ORG,
    INSTALL_MOVE_UP_50MM,
    INSTALL_MOVE_DN_50MM,
    INSTALL_MOVE_TO_UP_LIMIT,
    INSTALL_MOVE_TO_DN_LIMIT,
    INSTALL_SUCCESSFUL,  //bug_NO.35
    INSTALL_COMPLETE,
    INSTALL_RESTART,   
    INSTALL_STATE_END      
}installationState_en;

typedef struct _powerUpCalib
{
    SHORT currentState;
    SHORT osToggle;
    SHORT targetPosition;  
    SHORT currentPosition;
    SHORT operationCnt;
    SHORT apertureCalib;
}powerUpCalib_t;

typedef struct _shutterInstall
{
    SHORT currentState;
    SHORT osToggle;
    SHORT targetPosition;  
    SHORT currentPosition;
    SHORT operationCnt;
    SHORT enterCmdRcvd;
}shutterInstall_t;

EXTERN powerUpCalib_t powerUpCalib;
EXTERN shutterInstall_t shutterInstall;

EXTERN UINT16 TIME_CMD_open_shutter;
EXTERN UINT16 TIME_CMD_close_shutter;
EXTERN UINT8  FLAG_CMD_open_shutter;
EXTERN UINT8  CMD_open_shutter;
EXTERN UINT8  FLAG_StartApertureCorrection ;   //bug_No.12

/* This function initializes all variables required by the Application */
VOID initApplication(VOID); 

/* This function implements the task loop for the Drive Application */ 
VOID application(VOID); 

/* Interface used by all modules to set Drive Fault register */
unsigned char setDriveFault(unsigned char faultCategory, unsigned char faultType); 

/* Interface used by all modules to set Drive Status register */
unsigned char setDriveStatus(unsigned char status, unsigned char faultCategory); 

/* Function to handle run-time calibration */
//UINT8 runtimeCalibration(VOID); 

/* Function to perform fault checks during runtime calibration */
//BOOL runtimeCalibFaultChecks(BOOL swing); // 0 - UpSwing, 1 - DownSwing  

/* This function implements system counters - operation count and aperture frequency */ 
VOID updateSytemCounters(VOID); 

/* This function updates drive fault flags, this is mainly required for flag reset operations 
	as the set operations are handled from all over the code - sensor monitors, ramp generator etc */
VOID updateDriveFaultFlags(VOID); 
VOID updateFaultRecoverFlag(VOID);

/* This function updates drive status flags */
VOID updateDriveStatusFlags(VOID); 

/* Method to get system tick counter */
UINT32 getSystemTick(VOID); 

VOID checkShutterPosition(VOID);
VOID powerUpCalibration(VOID);
VOID startInstallation(VOID);

VOID checkShutterInstallation(VOID);
VOID shutterInstallation(VOID);

//VOID HandlePowerupCalibration(VOID);

#endif /* APPLICATION_H */

