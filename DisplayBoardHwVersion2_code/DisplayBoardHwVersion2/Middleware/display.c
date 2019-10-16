/*********************************************************************************
* FileName: display.c
* Description:
* This source file contains the definition for functions used to carry
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

/****************************************************************************
 *  Include:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "grlib/grlib.h"
//#include "cfal96x64x16.h"
#include "bolymindisplay.h"

/****************************************************************************/

/****************************************************************************
 *  Macro definitions:
****************************************************************************/
#define	RECTANGLE_HEIGHT		15
#define BORDER_SPACING			 1
#define BORDER_SPACING_4			 4
#define BORDER_SPACING_FROM_TOP	 4
//#define DISPLAY_AREA_WIDTH		127
#define DISPLAY_AREA_WIDTH		128
#define DISPLAY_AREA_HEIGHT		63

/****************************************************************************/


/****************************************************************************
 *  Global variables for other files:
****************************************************************************/


/****************************************************************************/



/****************************************************************************
 *  Global variables for this file:
****************************************************************************/


/****************************************************************************/


/****************************************************************************
 *  Function prototypes for this file:
****************************************************************************/


/****************************************************************************/

#if 1

//unsigned char * Single_To_Doube_cyw(unsigned char *Text)
//{


//}
/******************************************************************************
 * FunctionName: displayText
 *
 * Function Description:
 * This function draws a English text on screen within a rectangle.
 * It also provides options for creating border, highlight and curved corners
 * of rectangle.
 * The function is developed to support new LCD JLX12864G - 10163
 *
 * Function Parameters:
 * Text:				Text to be displayed.
 * XMin: 				x coordinate of upper left corner of rectangle.
 * YMin: 				y coordinate of upper left corner of rectangle.
 * HighlightEnable:		Enable or disable highlight on string.
 * BorderEnable: 		Enable or disable a rectangular border around string.
 * CurvedCornerEnable:	Enable or disable curved corners of rectangular border.
 * MenuBoundaryEnable:  Enable menu boundary for a menu item type display item
 *
 * Function Returns:
 *
 *
 ********************************************************************************/

