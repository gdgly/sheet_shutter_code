
#include "eeprom.h"

//Bead shutter default parameters
CONST _EEPDriveMotorCtrlBlock uEEP_BeadDriveMotorCtrlBlockDefault = 
{
    .stEEPDriveMotorCtrlBlock = 
    {
        0,      //UINT8 decelByPhotoElecBlckingLim_A011;
		0,      //UINT8 waitForStoppage_A011;
		//	Default parameter values changed as per request from Bx - Dec 2015
		450,//200,    //UINT16 riseChangeGearPos1_A103;     //bug_NO.45
		250,//100,    //UINT16 riseChangeGearPos2_A104; 
		150,//0,      //UINT16 riseChangeGearPos3_A105;		
		250,//500,    //UINT16 fallChangeGearPos1_A106; 
		150,//600,    //UINT16 fallChangeGearPos2_A107; 
		100,//0,      //UINT16 fallChangeGearPos3_A108;         
		20,//50,      //UINT8 	shtrRevOperMinLimit_A110;20160915         
		0,      //UINT16 	PWMFreqMotorCtrl_A500; 
		0,      //UINT8 	startupDutyCycle_A501; 
		0,      //UINT16 	maxStartupTimeLim_A504; 
		0,      //UINT16	startupSectorConst_A505; 
        
		0,      //UINT16  timer2PreScalar_A506; 
		0,      //UINT16	timer2RPM_A507; 
		0,      //UINT16	timer2Min_A508; 
		0,      //UINT16	timer2Max_A509; 
		0,      //UINT16	minDutyCycle_A510; 
		0,      //UINT8	breakEnabled_A511; 
		100,      //UINT16	speed_PI_KP_A512; 
		70,      //UINT8	speed_PI_KI_A513; 
		100,      //UINT16	current_PI_KP_A514; //STT 20160803
		40,      //UINT8	current_PI_KI_A515;    //STT 20160803    
		0,      //UINT16	outputFreq_A516;
		9,     //UINT8	inchSpeed_A517; 
		0,      //UINT8	driveStatus_A518; 
		0,      //UINT8	currentError_A519; 

		500,      //UINT32	accel1Up_A520; 
        600,      //UINT32	decel1Up_A521;
		1750,   //1900,CHANGED BY AOYAGI_ST_20160415   //UINT16  s1Up_A522;   //bug_NO.44
		400,    //UINT16  s2Up_A523; 
		200,      //UINT16  s3Up_A524; 
		3,      //UINT8	upStepCount_A525; 
        
		0,      //UINT32	accel1Down_A526; 
		0,      //UINT32	decel1Down_A527;         
		900,    //1000,CHANGED BY AOYAGI_ST_20160415   //UINT16  s1Down_A528; 
		400,    //UINT16  s2Down_A529; 
		200,      //UINT16  s3Down_A530; 
		3,      //UINT8 	downStepCount_A531; 
		0,      //UINT8 	shutterLength_A536; 
		0,      //UINT8 	shutterType_A537;         
		0,      //UINT16  OVLim_A538; 
		0,      //UINT16  OILim_A539; 
		0,      //UINT16  OSLim_A540;         
		0,      //UINT16  OFLim_A541; 
		0,      //UINT8 	thermalProtect_A542;         
		0,      //UINT16	torqueConst_A543; 
		0,      //UINT16	backEMFConst_A544; 
		0,      //UINT16	speedConst_A545; 
		2900,   //UINT16	ratedSpeed_A546; 
		0,      //UINT32	ratedCurrent_A547; 
		0,     //UINT8	polePairs_A548; 
		15, 		//UINT8 jogSpeed_A551; 
		0,      //UINT16 	blockCRC;
    }
};

