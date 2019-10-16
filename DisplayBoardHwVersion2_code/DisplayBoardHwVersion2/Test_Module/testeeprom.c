/*
 * testeeprom.c
 *
 *  Created on: Apr 4, 2014
 *      Author: rk803609
 */

#include <stdbool.h>
#include <stdint.h>
#include <driverlib/eeprom.h>
#include "Middleware/eeprom.h"
#include "Middleware/bolymindisplay.h"

uint8_t dataToWrite[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
uint32_t bufferToRead[20] = {0};
uint8_t buffer[20] = {0};
uint8_t retVal;

void testEEPROMWrite_OneByte()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x001, 1);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_OneByteLast()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x003, 1);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_TwoBytesMiddle()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x001, 2);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_TwoBytesLast()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x002, 2);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_TwoWords()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x002, 3);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_TwoWordsLastFull()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x002, 6);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_ThreeWords()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x002, 7);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_ThreeWordsLastFull()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x002, 10);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_FourWords()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x002, 12);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_FourWordsLastFull()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x002, 14);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_FirstWordFull()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x000, 6);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_BothWordsFull()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x000, 8);
	EEPROMRead(bufferToRead, 0x000, 20);
}

void testEEPROMWrite_AddressOutOfRange()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x888, 5);
	EEPROMRead(bufferToRead, 0x000, 20);
}

//void testEEPROMWrite_CountOutOfRange()
//{
//	retVal = EEPROMProgramByte(dataToWrite, 0x001, 300);
//	EEPROMRead(bufferToRead, 0x000, 20);
//}

void testEEPROMWrite_LastByteOutOfRange()
{
	retVal = EEPROMProgramByte(dataToWrite, 0x790, 150);
	EEPROMRead(bufferToRead, 0x000, 16);
}






void testEEPROMRead_OneByte()
{
	retVal = EEPROMReadByte(buffer, 0x001, 1);
}

void testEEPROMRead_OneByteLast()
{
	retVal = EEPROMReadByte(buffer, 0x003, 1);
}

void testEEPROMRead_TwoByteMiddle()
{
	retVal = EEPROMReadByte(buffer, 0x001, 2);
}

void testEEPROMRead_TwoByteLast()
{
	retVal = EEPROMReadByte(buffer, 0x002, 2);
}

void testEEPROMRead_TwoWords()
{
	retVal = EEPROMReadByte(buffer, 0x002, 3);
}

void testEEPROMRead_TwoWordsLastFull()
{
	retVal = EEPROMReadByte(buffer, 0x002, 6);
}

void testEEPROMRead_ThreeWords()
{
	retVal = EEPROMReadByte(buffer, 0x002, 7);
}

void testEEPROMRead_ThreeWordsLastFull()
{
	retVal = EEPROMReadByte(buffer, 0x002, 10);
}

void testEEPROMRead_FourWords()
{
	retVal = EEPROMReadByte(buffer, 0x002, 12);
}

void testEEPROMRead_FourWordsLastFull()
{
	retVal = EEPROMReadByte(buffer, 0x002, 14);
}

void testEEPROMRead_FirstWordFull()
{
	retVal = EEPROMReadByte(buffer, 0x000, 6);
}

void testEEPROMRead_BothWordsFull()
{
	retVal = EEPROMReadByte(buffer, 0x000, 8);
}

void testEEPROMRead_AddressOutOfRange()
{
	retVal = EEPROMReadByte(buffer, 0x888, 5);
}

//void testEEPROMRead_CountOutOfRange()
//{
//	retVal = EEPROMReadByte(buffer, 0x001, 300);
//}

void testEEPROMRead_LastByteOutOfRange()
{
	retVal = EEPROMReadByte(buffer, 0x790, 150);
}
