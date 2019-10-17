/*
 * ram.h
 *
 *  Created on: 2015年7月6日
 *      Author: x220
 */

#ifndef RAM_H_
#define RAM_H_


//#define GP2A054Register_DEBUG 0  //定义了这个对054的寄存器器值进行调试
//#define UART_DEBUG 0//定义了这个在手势感应到的时候会通过串口给电脑发数据
//#define GUEST_DISP 0//手势显示在液晶上  上 下 左 右 停
//#define GP2A054_DUBUG 0//调试054 把D0D1D2D3的数据显示在LCD上 与手势在液晶上显示不兼容
//#define CLEAR_RAM_DEBUG 0//定义了这个变量会在一定时候清除计算的中间结果 对改善手势的准确没什么效果
#define GUEST_LEVELOUT 0//定义了这个手势作为脉冲输出 与手势在液晶上显示并不冲突


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




#endif /* RAM_H_ */
