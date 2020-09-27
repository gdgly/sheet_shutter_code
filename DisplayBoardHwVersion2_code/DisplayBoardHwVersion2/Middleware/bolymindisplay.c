/*********************************************************************************
* FileName: bolyminDisplay.c
* Description:
* This source file contains the definition of all driver routine for bolymin display
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
 *  	0.2D	10/07/2014									Patch for OLED garbage value
 *  	0.1D	22/05/2014      	iGATE Offshore team       Initial Creation
****************************************************************************/

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "bolymindisplay.h"
#include "Application/intertaskcommunication.h"


void Set_lcdlightON(void);

uint8_t enable_15display_cyw = 0;
/****************************************************************************/
//#define   OLED_LCD_SELET

#ifdef OLED_LCD_SELET
/****************************************************************************
 *  Macro definitions:
****************************************************************************/
//*****************************************************************************
//
// Defines the SSI and GPIO peripherals that are used for this display.
//
//*****************************************************************************
#define DISPLAY_SSI_PERIPH          SYSCTL_PERIPH_SSI2
#define DISPLAY_SSI_GPIO_PERIPH     SYSCTL_PERIPH_GPIOB
#define DISPLAY_RST_GPIO_PERIPH     SYSCTL_PERIPH_GPIOB

//*****************************************************************************
//
// Defines the GPIO pin configuration macros for the pins that are used for
// the SSI function.
//
//*****************************************************************************
//#define DISPLAY_PINCFG_SSICLK       GPIO_PH4_SSI2CLK
//#define DISPLAY_PINCFG_SSIFSS       GPIO_PH5_SSI2FSS
//#define DISPLAY_PINCFG_SSITX        GPIO_PH7_SSI2TX

#define DISPLAY_PINCFG_SSICLK       GPIO_PB4_SSI2CLK
#define DISPLAY_PINCFG_SSIFSS       GPIO_PB5_SSI2FSS
#define DISPLAY_PINCFG_SSITX        GPIO_PB7_SSI2TX

//*****************************************************************************
//
// Defines the port and pins for the SSI peripheral.
//
//*****************************************************************************
#define DISPLAY_SSI_PORT            GPIO_PORTB_BASE
#define DISPLAY_SSI_PINS            (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_7)

//*****************************************************************************
//
// Defines the port and pins for the display voltage enable signal.
//
//*****************************************************************************
#define DISPLAY_ENV_PORT            GPIO_PORTB_BASE
#define DISPLAY_ENV_PIN             GPIO_PIN_3

//*****************************************************************************
//
// Defines the port and pins for the display reset signal.
//
//*****************************************************************************
#define DISPLAY_RST_PORT            GPIO_PORTB_BASE
#define DISPLAY_RST_PIN             GPIO_PIN_2

//*****************************************************************************
//
// Defines the port and pins for the display Data/Command (D/C) signal.
//
//*****************************************************************************
#define DISPLAY_D_C_PORT            GPIO_PORTB_BASE
#define DISPLAY_D_C_PIN             GPIO_PIN_6

//*****************************************************************************
//
// Defines the SSI peripheral used and the data speed.
//
//*****************************************************************************
#define DISPLAY_SSI_BASE            SSI2_BASE // SSI2
#define DISPLAY_SSI_CLOCK           3000000

/****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
****************************************************************************/
// An array that holds a set of commands that are sent to the display when
// it is initialized.
static
uint8_t g_ui8DisplayInitCommands[] =
{
	0x15, 0x00, 0x3F,	//set Column address
	0x75, 0x00, 0x3F,	//set Row address
	0xA0, 0x57,			//Re-map
	0xA2, 64,			//Set display offset, maximum row
	0xA8, 63,			//Set Multiplex Ratio, MAX_Row-1
	0xAD, 0x02,			//Set Master Configuration
//	0xB8, 0xBB,
//	0xAF				//Display on
};

/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Global constant for this file:
****************************************************************************/
// Defines a 5x7 font
unsigned char const font_5x7[][5] = {
										  { 0x00, 0x00, 0x00, 0x00, 0x00 }, // space
										  { 0x00, 0x00, 0x2f, 0x00, 0x00 }, // ! 
										  { 0x00, 0x07, 0x00, 0x07, 0x00 }, // "
										  { 0x14, 0x7f, 0x14, 0x7f, 0x14 }, // #
										  { 0x24, 0x2a, 0x7f, 0x2a, 0x12 }, // $
										  { 0xc4, 0xc8, 0x10, 0x26, 0x46 }, // %
										  { 0x36, 0x49, 0x55, 0x22, 0x50 }, // &
										  { 0x00, 0x05, 0x03, 0x00, 0x00 }, // '
										  { 0x00, 0x1c, 0x22, 0x41, 0x00 }, // (
										  { 0x00, 0x41, 0x22, 0x1c, 0x00 }, // )
										  { 0x14, 0x08, 0x3E, 0x08, 0x14 }, // *
										  { 0x08, 0x08, 0x3E, 0x08, 0x08 }, // +
										  { 0x00, 0x00, 0x50, 0x30, 0x00 }, // ,
										  { 0x10, 0x10, 0x10, 0x10, 0x10 }, // -
										  { 0x00, 0x60, 0x60, 0x00, 0x00 }, // .
										  { 0x20, 0x10, 0x08, 0x04, 0x02 }, // /
										  { 0x3E, 0x51, 0x49, 0x45, 0x3E }, // 0
										  { 0x00, 0x42, 0x7F, 0x40, 0x00 }, // 1
										  { 0x42, 0x61, 0x51, 0x49, 0x46 }, // 2
										  { 0x21, 0x41, 0x45, 0x4B, 0x31 }, // 3
										  { 0x18, 0x14, 0x12, 0x7F, 0x10 }, // 4  
										  { 0x27, 0x45, 0x45, 0x45, 0x39 }, // 5
										  { 0x3C, 0x4A, 0x49, 0x49, 0x30 }, // 6
										  { 0x01, 0x71, 0x09, 0x05, 0x03 }, // 7
										  { 0x36, 0x49, 0x49, 0x49, 0x36 }, // 8
										  { 0x06, 0x49, 0x49, 0x29, 0x1E }, // 9
										  { 0x00, 0x36, 0x36, 0x00, 0x00 }, // :
										  { 0x00, 0x56, 0x36, 0x00, 0x00 }, // ;
										  { 0x08, 0x14, 0x22, 0x41, 0x00 }, // <
										  { 0x14, 0x14, 0x14, 0x14, 0x14 }, // =
										  { 0x00, 0x41, 0x22, 0x14, 0x08 }, // >
										  { 0x02, 0x01, 0x51, 0x09, 0x06 }, // ?
										  { 0x32, 0x49, 0x59, 0x51, 0x3E }, // @
										  { 0x7E, 0x11, 0x11, 0x11, 0x7E }, // A
										  { 0x7F, 0x49, 0x49, 0x49, 0x36 }, // B
										  { 0x3E, 0x41, 0x41, 0x41, 0x22 }, // C
										  { 0x7F, 0x41, 0x41, 0x22, 0x1C }, // D
										  { 0x7F, 0x49, 0x49, 0x49, 0x41 }, // E
										  { 0x7F, 0x09, 0x09, 0x09, 0x01 }, // F
										  { 0x3E, 0x41, 0x49, 0x49, 0x7A }, // G
										  { 0x7F, 0x08, 0x08, 0x08, 0x7F }, // H
										  { 0x00, 0x41, 0x7F, 0x41, 0x00 }, // I
										  { 0x20, 0x40, 0x41, 0x3F, 0x01 }, // J
										  { 0x7F, 0x08, 0x14, 0x22, 0x41 }, // K
										  { 0x7F, 0x40, 0x40, 0x40, 0x40 }, // L
										  { 0x7F, 0x02, 0x0C, 0x02, 0x7F }, // M
										  { 0x7F, 0x04, 0x08, 0x10, 0x7F }, // N
										  { 0x3E, 0x41, 0x41, 0x41, 0x3E }, // O
										  { 0x7F, 0x09, 0x09, 0x09, 0x06 }, // P
										  { 0x3E, 0x41, 0x51, 0x21, 0x5E }, // Q
										  { 0x7F, 0x09, 0x19, 0x29, 0x46 },	// R
										  { 0x46, 0x49, 0x49, 0x49, 0x31 }, // S
										  { 0x01, 0x01, 0x7F, 0x01, 0x01 }, // T
										  { 0x3F, 0x40, 0x40, 0x40, 0x3F }, // U
										  { 0x1F, 0x20, 0x40, 0x20, 0x1F }, // V
										  { 0x3F, 0x40, 0x38, 0x40, 0x3F }, // W
										  { 0x63, 0x14, 0x08, 0x14, 0x63 }, // X
										  { 0x07, 0x08, 0x70, 0x08, 0x07 }, // Y
										  { 0x61, 0x51, 0x49, 0x45, 0x43 }, // Z
										  { 0x00, 0x7F, 0x41, 0x41, 0x00 }, // [
										  { 0x55, 0x2A, 0x55, 0x2A, 0x55 }, // chess
										  { 0x00, 0x41, 0x41, 0x7F, 0x00 }, // ]
										  { 0x04, 0x02, 0x01, 0x02, 0x04 }, // ^
										  { 0x40, 0x40, 0x40, 0x40, 0x40 }, // _
										  { 0x00, 0x01, 0x02, 0x04, 0x00 }, // '
										  { 0x20, 0x54, 0x54, 0x54, 0x78 }, // a
										  { 0x7F, 0x48, 0x44, 0x44, 0x38 }, // b
										  { 0x38, 0x44, 0x44, 0x44, 0x00 }, // c
										  { 0x38, 0x44, 0x44, 0x48, 0x7F }, // d
										  { 0x38, 0x54, 0x54, 0x54, 0x18 }, // e
										  { 0x08, 0x7E, 0x09, 0x01, 0x02 }, // f
										  { 0x48, 0x54, 0x54, 0x54, 0x3C }, // g
										  { 0x7F, 0x08, 0x04, 0x04, 0x78 }, // h
										  { 0x00, 0x44, 0x7D, 0x40, 0x00 }, // i
										  { 0x20, 0x40, 0x44, 0x3D, 0x00 }, // j
										  { 0x7E, 0x10, 0x28, 0x44, 0x00 }, // k
										  { 0x00, 0x41, 0x7F, 0x40, 0x00 }, // l
										  { 0x7C, 0x04, 0x18, 0x04, 0x78 }, // m
										  { 0x7C, 0x08, 0x04, 0x04, 0x78 }, // n
										  { 0x38, 0x44, 0x44, 0x44, 0x38 }, // o
										  { 0x7C, 0x14, 0x14, 0x14, 0x08 }, // p
										  { 0x08, 0x14, 0x14, 0x18, 0x7C }, // q
										  { 0x7C, 0x08, 0x04, 0x04, 0x08 }, // r
										  { 0x48, 0x54, 0x54, 0x54, 0x20 }, // s
										  { 0x04, 0x3F, 0x44, 0x40, 0x20 }, // t
										  { 0x3C, 0x40, 0x40, 0x20, 0x7C }, // u
										  { 0x1C, 0x20, 0x40, 0x20, 0x1C }, // v
										  { 0x3C, 0x40, 0x30, 0x40, 0x3C }, // w
										  { 0x44, 0x28, 0x10, 0x28, 0x44 }, // x
										  { 0x0C, 0x50, 0x50, 0x50, 0x3C }, // y
										  { 0x44, 0x64, 0x54, 0x4C, 0x44 }, // z
										  { 0x00, 0x08, 0x36, 0x41, 0x41 }, // {
										  { 0x00, 0x00, 0x7F, 0x00, 0x00 }, // |
										  { 0x41, 0x41, 0x36, 0x08, 0x00 }, // }
										  { 0x02, 0x01, 0x02, 0x04, 0x02 }	// ~
																					};

/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
****************************************************************************/


/****************************************************************************/

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
void bolyminDisplayWriteCommand(const uint8_t *pi8Cmd, uint32_t ui32Count)
{
    //
    // Wait for any previous SSI operation to finish.
    //
    while(ROM_SSIBusy(DISPLAY_SSI_BASE))
    {
    }

    //
    // Set the D/C pin low to indicate command
    //
    ROM_GPIOPinWrite(DISPLAY_D_C_PORT, DISPLAY_D_C_PIN, 0);

    //
    // Send all the command bytes to the display
    //
    while(ui32Count--)
    {
        ROM_SSIDataPut(DISPLAY_SSI_BASE, *pi8Cmd);
        pi8Cmd++;
    }
}

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
void bolyminDisplayWriteData(const uint8_t *pi8Data, uint32_t ui32Count)
{
    //
    // Wait for any previous SSI operation to finish.
    //
    while(ROM_SSIBusy(DISPLAY_SSI_BASE))
    {
    }

    //
    // Set the D/C pin high to indicate data
    //
    ROM_GPIOPinWrite(DISPLAY_D_C_PORT, DISPLAY_D_C_PIN, DISPLAY_D_C_PIN);

    //
    // Send all the data bytes to the display
    //
    while(ui32Count--)
    {
        ROM_SSIDataPut(DISPLAY_SSI_BASE, (*pi8Data));
        pi8Data++;
    }
}

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
void GrPixelDrawBolymin(uint8_t lucX, uint8_t lucY, bool lbFillColor, bool lGlowColumn)
{
	uint8_t ui8Cmd[7];
	uint8_t ui8Value = 0;
    //
    // Load column command, start and end column
    //
    ui8Cmd[0] = 0x15;
    ui8Cmd[1] = lucX/2;
    ui8Cmd[2] = lucX/2;

    //
    // Load row command, start and end row
    //
    ui8Cmd[3] = 0x75;
    ui8Cmd[4] = lucY;
    ui8Cmd[5] = lucY;

//    ui8Cmd[6] = 0x5C; //Removed - 11 Jul 14

    //
    // Send the column, row commands to the display
    //
    bolyminDisplayWriteCommand(ui8Cmd, 6);

    if(lGlowColumn == true)
    {
        if(lbFillColor == true)
        	ui8Value = 0xFF;
        else
        	ui8Value = 0x00;
    }
    else
    {
		if(lbFillColor == true)
			ui8Value = 0xF0;
		else
			ui8Value = 0x0F;
    }

    //
    // Send the data value representing the pixel to the display
    //
    bolyminDisplayWriteData(&ui8Value, 1);
}

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
/*void GrLineDrawHorizontalBolymin(uint8_t lucX1, uint8_t lucX2, uint8_t lucY, bool lbFillColor)
{
    uint8_t uix = 0;

    for(uix = lucX1 ; uix <= lucX2; uix++)
    {
    	GrPixelDrawBolymin(uix, lucY, lbFillColor, true);
    }
}*/

