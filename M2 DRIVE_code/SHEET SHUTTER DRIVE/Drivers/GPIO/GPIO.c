/*********************************************************************************
* FileName: GPIO.c
* Description:
* This source file contains the definition of all the functions for GPIO.
* It initializes all the port pins required in application.
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
#include "GPIO.h"
#include "./Common/UserDefinition/Userdef.h"

/******************************************************************************
 * initGPIO
 *
 * This function initializes all the I/O's required by application.
 *
 * PARAMETER REQ: none
 *
 * RETURNS: none
 *
 * ERRNO: none
 ********************************************************************************/
VOID initGPIO(VOID)
{
#if (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER1)
	/*	Unlock the Reprogrammable Pin Mechanism */
	__builtin_write_OSCCONL(OSCCON & (~(1<<6))); /* clear bit 6  */
    
    //TRISA &= 0|(0|(1<<0)|(1<<1)|(1<<7)|(1<<10)|(1<<8)); 
    //PORTA &= 0|(0|(1<<0)|(1<<1)|(1<<7)|(1<<10)|(1<<8));
    
    //TRISB &= 0|(0|(1<<1)|(1<<7)|(1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<14)|(1<<15)); 
    //PORTB &= 0|(0|(1<<1)|(1<<7)|(1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<14)|(1<<15));
    
    ////TRISC &= 0|(0|(1<<0)|(1<<10)|(1<<9)|(1<<4)|(1<<8)|(1<<7)|(1<<13)|(1<<6)|(1<<5)|(1<<1)|(1<<2)); 
    ////PORTC &= 0|(0|(1<<0)|(1<<10)|(1<<9)|(1<<4)|(1<<8)|(1<<7)|(1<<13)|(1<<6)|(1<<5)|(1<<1)|(1<<2));
    
    ////TRISD &= 0|(0|(1<<8)|(1<<5)|(1<<6));
    ////PORTD &= 0|(0|(1<<8)|(1<<5)|(1<<6));
    
    //TRISC &= 0|(0|(1<<0)|(1<<10)|(1<<4)|(1<<8)|(1<<13)|(1<<6)|(1<<5)|(1<<1)|(1<<2)); 
    //PORTC &= 0|(0|(1<<0)|(1<<10)|(1<<4)|(1<<8)|(1<<13)|(1<<6)|(1<<5)|(1<<1)|(1<<2));
    
    //TRISD &= 0|(0|(1<<8));
    //PORTD &= 0|(0|(1<<8));
    
    //TRISE &= 0|(0|(1<<13)|(1<<15)|(1<<14));
    //PORTE &= 0|(0|(1<<13)|(1<<15)|(1<<14));
    
    //TRISF &= 0|(0|(1<<0)|(1<<1)); 
    //PORTF &= 0|(0|(1<<0)|(1<<1)); 
    
    //TRISG &= 0|(0|(1<<9)); 
    //PORTG &= 0|(0|(1<<9));
    
    /*PWM Initialisation starts*/
	TRISBbits.TRISB10= 0;//PWM3H
	TRISBbits.TRISB11= 0;//PWM3L
	TRISBbits.TRISB12= 0;//PWM2H
	TRISBbits.TRISB13= 0;//PWM2L
	TRISBbits.TRISB14= 0;//PWM1H
	TRISBbits.TRISB15= 0;//PWM1L
	TRISEbits.TRISE13 = 1;/*FLT6*/
	ANSELEbits.ANSE13 = 0;
    CNPUEbits.CNPUE13 = 1; 
	TRISCbits.TRISC0 = 0;//LED 2
    PORTGbits.RG9 = 1;//disable PWM buffer
	TRISGbits.TRISG9= 0;//PWM enable
	ANSELB = 0x0000;// Disable Analog input on PWM channels
	ANSELCbits.ANSC0 = 0;//Disable Analog input on LED 2
	ANSELGbits.ANSG9 = 0;//Disable Analog input on PWM enable pin
	/*PWM initialisation ends*/
	
	/*	Sensor interface to motor drive board start*/
	TRISAbits.TRISA0 = 1;	/*Photoelec Obs sensor */
    CNPUAbits.CNPUA0 = 1; 
	TRISAbits.TRISA1 = 1;	/*Emergency Stop sensor*/
    CNPUAbits.CNPUA1 = 1; 
	TRISBbits.TRISB1 = 1;	/*Air Switch,Wrap around sensor,Micro Switch sensor*/
    CNPUBbits.CNPUB1 = 1; 
	TRISBbits.TRISB7 = 1;	/*Power fail Occured*/
    CNPUBbits.CNPUB7 = 1; 
	TRISCbits.TRISC10 = 1;	/*4 Point Limit Switch D*/
    CNPUCbits.CNPUC10 = 1; 
	TRISDbits.TRISD8 = 1;	/*origin sensor*/
    CNPUDbits.CNPUD8 = 1; 
	TRISEbits.TRISE15 = 1;	/*Temperature sensor*/
    CNPUEbits.CNPUE15 = 1; 
	
	TRISAbits.TRISA10 = 0;//DBR control
    PORTAbits.RA10 = 0; //DBR output disable
	TRISAbits.TRISA7 = 0;//led1
	TRISCbits.TRISC0 = 0;//led2
	TRISCbits.TRISC4 = 0;//uart control output
	TRISCbits.TRISC8 = 0;//enable brake
	TRISCbits.TRISC13 = 0;//enable fan
	
	ANSELAbits.ANSA0 = 0;/*Photoelec Obs sensor */
	ANSELAbits.ANSA1 = 0;/*Emergency Stop sensor*/
	ANSELBbits.ANSB1 = 0;/*Air Switch,Wrap around sensor,Micro Switch sensor*/
	ANSELBbits.ANSB7 = 0;/*Power fail Occured*/
	ANSELCbits.ANSC10 = 0;/*4 Point Limit Switch D*/
	ANSELEbits.ANSE15 = 0;/*Temperature sensor*/
	
	ANSELCbits.ANSC0 = 0;//led2
	ANSELCbits.ANSC4 = 0;//uart control output
    PORTCbits.RC4 = 0;
	
	/*HAll sensor configuration starts*/
	TRISAbits.TRISA8 = 1;
    CNPUAbits.CNPUA8 = 1; 
	TRISCbits.TRISC6 = 1;
    CNPUCbits.CNPUC6 = 1; 
	RPINR7 = 0;
	RPINR7bits.IC1R = 0x18;/*  Assign IC1(HALLA) to RPI24 */
	RPINR7bits.IC2R = 0x36;/*  Assign IC2(HALLB) to RP54 */
    TRISFbits.TRISF0 = 1;
    CNPUFbits.CNPUF0 = 1; 
	RPINR8 = 0;
	RPINR8bits.IC3R = 0x60;/*  Assign IC3(HALLC) to RPI96 */
	/* Configure interrupt 0 for power fail*/
	INTCON2bits.INT0EP = 1;
	IEC0bits.INT0IE = 1;	
    IPC0bits.INT0IP = 7;
	IFS0bits.INT0IF = 0;
	/*	Sensor interface to motor drive board ends*/
	
	/*UART 1 initialization*/
	TRISFbits.TRISF1 = 0;
    //CNPUFbits.CNPUF1 = 1; 
	TRISCbits.TRISC5 = 1;
    CNPUCbits.CNPUC5 = 1; 
  	ANSELCbits.ANSC5 = 0;
	RPINR18bits.U1RXR = 0x0035;		/* Make Pin RPI53 U1RX Used by RTDM */
  	RPOR9bits.RP97R = 0x0001;		/* Make Pin RP97 U1TX Used by RTDM */

	/*UART 2 initialization*/
	TRISCbits.TRISC1 = 0;
    //CNPUCbits.CNPUC1 = 1; 
    ANSELCbits.ANSC1 = 0;
	TRISCbits.TRISC2 = 1;
    CNPUCbits.CNPUC2 = 1; 
   	ANSELCbits.ANSC2 = 0;
	RPINR19bits.U2RXR = 0x0032;		/* Make Pin RPI50 U2RX Used by RTDM */
  	RPOR5bits.RP49R = 0x0003;		/* Make Pin RP49 U2TX Used by RTDM */
	
	/* Analog inputs configuration starts*/
	TRISEbits.TRISE14 = 1; /*BLDC Curent Monitoring, common current in all legs*/
	ANSELEbits.ANSE14 = 1; /*Itotal current*/
    
    //Hardware version 
    //TRISCbits.TRISC7 = 1; //strap1
    //CNPUCbits.CNPUC7 = 1; 
    //TRISCbits.TRISC9 = 1; //strap2
    //CNPUCbits.CNPUC9 = 1; 
    //TRISDbits.TRISD5 = 1; //strap3
    //CNPUDbits.CNPUD5 = 1; 
    //TRISDbits.TRISD6 = 1; //strap4
    //CNPUDbits.CNPUD6 = 1; 
    
    PORTAbits.RA7 = 1; //Turn OFF LED1
    PORTCbits.RC0 = 1; //Turn OFF LED2
    
	__builtin_write_OSCCONL(OSCCON | (1<<6)); 	 /* Set bit 6 */
	
#elif (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER2)
/*
Added for motor control board hardware version 2 on 30 Dec 2014
*/
	/*	Unlock the Reprogrammable Pin Mechanism */
	__builtin_write_OSCCONL(OSCCON & (~(1<<6))); /* clear bit 6  */
    
    //TRISA &= 0|(0|(1<<0)|(1<<1)|(1<<7)|(1<<10)|(1<<8)); 
    //PORTA &= 0|(0|(1<<0)|(1<<1)|(1<<7)|(1<<10)|(1<<8));
    
    //TRISB &= 0|(0|(1<<1)|(1<<7)|(1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<14)|(1<<15)); 
    //PORTB &= 0|(0|(1<<1)|(1<<7)|(1<<10)|(1<<11)|(1<<12)|(1<<13)|(1<<14)|(1<<15));
    
    ////TRISC &= 0|(0|(1<<0)|(1<<10)|(1<<9)|(1<<4)|(1<<8)|(1<<7)|(1<<13)|(1<<6)|(1<<5)|(1<<1)|(1<<2)); 
    ////PORTC &= 0|(0|(1<<0)|(1<<10)|(1<<9)|(1<<4)|(1<<8)|(1<<7)|(1<<13)|(1<<6)|(1<<5)|(1<<1)|(1<<2));
    
    ////TRISD &= 0|(0|(1<<8)|(1<<5)|(1<<6));
    ////PORTD &= 0|(0|(1<<8)|(1<<5)|(1<<6));
    
    //TRISC &= 0|(0|(1<<0)|(1<<10)|(1<<4)|(1<<8)|(1<<13)|(1<<6)|(1<<5)|(1<<1)|(1<<2)); 
    //PORTC &= 0|(0|(1<<0)|(1<<10)|(1<<4)|(1<<8)|(1<<13)|(1<<6)|(1<<5)|(1<<1)|(1<<2));
    
    //TRISD &= 0|(0|(1<<8));
    //PORTD &= 0|(0|(1<<8));
    
    //TRISE &= 0|(0|(1<<13)|(1<<15)|(1<<14));
    //PORTE &= 0|(0|(1<<13)|(1<<15)|(1<<14));
    
    //TRISF &= 0|(0|(1<<0)|(1<<1)); 
    //PORTF &= 0|(0|(1<<0)|(1<<1)); 
    
    //TRISG &= 0|(0|(1<<9)); 
    //PORTG &= 0|(0|(1<<9));
    
    /*PWM Initialisation starts*/
	TRISBbits.TRISB10= 0;//PWM3H
	TRISBbits.TRISB11= 0;//PWM3L
	TRISBbits.TRISB12= 0;//PWM2H
	TRISBbits.TRISB13= 0;//PWM2L
	TRISBbits.TRISB14= 0;//PWM1H
	TRISBbits.TRISB15= 0;//PWM1L
	TRISEbits.TRISE13 = 1;/*FLT6*/
	ANSELEbits.ANSE13 = 0;
    CNPUEbits.CNPUE13 = 1; 
	TRISCbits.TRISC0 = 0;//LED 2
    PORTGbits.RG9 = 1;//disable PWM buffer
	TRISGbits.TRISG9= 0;//PWM enable
	ANSELB = 0x0000;// Disable Analog input on PWM channels
	ANSELCbits.ANSC0 = 0;//Disable Analog input on LED 2
	ANSELGbits.ANSG9 = 0;//Disable Analog input on PWM enable pin
	/*PWM initialisation ends*/
	
	/*	Sensor interface to motor drive board start*/
	TRISAbits.TRISA12 = 1;	/*Photoelec Obs sensor */
    CNPUAbits.CNPUA12 = 1; 
	TRISGbits.TRISG6 = 1;	/*Emergency Stop sensor*/
    CNPUGbits.CNPUG6 = 1; 
	TRISAbits.TRISA11 = 1;	/*Air Switch,Wrap around sensor,Micro Switch sensor*/
    CNPUAbits.CNPUA11 = 1; 
	TRISBbits.TRISB7 = 1;	/*Power fail Occured*/
    CNPUBbits.CNPUB7 = 1; 
	TRISCbits.TRISC10 = 1;	/*4 Point Limit Switch D*/
    CNPUCbits.CNPUC10 = 1; 
	TRISDbits.TRISD8 = 1;	/*origin sensor*/
    CNPUDbits.CNPUD8 = 1; 
	TRISEbits.TRISE15 = 1;	/*Temperature sensor*/
    CNPUEbits.CNPUE15 = 1; 
	
	//	Added on 20Feb2015 for IGBT over temperature fault
	TRISBbits.TRISB4 = 1;	/*IGBT over temperature fault*/
    CNPUBbits.CNPUB4 = 1;
	
	// For testing only (Added on 27 Jan 2015 to enable MCU_LATCH_CTRL output)
	/**************************************************/
	ANSELGbits.ANSG8 = 0;	//Disable Analog input on MCU_LATCH_CTRL fault latch clear bit
	TRISGbits.TRISG8 = 0;	/*MCU_LATCH_CTRL fault latch clear bit*/
    CNPUGbits.CNPUG8 = 1;
	PORTGbits.RG8 = 1;		//	Set HIGH as default value for MCU_LATCH_CTRL
	/**************************************************/
	
	TRISAbits.TRISA10 = 0;//DBR control
    PORTAbits.RA10 = 0; //DBR output disable
	TRISAbits.TRISA7 = 0;//led1
	TRISCbits.TRISC0 = 0;//led2
	TRISCbits.TRISC11 = 0;//led2
	TRISCbits.TRISC4 = 0;//uart control output
	TRISCbits.TRISC8 = 0;//enable brake
	TRISCbits.TRISC13 = 0;//enable fan
    PORTCbits.RC13 = 0;
	
	ANSELAbits.ANSA12 = 0;/*Photoelec Obs sensor */
	ANSELGbits.ANSG6 = 0;/*Emergency Stop sensor*/
	ANSELAbits.ANSA11 = 0;/*Air Switch,Wrap around sensor,Micro Switch sensor*/
	ANSELBbits.ANSB7 = 0;/*Power fail Occured*/
	ANSELCbits.ANSC10 = 0;/*4 Point Limit Switch D*/
	ANSELEbits.ANSE15 = 0;/*Temperature sensor*/
	
	ANSELCbits.ANSC11 = 0;//led2
	ANSELCbits.ANSC4 = 0;//uart control output
    PORTCbits.RC4 = 0;
	
	/*HAll sensor configuration starts*/
	TRISAbits.TRISA8 = 1;
    CNPUAbits.CNPUA8 = 1; 
	TRISCbits.TRISC6 = 1;
    CNPUCbits.CNPUC6 = 1; 
	RPINR7 = 0;
	RPINR7bits.IC1R = 0x18;/*  Assign IC1(HALLA) to RPI24 */
	RPINR7bits.IC2R = 0x36;/*  Assign IC2(HALLB) to RP54 */
    TRISFbits.TRISF0 = 1;
    CNPUFbits.CNPUF0 = 1; 
	RPINR8 = 0;
	RPINR8bits.IC3R = 0x60;/*  Assign IC3(HALLC) to RPI96 */
	/* Configure interrupt 0 for power fail*/
	INTCON2bits.INT0EP = 1;
	//	Disable power fail interrupt for version 4 board as power fail feature is not present in it
	IEC0bits.INT0IE = 0;
    IPC0bits.INT0IP = 7;
	IFS0bits.INT0IF = 0;
	
	//	Added on 20Feb2015 for IGBT over temperature fault
	/* Configure interrupt 1 for over temperature*/
	INTCON2bits.INT1EP = 0;		//	Interrupt on positive edge as default value of this line is low in HW ver 2.
	IEC1bits.INT1IE = 1;		//	Enable INT 1 interrupt.
    IPC5bits.INT1IP = 7;		//	Interrupt priority (highest)
	IFS1bits.INT1IF = 0;		//	Clear interrupt flag.
	RPINR0 = 0;					//	Clear PERIPHERAL PIN SELECT INPUT REGISTER 0 before selecting INT1 functionality on RP36
	RPINR0bits.INT1R = 0x24;	//  Assign INT1 to RPI36
	/*	Sensor interface to motor drive board ends*/
	
    PORTAbits.RA7=0;    
    
	/*UART 1 initialization*/
	TRISFbits.TRISF1 = 0;
    //CNPUFbits.CNPUF1 = 1; 
	TRISCbits.TRISC5 = 1;
    CNPUCbits.CNPUC5 = 1; 
  	ANSELCbits.ANSC5 = 0;
	RPINR18bits.U1RXR = 0x0035;		/* Make Pin RPI53 U1RX Used by RTDM */
  	RPOR9bits.RP97R = 0x0001;		/* Make Pin RP97 U1TX Used by RTDM */
	
	/*UART 2 initialization*/
	TRISCbits.TRISC1 = 0;
    //CNPUCbits.CNPUC1 = 1; 
    ANSELCbits.ANSC1 = 0;
	TRISCbits.TRISC2 = 1;
    CNPUCbits.CNPUC2 = 1; 
   	ANSELCbits.ANSC2 = 0;
	RPINR19bits.U2RXR = 0x0032;		/* Make Pin RPI50 U2RX Used by RTDM */
  	RPOR5bits.RP49R = 0x0003;		/* Make Pin RP49 U2TX Used by RTDM */
	
	/* Analog inputs configuration starts*/
	TRISEbits.TRISE14 = 1; /*BLDC Curent Monitoring, common current in all legs*/
	ANSELEbits.ANSE14 = 1; /*Itotal current*/
    
	TRISAbits.TRISA0 = 1; /*Phase 1 current monitoring*/
	ANSELAbits.ANSA0 = 1; /*IA*/
	
	TRISAbits.TRISA1 = 1; /*Phase 2 current monitoring*/
	ANSELAbits.ANSA1 = 1; /*IB*/
	
	TRISBbits.TRISB0 = 1; /*Boost Converter O/P Voltage monitor*/
	ANSELBbits.ANSB0 = 1; /*VBUS/DC_BUS*/
	
	TRISBbits.TRISB1 = 1; /*Motor Phase Voltage Monitoring*/
	ANSELBbits.ANSB1 = 1; /*V_M1*/
	
	TRISBbits.TRISB2 = 1; /*Motor Phase Voltage Monitoring*/
	ANSELBbits.ANSB2 = 1; /*V_M2*/
	
	TRISBbits.TRISB3 = 1; /*Motor Phase Voltage Monitoring*/
	ANSELBbits.ANSB3 = 1; /*V_M3*/
	/* Analog inputs configuration ends*/
    //Hardware version 
    //TRISCbits.TRISC7 = 1; //strap1
    //CNPUCbits.CNPUC7 = 1; 
    //TRISCbits.TRISC9 = 1; //strap2
    //CNPUCbits.CNPUC9 = 1; 
    //TRISDbits.TRISD5 = 1; //strap3
    //CNPUDbits.CNPUD5 = 1; 
    //TRISDbits.TRISD6 = 1; //strap4
    //CNPUDbits.CNPUD6 = 1; 
    
    //PORTAbits.RA7 = 1; //Turn OFF LED1
    PORTCbits.RC0 = 1; //Turn OFF LED2 Orange
    
	__builtin_write_OSCCONL(OSCCON | (1<<6)); 	 /* Set bit 6 */
#else
#error Motor control hardware type not defined in GPIO.c
    
#endif 
}
