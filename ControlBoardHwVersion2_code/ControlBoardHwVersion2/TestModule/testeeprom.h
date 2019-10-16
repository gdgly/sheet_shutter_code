/*
 * testeeprom.h
 *
 *  Created on: Apr 7, 2014
 *      Author: rk803609
 */

#ifndef TESTEEPROM_H_
#define TESTEEPROM_H_

void testEEPROMWrite_OneByte();
void testEEPROMWrite_OneByteLast();
void testEEPROMWrite_TwoBytesMiddle();
void testEEPROMWrite_TwoBytesLast();
void testEEPROMWrite_TwoWords();
void testEEPROMWrite_TwoWordsLastFull();
void testEEPROMWrite_ThreeWords();
void testEEPROMWrite_ThreeWordsLastFull();
void testEEPROMWrite_FourWords();
void testEEPROMWrite_FourWordsLastFull();
void testEEPROMWrite_FirstWordFull();
void testEEPROMWrite_BothWordsFull();
void testEEPROMWrite_AddressOutOfRange();
void testEEPROMWrite_CountOutOfRange();
void testEEPROMWrite_LastByteOutOfRange();
void testEEPROMWrite_FourWordsComplete();

void testEEPROMRead_OneByte();
void testEEPROMRead_OneByteLast();
void testEEPROMRead_TwoByteMiddle();
void testEEPROMRead_TwoByteLast();
void testEEPROMRead_TwoWords();
void testEEPROMRead_TwoWordsLastFull();
void testEEPROMRead_ThreeWords();
void testEEPROMRead_ThreeWordsLastFull();
void testEEPROMRead_FourWords();
void testEEPROMRead_FourWordsLastFull();
void testEEPROMRead_FirstWordFull();
void testEEPROMRead_BothWordsFull();
void testEEPROMRead_AddressOutOfRange();
void testEEPROMRead_CountOutOfRange();
void testEEPROMRead_LastByteOutOfRange();


#endif /* TESTEEPROM_H_ */