void GrLineDrawHorizontalBolymin(uint8_t lucX1, uint8_t lucX2, uint8_t lucY, bool lbFillColor)
{
    uint8_t ui8Cmd[8];
    uint8_t i;
    uint8_t ui8Value;
    uint8_t lucXMin, lucXMax;

    //
    // Send command for starting row and column.  Also, set vertical
    // address increment.
    //
    ui8Cmd[0] = 0x15;
    ui8Cmd[1] = (lucX1 < lucX2 ? lucX1 : lucX2)/2;
    ui8Cmd[2] = 63;
    ui8Cmd[3] = 0x75;
    ui8Cmd[4] = lucY;
    ui8Cmd[5] = 63;
    ui8Cmd[6] = 0xA0;
    ui8Cmd[7] = 0x51;
    bolyminDisplayWriteCommand(ui8Cmd, 8);

    if(lbFillColor == true)
    	ui8Value = 0xFF;
    else
    	ui8Value = 0x00;

    lucXMin = (lucX1<lucX2?lucX1:lucX2)/2;
    lucXMax = (lucX1<lucX2?lucX2:lucX1)/2;

    for(i = lucXMin ; i <= lucXMax; i++)
    {
    	bolyminDisplayWriteData(&ui8Value, 1);
    }

    //
    // Set vertical address increment again
    //
    ui8Cmd[0] = 0xA0;
    ui8Cmd[1] = 0x55;
    bolyminDisplayWriteCommand(ui8Cmd, 2);
}

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
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrLineDrawVerticalBolymin(uint8_t lucX, uint8_t lucY1, uint8_t lucY2, bool lbFillColor, bool lGlowColumn)
{
    uint8_t ui8Cmd[8];
    uint8_t i;
    uint8_t ui8Value;
    uint8_t lucYMin, lucYMax;

    //
    // Send command for starting row and column.  Also, set vertical
    // address increment.
    //
    ui8Cmd[0] = 0x15;
    ui8Cmd[1] = lucX/2;
    ui8Cmd[2] = 63;
    ui8Cmd[3] = 0x75;
    ui8Cmd[4] = lucY1 < lucY2 ? lucY1 : lucY2;
    ui8Cmd[5] = 63;
    ui8Cmd[6] = 0xA0;
    ui8Cmd[7] = 0x55;
    bolyminDisplayWriteCommand(ui8Cmd, 8);

    if(lGlowColumn == true)
    {
        if(lbFillColor == true)
        	ui8Value = 0xFF;
        else
        	ui8Value = 0x00;
    }
    else
    {
		if(lbFillColor == true)
			ui8Value = 0xF0;
		else
			ui8Value = 0x0F;
    }

    lucYMin = lucY1<lucY2?lucY1:lucY2;
    lucYMax = lucY1<lucY2?lucY2:lucY1;

    for(i = lucYMin ; i <= lucYMax; i++)
    {
    	bolyminDisplayWriteData(&ui8Value, 1);
    }
}

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
void GrRectFIllBolymin(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax, bool lbFillColor, bool lGlowRightmostColumn)
{
    uint8_t ui8Cmd[8];
    uint8_t i,j;
    uint8_t ui8ColorValue;

    if(lbFillColor == true)
    	ui8ColorValue = 0xFF;
    else
    	ui8ColorValue = 0x00;

    for(i = lucXMin/2; i <= lucXMax/2; i++)
    {
        //
        // Send command for starting row and column.  Also, set vertical
        // address increment.
        //
        ui8Cmd[0] = 0x15;
        ui8Cmd[1] = i;
        ui8Cmd[2] = 63;
        ui8Cmd[3] = 0x75;
        ui8Cmd[4] = lucYMin;
        ui8Cmd[5] = 63;
        ui8Cmd[6] = 0xE3;
        ui8Cmd[7] = 0xE3;
        bolyminDisplayWriteCommand(ui8Cmd, 8);

        for(j = lucYMin ; j <= lucYMax; j++)
        {
        	bolyminDisplayWriteData(&ui8ColorValue, 1);
        }

    }

    if(lGlowRightmostColumn == false)
    	GrLineDrawVerticalBolymin(lucXMax, lucYMin, lucYMax, ui8ColorValue, false);
}

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
void GrRectDrawBolymin(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax, bool lbFillColor)
{
    //
    // Draw a line up the left side of the rectangle.
    //
    GrLineDrawVerticalBolymin(lucXMin, lucYMax, lucYMin, lbFillColor, false);

    //
    // Return if the rectangle is one pixel wide.
    //
    if(lucXMin == lucXMax)
    {
        return;
    }

	//
	// Draw a line across top of rectangle
	//
	GrLineDrawHorizontalBolymin(lucXMin, lucXMax, lucYMin, lbFillColor);

	//
	// Return if rectangle is one pixel tall
	//
	if(lucYMin == lucYMax)
	{
		return;
	}

    //
    // Draw a line across the bottom of the rectangle.
    //
    GrLineDrawHorizontalBolymin(lucXMin, lucXMax, lucYMax, lbFillColor);

    //
    // Return if the rectangle is two pixels tall.
    //
    if((lucYMin + 1) == lucYMax)
    {
        return;
    }

    //
    // Draw a line down the right side of the rectangle.
    //
	GrLineDrawVerticalBolymin(lucXMax, lucYMin, lucYMax, lbFillColor, false);
}

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
             unsigned char lucX,unsigned char lucY, bool lbFillColor)
{
	
		uint8_t ui8Cmd[9];
		unsigned char lucScanAllChar;
		unsigned char lucSendHozBytePerChar;
		unsigned char lucSendVerBytePerChar;
		
		unsigned char lucTempFont[3][8];
		unsigned char lucHozPixelData1,lucHozPixelData2,lucHozPixelDataIndex;
		unsigned char lucBolyminPixelData;
		
		unsigned char i,j;
		unsigned char luctemp;
	    //
	    // Load column command, start and end column
	    //
	    ui8Cmd[0] = 0x15;
	    ui8Cmd[1] = lucX / 2;
	    ui8Cmd[2] = (lucX / 2) + (lucLength * 3) - 1;

	    //
	    // Load row command, start and end row
	    //
	    ui8Cmd[3] = 0x75;
	    ui8Cmd[4] = lucY;
	    ui8Cmd[5] = lucY + 7;

	    ui8Cmd[6] = 0x5C;

	    ui8Cmd[7] = 0xA0;
	    ui8Cmd[8] = 0x55;

	    //
	    // Send the column, row commands to the display
	    //
	    bolyminDisplayWriteCommand(ui8Cmd, 9);
	    
	    //
	    // convert input char from string to 'lucTempFont' in a format supported by the OLED
	    // Send the data value representing the pixel to the display (cosidering data updated vertically)
	    for (lucScanAllChar = 0 ;lucScanAllChar < lucLength;lucScanAllChar++)
	    {
	    	
	    	//convert input char from string to 'lucTempFont' in a format supported by the OLED
	    	lucHozPixelDataIndex = 0;
	    	for (i = 0 ;i < 3;i++)
	    	{
	    		
	    		lucHozPixelData1 = font_5x7[(*(plucBolyminPixelData + lucScanAllChar)) - 32][lucHozPixelDataIndex++];
	    		 
	    		if (lucHozPixelDataIndex < 5)
	    		{
	    		lucHozPixelData2 = font_5x7[(*(plucBolyminPixelData + lucScanAllChar)) - 32][lucHozPixelDataIndex++];
	    		}
	    		else
	    		{
	    		lucHozPixelData2 = 0;	
	    		}
	    		

	    		for (j = 0 ;j < 8;j++)
	    		{
	    			if(lbFillColor == false)
	    			{
						lucBolyminPixelData = 0xFF;

						if (lucHozPixelData1 & 0x01)
						{
							lucBolyminPixelData = 0x0F;
						}

						if (lucHozPixelData2 & 0x01)
						{
							lucBolyminPixelData &= 0xF0;
						}
	    			}
	    			else
	    			{
						lucBolyminPixelData = 0x00;

						if (lucHozPixelData1 & 0x01)
						{
							lucBolyminPixelData = 0xF0;
						}

						if (lucHozPixelData2 & 0x01)
						{
							lucBolyminPixelData |= 0x0F;
						}
	    			}
					
					lucHozPixelData1 = lucHozPixelData1 >> 1;
					lucHozPixelData2 = lucHozPixelData2 >> 1;
					
					lucTempFont[i][j] = lucBolyminPixelData;
	    		
	    		} //for (j = 0 ;j < 8;j++)
	    		
	    	} //for (i = 0 ;i < 3;i++)
	    	
	    	// Send the data value representing the pixel to the display (cosidering data updated vertically)
	    	for (lucSendHozBytePerChar = 0 ;lucSendHozBytePerChar < 3;lucSendHozBytePerChar++)	
	    	{

	    		for (lucSendVerBytePerChar = 0 ;lucSendVerBytePerChar < 8;lucSendVerBytePerChar++)
		    	{
		    	
	    			luctemp = lucTempFont[lucSendHozBytePerChar][lucSendVerBytePerChar];
	    			bolyminDisplayWriteData(&luctemp, 1);
		    		//CFAL96x64x16WriteData(lucTempFont[lucSendHozBytePerChar][lucSendVerBytePerChar], 1);
		    		
		    	} // for (lucSendVerBytePerChar = 0 ;lucSendVerBytePerChar < 8;lucSendVerBytePerChar++)

	    		
	    	} // for (lucSendHozBytePerChar = 0 ;lucSendHozBytePerChar < 3;lucSendHozBytePerChar++)
	    	
	    	
	    } // for (lucScanAllChar = 0 ;lucScanAllChar < lucLength;lucScanAllChar++)
	    

} //GrStringDraw


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
void bolyminDisplayInit(void)
{
    uint8_t DispON = 0xAF;

    //
    // Enable the peripherals used by this driver
    //
    ROM_SysCtlPeripheralEnable(DISPLAY_SSI_PERIPH);
    ROM_SysCtlPeripheralEnable(DISPLAY_SSI_GPIO_PERIPH);
    ROM_SysCtlPeripheralEnable(DISPLAY_RST_GPIO_PERIPH);

    //
    // Select the SSI function for the appropriate pins
    //
    ROM_GPIOPinConfigure(DISPLAY_PINCFG_SSICLK);
    ROM_GPIOPinConfigure(DISPLAY_PINCFG_SSIFSS);
    ROM_GPIOPinConfigure(DISPLAY_PINCFG_SSITX);

    //
    // Configure the pins for the SSI function
    //
    ROM_GPIOPinTypeSSI(DISPLAY_SSI_PORT, DISPLAY_SSI_PINS);

    //
    // Configure display control pins as GPIO output
    //
    ROM_GPIOPinTypeGPIOOutput(DISPLAY_RST_PORT, DISPLAY_RST_PIN);
    ROM_GPIOPinTypeGPIOOutput(DISPLAY_ENV_PORT, DISPLAY_ENV_PIN);
    ROM_GPIOPinTypeGPIOOutput(DISPLAY_D_C_PORT, DISPLAY_D_C_PIN);

    //
    // Reset pin high, power off
    //
    ROM_GPIOPinWrite(DISPLAY_RST_PORT, DISPLAY_RST_PIN, DISPLAY_RST_PIN);
    ROM_GPIOPinWrite(DISPLAY_ENV_PORT, DISPLAY_ENV_PIN, 0);

    ROM_SysCtlDelay(1000);


    //
    // Drive the reset pin low while we do other stuff
    //
    ROM_GPIOPinWrite(DISPLAY_RST_PORT, DISPLAY_RST_PIN, 0);

    //
    // Configure the SSI port
    //
    ROM_SSIDisable(DISPLAY_SSI_BASE);
    ROM_SSIConfigSetExpClk(DISPLAY_SSI_BASE, ROM_SysCtlClockGet(),
                           SSI_FRF_MOTO_MODE_3, SSI_MODE_MASTER,
                           DISPLAY_SSI_CLOCK, 8);
    ROM_SSIEnable(DISPLAY_SSI_BASE);

    //
    // Take the display out of reset
    //
    ROM_SysCtlDelay(1000);
    ROM_GPIOPinWrite(DISPLAY_RST_PORT, DISPLAY_RST_PIN, DISPLAY_RST_PIN);
    ROM_SysCtlDelay(1000);

    //
    // Enable display power supply
    //
    ROM_GPIOPinWrite(DISPLAY_ENV_PORT, DISPLAY_ENV_PIN, DISPLAY_ENV_PIN);
    ROM_SysCtlDelay(1000);

    //
    // Send the initial configuration command bytes to the display
    //
    bolyminDisplayWriteCommand(g_ui8DisplayInitCommands,
                             sizeof(g_ui8DisplayInitCommands));
    //ROM_SysCtlDelay(1000);
    ROM_SysCtlDelay(1000000);
    ROM_SysCtlDelay(1000000);

    //
    // Fill the entire display with a black rectangle, to clear it.
    //
    GrRectFIllBolymin(0, 126, 0, 63, false, true);

    //
    // Send Display On command
    //
    bolyminDisplayWriteCommand(&DispON, 1);
}

/********************************************************************************/
#endif



















#ifndef OLED_LCD_SELET
/****************************************************************************
 *  Macro definitions:
****************************************************************************/
//*****************************************************************************
//
// Defines the SSI and GPIO peripherals that are used for this display.
//
#define DISPLAY_RST_GPIO_PERIPH     SYSCTL_PERIPH_GPIOB


//*****************************************************************************
//
// Defines the port and pins for the display voltage enable signal.
//
//*****************************************************************************
#define DISPLAY_ENV_PORT            GPIO_PORTB_BASE
#define DISPLAY_ENV_PIN             GPIO_PIN_3

//*****************************************************************************
//
// Defines the port and pins for the display reset signal.
//
//*****************************************************************************
#define DISPLAY_RST_PORT            GPIO_PORTB_BASE
#define DISPLAY_RST_PIN             GPIO_PIN_2

//*****************************************************************************
//
// Defines the port and pins for the display Data/Command (D/C) signal.
//
//*****************************************************************************
#define DISPLAY_D_C_PORT            GPIO_PORTB_BASE
#define DISPLAY_D_C_PIN             GPIO_PIN_7


//*****************************************************************************
//
// Defines the port and pins for the display SSI peripheral signal.
//
//*****************************************************************************
#define DISPLAY_SDIN_PORT            GPIO_PORTB_BASE
#define DISPLAY_SDIN_PIN             GPIO_PIN_6

#define DISPLAY_SCLK_PORT            GPIO_PORTB_BASE
#define DISPLAY_SCLK_PIN             GPIO_PIN_4

#define DISPLAY_CS_PORT              GPIO_PORTB_BASE
#define DISPLAY_CS_PIN               GPIO_PIN_5


