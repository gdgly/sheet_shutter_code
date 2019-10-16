/*
 * GP2AP054A.h
 *
 *  Created on: 2015年7月7日
 *      Author: x220
 */

#ifndef GP2AP054A_H_
#define GP2AP054A_H_


#include <stdint.h>
#include <stdbool.h>
#include <driverlib/gpio.h>



typedef unsigned char u8 ;
typedef signed char s8 ;
typedef unsigned short int u16 ;
typedef signed short int s16 ;
typedef unsigned int u32;
typedef signed int s32;

//// Constants for I2C ////
extern const u8 SLAVE_ADDR;
extern const u8 WRITE_NUM 				;

extern const u8 PIC_I2C_ACK 			;
extern const u8 INIT_REG_ON			;
extern const u8 INIT_REG_OFF			;
extern const u8 PS_ONLY_MODE			;
extern const u8 GS_ONLY_MODE			;
extern const u8 PS_GS_MODE				;
extern const u8 ALS_ONLY_MODE			;
extern const u8 PS_ALS_MODE			;
extern const u8 GS_ALS_MODE			;
extern const u8 GS_PS_ALS_MODE			;
extern const u8 BLINK_MODE				;
extern const u8 PULSE_MODE				;

//// Constants for GS ////
extern const u8 GS_A_MODE 				;
extern const u8 GS_B_MODE 				;

extern const u8 STATE_ONE 				;
extern const u8 STATE_TWO 				;
extern const u8 STATE_THREE 			;
extern const u16 OVER_FLOW_DATA		;
extern const u8 DIR_RIGHT 				;
extern const u8 DIR_LEFT 				;
extern const u8 DIR_TOP 				;
extern const u8 DIR_BOTTOM 			;

extern const u8 COUNTS_HIGH_SP			;
extern const u8 COUNTS_MID_SP			;
extern const u8 SPEED_HIGH 			;
extern const u8 SPEED_MID 				;
extern const u8 STANDARD_SAMPLING_RATE	;
extern const u8 ZOOM_LEVEL1			;
extern const u8 ZOOM_LEVEL2			;
extern const u8 ZOOM_LEVEL3			;
extern const u8 ZOOM_LEVEL4			;
extern const u8 ZOOM_LEVEL5			;
extern const u8 ZOOM_LEVEL_DEF			;

extern const u8 pre3					;
extern const u8 pre2					;
extern const u8 pre1					;
extern const u8 now					;

extern const u8 FAST_PULSE_RANGE		;
extern const u8 SLOW_PULSE_RANGE		;


//// Constants for RGB ////
extern const u8 LOW_LUX 				;
extern const u8 HIGH_LUX 				;
extern const u8 MID_LUX				;

extern const u32 MAX_LUX_VALUE			;
extern const u32 OVER_FLOW_COUNT		;
//// Compensation matrix for Lux Calculation ////
extern const u8 HIGH_LUX_RANGE			;
extern const u8 MID_LUX_RANGE			;
extern const u8 LOW_LUX_RANGE			;

extern const u32 CHANGEOVER_VAL_M		;
extern const u32 CHANGEOVER_VAL_H		;

extern const u16 ALS_L_to_M_counts		;
extern const u16 ALS_M_to_H_counts		;

extern const u16 ALS_H_to_M_counts		;
extern const u16 ALS_M_to_L_counts		;

extern const u8 zero_lux_th			;

//// Constants for LCD ////
extern const u8 LCD_CLEAR_COUNTS 		;


extern const float alfa1				;
extern const float alfa2				;
extern const float alfa3				;
extern const float beta1				;
extern const float beta2				;
extern const float beta3				;
extern const float RATIO_FIRST_BOUND	;
extern const float RATIO_SECOND_BOUND	;
extern const float RATIO_THIRD_BOUND	;

extern const u16 ps_hi_th;
extern const u16 ps_low_th;
extern const u16 ps_os_val[4];
extern const u8 gp_init_reg[20];


extern bool do_gs_process;
extern u8 do_int_process;
extern bool do_als_process;
//u8 i,m,n,r;

extern u16 gs_set_timer_counts, als_set_timer_counts;
extern u16 max_number_polling, debug_polling_counts, encoder_counts;
extern u8 debug_mode;
extern u16 gs_sampling_rate;//Hz
extern u16 als_polling_msec;
extern void Clear_dir(void);

struct gp_params{
	//// OPERATION ////
	u8 op_mode;

