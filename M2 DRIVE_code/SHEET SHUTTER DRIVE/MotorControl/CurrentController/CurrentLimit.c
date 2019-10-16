#include <p33Exxxx.h>
#include "CurrentLimit.h"
#include "CurrentController.h"
#include "./MotorControl/PIController/pi.h"
#include "./DMCI/RtdmInterface.h"
#include "./Common/Extern/Extern.h"
#include "./Application/RampGenerator/RampGenerator.h"
#include "./Middleware/ParameterDatabase/eeprom.h"

/* current limit PI parameters */
#define P_CURRENT_LIMIT_PI 15000
#define I_CURRENT_LIMIT_PI 1500
#define C_CURRENT_LIMIT_PI 0x7FFF
#define MAX_CURRENT_LIMIT_PI 31128   //95% of max value ie 32767

#define TARGET_CURRENT_LIMIT   15000		//10000 //7000 //15000//12000//7000   

#define SUSTAINED_OVER_CURRENT_TIMEOUT      50//10//100 //100ms
#define SUSTAINED_OVER_CURRENT_VALUE        20000 //1A = 1000mA

/* PI configuration structure */
tPIParm currentLimitPIparms;
WORD refCurrentLimit;
WORD feedbackCurrent;
SHORT currentLimitClamp;

WORD sustainedOcTimer;
BOOL sustainedOcFlg = FALSE;
SHORT outputDecRate;

VOID initCurrentLimitPI(VOID)
{
    sustainedOcTimer = 0;
    sustainedOcFlg = FALSE;
    outputDecRate = 0;
    
    refCurrentLimit = TARGET_CURRENT_LIMIT;
    feedbackCurrent = 0;
    currentLimitClamp = MAX_CURRENT_LIMIT_PI;
    
    initPiNew(&currentLimitPIparms,P_CURRENT_LIMIT_PI,I_CURRENT_LIMIT_PI,C_CURRENT_LIMIT_PI,MAX_CURRENT_LIMIT_PI,0,0);
}

VOID runCurrentLimitPI(VOID) 
{   
    //Execute current limit only when sustained overcurrent and PWM coasting not required
    if(!sustainedOcFlg && !pwmCostingReq)
    {
        currentLimitPIparms.qInRef = refCurrentLimit;
        currentLimitPIparms.qInMeas = feedbackCurrent;
        calcPiNew(&currentLimitPIparms);
        //set current limit PI output as clamp for speed and current PI
        currentLimitClamp = currentLimitPIparms.qOut;
    }
    
    //Check PWM coasting is required or not
    checkPwmCoastingRequired();
    
    //Check sustained overcurrent occured or not
    checkSustainedOvercurrent();
}

VOID checkSustainedOvercurrent(VOID)
{
    //check measured current is more than allowed current
    if(sustainedOcFlg == FALSE)
    {   
        //check sustained over current
        if(feedbackCurrent > SUSTAINED_OVER_CURRENT_VALUE)
        {
            if(sustainedOcTimer < SUSTAINED_OVER_CURRENT_TIMEOUT)
            {
                sustainedOcTimer++;
            }
            else
            {
                //set the condition for sustained overcurrent flag
                sustainedOcFlg = TRUE;
                //set the clamp limit for all outputs
                currentLimitClamp = controlOutput;
                outputDecRate = __builtin_divud(currentLimitClamp, PWM_COASTING_TIME);
                //set the overcurrent flag to send to control board
            }
        }
        else
        {
            sustainedOcTimer = 0;
            sustainedOcFlg = FALSE;
        }
    }
    else
    {
        //if sustained overcurrent occured then reduce output and stop the motor
        //if output is zero then stop the motor directly
        if(currentLimitClamp > 0)
        {
            currentLimitClamp -= outputDecRate;
            if(currentLimitClamp < 0)
                currentLimitClamp = 0;
//            if(feedbackCurrent <= SUSTAINED_OVER_CURRENT_VALUE)
//            {
//                sustainedOcTimer = 0;
//                sustainedOcFlg = FALSE;
//            }
        }
        else
        {
            currentLimitClamp = 0;
            forceStopShutter();
            uDriveStatusFaultBlockEEP.stEEPDriveStatFaultBlock.uDriveMotorFault.bits.motorSusOC = TRUE;
        }
    }
}