// Defines a 5x7 font
unsigned char const font_5x7[][5] = {
                                          { 0x00, 0x00, 0x00, 0x00, 0x00 }, // space 0x20  32 sub-0
                                          { 0x00, 0x00, 0x2f, 0x00, 0x00 }, // !
                                          { 0x00, 0x07, 0x00, 0x07, 0x00 }, // "
                                          { 0x14, 0x7f, 0x14, 0x7f, 0x14 }, // #
                                          { 0x24, 0x2a, 0x7f, 0x2a, 0x12 }, // $
                                          { 0xc4, 0xc8, 0x10, 0x26, 0x46 }, // %
                                          { 0x36, 0x49, 0x55, 0x22, 0x50 }, // &
                                          { 0x00, 0x05, 0x03, 0x00, 0x00 }, // '
                                          { 0x00, 0x1c, 0x22, 0x41, 0x00 }, // (
                                          { 0x00, 0x41, 0x22, 0x1c, 0x00 }, // )
                                          { 0x14, 0x08, 0x3E, 0x08, 0x14 }, // *
                                          { 0x08, 0x08, 0x3E, 0x08, 0x08 }, // +
                                          { 0x00, 0x00, 0x50, 0x30, 0x00 }, // ,
                                          { 0x10, 0x10, 0x10, 0x10, 0x10 }, // -
                                          { 0x00, 0x60, 0x60, 0x00, 0x00 }, // .
                                          { 0x20, 0x10, 0x08, 0x04, 0x02 }, // /
                                          { 0x3E, 0x51, 0x49, 0x45, 0x3E }, // 0
                                          { 0x00, 0x42, 0x7F, 0x40, 0x00 }, // 1
                                          { 0x42, 0x61, 0x51, 0x49, 0x46 }, // 2
                                          { 0x21, 0x41, 0x45, 0x4B, 0x31 }, // 3
                                          { 0x18, 0x14, 0x12, 0x7F, 0x10 }, // 4
                                          { 0x27, 0x45, 0x45, 0x45, 0x39 }, // 5
                                          { 0x3C, 0x4A, 0x49, 0x49, 0x30 }, // 6
                                          { 0x01, 0x71, 0x09, 0x05, 0x03 }, // 7
                                          { 0x36, 0x49, 0x49, 0x49, 0x36 }, // 8
                                          { 0x06, 0x49, 0x49, 0x29, 0x1E }, // 9
                                          { 0x00, 0x36, 0x36, 0x00, 0x00 }, // :
                                          { 0x00, 0x56, 0x36, 0x00, 0x00 }, // ;
                                          { 0x08, 0x14, 0x22, 0x41, 0x00 }, // <
                                          { 0x14, 0x14, 0x14, 0x14, 0x14 }, // =
                                          { 0x00, 0x41, 0x22, 0x14, 0x08 }, // >
                                          { 0x02, 0x01, 0x51, 0x09, 0x06 }, // ?
                                          { 0x32, 0x49, 0x59, 0x51, 0x3E }, // @
                                          { 0x7E, 0x11, 0x11, 0x11, 0x7E }, // A
                                          { 0x7F, 0x49, 0x49, 0x49, 0x36 }, // B
                                          { 0x3E, 0x41, 0x41, 0x41, 0x22 }, // C
                                          { 0x7F, 0x41, 0x41, 0x22, 0x1C }, // D
                                          { 0x7F, 0x49, 0x49, 0x49, 0x41 }, // E
                                          { 0x7F, 0x09, 0x09, 0x09, 0x01 }, // F
                                          { 0x3E, 0x41, 0x49, 0x49, 0x7A }, // G
                                          { 0x7F, 0x08, 0x08, 0x08, 0x7F }, // H
                                          { 0x00, 0x41, 0x7F, 0x41, 0x00 }, // I
                                          { 0x20, 0x40, 0x41, 0x3F, 0x01 }, // J
                                          { 0x7F, 0x08, 0x14, 0x22, 0x41 }, // K
                                          { 0x7F, 0x40, 0x40, 0x40, 0x40 }, // L
                                          { 0x7F, 0x02, 0x0C, 0x02, 0x7F }, // M
                                          { 0x7F, 0x04, 0x08, 0x10, 0x7F }, // N
                                          { 0x3E, 0x41, 0x41, 0x41, 0x3E }, // O
                                          { 0x7F, 0x09, 0x09, 0x09, 0x06 }, // P
                                          { 0x3E, 0x41, 0x51, 0x21, 0x5E }, // Q
                                          { 0x7F, 0x09, 0x19, 0x29, 0x46 }, // R
                                          { 0x46, 0x49, 0x49, 0x49, 0x31 }, // S
                                          { 0x01, 0x01, 0x7F, 0x01, 0x01 }, // T
                                          { 0x3F, 0x40, 0x40, 0x40, 0x3F }, // U
                                          { 0x1F, 0x20, 0x40, 0x20, 0x1F }, // V
                                          { 0x3F, 0x40, 0x38, 0x40, 0x3F }, // W
                                          { 0x63, 0x14, 0x08, 0x14, 0x63 }, // X
                                          { 0x07, 0x08, 0x70, 0x08, 0x07 }, // Y
                                          { 0x61, 0x51, 0x49, 0x45, 0x43 }, // Z
                                          { 0x00, 0x7F, 0x41, 0x41, 0x00 }, // [
                                          { 0x55, 0x2A, 0x55, 0x2A, 0x55 }, // chess
                                          { 0x00, 0x41, 0x41, 0x7F, 0x00 }, // ]
                                          { 0x04, 0x02, 0x01, 0x02, 0x04 }, // ^
                                          { 0x40, 0x40, 0x40, 0x40, 0x40 }, // _
                                          { 0x00, 0x01, 0x02, 0x04, 0x00 }, // '
                                          { 0x20, 0x54, 0x54, 0x54, 0x78 }, // a
                                          { 0x7F, 0x48, 0x44, 0x44, 0x38 }, // b
                                          { 0x38, 0x44, 0x44, 0x44, 0x00 }, // c
                                          { 0x38, 0x44, 0x44, 0x48, 0x7F }, // d
                                          { 0x38, 0x54, 0x54, 0x54, 0x18 }, // e
                                          { 0x08, 0x7E, 0x09, 0x01, 0x02 }, // f
                                          { 0x48, 0x54, 0x54, 0x54, 0x3C }, // g
                                          { 0x7F, 0x08, 0x04, 0x04, 0x78 }, // h
                                          { 0x00, 0x44, 0x7D, 0x40, 0x00 }, // i
                                          { 0x20, 0x40, 0x44, 0x3D, 0x00 }, // j
                                          { 0x7E, 0x10, 0x28, 0x44, 0x00 }, // k
                                          { 0x00, 0x41, 0x7F, 0x40, 0x00 }, // l
                                          { 0x7C, 0x04, 0x18, 0x04, 0x78 }, // m
                                          { 0x7C, 0x08, 0x04, 0x04, 0x78 }, // n
                                          { 0x38, 0x44, 0x44, 0x44, 0x38 }, // o
                                          { 0x7C, 0x14, 0x14, 0x14, 0x08 }, // p
                                          { 0x08, 0x14, 0x14, 0x18, 0x7C }, // q
                                          { 0x7C, 0x08, 0x04, 0x04, 0x08 }, // r
                                          { 0x48, 0x54, 0x54, 0x54, 0x20 }, // s
                                          { 0x04, 0x3F, 0x44, 0x40, 0x20 }, // t
                                          { 0x3C, 0x40, 0x40, 0x20, 0x7C }, // u
                                          { 0x1C, 0x20, 0x40, 0x20, 0x1C }, // v
                                          { 0x3C, 0x40, 0x30, 0x40, 0x3C }, // w
                                          { 0x44, 0x28, 0x10, 0x28, 0x44 }, // x
                                          { 0x0C, 0x50, 0x50, 0x50, 0x3C }, // y
                                          { 0x44, 0x64, 0x54, 0x4C, 0x44 }, // z
                                          { 0x00, 0x08, 0x36, 0x41, 0x41 }, // {
                                          { 0x00, 0x00, 0x7F, 0x00, 0x00 }, // |
                                          { 0x41, 0x41, 0x36, 0x08, 0x00 }, // }
                                          { 0x02, 0x01, 0x02, 0x04, 0x02 },  // SUB 94
										  {0x01,0x41,0x3D,0x09,0x07},/*0 sub-95*/
										  {0x10,0x08,0x7C,0x02,0x01},/*1*/
										  {0x0E,0x02,0x43,0x22,0x1E},/*2*/
										  {0x42,0x42,0x7E,0x42,0x42},/*3*/
										  {0x22,0x12,0x0A,0x7F,0x02},/*4*/
										  {0x42,0x3F,0x02,0x42,0x3E},/*5*/
										  {0x0A,0x0A,0x7F,0x0A,0x0A},/*6*/
										  {0x08,0x06,0x42,0x22,0x1E},/*7*/
										  {0x04,0x03,0x42,0x3E,0x02},/*8*/
										  {0x42,0x42,0x42,0x42,0x7E},/*9*/
										  {0x02,0x4F,0x22,0x1F,0x02},/*10*/
										  {0x4A,0x4A,0x40,0x20,0x1C},/*11*/
										  {0x42,0x22,0x12,0x2A,0x46},/*12*/
										  {0x02,0x3F,0x42,0x4A,0x46},/*13*/
										  {0x06,0x48,0x40,0x20,0x1E},/*14*/
										  {0x08,0x46,0x4A,0x32,0x1E},/*15*/
										  {0x0A,0x4A,0x3E,0x09,0x08},/*16*/
										  {0x0E,0x00,0x4E,0x20,0x1E},/*17*/
										  {0x04,0x45,0x3D,0x05,0x04},/*18*/
										  {0x00,0x7F,0x08,0x10,0x00},/*19*/
										  {0x44,0x24,0x1F,0x04,0x04},/*20*/
										  {0x40,0x42,0x42,0x42,0x40},/*21*/
										  {0x42,0x2A,0x12,0x2A,0x06},/*22*/
										  {0x22,0x12,0x7B,0x16,0x22},/*23*/
										  {0x00,0x40,0x20,0x1F,0x00},/*24*/
										  {0x78,0x00,0x02,0x04,0x78},/*25*/
										  {0x3F,0x44,0x44,0x44,0x44},/*26*/
										  {0x02,0x42,0x42,0x22,0x1E},/*27*/
										  {0x04,0x02,0x04,0x08,0x30},/*28*/
										  {0x32,0x02,0x7F,0x02,0x32},/*29*/
										  {0x02,0x12,0x22,0x52,0x0E},/*30*/
										  {0x00,0x2A,0x2A,0x2A,0x40},/*31*/
										  {0x38,0x24,0x22,0x20,0x70},/*32*/
										  {0x40,0x28,0x10,0x28,0x06},/*33*/
										  {0x0A,0x3E,0x4A,0x4A,0x4A},/*34*/
										  {0x04,0x7F,0x04,0x14,0x0C},/*35*/
										  {0x40,0x42,0x42,0x7E,0x40},/*36*/
										  {0x4A,0x4A,0x4A,0x4A,0x7E},/*37*/
										  {0x04,0x05,0x45,0x25,0x1C},/*38*/
										  {0x0F,0x40,0x20,0x1F,0x00},/*39*/
										  {0x7C,0x00,0x7E,0x40,0x30},/*40*/
										  {0x7E,0x40,0x20,0x10,0x08},/*41*/
										  {0x7E,0x42,0x42,0x42,0x7E},/*42*/
										  {0x06,0x02,0x42,0x22,0x1E},/*43*/
										  {0x0A,0x0A,0x4A,0x2A,0x1E},/*44*/
										  {0x42,0x42,0x40,0x20,0x18},/*45*/
										  {0x02,0x04,0x01,0x02,0x00},/*46*/
										  {0x07,0x05,0x07,0x00,0x00},/*47*/
										  {0x70,0x40,0x40,0x40,0x70},/*48*/
										  {0x70,0x50,0x70,0x00,0x00},/*49*/
										  {0x00,0x00,0x07,0x01,0x01},/*50*/
										  {0x40,0x40,0x70,0x00,0x00},/*51*/
										  {0x10,0x20,0x40,0x00,0x00},/*52*/
										  {0x00,0x00,0x00,0x18,0x18},/*53*/
										  {0x44,0x3E,0x04,0x45,0x3D},/*54*/
										  {0x14,0x14,0x7E,0x15,0x15},/*55*/
										  {0x08,0x44,0x44,0x25,0x1D},/*56*/
										  {0x08,0x06,0x44,0x3D,0x05},/*57*/
										  {0x44,0x44,0x44,0x45,0x7D},/*58*/
										  {0x08,0x1E,0x48,0x3D,0x09},/*59*/
										  {0x54,0x54,0x40,0x21,0x1D},/*60*/
										  {0x44,0x24,0x14,0x2D,0x45},/*61*/
										  {0x04,0x3E,0x44,0x4D,0x45},/*62*/
										  {0x0C,0x50,0x40,0x21,0x1D},/*63*/
										  {0x10,0x4C,0x54,0x25,0x1D},/*64*/
										  {0x10,0x54,0x3C,0x15,0x11},/*65*/
										  {0x0C,0x40,0x4C,0x21,0x1D},/*66*/
										  {0x10,0x54,0x34,0x15,0x11},/*67*/
										  {0x00,0x00,0x7E,0x09,0x11},/*68*/
										  {0x78,0x00,0x0A,0x15,0x62},/*69*/
										  {0x3E,0x48,0x4A,0x4D,0x4A},/*70*/
										  {0x08,0x48,0x4A,0x4D,0x3A},/*71*/
										  {0x10,0x08,0x12,0x25,0x42},/*72*/
										  {0x68,0x08,0x7E,0x0D,0x6A},/*73*/
										  {0x78,0x00,0x04,0x09,0x71},/*74*/
										  {0x3E,0x44,0x44,0x45,0x45},/*75*/
										  {0x04,0x44,0x44,0x25,0x1D},/*76*/
										  {0x08,0x04,0x08,0x11,0x61},/*77*/
										  {0x34,0x04,0x7E,0x05,0x35},/*78*/
										  {0x04,0x44,0x34,0x14,0x0C},/*79*/
										  {0x20,0x10,0x78,0x04,0x00},/*80*/
										  {0x18,0x08,0x4C,0x48,0x38},/*81*/
										  {0x48,0x48,0x78,0x48,0x48},/*82*/
										  {0x48,0x28,0x18,0x7C,0x08},/*83*/
										  {0x08,0x7C,0x08,0x28,0x18},/*84*/
										  {0x40,0x48,0x48,0x78,0x40},/*85*/
										  {0x54,0x54,0x54,0x7C,0x00},/*86*/
										  {0x18,0x00,0x58,0x40,0x38},/*87*/
										  {0x08,0x08,0x08,0x08,0x08},/*88*/
                                          {0x40,0x40,0x40,0x40,0x40}/*89*/
                                                                                    };

