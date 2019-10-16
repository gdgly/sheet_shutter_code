/*********************************************************************************
* FileName: Main.c
* Description:
* This source file contains the definition of all the functions for Main.
* It configures the CPU and initializes all the peripherals required in
* application.
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
 *  22/04/2014			  iGate 		 Updated - Added Logic Solver
*****************************************************************************/
#include <p33Exxxx.h>
#include "math.h"
#include "./Common/UserDefinition/Userdef.h"
#include "./Drivers/ADC/ADC.h"
#include "./Drivers/InputCapture/InputCapture.h"
#include "./Drivers/Timer/Timer.h"
#include "./Drivers/PWM/MCPWM.h"
#include "./Drivers/GPIO/GPIO.h"
#include "./DMCI/RTDMUSER.h"
#include "./DMCI/RTDM.h"
#include "./DMCI/RtdmInterface.h"
#include "./Application/RampGenerator/RampGenerator.h"
#include "./Application/Application.h"
#include "./Common/Typedefs/Typedefs.h"
#include "./MotorControl/CurrentController/CurrentController.h"
#include "./Middleware/ParameterDatabase/eeprom.h"
#include "./Middleware/ParameterDatabase/spieeprom.h"
#include "./Middleware/ParameterDatabase/spi.h"
#include "./Common/Delay/Delay.h"
#include "./Common/Extern/Extern.h"
#include "CommandHandler.h"

/******************************************************************************/
/* Configuration bits                                                         */
/******************************************************************************/
_FPOR(ALTI2C1_OFF & ALTI2C2_OFF & BOREN_OFF);
_FWDT(WDTPOST_PS1024 & WDTPRE_PR32 & PLLKEN_ON & WINDIS_OFF & FWDTEN_ON);
_FOSCSEL(FNOSC_FRC & IESO_OFF & PWMLOCK_OFF);
_FGS(GWRP_OFF & GCP_OFF);
_FICD(ICS_PGD2 & JTAGEN_OFF);	/* PGD3 for 28pin 	PGD2 for 44pin */
_FOSC(FCKSM_CSECMD & POSCMD_XT);		/* XT W/PLL */


//	Added for implementation of power fail functionality on DC Bus for version 4 board- RN- NOV 2015
BYTE gucPowerFailFlag = 0;
BYTE gucPowerRestoredFlag = 0;
UINT8 MotorTypeStatus = 0;
UINT8 CurrentMotorType = 0;
UINT8 PreMotorType = 0;
UINT8 s = 0;
//UINT16 DutyCycleSet = 15000;
//UINT16 DutyCycle = 3600;

#define     Reset()     {__asm__   volatile ("reset");}
/******************************************************************************
 * main
 *
 * The main function gives starting point for application execution. It configures
 * the CPU and initializes all the peripherals required in application.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
INT main(VOID)
{
    /*********************************************************************
    * The settings below set up the oscillator and PLL for 70 MIPS as
    * follows:
    *             Crystal Frequency  * (DIVISOR+2)
    *  Fcy =     ---------------------------------
    *               PLLPOST * (PRESCLR+2) * 4
	*  Crystal  = 8 MHz
	*  Fosc		= 140 MHz
	*  Fcy		= 70 MIPS
    *********************************************************************/
	PLLFBD = 68;		        /* M=70 */
	CLKDIVbits.PLLPOST = 0;		/* N1=2 */
	CLKDIVbits.PLLPRE = 0;		/* N2=2 */
	__builtin_write_OSCCONH(0x03);
	__builtin_write_OSCCONL(0x01);
	while(OSCCONbits.COSC != 0b011);
	while(OSCCONbits.LOCK != 1);/* Wait for PLL to lock */

	CORCONbits.SATA = 0;
    CORCONbits.IF = 0;
    //Add powerup delay