//M1 shutter default parameters
CONST _EEPDriveMotorCtrlBlock uEEP_M1DriveMotorCtrlBlockDefault = 
{
    .stEEPDriveMotorCtrlBlock = 
    {
        0,      //UINT8 decelByPhotoElecBlckingLim_A011;
		0,      //UINT8 waitForStoppage_A011; 
		350,    //UINT16 riseChangeGearPos1_A103; 
		250,    //UINT16 riseChangeGearPos2_A104; 
		150,      //UINT16 riseChangeGearPos3_A105;		
		400,    //UINT16 fallChangeGearPos1_A106;   
		250,    //UINT16 fallChangeGearPos2_A107; 
		150,      //UINT16 fallChangeGearPos3_A108;         
		50,      //UINT8 	shtrRevOperMinLimit_A110;   
        0,      //UINT16 	PWMFreqMotorCtrl_A500; 
		0,      //UINT8 	startupDutyCycle_A501; 
		0,      //UINT16 	maxStartupTimeLim_A504; 
		0,      //UINT16	startupSectorConst_A505; 
        
		0,      //UINT16  timer2PreScalar_A506; 
		0,      //UINT16	timer2RPM_A507; 
		0,      //UINT16	timer2Min_A508; 
		0,      //UINT16	timer2Max_A509; 
		0,      //UINT16	minDutyCycle_A510; 
		0,      //UINT8	breakEnabled_A511; 
		100,      //UINT16	CW_PI_KP_A512;   //STT 20160803  //bug_NO.65
		70,      //UINT8	CW_PI_KI_A513;   //STT 20160803
		100,      //UINT16	CCW_PI_KP_A514;   //STT 20160803
		40,      //UINT8	CCW_PI_KI_A515;   //STT 20160803     
		0,      //UINT16	outputFreq_A516;
		9,     //UINT8	inchSpeed_A517; 
		0,      //UINT8	driveStatus_A518; 
		0,      //UINT8	currentError_A519; 
        
		500,      //UINT32	accel1Up_A520; 
		600,      //UINT32	decel1Up_A521;         
		1450,//1600,   //UINT16  s1Up_A522; 
		400,    //UINT16  s2Up_A523; 
		200,      //UINT16  s3Up_A524; 
		3,      //UINT8	upStepCount_A525; 
		0,      //UINT32	accel1Down_A526; 
		0,      //UINT32	decel1Down_A527;         
		1450,//1000,   //UINT16  s1Down_A528; 
		400,    //UINT16  s2Down_A529; 
		200,      //UINT16  s3Down_A530; 
		3,      //UINT8 	downStepCount_A531; 
		0,      //UINT8 	shutterLength_A536; 
		1,      //UINT8 	shutterType_A537;    //bug_NO.62     
		0,      //UINT16  OVLim_A538; 
		0,      //UINT16  OILim_A539; 
		0,      //UINT16  OSLim_A540;   
        0,      //UINT16  OFLim_A541; 
		0,      //UINT8 	thermalProtect_A542;  
        0,      //UINT16	torqueConst_A543; 
		0,      //UINT16	backEMFConst_A544; 
		0,      //UINT16	speedConst_A545; 
		2900,   //UINT16	ratedSpeed_A546; 
		0,      //UINT32	ratedCurrent_A547; 
		0,     //UINT8	polePairs_A548; 
		15, 		//UINT8 jogSpeed_A551; 
		0,      //UINT16 	blockCRC;
    }
};

CONST _EEPDriveCommonBlock uDriveCommonBlockEEPDefault = 
{
    .stEEPDriveCommonBlock = 
    {
        0,      //UINT16 	upperStoppingPos_A100; 
		0,    //UINT16 	lowerStoppingPos_A101; 
		500,      //UINT16 	photoElecPosMonitor_A102; 
		126,      //UINT16 	originSensorPosMonitor_A128; 
		0,      //INT8 	originSensorDrift_A637;  // signed value
		0,//678,      //UINT16 	currentValueMonitor_A129; //20160810  aoyagi 
		100,    //UINT16 	apertureHeightPos_A130; 
		0,      //UINT8 	apertureModeEnable_A131; 		
		0,      //UINT16 	blockCRC; 
    }
};

CONST _EEPDriveApplBlock uDriveApplBlockEEPDefault = 
{
    .stEEPDriveApplBlock = 
    {
        0,      //UINT8 	snowModePhotoelec_A008; 
		0,      //UINT8 	initialValSetting_A021; 
		500,      //UINT16 	maintenanceCountLimit_A025; // to avoid error flag getting set 
		0,      //UINT16 	maintenanceCountValue_A636; 	
		0,      //UINT8 	microSensorCounter_A080; 
		0,      //UINT8 	microSensorCountReset_A081; 
		50,    //UINT8 	overrunProtection_A112; 
		0,      //UINT8 	resetToDefaultValues_A120; 
		0,      //UINT8 	powerUpCalib_A125; 
		//	Default value of A126 changed to 10 as its range is 10 to 9999 - Jan 2016
		500,      //UINT8 	correctedFreqAperture_A126; //STT 20160803     //bug_NO.66
		0,      //UINT8 	autoCorrectionEnabled_A127; 
		0,      //UINT8 	driveFWVersion_A549;  //bug_NO.64
		001,      //UINT8 	driveHWVersion_A550; 
		0,      //UINT32 	operationCount_A600; 
		10,     //UINT8 	microSensorLimValue_A603;
		0,      //UINT32 	apertureHeightOperCount_A604; 		
		0, 		//UINT8	operationCounterReset_A020; 
		123, 		//UINT32	operationCount_A028; 
		0,      //UINT16 blockCRC; 
    }
};