unsigned char const font_8x8[][8] = {
                                          { 0x00, 0x00, 0x00, 0x00, 0x00 }, // space 0x20  32 sub-0
                                          { 0x00, 0x00, 0x2f, 0x00, 0x00 }, // !
                                          { 0x00, 0x07, 0x00, 0x07, 0x00 }, // "
                                          { 0x14, 0x7f, 0x14, 0x7f, 0x14 }, // #
                                          { 0x24, 0x2a, 0x7f, 0x2a, 0x12 }, // $
                                          { 0xc4, 0xc8, 0x10, 0x26, 0x46 }, // %
                                          { 0x36, 0x49, 0x55, 0x22, 0x50 }, // &
                                          { 0x00, 0x05, 0x03, 0x00, 0x00 }, // '
                                          { 0x00, 0x1c, 0x22, 0x41, 0x00 }, // (
                                          { 0x00, 0x41, 0x22, 0x1c, 0x00 }, // )
                                          { 0x14, 0x08, 0x3E, 0x08, 0x14 }, // *
                                          { 0x08, 0x08, 0x3E, 0x08, 0x08 }, // +
                                          { 0x00, 0x00, 0x50, 0x30, 0x00 }, // ,
                                          { 0x10, 0x10, 0x10, 0x10, 0x10 }, // -
                                          { 0x00, 0x60, 0x60, 0x00, 0x00 }, // .
                                          { 0x20, 0x10, 0x08, 0x04, 0x02 }, // /
                                          { 0x3E, 0x51, 0x49, 0x45, 0x3E }, // 0
                                          { 0x00, 0x42, 0x7F, 0x40, 0x00 }, // 1
                                          { 0x42, 0x61, 0x51, 0x49, 0x46 }, // 2
                                          { 0x21, 0x41, 0x45, 0x4B, 0x31 }, // 3
                                          { 0x18, 0x14, 0x12, 0x7F, 0x10 }, // 4
                                          { 0x27, 0x45, 0x45, 0x45, 0x39 }, // 5
                                          { 0x3C, 0x4A, 0x49, 0x49, 0x30 }, // 6
                                          { 0x01, 0x71, 0x09, 0x05, 0x03 }, // 7
                                          { 0x36, 0x49, 0x49, 0x49, 0x36 }, // 8
                                          { 0x06, 0x49, 0x49, 0x29, 0x1E }, // 9
                                          { 0x00, 0x36, 0x36, 0x00, 0x00 }, // :
                                          { 0x00, 0x56, 0x36, 0x00, 0x00 }, // ;
                                          { 0x08, 0x14, 0x22, 0x41, 0x00 }, // <
                                          { 0x14, 0x14, 0x14, 0x14, 0x14 }, // =
                                          { 0x00, 0x41, 0x22, 0x14, 0x08 }, // >
                                          { 0x02, 0x01, 0x51, 0x09, 0x06 }, // ?
                                          { 0x32, 0x49, 0x59, 0x51, 0x3E }, // @
                                          { 0x7E, 0x11, 0x11, 0x11, 0x7E }, // A
                                          { 0x7F, 0x49, 0x49, 0x49, 0x36 }, // B
                                          { 0x3E, 0x41, 0x41, 0x41, 0x22 }, // C
                                          { 0x7F, 0x41, 0x41, 0x22, 0x1C }, // D
                                          { 0x7F, 0x49, 0x49, 0x49, 0x41 }, // E
                                          { 0x7F, 0x09, 0x09, 0x09, 0x01 }, // F
                                          { 0x3E, 0x41, 0x49, 0x49, 0x7A }, // G
                                          { 0x7F, 0x08, 0x08, 0x08, 0x7F }, // H
                                          { 0x00, 0x41, 0x7F, 0x41, 0x00 }, // I
                                          { 0x20, 0x40, 0x41, 0x3F, 0x01 }, // J
                                          { 0x7F, 0x08, 0x14, 0x22, 0x41 }, // K
                                          { 0x7F, 0x40, 0x40, 0x40, 0x40 }, // L
                                          { 0x7F, 0x02, 0x0C, 0x02, 0x7F }, // M
                                          { 0x7F, 0x04, 0x08, 0x10, 0x7F }, // N
                                          { 0x3E, 0x41, 0x41, 0x41, 0x3E }, // O
                                          { 0x7F, 0x09, 0x09, 0x09, 0x06 }, // P
                                          { 0x3E, 0x41, 0x51, 0x21, 0x5E }, // Q
                                          { 0x7F, 0x09, 0x19, 0x29, 0x46 }, // R
                                          { 0x46, 0x49, 0x49, 0x49, 0x31 }, // S
                                          { 0x01, 0x01, 0x7F, 0x01, 0x01 }, // T
                                          { 0x3F, 0x40, 0x40, 0x40, 0x3F }, // U
                                          { 0x1F, 0x20, 0x40, 0x20, 0x1F }, // V
                                          { 0x3F, 0x40, 0x38, 0x40, 0x3F }, // W
                                          { 0x63, 0x14, 0x08, 0x14, 0x63 }, // X
                                          { 0x07, 0x08, 0x70, 0x08, 0x07 }, // Y
                                          { 0x61, 0x51, 0x49, 0x45, 0x43 }, // Z
                                          { 0x00, 0x7F, 0x41, 0x41, 0x00 }, // [
                                          { 0x55, 0x2A, 0x55, 0x2A, 0x55 }, // chess
                                          { 0x00, 0x41, 0x41, 0x7F, 0x00 }, // ]
                                          { 0x04, 0x02, 0x01, 0x02, 0x04 }, // ^
                                          { 0x40, 0x40, 0x40, 0x40, 0x40 }, // _
                                          { 0x00, 0x01, 0x02, 0x04, 0x00 }, // '
                                          { 0x20, 0x54, 0x54, 0x54, 0x78 }, // a
                                          { 0x7F, 0x48, 0x44, 0x44, 0x38 }, // b
                                          { 0x38, 0x44, 0x44, 0x44, 0x00 }, // c
                                          { 0x38, 0x44, 0x44, 0x48, 0x7F }, // d
                                          { 0x38, 0x54, 0x54, 0x54, 0x18 }, // e
                                          { 0x08, 0x7E, 0x09, 0x01, 0x02 }, // f
                                          { 0x48, 0x54, 0x54, 0x54, 0x3C }, // g
                                          { 0x7F, 0x08, 0x04, 0x04, 0x78 }, // h
                                          { 0x00, 0x44, 0x7D, 0x40, 0x00 }, // i
                                          { 0x20, 0x40, 0x44, 0x3D, 0x00 }, // j
                                          { 0x7E, 0x10, 0x28, 0x44, 0x00 }, // k
                                          { 0x00, 0x41, 0x7F, 0x40, 0x00 }, // l
                                          { 0x7C, 0x04, 0x18, 0x04, 0x78 }, // m
                                          { 0x7C, 0x08, 0x04, 0x04, 0x78 }, // n
                                          { 0x38, 0x44, 0x44, 0x44, 0x38 }, // o
                                          { 0x7C, 0x14, 0x14, 0x14, 0x08 }, // p
                                          { 0x08, 0x14, 0x14, 0x18, 0x7C }, // q
                                          { 0x7C, 0x08, 0x04, 0x04, 0x08 }, // r
                                          { 0x48, 0x54, 0x54, 0x54, 0x20 }, // s
                                          { 0x04, 0x3F, 0x44, 0x40, 0x20 }, // t
                                          { 0x3C, 0x40, 0x40, 0x20, 0x7C }, // u
                                          { 0x1C, 0x20, 0x40, 0x20, 0x1C }, // v
                                          { 0x3C, 0x40, 0x30, 0x40, 0x3C }, // w
                                          { 0x44, 0x28, 0x10, 0x28, 0x44 }, // x
                                          { 0x0C, 0x50, 0x50, 0x50, 0x3C }, // y
                                          { 0x44, 0x64, 0x54, 0x4C, 0x44 }, // z
                                          { 0x00, 0x08, 0x36, 0x41, 0x41 }, // {
                                          { 0x00, 0x00, 0x7F, 0x00, 0x00 }, // |
                                          { 0x41, 0x41, 0x36, 0x08, 0x00 }, // }
                                          { 0x02, 0x01, 0x02, 0x04, 0x02 },  // SUB 94
										  {0x00,0x01,0x41,0x3D,0x09,0x05,0x03,0x00},/*0 sub-95*/
										  {0x00,0x10,0x10,0x08,0x7C,0x02,0x01,0x00},/*1*/
										  {0x00,0x06,0x42,0x43,0x22,0x12,0x0E,0x00},/*2*/
										  										  {0x20,0x22,0x22,0x3E,0x22,0x22,0x20,0x00},/*3*/
										  										  {0x00,0x22,0x22,0x12,0x4A,0x7F,0x02,0x00},/*4*/
										  										  {0x00,0x42,0x22,0x1F,0x02,0x42,0x7E,0x00},/*5*/
										  										  {0x00,0x12,0x12,0x1F,0x72,0x12,0x10,0x00},/*6*/
										  										  {0x00,0x08,0x44,0x43,0x22,0x12,0x0E,0x00},/*7*/
										  										  {0x08,0x07,0x42,0x22,0x1E,0x02,0x02,0x00},/*8*/
										  										  {0x00,0x42,0x42,0x42,0x42,0x42,0x7E,0x00},/*9*/
										  										  {0x00,0x02,0x4F,0x42,0x22,0x1F,0x02,0x00},/*10*/
										  										  {0x00,0x45,0x4A,0x40,0x20,0x10,0x0C,0x00},/*11*/
										  										  {0x40,0x42,0x22,0x22,0x1A,0x26,0x40,0x00},/*12*/
										  										  {0x04,0x04,0x3F,0x44,0x44,0x54,0x4C,0x00},/*13*/
										  										  {0x00,0x01,0x46,0x40,0x20,0x10,0x0F,0x00},/*14*/
										  										  {0x00,0x08,0x44,0x4B,0x2A,0x12,0x0E,0x00},/*15*/
										  										  {0x08,0x0A,0x4A,0x3E,0x09,0x09,0x08,0x00},/*16*/
										                                            {0x02,0x0C,0x42,0x4C,0x20,0x10,0x0E,0x00},/*17*/
										  										  {0x04,0x05,0x45,0x3D,0x05,0x05,0x04,0x00},/*18*/
										  										  {0x00,0x00,0x7F,0x08,0x08,0x10,0x00,0x00},/*19*/
										  										  {0x04,0x44,0x24,0x1F,0x04,0x04,0x04,0x00},/*20*/
										  										  {0x20,0x22,0x22,0x22,0x22,0x22,0x20,0x00},/*21*/
										  										  {0x00,0x40,0x41,0x25,0x15,0x19,0x27,0x00},/*22*/
										  										  {0x20,0x22,0x12,0x7B,0x06,0x12,0x20,0x00},/*23*/
										  										  {0x40,0x40,0x20,0x10,0x08,0x07,0x00,0x00},/*24*/
										  										  {0x40,0x30,0x0E,0x00,0x02,0x0C,0x70,0x00},/*25*/
										  										  {0x00,0x3F,0x48,0x48,0x48,0x44,0x44,0x00},/*26*/
										  										  {0x00,0x02,0x42,0x42,0x22,0x12,0x0E,0x00},/*27*/
										  										  {0x08,0x04,0x02,0x04,0x08,0x10,0x20,0x00},/*28*/
										  										  {0x24,0x14,0x44,0x7F,0x04,0x14,0x24,0x00},/*29*/
										  										  {0x00,0x02,0x12,0x22,0x52,0x0A,0x06,0x00},/*30*/
										  										  {0x00,0x21,0x25,0x29,0x4A,0x42,0x00,0x00},/*31*/
										  										  {0x40,0x70,0x4C,0x43,0x50,0x20,0x40,0x00},/*32*/
										  										  {0x00,0x40,0x44,0x24,0x14,0x08,0x37,0x00},/*33*/
										  										  {0x00,0x08,0x09,0x3F,0x49,0x49,0x48,0x00},/*34*/
										  										  {0x04,0x04,0x0F,0x74,0x02,0x0A,0x06,0x00},/*35*/
										  										  {0x20,0x22,0x22,0x22,0x3E,0x20,0x20,0x00},/*36*/
										  										  {0x00,0x42,0x4A,0x4A,0x4A,0x4A,0x7E,0x00},/*37*/
										  										  {0x04,0x05,0x45,0x45,0x25,0x15,0x0C,0x00},/*38*/
										  										  {0x00,0x0F,0x40,0x40,0x20,0x1F,0x00,0x00},/*39*/
										  										  {0x40,0x20,0x1E,0x00,0x7F,0x20,0x10,0x00},/*40*/
										  										  {0x00,0x7F,0x40,0x20,0x10,0x08,0x00,0x00},/*41*/
										  										  {0x00,0x7E,0x42,0x42,0x42,0x42,0x7E,0x00},/*42*/
										  										  //{0x00,0x0C,0x44,0x44,0x24,0x1C,0x00,0x00},/*43*/
										  										 {0x00,0x06,0x42,0x42,0x22,0x12,0x0E,0x00},/*43*/
																				 {0x00,0x01,0x45,0x45,0x25,0x15,0x0F,0x00},/*44*/
										  										  {0x00,0x41,0x42,0x40,0x20,0x10,0x0C,0x00},/*45*/
										  {0x02,0x04,0x01,0x02,0x00},/*46*/
										  {0x07,0x05,0x07,0x00,0x00},/*47*/
										  {0x70,0x40,0x40,0x40,0x70},/*48*/
										  {0x70,0x50,0x70,0x00,0x00},/*49*/
										  {0x00,0x00,0x07,0x01,0x01},/*50*/
										  {0x40,0x40,0x70,0x00,0x00},/*51*/
										  {0x10,0x20,0x40,0x00,0x00},/*52*/
										  {0x00,0x00,0x00,0x18,0x18},/*53*/
										  {0x00,0x42,0x22,0x1F,0x02,0x42,0x7F,0x00},/*54*/
										 										  {0x00,0x12,0x12,0x1F,0x72,0x13,0x10,0x00},/*55*/
										 										  {0x00,0x08,0x44,0x43,0x22,0x12,0x0F,0x00},/*56*/
										                                           {0x08,0x07,0x42,0x22,0x1F,0x02,0x03,0x00},/*57*/
										 										  {0x00,0x42,0x42,0x42,0x43,0x42,0x7F,0x00},/*58*/
										 										  {0x00,0x02,0x4F,0x42,0x23,0x1E,0x03,0x00},/*59*/
										 										  {0x00,0x45,0x4A,0x40,0x21,0x10,0x0D,0x00},/*60*/
										 										  {0x40,0x42,0x22,0x22,0x1B,0x26,0x41,0x00},/*61*/
										 										  {0x04,0x04,0x3F,0x44,0x45,0x54,0x4D,0x00},/*62*/
										 										  {0x00,0x01,0x46,0x40,0x20,0x11,0x0F,0x00},/*63*/
										 										  {0x00,0x08,0x44,0x4B,0x2A,0x12,0x0F,0x00},/*64*/
										 										  {0x08,0x0A,0x4A,0x3E,0x09,0x08,0x09,0x00},/*65*/
										 										  {0x02,0x0C,0x42,0x4C,0x21,0x10,0x0D,0x00},/*66*/
										 										  {0x04,0x05,0x45,0x3D,0x05,0x04,0x05,0x00},/*67*/
										 										  {0x00,0x00,0x7F,0x08,0x09,0x10,0x01,0x00},/*68*/
										 										  {0x40,0x30,0x0E,0x00,0x02,0x0D,0x72,0x00},/*69*/
										 										  {0x00,0x3F,0x48,0x48,0x4A,0x45,0x46,0x00},/*70*/
										 										  {0x00,0x02,0x42,0x42,0x22,0x15,0x0E,0x00},/*71*/
										 										  {0x08,0x04,0x02,0x04,0x0A,0x15,0x22,0x00},/*72*/
										 										  {0x24,0x14,0x44,0x7F,0x06,0x15,0x22,0x00},/*73*/
										 										  {0x40,0x30,0x0E,0x00,0x02,0x0D,0x71,0x00},/*74*/
										 										  {0x00,0x3F,0x48,0x48,0x49,0x44,0x45,0x00},/*75*/
										 										  {0x00,0x02,0x42,0x42,0x23,0x12,0x0F,0x00},/*76*/
										 										  {0x08,0x04,0x02,0x04,0x09,0x10,0x21,0x00},/*77*/
										 										  {0x24,0x14,0x44,0x7F,0x05,0x14,0x25,0x00},/*78*/
										 										  {0x00,0x04,0x44,0x3C,0x14,0x0C,0x00,0x00},/*79*/
										 										  {0x00,0x20,0x20,0x10,0x78,0x04,0x00,0x00},/*80*/
										 										  {0x00,0x18,0x48,0x4C,0x28,0x18,0x00,0x00},/*81*/
										 										  {0x00,0x40,0x48,0x78,0x48,0x40,0x00,0x00},/*82*/
										 										  {0x00,0x28,0x28,0x58,0x7C,0x08,0x00,0x00},/*83*/
										 										  {0x00,0x10,0x1C,0x68,0x08,0x18,0x00,0x00},/*84*/
										 										  {0x00,0x40,0x48,0x48,0x78,0x40,0x00,0x00},/*85*/
										 										  {0x00,0x44,0x54,0x54,0x54,0x7C,0x00,0x00},/*86*/
										 										  {0x00,0x18,0x40,0x58,0x20,0x18,0x00,0x00},/*87*/
										 										  {0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},/*88*/
                                          {0x40,0x40,0x40,0x40,0x40}/*89*/
                                                                                    };
