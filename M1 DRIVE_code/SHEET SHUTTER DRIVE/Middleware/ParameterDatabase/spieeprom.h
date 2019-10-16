/**********************************************************************
* ?2007 Microchip Technology Inc.
*
* FileName:        spieeprom.h
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
//#ifndef SPIEEPROM_H
//#define SPIEEPROM_H

//#include "p24fxxxx.h"
#include <p33Exxxx.h>
#include "./Common/Typedefs/Typedefs.h"
#include "./Common/UserDefinition/Userdef.h"

// peripheral configurations
#define SPI_MASTER  0x013C	// select 8-bit master mode, CKE=1, CKP=0 1MHZ
#define SPI_ENABLE	0x8000	// enable SPI port, clear status

#define BLOCK_READ_SIZE     32
#define BLOCK_WRITE_SIZE    32

#define EEPROM_SIZE         8192    //8192*8bit organization

#define EEPROM_CS_SETUP_TIME    10  //100ns
#define EEPROM_CS_HOLD_TIME     20  //200ns

/************************************************************************
* EEPROM Commands                                                       *
*                                                                       *    
************************************************************************/
#define EEPROM_PAGE_SIZE    (unsigned)64
#define EEPROM_PAGE_MASK    (unsigned)0x003f
#define EEPROM_CMD_READ     (unsigned)0b00000011//Read data from memory array beginning at selected address
#define EEPROM_CMD_WRITE    (unsigned)0b00000010//Write data to memory array beginning at selected address
#define EEPROM_CMD_WRDI     (unsigned)0b00000100//Reset the write enable latch (disable write operations)	
#define EEPROM_CMD_WREN     (unsigned)0b00000110//Set the write enable latch (enable write operations)
#define EEPROM_CMD_RDSR     (unsigned)0b00000101//Read STATUS register
#define EEPROM_CMD_WRSR     (unsigned)0b00000001//Write STATUS register

/************************************************************************
* Aliases for IOs registers related to SPI connected to EEPROM          *
*                                                                       *    
************************************************************************/


#define EEPROM_SS_TRIS      TRISBbits.TRISB0
/*
Added for motor control board hardware version 2 on 30 Dec 2014
*/
#if (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER1)
#define EEPROM_SS_PORT      PORTBbits.RB0
#elif (MTR_CTRL_HW_TYPE == MTR_CTRL_HW_VER2)
#define EEPROM_SS_PORT		PORTGbits.RG7
#else
#error Motor control hardware type not defined in spieeprom.h
#endif

#define EEPROM_SCK_TRIS     TRISCbits.TRISC3
#define EEPROM_SCK_PORT     PORTCbits.RC3
#define EEPROM_SDO_TRIS     TRISAbits.TRISA4
#define EEPROM_SDO_PORT     PORTAbits.RA4
#define EEPROM_SDI_TRIS     TRISAbits.TRISA9
#define EEPROM_SDI_PORT     PORTAbits.RA9

/************************************************************************
* Structure STATREG and union _EEPROMStatus_                            *
*                                                                       *
* Overview: Provide a bits and byte access to EEPROM status value.      *
*                                                                       *
************************************************************************/
struct  STATREG{
	unsigned    WIP:1;
	unsigned    WEL:1;
	unsigned    BP0:1;
	unsigned    BP1:1;
	unsigned    RESERVED:3;
	unsigned    WPEN:1;
};

union _EEPROMStatus_{
	struct  STATREG Bits;
	unsigned char	Char;
};

/************************************************************************
* Macro: Lo                                                             *
*                                                                       *
* Preconditions: None                                                   *
*                                                                       *
* Overview: This macro extracts a low byte from a 2 byte word.          *
*                                                                       *
* Input: None.                                                          *
*                                                                       *
* Output: None.                                                         *
*                                                                       *
************************************************************************/
#define Lo(X)   (unsigned char)(X&0x00ff)

/************************************************************************
* Macro: Hi                                                             *
*                                                                       *
* Preconditions: None                                                   *
*                                                                       *
* Overview: This macro extracts a high byte from a 2 byte word.         *
*                                                                       *
* Input: None.                                                          *
*                                                                       *
* Output: None.                                                         *
*                                                                       *
************************************************************************/
#define Hi(X)   (unsigned char)((X>>8)&0x00ff)