CONST _DriveStatus_A605 uDriveStatusDefault = 
{
    .bits = 
    {
                //// drive system states 
        0,      //UINT16 driveReady				: 1; // Running/ ON actually 
        0,      //UINT16 drivePowerOnCalibration	: 1; // this is doing same as runtime calibration, this flag can be removed 
        0,      //UINT16 driveRuntimeCalibration	: 1;
        0,      //UINT16 driveInstallation		: 1; 
        
                //// drive fault status - can be Fault + Fault unrecoverable at same time 
        0,      //UINT16 driveFault				: 1;
        0,      //UINT16 driveFaultUnrecoverable	: 1;
        
                //// drive position status 
        0,      //UINT16 shutterUpperLimit		: 1;
        0,      //UINT16 shutterApertureHeight	: 1;
        0,      //UINT16 shutterLowerLimit		: 1;
        
                //// drive movement status 
        0,      //UINT16 shutterStopped			: 1;
        0,      //UINT16 shutterMovingUp			: 1;
        0,      //UINT16 shutterMovingDown		: 1;
        0,      //UINT16 shuttrMovDwnIgnoreSens	: 1;
        0,      //UINT16 unused					: 3;
    }
};

CONST _DriveInstallationStatus_A606 uDriveInstallationStatusDefault = 
{
    .bits = 
    {
        0,      //UINT8 installA100              : 1;
        0,      //UINT8 installA101              : 1;
        0,      //UINT8 installA102              : 1;
        0,      //UINT8 installationValidation   : 1; // this is doing same as runtime calibration, this flag can be removed 
        0,      //UINT8 installationSuccess      : 1; // success can be flagged on completion of A102, then runtime calibration 
                //// can be taken up separately 
        0,      //UINT8 installationFailed       : 1;
        0,      //A130
        0,      //UINT8 unused                   : 1;
    }
};

CONST _DriveFault_A607 uDriveFaultDefault = 
{
    .bits = 
    {
        0,      //UINT8 driveCommunicationFault   : 1;
        0,      //UINT8 driveMotorFault           : 1;
        0,      //UINT8 driveApplicationFault     : 1;
        0,      //UINT8 driveProcessorFault       : 1;
        0,      //UINT8 unused                    : 1;
    }
};

CONST _DriveCommunicationFault_A608 uDriveCommunicationFaultDefault = 
{
    .bits = 
    {
        0,      //UINT8 crcError                  : 1;
        0,      //UINT8 commandFrameError		  : 1; 
        0,      //UINT8 uartError                 : 1;
        0,      //UINT8 unused                    : 5;
    }
};

CONST _DriveMotorFault_A609 uDriveMotorFaultDefault = 
{
    .bits = 
    {
        0,      //UINT16 motorOpenphase           : 1;
        0,      //UINT16 motorDCbusOverVoltage    : 1;
        0,      //UINT16 motorOverCurrent         : 1;
        0,      //UINT16 motorExceedingTorque     : 1;
        0,      //UINT16 motorPFCshutdown         : 1;
        0,      //UINT16 motorStall               : 1;
        0,      //UINT16 motorOverheat            : 1;
        0,      //UINT16 unused                   : 9;
    }
};