// Defines a 5x7 font
//unsigned char const font_8x8[][8] = {
//                                          { 0x00, 0x00, 0x00, 0x00, 0x00 }, // space 0x20  32 sub-0
//										  {0x00,0x00,0x00,0x5F,0x00,0x00,0x00,0x00}, // !
//                                          { 0x00, 0x07, 0x00, 0x07, 0x00 }, // "
//										  {0x10,0x14,0x3C,0x16,0x3C,0x16,0x04,0x00},// #
//                                          { 0x24, 0x2a, 0x7f, 0x2a, 0x12 }, // $
//                                          { 0xc4, 0xc8, 0x10, 0x26, 0x46 }, // %
//                                          { 0x36, 0x49, 0x55, 0x22, 0x50 }, // &
//                                          { 0x00, 0x05, 0x03, 0x00, 0x00 }, // '
//										  {0x00,0x1C,0x22,0x41,0x00,0x00,0x00,0x00}, // (
//										  {0x00,0x00,0x00,0x41,0x22,0x1C,0x00,0x00}, // )
//										  {0x00,0x44,0x28,0x7E,0x28,0x44,0x00,0x00}, // *
//										  {0x08,0x08,0x08,0x7F,0x08,0x08,0x08,0x00}, // +
//                                          { 0x00, 0x00, 0x50, 0x30, 0x00 }, // ,
//										  {0x00,0x08,0x08,0x08,0x08,0x08,0x08,0x00},//  -
//										  { 0x00, 0x60, 0x60, 0x00, 0x00 }, // .
//										  {0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x00},// /
//										  //{0x00,0x3E,0x41,0x41,0x41,0x41,0x3E,0x00}, // 0
//										  {0x00,0x3E,0x61,0x51,0x49,0x45,0x3E,0x00},//0
//										  {0x00,0x00,0x42,0x7F,0x40,0x00,0x00,0x00}, // 1
//										  {0x00,0x62,0x51,0x51,0x49,0x49,0x46,0x00}, // 2
//										  {0x00,0x22,0x41,0x49,0x49,0x49,0x36,0x00}, // 3
//										  {0x00,0x30,0x28,0x24,0x22,0x7F,0x20,0x00}, // 4
//										  {0x00,0x27,0x49,0x49,0x49,0x49,0x31,0x00}, // 5
//										  {0x00,0x3E,0x49,0x49,0x49,0x49,0x32,0x00}, // 6
//										  {0x00,0x01,0x01,0x61,0x19,0x05,0x03,0x00},//  7
//										  {0x00,0x36,0x49,0x49,0x49,0x49,0x36,0x00},//  8
//										  {0x00,0x26,0x49,0x49,0x49,0x49,0x3E,0x00},//  9
//										  {0x00,0x00,0x00,0x36,0x36,0x00,0x00,0x00}, // :
//										  {0x00,0x00,0x00,0x56,0x36,0x00,0x00,0x00}, // ;
//										  {0x00,0x08,0x14,0x22,0x41,0x00,0x00,0x00}, // <
//										  {0x00,0x24,0x24,0x24,0x24,0x24,0x24,0x00}, // =
//										  {0x00,0x41,0x22,0x14,0x08,0x00,0x00,0x00}, // >
//										  {0x00,0x02,0x01,0x51,0x09,0x09,0x06,0x00}, // ?
//										  {0x1C,0x22,0x59,0x55,0x4D,0x02,0x1C,0x00}, // @
//										  {0x60,0x18,0x16,0x11,0x16,0x18,0x60,0x00}, // A
//										  {0x00,0x7F,0x49,0x49,0x49,0x49,0x36,0x00}, // B
//										  {0x00,0x1C,0x22,0x41,0x41,0x41,0x22,0x00},// C
//										  {0x00,0x7F,0x41,0x41,0x41,0x22,0x1C,0x00}, // D
//										  {0x00,0x7F,0x49,0x49,0x49,0x49,0x41,0x00}, // E
//										  {0x00,0x7F,0x09,0x09,0x09,0x09,0x01,0x00}, // F
//										  {0x00,0x1C,0x22,0x41,0x49,0x49,0x3A,0x00}, // G
//										  {0x00,0x7F,0x08,0x08,0x08,0x08,0x7F,0x00}, // H
//										  {0x00,0x00,0x41,0x7F,0x41,0x00,0x00,0x00}, // I
//										  {0x00,0x20,0x40,0x40,0x40,0x40,0x3F,0x00}, // J
//										  {0x00,0x7F,0x10,0x08,0x14,0x22,0x41,0x00}, // K
//										  {0x00,0x7F,0x40,0x40,0x40,0x40,0x40,0x00}, // L
//										  {0x7F,0x02,0x0C,0x30,0x0C,0x02,0x7F,0x00}, // M
//										  {0x00,0x7F,0x02,0x04,0x08,0x10,0x7F,0x00}, // N
//										  {0x1C,0x22,0x41,0x41,0x41,0x22,0x1C,0x00}, // O
//										  {0x00,0x7F,0x09,0x09,0x09,0x09,0x06,0x00}, // P
//										  {0x1C,0x22,0x41,0x41,0x51,0x22,0x5C,0x00}, // Q
//										  {0x00,0x7F,0x09,0x09,0x19,0x29,0x46,0x00}, // R
//										  {0x00,0x26,0x49,0x49,0x49,0x49,0x32,0x00},// S
//										  {0x01,0x01,0x01,0x7F,0x01,0x01,0x01,0x00}, // T
//										  {0x00,0x3F,0x40,0x40,0x40,0x40,0x3F,0x00}, // U
//										  {0x03,0x0C,0x30,0x40,0x30,0x0C,0x03,0x00}, // V
//										  {0x1F,0x60,0x18,0x06,0x18,0x60,0x1F,0x00}, // W
//										  {0x41,0x22,0x14,0x08,0x14,0x22,0x41,0x00},//X
//										  {0x01,0x02,0x04,0x78,0x04,0x02,0x01,0x00}, // Y
//										  {0x00,0x41,0x61,0x51,0x49,0x45,0x43,0x00}, // Z
//										  {0x00,0x7F,0x41,0x41,0x00,0x00,0x00,0x00}, // [
//                                          { 0x55, 0x2A, 0x55, 0x2A, 0x55 }, // chess
//										  {0x00,0x00,0x00,0x00,0x41,0x41,0x7F,0x00}, // ]
//                                          { 0x04, 0x02, 0x01, 0x02, 0x04 }, // ^
//                                          { 0x40, 0x40, 0x40, 0x40, 0x40 }, // _
//                                          { 0x00, 0x01, 0x02, 0x04, 0x00 }, // '
//										  {0x00,0x20,0x54,0x54,0x54,0x78,0x00,0x00}, // a
//										  {0x00,0x7F,0x48,0x48,0x48,0x30,0x00,0x00}, // b
//										  {0x00,0x38,0x44,0x44,0x44,0x28,0x00,0x00}, // c
//										  {0x00,0x30,0x48,0x48,0x48,0x7F,0x00,0x00}, // d
//										  {0x00,0x38,0x54,0x54,0x54,0x18,0x00,0x00}, // e
//										  {0x00,0x00,0x04,0x7E,0x05,0x01,0x00,0x00}, // f
//										  {0x00,0x08,0x54,0x54,0x54,0x3C,0x00,0x00}, // g
//										  {0x00,0x7F,0x08,0x04,0x04,0x78,0x00,0x00}, // h
//										  {0x00,0x00,0x00,0x7D,0x00,0x00,0x00,0x00}, // i
//										  {0x00,0x20,0x40,0x40,0x3D,0x00,0x00,0x00}, // j
//										  {0x00,0x00,0x7F,0x10,0x28,0x44,0x00,0x00}, // k
//										  {0x00,0x00,0x01,0x7F,0x00,0x00,0x00,0x00}, // l
//										  {0x00,0x7C,0x04,0x78,0x04,0x78,0x00,0x00}, // m
//										  {0x00,0x7C,0x08,0x04,0x04,0x78,0x00,0x00},// n
//										  {0x00,0x38,0x44,0x44,0x44,0x38,0x00,0x00}, // o
//										  {0x00,0x7C,0x14,0x14,0x14,0x08,0x00,0x00}, // p
//										  {0x00,0x08,0x14,0x14,0x14,0x7C,0x00,0x00}, // q
//										  {0x00,0x7C,0x08,0x04,0x04,0x08,0x00,0x00}, // r
//										  {0x00,0x48,0x54,0x54,0x54,0x24,0x00,0x00}, // s
//										  {0x00,0x04,0x3E,0x44,0x44,0x20,0x00,0x00}, // t
//										  {0x00,0x3C,0x40,0x40,0x20,0x7C,0x00,0x00}, // u
//										  {0x00,0x0C,0x30,0x40,0x30,0x0C,0x00,0x00}, // v
//										  {0x00,0x1C,0x60,0x18,0x60,0x1C,0x00,0x00}, // w
//										  {0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00}, // x
//										  {0x00,0x44,0x58,0x20,0x18,0x04,0x00,0x00}, // y
//										  {0x00,0x44,0x64,0x54,0x4C,0x44,0x00,0x00},// z
//                                          { 0x00, 0x08, 0x36, 0x41, 0x41 }, // {
//                                          { 0x00, 0x00, 0x7F, 0x00, 0x00 }, // |
//                                          { 0x41, 0x41, 0x36, 0x08, 0x00 }, // }
//                                          { 0x02, 0x01, 0x02, 0x04, 0x02 },  // SUB 94
//										  {0x00,0x02,0x42,0x3A,0x12,0x0A,0x06,0x00},/*0 sub-95*/
//										  {0x00,0x20,0x20,0x10,0x78,0x04,0x02,0x00},/*1*/
//										  {0x00,0x0C,0x44,0x46,0x24,0x14,0x0C,0x00},/*2*/
//										  {0x20,0x22,0x22,0x3E,0x22,0x22,0x20,0x00},/*3*/
//										  {0x00,0x22,0x22,0x12,0x4A,0x7F,0x02,0x00},/*4*/
//										  {0x00,0x42,0x22,0x1F,0x02,0x42,0x7E,0x00},/*5*/
//										  {0x00,0x12,0x12,0x1F,0x72,0x12,0x10,0x00},/*6*/
//										  {0x00,0x08,0x44,0x43,0x22,0x12,0x0E,0x00},/*7*/
//										  {0x08,0x07,0x42,0x22,0x1E,0x02,0x02,0x00},/*8*/
//										  {0x00,0x42,0x42,0x42,0x42,0x42,0x7E,0x00},/*9*/
//										  {0x00,0x02,0x4F,0x42,0x22,0x1F,0x02,0x00},/*10*/
//										  {0x00,0x45,0x4A,0x40,0x20,0x10,0x0C,0x00},/*11*/
//										  {0x40,0x42,0x22,0x22,0x1A,0x26,0x40,0x00},/*12*/
//										  {0x04,0x04,0x3F,0x44,0x44,0x54,0x4C,0x00},/*13*/
//										  {0x00,0x01,0x46,0x40,0x20,0x10,0x0F,0x00},/*14*/
//										  {0x00,0x08,0x44,0x4B,0x2A,0x12,0x0E,0x00},/*15*/
//										  {0x08,0x0A,0x4A,0x3E,0x09,0x09,0x08,0x00},/*16*/
//                                          {0x02,0x0C,0x42,0x4C,0x20,0x10,0x0E,0x00},/*17*/
//										  {0x04,0x05,0x45,0x3D,0x05,0x05,0x04,0x00},/*18*/
//										  {0x00,0x00,0x7F,0x08,0x08,0x10,0x00,0x00},/*19*/
//										  {0x04,0x44,0x24,0x1F,0x04,0x04,0x04,0x00},/*20*/
//										  {0x20,0x22,0x22,0x22,0x22,0x22,0x20,0x00},/*21*/
//										  {0x00,0x40,0x41,0x25,0x15,0x19,0x27,0x00},/*22*/
//										  {0x20,0x22,0x12,0x7B,0x06,0x12,0x20,0x00},/*23*/
//										  {0x40,0x40,0x20,0x10,0x08,0x07,0x00,0x00},/*24*/
//										  {0x40,0x30,0x0E,0x00,0x02,0x0C,0x70,0x00},/*25*/
//										  {0x00,0x3F,0x48,0x48,0x48,0x44,0x44,0x00},/*26*/
//										  {0x00,0x02,0x42,0x42,0x22,0x12,0x0E,0x00},/*27*/
//										  {0x08,0x04,0x02,0x04,0x08,0x10,0x20,0x00},/*28*/
//										  {0x24,0x14,0x44,0x7F,0x04,0x14,0x24,0x00},/*29*/
//										  {0x00,0x02,0x12,0x22,0x52,0x0A,0x06,0x00},/*30*/
//										  {0x00,0x21,0x25,0x29,0x4A,0x42,0x00,0x00},/*31*/
//										  {0x40,0x70,0x4C,0x43,0x50,0x20,0x40,0x00},/*32*/
//										  {0x00,0x40,0x44,0x24,0x14,0x08,0x37,0x00},/*33*/
//										  {0x00,0x08,0x09,0x3F,0x49,0x49,0x48,0x00},/*34*/
//										  {0x04,0x04,0x0F,0x74,0x02,0x0A,0x06,0x00},/*35*/
//										  {0x20,0x22,0x22,0x22,0x3E,0x20,0x20,0x00},/*36*/
//										  {0x00,0x42,0x4A,0x4A,0x4A,0x4A,0x7E,0x00},/*37*/
//										  {0x04,0x05,0x45,0x45,0x25,0x15,0x0C,0x00},/*38*/
//										  {0x00,0x0F,0x40,0x40,0x20,0x1F,0x00,0x00},/*39*/
//										  {0x40,0x20,0x1E,0x00,0x7F,0x20,0x10,0x00},/*40*/
//										  {0x00,0x7F,0x40,0x20,0x10,0x08,0x00,0x00},/*41*/
//										  {0x00,0x7E,0x42,0x42,0x42,0x42,0x7E,0x00},/*42*/
//										  {0x00,0x0C,0x44,0x44,0x24,0x1C,0x00,0x00},/*43*/
//										  {0x00,0x01,0x45,0x45,0x25,0x15,0x0F,0x00},/*44*/
//										  {0x00,0x41,0x42,0x40,0x20,0x10,0x0C,0x00},/*45*/
//										  {0x02,0x04,0x01,0x02,0x00},/*46*/
//										  {0x07,0x05,0x07,0x00,0x00},/*47*/
//										  {0x70,0x40,0x40,0x40,0x70},/*48*/
//										  {0x70,0x50,0x70,0x00,0x00},/*49*/
//										  {0x00,0x00,0x07,0x01,0x01},/*50*/
//										  {0x40,0x40,0x70,0x00,0x00},/*51*/
//										  {0x10,0x20,0x40,0x00,0x00},/*52*/
//										  {0x00,0x00,0x00,0x18,0x18},/*53*/
//										  {0x00,0x42,0x22,0x1F,0x02,0x42,0x7F,0x00},/*54*/
//										  {0x00,0x12,0x12,0x1F,0x72,0x13,0x10,0x00},/*55*/
//										  {0x00,0x08,0x44,0x43,0x22,0x12,0x0F,0x00},/*56*/
//                                          {0x08,0x07,0x42,0x22,0x1F,0x02,0x03,0x00},/*57*/
//										  {0x00,0x42,0x42,0x42,0x43,0x42,0x7F,0x00},/*58*/
//										  {0x00,0x02,0x4F,0x42,0x23,0x1E,0x03,0x00},/*59*/
//										  {0x00,0x45,0x4A,0x40,0x21,0x10,0x0D,0x00},/*60*/
//										  {0x40,0x42,0x22,0x22,0x1B,0x26,0x41,0x00},/*61*/
//										  {0x04,0x04,0x3F,0x44,0x45,0x54,0x4D,0x00},/*62*/
//										  {0x00,0x01,0x46,0x40,0x20,0x11,0x0F,0x00},/*63*/
//										  {0x00,0x08,0x44,0x4B,0x2A,0x12,0x0F,0x00},/*64*/
//										  {0x08,0x0A,0x4A,0x3E,0x09,0x08,0x09,0x00},/*65*/
//										  {0x02,0x0C,0x42,0x4C,0x21,0x10,0x0D,0x00},/*66*/
//										  {0x04,0x05,0x45,0x3D,0x05,0x04,0x05,0x00},/*67*/
//										  {0x00,0x00,0x7F,0x08,0x09,0x10,0x01,0x00},/*68*/
//										  {0x40,0x30,0x0E,0x00,0x02,0x0D,0x72,0x00},/*69*/
//										  {0x00,0x3F,0x48,0x48,0x4A,0x45,0x46,0x00},/*70*/
//										  {0x00,0x02,0x42,0x42,0x22,0x15,0x0E,0x00},/*71*/
//										  {0x08,0x04,0x02,0x04,0x0A,0x15,0x22,0x00},/*72*/
//										  {0x24,0x14,0x44,0x7F,0x06,0x15,0x22,0x00},/*73*/
//										  {0x40,0x30,0x0E,0x00,0x02,0x0D,0x71,0x00},/*74*/
//										  {0x00,0x3F,0x48,0x48,0x49,0x44,0x45,0x00},/*75*/
//										  {0x00,0x02,0x42,0x42,0x23,0x12,0x0F,0x00},/*76*/
//										  {0x08,0x04,0x02,0x04,0x09,0x10,0x21,0x00},/*77*/
//										  {0x24,0x14,0x44,0x7F,0x05,0x14,0x25,0x00},/*78*/
//										  {0x00,0x04,0x44,0x3C,0x14,0x0C,0x00,0x00},/*79*/
//										  {0x00,0x20,0x20,0x10,0x78,0x04,0x00,0x00},/*80*/
//										  {0x00,0x18,0x48,0x4C,0x28,0x18,0x00,0x00},/*81*/
//										  {0x00,0x40,0x48,0x78,0x48,0x40,0x00,0x00},/*82*/
//										  {0x00,0x28,0x28,0x58,0x7C,0x08,0x00,0x00},/*83*/
//										  {0x00,0x10,0x1C,0x68,0x08,0x18,0x00,0x00},/*84*/
//										  {0x00,0x40,0x48,0x48,0x78,0x40,0x00,0x00},/*85*/
//										  {0x00,0x44,0x54,0x54,0x54,0x7C,0x00,0x00},/*86*/
//										  {0x00,0x18,0x40,0x58,0x20,0x18,0x00,0x00},/*87*/
//										  {0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08},/*88*/
//                                          {0x40,0x40,0x40,0x40,0x40}/*89*/
//                                                                                    };