	//// GS_PS_mode ////
	u8 gs_ps_mode;
	//// GS Wakeup/Shutdown flag////
	bool gs_enabled;
	//// Clear PROX & Interrupt FLAG registers for ONOFF Func////
	bool clear_int;
	//// GS Operation mode ////
	u8 gs_operation;
	//// Temporal parameters for Direction Judgement ////
	float max_x1, min_x1, max_y1, min_y1;
	float max_x2, min_x2, max_y2, min_y2;
	u8 diff_max_x, diff_max_y;
	bool x_plus, x_minus, y_plus, y_minus;
	u8 gs_state;
	u8 speed_counts;
	//// Thresholds depending on the performance ////
	s16 ignore_diff_th;
	u16 ignore_z_th;
	s8 ratio_th;
	//// Parameters for active offset cancelation ////
	bool active_osc_on;
	u8 allowable_variation;
	u8 acquisition_num;
	u16 max_aoc;
	u16 min_aoc;
	//// Parameters for Zoom function ////
	bool zoomFuncOn;
	u8 to_zoom_th;
	u8 out_zoom_th;
	bool zoomModeNow;
	u8 zoom_mode_th;
	//// Saturation notification //// pre3 sere
	u8 sat_flag[4][5];
	//// Store raw_data ////
	u16 raw_d[4][5];
	//// Store aoc_data ////
	u16 aoc_d[4];
	//// Store aoc_data for Zoom ////
	u16 zoom_aoc[5];
	//// Store zoom_data ////
	u16 zoom_d[5];
	//// Store sub_data ////
	u16 sub_d[5];
	//// For_PULSE_evaluation ///
	float d12[48];//[ <moving_ave_num]
	float d12_ave[48];//[ <moving_ave_num]
	float d12_ave_now;//0
	float d12_now;//0
	float d12_lhpf;//0
	float mult_data;
	u8  hpf_num;//16
	u8	moving_ave_num;//16
	u8  pulse_ave_num;//3
	u16 pulse[5];//[ <pulse_ave_num]
//	u16 pulse;
	u16 pulse_ave;
	u16 pulse_ave_pre;
	u16  count;
	u8  analog_offset_reg;
	u8  pulse_filer_range;
	float min;
	float min_pre;
	bool ao_flag;
	bool pulse_count_start_flag;
	//// GS Direction & Zoom Results ////
	u16 res_gs;
	//// PROXIMITY STATE NEAR:1, FAR:0////
	bool prox_state;
	//// GS_INT			////
	u8 gs_int;

	//// RGB Wakeup/Shutdown flag////
	bool als_enabled;
	//// RGB TYPE ON ////
//	u8 rgb_type;
	//// Store RGBIR_data ////
	u16 als_d[2];
	float ratio;
	//// Low/High lux mode////
	u8 lux_mode;
	//// Lux & CCT ////
	u16 CCT, CCT_prev;
	float lux[3], lux_prev, lux_mean;

	u8 BLINK_state;
	bool BLINK_assert;

};
extern struct gp_params st_gp;

#define GP2AP054A_SCL_High ROM_GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_PIN_4)
#define GP2AP054A_SCL_Low ROM_GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_4, 0)
#define GP2AP054A_SDA_High ROM_GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, GPIO_PIN_5)
#define GP2AP054A_SDA_Low ROM_GPIOPinWrite(GPIO_PORTE_BASE, GPIO_PIN_5, 0)
#define GP2AP054A_SDA_OUT   ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);//SDA
#define GP2AP054A_SDA_IN  ROM_GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_5);

#define iic_delay ROM_SysCtlDelay(20)


