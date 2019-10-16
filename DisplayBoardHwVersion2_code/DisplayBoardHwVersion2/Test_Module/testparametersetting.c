/*
 * testparametersetting.c
 *
 *  Created on: Jun 18, 2014
 *      Author: rk803609
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <driverlib/gpio.h>
#include "Application/ustdlib.h"
#include "Application/userinterface.h"
#include "Application/parameterlist.h"
#include "Middleware/display.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"

void paramSettingTestFunction(void)
{
	if(psActiveFunctionalBlock == &gsValueTypeParamFunctionalBlock)
	{
		if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		{
			if( (gstUMtoCMdatabase.dataToControlBoard.parameterNumber == gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex) &&
				(1 == gstUMtoCMdatabase.commandToControlBoard.bits.getParameter)
			  )
			{
				gstUMtoCMdatabase.getParameterValue = 33;

				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
				gstUMtoCMdatabase.acknowledgementReceived = eACK;
				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
			}

			if( (gstUMtoCMdatabase.dataToControlBoard.parameterNumber == gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex) &&
				(1 == gstUMtoCMdatabase.commandToControlBoard.bits.setParameter)
			  )
			{
				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
			}
		}
	}

	if(psActiveFunctionalBlock == &gsStateTypeParamFunctionalBlock)
	{
		if(gstUMtoCMdatabase.commandRequestStatus == eACTIVE)
		{
			if( (gstUMtoCMdatabase.dataToControlBoard.parameterNumber == gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex) &&
				(1 == gstUMtoCMdatabase.commandToControlBoard.bits.getParameter)
			  )
			{
				gstUMtoCMdatabase.getParameterValue = 1;

				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
				gstUMtoCMdatabase.acknowledgementReceived = eACK;
				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
			}

			if( (gstUMtoCMdatabase.dataToControlBoard.parameterNumber == gsParamDatabase[gHighlightedItemIndex].paramEEPROMIndex) &&
				(1 == gstUMtoCMdatabase.commandToControlBoard.bits.setParameter)
			  )
			{
				gstUMtoCMdatabase.commandResponseStatus = eSUCCESS;
				gstUMtoCMdatabase.dataToControlBoard.parameterNumber = 0;
			}
		}
	}
}