//unsigned char const font_5x7_KATAKANA[][5] = {
//
//		                                 {0x01,0x41,0x3D,0x09,0x07},/*0*/
//										 {0x10,0x08,0x7C,0x02,0x01},/*1*/
//										 {0x0E,0x02,0x43,0x22,0x1E},/*2*/
//										 {0x42,0x42,0x7E,0x42,0x42},/*3*/
//                                         {0x22,0x12,0x0A,0x7F,0x02},/*4*/
//										 {0x42,0x3F,0x02,0x42,0x3E},/*5*/
//										 {0x0A,0x0A,0x7F,0x0A,0x0A},/*6*/
//                                         {0x08,0x06,0x42,0x22,0x1E},/*7*/
//										 {0x04,0x03,0x42,0x3E,0x02},/*8*/
//                                         {0x42,0x42,0x42,0x42,0x7E},/*9*/
//										 {0x02,0x4F,0x22,0x1F,0x02},/*10*/
//										 {0x4A,0x4A,0x40,0x20,0x1C},/*11*/
//										 {0x42,0x22,0x12,0x2A,0x46},/*12*/
//										 {0x02,0x3F,0x42,0x4A,0x46},/*13*/
//										 {0x06,0x48,0x40,0x20,0x1E},/*14*/
//										 {0x08,0x46,0x4A,0x32,0x1E},/*15*/
//										 {0x0A,0x4A,0x3E,0x09,0x08},/*16*/
//										 {0x0E,0x00,0x4E,0x20,0x1E},/*17*/
//										 {0x04,0x45,0x3D,0x05,0x04},/*18*/
//										 {0x00,0x7F,0x08,0x10,0x00},/*19*/
//										 {0x44,0x24,0x1F,0x04,0x04},/*20*/
//										 {0x40,0x42,0x42,0x42,0x40},/*21*/
//										 {0x42,0x2A,0x12,0x2A,0x06},/*22*/
//										 {0x22,0x12,0x7B,0x16,0x22},/*23*/
//										 {0x00,0x40,0x20,0x1F,0x00},/*24*/
//										 {0x78,0x00,0x02,0x04,0x78},/*25*/
//										 {0x3F,0x44,0x44,0x44,0x44},/*26*/
//										 {0x02,0x42,0x42,0x22,0x1E},/*27*/
//										 {0x04,0x02,0x04,0x08,0x30},/*28*/
//										 {0x32,0x02,0x7F,0x02,0x32},/*29*/
//										 {0x02,0x12,0x22,0x52,0x0E},/*30*/
//										 {0x00,0x2A,0x2A,0x2A,0x40},/*31*/
//										 {0x38,0x24,0x22,0x20,0x70},/*32*/
//										 {0x40,0x28,0x10,0x28,0x06},/*33*/
//                                         {0x0A,0x3E,0x4A,0x4A,0x4A},/*34*/
//										 {0x04,0x7F,0x04,0x14,0x0C},/*35*/
//										 {0x40,0x42,0x42,0x7E,0x40},/*36*/
//										 {0x4A,0x4A,0x4A,0x4A,0x7E},/*37*/
//										 {0x04,0x05,0x45,0x25,0x1C},/*38*/
//										 {0x0F,0x40,0x20,0x1F,0x00},/*39*/
//										 {0x7C,0x00,0x7E,0x40,0x30},/*40*/
//										 {0x7E,0x40,0x20,0x10,0x08},/*41*/
//										 {0x7E,0x42,0x42,0x42,0x7E},/*42*/
//										 {0x06,0x02,0x42,0x22,0x1E},/*43*/
//										 {0x0A,0x0A,0x4A,0x2A,0x1E},/*44*/
//										 {0x42,0x42,0x40,0x20,0x18},/*45*/
//										 {0x02,0x04,0x01,0x02,0x00},/*46*/
//										 {0x07,0x05,0x07,0x00,0x00},/*47*/
//										 {0x70,0x40,0x40,0x40,0x70},/*48*/
//										 {0x70,0x50,0x70,0x00,0x00},/*49*/
//										 {0x00,0x00,0x07,0x01,0x01},/*50*/
//										 {0x40,0x40,0x70,0x00,0x00},/*51*/
//										 {0x10,0x20,0x40,0x00,0x00},/*52*/
//										 {0x00,0x00,0x00,0x18,0x18},/*53*/
//										 {0x44,0x3E,0x04,0x45,0x3D},/*54*/
//										 {0x14,0x14,0x7E,0x15,0x15},/*55*/
//										 {0x08,0x44,0x44,0x25,0x1D},/*56*/
//										 {0x08,0x06,0x44,0x3D,0x05},/*57*/
//										 {0x44,0x44,0x44,0x45,0x7D},/*58*/
//										 {0x08,0x1E,0x48,0x3D,0x09},/*59*/
//										 {0x54,0x54,0x40,0x21,0x1D},/*60*/
//										 {0x44,0x24,0x14,0x2D,0x45},/*61*/
//										 {0x04,0x3E,0x44,0x4D,0x45},/*62*/
//										 {0x0C,0x50,0x40,0x21,0x1D},/*63*/
//										 {0x10,0x4C,0x54,0x25,0x1D},/*64*/
//										 {0x10,0x54,0x3C,0x15,0x11},/*65*/
//										 {0x0C,0x40,0x4C,0x21,0x1D},/*66*/
//										 {0x10,0x54,0x34,0x15,0x11},/*67*/
//										 {0x00,0x00,0x7E,0x09,0x11},/*68*/
//										 {0x78,0x00,0x0A,0x15,0x62},/*69*/
//										 {0x3E,0x48,0x4A,0x4D,0x4A},/*70*/
//										 {0x08,0x48,0x4A,0x4D,0x3A},/*71*/
//										 {0x10,0x08,0x12,0x25,0x42},/*72*/
//										 {0x68,0x08,0x7E,0x0D,0x6A},/*73*/
//										 {0x78,0x00,0x04,0x09,0x71},/*74*/
//										 {0x3E,0x44,0x44,0x45,0x45},/*75*/
//										 {0x04,0x44,0x44,0x25,0x1D},/*76*/
//										 {0x08,0x04,0x08,0x11,0x61},/*77*/
//										 {0x34,0x04,0x7E,0x05,0x35},/*78*/
//										 {0x04,0x44,0x34,0x14,0x0C},/*79*/
//										 {0x20,0x10,0x78,0x04,0x00},/*80*/
//										 {0x18,0x08,0x4C,0x48,0x38},/*81*/
//										 {0x48,0x48,0x78,0x48,0x48},/*82*/
//										 {0x48,0x28,0x18,0x7C,0x08},/*83*/
//										 {0x08,0x7C,0x08,0x28,0x18},/*84*/
//										 {0x40,0x48,0x48,0x78,0x40},/*85*/
//										 {0x54,0x54,0x54,0x7C,0x00},/*86*/
//										 {0x18,0x00,0x58,0x40,0x38},/*87*/
//
//};
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
void GrPixelDrawBolymin(uint8_t lucX, uint8_t lucY, bool lbFillColor, bool lGlowColumn)
{

}

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
void GrLineDrawHorizontalBolymin(uint8_t lucX1, uint8_t lucX2, uint8_t lucY, bool lbFillColor)
{

}

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
 *
 * Function Returns: void
 *
*********************************************************************************/
void GrLineDrawVerticalBolymin(uint8_t lucX, uint8_t lucY1, uint8_t lucY2, bool lbFillColor, bool lGlowColumn)
{

}

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
 * NOTE:lucYMin��lucYMax,It should be always even number from 0,8,16,24,36,40,48,56,64
 *
*********************************************************************************/
void GrRectFIllBolymin(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax, bool lbFillColor, bool lGlowRightmostColumn)
{
    uint8_t i,j,data_lucYMin,data_lucYMax;
    if((lucXMax==127)||(lucXMax==126)) lucXMax = 128;
   if(lGlowRightmostColumn==true)
   {
       ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
       data_lucYMin=lucYMin/8;
       data_lucYMax=lucYMax/8+1;
       for(i=data_lucYMin;i<data_lucYMax;i++)
       {
          lcd_address(1+i,lucXMin);
          for(j=lucXMin;j<=lucXMax;j++)
          {
             send_data(0x00);
          }
       }
      ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, DISPLAY_CS_PIN);//not Selected LCD
   }
}
void GrRectFIllBolymin_cyw(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax, uint8_t  lbFillColor, bool lGlowRightmostColumn)
{
    uint8_t i,j,data_lucYMin,data_lucYMax;
    if((lucXMax==127)||(lucXMax==126)) lucXMax = 128;
   if(lGlowRightmostColumn==true)
   {
       ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
       data_lucYMin=lucYMin/8;
       data_lucYMax=lucYMax/8+1;
       for(i=data_lucYMin;i<data_lucYMax;i++)
       {
          lcd_address(1+i,lucXMin);
          for(j=lucXMin;j<=lucXMax;j++)
          {
            if(i%2==0)
            {
        	  send_data(0xfe);//7f
            }
            else
            {
              send_data(0x3f);
            }
          }
       }
      ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, DISPLAY_CS_PIN);//not Selected LCD
   }
}