//// Parameters of registers for SERENA
	//Register
	#define REG_COM1    		0x00
	#define REG_COM2    		0x01
	#define REG_COM3   			0x02
	#define REG_ALS1   			0x03
	#define REG_ALS2			0x04
	#define REG_PS1				0x05
	#define REG_PS2				0x06
	#define REG_PS3				0x07
	#define REG_PS_LT_LSB	 	0x08
	#define REG_PS_LT_MSB		0x09
	#define REG_PS_HT_LSB		0x0A
	#define REG_PS_HT_MSB		0x0B
	#define REG_OS_D0_LSB		0x0C
	#define REG_OS_D0_MSB		0x0D
	#define REG_OS_D1_LSB		0x0E
	#define REG_OS_D1_MSB		0x0F
	#define REG_OS_D2_LSB		0x10
	#define REG_OS_D2_MSB   	0x11
	#define REG_OS_D3_LSB   	0x12
	#define REG_OS_D3_MSB   	0x13
	#define REG_PRE3_D0_LSB		0x14
	#define REG_PRE3_D0_MSB		0x15
	#define REG_PRE3_D1_LSB	n	0x16
	#define REG_PRE3_D1_MSB		0x17
	#define REG_PRE3_D2_LSB		0x18
	#define REG_PRE3_D2_MSB		0x19
	#define REG_PRE3_D3_LSB		0x1A
	#define REG_PRE3_D3_MSB		0x1B
	#define REG_PRE2_D0_LSB		0x1C
	#define REG_PRE2_D0_MSB		0x1D
	#define REG_PRE2_D1_LSB		0x1E
	#define REG_PRE2_D1_MSB		0x1F
	#define REG_PRE2_D2_LSB		0x20
	#define REG_PRE2_D2_MSB		0x21
	#define REG_PRE2_D3_LSB		0x22
	#define REG_PRE2_D3_MSB		0x23
	#define REG_PRE1_D0_LSB		0x24
	#define REG_PRE1_D0_MSB		0x25
	#define REG_PRE1_D1_LSB		0x26
	#define REG_PRE1_D1_MSB		0x27
	#define REG_PRE1_D2_LSB		0x28
	#define REG_PRE1_D2_MSB		0x29
	#define REG_PRE1_D3_LSB		0x2A
	#define REG_PRE1_D3_MSB		0x2B
	#define REG_D0_LSB			0x2C
	#define REG_D0_MSB			0x2D
	#define REG_D1_LSB			0x2E
	#define REG_D1_MSB			0x2F
	#define REG_D2_LSB			0x30
	#define REG_D2_MSB			0x31
	#define REG_D3_LSB			0x32
	#define REG_D3_MSB			0x33
	#define REG_D4_LSB			0x34
	#define REG_D4_MSB			0x35
	#define REG_D5_LSB			0x36
	#define REG_D5_MSB			0x37
	#define REG_D6_LSB			0x38
	#define REG_D6_MSB			0x39
	#define REG_D7_LSB			0x3A
	#define REG_D7_MSB			0x3B
	#define REG_D8_LSB			0x3C
	#define REG_D8_MSB			0x3D
	#define REG_REV_CODE		0x3E
	#define REG_REVF			0x3F
	#define REG_TEST1   		0x40
	#define REG_TEST2			0x41
	#define REG_TEST3			0x42


	// COMMAND1
	#define COM1_WAKEUP			0x80
	#define COM1_SD				0x00
	#define COM1_ALS_GS			0x00
	#define COM1_ALS			0x10
	#define COM1_GS				0x20
	#define COM1_IRBEAM			0x30
//	#define COM1_RGB_GS_LEDOFF	0x40


	// COMMAND2
	#define COM2_NO_INT_CLEAR	0x0F
	#define COM2_INT_CLEAR		0x00
	#define COM2_GS_INT_CLEAR	0x0E
	#define COM2_PS_INT_CLEAR	0x03
	#define COM2_ALS_INT_CLEAR	0x0D


	// COMMAND3
	#define COM3_PS_INT_D2		0x80
	#define COM3_PS_INT_D4		0x00
	#define COM3_INT_PROX		0x00
	#define COM3_INT_PS			0x10
	#define COM3_INT_ALS		0x20
	#define COM3_INT_GS			0x40
	#define COM3_INT_PS_LEVEL	0x00
	#define COM3_INT_PS_PULSE	0x02
	#define COM3_INT_ALS_LEVEL	0x00
	#define COM3_INT_ALS_PULSE	0x04
	#define COM3_INT_GS_LEVEL	0x00
	#define COM3_INT_GS_PULSE	0x08
	#define COM3_REG_RST		0x01


