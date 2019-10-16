/**********************************************************************
* ?2007 Microchip Technology Inc.
*
* FileName:        spieeprom.c
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
#include <p33Exxxx.h>
#include "spi.h"
#include "spieeprom.h"

#define WRITE_WAIT_CNT      140000
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
void EEPROMInit()
{
    // Set IOs directions for EEPROM SPI
    EEPROM_SS_PORT = 1;
    EEPROM_SS_TRIS = 0;
}

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
BOOL EEPROMWriteByte(BYTE Data, UINT Address)
{
	unsigned char Local_8;
    //LONG writeWaitCnt;
    BOOL writeSucess = TRUE;
    
    INTCON2bits.GIE = 0;
    EEPROMWriteEnable();
    mEEPROMSSLow();
    Local_8 = writeSPI1(EEPROM_CMD_WRITE);
    Local_8 = writeSPI1(Hi(Address));
    Local_8 = writeSPI1(Lo(Address));
    Local_8 = writeSPI1(Data);
    mEEPROMSSHigh();
    // wait for completion of previous write operation
    //if write is not completed within 1ms then abort the loop and
    //log eeprom write fail error
    //writeWaitCnt = WRITE_WAIT_CNT;
    //while(EEPROMReadStatus().Bits.WIP)
    //{
    //    writeWaitCnt--;
    //    if(writeWaitCnt <= 0)
    //    {
    //        writeSucess = FALSE;
    //        break;
    //    }
    //}
    
    while(EEPROMReadStatus().Bits.WIP);
    
    EEPROMWriteDisable();
    INTCON2bits.GIE = 1;
    
    return(writeSucess);
}

BOOL EEPROMWriteWord(WORD Data, UINT Address)
{
    BYTE dataSize = 2;
    BYTE* bytePtr = NULL;    
	unsigned char Local_8;
    //LONG writeWaitCnt;
    BOOL writeSucess = TRUE;
    
    EEPROMWriteEnable();
    mEEPROMSSLow();    
    Local_8 = writeSPI1(EEPROM_CMD_WRITE);    
    Local_8 = writeSPI1(Hi(Address));
    Local_8 = writeSPI1(Lo(Address));
    
    bytePtr = (BYTE*)&Data;
    do
    {
        Local_8 = writeSPI1(*bytePtr++);
    }while(--dataSize);
    
    mEEPROMSSHigh();    
    // wait for completion of previous write operation
    //if write is not completed within 1ms then abort the loop and
    //log eeprom write fail error
    //writeWaitCnt = WRITE_WAIT_CNT;
    //while(EEPROMReadStatus().Bits.WIP)
    //{
    //    writeWaitCnt--;
    //    if(writeWaitCnt <= 0)
    //    {
    //        writeSucess = FALSE;
    //        break;
    //    }
    //}
    
    while(EEPROMReadStatus().Bits.WIP);
        
    EEPROMWriteDisable();
    
    return(writeSucess);
}

BOOL EEPROMWriteDword(DWORD Data, UINT Address)
{
    BYTE dataSize = 4;
    BYTE* bytePtr = NULL;    
	unsigned char Local_8;
    //LONG writeWaitCnt;
    BOOL writeSucess = TRUE;
    
    EEPROMWriteEnable();
    mEEPROMSSLow();    
    Local_8 = writeSPI1(EEPROM_CMD_WRITE);    
    Local_8 = writeSPI1(Hi(Address));
    Local_8 = writeSPI1(Lo(Address));
    
    bytePtr = (BYTE*)&Data;
    do
    {
        Local_8 = writeSPI1(*bytePtr++);
    }while(--dataSize);
    
    mEEPROMSSHigh();    
    // wait for completion of previous write operation
    //if write is not completed within 1ms then abort the loop and
    //log eeprom write fail error
    //writeWaitCnt = WRITE_WAIT_CNT;
    //while(EEPROMReadStatus().Bits.WIP)
    //{
    //    writeWaitCnt--;
    //    if(writeWaitCnt <= 0)
    //    {
    //        writeSucess = FALSE;
    //        break;
    //    }
    //}
    
    while(EEPROMReadStatus().Bits.WIP);
        
    EEPROMWriteDisable();

	return(writeSucess);
}

BOOL EEPROMWriteBlock(UINT startAddr, WORD blockLen, BYTE* dataBuf)
{
   	BYTE Local_8;    
    BYTE byteCntr = 0;
    BYTE remainingPageByte = 0;
    //LONG writeWaitCnt;
    BOOL writeSucess = TRUE;

    EEPROMWriteEnable();
    while(blockLen)
    {   
        //if block size is more than page read size then process the requrest in multiple pages
        if(blockLen > BLOCK_WRITE_SIZE)
        {
            //EEPROMWriteEnable();
            mEEPROMSSLow();            
            Local_8 = writeSPI1(EEPROM_CMD_WRITE);            
            Local_8 = writeSPI1(Hi(startAddr));
            Local_8 = writeSPI1(Lo(startAddr));
            
            for(byteCntr = 0; byteCntr < BLOCK_WRITE_SIZE; byteCntr++)
            {
                Local_8 = writeSPI1(*dataBuf++);
            }         
            blockLen -= BLOCK_WRITE_SIZE;
            startAddr += BLOCK_WRITE_SIZE;
            
            mEEPROMSSHigh();
            // wait for completion of previous write operation
            //if write is not completed within 1ms then abort the loop and
            //log eeprom write fail error
            //writeWaitCnt = WRITE_WAIT_CNT;
            //while(EEPROMReadStatus().Bits.WIP)
            //{
            //    writeWaitCnt--;
            //    if(writeWaitCnt <= 0)
            //    {
            //        writeSucess = FALSE;
            //        break;
            //    }
            //} 
            
            while(EEPROMReadStatus().Bits.WIP);
            //EEPROMWriteDisable();                
        }
        //if block size is less than page read size then process the requrest in one pages
        else
        {            
            //EEPROMWriteEnable();
            mEEPROMSSLow();            
            Local_8 = writeSPI1(EEPROM_CMD_WRITE);            
            Local_8 = writeSPI1(Hi(startAddr));
            Local_8 = writeSPI1(Lo(startAddr));
            
            for(byteCntr = 0; byteCntr < blockLen; byteCntr++)
            {
                Local_8 = writeSPI1(*dataBuf++);
            }         
            
            //Fill 0xFF in remaing page bytes
            remainingPageByte = BLOCK_WRITE_SIZE - blockLen;
            for(byteCntr = 0; byteCntr < remainingPageByte; byteCntr++)
            {
                Local_8 = writeSPI1(0xFF);
            }
            blockLen = 0;
            
            mEEPROMSSHigh();
            // wait for completion of previous write operation
            //if write is not completed within 1ms then abort the loop and
            //log eeprom write fail error
            //writeWaitCnt = WRITE_WAIT_CNT;
            //while(EEPROMReadStatus().Bits.WIP)
            //{
            //    writeWaitCnt--;
            //    if(writeWaitCnt <= 0)
            //    {
            //        writeSucess = FALSE;
            //        break;
            //    }
            //}
            
            while(EEPROMReadStatus().Bits.WIP);
            //EEPROMWriteDisable();            
        }
    }     
    EEPROMWriteDisable();
    
    return(writeSucess);
}

BOOL EEPROMEraseBlock(UINT startAddr, WORD blockLen)
{
   	BYTE Local_8;    
    BYTE byteCntr = 0;
    BYTE remainingPageByte = 0;
    //LONG writeWaitCnt;
    BOOL writeSucess = TRUE;

    EEPROMWriteEnable();
    while(blockLen)
    {   
        //if block size is more than page read size then process the requrest in multiple pages
        if(blockLen > BLOCK_WRITE_SIZE)
        {
            //EEPROMWriteEnable();
            mEEPROMSSLow();            
            Local_8 = writeSPI1(EEPROM_CMD_WRITE);            
            Local_8 = writeSPI1(Hi(startAddr));
            Local_8 = writeSPI1(Lo(startAddr));
            
            for(byteCntr = 0; byteCntr < BLOCK_WRITE_SIZE; byteCntr++)
            {
                Local_8 = writeSPI1(0xFF);
            }         
            blockLen -= BLOCK_WRITE_SIZE;
            startAddr += BLOCK_WRITE_SIZE;
            
            mEEPROMSSHigh();
            // wait for completion of previous write operation
            //if write is not completed within 1ms then abort the loop and
            //log eeprom write fail error
            //writeWaitCnt = WRITE_WAIT_CNT;
            //while(EEPROMReadStatus().Bits.WIP)
            //{
            //    writeWaitCnt--;
            //    if(writeWaitCnt <= 0)
            //    {
            //        writeSucess = FALSE;
            //        break;
            //    }
            //} 
            
            while(EEPROMReadStatus().Bits.WIP);
            //EEPROMWriteDisable();            
        }
        //if block size is less than page read size then process the requrest in one pages
        else
        {
            //EEPROMWriteEnable();
            mEEPROMSSLow();            
            Local_8 = writeSPI1(EEPROM_CMD_WRITE);            
            Local_8 = writeSPI1(Hi(startAddr));
            Local_8 = writeSPI1(Lo(startAddr));
            
            for(byteCntr = 0; byteCntr < blockLen; byteCntr++)
            {
                Local_8 = writeSPI1(0xFF);
            }  
            
            //Fill 0xFF in remaing page bytes
            remainingPageByte = BLOCK_WRITE_SIZE - blockLen;
            for(byteCntr = 0; byteCntr < remainingPageByte; byteCntr++)
            {
                Local_8 = writeSPI1(0xFF);
            }
            blockLen = 0;
            
            mEEPROMSSHigh();
            // wait for completion of previous write operation
            //if write is not completed within 1ms then abort the loop and
            //log eeprom write fail error
            //writeWaitCnt = WRITE_WAIT_CNT;
            //while(EEPROMReadStatus().Bits.WIP)
            //{
            //    writeWaitCnt--;
            //    if(writeWaitCnt <= 0)
            //    {
            //        writeSucess = FALSE;
            //        break;
            //    }
            //}
            
            while(EEPROMReadStatus().Bits.WIP);
            //EEPROMWriteDisable();            
        }
    } 
    EEPROMWriteDisable();
    
    return(writeSucess);
}


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
BYTE EEPROMReadByte(UINT Address)
{
	BYTE Local_8;

    INTCON2bits.GIE = 0;
    mEEPROMSSLow();
    Local_8 = writeSPI1(EEPROM_CMD_READ);
	Local_8 = writeSPI1(Hi(Address));
    Local_8 = writeSPI1(Lo(Address));
    Local_8 = writeSPI1(0);

    mEEPROMSSHigh();
    INTCON2bits.GIE = 1;
    
    return Local_8;
}

WORD EEPROMReadWord(UINT Address)
{
    BYTE dataSize = 2;
    BYTE* bytePtr = NULL;    
	WORD Local_16;
	BYTE Local_8;
    
    mEEPROMSSLow();    
    Local_8 = writeSPI1(EEPROM_CMD_READ);    
	Local_8 = writeSPI1(Hi(Address));
    Local_8 = writeSPI1(Lo(Address));
    
    bytePtr = (BYTE*)&Local_16;
    do
    {        
        *bytePtr = writeSPI1(0);
        bytePtr++;        
    }while(--dataSize); 
    
    mEEPROMSSHigh();
    
    return Local_16;
}

DWORD EEPROMReadDword(UINT Address)
{
    BYTE dataSize = 4;
    BYTE* bytePtr = NULL;    
	DWORD Local_32;
	BYTE Local_8;

    mEEPROMSSLow();    
    Local_8 = writeSPI1(EEPROM_CMD_READ);    
	Local_8 = writeSPI1(Hi(Address));
    Local_8 = writeSPI1(Lo(Address));
    
    bytePtr = (BYTE*)&Local_32;
    do
    {        
        *bytePtr = writeSPI1(0);
        bytePtr++;        
    }while(--dataSize);        
    
    mEEPROMSSHigh();
    
    return Local_32;
}

BOOL EEPROMReadBlock(UINT startAddr, WORD blockLen, BYTE* dataBuf)
{
	BYTE Local_8;    
    BYTE byteCntr = 0;
    
    while(blockLen)
    {   
        //if block size is more than page read size then process the requrest in multiple pages
        if(blockLen > BLOCK_READ_SIZE)
        {
            mEEPROMSSLow();            
            Local_8 = writeSPI1(EEPROM_CMD_READ);            
            Local_8 = writeSPI1(Hi(startAddr));
            Local_8 = writeSPI1(Lo(startAddr));
            
            for(byteCntr = 0; byteCntr < BLOCK_READ_SIZE; byteCntr++)
            {
                *dataBuf = writeSPI1(0);
                dataBuf++;
            }         
            blockLen -= BLOCK_READ_SIZE;
            startAddr += BLOCK_READ_SIZE;
            
            mEEPROMSSHigh();
        }
        //if block size is less than page read size then process the requrest in one pages
        else
        {
            mEEPROMSSLow();            
            Local_8 = writeSPI1(EEPROM_CMD_READ);            
            Local_8 = writeSPI1(Hi(startAddr));
            Local_8 = writeSPI1(Lo(startAddr));
            
            for(byteCntr = 0; byteCntr < blockLen; byteCntr++)
            {
                *dataBuf = writeSPI1(0);
                dataBuf++;
            } 
            blockLen = 0;
            
            mEEPROMSSHigh();
        }
    } 
    
    return TRUE;
}

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
void EEPROMWriteEnable()
{
	unsigned char Local_8;
    mEEPROMSSLow();
    Local_8 = writeSPI1(EEPROM_CMD_WREN);
    mEEPROMSSHigh();
}

void EEPROMWriteDisable()
{
	unsigned char Local_8;
    mEEPROMSSLow();
    Local_8 = writeSPI1(EEPROM_CMD_WRDI);
    mEEPROMSSHigh();
}

VOID mEEPROMSSLow(VOID)
{
    SHORT setupTime = EEPROM_CS_SETUP_TIME;
    EEPROM_SS_PORT = 0;
    while(setupTime--);    
}

VOID mEEPROMSSHigh(VOID)
{
    SHORT holdTime = EEPROM_CS_HOLD_TIME;
    EEPROM_SS_PORT = 1;
    while(holdTime--);
}

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
union _EEPROMStatus_ EEPROMReadStatus()
{
	unsigned char Local_8;

    mEEPROMSSLow();
    Local_8 = writeSPI1(EEPROM_CMD_RDSR);
    Local_8 = writeSPI1(0);
    mEEPROMSSHigh();

    return (union _EEPROMStatus_)Local_8;
}

