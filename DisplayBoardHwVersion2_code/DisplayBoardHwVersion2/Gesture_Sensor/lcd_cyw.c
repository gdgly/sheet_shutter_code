/*
 * lcd.c
 *
 *  Created on: 2015年7月6日
 *      Author: x220
 */

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "lcd_cyw.h"
#include "ram_cyw.h"
#include "Middleware/bolymindisplay.h"



/*void send_command(int data)
{
   char i;
   LCD_Select;//选中
   LCD_Command;
   for(i=0;i<8;i++)
   {
	   LCD_CLKLow;
     if(data&0x80)
    	 LCD_SDAHigh;
     else
    	 LCD_SDALow;
     LCD_CLKHigh;
     data= data <<1;
   }
}

void send_data(int data)
{
   char i;
   LCD_Select;//选中
   LCD_Data;
   for(i=0;i<8;i++)
   {
	   LCD_CLKLow;
     if(data&0x80)
    	 LCD_SDAHigh;
     else
    	 LCD_SDALow;
     LCD_CLKHigh;
     data= data <<1;
   }
}

void lcd_address(unsigned char page,unsigned char column)
{

	LCD_Select;//选中
   column  = column -1;
   page    = page   -1;
   send_command(0xb0+page);
   send_command(((column>>4)&0x0f)+0x10);
   send_command(column&0x0f);
}*/



//清屏幕
void clear_clear(void)
{
   unsigned char i,j;

   LCD_Select;//选中
   for(i=0;i<9;i++)
   {

      lcd_address(1+i,1);
      for(j=0;j<132;j++)
      {
         send_data(0x00);
      }
   }
   LCD_UnSelect;//不选中

}
void meun_open(void)
{
   unsigned char i,j;
   //ClearWDT(); // Service the WDT
   //lcd_init();
   LCD_Select;//选中
   for(i=0;i<9;i++)
   {

      lcd_address(1+i,1);
      for(j=0;j<132;j++)
     // for(j=0;j<128;j++)
      {
        if(j<128)
        send_data(FONT_OPEN[j+i*128]);
        else
        send_data(0x00);
      }
   }
   LCD_UnSelect;//不选中

}

void meun_close(void)
{
   unsigned char i,j;
   //ClearWDT(); // Service the WDT
   //lcd_init();
   LCD_Select;//选中
   for(i=0;i<9;i++)
   {

      lcd_address(1+i,1);
      for(j=0;j<132;j++)
     // for(j=0;j<128;j++)
      {
        if(j<128)
        send_data(FONT_CLOSE[j+i*128]);
        else
        send_data(0x00);
      }
   }
   LCD_UnSelect;//不选中
}

void meun_stop(void)
{
   unsigned char i,j;
   //ClearWDT(); // Service the WDT
  // lcd_init();
   LCD_Select;//选中
   for(i=0;i<9;i++)
   {

      lcd_address(1+i,1);
      for(j=0;j<132;j++)
      //for(j=0;j<128;j++)
      {
        if(j<128)
        send_data(FONT_STOP[j+i*128]);
        else
         send_data(0x00);
      }
   }
   LCD_UnSelect;//不选中
}

void meun_right(void)
{
   unsigned char i,j;
   //ClearWDT(); // Service the WDT
  // lcd_init();
   LCD_Select;//选中
   for(i=0;i<9;i++)
   {

      lcd_address(1+i,1);
      for(j=0;j<132;j++)
      //for(j=0;j<128;j++)
      {
        if(j<128)
        send_data(FONT_RIGHT[j+i*128]);
        else
         send_data(0x00);
      }
   }
   LCD_UnSelect;//不选中
}

void meun_bottom(void)
{
    unsigned char i,j;
      //ClearWDT(); // Service the WDT
     // lcd_init();
      LCD_Select;//选中
      for(i=0;i<9;i++)
      {

         lcd_address(1+i,1);
         for(j=0;j<132;j++)
         //for(j=0;j<128;j++)
         {
           if(j<128)
           send_data(FONT_BOTTOM[j+i*128]);
           else
            send_data(0x00);
         }
      }
      LCD_UnSelect;//不选中
}

void meun_top(void)
{
    unsigned char i,j;
      //ClearWDT(); // Service the WDT
     // lcd_init();
      LCD_Select;//选中
      for(i=0;i<9;i++)
      {

         lcd_address(1+i,1);
         for(j=0;j<132;j++)
         //for(j=0;j<128;j++)
         {
           if(j<128)
           send_data(FONT_TOP[j+i*128]);
           else
            send_data(0x00);
         }
      }
      LCD_UnSelect;//不选中
}

void meun_left(void)
{
    unsigned char i,j;
      //ClearWDT(); // Service the WDT
     // lcd_init();
      LCD_Select;//选中
      for(i=0;i<9;i++)
      {

         lcd_address(1+i,1);
         for(j=0;j<132;j++)
         //for(j=0;j<128;j++)
         {
           if(j<128)
           send_data(FONT_LEFT[j+i*128]);
           else
            send_data(0x00);
         }
      }
      LCD_UnSelect;//不选中
}

void display_graphic_8x16(unsigned char column,unsigned char page,const unsigned char *dp,unsigned char x_flag)
{
    unsigned char  i,j;
    //cs1=0;

    LCD_Select;//选中
    for(j=0; j<2; j++)
    {
        lcd_address(page,column+j*8);
        for (i=0; i<8; i++)
        {
            if(x_flag == 0)
            {
            send_data(*dp);                 /*写数据到LCD,每写完一个8位的数据后列地址自动加1*/
            }
            else
            {
            send_data(~(*dp));
            }
            dp++;
        }
    }
    //cs1=1;
    LCD_UnSelect;//不选中
}

//*****************************************************************************
//
// Toggle a GPIO.
//
//*****************************************************************************
//液晶初始化
/*void  lcd_init(void)
{
  // if(FLAG_INITLCD == 0)
  // {
  // FLAG_INITLCD = 1;
   LCD_Select;//选中
   LCD_Reset;//低电平复位
   ROM_SysCtlDelay(100);
   LCD_UnReset;//复位完毕
   ROM_SysCtlDelay(20);
   send_command(0xe2);//软件复位
   ROM_SysCtlDelay(5);
   send_command(0x2c);//升压1
   ROM_SysCtlDelay(5);
   send_command(0x2e);//升压2
   ROM_SysCtlDelay(5);
   send_command(0x2f);//升压3
   ROM_SysCtlDelay(5);
   send_command(0x24);//粗调对比度
   send_command(0x81);微调对比度
   send_command(0x1a);微调对比度的值，可设置范围0～63
   send_command(0xa2);1/9偏压比（bias）
   send_command(0xc8);行扫描顺序：从上到下
   send_command(0xa1);列扫描顺序：从左到右
   send_command(0x40);//显示初始行设置
   send_command(0xaf); 开显示


   LCD_ON;

   clear_clear();

  // meun_close();
   //display_dianci(0,0,3,0);//电池与模式无关

  // PIN_LCD_LIGHT = 0;//关背光
   //PIN_LCD_SEL  = 1;//不选中

  // FLAG_POWER = 0;//这个标志防止液晶打开了 由于无线部分导致的关机 '

   //if(FG_PWRON==0)
   //  FG_PWRON = 1;
  // }
}*/