// ALS1
	#define ALS1_RANGE_H_HI		0x80
	#define ALS1_RANGE_H_LO		0x00
	#define ALS1_PD_H_HI		0x40
	#define ALS1_PD_H_LO		0x00
	#define ALS1_RES18			0x00
	#define ALS1_RES16			0x08
	#define ALS1_RES14			0x10
	#define ALS1_RES12			0x18
	#define ALS1_RANGEX1		0x00
	#define ALS1_RANGEX2		0x01
	#define ALS1_RANGEX4		0x02
	#define ALS1_RANGEX8		0x03
	#define ALS1_RANGEX16		0x04
	#define ALS1_RANGEX32		0x05
	#define ALS1_RANGEX64		0x06
	#define ALS1_RANGEX128		0x07


	// ALS2
	#define ALS2_RGB_MODE		0x80
	#define ALS2_ALS_INTVAL0	0x00
	#define ALS2_ALS_INTVAL1P56	0x01
	#define ALS2_ALS_INTVAL6P25	0x02
	#define ALS2_ALS_INTVAL25	0x03
	#define ALS2_ALS_INTVAL50	0x04
	#define ALS2_ALS_INTVAL100	0x05
	#define ALS2_ALS_INTVAL200	0x06
	#define ALS2_ALS_INTVAL400	0x07


	// PS1
	#define PS1_PRST1			0x00
	#define PS1_PRST2			0x20
	#define PS1_PRST3			0x40
	#define PS1_PRST4			0x60
	#define PS1_PRST5			0x80
	#define PS1_PRST6			0xA0
	#define PS1_PRST7			0xC0
	#define PS1_PRST8			0xE0
	#define PS1_RES14			0x00
	#define PS1_RES12			0x08
	#define PS1_RES10			0x10
	#define PS1_RES8			0x18
	#define PS1_RANGEX1			0x00
	#define PS1_RANGEX2			0x01
	#define PS1_RANGEX4	 		0x02
	#define PS1_RANGEX8			0x03
	#define PS1_RANGEX16		0x04
	#define PS1_RANGEX32		0x05
	#define PS1_RANGEX64		0x06
	#define PS1_RANGEX128		0x07


	// PS2
	#define PS2_IS2				0x00
	#define PS2_IS4				0x20
	#define PS2_IS8				0x40
	#define PS2_IS16			0x60
	#define PS2_IS32			0x80
	#define PS2_IS64			0xA0
	#define PS2_IS128			0xC0
	#define PS2_IS256			0xE0
	#define PS2_SUM4			0x00
	#define PS2_SUM8			0x04
	#define PS2_SUM12			0x08
	#define PS2_SUM16			0x0C
	#define PS2_SUM20			0x10
	#define PS2_SUM24			0x14
	#define PS2_SUM28			0x18
	#define PS2_SUM32			0x1C
	#define PS2_PULSE1			0x00
	#define PS2_PULSE2			0x01
	#define PS2_PULSE3			0x02
	#define PS2_PULSE4			0x03


	//PS3
	#define PS3_PATTERN_CYGNUS	0x00
	#define PS3_PATTERN_ALBIREO	0x80
	#define PS3_SEL_LO_OFF		0x00
	#define PS3_SEL_LO_ON		0x40
	#define PS3_GS_INT1			0x00
	#define PS3_GS_INT2			0x10
	#define PS3_GS_INT3			0x20
	#define PS3_GS_INT4			0x30
	#define PS3_SEL_SAT_ONOFF	0x00
	#define PS3_SEL_SAT_OFF		0x08
	#define PS3_GS_INTVAL0		0x00
	#define PS3_GS_INTVAL1P56	0x01
	#define PS3_GS_INTVAL6P25	0x02
	#define PS3_GS_INTVAL25		0x03
	#define PS3_GS_INTVAL50		0x04
	#define PS3_GS_INTVAL100	0x05
	#define PS3_GS_INTVAL200	0x06
	#define PS3_GS_INTVAL400	0x07


	//TEST1
	#define TEST1_VFC_SW_ON		0x04

	//TEST3
	#define TEST3_ONCE_MODE		0x40



#define PORTF_OPENPLUSE     (GPIO_PIN_0)//开脉冲 这个宽度是  只在AUTO有作用把
#define PORTF_STOPPLUSE     (GPIO_PIN_1)//停脉冲 这个宽度不是一个定值和手在感应器的时间有关系  只在AUTO有作用把
#define PORTF_CLOSEPLUSE    (GPIO_PIN_2)//闭脉冲 这个宽度是  只在AUTO有作用把
#define OPENPLUSE_LEVELLOW  ROM_GPIOPinWrite(GPIO_PORTF_BASE, PORTF_OPENPLUSE, 0);
//只在OPENPLUSE那个脚写0
#define OPENPLUSE_LEVELHIGH  ROM_GPIOPinWrite(GPIO_PORTF_BASE, PORTF_OPENPLUSE, PORTF_OPENPLUSE);
#define STOPPLUSE_LEVELLOW  ROM_GPIOPinWrite(GPIO_PORTF_BASE, PORTF_STOPPLUSE, 0);
#define STOPPLUSE_LEVELHIGH ROM_GPIOPinWrite(GPIO_PORTF_BASE, PORTF_STOPPLUSE, PORTF_STOPPLUSE);
#define CLOSEPLUSE_LEVELLOW  ROM_GPIOPinWrite(GPIO_PORTF_BASE, PORTF_CLOSEPLUSE, 0);
#define CLOSEPLUSE_LEVELHIGH ROM_GPIOPinWrite(GPIO_PORTF_BASE, PORTF_CLOSEPLUSE, PORTF_CLOSEPLUSE);

void GP2AP054A_Init(void);
void GP2A054A_calculation(void);
void GP2AP054A_Init_cyw(void);
void GP2A054A_calculation_cyw(void);
void GP2AP054A_Init_cyw_debug(void);
void init054A(void);
void clear_gs_status(struct gp_params *p_gp);
void guest_reinit(void);
#endif /* GP2AP054A_H_ */
