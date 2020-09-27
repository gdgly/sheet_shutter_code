/*
 * ram.h
 *
 *  Created on: 2015��7��6��
 *      Author: x220
 */

#ifndef RAM_H_
#define RAM_H_


//#define GP2A054Register_DEBUG 0  //�����������054�ļĴ�����ֵ���е���
//#define UART_DEBUG 0//��������������Ƹ�Ӧ����ʱ���ͨ�����ڸ����Է�����
//#define GUEST_DISP 0//������ʾ��Һ����  �� �� �� �� ͣ
//#define GP2A054_DUBUG 0//����054 ��D0D1D2D3��������ʾ��LCD�� ��������Һ������ʾ������
//#define CLEAR_RAM_DEBUG 0//�����������������һ��ʱ�����������м��� �Ը������Ƶ�׼ȷûʲôЧ��
#define GUEST_LEVELOUT 0//���������������Ϊ������� ��������Һ������ʾ������ͻ


extern const unsigned char FONT_STOP[];
extern const unsigned char FONT_CLOSE[];
extern const unsigned char FONT_OPEN[];
extern const unsigned char FONT_TOP[];
extern const unsigned char FONT_LEFT[];
extern const unsigned char FONT_RIGHT[];
extern const unsigned char FONT_BOTTOM[];
extern const unsigned char  FONT_BUF_8_16[160];
extern unsigned char warray_r[58];
extern unsigned int LCD_Count;
extern unsigned char flag_stop;
extern unsigned char menu_gesture_flag_cyw;
extern unsigned char menu_gesture_flag_A007;
extern uint8_t shoushi_cyw;


void LCD_BACKLIGHT_TOGGLE(void);
void LED_AOTUMAU_TOGGLE(void);
//void LCD_BACKLIGHT_SETSTATUS(void);
//void LCD_BACKLIGHT_GETSTATUS(void);
void LCD_BACKLIGHT_OFF(void);
void Parameter_check(void);
void LED_AOTUMAU_ON(void);
void LED_AOTUMAU_OFF(void);
#endif /* RAM_H_ */
