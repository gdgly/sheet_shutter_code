/*********************************************************************************
* FileName: display.h
* Description: 
* This source file contains the prototype definition for functions used to carry
* out display operations
* Version: 0.1D
*
*
**********************************************************************************/

/****************************************************************************
 * Copyright 2014 Bunka Shutters.
 * This program is the property of the Bunka Shutters
 * and it shall not be reproduced, distributed or used
 * without permission of an authorized company official.
 * This is an unpublished work subject to Trade Secret
 * and Copyright protection.
*****************************************************************************/


/****************************************************************************
 *  Modification History
 *
 *  Revision		Date                  Name          			Comments
 *  	0.1D	dd/mm/yyyy      	iGATE Offshore team       Initial Creation
****************************************************************************/

#ifndef DISPLAY_H
#define DISPLAY_H

#ifdef __cplusplus
extern "C"
{
#endif

/****************************************************************************
 *  Macro definitions:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global variables:
****************************************************************************/


/****************************************************************************/


/******************************************************************************
 * FunctionName: displayText
 *
 * Function Description: 
 * This function draws a English text on screen within a rectangle.
 * It also provides options for creating border, highlight and curved corners 
 * of rectangle.
 *
 * Function Parameters:
 * Text:				Text to be displayed.
 * XMin: 				x coordinate of upper left corner of rectangle.
 * YMin: 				y coordinate of upper left corner of rectangle.
 * HighlightEnable:		Enable or disable highlight on string.
 * BorderEnable: 		Enable or disable a rectangular border around string.
 * CurvedCornerEnable:	Enable or disable curved corners of rectangular border.
 *
 * Function Returns:
 * 0	:	Success
 * 1	:	coordinates out of display area
 *
 ********************************************************************************/

uint8_t
displayText(unsigned char *Text, int16_t XMin, int16_t YMin, bool HighlightEnable,
					bool BorderEnable, bool CurvedCornerEnable, bool MenuBoundaryEnable,bool MenuNearOneByOneEnable,bool MenusmallEnable);

/********************************************************************************/

#endif /*DISPLAY_H*/
