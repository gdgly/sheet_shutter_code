/*
 * testdebounce.h
 *
 */

#ifndef __TESTDEBOUNCE_H__
#define __TESTDEBOUNCE_H__

//#ifdef __cplusplus
//extern "C"
//{
//#endif
#ifdef UART_CONSOLE	// Use uart as console. Need to include uartstdio.c
void testDebounceSetup(void);
#endif	/* UART_CONSOLE */
void testKeyDebounceModule(void);


#endif /* __TESTDEBOUNCE_H__ */
