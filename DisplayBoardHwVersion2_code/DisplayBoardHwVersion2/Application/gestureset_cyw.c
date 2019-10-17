/*********************************************************************************
 * FileName: paraminit.c
 * Description: Code for Parameter Initialization screen
 * Version: 0.1D
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
 *  Revision        Date                  Name                      Comments
 *      0.1D    20/06/2014          iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <driverlib/gpio.h>
#include "Application/ustdlib.h"
#include "Middleware/display.h"
#include "grlib/grlib.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "userinterface.h"
#include "intertaskcommunication.h"
#include "Middleware/paramdatabase.h"
#include "sdcardlogs.h"
#include "logger.h"
#include "Middleware/sdcard.h"



#include "Gesture_Sensor/ram_cyw.h"
//#include "Gesture_Sensor/GP2AP054A_cyw.h"
#include "Middleware/eeprom_cyw.h"

uint8_t disp_gesture_flag_cyw = 0;
/****************************************************************************/

#define PARAMNO_INITVAL 21
/****************************************************************************
 *  Global variables for this file:
****************************************************************************/
// Data Structure for Flags for Internal modules
typedef union
{
    uint8_t val;
    struct
    {
        //ParamInitFlags module flag
        uint8_t paramInit       : 1;
        uint8_t bit1            : 1;
        uint8_t bit2            : 1;
        uint8_t bit3            : 1;
        uint8_t bit4            : 1;
        uint8_t bit5            : 1;
        uint8_t bit6            : 1;
        uint8_t bit7            : 1;
    } flags;
} _ParamInitFlags;

// Define Flags for Internal modules
_ParamInitFlags ParamInitFlags;
extern uint8_t startTimer;
extern uint32_t tickCount;


/******************************************************************************
 * FunctionName: gestureInitRunTime
 *
 * Function Description:
 *
 * Function Parameters:
 *
 * Function Returns:

 *
 ********************************************************************************/
uint8_t gestureRunTime(void)
{
    //
    // This function is called periodically
    //

    updateFaultLEDStatus();

    return 0;
}
/******************************************************************************
 * FunctionName: gesturePaint
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/

uint8_t gesturePaint(void)
{
   char test_test[]="ユウコウ";
    //test_test[0] = test_test[0];
	GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);

    //displayText("ENABLE GESTICULATORY", 2, 0, false, false, false, false,false,true);
   // displayText("ACTION?", 2, 16, false, false, false, false, false, true);
	displayText("ヒセッショクセンサヲ", 2, 0, false, false, false, false,false,false);
    displayText("ユウコウニシマスカ?", 2, 16, false, false, false, false, false, false);
    disp_gesture_flag_cyw = menu_gesture_flag_cyw;
    if(disp_gesture_flag_cyw == 0)
    {
     displayText("0 ユウコウ", 2, 32, true,  false, false, true,false, false);
     displayText("1 ムコウ", 2, 48, false, false, false, false, false, false);
   //  displayText("", 20, 32, true, true, false, false, false, false);
   //  displayText("", 20, 48, false, true, false, false, false, false);
    }
    else
    {
   // displayText("", 20, 32, false, true, false, false, false, false);
   // displayText("", 20, 48, true, true, false, false, false, false);
    displayText("0 ユウコウ", 2, 32, false, false, false, false, false, false);
       displayText("1 ムコウ", 2, 48, true,  false, false, true,false, false);
    }


    return 0;
}
/******************************************************************************
 * FunctionName: paramInitMode
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t gestureMode(void)
{
    //
    // This function is called periodically
    //

    //
    // Handle Mode key press
    //
    if(gKeysStatus.bits.Key_Mode_pressed)
    {
        gKeysStatus.bits.Key_Mode_pressed = 0;

        psActiveFunctionalBlock = &gsMenuFunctionalBlock;
        psActiveFunctionalBlock->pfnPaintFirstScreen();
    }

    return 0;
}

/******************************************************************************
 * FunctionName: paramInitEnter
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t gestureEnter(void)
{


    if(gKeysStatus.bits.Key_Enter_pressed)
       {
    	//Clear_dir();
    	gKeysStatus.bits.Key_Enter_pressed = 0;
        menu_gesture_flag_cyw= disp_gesture_flag_cyw ;
        EEPROM_Gesture_Sava_cyw();
        psActiveFunctionalBlock = &gsMenuFunctionalBlock;
              psActiveFunctionalBlock->pfnPaintFirstScreen();
       }
//  gKeysStatus.bits.Key_Enter_pressed = 0;

    return 0;
}


/******************************************************************************
 * FunctionName: gestureUp
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t gestureUp(void)
{
    //
    // This function is called periodically
    //

    //
    // Handle Up key press
    //
    if(gKeysStatus.bits.Key_Up_pressed)
    {
        gKeysStatus.bits.Key_Up_pressed = 0;
        GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);

         //   displayText("ENABLE GESTICULATORY", 2, 0, false, false, false, false,false,true);
        //    displayText("ACTION?", 2, 16, false, false, false, false, false, true);
        displayText("ヒセッショクセンサヲ", 2, 0, false, false, false, false,false,false);
        displayText("ユウコウニシマスカ?", 2, 16, false, false, false, false, false, false);
        if(disp_gesture_flag_cyw==0)
        {
            disp_gesture_flag_cyw = 1;
            displayText("0 ユウコウ", 2, 32, false, false, false, false, false, false);
            displayText("1 ムコウ", 2, 48, true, false, false, true, false, false);
        }
        else
        {
            disp_gesture_flag_cyw = 0;
            displayText("0 ユウコウ", 2, 32, true, false, false, true,false, false);
            displayText("1 ムコウ", 2, 48, false, false, false, false, false, false);
        }
    }

    return 0;
}
/******************************************************************************
 * FunctionName: gestureDown
 *
 * Function Description:
 *
 * Function Parameters: None
 *
 * Function Returns:
 *
 ********************************************************************************/
uint8_t gestureDown(void)
{
    //
    // This function is called periodically
    //

    //
    // Handle Down key press
    //
    if(gKeysStatus.bits.Key_Down_pressed)
    {
        gKeysStatus.bits.Key_Down_pressed = 0;
        GrRectFIllBolymin(0, 126, 0, 63, 0x00, true);

         //   displayText("ENABLE GESTICULATORY", 2, 0, false, false, false, false,false,true);
         //   displayText("ACTION?", 2, 16, false, false, false, false, false, true);
        displayText("ヒセッショクセンサヲ", 2, 0, false, false, false, false,false,false);
            displayText("ユウコウニシマスカ?", 2, 16, false, false, false, false, false, false);
        if(disp_gesture_flag_cyw==0)
               {
                   disp_gesture_flag_cyw = 1;
                   displayText("0 ユウコウ", 2, 32, false, false, false, false, false, false);
                   displayText("1 ムコウ", 2, 48, true, false, false, true, false, false);
               }
               else
               {
                   disp_gesture_flag_cyw = 0;
                   displayText("0 ユウコウ", 2, 32, true,false, false,  true, false, false);
                   displayText("1 ムコウ", 2, 48, false, false, false, false, false, false);
               }
    }

    return 0;
}

/******************************************************************************
 * Define Home screen functional block object
*********************************************************************************/


stInternalFunctions gsGestureFunctionalBlock =
{
    0,
    &gsMenuFunctionalBlock,
    gesturePaint,//?????????
    gestureRunTime,//????LED
    gestureUp,//UP??????
    gestureDown,//DOWN??????
    gestureMode,//MODE??????
    gestureEnter//Enter??????
};
