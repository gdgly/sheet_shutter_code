/*********************************************************************************
 * FileName: wirelessset_cyw.c
 * Description: Code for Parameter Initialization screen
 * Version: 0.1D
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
 *  Revision        Date                  Name                      Comments
 *      0.1D    20/06/2014          iGATE Offshore team       Initial Creation
 ****************************************************************************/

/****************************************************************************
 *  Includes:
****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "userinterface.h"
#include "Middleware/display.h"
#include "Middleware/bolymindisplay.h"
#include "Middleware/debounce.h"
#include "communicationmodule.h"
#include "Middleware/paramdatabase.h"

#define  WIRELESS_MODECHOOES  0
#define  WIRELESS_DATAWAIT    1
#define  WIRELESS_STATEOK     2
#define  WIRELESS_STATENG     3
#define  WIRELESS_STATEGOON   4

extern _CommunicationModuleInnerTaskComm lstCommunicationModuleInnerTaskComm;
uint8_t  Flag_askok_wireless = 0;

uint8_t flag_wireless_newold = 0;
uint8_t wireless_state       = 0;
uint16_t time_val_askloginok  = 0;
uint8_t flag_commask_loginok = 0;

uint8_t wirelessPaintScreencyw(void)
{
	GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
	wireless_state = 0;
	if(gu8_language == Japanese_IDX)
	{
	displayText("ツイカトウロク", 2,  0, true, false, false, true, false, false);
	displayText("シンキトウロク", 2, 16, false, false, false, false, false, false);
	}
	else
	{
	displayText("ADD LOGIN", 2,  0, true, false, false, true, false, false);
	displayText("NEW LOGIN", 2, 16, false, false, false, false, false, false);
	}
	return 0;
}

uint8_t wirelessRunTimecyw(void)
{
    switch(wireless_state)
    {
       case WIRELESS_MODECHOOES:

    	   if((Flag_askok_wireless == lstCommunicationModuleInnerTaskComm.additionalCommandData)&&(Flag_askok_wireless!=0))
    	       	   {
    	       		   Flag_askok_wireless = 0;
    	       		   if((flag_wireless_newold==0)&&(lstCommunicationModuleInnerTaskComm.additionalCommandData ==0x01))
    	       		   {
    	       			 GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
    	       			if(gu8_language == Japanese_IDX)
    	       			{
    	       			 displayText("ツイカトウロク", 2,  0, false, false, false, false, false, false);
    	       			 displayText("リモコンノボタンヲオシテクダサイ", 2, 16, false, false, false, false, false, false);
    	       			}
    	       			else
    	       			{
    	       			displayText("ADD LOGIN", 2,  0, false, false, false, false, false, true);
    	       		    displayText("PLEASE SEND WIRELESS", 2, 16, false, false, false, false, false, true);
    	       			}
    	       			 wireless_state = WIRELESS_DATAWAIT;
    	       			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
    	       			 lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
    	       			 lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.recover_anomaly = 0;
    	       			lstCommunicationModuleInnerTaskComm.commandToControlBoard.val=0;
    	       			lstCommunicationModuleInnerTaskComm.additionalCommandData = 0;
    	       			time_val_askloginok = 0;
    	       			 //lstCommunicationModuleInnerTaskComm.acknowledgementReceived = eNO_ACK;
    	       		   }
    	       		   if((flag_wireless_newold==1)&&(lstCommunicationModuleInnerTaskComm.additionalCommandData ==0x02))
    	       		   {
    	       			 GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
    	       			if(gu8_language == Japanese_IDX)
    	       			{
    	       			 displayText("シンキトウロク", 2,  0, false, false, false, false, false, false);
    	       			 displayText("リモコンノボタンヲオシテクダサイ", 2, 16, false, false, false, false, false, false);
    	       			}
    	       			else
    	       			{
    	       			 displayText("NEW LOGIN", 2,  0, false, false, false, false, false, true);
    	       			 displayText("PLEASE SEND WIRELESS", 2, 16, false, false, false, false, false, true);
    	       			}

    	       			 wireless_state = WIRELESS_DATAWAIT;
    	       			lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
    	       			lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
    	       			lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.recover_anomaly = 0;
    	       			lstCommunicationModuleInnerTaskComm.commandToControlBoard.val=0;
    	       			lstCommunicationModuleInnerTaskComm.additionalCommandData =0;
    	       			time_val_askloginok = 0;
    	       			//lstCommunicationModuleInnerTaskComm.acknowledgementReceived = eNO_ACK;
    	       		   }
    	       	   }
           break;
       case WIRELESS_DATAWAIT:
    	   if((Flag_askok_wireless == lstCommunicationModuleInnerTaskComm.additionalCommandData)&&((Flag_askok_wireless!=0)))
    	   {
    		   Flag_askok_wireless = 0;
    		   if(lstCommunicationModuleInnerTaskComm.additionalCommandData ==0x03)
    		   {
    			   flag_wireless_newold  = 0;
    			  	psActiveFunctionalBlock = &gsMenuFunctionalBlock;
    			  	 psActiveFunctionalBlock->pfnPaintFirstScreen();
    			  	 wireless_state = WIRELESS_MODECHOOES;
    			  	lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
    			  	lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
    			  	lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.recover_anomaly = 0;
    			  	lstCommunicationModuleInnerTaskComm.commandToControlBoard.val=0;
    			  	lstCommunicationModuleInnerTaskComm.additionalCommandData = 0;
    		   }
    		   if(lstCommunicationModuleInnerTaskComm.additionalCommandData ==0x04)
    		   {
    			   if(flag_commask_loginok==1)
    			   {
    				   flag_commask_loginok = 0;
    				   lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
    				   lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
    				   lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.recover_anomaly = 0;
    				   lstCommunicationModuleInnerTaskComm.commandToControlBoard.val=0;
    				   lstCommunicationModuleInnerTaskComm.additionalCommandData = 0;
    				   wireless_state = WIRELESS_STATEOK;
    				   time_val_askloginok = 0;
    				   GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
    				   if(gu8_language == Japanese_IDX)
    					 displayText("トウロクカンリョウ", 2, 0, false, false, false, false, false, false);
    				   else
    					 displayText("SUCCESSFUL", 2, 0, false, false, false, false, false, true);
    			   }
    			   else
    			   {
    				   lstCommunicationModuleInnerTaskComm.commandRequestStatus = eINACTIVE;
    				   lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
    				   lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.recover_anomaly = 0;
    				   lstCommunicationModuleInnerTaskComm.commandToControlBoard.val=0;
    				   lstCommunicationModuleInnerTaskComm.additionalCommandData = 0;
    			   }
    		   }
    	   }
    	   time_val_askloginok++;
    	   if(time_val_askloginok > 120 )
    	   {
    		   time_val_askloginok = 0;
    		   lstCommunicationModuleInnerTaskComm.commandToControlBoard.val=0x10000000;
    		   lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
    		   lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE ;
    		   lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
    		   lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.recover_anomaly=1;
    		   lstCommunicationModuleInnerTaskComm.additionalCommandData =0x04;
    	   }
    	   break;
       case WIRELESS_STATEOK:
    	   time_val_askloginok++;
    	   if(time_val_askloginok > 250 )
    	    {
    		   time_val_askloginok = 0;
    		   wireless_state = WIRELESS_DATAWAIT;
    		   if(flag_wireless_newold==0)
    		   {
    			   GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
    			   if(gu8_language == Japanese_IDX)
    			   {
    			   displayText("ツイカトウロク", 2,  0, false, false, false, false, false, false);
    			  displayText("リモコンノボタンヲオシテクダサイ", 2, 16, false, false, false, false, false, false);
    			   }
    			   else
    			   {
    				   displayText("ADD LOGIN", 2,  0, false, false, false, false, false, true);
    				   displayText("PLEASE SEND WIRELESS", 2, 16, false, false, false, false, false, true);
    			   }

    		   }
    		   if(flag_wireless_newold==1)
    		   {
    			   GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
    			   if(gu8_language == Japanese_IDX)
    			   {
    			   displayText("シンキトウロク", 2,  0, false, false, false, false, false, false);
    			   displayText("リモコンノボタンヲオシテクダサイ", 2, 16, false, false, false, false, false, false);
    			   }
    			   else
    			   {
    			   displayText("NEW LOGIN", 2,  0, false, false, false, false, false, true);
    			   displayText("PLEASE SEND WIRELESS", 2, 16, false, false, false, false, false, true);
    			   }
    		   }

    	    }
    	    break;
       case WIRELESS_STATENG:
    	   break;
       case WIRELESS_STATEGOON:
    	   break;
       default:
    	   break;
    }
	return 0;
}

uint8_t wirelessUpcyw(void)
{
	 if(gKeysStatus.bits.Key_Up_pressed)
	    {
		 gKeysStatus.bits.Key_Up_pressed = 0;
		 switch(wireless_state)
	      {
	       case WIRELESS_MODECHOOES:
	    	   if(flag_wireless_newold == 1)
	    	   {
	    		   flag_wireless_newold = 0;
	    		   GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
	    		   if(gu8_language == Japanese_IDX)
	    		   {
	    		   displayText("ツイカトウロク", 2,  0, true, false, false, true, false, false);
	    		  displayText("シンキトウロク", 2, 16, false, false, false, false, false, false);
	    		   }
	    		   else
	    		   {
	    			  displayText("ADD LOGIN", 2,  0, true, false, false, true, false, false);
	    			  displayText("NEW LOGIN", 2, 16, false, false, false, false, false, false);
	    		   }
	    	   }

	           break;
	       case WIRELESS_DATAWAIT:
	    	   break;
	       case WIRELESS_STATEOK:
	    	    break;
	       case WIRELESS_STATENG:
	    	   break;
	       case WIRELESS_STATEGOON:
	    	   break;
	       default:
	    	   break;
	      }
	    }
	return 0;
}

uint8_t wirelessDowncyw(void)
{
	if(gKeysStatus.bits.Key_Down_pressed)
		    {
		gKeysStatus.bits.Key_Down_pressed = 0;
	switch(wireless_state)
	    {
	       case WIRELESS_MODECHOOES:
	    	   if(flag_wireless_newold == 0)
	    	   {
	    	  	    flag_wireless_newold = 1;
	    	  	    GrRectFIllBolymin(0, 127, 0, 63, 0x00, true);
	    	  	  if(gu8_language == Japanese_IDX)
	    	  	  	 {
	    	  	    displayText("ツイカトウロク", 2,  0, false, false, false, false, false, false);
	    	  	   	displayText("シンキトウロク", 2, 16, true, false, false, true, false, false);
	    	  	  	 }
	    	  	  else
	    	  	  {
	    	  		 displayText("ADD LOGIN", 2,  0, false, false, false, false, false, false);
	    	  	    displayText("NEW LOGIN", 2, 16, true, false, false, true, false, false);
	    	  	  }

	    	  	}
	           break;
	       case WIRELESS_DATAWAIT:
	    	   break;
	       case WIRELESS_STATEOK:
	    	    break;
	       case WIRELESS_STATENG:
	    	   break;
	       case WIRELESS_STATEGOON:
	    	   break;
	       default:
	    	   break;
	    }
		    }
	return 0;
}

uint8_t wirelessModecyw(void)
{
	uint8_t Tp_exitlogin[7]={0x01,0x02,0x07,0x55,0x03,0xe8,0xf2};

	if(gKeysStatus.bits.Key_Mode_pressed)
	    {
		 gKeysStatus.bits.Key_Mode_pressed = 0;
	     switch(wireless_state)
	     {
	       case WIRELESS_MODECHOOES:
	    	   flag_wireless_newold  = 0;
	    	   psActiveFunctionalBlock = &gsMenuFunctionalBlock;
	    	   psActiveFunctionalBlock->pfnPaintFirstScreen();
	           break;
	       case WIRELESS_DATAWAIT:
	    	 //  uartSendTxBuffer(UART_control,Tp_exitlogin,7);
	    	   lstCommunicationModuleInnerTaskComm.commandToControlBoard.val=0x10000000;
	    	  lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
	    	  lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE ;
	    	  lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
	    	  lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.recover_anomaly=1;
	    	  lstCommunicationModuleInnerTaskComm.additionalCommandData =0x03;

	    	   break;
	       case WIRELESS_STATEOK:
	    	    break;
	       case WIRELESS_STATENG:
	    	   break;
	       case WIRELESS_STATEGOON:
	    	   break;
	       default:
	    	   break;
	     }
	    }
      return 0;
}

uint8_t wirelessEntercyw(void)
{
	//uint8_t Tp_cyedata[6]={0x01,0x02,0x06,0x55,0xa3,0x63};
	uint8_t Tp_addlogin[7]={0x01,0x02,0x07,0x55,0x01,0x29,0x73};
	uint8_t Tp_newlogin[7]={0x01,0x02,0x07,0x55,0x02,0x28,0x33};
	 if(gKeysStatus.bits.Key_Enter_pressed)
	       {
		 gKeysStatus.bits.Key_Enter_pressed = 0;
	switch(wireless_state)
	    {
	       case WIRELESS_MODECHOOES:
	    	  // psActiveFunctionalBlock = &gsMenuFunctionalBlock;
	    	  // psActiveFunctionalBlock->pfnPaintFirstScreen();
               if(flag_wireless_newold ==0)
               {


	    	  lstCommunicationModuleInnerTaskComm.commandToControlBoard.val=0x10000000;
	    	  lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
	    	   lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE ;
	    	   lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
	    	   lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.recover_anomaly=1;
	    	   lstCommunicationModuleInnerTaskComm.additionalCommandData =0x01;
            	 //uartSendTxBuffer(UART_control,Tp_addlogin,7);
               }
               if(flag_wireless_newold ==1)
               {

            	  // uartSendTxBuffer(UART_control,Tp_newlogin,7);
            	   lstCommunicationModuleInnerTaskComm.commandToControlBoard.val=0x10000000;
            	  lstCommunicationModuleInnerTaskComm.destination = eDestControlBoard;
            	  lstCommunicationModuleInnerTaskComm.commandRequestStatus = eACTIVE ;
            	  lstCommunicationModuleInnerTaskComm.commandResponseStatus = eNO_STATUS;
            	  lstCommunicationModuleInnerTaskComm.commandToControlBoard.bits.recover_anomaly=1;
            	  lstCommunicationModuleInnerTaskComm.additionalCommandData =0x02;
               }
	           break;
	       case WIRELESS_DATAWAIT:
	    	   break;
	       case WIRELESS_STATEOK:
	    	    break;
	       case WIRELESS_STATENG:
	    	   break;
	       case WIRELESS_STATEGOON:
	    	   break;
	       default:
	    	   break;
	    }
	       }
	return 0;
}

stInternalFunctions gsWirelessFunctionalBlockcyw =
{
		0,
		&gsMenuFunctionalBlock,
		wirelessPaintScreencyw,
		wirelessRunTimecyw,
		wirelessUpcyw,
		wirelessDowncyw,
		wirelessModecyw,
		wirelessEntercyw
};