void DISP_GUESTER_KONG(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax)
{
	uint8_t i,j,z=0,data_lucYMin,data_lucYMax;
		const uint8_t data_1[] =
	//{0xF0,0x0F,0xC0,0x1F,0xFE,0x3F,0xC0,0x3F,0xFF,0x3F,0xC0,0x3F,
	//	0xFE,0x1F,0x00,0x0F,0xE0,0x07};
	{0xF0,0x80,0x7E,0x40,0x7F,0x40,0xFE,0x00,0xE0,0x0F,0x10,0x20,
			0x20,0x20,0x20,0x10,0x09,0x07};
		ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
		       data_lucYMin=lucYMin/8;
		       data_lucYMax=lucYMax/8+1;
		       for(i=data_lucYMin;i<data_lucYMax;i++)
		       {
		          lcd_address(1+i,lucXMin);

		          for(j=lucXMin;j<=lucXMax;j++)
		          {
		              send_data(data_1[z++]);

		          }
		       }
		 ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, DISPLAY_CS_PIN);//not Selected LCD
}

void DISP_GUESTER_FAN(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax)
{
	uint8_t i,j,z=0,data_lucYMin,data_lucYMax;
		const uint8_t data_1[] =
	//{0xF0,0x0F,0xC0,0x1F,0xFE,0x3F,0xC0,0x3F,0xFF,0x3F,0xC0,0x3F,
	//	0xFE,0x1F,0x00,0x0F,0xE0,0x07};
	{0xFE,0xFF,0x0F,0x7F,0x01,0x3F,0x00,0x3F,0x01,0xFF,0x1F,0xFF,
	0xFE,0x1F,0x3F,0x30,0x20,0x00,0x00,0x00,0x00,0x20,0x30,0x38,
	0x3F,0x1F};
		ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
		       data_lucYMin=lucYMin/8;
		       data_lucYMax=lucYMax/8+1;
		       for(i=data_lucYMin;i<data_lucYMax;i++)
		       {
		          lcd_address(1+i,lucXMin);

		          for(j=lucXMin;j<=lucXMax;j++)
		          {
		              send_data(data_1[z++]);

		          }
		       }
		 ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, DISPLAY_CS_PIN);//not Selected LCD
}
void DISP_GUESTER_FAN_DIS(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax)
{
	uint8_t i,j,z=0,data_lucYMin,data_lucYMax;
		const uint8_t data_1[] =
	//{0xF0,0x0F,0xC0,0x1F,0xFE,0x3F,0xC0,0x3F,0xFF,0x3F,0xC0,0x3F,
	//	0xFE,0x1F,0x00,0x0F,0xE0,0x07};
	{0xFE,0xFF,0x0F,0x7F,0x01,0x3F,0x00,0x3F,0x01,0xFF,0x1F,0xFF,
			0xFE,0x1F,0x3F,0x30,0x20,0x09,0x06,0x06,0x09,0x20,0x30,0x38,
			0x3F,0x1F};
		ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
		       data_lucYMin=lucYMin/8;
		       data_lucYMax=lucYMax/8+1;
		       for(i=data_lucYMin;i<data_lucYMax;i++)
		       {
		          lcd_address(1+i,lucXMin);

		          for(j=lucXMin;j<=lucXMax;j++)
		          {
		              send_data(data_1[z++]);

		          }
		       }
		 ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, DISPLAY_CS_PIN);//not Selected LCD
}
void DISP_GUESTER_9_16(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax)
{
	uint8_t i,j,z=0,data_lucYMin,data_lucYMax;
	const uint8_t data_1[] = 
//{0xF0,0x0F,0xC0,0x1F,0xFE,0x3F,0xC0,0x3F,0xFF,0x3F,0xC0,0x3F,
//	0xFE,0x1F,0x00,0x0F,0xE0,0x07};
{0xF0,0xC0,0xFE,0xC0,0xFF,0xC0,0xFE,0x00,0xE0,0x0F,0x1F,0x3F,
 0x3F,0x3F,0x3F,0x1F,0x0F,0x07};
	ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
	       data_lucYMin=lucYMin/8;
	       data_lucYMax=lucYMax/8+1;
	       for(i=data_lucYMin;i<data_lucYMax;i++)
	       {
	          lcd_address(1+i,lucXMin);

	          for(j=lucXMin;j<=lucXMax;j++)
	          {
	              send_data(data_1[z++]);

	          }
	       }
	 ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, DISPLAY_CS_PIN);//not Selected LCD
}
void DISP_GUESTER_13_16(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax)
{
	uint8_t i,j,z=0,data_lucYMin,data_lucYMax;
	const uint8_t data_1[] =
//{0xF0,0x0F,0xC0,0x1F,0xFE,0x3F,0xC0,0x3F,0xFF,0x3F,0xC0,0x3F,
//	0xFE,0x1F,0x00,0x0F,0xE0,0x07};
{0x00,0x00,0xF0,0x80,0xFE,0xC0,0xFF,0xC0,0xFE,0x00,0xE0,0x00,
		0x00,0x00,0x00,0x0F,0x1F,0x3F,0x3F,0x3F,0x3F,0x1F,0x0F,0x07,
		0x00,0x00};
	ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
	       data_lucYMin=lucYMin/8;
	       data_lucYMax=lucYMax/8+1;
	       for(i=data_lucYMin;i<data_lucYMax;i++)
	       {
	          lcd_address(1+i,lucXMin);

	          for(j=lucXMin;j<=lucXMax;j++)
	          {
	              send_data(data_1[z++]);

	          }
	       }
	 ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, DISPLAY_CS_PIN);//not Selected LCD
}
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
void GrRectDrawBolymin(uint8_t lucXMin, uint8_t lucXMax, uint8_t lucYMin, uint8_t lucYMax, bool lbFillColor)
{

}

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
 * NOTE:lucYMin��lucYMax,It should be always even number from 0,8,16,24,36,40,48,56,64
 *
*********************************************************************************/
void GrStringDrawBolymin(unsigned char *plucBolyminPixelData, unsigned char lucLength,
             unsigned char lucX,unsigned char lucY, bool lbFillColor)
{
    uint16_t i=0;
    i=lucY/8;
    display_string_5x7(i+1,lucX+1,plucBolyminPixelData,lbFillColor);

} //GrStringDraw


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

void bolyminDisplayInit(void)    //ROM_SysCtlDelay(1) ==0.06us
{
    //
    // Enable the peripherals used by this driver
    //
    ROM_SysCtlPeripheralEnable(DISPLAY_RST_GPIO_PERIPH);
    //
    // Configure display control pins as GPIO output
    //
    ROM_GPIOPinTypeGPIOOutput(DISPLAY_RST_PORT, DISPLAY_RST_PIN);
    ROM_GPIOPinTypeGPIOOutput(DISPLAY_ENV_PORT, DISPLAY_ENV_PIN);
    ROM_GPIOPinTypeGPIOOutput(DISPLAY_D_C_PORT, DISPLAY_D_C_PIN);
    ROM_GPIOPinTypeGPIOOutput(DISPLAY_SDIN_PORT, DISPLAY_SDIN_PIN);
    ROM_GPIOPinTypeGPIOOutput(DISPLAY_SCLK_PORT, DISPLAY_SCLK_PIN);
    ROM_GPIOPinTypeGPIOOutput(DISPLAY_CS_PORT, DISPLAY_CS_PIN);

    //
    //  power on
    //
    //Set_lcdlightON();
    ROM_GPIOPinWrite(DISPLAY_ENV_PORT, DISPLAY_ENV_PIN, DISPLAY_ENV_PIN);

    lcd_init();   //LCD Initialize
}

extern uint8_t Flag_lcdbackon;

void LCD_BACKLIGHT_ON(void)
{
	//
	//  power on
	//
	 ROM_GPIOPinWrite(DISPLAY_ENV_PORT, DISPLAY_ENV_PIN, DISPLAY_ENV_PIN);
	
}

void LCD_BACKLIGHT_OFF(void)
{
	//
	//  power off
	//
	ROM_GPIOPinWrite(DISPLAY_ENV_PORT, DISPLAY_ENV_PIN, 0);
	
}

void LCD_BACKLIGHT_TOGGLE(void)
{
    if(ROM_GPIOPinRead(DISPLAY_ENV_PORT, DISPLAY_ENV_PIN))
	{
		ROM_GPIOPinWrite(DISPLAY_ENV_PORT, DISPLAY_ENV_PIN, 0);
	}
	else
	{
		ROM_GPIOPinWrite(DISPLAY_ENV_PORT, DISPLAY_ENV_PIN, DISPLAY_ENV_PIN);
	}
	
}

// void LCD_BACKLIGHT_GETSTATUS(void)
// {
// 	//  if(ROM_GPIOPinRead(DISPLAY_ENV_PORT, DISPLAY_ENV_PIN))
// 	// {
// 	// 	gstLEDcontrolRegister.BacklightLCD = 1;
// 	// }
// 	// else
// 	// {
// 	// 	gstLEDcontrolRegister.BacklightLCD = 0;
// 	// };
// 	gstLEDcontrolRegister.BacklightLCD  = Flag_lcdbackon;
// }

// void LCD_BACKLIGHT_SETSTATUS(void)
// {
// 	 if(gstLEDcontrolRegister.BacklightLCD == 1)
// 	{
// 		LCD_BACKLIGHT_ON();
// 	}
// 	else
// 	{
// 		LCD_BACKLIGHT_OFF();
// 	}
// }



/******************************************************************************
 * FunctionName: lcd_init
 *
 * Function Description: This function initializes display controller.
 *
 * Function Parameter: None
 *
 * Function Returns: void
 *
*********************************************************************************/
void  lcd_init(void)
{
   ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);  //Selected LCD
   ROM_GPIOPinWrite(DISPLAY_RST_PORT, DISPLAY_RST_PIN, 0); //Reset
   delay(100);
   ROM_GPIOPinWrite(DISPLAY_RST_PORT, DISPLAY_RST_PIN, DISPLAY_RST_PIN);//Reset OK
   delay(20);
   send_command(0xe2);//LCD soft Reset
   delay(5);
   send_command(0x2c);//Booster ratio set1
   delay(5);
   send_command(0x2e);//Booster ratio set2
   delay(5);
   send_command(0x2f);//Booster ratio set3
   delay(5);
   send_command(0x24);//Coarse contrast
   send_command(0x81);//Contrast trimming
   send_command(0x1a);//Contrast trimming ,Value
   send_command(0xa2);//1/9 LCD bias set)
   send_command(0xc8);//Common output mode select
   send_command(0xa0);//Column scanning*/
   send_command(0x40);//The initial line of display settings
   send_command(0xaf); //display ON

   //clear_clear();
   //GrRectFIllBolymin(0, 128, 0, 64, false, true);
   GrRectFIllBolymin(0, 127, 0, 63, false, true);

/*****************lcd full test****************************************/
//   display_string_5x7(1,1," ! #$%&'()*+,-./01234",0);
//   display_string_5x7(2,1,"56789:;<=>?@ABCDEFGHI",0);
//   display_string_5x7(3,1,"JKLMNOPQRSTUVWXYZ[\]^",0);
//   display_string_5x7(4,1,"_`abcdefghijklmnopqrs",0);
//   display_string_5x7(5,1," ! #$%&'()*+,-./01234",1);
//   display_string_5x7(6,1,"56789:;<=>?@ABCDEFGHI",1);
//   display_string_5x7(7,1,"JKLMNOPQRSTUVWXYZ[\]^",1);
//   display_string_5x7(8,1,"_`abcdefghijklmnopqrs",1);
/*********************************************************************/

   ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, DISPLAY_CS_PIN);// not Selected LCD
}

//CYW �I����  ��� ��?�И �ݐ�?���I�ʒu
uint8_t found_KATAKANA_sub_cyw(uint16_t x_code)
{
	uint8_t Tp_sub=0;
	switch(x_code)
	{
	case 0x6321://"�A"
		Tp_sub = 95;
		break;
	case 0x6323://"�C"
		Tp_sub = 95+1;
		 break;
	case 0x6325://"�E"
		Tp_sub = 95+2;
		break;
	case 0x6327://"�G"
		Tp_sub = 95+3;
		break;
	case 0x6329://"�G"
		Tp_sub = 95+4;
		break;
	case 0x632a://"�J"
		Tp_sub = 95+5;
		break;
	case 0x632c://"�L"
		Tp_sub = 95+6;
		break;
	case 0x632e://"�N"
		Tp_sub = 95+7;
		break;
	case 0x6330://"�P"
		Tp_sub = 95+8;
		break;
	case 0x6332://"�R"
		Tp_sub = 95+9;
		break;
	case 0x6334://"�T"
		Tp_sub = 95+10;
		break;
	case 0x6336://"�V"
		Tp_sub = 95+11;
		break;
	case 0x6338://"�X"
		Tp_sub = 95+12;
		break;
	case 0x633a://"�Z"
		Tp_sub = 95+13;
		break;
	//case 0x633c://"�\"
	case 0x6300://"�\"
		Tp_sub = 95+14;
		break;
	case 0x633e://"�^"
		Tp_sub = 95+15;
		break;
	case 0x6340://"�`"
		Tp_sub = 95+16;
		break;
	case 0x6343://"�c"
		Tp_sub = 95+17;
		break;
	case 0x6345://"�e"
		Tp_sub = 95+18;
		break;
	case 0x6347://"�g"
		Tp_sub = 95+19;
		break;
	case 0x6349://"�i"
		Tp_sub = 95+20;
		break;
	case 0x634a://"�j"
		Tp_sub = 95+21;
		break;
	case 0x634b://"�k"
		Tp_sub = 95+22;
		break;
	case 0x634c://"�l"
		Tp_sub = 95+23;
		break;
	case 0x634d://"�m"
		Tp_sub = 95+24;
		break;
	case 0x634e://"�n"
		Tp_sub = 95+25;
		break;
	case 0x6351://"�q"
		Tp_sub = 95+26;
		break;
	case 0x6354://"�t"
		Tp_sub = 95+27;
		break;
	//case 0x6357://"�w"
	case 0x62b6:
		Tp_sub = 95+28;
		break;
	case 0x635a://"�z"
		Tp_sub = 95+29;
		break;
	case 0x635d://"�}"
		Tp_sub = 95+30;
		break;
	case 0x635e://"�~"
		Tp_sub = 95+31;
		break;
	case 0x6360://"��"
		Tp_sub = 95+32;
		break;
	case 0x6361://"��"
		Tp_sub = 95+33;
		break;
	case 0x6362://"��"
		Tp_sub = 95+34;
		break;
	case 0x6364://"��"
		Tp_sub = 95+35;
		break;
	case 0x6366://"��"
		Tp_sub = 95+36;
		break;
	case 0x6368://"��"
		Tp_sub = 95+37;
		break;
	case 0x6369://"��"
		Tp_sub = 95+38;
		break;
	case 0x636a://"��"
		Tp_sub = 95+39;
		break;
	case 0x636b://"��"
		Tp_sub = 95+40;
		break;
	case 0x636c://"��"
		Tp_sub = 95+41;
		break;
	case 0x636d://"��"
		Tp_sub = 95+42;
		break;
	case 0x636f://"��"
		Tp_sub = 95+43;
		break;
	case 0x6372://"��"
		Tp_sub = 95+44;
		break;
	case 0x6373://"��"
		Tp_sub = 95+45;
		break;
	case 0x632b://"�K"
		Tp_sub = 95+54;
		break;
	case 0x632d://"�M"
		Tp_sub = 95+55;
		break;
	case 0x632f://"�O"
		Tp_sub = 95+56;
		break;
	case 0x6331://"�Q"
		Tp_sub = 95+57;
		break;
	case 0x6333://"�S"
		Tp_sub = 95+58;
		break;
	case 0x6335://"�U"
		Tp_sub = 95+59;
		break;
	case 0x6337://"�W"
		Tp_sub = 95+60;
		break;
	case 0x6339://"�Y"
		Tp_sub = 95+61;
		break;
	case 0x633b://"�["
		Tp_sub = 95+62;
		break;
	case 0x633d://"�]"
		Tp_sub = 95+63;
		break;
	case 0x633f://"�_"
		Tp_sub = 95+64;
		break;
	case 0x6341://"�a"
		Tp_sub = 95+65;
		break;
	case 0x6344://"�d"
		Tp_sub = 95+66;
		break;
	case 0x6346://"�f"
		Tp_sub = 95+67;
		break;
	case 0x6348://"�h"
		Tp_sub = 95+68;
		break;
	case 0x6350://"�p"
		Tp_sub = 95+69;
		break;
	case 0x6353://"�s"
		Tp_sub = 95+70;
		break;
	case 0x6356://"�v"
		Tp_sub = 95+71;
		break;
	case 0x6359://"�y"
		Tp_sub = 95+72;
		break;
	case 0x635c://"�|"
		Tp_sub = 95+73;
		break;
	case 0x634f://"�o"
		Tp_sub = 95+74;
		break;
	case 0x6352://"�r"
		Tp_sub = 95+75;
		break;
	case 0x6355://"�u"
		Tp_sub = 95+76;
		break;
	case 0x6358://"�x"
		Tp_sub = 95+77;
		break;
	case 0x635b://"�{"
		Tp_sub = 95+78;
		break;
	case 0x6320://"�@"
		Tp_sub = 95+79;
		break;
	case 0x6322://"�B"
		Tp_sub = 95+80;
		break;
	case 0x6324://"�D"
		Tp_sub = 95+81;
		break;
	case 0x6326://"�F"
		Tp_sub = 95+82;
		break;

	case 0x6328://"�H"
		Tp_sub = 95+83;
		break;
	case 0x6363://"��"
		Tp_sub = 95+84;
		break;
	case 0x6365://"��"
		Tp_sub = 95+85;
		break;
	case 0x6367://"��"
		Tp_sub = 95+86;
		break;
	case 0x6342://"�b"
		Tp_sub = 95+87;
		break;
	case 0x613b://"�["
		Tp_sub = 95+88;
		break;

	default:break;
	}
	return Tp_sub;
}