CONST _DriveApplicationFault_A610 uDriveApplicationFaultDefault = 
{
    .bits = 
    {
        0,      //UINT32 peObstacle               : 1;
        0,      //UINT32 lowInputVoltage          : 1;
        0,      //UINT32 highInputVoltage         : 1;
        0,      //UINT32 hallSensor               : 1;
        0,      //UINT32 wraparound               : 1; // these 3 bits can be clubbed into 1 as any variant will have only one of these sensors 
        0,      //UINT32 microSwitch              : 1; // these 3 bits can be clubbed into 1 as any variant will have only one of these sensors
        0,      //UINT32 airSwitch                : 1; // these 3 bits can be clubbed into 1 as any variant will have only one of these sensors
        0,      //UINT32 emergencyStop            : 1;
        0,      //UINT32 osNotDetectUP            : 1;  // origin sensor not detected while rolling up
        0,      //UINT32 osNotDetectDown          : 1;  // origin sensor not detected while rolling down
        0,      //UINT32 osDetectOnUp             : 1;  // origin Sensor becomes ON while shutter is between the upper limit and origin sensor level. (expected is OFF)
        0,      //UINT32 osDetectOnDown           : 1;  // origin Sensor becomes OFF while shutter is between origin sensor level and lower limit (expected is ON)
        0,      //UINT32 osFailValidation         : 1;  // In case of teaching mode, error from limit calculation is more than or equal to 500mm
        0,      //UINT32 driveCalibrationFailed   : 1;
        0,      //UINT32 microSwitchSensorLimit   : 1;   //     A080
        0,      //UINT32 maintenanceCountOverflow : 1;   //     A025
        0,      //UINT32 igbtOverTemperature 	  : 1;   //     IGBT overtemperature
        0,      //UINT32 unused                   : 15;
    }
};

CONST _DriveProcessorfault_A611 uDriveProcessorFaultDefault = 
{
    .bits = 
    {
        0,      //UINT16 flashImageCRC           : 1;
        0,      //UINT16 eepromParameterDbCRC    : 1;
        0,      //UINT16 ramIntegrity            : 1;
        0,      //UINT16 eepromProgramming       : 1;
        0,      //UINT16 eepromErase             : 1;
        0,      //UINT16 unsued                  : 11;
    }
};

CONST _DriveAnomalyHistory_A616 uDriveAnamolyHistoryDefault = 
{
    .bytes = 
    {
        0,      //UINT8 drive_anomaly_history_1;  // indicates oldest error           
        0,      //UINT8 drive_anomaly_history_2;
        0,      //UINT8 drive_anomaly_history_3;
        0,      //UINT8 drive_anomaly_history_4;
        0,      //UINT8 drive_anomaly_history_5;
        0,      //UINT8 drive_anomaly_history_6;
        0,      //UINT8 drive_anomaly_history_7;
        0,      //UINT8 drive_anomaly_history_8;
        0,      //UINT8 drive_anomaly_history_9;
        0,      //UINT8 drive_anomaly_history_10; // indicates latest error 
    }
};

CONST _EEPDriveStatFaultBlock uDriveStatusFaultBlockEEPDefault = 
{
  .stEEPDriveStatFaultBlock = 
  {
                  //// Drive Status 
      {.bits = {0}},        //_DriveStatus_A605 uDriveStatus; 
      {.bits = {0}},        //_DriveInstallationStatus_A606 uDriveInstallationStatus; 
      
                  //// Drive Faults 
      {.bits = {0}},        //_DriveFault_A607 uDriveFault; 
      {.bits = {0}},        //_DriveCommunicationFault_A608 uDriveCommunicationFault; 
      {.bits = {0}},        //_DriveMotorFault_A609 uDriveMotorFault; 
      {.bits = {0}},        //_DriveApplicationFault_A610 uDriveApplicationFault; 
      {.bits = {0}},        //_DriveProcessorfault_A611 uDriveProcessorFault; 
      {.bytes = {
			        100,      //UINT8 drive_anomaly_history_1;  // indicates oldest error           
			        133,      //UINT8 drive_anomaly_history_2;
			        0,      //UINT8 drive_anomaly_history_3;
			        0,      //UINT8 drive_anomaly_history_4;
			        0,      //UINT8 drive_anomaly_history_5;
			        0,      //UINT8 drive_anomaly_history_6;
			        0,      //UINT8 drive_anomaly_history_7;
			        123,      //UINT8 drive_anomaly_history_8;
			        114,      //UINT8 drive_anomaly_history_9;
			        102,      //UINT8 drive_anomaly_history_10; // indicates latest error 
    			}
		},        //_DriveAnomalyHistory_A616 uDriveAnamolyHistory; 

	  64999, //UINT16 crcCheckFailedCountA803;
	  64920, //UINT16 commBufferOverflowCountA807;  
      
      0,        //UINT16 blockCRC; 
  }
};
