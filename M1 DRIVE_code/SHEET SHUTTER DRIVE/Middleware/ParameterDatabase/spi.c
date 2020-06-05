/**********************************************************************
* ?2007 Microchip Technology Inc.
*
* FileName:        eeprom.c
* Dependencies:    Header (.h) files if applicable, see below
* Processor:       PIC24Fxxxx
* Compiler:        MPLAB?C30 v3.00 or higher
*
* SOFTWARE LICENSE AGREEMENT:
* Microchip Technology Incorporated ("Microchip") retains all
* ownership and intellectual property rights in the code accompanying
* this message and in all derivatives hereto.  You may use this code,
* and any derivatives created by any person or entity by or on your
* behalf, exclusively with Microchip's proprietary products.  Your
* acceptance and/or use of this code constitutes agreement to the
* terms and conditions of this notice.
*
* CODE ACCOMPANYING THIS MESSAGE IS SUPPLIED BY MICROCHIP "AS IS". NO
* WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
* NOT LIMITED TO, IMPLIED WARRANTIES OF NON-INFRINGEMENT,
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS
* CODE, ITS INTERACTION WITH MICROCHIP'S PRODUCTS, COMBINATION WITH
* ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
*
* YOU ACKNOWLEDGE AND AGREE THAT, IN NO EVENT, SHALL MICROCHIP BE
* LIABLE, WHETHER IN CONTRACT, WARRANTY, TORT (INCLUDING NEGLIGENCE OR
* BREACH OF STATUTORY DUTY), STRICT LIABILITY, INDEMNITY,
* CONTRIBUTION, OR OTHERWISE, FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, FOR COST OR
* EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE CODE, HOWSOEVER
* CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE
* DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT ALLOWABLE BY LAW,
* MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS
* CODE, SHALL NOT EXCEED THE PRICE YOU PAID DIRECTLY TO MICROCHIP
* SPECIFICALLY TO HAVE THIS CODE DEVELOPED.
*
* You agree that you are solely responsible for testing the code and
* determining its suitability.  Microchip has no obligation to modify,
* test, certify, or support the code.
*
* REVISION HISTORY:
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Author        Date      	Comments on this revision
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Albert Z.		05/16/08	First release of source file
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* 25LC256 is connected to SPI2
************************************************************************/

#include "spi.h"
#include "spieeprom.h"

#include "./Common/UserDefinition/Userdef.h"
// send one byte of data and receive one back at the same time
unsigned char writeSPI1( unsigned char i )
{
    //long waitCnt = 140000;

    SPI1BUF = i;					// write to buffer for TX
    //while(!SPI1STATbits.SPIRBF) 	// wait for transfer to complete
    //{
    //    waitCnt--;
    //    if(waitCnt <= 0)
    //    {
    //        break;
    //    }
    //}

    while(!SPI1STATbits.SPIRBF);

    return SPI1BUF;    				// read the received value
}//writeSPI1

/************************************************************************
* Function: SPI1INTInit                                        			*
*                                                                		*
* Preconditions: TRIS bit of Slave Chip Select pin (if any used) should *
* be made output. This function is used for initializing the SPI module	*
* It initializes the module according to Application Maestro options.   *
*                                                                     	*
* Input: Application Maestro options                   					*
*                                                                      	*
* Output: None                                                   		*
*                                                                   	*
************************************************************************/
void SPI1INTInit()
{
	//20191223 ADD by IME
    SPI1CON1 = 0;  // select mode clear
    SPI1STAT = 0;  // enable the peripheral clear
    //set as digital i/o
/*
Added for motor control board hardware version 2 on 30 Dec 2014
*/
#if (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER1)
    ANSELBbits.ANSB0 = 0; //chip select
#elif (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER2)
	ANSELGbits.ANSG7 = 0; //chip select
#else
	#error Motor control hardware type not defined
#endif
    ANSELAbits.ANSA4 = 0; //SDO data out
    ANSELAbits.ANSA9 = 0; //SDI data in
    ANSELCbits.ANSC3 = 0; //SCK clock

    //set port pin status
/*
Added for motor control board hardware version 2 on 30 Dec 2014
*/
#if (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER1)
    PORTBbits.RB0 = 1;
#elif (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER2)
	PORTGbits.RG7 = 1;
#else
#error Motor control hardware type not defined
#endif
    PORTCbits.RC3 = 1;
    PORTAbits.RA4 = 1;
    PORTAbits.RA9 = 1;

    //set direction
/*
Added for motor control board hardware version 2 on 30 Dec 2014
*/
#if (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER1)
    TRISBbits.TRISB0 = 0;
#elif (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER2)
	TRISGbits.TRISG7 = 0;
#else
#error Motor control hardware type not defined in spi.c
#endif
    TRISCbits.TRISC3 = 0;
    TRISAbits.TRISA4 = 0;
    TRISAbits.TRISA9 = 1;
	CNPUAbits.CNPUA9 = 1; //Enable pullup for data out

    IFS0bits.SPI1IF = 0;	// clear interrupt flag
    IEC0bits.SPI1IE = 1;	// disable interrupt

    SPI1CON1 = SPI_MASTER;  // select mode
    SPI1STAT = SPI_ENABLE;  // enable the peripheral
} // SPI1INTInit

void __attribute__((interrupt, no_auto_psv)) _SPI1Interrupt()
{
	/* add code here */
	IFS0bits.SPI1IF = 0;
}