uint8_t
displayText(unsigned char *Text, int16_t XMin, int16_t YMin, bool HighlightEnable,
			bool BorderEnable, bool CurvedCornerEnable, bool MenuBoundaryEnable,bool MenuNearOneByOneEnable,bool MenusmallEnable)
{
	int16_t lucXMin, lucXMax, lucYMin, lucYMax;
	uint16_t lucLCDTwoPagePixelData[128]; // It hold the pixel data for two pages of LCD. It consist of the data for the string to display with or without highlight
	uint8_t lucTemp;
//	unsigned char *Text_cyw;
	const unsigned char *tp_ss="BX SHUTTERS";


	uint8_t Tp_long,tp_i=0,tp_p_old = 0;
		//char text_his[44]={0};
		unsigned char text_new[44];

		Tp_long = strlen((char*)Text);
		if((strlen((char*)Text))>42)
		{
			return 0;
		}
		//memcpy(text_his,(char*)Text,Tp_long);
		//memset(text_new,0x20,44);

		for(tp_i = 0;tp_i<Tp_long;)
		{


		   if((Text[tp_i]&0x80)==0x80)
		    {

		    	text_new[tp_p_old++]=Text[tp_i];
		    	text_new[tp_p_old++]=Text[tp_i+1];
		    	tp_i= tp_i + 2;
		    	continue;
		    }
		    if((Text[tp_i]&0x80) == 0x00)//”ñ“ú?
		    {
		    	//tp_i= tp_i + 1;
		    	text_new[tp_p_old++]=0x20;
		        text_new[tp_p_old++]=Text[tp_i++];
		        continue;
		    }
		    tp_i++;
		}
		text_new[tp_p_old]=0;
		text_new[tp_p_old+1]=0;
	//(char*)Text = "BX SHUTTERS";
   // unsigned char *cccccc = "E321: ƒ†ƒEƒRƒEA_D";
	// Single_To_Doube_cyw(Text);
	//Single_To_Doube_cyw(cccccc);
	if((strlen((char*)text_new)/2)>21)
	{
		return 0;
	}
	if((strlen((char*)text_new)/2)==16)
	{
		if(XMin == 2) XMin= 1;
	}
	if((strlen((char*)text_new)/2)>16)
	{
		MenusmallEnable = true;
	}

	// Set Rectangle boundary
	//if(MenuBoundaryEnable == true)
	if ((HighlightEnable == true && MenuBoundaryEnable == true))
		lucXMin = 1;
	else
		lucXMin = XMin;

	//if(MenuBoundaryEnable == true)
	if (HighlightEnable == true && MenuBoundaryEnable == true)
		lucXMax = DISPLAY_AREA_WIDTH;
	else if(HighlightEnable == true || BorderEnable == true)
		//lucXMax = XMin + ((strlen((char*)Text)/2) * 6) + 8;
		lucXMax = XMin + ((strlen((char*)text_new)/2) * 9)+4 ;
	else
		//lucXMax = XMin + ((strlen((char*)Text)/2) * 6);
		lucXMax = XMin + ((strlen((char*)text_new)/2) * 9) ;
	enable_15display_cyw = 0;
	if(((strlen((char*)text_new)/2) == 15)||((strlen((char*)text_new)/2) == 16)||(strcmp((char*)tp_ss,(char*)text_new)==0)||(MenuNearOneByOneEnable == true))
	{
		enable_15display_cyw = 1;
		if (HighlightEnable == true && MenuBoundaryEnable == true)
				lucXMax = DISPLAY_AREA_WIDTH;
			else if(HighlightEnable == true || BorderEnable == true)
				//lucXMax = XMin + ((strlen((char*)Text)/2) * 6) + 8;
				lucXMax = XMin + ((strlen((char*)text_new)/2) * 8)+4 ;
			else
				//lucXMax = XMin + ((strlen((char*)Text)/2) * 6);
				lucXMax = XMin + ((strlen((char*)text_new)/2) * 8) ;
	}
    if(MenusmallEnable == true)
    {
    	enable_15display_cyw = 0;
    	if (HighlightEnable == true && MenuBoundaryEnable == true)
    			lucXMax = DISPLAY_AREA_WIDTH;
    		else if(HighlightEnable == true || BorderEnable == true)
    			lucXMax = XMin + ((strlen((char*)text_new)/2) * 6) +4;
    			//lucXMax = XMin + ((strlen((char*)Text)/2) * 9) + 5;
    		else
    			lucXMax = XMin + ((strlen((char*)text_new)/2) * 6);
    			//lucXMax = XMin + ((strlen((char*)Text)/2) * 9) ;
    }
	lucYMin = YMin;
	lucYMax = YMin + RECTANGLE_HEIGHT;

	// Check whether rectangle boundary is out of display area
	if(	((lucXMax > DISPLAY_AREA_WIDTH+1)) ||
		(lucYMax > DISPLAY_AREA_HEIGHT)
		)
	{
		return 0;
	}


	if(HighlightEnable == true && MenuBoundaryEnable != true)
	{

		//for (lucTemp = 0;lucTemp < 127;lucTemp++)
		for (lucTemp = 0;lucTemp < 128;lucTemp++)
		{

			lucLCDTwoPagePixelData[lucTemp] = 0x3FFE;

		}

	}
	else if(HighlightEnable == true && MenuBoundaryEnable == true)
	{

		//for (lucTemp = 0;lucTemp < 127;lucTemp++)
		for (lucTemp = 0;lucTemp < 128;lucTemp++)
		{

			lucLCDTwoPagePixelData[lucTemp] = 0x7FFF;

		}

	}
	else
	{

		//for (lucTemp = 0;lucTemp < 127;lucTemp++)
		for (lucTemp = 0;lucTemp < 128;lucTemp++)
		{

			lucLCDTwoPagePixelData[lucTemp] = 0x0000;

		}

	}

	if(CurvedCornerEnable==true)
	{

		lucLCDTwoPagePixelData[0] = 0x1FFC;

		lucLCDTwoPagePixelData[lucXMax - lucXMin - 1] = 0x1FFC;

	}

	//if(HighlightEnable== true || MenuBoundaryEnable == true)
	if(MenusmallEnable == false)
	{
	if ((HighlightEnable == true)&&(BorderEnable==true))
	{
		UpdateRamString_8x8_DOUBLE(text_new,lucLCDTwoPagePixelData + BORDER_SPACING_4,true);

	}
	else if ((HighlightEnable == true && MenuBoundaryEnable != true) || (HighlightEnable == true && MenuBoundaryEnable == true))
	{

		UpdateRamString_8x8_DOUBLE(text_new,lucLCDTwoPagePixelData + BORDER_SPACING,true);

	}

	else if (HighlightEnable == false && MenuBoundaryEnable == false && BorderEnable == true)
	{

		UpdateRamString_8x8_DOUBLE(text_new,lucLCDTwoPagePixelData + BORDER_SPACING,false);

	}

	else
	{

		UpdateRamString_8x8_DOUBLE(text_new,lucLCDTwoPagePixelData,false);

	}
	}
	else
	{
		if ((HighlightEnable == true)&&(BorderEnable==true))
			{
				UpdateRamString_5x7_DOUBLE(text_new,lucLCDTwoPagePixelData + BORDER_SPACING_4,true);

			}
		else if ((HighlightEnable == true && MenuBoundaryEnable != true) || (HighlightEnable == true && MenuBoundaryEnable == true))
			{

				UpdateRamString_5x7_DOUBLE(text_new,lucLCDTwoPagePixelData + BORDER_SPACING,true);

			}
			else if (HighlightEnable == false && MenuBoundaryEnable == false && BorderEnable == true)
			{

				UpdateRamString_5x7_DOUBLE(text_new,lucLCDTwoPagePixelData + BORDER_SPACING,false);

			}
			else
			{

				UpdateRamString_5x7_DOUBLE(text_new,lucLCDTwoPagePixelData,false);

			}
	}
	display_ram(((lucYMin / 8) + 1),lucXMin,lucXMax,lucLCDTwoPagePixelData);

	//return 1;
	if(MenusmallEnable == true)
	{
	return 	((BorderEnable == true)/*||(HighlightEnable == true)*/) ?
			(XMin + (strlen((char*)text_new)/2 ) * 6+4) :
			(XMin + (strlen((char*)text_new))/2 * 6);
	}
	else
	{
		if(enable_15display_cyw == 1)
		{
		return 	((BorderEnable == true)/*||(HighlightEnable == true)*/) ?
					(XMin + (strlen((char*)text_new)/2 ) * 8+4) :
					(XMin + (strlen((char*)text_new))/2 * 8);
		}
		else
		{
		return 	((BorderEnable == true)/*||(HighlightEnable == true)*/) ?
								(XMin + (strlen((char*)text_new)/2 ) * 9+4) :
								(XMin + (strlen((char*)text_new))/2 * 9);
		}
	}

}