/******************************************************************************
 * FunctionName: display_string_5x7
 *
 * Function Description: This function display_string_5x7.
 *
 * Function Parameter:
 * page   :   It should be always even number from 1 to 8.
 * column :   It should be always even number from 1 to 128
 * *text  :   data to be displayed.
 * lbFillColor_b  :
 *
 * Function Returns: void
 *
*********************************************************************************/
//void display_string_5x7(uint16_t page,uint16_t column,uint8_t *text,bool lbFillColor_b)
//{
//    uint16_t i=0,j,k;
//    uint8_t m;
//   ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
//   while(text[i]>0x00)
//   {
//    //  if((text[i]>=0x20)&&(text[i]<0x7e))
//	   if((text[i]>=0x20))//cyw
//       {
//          j=text[i]-0x20;
//          lcd_address(page,column);
//          for(k=0;k<5;k++)
//          {
//              if(lbFillColor_b==true)m=~font_5x7[j][k];
//              else m=font_5x7[j][k];
//              send_data(m);
//          }
//          if(lbFillColor_b==true)send_data(0xff);
//          i++;
//          column+=6;
//       }
//       else i++;
//   }
//}
void display_string_5x7_DOUBLE(uint16_t page,uint16_t column,uint8_t *text,bool lbFillColor_b)
{
    uint16_t i=0,j,k,tp_sum;
    uint8_t m;
   ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
   while((text[i]>0x00)&&(text[i+1]>0x00))
   {
    //  if((text[i]>=0x20)&&(text[i]<0x7e))
	   tp_sum = (text[i]-0x20)*256 + (text[i+1]-0x20);
	   if((tp_sum>=0x20)&&(tp_sum<=0x7e))//cyw
       {
          j=text[i+1]-0x20;
          lcd_address(page,column);
          for(k=0;k<5;k++)
          {
              if(lbFillColor_b==true)m=~font_5x7[j][k];
              else m=font_5x7[j][k];
              send_data(m);
          }
          if(lbFillColor_b==true)send_data(0xff);
          i=i+2;
          column+=6;
       }
	   else if(tp_sum > 0x100)
	   {
		   j=found_KATAKANA_sub_cyw(tp_sum);
		             lcd_address(page,column);
		             for(k=0;k<5;k++)
		             {
		                 if(lbFillColor_b==true)m=~font_5x7[j][k];
		                 else m=font_5x7[j][k];
		                 send_data(m);
		             }
		             if(lbFillColor_b==true)send_data(0xff);
		             i=i+2;
		             column+=6;
	   }
       else
       {
    	   i=i+2;
       }
   }
}
/******************************************************************************
 * FunctionName: send_command
 *
 * Function Description: This function is send command.
 *
 * Function Parameter:
 * data   :   command.
 *
 * Function Returns: void
 *
*********************************************************************************/
void send_command(uint16_t data)
{
    uint8_t i;
   ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
   ROM_GPIOPinWrite(DISPLAY_D_C_PORT, DISPLAY_D_C_PIN, 0);//command
   for(i=0;i<8;i++)
   {
       ROM_GPIOPinWrite(DISPLAY_SCLK_PORT, DISPLAY_SCLK_PIN, 0);
     if(data&0x80)
         ROM_GPIOPinWrite(DISPLAY_SDIN_PORT, DISPLAY_SDIN_PIN, DISPLAY_SDIN_PIN);
     else
         ROM_GPIOPinWrite(DISPLAY_SDIN_PORT, DISPLAY_SDIN_PIN, 0);
     ROM_GPIOPinWrite(DISPLAY_SCLK_PORT, DISPLAY_SCLK_PIN, DISPLAY_SCLK_PIN);
     data= data <<1;
   }
}
/******************************************************************************
 * FunctionName: send_data
 *
 * Function Description: This function is display data.
 *
 * Function Parameter:
 * data   :   display data.
 *
 * Function Returns: void
 *
*********************************************************************************/
void send_data(uint16_t data)
{
    uint8_t i;
   ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
   ROM_GPIOPinWrite(DISPLAY_D_C_PORT, DISPLAY_D_C_PIN, DISPLAY_D_C_PIN);//data
   for(i=0;i<8;i++)
   {
       ROM_GPIOPinWrite(DISPLAY_SCLK_PORT, DISPLAY_SCLK_PIN, 0);
     if(data&0x80)
         ROM_GPIOPinWrite(DISPLAY_SDIN_PORT, DISPLAY_SDIN_PIN, DISPLAY_SDIN_PIN);
     else
         ROM_GPIOPinWrite(DISPLAY_SDIN_PORT, DISPLAY_SDIN_PIN, 0);
     ROM_GPIOPinWrite(DISPLAY_SCLK_PORT, DISPLAY_SCLK_PIN, DISPLAY_SCLK_PIN);
     data= data <<1;
   }
}
/******************************************************************************
 * FunctionName: lcd_address
 *
 * Function Description: This function is display data.
 *
 * Function Parameter:
 * page   :   display page.
 * column :   display column
 *
 * Function Returns: void
 *
*********************************************************************************/
void lcd_address(uint8_t page,uint8_t column)
{

   ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
   column  = column -1;
   page    = page   -1;
   send_command(0xb0+page);
   send_command(((column>>4)&0x0f)+0x10);
   send_command(column&0x0f);
}
/******************************************************************************
 * FunctionName: delay
 *
 * Function Description: This function is delay.
 *
 * Function Parameter:
 * i   :  delay NUM.
 *
 * Function Returns: void
 *
*********************************************************************************/
void delay(uint16_t i)
{
    uint16_t j,k;
   for(j=0;j<i;j++)
     for(k=0;k<220;k++);
}



//
//void clear_clear(void)
//{
//    uint8_t i,j;
//   //ClearWDT(); // Service the WDT
//   ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);
//   for(i=0;i<9;i++)
//   {
//      //ClearWDT(); // Service the WDT
//      lcd_address(1+i,1);
//      for(j=0;j<132;j++)
//      {
//         send_data(0x00);
//      }
//   }
//   ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, DISPLAY_CS_PIN);
//
//}


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
//void UpdateRamString_5x7(uint8_t *text,uint16_t *ram,bool lbFillColor_b)
//{
//    uint16_t i=0,j,k,lucRamIndex = 0;
//    uint16_t m;
//
//   while(text[i]>0x00)
//   {
//
//       if((text[i]>=0x20)&&(text[i]<0x7e))
//       {
//
//          j=text[i]-0x20;
//
//          for(k=0;k<5;k++)
//          {
//        	  m = 0;
//              if(lbFillColor_b==true)
//			  {
//			  m=~font_5x7[j][k];
//			  m = m << 4;
//			  m = m | 0x000F;
//			  *(ram + lucRamIndex) &= m;
//			  }
//              else
//			  {
//			  m=font_5x7[j][k];
//			  m = m << 4;
//			  *(ram + lucRamIndex) |= m;
//			  }
//
//              lucRamIndex++;
//
//          } //for(k=0;k<5;k++)
//
//          i++;
//          lucRamIndex++;
//
//       }
//       else
//       {
//    	   i++;
//       }
//
//   } //while(text[i]>0x00)
//
//} //void UpdateRamString_5x7(uint8_t *text,int16_t *ram,bool lbFillColor_b)
void UpdateRamString_5x7_DOUBLE(uint8_t *text,uint16_t *ram,bool lbFillColor_b)
{
    uint16_t i=0,j,k,lucRamIndex = 0;
    uint16_t m,tp_sum;

   while((text[i]>0x00)&&(text[i+1]>0x00))
   {
	   tp_sum = (text[i]-0x20)*256 + (text[i+1]-0x20);
       if(tp_sum<=0x5e)
       {

          j=text[i+1]-0x20;

          for(k=0;k<5;k++)
          {
        	  m = 0;
              if(lbFillColor_b==true)
			  {
			  m=~font_5x7[j][k];
			  m = m << 4;
			  m = m | 0x000F;
			  *(ram + lucRamIndex) &= m;
			  }
              else
			  {
			  m=font_5x7[j][k];
			  m = m << 4;
			  *(ram + lucRamIndex) |= m;
			  }

              lucRamIndex++;

          } //for(k=0;k<5;k++)

          i=i+2;
          lucRamIndex++;

       }
       else if(tp_sum >= 0x100)
       {
    	   j=found_KATAKANA_sub_cyw(tp_sum);

    	            for(k=0;k<5;k++)
    	            {
    	          	  m = 0;
    	                if(lbFillColor_b==true)
    	  			  {
    	  			  m=~font_5x7[j][k];
    	  			  m = m << 4;
    	  			  m = m | 0x000F;
    	  			  *(ram + lucRamIndex) &= m;
    	  			  }
    	                else
    	  			  {
    	  			  m=font_5x7[j][k];
    	  			  m = m << 4;
    	  			  *(ram + lucRamIndex) |= m;
    	  			  }

    	                lucRamIndex++;

    	            } //for(k=0;k<5;k++)

    	            i=i+2;
    	            lucRamIndex++;
       }
       else
       {
    	   i= i+2;
       }

   } //while(text[i]>0x00)

} //void UpdateRamString_5x7(uint8_t *text,int16_t *ram,bool lbFillColor_b)
void UpdateRamString_8x8_DOUBLE(uint8_t *text,uint16_t *ram,bool lbFillColor_b)
{
    uint16_t i=0,j,k,lucRamIndex = 0;
    uint16_t m,tp_sum;

   while((text[i]>0x00)&&(text[i+1]>0x00))
   {
	   tp_sum = (text[i]-0x20)*256 + (text[i+1]-0x20);
       if(tp_sum<=0x5e)
       {

          j=text[i+1]-0x20;

          for(k=0;k<8;k++)
          {
        	  m = 0;
              if(lbFillColor_b==true)
			  {
			  m=~font_8x8[j][k];
			  m = m << 4;
			  m = m | 0x000F;
			  *(ram + lucRamIndex) &= m;
			  }
              else
			  {
			  m=font_8x8[j][k];
			  m = m << 4;
			  *(ram + lucRamIndex) |= m;
			  }

              lucRamIndex++;

          } //for(k=0;k<5;k++)

          i=i+2;
          if(enable_15display_cyw == 0)
          lucRamIndex++;

       }
       else if(tp_sum >= 0x100)
       {
    	   j=found_KATAKANA_sub_cyw(tp_sum);

    	            for(k=0;k<8;k++)
    	            {
    	          	  m = 0;
    	                if(lbFillColor_b==true)
    	  			  {
    	  			  m=~font_8x8[j][k];
    	  			  m = m << 4;
    	  			  m = m | 0x000F;
    	  			  *(ram + lucRamIndex) &= m;
    	  			  }
    	                else
    	  			  {
    	  			  m=font_8x8[j][k];
    	  			  m = m << 4;
    	  			  *(ram + lucRamIndex) |= m;
    	  			  }

    	                lucRamIndex++;

    	            } //for(k=0;k<5;k++)

    	            i=i+2;
    	            if(enable_15display_cyw == 0)
    	            lucRamIndex++;
       }
       else
       {
    	   i= i+2;
       }

   } //while(text[i]>0x00)

} //void UpdateRamString_5x7(uint8_t *text,int16_t *ram,bool lbFillColor_b)

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
void display_ram(uint16_t page,uint16_t columnMin,uint16_t columnMax,uint16_t *ram)
{
    uint16_t i,j,k;
    uint8_t m;

   ROM_GPIOPinWrite(DISPLAY_CS_PORT, DISPLAY_CS_PIN, 0);//Selected LCD
   k = 0;

   for(i = columnMin; i < columnMax; i++ )
   {
          j = ram[k];

          //m = (j >> 8) & 0x00FF;
          //lcd_address(page,i);
          //send_data(m);

          m = j & 0x00FF;
          lcd_address(page,i);
          send_data(m);

          m = (j >> 8) & 0x00FF;
          lcd_address(page + 1,i);
          send_data(m);

          k++;

   } // for(i = columnMin; i < columnMax; i++ )

} //void display_ram(uint16_t page,uint16_t columnMin,uint16_t columnMax,uint16_t *ram)

#endif
