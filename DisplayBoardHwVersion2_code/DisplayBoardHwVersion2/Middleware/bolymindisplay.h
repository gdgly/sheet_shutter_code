/*********************************************************************************
* FileName: bolyminDisplay.h
* Description:
* This header file contains the prototypes of all driver routines for bolymin display.
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
 *  	0.1D	26/05/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/
#ifndef BOLYMINDISPLAY_H_
#define BOLYMINDISPLAY_H_

//#define   OLED_LCD_SELET

#ifdef OLED_LCD_SELET
/******************************************************************************
 * FunctionName: bolyminDisplayInit
 *
 * Function Description: This function initializes display controller.
 *
 * Function Parameter: None
 *
 * Function Returns: void
 *
*********************************************************************************/
void bolyminDisplayInit(void);

/******************************************************************************
 * FunctionName: bolyminDisplayWriteCommand
 *
 * Function Description: Writes a set of command bytes to bolymin display controller.
 *
 * Function Parameter:
 * pi8Cmd		:	a pointer to a set of command bytes.
 * ui32Count	:	count of command bytes.
 *
 * Function Returns: void
 *
*********************************************************************************/
void bolyminDisplayWriteCommand(const uint8_t *pi8Cmd, uint32_t ui32Count);

/******************************************************************************
 * FunctionName: bolyminDisplayWriteData
 *
 * Function Description: Writes a set of data bytes to bolymin display controller.
 *
 * Function Parameter:
 * pi8Data		:	a pointer to a set of data bytes, containing pixel data.
 * ui32Count	:	count of command bytes.
 *
 * Function Returns: void
 *
*********************************************************************************/
void bolyminDisplayWriteData(const uint8_t *pi8Data, uint32_t ui32Count);