/********************************************************************************/
#endif

//Commented by YPG on 8 Jan 2014 as same function need to modify to support new LCD with highlighted text
#if 0
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
 * MenuBoundaryEnable:  Enable menu boundary for a menu item type display item
 *
 * Function Returns: 	Returns the x coordinate of the top right corner of rectangle
 * 						or string whichever is enabled
 *
 *
 ********************************************************************************/
uint8_t
displayText(unsigned char *Text, int16_t XMin, int16_t YMin, bool HighlightEnable,
					bool BorderEnable, bool CurvedCornerEnable, bool MenuBoundaryEnable)
{
	int16_t lucXMin, lucXMax, lucYMin, lucYMax;

	// Set Rectangle boundary
	if(MenuBoundaryEnable == true)
		lucXMin = 1;
	else
		lucXMin = XMin;

	if(MenuBoundaryEnable == true)
		lucXMax = DISPLAY_AREA_WIDTH;
	else if(BorderEnable == true)
		lucXMax = XMin + ((strlen((char*)Text)+1) * 6);
	else
		lucXMax = XMin + ((strlen((char*)Text)) * 6);

	lucYMin = YMin;
	lucYMax = YMin + RECTANGLE_HEIGHT;

	// Check whether rectangle boundary is out of display area
	if(	(lucXMax > DISPLAY_AREA_WIDTH) ||
		(lucYMax > DISPLAY_AREA_HEIGHT)
		)
	{
		return 0;
	}

	// Highlight Rectangle
	if(HighlightEnable==true)
	{
		if(BorderEnable==true)
			GrRectDrawBolymin(lucXMin, lucXMax, lucYMin + 1, lucYMax - 1, false);

		if(MenuBoundaryEnable == true)
			GrRectFIllBolymin(lucXMin, lucXMax, lucYMin, lucYMax, false, true);
		else
			GrRectFIllBolymin(lucXMin, lucXMax, lucYMin + 1, lucYMax - 1, false, false);

		if(BorderEnable==true)
		{
			GrStringDrawBolymin(Text, strlen((char*)Text),
					XMin + BORDER_SPACING,YMin + BORDER_SPACING_FROM_TOP , true);
		}
		else
		{
			GrStringDrawBolymin(Text, strlen((char*)Text),
					XMin,YMin + BORDER_SPACING_FROM_TOP , true);
		}
	}
	else
	{
		GrRectFIllBolymin(lucXMin, lucXMax, lucYMin, lucYMax, true, true);

		if(BorderEnable==true)
		{
			//GrRectDrawBolymin(lucXMin, lucXMax, lucYMin, lucYMax, false);
			GrStringDrawBolymin(Text, strlen((char*)Text),
							XMin + BORDER_SPACING, YMin + BORDER_SPACING_FROM_TOP , false);
		}
		else
			GrStringDrawBolymin(Text, strlen((char*)Text),
										XMin, YMin + BORDER_SPACING_FROM_TOP , false);

		if(CurvedCornerEnable==true)
		{
//			GrLineDraw(&sContext,XMin,YMin+2,XMin+2,YMin);
//			GrLineDraw(&sContext,XMin,YMin + RECTANGLE_HEIGHT - 2,XMin+2,YMin + RECTANGLE_HEIGHT);
//			GrLineDraw(&sContext,XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8)-2,YMin,\
//								 XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8),YMin+2);
//			GrLineDraw(&sContext,XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8)-2,YMin + RECTANGLE_HEIGHT,\
//								 XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8),YMin + RECTANGLE_HEIGHT-2);
		}
	}

	// Draw curved corners of rectangle if curvedCornerEnable is true
	if(CurvedCornerEnable==true)
	{
//		GrPixelDraw(&sContextBlack,XMin,YMin);
//		GrPixelDraw(&sContextBlack,XMin,YMin + RECTANGLE_HEIGHT);
//		GrPixelDraw(&sContextBlack,XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8),YMin);
//		GrPixelDraw(&sContextBlack,XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8),YMin + RECTANGLE_HEIGHT);

		GrPixelDrawBolymin(XMin, YMin + 1, true, false);
		GrPixelDrawBolymin(XMin, YMin + RECTANGLE_HEIGHT - 1, true, false);
		GrPixelDrawBolymin(XMin + (strlen((char*)Text) + 1) * 6, YMin + 1, true, true);
		GrPixelDrawBolymin(XMin + (strlen((char*)Text) + 1) * 6, YMin + RECTANGLE_HEIGHT - 1, true, true);
	}

	return 	((BorderEnable == true)/*||(HighlightEnable == true)*/) ?
			(XMin + (strlen((char*)Text) + 1) * 6) :
			(XMin + (strlen((char*)Text)) * 6);
}