/************************************************************************
* Macro: mEEPROMSSLow                                                   *
*                                                                       *
* Preconditions: SS IO must be configured as output.                    *
*                                                                       *
* Overview: This macro pulls down SS line                               *
*           to start a new EEPROM operation.                            *
*                                                                       *
* Input: None.                                                          *
*                                                                       *
* Output: None.                                                         *
*                                                                       *
************************************************************************/
//#define mEEPROMSSLow()      EEPROM_SS_PORT = 0;

/************************************************************************
* Macro: mEEPROMSSHigh                                                  *
*                                                                       *
* Preconditions: SS IO must be configured as output.                    *
*                                                                       *
* Overview: This macro set SS line to high level                        *
*           to start a new EEPROM operation.                            *
*                                                                       *
* Input: None.                                                          *
*                                                                       *
* Output: None.                                                         *
*                                                                       *
************************************************************************/
//#define mEEPROMSSHigh()     EEPROM_SS_PORT = 1;

/************************************************************************
* Function: EEPROMInit                                                  *
*                                                                       *
* Preconditions: SPI module must be configured to operate with          *
*                 parameters: Master, MODE16 = 0, CKP = 1, SMP = 1.     *
*                                                                       *
* Overview: This function setup SPI IOs connected to EEPROM.            *
*                                                                       *
* Input: None.                                                          *
*                                                                       *
* Output: None.                                                         *
*                                                                       *
************************************************************************/
extern void EEPROMInit(void);

/************************************************************************
* Function: EEPROMReadStatus()                                          *
*                                                                       *
* Preconditions: SPI module must be configured to operate with  EEPROM. *
*                                                                       *
* Overview: This function reads status register from EEPROM.            *
*                                                                       *
* Input: None.                                                          *
*                                                                       *
* Output: Status register value.                                        *
*                                                                       *
************************************************************************/
extern union _EEPROMStatus_ EEPROMReadStatus(void);

/************************************************************************
* Function: EEPROMWriteByte()                                           *
*                                                                       *
* Preconditions: SPI module must be configured to operate with  EEPROM. *
*                                                                       *
* Overview: This function writes a new value to address specified.      *
*                                                                       *
* Input: Data to be written and address.                                *
*                                                                       *
* Output: None.                                                         *
*                                                                       *
************************************************************************/

EXTERN VOID mEEPROMSSLow(VOID);
EXTERN VOID mEEPROMSSHigh(VOID);

EXTERN BOOL EEPROMWriteByte(BYTE, UINT);
EXTERN BOOL EEPROMWriteWord(WORD, UINT);
EXTERN BOOL EEPROMWriteDword(DWORD, UINT);
EXTERN BOOL EEPROMWriteBlock(UINT , WORD , BYTE*);
EXTERN BOOL EEPROMEraseBlock(UINT , WORD);

/************************************************************************
* Function: EEPROMReadByte()                                            *
*                                                                       *
* Preconditions: SPI module must be configured to operate with  EEPROM. *
*                                                                       *
* Overview: This function reads a value from address specified.         *
*                                                                       *
* Input: Address.                                                       *
*                                                                       *
* Output: Data read.                                                    *
*                                                                       *
************************************************************************/
EXTERN BYTE EEPROMReadByte(UINT);
EXTERN WORD EEPROMReadWord(UINT);
EXTERN DWORD EEPROMReadDword(UINT);
EXTERN BOOL EEPROMReadBlock(UINT , WORD , BYTE*);

/************************************************************************
* Function: EEPROMWriteEnable()                                         *
*                                                                       *
* Preconditions: SPI module must be configured to operate with EEPROM.  *
*                                                                       *
* Overview: This function allows a writing into EEPROM. Must be called  *
* before every writing command.                                         *
*                                                                       *
* Input: None.                                                          *
*                                                                       *
* Output: None.                                                         *
*                                                                       *
************************************************************************/
extern void EEPROMWriteEnable(void);
extern void EEPROMWriteDisable(void);

//#endif //SPIEEPROM_H

