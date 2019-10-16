/**********************************************************************
 *                                                                     *
 *                        Software License Agreement                   *
 *                                                                     *
 *    The software supplied herewith by Microchip Technology           *
 *    Incorporated (the "Company") for its dsPIC controller            *
 *    is intended and supplied to you, the Company's customer,         *
 *    for use solely and exclusively on Microchip dsPIC                *
 *    products. The software is owned by the Company and/or its        *
 *    supplier, and is protected under applicable copyright laws. All  *
 *    rights are reserved. Any use in violation of the foregoing       *
 *    restrictions may subject the user to criminal sanctions under    *
 *    applicable laws, as well as to civil liability for the breach of *
 *    the terms and conditions of this license.                        *
 *                                                                     *
 *    THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION.  NO           *
 *    WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING,    *
 *    BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND    *
 *    FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE     *
 *    COMPANY SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL,  *
 *    INCIDENTAL OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.  *
 *                                                                     *
  **********************************************************************/

/**********************************************************************
 *                                                                     * 
 *    Filename:       svm.c					                           *
 *                                                                     *
 *                                                                     *
 ***********************************************************************
 *	Code Description
 *  
 *  This file implements 3-phase space vector modulation.  
 **********************************************************************/
#include <p33Exxxx.h>
#include "svm.h"
#include "./Common/UserDefinition/Userdef.h"
#include "./Common/Extern/Extern.h"
#include "./DMCI/RtdmInterface.h"
#include "./Application/RampGenerator/RampGenerator.h"
#include "./MotorControl/SpeedController/SpeedController.h"

/************************************************************************/
/* These are the definitions for various angles used in the SVM */
/* routine.  A 16-bit unsigned value is used as the angle variable. */
/* The SVM algorithm determines the 60 degree sector */
#define	VECTOR1	0				/* 0 degrees   */
#define	VECTOR2	0x2aaa			/* 60 degrees  */
#define	VECTOR3	0x5555			/* 120 degrees */
#define	VECTOR4	0x8000			/* 180 degrees */
#define	VECTOR5	0xaaaa			/* 240 degrees */
#define	VECTOR6	0xd555			/* 300 degrees */
#define	SIXTY_DEG	0x2aaa
/*************************************************************************/

/* This is the maximum value that may be passed to the SVM  */
/* function without overmodulation.  This limit is equivalent */
/* to 0.866, which is sqrt(3)/2. */
#define VOLTS_LIMIT	28377
#define SINE_TABLE_SIZE 172

SHORT adcTrigger;

/* This sinewave lookup table has 171 entries.  (1024 points per */
/* electrical cycle -- 1024*(60/360) = 171) */
/* The table covers 60 degrees of the sine function. */
SHORT __attribute__((near)) sinetable[SINE_TABLE_SIZE] = 
{0,201,401,602,803,1003,1204,1404,1605,1805,
2005,2206,2406,2606,2806,3006,3205,3405,3605,3804,
4003,4202,4401,4600,4799,4997,5195,5393,5591,5789,
5986,6183,6380,6577,6773,6970,7166,7361,7557,7752,
7947,8141,8335,8529,8723,8916,9109,9302,9494,9686,
9877,10068,10259,10449,10639,10829,11018,11207,11395,11583,
11771,11958,12144,12331,12516,12701,12886,13070,13254,13437,
13620,13802,13984,14165,14346,14526,14706,14885,15063,15241,
15419,15595,15772,15947,16122,16297,16470,16643,16816,16988,
17159,17330,17500,17669,17838,18006,18173,18340,18506,18671,
18835,18999,19162,19325,19487,19647,19808,19967,20126,20284,
20441,20598,20753,20908,21062,21216,21368,21520,21671,21821,
21970,22119,22266,22413,22559,22704,22848,22992,23134,23276,
23417,23557,23696,23834,23971,24107,24243,24377,24511,24644,
24776,24906,25036,25165,25293,25420,25547,25672,25796,25919,
26042,26163,26283,26403,26521,26638,26755,26870,26984,27098,
27210,27321,27431,27541,27649,27756,27862,27967,28071,28174,
28276,28377};