//uint8_t
//displayText(unsigned char *Text, int16_t XMin, int16_t YMin, bool HighlightEnable,
//					bool BorderEnable, bool CurvedCornerEnable, bool MenuBoundaryEnable)
//{
//		tRectangle sRect;
//	    tContext sContext, sContextBlack;
//
//	    GrContextInit(&sContext, &g_sCFAL96x64x16);
//	    GrContextInit(&sContextBlack, &g_sCFAL96x64x16);
//
//
//	    GrContextForegroundSet(&sContext, ClrWhite);
//	    GrContextForegroundSet(&sContextBlack, ClrBlack);
//
//	    GrContextFontSet(&sContext, g_psFontFixed6x8);
//	    GrContextFontSet(&sContextBlack, g_psFontFixed6x8);
//
//	    // Set Rectangle boundary
//	    sRect.i16XMin = XMin;
//
//	    if(MenuBoundaryEnable == true)
//	    	sRect.i16XMax = XMin + DISPLAY_AREA_WIDTH;
//	    else
//	    	sRect.i16XMax = XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8);
//
//	    sRect.i16YMin = YMin;
//	    sRect.i16YMax = YMin + RECTANGLE_HEIGHT;
//
//	    // Check whether rectangle boundary is out of display area
//	    if(	(sRect.i16XMax > DISPLAY_AREA_WIDTH) ||
//	    	(sRect.i16YMax > DISPLAY_AREA_HEIGHT)
//	    	)
//	    {
//	    	return 0;
//	    }
//
//	    // Highlight Rectangle
//	    if(HighlightEnable==true)
//	    {
//	    	GrRectFill(&sContext, &sRect);
//
//	    	if(BorderEnable==true)
//	    		GrRectDraw(&sContext, &sRect);
//	    	GrStringDraw(&sContextBlack, (char*)Text, -1,
//	    			XMin + BORDER_SPACING,YMin + BORDER_SPACING_FROM_TOP , false);
//	    }
//	    else
//	    {
//	    	GrRectFill(&sContextBlack, &sRect);
//
//	    	if(BorderEnable==true)
//	    	{
//	    		GrRectDraw(&sContext, &sRect);
//	    		GrStringDraw(&sContext, (char*)Text, -1, XMin + BORDER_SPACING,YMin + BORDER_SPACING_FROM_TOP , false);
//	    	}
//	    	else
//	    		GrStringDraw(&sContext, (char*)Text, -1, XMin, YMin + BORDER_SPACING_FROM_TOP , false);
//
//	    	if(CurvedCornerEnable==true)
//	    	{
//	    		GrLineDraw(&sContext,XMin,YMin+2,XMin+2,YMin);
//	    		GrLineDraw(&sContext,XMin,YMin + RECTANGLE_HEIGHT - 2,XMin+2,YMin + RECTANGLE_HEIGHT);
//	    		GrLineDraw(&sContext,XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8)-2,YMin,\
//	    							 XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8),YMin+2);
//	    		GrLineDraw(&sContext,XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8)-2,YMin + RECTANGLE_HEIGHT,\
//	    							 XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8),YMin + RECTANGLE_HEIGHT-2);
//	    	}
//	    }
//
//	    // Draw curved corners of rectangle if curvedCornerEnable is true
//	    if(CurvedCornerEnable==true){
//			 GrPixelDraw(&sContextBlack,XMin,YMin);
//			 GrLineDraw(&sContextBlack,XMin,YMin+1,XMin+1,YMin);
//
//			 GrPixelDraw(&sContextBlack,XMin,YMin + RECTANGLE_HEIGHT);
//			 GrLineDraw(&sContextBlack,XMin,YMin + RECTANGLE_HEIGHT - 1,XMin+1,YMin + RECTANGLE_HEIGHT);
//
//			 GrPixelDraw(&sContextBlack,XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8),YMin);
//			 GrLineDraw(&sContextBlack,XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8)-1,YMin,\
//					 XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8),YMin+1);
//
//			 GrPixelDraw(&sContextBlack,XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8),YMin + RECTANGLE_HEIGHT);
//			 GrLineDraw(&sContextBlack,XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8)-1,YMin + RECTANGLE_HEIGHT,\
//					 XMin + (strlen((char*)Text)+1)*GrFontMaxWidthGet(g_psFontFixed6x8),YMin+ RECTANGLE_HEIGHT -1);
//		}
//
//	    return 	((BorderEnable == true)||(HighlightEnable == true)) ?
//	    		(XMin + (strlen((char*)Text) + 1)*GrFontMaxWidthGet(g_psFontFixed6x8)) :
//	    		(XMin + (strlen((char*)Text))*GrFontMaxWidthGet(g_psFontFixed6x8));
//}
/********************************************************************************/
#endif
