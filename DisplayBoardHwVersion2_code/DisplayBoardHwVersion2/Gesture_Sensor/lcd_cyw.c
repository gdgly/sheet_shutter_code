/*
 * lcd.c
 *
 *  Created on: 2015��7��6��
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
   LCD_Select;//ѡ��
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
   LCD_Select;//ѡ��
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

	LCD_Select;//ѡ��
   column  = column -1;
   page    = page   -1;
   send_command(0xb0+page);
   send_command(((column>>4)&0x0f)+0x10);
   send_command(column&0x0f);
}*/



//����Ļ
void clear_clear(void)
{
   unsigned char i,j;

   LCD_Select;//ѡ��
   for(i=0;i<9;i++)
   {

      lcd_address(1+i,1);
      for(j=0;j<132;j++)
      {
         send_data(0x00);
      }
   }
   LCD_UnSelect;//��ѡ��

}
void meun_open(void)
{
   unsigned char i,j;
   //ClearWDT(); // Service the WDT
   //lcd_init();
   LCD_Select;//ѡ��
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
   LCD_UnSelect;//��ѡ��

}

void meun_close(void)
{
   unsigned char i,j;
   //ClearWDT(); // Service the WDT
   //lcd_init();
   LCD_Select;//ѡ��
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
   LCD_UnSelect;//��ѡ��
}

void meun_stop(void)
{
   unsigned char i,j;
   //ClearWDT(); // Service the WDT
  // lcd_init();
   LCD_Select;//ѡ��
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
   LCD_UnSelect;//��ѡ��
}

void meun_right(void)
{
   unsigned char i,j;
   //ClearWDT(); // Service the WDT
  // lcd_init();
   LCD_Select;//ѡ��
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
   LCD_UnSelect;//��ѡ��
}

void meun_bottom(void)
{
    unsigned char i,j;
      //ClearWDT(); // Service the WDT
     // lcd_init();
      LCD_Select;//ѡ��
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
      LCD_UnSelect;//��ѡ��
}

void meun_top(void)
{
    unsigned char i,j;
      //ClearWDT(); // Service the WDT
     // lcd_init();
      LCD_Select;//ѡ��
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
      LCD_UnSelect;//��ѡ��
}

void meun_left(void)
{
    unsigned char i,j;
      //ClearWDT(); // Service the WDT
     // lcd_init();
      LCD_Select;//ѡ��
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
      LCD_UnSelect;//��ѡ��
}

void display_graphic_8x16(unsigned char column,unsigned char page,const unsigned char *dp,unsigned char x_flag)
{
    unsigned char  i,j;
    //cs1=0;

    LCD_Select;//ѡ��
    for(j=0; j<2; j++)
    {
        lcd_address(page,column+j*8);
        for (i=0; i<8; i++)
        {
            if(x_flag == 0)
            {
            send_data(*dp);                 /*д���ݵ�LCD,ÿд��һ��8λ�����ݺ��е�ַ�Զ���1*/
            }
            else
            {
            send_data(~(*dp));
            }
            dp++;
        }
    }
    //cs1=1;
    LCD_UnSelect;//��ѡ��
}

//*****************************************************************************
//
// Toggle a GPIO.
//
//*****************************************************************************
//Һ����ʼ��
/*void  lcd_init(void)
{
  // if(FLAG_INITLCD == 0)
  // {
  // FLAG_INITLCD = 1;
   LCD_Select;//ѡ��
   LCD_Reset;//�͵�ƽ��λ
   ROM_SysCtlDelay(100);
   LCD_UnReset;//��λ���
   ROM_SysCtlDelay(20);
   send_command(0xe2);//�����λ
   ROM_SysCtlDelay(5);
   send_command(0x2c);//��ѹ1
   ROM_SysCtlDelay(5);
   send_command(0x2e);//��ѹ2
   ROM_SysCtlDelay(5);
   send_command(0x2f);//��ѹ3
   ROM_SysCtlDelay(5);
   send_command(0x24);//�ֵ��Աȶ�
   send_command(0x81);΢���Աȶ�
   send_command(0x1a);΢���Աȶȵ�ֵ�������÷�Χ0��63
   send_command(0xa2);1/9ƫѹ�ȣ�bias��
   send_command(0xc8);��ɨ��˳�򣺴��ϵ���
   send_command(0xa1);��ɨ��˳�򣺴�����
   send_command(0x40);//��ʾ��ʼ������
   send_command(0xaf); ����ʾ


   LCD_ON;

   clear_clear();

  // meun_close();
   //display_dianci(0,0,3,0);//�����ģʽ�޹�

  // PIN_LCD_LIGHT = 0;//�ر���
   //PIN_LCD_SEL  = 1;//��ѡ��

  // FLAG_POWER = 0;//�����־��ֹҺ������ �������߲��ֵ��µĹػ� '

   //if(FG_PWRON==0)
   //  FG_PWRON = 1;
  // }
}*/

