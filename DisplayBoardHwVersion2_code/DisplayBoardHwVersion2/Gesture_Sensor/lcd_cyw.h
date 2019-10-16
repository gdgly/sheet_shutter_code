/*
 * lcd.h
 *
 *  Created on: 2015Äê7ÔÂ6ÈÕ
 *      Author: x220
 */

#ifndef LCD_H_
#define LCD_H_


#define LCD_ON  ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_PIN_3)
#define LCD_OFF  ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0)
#define LCD_Select ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0)
#define LCD_UnSelect ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5)
#define LCD_Reset  ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0)
#define LCD_UnReset  ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_PIN_2)
#define LCD_Command  ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, 0)
#define LCD_Data  ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, GPIO_PIN_7)
#define LCD_SDAHigh ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_PIN_6)
#define LCD_SDALow ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_6, 0)
#define LCD_CLKHigh ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_PIN_4)
#define LCD_CLKLow ROM_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_4, 0)


void lcd_init(void);
void meun_stop(void);
void meun_open(void);
void meun_close(void);
void meun_right(void);
void meun_left(void);
void meun_top(void);
void meun_bottom(void);
void clear_clear(void);

void display_graphic_8x16(unsigned char column,unsigned char page,const unsigned char *dp,unsigned char x_flag);

#endif /* LCD_H_ */