//    delayMs(1000);
    delayMs(250);
	ClrWdt();   // clear the WDT to inhibit the device reset
    delayMs(250);
	ClrWdt();   // clear the WDT to inhibit the device reset
    delayMs(250);
	ClrWdt();   // clear the WDT to inhibit the device reset
    delayMs(250);
	ClrWdt();   // clear the WDT to inhibit the device reset
    
    initGPIO(); /* Initialize all the I/O's required in application */
    
    MotorTypeStatus = PORTAbits.RA12;
    
	initADC();		/* Initialize ADC to be signed fractional */
	//Added for ADC2- RN- NOV2015
    if(MotorTypeStatus == 1)
    {
        initADC2();		/* Initialize ADC to be signed fractional */
        CurrentMotorType = MOTOR_750W;
        PreMotorType = CurrentMotorType;
    }
    else
    {
        initADC2For1500W();
        CurrentMotorType = MOTOR_1500W;
        PreMotorType = CurrentMotorType;
    }
	initInputCapture();		/* Initialize Hall sensor inputs ISRs	 */
    SPI1INTInit();
	initTMR1();		/* Initialize TMR1 1 ms periodic ISR for speed controller */
    initTMR2();		/* Initialize TMR2 1 ms periodic ISR for current controller */
    if(MotorTypeStatus == 1)
    {
        initTMR3();		/* Initialize TMR3 for timebase of capture */
        initTMR5();
    }
    else
    {
        initTMR3For1500W();
    }
    //initTMR4();     // Initialize TMR4 for Ramp Generator
			// Initialize TMR5 for ADC2- DC bus volatage monitoring - Dec 2015
	ClrWdt();   // clear the WDT to inhibit the device reset
    //initTMR6();     // Timer6 is used for application software
    initTMR7();
/*#ifdef PROGRAMMABLE_DEBOUNCE
	initTMR8();
#endif*/
	initMCPWM();
    initTMR8();
    initTMR9();

	//	No need to initialize RTDM as we are not going to use DMCI for shutter application
    //initRTDMD();

    //initSensorList();
	//initApplication();
    initCommandHandler(); 
	//initRampGenerator(); /* Initialize ramp generator variables */
    measureADCOffset();
	ClrWdt();   // clear the WDT to inhibit the device reset
	//startRampGenerator();
    //T6CONbits.TON = 1; //start main application timer
#ifdef PROGRAMMABLE_DEBOUNCE
	//T8CONbits.TON = 1; //start debounce timer
#endif

	// For testing only (Added on 27 Jan 2015 to generate latch pulse on MCU_LATCH_CTRL pin)
	// to unlatch motor_stall fault
	/**************************************************/
	PORTGbits.RG8 = 0;
	delayUs(1);
	PORTGbits.RG8 = 1;			//	1uS pulse generated on MCU_LATCH_CTRL pin
	/**************************************************/

	// For testing only (Added on 27 Jan 2015 to enable fault input)
	// These lines were disabled in initMCPWM() function called above
	// in MCPWM.c file.
	/**************************************************/
	PWMCON1bits.FLTIEN = 1; /* Enable fault interrupt */
	FCLCON1 = 0x002C;
	FCLCON2 = 0x002C;
	FCLCON3 = 0x002C;

	
	AD2CON1bits.ADON = 1;
    if(MotorTypeStatus == 1)
    {
        T5CONbits.TON = 1;
    }
    else
    {
        IEC0bits.T3IE = 1;
        T3CONbits.TON = 1;
    }
//    T9CONbits.TON = 1;
//    T8CONbits.TON = 1;
    fanON;
    //startMotorCW();
	for(;;)
	{
        ClrWdt();   // clear the WDT to inhibit the device reset
        //runTestCode();
        MotorTypeStatus = PORTAbits.RA12;
        if(MotorTypeStatus == 1)
        {
            CurrentMotorType = MOTOR_750W;
        }
        else
        {
            CurrentMotorType = MOTOR_1500W;
        }  
        
        if((CurrentMotorType != PreMotorType) && (!flags.motorRunning))
        {
            ClrWdt();
            PreMotorType = CurrentMotorType;
            Reset();
        }
        application();
        
		ClrWdt();   // clear the WDT to inhibit the device reset
		
    }
    return 0; /* Return without error */
}