/******************************************************************************
 * FunctionName: GrPixelDrawBolymin
 *
 * Function Description: This function draws a pixel on display screen w.r.t. input coordinates.
 *
 * Function Parameter:
 * lucX			:	x coordinate of the pixel to be drawn.
 * lucY			:	y coordinate of the pixel to be drawn.
 * lbFillColor	:	pixel color. If true, then glow the pixel else don't glow the pixel.
 * lGlowColumn	:	If true then glow column pixels else glow a single pixel.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrPixelDrawBolymin(uint8_t lucX, uint8_t lucY, bool lbFillColor, bool lGlowColumn);

/******************************************************************************
 * FunctionName: GrLineDrawHorizontalBolymin
 *
 * Function Description: Draws a horizontal line on display.
 *
 * Function Parameter:
 * lucX1		: x coordinate of the start of the line
 * lucX2		: x coordinate of the end of line
 * lucY			: y coordinate of the line. Position from top
 * lbFillColor	: pixel color. If true, then glow the pixel else don't glow the pixel.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrLineDrawHorizontalBolymin(uint8_t lucX1, uint8_t lucX2, uint8_t lucY, bool lbFillColor);

/******************************************************************************
 * FunctionName: GrLineDrawVerticalBolymin
 *
 * Function Description: Draws a vertical line on display.
 *
 * Function Parameter:
 * lucX			:	x coordinate of the vertical line. Position from left.
 * lucY1		:	y coordinate of the start of line.
 * lucY2		:	y coordinate of the end of line.
 * lbFillColor	:	pixel color. If true, then glow the pixel else don't glow the pixel.
 * lGlowColumn	:	If true then glow column pixels else glow a single pixel.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrLineDrawVerticalBolymin(uint8_t lucX, uint8_t lucY1, uint8_t lucY2, bool lbFillColor, bool lGlowColumn);

/******************************************************************************
 * FunctionName: GrLineRectFIllBolymin
 *
 * Function Description: Fills a rectangle.
 *
 * Function Parameter:
 * lucXMin					:	x coordinate of the left edge of rectangle.
 * lucXMax					:	x coordinate of the right edge of rectangle.
 * lucYMin					:	y coordinate of the top edge of the rectangle.
 * lucYMax					:	y coordinate of the bottom edge of the rectangle.
 * lbFillColor				:	Fill color of rectangle. If true, then glow the pixel
 * 								else don't glow the pixel.
 * lGlowRightmostColumn		:	If true, glow rightmost column of rectangle. Else, glow
 * 								rightmost pixel of rectangle.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrRectFIllBolymin(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin,
		uint8_t lucYMax, bool lbFillColor, bool lGlowRightmostColumn);

/******************************************************************************
 * FunctionName: GrRectDrawBolymin
 *
 * Function Description: Draws a rectangle on display.
 *
 * Function Parameter:
 * lucXMin		:	x coordinate of the left edge of rectangle.
 * lucXMax		:	x coordinate of the right edge of rectangle.
 * lucYMin		:	y coordinate of the top edge of the rectangle.
 * lucYMax		:	y coordinate of the bottom edge of the rectangle.
 * lbFillColor	:	Color of edges. If true, then glow the pixel else don't glow the pixel.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrRectDrawBolymin(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax, bool lbFillColor);

/******************************************************************************
 * FunctionName: GrStringDrawBolymin
 *
 * Function Description: Write a string on display.
 *
 * Function Parameter:
 * lucBolyminPixelData	:	data to be displayed
 * lucLength			:	length of data to be displayed
 * lucX 				:	horizontal starting position. It should be always even number from 0 to 127.
 * lucY					:	vertical position ranging from 0 to 63.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrStringDrawBolymin(unsigned char *plucBolyminPixelData, unsigned char lucLength,
             unsigned char lucX,unsigned char lucY, bool lbFillColor);


#endif



















#ifndef OLED_LCD_SELET

extern uint8_t enable_15display_cyw;

void delay(uint16_t i);
//void clear_clear(void);
void send_command(uint16_t data);
void  lcd_init(void);
void lcd_address(uint8_t page,uint8_t column);
void send_data(uint16_t data);
void display_string_5x7(uint16_t page,uint16_t column,uint8_t *text,bool lbFillColor_b);
/******************************************************************************
 * FunctionName: bolyminDisplayInit
 *
 * Function Description: This function initializes display controller.
 *
 * Function Parameter: None
 *
 * Function Returns: void
 *
*********************************************************************************/
void bolyminDisplayInit(void);
void LCD_BACKLIGHT_ON(void);
void LCD_BACKLIGHT_OFF(void);
/******************************************************************************
 * FunctionName: GrPixelDrawBolymin
 *
 * Function Description: This function draws a pixel on display screen w.r.t. input coordinates.
 *
 * Function Parameter:
 * lucX         :   x coordinate of the pixel to be drawn.
 * lucY         :   y coordinate of the pixel to be drawn.
 * lbFillColor  :   pixel color. If true, then glow the pixel else don't glow the pixel.
 * lGlowColumn  :   If true then glow column pixels else glow a single pixel.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrPixelDrawBolymin(uint8_t lucX, uint8_t lucY, bool lbFillColor, bool lGlowColumn);

/******************************************************************************
 * FunctionName: GrLineDrawHorizontalBolymin
 *
 * Function Description: Draws a horizontal line on display.
 *
 * Function Parameter:
 * lucX1        : x coordinate of the start of the line
 * lucX2        : x coordinate of the end of line
 * lucY         : y coordinate of the line. Position from top
 * lbFillColor  : pixel color. If true, then glow the pixel else don't glow the pixel.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrLineDrawHorizontalBolymin(uint8_t lucX1, uint8_t lucX2, uint8_t lucY, bool lbFillColor);

/******************************************************************************
 * FunctionName: GrLineDrawVerticalBolymin
 *
 * Function Description: Draws a vertical line on display.
 *
 * Function Parameter:
 * lucX         :   x coordinate of the vertical line. Position from left.
 * lucY1        :   y coordinate of the start of line.
 * lucY2        :   y coordinate of the end of line.
 * lbFillColor  :   pixel color. If true, then glow the pixel else don't glow the pixel.
 * lGlowColumn  :   If true then glow column pixels else glow a single pixel.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrLineDrawVerticalBolymin(uint8_t lucX, uint8_t lucY1, uint8_t lucY2, bool lbFillColor, bool lGlowColumn);

/******************************************************************************
 * FunctionName: GrLineRectFIllBolymin
 *
 * Function Description: Fills a rectangle.
 *
 * Function Parameter:
 * lucXMin                  :   x coordinate of the left edge of rectangle.
 * lucXMax                  :   x coordinate of the right edge of rectangle.
 * lucYMin                  :   y coordinate of the top edge of the rectangle.
 * lucYMax                  :   y coordinate of the bottom edge of the rectangle.
 * lbFillColor              :   Fill color of rectangle. If true, then glow the pixel
 *                              else don't glow the pixel.
 * lGlowRightmostColumn     :   If true, glow rightmost column of rectangle. Else, glow
 *                              rightmost pixel of rectangle.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrRectFIllBolymin(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin,
        uint8_t lucYMax, bool lbFillColor, bool lGlowRightmostColumn);
void GrRectFIllBolymin_cyw(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin,
        uint8_t lucYMax, uint8_t lbFillColor, bool lGlowRightmostColumn);
void DISP_GUESTER_9_16(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax);
void DISP_GUESTER_13_16(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax);
void DISP_GUESTER_KONG(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax);
void DISP_GUESTER_FAN(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax);
void DISP_GUESTER_FAN_DIS(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax);
/******************************************************************************
 * FunctionName: GrRectDrawBolymin
 *
 * Function Description: Draws a rectangle on display.
 *
 * Function Parameter:
 * lucXMin      :   x coordinate of the left edge of rectangle.
 * lucXMax      :   x coordinate of the right edge of rectangle.
 * lucYMin      :   y coordinate of the top edge of the rectangle.
 * lucYMax      :   y coordinate of the bottom edge of the rectangle.
 * lbFillColor  :   Color of edges. If true, then glow the pixel else don't glow the pixel.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrRectDrawBolymin(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax, bool lbFillColor);

/******************************************************************************
 * FunctionName: GrStringDrawBolymin
 *
 * Function Description: Write a string on display.
 *
 * Function Parameter:
 * lucBolyminPixelData  :   data to be displayed
 * lucLength            :   length of data to be displayed
 * lucX                 :   horizontal starting position. It should be always even number from 0 to 127.
 * lucY                 :   vertical position ranging from 0 to 63.
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrStringDrawBolymin(unsigned char *plucBolyminPixelData, unsigned char lucLength,
             unsigned char lucX,unsigned char lucY, bool lbFillColor);


/******************************************************************************
 * FunctionName: UpdateRamString_5x7
 *
 * Function Description: This function update 5x7 font data for input string (text) into output string (ram)
 *
 * Function Parameter:
 * *text  		  :   data to be displayed.
 * *ram			  :   output string in which pixel information for font to be copied
 * lbFillColor_b  :
 *
 * Function Returns: void
 *
*********************************************************************************/
void UpdateRamString_5x7_DOUBLE(uint8_t *text,uint16_t *ram,bool lbFillColor_b);

void UpdateRamString_8x8_DOUBLE(uint8_t *text,uint16_t *ram,bool lbFillColor_b);
/******************************************************************************
 * FunctionName: display_ram
 *
 * Function Description	: This function is use to display 16 bit ram data on two pages of display for specified column
 *
 * Function Parameter:
 * page   	 			:   It should be always even number from 1 to 8.
 * columnMin,columnMax 	:   It should be always even number from 1 to 128
 * *ram  				:   ram data to be displayed
  *
 * Function Returns: void
 *
*********************************************************************************/
void display_ram(uint16_t page,uint16_t columnMin,uint16_t columnMax,uint16_t *ram);

#endif





#endif /* BOLYMINDISPLAY_H_ */