/**********************************************************************/
/* The function SVM() determines which sector the input angle is */
/* located in.  Then, the modulation angle is normalized to the current */
/* 60 degree sector.  Two angles are calculated from the normalized */
/* angle.  These two angles determine the times for the T1 and T2 */
/* vectors.  The T1 and T2 vectors are then scaled by the modulation */
/* amplitude and the duty cycle range.  Finally, the T0 vector time */
/* is the time left over in the PWM counting period. */
/* The SVM() function then calculates three duty cycle values based */
/* on the T0, T1, and T2 times.  The duty cycle calculation depends */
/* on the  */
/* appropriate duty cycle values depending on the type of SVM to be */
/* generated. */
/**********************************************************************/
VOID svm(SHORT volts, WORD angle)
{
	WORD PDC1Latch, PDC2Latch, PDC3Latch;
	
	/* These variables hold the normalized sector angles used to find */
	/* t1, t2. */
	WORD angle1, angle2;
	
	/* These variables hold the space vector times. */
	WORD half_t0,t1,t2,tpwm,voltstpwm;
    
    /* These variable used for ADC trigger point calculation */
    SHORT Ta, Tb, Tc;
	
	/* Calculate the total PWM count period */
	tpwm = PHASE1;
	
	/* Limit volts input to avoid overmodulation.*/
	if(volts > VOLTS_LIMIT) 
		volts = VOLTS_LIMIT;
	
	if(angle < VECTOR4)
	{
		if(angle < VECTOR3)
		{
			if(angle < VECTOR2)
			{
				angle2 = angle - VECTOR1;		/* Reference SVM angle to the current sector */
				angle1 = SIXTY_DEG - angle2;	/* Calculate second angle referenced to sector */
                
                angle2 >>= 6; angle1 >>= 6;			
                //check if calculated angles are within the range
                angle2 = (angle2 < SINE_TABLE_SIZE) ? angle2 : (SINE_TABLE_SIZE - 1);
                angle1 = (angle1 < SINE_TABLE_SIZE) ? angle1 : (SINE_TABLE_SIZE - 1);

				t1 = sinetable[(BYTE)angle1];	/* Look up values from table */
				t2 = sinetable[(BYTE)angle2];
				
				/* Scale t1 and t2 to by the volts variable and for the duty cycle range */
				voltstpwm = (SHORT)(__builtin_mulss(volts,tpwm) >> 15);
				t1 = (SHORT)(__builtin_mulss(t1,voltstpwm) >> 15);
				t2 = (SHORT)(__builtin_mulss(t2,voltstpwm) >> 15);
				
				half_t0 = (tpwm - t1 - t2) >> 1;		/* Calculate half_t0 null time from period and t1,t2 */
				
				/* Calculate duty cycles for Sector 1  (0 - 59 degrees) */
				PDC1Latch = t1 + t2 + half_t0;
				PDC2Latch = t2 + half_t0;
				PDC3Latch = half_t0;
			}
			else
			{
				angle2 = angle - VECTOR2;		/* Reference SVM angle to the current sector */
				angle1 = SIXTY_DEG - angle2;	/* Calculate second angle referenced to sector */
                
                angle2 >>= 6; angle1 >>= 6;		
                //check if calculated angles are within the range
                angle2 = (angle2 < SINE_TABLE_SIZE) ? angle2 : (SINE_TABLE_SIZE - 1);
                angle1 = (angle1 < SINE_TABLE_SIZE) ? angle1 : (SINE_TABLE_SIZE - 1);
			
				t1 = sinetable[(BYTE)angle1];	/* Look up values from table */
				t2 = sinetable[(BYTE)angle2];
			
				/* Scale t1 and t2 to by the volts variable and for the duty cycle range */
				voltstpwm = (SHORT)(__builtin_mulss(volts,tpwm) >> 15);
				t1 = (SHORT)(__builtin_mulss(t1,voltstpwm) >> 15);
				t2 = (SHORT)(__builtin_mulss(t2,voltstpwm) >> 15);
			
				half_t0 = (tpwm - t1 - t2) >> 1;		/* Calculate half_t0 null time from period and t1,t2 */
				
				/* Calculate duty cycles for Sector 2  (60 - 119 degrees) */
				PDC1Latch = t1 + half_t0;
				PDC2Latch = t1 + t2 + half_t0;
				PDC3Latch = half_t0;
			}
		}
		else
		{
			angle2 = angle - VECTOR3;		/* Reference SVM angle to the current sector */
			angle1 = SIXTY_DEG - angle2;	/* Calculate second angle referenced to sector */
            
            angle2 >>= 6; angle1 >>= 6;		
            //check if calculated angles are within the range
            angle2 = (angle2 < SINE_TABLE_SIZE) ? angle2 : (SINE_TABLE_SIZE - 1);
            angle1 = (angle1 < SINE_TABLE_SIZE) ? angle1 : (SINE_TABLE_SIZE - 1);
		
			t1 = sinetable[(BYTE)angle1];	// Look up values from table
			t2 = sinetable[(BYTE)angle2];
		
			/* Scale t1 and t2 to by the volts variable and for the duty cycle range */
			voltstpwm = (SHORT)(__builtin_mulss(volts,tpwm) >> 15);
			t1 = (SHORT)(__builtin_mulss(t1,voltstpwm) >> 15);
			t2 = (SHORT)(__builtin_mulss(t2,voltstpwm) >> 15);
		
			half_t0 = (tpwm - t1 - t2) >> 1;		/* Calculate half_t0 null time from period and t1,t2 */
			
			/* Calculate duty cycles for Sector 3  (120 - 179 degrees) */
			PDC1Latch = half_t0;
			PDC2Latch = t1 + t2 + half_t0;
			PDC3Latch = t2 + half_t0;	
		}
	}
	else
	{
		if(angle < VECTOR6)
		{
			if(angle < VECTOR5)
			{
				angle2 = angle - VECTOR4;		/* Reference SVM angle to the current sector */
				angle1 = SIXTY_DEG - angle2;	/* Calculate second angle referenced to sector */
                
                angle2 >>= 6; angle1 >>= 6;		
                //check if calculated angles are within the range
                angle2 = (angle2 < SINE_TABLE_SIZE) ? angle2 : (SINE_TABLE_SIZE - 1);
                angle1 = (angle1 < SINE_TABLE_SIZE) ? angle1 : (SINE_TABLE_SIZE - 1);
			
				t1 = sinetable[(BYTE)angle1];	// Look up values from table
				t2 = sinetable[(BYTE)angle2];
			
				/* Scale t1 and t2 to by the volts variable and for the duty cycle range */
				voltstpwm = (SHORT)(__builtin_mulss(volts,tpwm) >> 15);
				t1 = (SHORT)(__builtin_mulss(t1,voltstpwm) >> 15);
				t2 = (SHORT)(__builtin_mulss(t2,voltstpwm) >> 15);
			
				half_t0 = (tpwm - t1 - t2) >> 1;		/* Calculate half_t0 null time from period and t1,t2 */
				
				/* Calculate duty cycles for Sector 4  (180 - 239 degrees) */
				PDC1Latch = half_t0;
				PDC2Latch = t1 + half_t0;
				PDC3Latch = t1 + t2 + half_t0;
			}
			else
			{
				angle2 = angle - VECTOR5;		// Reference SVM angle to the current sector
				angle1 = SIXTY_DEG - angle2;	// Calculate second angle referenced to sector
                
                angle2 >>= 6; angle1 >>= 6;		
                //check if calculated angles are within the range
                angle2 = (angle2 < SINE_TABLE_SIZE) ? angle2 : (SINE_TABLE_SIZE - 1);
                angle1 = (angle1 < SINE_TABLE_SIZE) ? angle1 : (SINE_TABLE_SIZE - 1);
			
				t1 = sinetable[(BYTE)angle1];	// Look up values from table
				t2 = sinetable[(BYTE)angle2];
			
				/* Scale t1 and t2 to by the volts variable and for the duty cycle range */
				voltstpwm = (SHORT)(__builtin_mulss(volts,tpwm) >> 15);
				t1 = (SHORT)(__builtin_mulss(t1,voltstpwm) >> 15);
				t2 = (SHORT)(__builtin_mulss(t2,voltstpwm) >> 15);
			
				half_t0 = (tpwm - t1 - t2) >> 1;		/* Calculate half_t0 null time from period and t1,t2 */
				
				/* Calculate duty cycles for Sector 5  (240 - 299 degrees) */
				PDC1Latch = t2 + half_t0;
				PDC2Latch = half_t0;
				PDC3Latch = t1 + t2 + half_t0;
			}
		}
		else
		{
			angle2 = angle - VECTOR6;		/* Reference SVM angle to the current sector */
			angle1 = SIXTY_DEG - angle2;	/* Calculate second angle referenced to sector */
            
            angle2 >>= 6; angle1 >>= 6;		
            //check if calculated angles are within the range
            angle2 = (angle2 < SINE_TABLE_SIZE) ? angle2 : (SINE_TABLE_SIZE - 1);
            angle1 = (angle1 < SINE_TABLE_SIZE) ? angle1 : (SINE_TABLE_SIZE - 1);
		
			t1 = sinetable[(BYTE)angle1];	// Look up values from table
			t2 = sinetable[(BYTE)angle2];
		
			/* Scale t1 and t2 to by the volts variable and for the duty cycle range */
			voltstpwm = (SHORT)(__builtin_mulss(volts,tpwm) >> 15);
			t1 = (SHORT)(__builtin_mulss(t1,voltstpwm) >> 15);
			t2 = (SHORT)(__builtin_mulss(t2,voltstpwm) >> 15);
		
			half_t0 = (tpwm - t1 - t2) >> 1;		/* Calculate half_t0 null time from period and t1,t2 */
			
			/* Calculate duty cycles for Sector 6  ( 300 - 359 degrees ) */
			PDC1Latch = t1 + t2 + half_t0;
			PDC2Latch = half_t0;
			PDC3Latch = t1 + half_t0;	
		}
	
	}
    
	/* Limit the minimum duty cycle per Errata #21 workaround */
	if (PDC1Latch > ALTDTR_DIV2)
		PDC1 = PDC1Latch;		
	else
		PDC1 = ALTDTR_DIV2;
	
	if (PDC2Latch > ALTDTR_DIV2)
		PDC2 = PDC2Latch;
	else
		PDC2 = ALTDTR_DIV2;
	
	if (PDC3Latch > ALTDTR_DIV2)
		PDC3 = PDC3Latch;
	else
		PDC3 = ALTDTR_DIV2;
    
    /* Calculate Trigger points */
    
    Ta = t1 + t2 + half_t0;
    Tb = t2 + half_t0;
    Tc = half_t0;

    TRIG1 = adcTrigger = ((Ta + Tb) >> 1) + 42;
}

