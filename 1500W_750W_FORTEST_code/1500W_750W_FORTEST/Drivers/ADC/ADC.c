/*********************************************************************************
* FileName: ADC.c
* Description:
* This source file contains the definition of all the functions for ADC.
* It initializes all the ADC channels required in application.
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
#include "ADC.h"

/******************************************************************************
 * initADC
 *
 * This function initializes all the ADC channels required by application.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initADC(VOID)
{
	AD1CON1bits.FORM = 0;//3; /* Signed Fractional */	
	AD1CON1bits.SSRC = 0;//3; /* PWM Generator 1 primary trigger compare ends sampling and starts conversion */
	AD1CON1bits.SSRCG = 1;//0; /* Sample Trigger Source Group bit */	
    AD1CON1bits.SIMSAM = 1;
	AD1CON1bits.ASAM = 1; /* Sampling begins immediately after last conversion */	
	AD1CON1bits.AD12B = 0; /* Select 10-bit, 4 channel ADC operation */	
	/* No channel scan for CH0+, Use MUX A, SMPI = 1 per interrupt, Vref = AVdd/AVss */ 
	AD1CON2 = 0x0000;	
	/* Samples CH0, CH1, CH2, CH3 simultaneously (when CHPS = 1x) */
    AD1CON2bits.CHPS = 0;
	/* Set Samples and bit conversion time */
    AD1CON3 = 0x0204;
	/* Disable DMA */    
	AD1CON4 = 0x0000;	
	/* CH1 positive input is CMP0, CH2 positive input is CMP1, CH3 positive input is CMP2 */
	/* No Channels to Scan */
	AD1CSSL = 0x0000;
	//Changed while working on ADC2- RN- NOV2015
	//AD1CHS0 = 14;	// Itotal measurement AN14
	AD1CHS0bits.CH0SA = 14;	// Itotal measurement on AN14
	AD1CHS0bits.CH0NA = 0x0;
    /* ADCSSL: ADC Input Scan Select Register */
    AD1CSSL = 0;    
	/* Reset ADC interrupt flag */
	IFS0bits.AD1IF = 0;	
	/* Disable ADC interrupts, disable this interrupt if the DMA is enabled */	  
	IEC0bits.AD1IE = 1;	
	/* Turn off ADC module */
	AD1CON1bits.ADON = 0;      
    IPC3bits.AD1IP = 4;
}

//Added for ADC2- RN- NOV2015
VOID initADC2(VOID)
{
	AD2CON1bits.FORM = 0;//3; /* Signed Fractional */	
	//	Use Timer 5 as trigger source for ADC2 conversion
	AD2CON1bits.SSRC = 4;//2;//0;//3; /* Timer5 compare ends sampling and starts conversion */
	AD2CON1bits.SSRCG = 0;//1;//0; /* Sample Trigger Source Group bit */	
    AD2CON1bits.SIMSAM = 1;
	AD2CON1bits.ASAM = 1; /* Sampling begins immediately after last conversion */	
	AD2CON1bits.AD12B = 0; /* Select 10-bit, 4 channel ADC operation */	
	/* No channel scan for CH0+, Use MUX A, SMPI = 1 per interrupt, Vref = AVdd/AVss */ 
	AD2CON2 = 0x0000;	
	/* Samples CH0, CH1, CH2, CH3 simultaneously (when CHPS = 1x) */
    AD2CON2bits.CHPS = 0;
	/*Select alternet input selection*/
	//AD1CON2bits.ALTS = 1;
	/* Set Samples and bit conversion time */
    AD2CON3 = 0x4;//0x0204;
	/* Disable DMA */    
	AD2CON4 = 0x0000;	
	/* CH1 positive input is CMP0, CH2 positive input is CMP1, CH3 positive input is CMP2 */
	/* No Channels to Scan */
	AD2CSSL = 0x0000;	
	//AD1CHS0 = 14;	// Itotal measurement AN14
	AD2CHS0bits.CH0SA = 0x02;	// DC Bus measurement on AN2
	AD2CHS0bits.CH0NA = 0x0;
    /* ADCSSL: ADC Input Scan Select Register */
    AD2CSSL = 0;    
	/* Reset ADC interrupt flag */
	IFS1bits.AD2IF = 0;	
	/* Disable ADC interrupts, disable this interrupt if the DMA is enabled */	  
	IEC1bits.AD2IE = 1;	
	/* Turn off ADC module */
	AD2CON1bits.ADON = 0;      
    IPC5bits.AD2IP = 4;
}

VOID initADC2For1500W(VOID)
{
	AD2CON1bits.FORM = 0;//3; /* Signed Fractional */	
	AD2CON1bits.SSRC = 2;//0;//3; /* Timer3 compare ends sampling and starts conversion */
	AD2CON1bits.SSRCG = 0;//1;//0; /* Sample Trigger Source Group bit */	
    AD2CON1bits.SIMSAM = 1;
	AD2CON1bits.ASAM = 1; /* Sampling begins immediately after last conversion */	
	AD2CON1bits.AD12B = 0; /* Select 10-bit, 4 channel ADC operation */	
	/* No channel scan for CH0+, Use MUX A, SMPI = 1 per interrupt, Vref = AVdd/AVss */ 
	AD2CON2 = 0x0000;	
	/* Samples CH0, CH1, CH2, CH3 simultaneously (when CHPS = 1x) */
    AD2CON2bits.CHPS = 0;
	/*Select alternet input selection*/
	//AD1CON2bits.ALTS = 1;
	/* Set Samples and bit conversion time */
    AD2CON3 = 0x4;//0x0204;
	/* Disable DMA */    
	AD2CON4 = 0x0000;	
	/* CH1 positive input is CMP0, CH2 positive input is CMP1, CH3 positive input is CMP2 */
	/* No Channels to Scan */
	AD2CSSL = 0x0000;	
	//AD1CHS0 = 14;	// Itotal measurement AN14
	AD2CHS0bits.CH0SA = 0x02;	// DC Bus measurement on AN2
	AD2CHS0bits.CH0NA = 0x0;
    /* ADCSSL: ADC Input Scan Select Register */
    AD2CSSL = 0;    
	/* Reset ADC interrupt flag */
	IFS1bits.AD2IF = 0;	
	/* Disable ADC interrupts, disable this interrupt if the DMA is enabled */	  
	IEC1bits.AD2IE = 1;	
	/* Turn off ADC module */
	AD2CON1bits.ADON = 0;      
    IPC5bits.AD2IP = 4;
}
