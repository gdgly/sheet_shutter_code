/*
 * test.c
 *
 *  Created on: Mar 28, 2014
 *      Author: rk803609
 */
#include <stdbool.h>
#include <stdint.h>
#include "Middleware/display.h"
#include "Middleware/bolymindisplay.h"

void testDisplayTopLeft()
{
	bolyminDisplayInit();
	displayText("Test",0,0,false,true,false,false,false,false);
}

void testDisplayMiddleArea()
{
	bolyminDisplayInit();
	displayText("Test",48,32,false,true,false,false,false,false);
}

void testDisplayBottomRight()
{
	bolyminDisplayInit();
	displayText("Test",65,49,false,true,false,false,false,false);
}

void testDisplayHighlighted()
{
	bolyminDisplayInit();
	displayText("Test",2,2,true,true,false,false,false,false);
}

void testDisplayCurvedBorders()
{
	bolyminDisplayInit();
	displayText("Test",2,2,false,true,true,false,false,false);
}

void testDisplayOutOfBounds()
{
	bolyminDisplayInit();
	displayText("Test",99,69,false,true,false,false,false,false);
}

void testDisplayLongString()
{
	bolyminDisplayInit();
	displayText("longer string to be displayed",2,2,false,true,false,false,false,false);
}