/******************************************************************************
 * _T7Interrupt
 *
 * The _T7Interrupt calculates trigger point for ADC calculation.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/  
void __attribute__((interrupt, no_auto_psv)) _T7Interrupt (void)
{
    WORD phaseDiff;
    
    IFS3bits.T7IF = 0;
    
    if(PWMCON1bits.FLTSTAT)
		overcurrentfaultTriggered(TRUE);    

#ifdef USE_PHASE_INC_AND_CORRECTION
    //Use phase increment only when shutter moving Up
    //if((requiredDirection == CW) && (!rampStatusFlags.rampDcInjectionOn))
    if(!rampStatusFlags.rampDcInjectionOn)
    {
        if(phaseIncFlg == PHASE_DECREMENT_FLAG)
        {
            phase -= phaseInc;   
            phaseDiff = phaseCopy - phase;
            if (phaseDiff > SIXTY_DEG)
                phase = phaseCopy - SIXTY_DEG;
        }
        else if(phaseIncFlg == PHASE_INCREMENT_FLAG)
        {
            phase += phaseInc;            
            phaseDiff =  phase - phaseCopy;
            if (phaseDiff > SIXTY_DEG)
                phase = phaseCopy + SIXTY_DEG;
        }
    }
#endif
    
    if(controlOutput < 0)
        svm(-(controlOutput+1), phase);  
    else
        svm(controlOutput, phase);  
}

void __attribute__((interrupt, no_auto_psv)) _PWM1Interrupt (void)
{
    WORD phaseDiff;
    
    IFS5bits.PWM1IF = 0;
    
    if(PWMCON1bits.FLTSTAT)
		overcurrentfaultTriggered(TRUE);    

#ifdef USE_PHASE_INC_AND_CORRECTION
    //Use phase increment only when shutter moving Up
    //if((requiredDirection == CW) && (!rampStatusFlags.rampDcInjectionOn))
    if(!rampStatusFlags.rampDcInjectionOn)
    {
        if(phaseIncFlg == PHASE_DECREMENT_FLAG)
        {
            phase -= phaseInc;   
            phaseDiff = phaseCopy - phase;
            if (phaseDiff >= SIXTY_DEG)
                phase = phaseCopy - SIXTY_DEG;
        }
        else if(phaseIncFlg == PHASE_INCREMENT_FLAG)
        {
            phase += phaseInc;            
            phaseDiff =  phase - phaseCopy;
            if (phaseDiff >= SIXTY_DEG)
                phase = phaseCopy + SIXTY_DEG;
        }
    }
#endif
    
    if((requiredDirection == CW) && (measuredSpeed >= 1000))
    {
        if(controlOutput < 0)
            //svm(-(controlOutput+1), phase+ PhaseAdvance);
            svm(-(controlOutput+1), phase);  
        else
            //svm(controlOutput, phase + PhaseAdvance);
            svm(controlOutput, phase);  
    }
    else
    {
        if(controlOutput < 0)
            svm(-(controlOutput+1), phase);  
        else
            svm(controlOutput, phase);
    }
}
