/*
 * GP2AP054A.C
 *
 *  Created on: 2015�?��7��
 *      Author: x220
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
//#include <stdio.h>
//#include <stdlibm.h>// malloc
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"//����
#include "Application/backlight_control.h"
#include "Middleware/bolymindisplay.h"
#include "lcd_cyw.h"
#include "ram_cyw.h"
#include "GP2AP054A_cyw.h"
#include "inc/hw_gpio.h"//cyw add 20150902
#include "inc/hw_types.h"//cyw add 20150902
uint8_t  LCD_DISP_GUESTURE=0;
uint8_t setting_flag=1;//���������ò��?
uint8_t error_guesture_cyw=0;

void Start(void)
 {
	  GP2AP054A_SDA_High;
	  GP2AP054A_SCL_High;
      iic_delay;
      GP2AP054A_SDA_Low;
      iic_delay;
 }

void Stop(void)
 {
	GP2AP054A_SDA_Low;
	GP2AP054A_SCL_High;
    iic_delay;
    GP2AP054A_SDA_High;
    iic_delay;
}

void Reack(void)
 {
	unsigned char i=0;
    GP2AP054A_SDA_IN;
    iic_delay;
    GP2AP054A_SCL_High; //׼���?�SDA Delay5us();
    while((ROM_GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_5))&&(i<100))
    {
      i++;//SDA=0ΪӦ���ź�,SDA=1Ϊ��Ӧ�?/P>
      if(i>=100)
      {
    	  error_guesture_cyw = 1;
      }
    }
    GP2AP054A_SDA_OUT;
    iic_delay;
    GP2AP054A_SCL_Low; //׼����һ�仯�?�</P>
 }

void WriteByte(unsigned char ucByte)
 {
	unsigned char i;
	GP2AP054A_SCL_Low;
    for(i=0;i<8;i++)
    {
      if(ucByte&0x80)//��д�?�λ</P>
      {
    	  GP2AP054A_SDA_High;
      }
      else
      {
    	  GP2AP054A_SDA_Low;
      }
      GP2AP054A_SCL_High;
      iic_delay;
      GP2AP054A_SCL_Low;
      ucByte<<=1;
    }
    GP2AP054A_SDA_High; //�ͷ��?���</P>
 }

unsigned char ReadByte(void)
 {
	unsigned char i,ucByte=0;

	GP2AP054A_SDA_IN;
    iic_delay;

    GP2AP054A_SCL_Low;
    for(i=0;i<8;i++)
    {
        ucByte<<=1;
       if(ROM_GPIOPinRead(GPIO_PORTE_BASE, GPIO_PIN_5))
          ucByte++;
       GP2AP054A_SCL_High;
       iic_delay;
       GP2AP054A_SCL_Low;
    }

    GP2AP054A_SDA_OUT;
    iic_delay;

    return ucByte;
 }

void Write054(unsigned char ucAddr,unsigned char ucData)
 {
       iic_delay;
      Start();
      WriteByte(0x72);//���?�ַ
      Reack();
      WriteByte(ucAddr);
      Reack();
      WriteByte(ucData);
      Reack();
      Stop();
}

unsigned char Read054(unsigned char ucAddr)
 {
	unsigned char ucData;
       iic_delay;
      Start();
      WriteByte(0x72); //д���?�ַ</P>
      Reack();
      WriteByte(ucAddr); //д�ֽڵ�ַ</P>
      Reack();
      Start();
      WriteByte(0x73); //д���?�ַ,����Ϊ1��澶�</P>
      Reack();
      ucData=ReadByte(); //д�ֽڵ�ַ</P>
      Stop();
      return ucData; //���?�</P>
}

// *******************************************************************
// * Name	: clearGSparams(Input)
// * Input	: struct gp_params pointer
// * Output	:
// * Note	: clear temporal variables for direction judgement
// *******************************************************************
void clearGSparams(struct gp_params *p_gp) {
	p_gp->x_plus		= 0;
	p_gp->x_minus		= 0;
	p_gp->y_plus		= 0;
	p_gp->y_minus		= 0;
	p_gp->max_x1		= 0;
	p_gp->min_x1		= 0;
	p_gp->max_y1		= 0;
	p_gp->min_y1		= 0;
	p_gp->max_x2		= 0;
	p_gp->min_x2		= 0;
	p_gp->max_y2		= 0;
	p_gp->min_y2		= 0;
	p_gp->diff_max_x	= 0;
	p_gp->diff_max_y	= 0;
	p_gp->speed_counts	= 0;
	p_gp->gs_state		= STATE_ONE;

	return;
}
// *******************************************************************
// * Name	: initGSparams(Input)
// * Input	: struct gp_params pointer
// * Output	:
// * Note	: Initialize variables for GS
// *******************************************************************
void initGSparams(struct gp_params *p_gp){

	unsigned char i,m,n;

	clearGSparams(p_gp);
//	p_gp->gs_operation = GS_A_MODE;		//gs_polling_mode
	p_gp->gs_operation = GS_B_MODE;		//gs_interupt_mode
	p_gp->clear_int		 		= 1;
	p_gp->ignore_z_th 			= 20;
	p_gp->ignore_diff_th 		= 10;
	p_gp->ratio_th 				= 0.1;
	p_gp->active_osc_on 		= 1;
	p_gp->allowable_variation 	= 30;
	p_gp->acquisition_num		= 10;
	p_gp->max_aoc				= 8000;
	p_gp->min_aoc				= 0;
	p_gp->zoomFuncOn 			= 1;//���ٽ???
	p_gp->to_zoom_th			= 100;
	p_gp->out_zoom_th			= 30;
	p_gp->zoomModeNow 			= 0;
	p_gp->zoom_mode_th			= 4;
	p_gp->res_gs				= 0;
	p_gp->prox_state			= 0;
 	p_gp->gs_int				= 0;


	for(i=0; i<4; i++){
		p_gp->sat_flag[0][i]	= 0;
		p_gp->raw_d[0][i]		= 0;
		p_gp->aoc_d[i]			= p_gp->max_aoc/4;
		p_gp->sub_d[i]			= 0;
		p_gp->zoom_aoc[i]		= 0;
		p_gp->zoom_d[i]			= 0;
	}
	p_gp->sat_flag[0][4]		= 0;
	p_gp->raw_d[0][4]			= 0;
	p_gp->sub_d[4]				= 0;

	for(m=0; m<4; m++){
		for(n=0; n<4; n++){
			p_gp->sat_flag[m][n]		= 0;
			p_gp->raw_d[m][n]			= 0;
		}
	}
}
// *******************************************************************
// * Name	: initPULSEparams(Input)
// * Input	: struct gp_params pointer
// * Output	:
// * Note	: Initialize variables for GS
// *******************************************************************
void initPULSEparams(struct gp_params *p_gp){

	unsigned char i;
	clearGSparams(p_gp);
//	p_gp->gs_operation = GS_A_MODE;

	p_gp->d12_ave_now			= 0;
	p_gp->d12_now				= 0;
	p_gp->pulse_ave_num			= 5;
 	p_gp->pulse_ave				= 0;
 	p_gp->pulse_ave_pre			= 0;
 	p_gp->d12_lhpf				= 0;
 	p_gp->moving_ave_num		= 32;
 	p_gp->hpf_num				= 32;
 	p_gp->mult_data				= 1;
 	p_gp->count					= 85;
 	p_gp->pulse_filer_range		= SLOW_PULSE_RANGE;

 	for(i=0; i<32; i++){
 		p_gp->d12[i] = 0;
 		p_gp->d12_ave[i] = 0;
	}

 	p_gp->pulse[0] = 85;
 	p_gp->pulse[1] = 85;
 	p_gp->pulse[2] = 85;
 	p_gp->pulse[3] = 85;
 	p_gp->pulse[4] = 85;
 	p_gp->min = 0;
	p_gp->min_pre = 0;
	p_gp->ao_flag = 0;
	p_gp->pulse_count_start_flag = 0;
	for(i=0; i<4; i++){
		p_gp->raw_d[0][i]		= 0;
	};
}
// *******************************************************************
// * Name	: initBLINKparams(Input)
// * Input	: struct gp_params pointer
// * Output	:
// * Note	: Initialize variables for BLINK
// *******************************************************************
void initBLINKparams(struct gp_params *p_gp){

	unsigned char i;

	p_gp->d12_ave_now			= 0;
	p_gp->d12_now				= 0;
 	p_gp->d12_lhpf				= 0;
 	p_gp->moving_ave_num		= 48;
 	p_gp->hpf_num				= 48;
 	p_gp->BLINK_state			= STATE_ONE;
	p_gp->BLINK_assert			= 0;

 	for(i=0; i<p_gp->moving_ave_num; i++){
 		p_gp->d12[i] = 0;
 		p_gp->d12_ave[i] = 0;
	}

// 	p_gp->pulse[0] = 85;
//	p_gp->pulse[1] = 85;
//	p_gp->pulse[2] = 85;
//	p_gp->pulse[3] = 85;
//	p_gp->pulse[4] = 85;
//	p_gp->min = 0;
//	p_gp->min_pre = 0;
//	p_gp->ao_flag = 0;
//	p_gp->pulse_count_start_flag = 0;
//	for(i=0; i<4; i++){
//		p_gp->raw_d[0][i]		= 0;
//	};
}



// *******************************************************************
// * Name	: initGSparams(Input)
// * Input	: struct gp_params pointer
// * Output	:
// * Note	: Initialize variables for GS
// *******************************************************************
void initALSparams(struct gp_params *p_gp){

	unsigned char i;
	p_gp->lux_mode	 		= MID_LUX;
	p_gp->CCT				= 0;
	p_gp->lux_mean			= 0;
	p_gp->CCT_prev			= 0;
	p_gp->lux_prev			= 0;
	p_gp->lux[0]			= 0;
	p_gp->lux[1]			= 0;
	p_gp->lux[2]			= 60000;

	for(i=0; i<2; i++){
		p_gp->als_d[i] = 0;
	}

}



// *******************************************************************
// * Name	: initGSparams(Input)
// * Input	: struct gp_params pointer
// * Output	:
// * Note	: Initialize variables for GS
// *******************************************************************
void IniALLparams(struct gp_params *p_gp){
	initGSparams(p_gp);
	initALSparams(p_gp);
	initPULSEparams(p_gp);
	initBLINKparams(p_gp);
}

// *******************************************************************
// * Name	: setGS_PSmode(Input1, Input2)
// * Input	: Input1 = struct gs_params pointer
//			: Input2 = operation_mode
// * Output	:
// * Note	: GS_PS mode change according to operation_mode
// *******************************************************************
void setGS_PSmode(struct gp_params *p_gp){
	if( (p_gp->op_mode == GS_PS_ALS_MODE) | (p_gp->op_mode == PS_GS_MODE)){
		p_gp->gs_ps_mode = PS_GS_MODE;
	}else if((p_gp->op_mode == PS_ONLY_MODE) | (p_gp->op_mode == PS_ALS_MODE)){
		p_gp->gs_ps_mode = PS_ONLY_MODE;
	}else{
		p_gp->gs_ps_mode = GS_ONLY_MODE;
	}
}


void getModeRegSet(struct gp_params *p_gp, u8 *warray){

	//memcpy(warray, gp_init_reg, sizeof(gp_init_reg));
    u8 tp_i;

    for(tp_i = 0 ;tp_i<20;tp_i++)
    {
        warray[tp_i] = gp_init_reg[tp_i];
    }

	if( (p_gp->op_mode == PS_ONLY_MODE) | (p_gp->op_mode == PS_ALS_MODE) ) {
		warray[2] = warray[2]&0x80 | COM3_INT_PROX;
		warray[7] = warray[7]&0xC8 | PS3_GS_INT1 | PS3_GS_INTVAL25;
	}else if(p_gp->op_mode == PULSE_MODE){
		warray[2] = 0x00;
		warray[3] = 0x00;
		warray[4] = 0x00;
		warray[5] = (PS1_RANGEX2 | PS1_RES12);//X16
		warray[6] = (PS2_IS32|PS2_SUM16);
		warray[7] = (PS3_PATTERN_ALBIREO|PS3_SEL_LO_OFF|PS3_GS_INT4|PS3_SEL_SAT_ONOFF|PS3_GS_INTVAL6P25);
		warray[12] = 0x00;
		warray[13] = 0x00;
		warray[14] = 0x00;
		warray[15] = 0x00;
	}else if(p_gp->op_mode == BLINK_MODE){
		warray[2] = 0x00;
		warray[3] = 0x00;
		warray[4] = 0x00;
		warray[5] = (PS1_RANGEX2 | PS1_RES12);//X16
		warray[6] = (PS2_IS16|PS2_SUM16);
		warray[7] = (PS3_PATTERN_ALBIREO|PS3_SEL_LO_OFF|PS3_GS_INT4|PS3_SEL_SAT_ONOFF|PS3_GS_INTVAL6P25);
		warray[12] = 0x00;
		warray[13] = 0x00;
		warray[14] = 0x00;
		warray[15] = 0x00;
	}else{
		if(p_gp->gs_operation == GS_A_MODE){				//gs_polling_mode
			warray[2] = warray[2]&0x80 | COM3_INT_PROX;
			warray[7] = warray[7]&0xC8 | PS3_GS_INT1 | PS3_GS_INTVAL6P25;
		}else if(p_gp->gs_operation == GS_B_MODE){			//gs_interupt_mode
			warray[2] = warray[2]&0x80 | COM3_INT_GS |COM3_INT_GS_PULSE;
			warray[7] = warray[7]&0xC8 | PS3_GS_INT4 | PS3_GS_INTVAL6P25;
		}
	}
	return ;
}

//��ʼӲ�?IC�ӿ���
void seq_i2c_write(u8 slave, u8 word, u8 *wdata, u8 num)
{
	unsigned char i;
	iic_delay;
	Start();
    WriteByte(slave);	Reack();
    WriteByte(word);	Reack();

	for(i=0;i<num;i++){
		 WriteByte(*(wdata+i));	Reack();
	}
	Stop();

	return;
}

// *******************************************************************
// * Name	: setreg(Input)
// * Input	: struct gs_params pointer
// *		: Const:gs_init_reg[], ps_init_reg[], gsps_init_reg[]
// * Output	:
// * Note	: set up registers for GS/PS mode(without wakeup).
// *******************************************************************
void initRegister(struct gp_params *p_gp)
{
	unsigned char warray[20];
	//unsigned char warray2[1];
	//unsigned char wsize =1;

	getModeRegSet(p_gp, warray);//��ֵ�ӽṹ��COPY��warray�??
	seq_i2c_write(SLAVE_ADDR, REG_COM1, warray, WRITE_NUM);//�ӻ�dizhi  �Ĵ���dizhi ,value,���?

	return ;
}

// *******************************************************************
// * Name	: gs_onoff(Input1, Input2)
// * Input	: GS -> ON	= Input1 = 1
//			:	 -> OFF	= Input1 = 0
//			: struct gs_params.clear_int = 1 -> Clear Interrupt flag
//			: struct gs_params.clear_int = 0 -> Not clear Interrupt flag
// * Output	: Update struct gs_params.gs_enabled
// * Note	: GS ON/OFF funciton(with INT_CLEAR/NO_CLEAR)
// *******************************************************************
void sensor_onoff(bool onoff, struct gp_params *p_gp){

	u8 wdata1[2];

	if(p_gp->op_mode==ALS_ONLY_MODE){
		wdata1[0] = COM1_ALS;
	}else if((p_gp->op_mode==GS_ONLY_MODE) | (p_gp->op_mode==PS_ONLY_MODE) |
			 (p_gp->op_mode==PS_GS_MODE) | (p_gp->op_mode==PULSE_MODE) ){
		wdata1[0] = COM1_GS;
	}else{
		wdata1[0] = COM1_ALS_GS;
	}

	if(onoff){
		if(p_gp->clear_int){
			wdata1[0] |= COM1_WAKEUP;
			wdata1[1] = COM2_INT_CLEAR;
		}else{
			wdata1[0] |= COM1_WAKEUP;
			wdata1[1] = COM2_NO_INT_CLEAR;
		}
	}else{
		if(p_gp->clear_int){
			wdata1[0] |= COM1_SD;
			wdata1[1] = COM2_INT_CLEAR;
		}else{
			wdata1[0] |= COM1_SD;
			wdata1[1] = COM2_NO_INT_CLEAR;
		}
	}
	seq_i2c_write(SLAVE_ADDR, REG_COM1, wdata1, 2);

	if(p_gp->op_mode==ALS_ONLY_MODE){
		p_gp->gs_enabled = 0;
	}else{
		p_gp->gs_enabled = onoff;
	}
	if((p_gp->op_mode==GS_ONLY_MODE) | (p_gp->op_mode==PS_ONLY_MODE)){
		p_gp->als_enabled = 0;
	}else{
		p_gp->als_enabled = onoff;
	}

	return;
}

// *******************************************************************
// * Name	: changeOPmode(Intput1, Input2, Input3)
// * Input	: Input1 = struct gs_params pointer
//			: Input2 = struct als_params pointer
//			: Input3 = operation_mode:PS_ONLY, GS_ONLY, PS_GS, RGB
// * Output	:
// * Note	: change operation mode
// *******************************************************************
void changeOPmode(struct gp_params *p_gp, bool init_reg_on)
{
	bool int_pin_en = 0;
	bool als_timer_en = 0;
	bool gs_timer_en = 0;
	u8 wdata2[1];

	do_gs_process = 0;
	do_int_process = 0;
	do_als_process = 0;
	//disable_interrupts(INT_EXT);
	//disable_interrupts(INT_TIMER0);
	//disable_interrupts(INT_TIMER1);
	//set_timer1(gs_set_timer_counts);
	//set_timer0(als_set_timer_counts);
	setGS_PSmode(p_gp);
	p_gp->clear_int=1;

	if(init_reg_on){//=1
		initRegister(p_gp);
		if((p_gp->op_mode != PULSE_MODE) && (p_gp->op_mode != BLINK_MODE)){
			initALSparams(p_gp);
			wdata2[0] = 0x00;//analog_offset
			p_gp->analog_offset_reg = 0x00;
			seq_i2c_write(SLAVE_ADDR, 0x41, wdata2, 1);
			als_polling_msec = 200;
			als_set_timer_counts = 65536 - 20000000/4/256*als_polling_msec/1000;
			//set_timer0(als_set_timer_counts);
		}else{
			wdata2[0] = 0x36;//analog_offset     //1:09 2:12 3:1B 4:24 5:2D 6:36 7:3F
			p_gp->analog_offset_reg = 0x36;
			initPULSEparams(p_gp);
			seq_i2c_write(SLAVE_ADDR, 0x41, wdata2, 1);
			als_polling_msec = 10;
			als_set_timer_counts = 65536 - 20000000/4/256*als_polling_msec/1000;
			//set_timer0(als_set_timer_counts);
		}

	}

	sensor_onoff(1, p_gp);
	switch(p_gp->op_mode){
		case 1://PS_ONLY_MODE:
			als_timer_en = 0;
			int_pin_en = 1;
			gs_timer_en = 1;
		break;
		case 2://GS_ONLY_MODE:
			als_timer_en = 0;
			if(p_gp->gs_operation == GS_A_MODE){
				int_pin_en = 0;
				gs_timer_en = 1;
			}else{
				int_pin_en = 1;
				gs_timer_en = 0;
			}
		break;
		case 3://PS_GS_MODE:
			als_timer_en = 0;
			int_pin_en = 1;
			if(p_gp->gs_operation == GS_A_MODE){
				gs_timer_en = 1;
			}else{
				gs_timer_en = 0;
			}
		break;
		case 4://ALS_ONLY_MODE:
			als_timer_en = 1;
			int_pin_en = 0;
			gs_timer_en = 0;
		break;
		case 5://PS_ALS_MODE:
			als_timer_en = 1;
			int_pin_en = 1;
			gs_timer_en = 0;
		break;
		case 6://GS_ALS_MODE:
			als_timer_en = 1;
			if(p_gp->gs_operation == GS_A_MODE){
				int_pin_en = 0;
				gs_timer_en = 1;
			}else{
				int_pin_en = 1;
				gs_timer_en = 0;
			}
		break;
		case 7://GS_PS_ALS_MODE:
			als_timer_en = 1;
			int_pin_en = 1;
			if(p_gp->gs_operation == GS_A_MODE){
				gs_timer_en = 1;
			}else{
				gs_timer_en = 0;
			}
		break;

		case 9://PULSE_MODE:
			als_timer_en = 1;
			int_pin_en = 0;
			gs_timer_en = 0;
		break;

		case 8://BLINK_MODE:
			als_timer_en = 1;
			int_pin_en = 0;
			gs_timer_en = 0;
		break;

		default:
		break;
	}

	if(als_timer_en){
		//enable_interrupts(INT_TIMER0);
	}
	if(int_pin_en){
		//enable_interrupts(INT_EXT_H2L);
	}
	if(gs_timer_en){
		//enable_interrupts(INT_TIMER1);
	}

	return;
}

// *******************************************************************
   // * Name	: changeStandalone(Intput1, Input2, Input3)
   // * Input	: Input1 = pin setting of EV board
   //			: Input2 = struct gs_params pointer
   //			: Input3 = struct als_params pointer
   // * Output	:
   // * Note	: change sensor modes according to the state of EV board switch
   // *******************************************************************
   void changeStandalone(u8 pin, struct gp_params *p_gp){

   				p_gp->op_mode = GS_ONLY_MODE;
   				changeOPmode(p_gp, INIT_REG_ON);


   }


   ///////////////// Sequential I2C READ ////////////////////
   void seq_i2c_read(u8 slave, u8 word, u8 *rdata, u8 num)
   {


   	u8 i;
   	Start();
   	WriteByte(slave);	Reack();
   	WriteByte(word);	Reack();

   	Start();
   	WriteByte(slave|0x01);	Reack();
   	if(num>1){
   		for(i=0;i<num-1;i++){
   			*(rdata+i) = ReadByte();
   		}
   	}
   	*(rdata+num-1) = ReadByte();
   	Stop();

   	return;
   }


   // *******************************************************************
   // * Name	: getGSdata(Output)
   // * Input	:
   // * Output	: rdata[5] 16bits integer
   // * Note	: read out data through I2C
   // *******************************************************************
   void getGSdata(struct gp_params *p_gp){
   	u8 rbuf[32]={0,};
   	u8 k,i;


   	for(k=0;k<4;k++){
   	p_gp->raw_d[k][4] = 0;
   	p_gp->sat_flag[k][4] = 0;
   	}

   //	p_gp->raw_d[now][0]  = warray_r[0]+warray_r[1]*256;
   	//p_gp->raw_d[now][1]  = warray_r[2]+warray_r[3]*256;
   //	p_gp->raw_d[now][2]  = warray_r[4]+warray_r[5]*256;
   //	p_gp->raw_d[now][3]  = warray_r[6]+warray_r[7]*256;
   	for(i=0;i<8;i++)
   	{
   	 rbuf[i] = warray_r[i];
   	}


   for(i=0;i<4;i++){
   			p_gp->raw_d[now][i]		= (u16)rbuf[2*i] + (((u16)(rbuf[2*i+1] & 0x7F))<<8);
   			p_gp->raw_d[now][4]		+= p_gp->raw_d[now][i];
   			p_gp->sat_flag[now][i]	= rbuf[2*i+1]>>7;
   			p_gp->sat_flag[now][4]	|= p_gp->sat_flag[now][i];
   		}


   	return;
   }

   // *******************************************************************
   // * Name	: getActiveOffset(Input1, Output, Input2)
   // * Input	: Input1 = p_gp->raw_d[5], Input2 = struct gp_params pointer
   // * Output	: p_gp->aoc_d[4]
   // * Note	: to get offset value from acquisition times of raw data.
   // *******************************************************************
   void getOffsetData(struct gp_params *p_gp, u8 j){
   	static u8 num_counts=0;
   	static u16 max_d[4]={0};
   	static u16 min_d[4]={0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF};
   	static u32 avg_d[4]={0};
     u8 i;

   	if( p_gp->sat_flag[j][4]){
   		num_counts = 0;
   		return;
   	}
   	if(!((p_gp->raw_d[j][4] <= p_gp->max_aoc) && (p_gp->raw_d[j][4] >= p_gp->min_aoc))){
   		num_counts = 0;
   		return;
   	}
   	//// The last measurement of offset
   	if(num_counts == p_gp->acquisition_num -1){
   		for(i=0;i<4;i++){
   			avg_d[i] = avg_d[i] + p_gp->raw_d[j][i];
   			if(p_gp->raw_d[j][i] > max_d[i]){
   				max_d[i] = p_gp->raw_d[j][i];
   			}
   			if(p_gp->raw_d[j][i] < min_d[i]){
   				min_d[i] = p_gp->raw_d[j][i];
   			}
   		}

   		for(i=0;i<4;i++){
   			avg_d[i] = avg_d[i] - max_d[i] - min_d[i];
   			avg_d[i] = avg_d[i]/(p_gp->acquisition_num-2);

   			//// If data variation is large, offset data doesn't update and uses the former data.
   			if( (max_d[i] < avg_d[i] + p_gp->allowable_variation) &&
   				(min_d[i] + p_gp->allowable_variation > avg_d[i])){
   					if(p_gp->op_mode != PULSE_MODE){
   						p_gp->aoc_d[i] = avg_d[i] + p_gp->allowable_variation;
   					}else{
   						p_gp->aoc_d[i] = avg_d[i];
   					}

   				}
   		}
   		num_counts = 0;
   	//// The first measurement of offset
   	}else if(num_counts == 0){
   		for(i=0;i<4;i++){
   			avg_d[i] = p_gp->raw_d[j][i];
   			max_d[i] = p_gp->raw_d[j][i];
   			min_d[i] = p_gp->raw_d[j][i];
   		}
   		num_counts++;
   	}else{
   		for(i=0;i<4;i++){
   			avg_d[i] = avg_d[i] + p_gp->raw_d[j][i];;
   			if(p_gp->raw_d[j][i] > max_d[i]){
   				max_d[i] = p_gp->raw_d[j][i];;
   			}
   			if(p_gp->raw_d[j][i] < min_d[i]){
   				min_d[i] = p_gp->raw_d[j][i];;
   			}
   		}
   		num_counts++;
   	}

   	return;
   }


   // *******************************************************************
   // * Name	: getActiveOffset(Input1, Output, Input2)
   // * Input	: Input1 = p_gp->raw_d[5], Input2 = struct gp_params pointer
   // * Output	: p_gp->aoc_d[4]
   // * Note	: to get offset value from acquisition times of raw data.
   // *******************************************************************
   void getActiveOffset(struct gp_params *p_gp, u8 j){

   	u8 i;
	if((p_gp->raw_d[j][4] <= p_gp->max_aoc) && (p_gp->raw_d[j][4] >= p_gp->min_aoc)){
   		getOffsetData(p_gp, j);
   	}
   	for(i=0;i<4;i++){
   				if(p_gp->raw_d[j][i] > p_gp->aoc_d[i]){
   					p_gp->sub_d[i] = p_gp->raw_d[j][i] - p_gp->aoc_d[i];
   				}else{
   					p_gp->sub_d[i] = 0;
   				}
   	}
   	return;
   }


   // *******************************************************************
   // * Name	: getDirection(Input1, Input2)
   // * Input	: Input1 = sub_os_data[4], Input2 = struct gp_params pointer
   // * Output	: Output return value is unsigned 16bits integer = 0x0ZSD
   //            Z(4bits from the higer 5th to 8th bit) ; Zoom results
   //            S(4bits from the lower 5th to 8th bit) ; Speed results
   //            D(lower 4bits) ; Direction results
   // * Note	: get the result of Direction or Zoom
   // *******************************************************************
   void getDirection(struct gp_params *p_gp){
   	s16 data_x = 0, data_y = 0;
   	float temp_ratio = 0;
   	float ratio_x = 0, ratio_y = 0;


#ifdef UART_DEBUG
   	u16 Tp_data=0;
    u8 Tp_i=0;

   	if((p_gp->sub_d[0]+p_gp->sub_d[1]+p_gp->sub_d[2]+p_gp->sub_d[3])!=0)
   	{
   	for(Tp_i=0;Tp_i<4;Tp_i++)
   	{
   	Tp_data =p_gp->sub_d[Tp_i];

    UARTCharPut(UART1_BASE, 48+(Tp_data/10000));
   	while(UARTBusy(UART1_BASE));

   	  UARTCharPut(UART1_BASE, 48+(Tp_data/1000)%10);

    while(UARTBusy(UART1_BASE));
    UARTCharPut(UART1_BASE, 48+(Tp_data/100)%10);

       while(UARTBusy(UART1_BASE));
       UARTCharPut(UART1_BASE, 48+(Tp_data/10)%10);

          while(UARTBusy(UART1_BASE));
          UARTCharPut(UART1_BASE, 48+Tp_data%10);
                  while(UARTBusy(UART1_BASE));
                  UARTCharPut(UART1_BASE, 32);
   	}


   	while(UARTBusy(UART1_BASE));
    UARTCharPut(UART1_BASE, 9);
    while(UARTBusy(UART1_BASE));

   	}
#endif

   	p_gp->res_gs = 0;
   	//// Diff calculation ////
   	data_y = p_gp->sub_d[2] + p_gp->sub_d[3] - p_gp->sub_d[0] - p_gp->sub_d[1];
   	if( ((data_y > -(p_gp->ignore_diff_th)) && (data_y < (p_gp->ignore_diff_th))) ||
   		(p_gp->sub_d[0] == OVER_FLOW_DATA) || (p_gp->sub_d[1] == OVER_FLOW_DATA) ||
   		(p_gp->sub_d[2] == OVER_FLOW_DATA) || (p_gp->sub_d[3] == OVER_FLOW_DATA) )
   	{
   		data_y = 0;
   	}
   	data_x = p_gp->sub_d[1] + p_gp->sub_d[2] - p_gp->sub_d[0] - p_gp->sub_d[3];
   	if( ((data_x > -(p_gp->ignore_diff_th)) && (data_x < (p_gp->ignore_diff_th))) ||
   		(p_gp->sub_d[0] == OVER_FLOW_DATA) || (p_gp->sub_d[1] == OVER_FLOW_DATA) ||
   		(p_gp->sub_d[2] == OVER_FLOW_DATA) || (p_gp->sub_d[3] == OVER_FLOW_DATA) )
   	{
   		data_x = 0;
   	}

   	//// Ratio calculation ////
   	if(p_gp->sub_d[4] < p_gp->ignore_z_th){
   		temp_ratio = 0;
   	}else{
   		temp_ratio = ((float)data_y / (float)p_gp->sub_d[4]);
   	}
   	ratio_y = temp_ratio;
   	if(p_gp->sub_d[4] < p_gp->ignore_z_th){
   		temp_ratio = 0;
   	}else{
   		temp_ratio = ((float)data_x / (float)p_gp->sub_d[4]);
   	}
   	ratio_x = temp_ratio;

   	//// Debug output ////
   //	if(debug_mode == 4){
   	//	printf("rx=%4.2f ry=%4.2f,st=%u,", ratio_x, ratio_y, p_gp->gs_state);
   //	}

   	//// Judgement FSM start ////


   	switch (p_gp->gs_state)
   	{
   		case 1://STATE_ONE:
   			if(p_gp->sub_d[4] >= p_gp->ignore_z_th){
   				if(ratio_x > (p_gp->ratio_th)){
   					p_gp->x_plus = 1;
   					p_gp->max_x1 = ratio_x;
   				}else{
   					p_gp->x_plus = 0;
   					p_gp->max_x1 = 0;
   				}

   				if(ratio_x < -(p_gp->ratio_th)){
   					p_gp->x_minus = 1 ;
   					p_gp->min_x1 = ratio_x;
   				}else{
   					p_gp->x_minus = 0;
   					p_gp->min_x1 = 0;
   				}

   				if(ratio_y > (p_gp->ratio_th)){
   					p_gp->y_plus = 1;
   					p_gp->max_y1 = ratio_y;
   				}else{
   					p_gp->y_plus = 0;
   					p_gp->max_y1 = 0;
   				}

   				if(ratio_y < -(p_gp->ratio_th)){
   					p_gp->y_minus = 1;
   					p_gp->min_y1 = ratio_y;
   				}else{
   					p_gp->y_minus = 0;
   					p_gp->min_y1 = 0;
   				}
   			}

   			if( (p_gp->x_plus > 0) | (p_gp->x_minus > 0) |
   				(p_gp->y_plus > 0) | (p_gp->y_minus > 0) )
   			{
   				p_gp->gs_state = STATE_TWO;
   			}else{
   				p_gp->gs_state = STATE_ONE;
   			}

   		break;

   		case 2://STATE_TWO:
   			if( (p_gp->sub_d[4] < p_gp->ignore_z_th) )
   			{
   				clearGSparams(p_gp);
   			}else if(
   				      ((p_gp->x_plus ) && (ratio_x < -(p_gp->ratio_th))) ||
   				      ((p_gp->x_minus) && (ratio_x >  (p_gp->ratio_th))) ||
   				      ((p_gp->y_plus ) && (ratio_y < -(p_gp->ratio_th))) ||
   				      ((p_gp->y_minus) && (ratio_y >  (p_gp->ratio_th))) )
   			{
   				if(ratio_x > (p_gp->ratio_th)){
   					p_gp->max_x2 = ratio_x;
   				}else{
   					p_gp->max_x2 = 0;
   				}

   				if(ratio_x < -(p_gp->ratio_th)){
   					p_gp->min_x2 = ratio_x;
   				}else{
   					p_gp->min_x2 = 0;
   				}

   				if(ratio_y > (p_gp->ratio_th)){
   					p_gp->max_y2 = ratio_y;
   				}else{
   					p_gp->max_y2 = 0;
   				}

   				if(ratio_y < -(p_gp->ratio_th)){
   					p_gp->min_y2 = ratio_y;
   				}else{
   					p_gp->min_y2 = 0;
   				}
   				p_gp->gs_state = STATE_THREE;

   			}else {
   				if( (ratio_x > (p_gp->max_x1)) && (ratio_x > (p_gp->ratio_th))){
   					p_gp->max_x1 = ratio_x;
   					p_gp->x_plus = 1;
   				}else if( (ratio_x < (p_gp->min_x1)) & (ratio_x < -(p_gp->ratio_th)) ){
   					p_gp->min_x1 = ratio_x;
   					p_gp->x_minus = 1;
   				}
   				if( (ratio_y > (p_gp->max_y1)) & (ratio_y > (p_gp->ratio_th)) ){
   					p_gp->max_y1 = ratio_y;
   					p_gp->y_plus = 1;
   				}else if( (ratio_y < (p_gp->min_y1)) & (ratio_y < -(p_gp->ratio_th)) ){
   					p_gp->min_y1 = ratio_y;
   					p_gp->y_minus =1;
   				}
   				if( p_gp->x_plus && p_gp->x_minus){
   					if((p_gp->max_x1) > -(p_gp->min_x1)) {
   						p_gp->x_plus  = 1;
   						p_gp->x_minus = 0;
   					}else {
   						p_gp->x_plus  = 0;
   						p_gp->x_minus = 1;
   					}
   				}
   				if( p_gp->y_plus && p_gp->y_minus){
   					if((p_gp->max_y1) > -(p_gp->min_y1)) {
   						p_gp->y_plus  = 1;
   						p_gp->y_minus = 0;
   					}else {
   						p_gp->y_plus  = 0;
   						p_gp->y_minus = 1;
   					}
   				}
   				p_gp->gs_state = STATE_TWO;
   			}
   		break;
   		case 3://STATE_THREE:
   					if( p_gp->sub_d[4] < (p_gp->ignore_z_th) )
   					{
   						if( (p_gp->x_plus) & (p_gp->min_x2 < -(p_gp->ratio_th))){
   							p_gp->diff_max_x = p_gp->max_x1 - p_gp->min_x2;
   						}else if( (p_gp->x_minus) & (p_gp->max_x2 > p_gp->ratio_th) ){
   							p_gp->diff_max_x = p_gp->max_x2 - p_gp->min_x1;
   						}else {
   							p_gp->diff_max_x = 0;
   						}

   						if( (p_gp->y_plus) & (p_gp->min_y2 < -(p_gp->ratio_th)) ){
   							p_gp->diff_max_y = p_gp->max_y1 - p_gp->min_y2;
   						}else if( (p_gp->y_minus) & (p_gp->max_y2 > p_gp->ratio_th) ){
   							p_gp->diff_max_y = p_gp->max_y2 - p_gp->min_y1;
   						}else{
   							p_gp->diff_max_y = 0;
   						}

   						//// Final direction Judgement ////
   						if( (p_gp->diff_max_x >= p_gp->diff_max_y)){
   						//if(0){////20151201
   						if(p_gp->x_plus == 1){
   								p_gp->res_gs = DIR_RIGHT;
#ifdef CLEAR_RAM_DEBUG
    clear_gs_status(p_gp);
#endif
#ifdef UART_DEBUG
   							   UARTCharPut(UART1_BASE, 12);
   							 while(UARTBusy(UART1_BASE));
   							 UARTCharPut(UART1_BASE, 82);
   							 while(UARTBusy(UART1_BASE));
   						  UARTCharPut(UART1_BASE, 12);
						      while(UARTBusy(UART1_BASE));
#endif

   							}else {
   								p_gp->res_gs = DIR_LEFT;
#ifdef CLEAR_RAM_DEBUG
    clear_gs_status(p_gp);
#endif
#ifdef UART_DEBUG
   							 UARTCharPut(UART1_BASE, 12);
   							                             while(UARTBusy(UART1_BASE));
   							                             UARTCharPut(UART1_BASE, 76);
   							                             while(UARTBusy(UART1_BASE));
   							                          UARTCharPut(UART1_BASE, 12);
   							                              while(UARTBusy(UART1_BASE));
#endif
   							}
   						}else {
   							if(p_gp->y_plus == 1){
   								p_gp->res_gs = DIR_TOP;
#ifdef CLEAR_RAM_DEBUG
    clear_gs_status(p_gp);
#endif
#ifdef UART_DEBUG
   							 UARTCharPut(UART1_BASE, 12);
   							                             while(UARTBusy(UART1_BASE));
   							                             UARTCharPut(UART1_BASE, 84);
   							                             while(UARTBusy(UART1_BASE));
   							                          UARTCharPut(UART1_BASE, 12);
   							                              while(UARTBusy(UART1_BASE));
#endif
   							}
   							else
   							{
   								p_gp->res_gs = DIR_BOTTOM;
#ifdef CLEAR_RAM_DEBUG
    clear_gs_status(p_gp);
#endif
#ifdef UART_DEBUG
   							 UARTCharPut(UART1_BASE, 12);
   							                             while(UARTBusy(UART1_BASE));
   							                             UARTCharPut(UART1_BASE, 66);
   							                             while(UARTBusy(UART1_BASE));
   							                          UARTCharPut(UART1_BASE, 12);
   							                              while(UARTBusy(UART1_BASE));
#endif
   							}
   						}


   						//if(p_gp->speed_counts < (u16)COUNTS_HIGH_SP*100/(u16)STANDARD_SAMPLING_RATE){
   						//if(p_gp->speed_counts < 10){
   						if(p_gp->speed_counts < 20)//20151201
   						{
   						    p_gp->res_gs |=((u16)SPEED_HIGH<<4);
   						     p_gp->res_gs = 0;//2015�?��� ̫�?˲������?
   						}//else if(p_gp->speed_counts < (u16)COUNTS_MID_SP*(u16)gs_sampling_rate/(u16)STANDARD_SAMPLING_RATE){
   						//	p_gp->res_gs |=((u16)SPEED_MID<<4);
   						//}
   						clearGSparams(p_gp);

   					}else {
   						if( (ratio_x > p_gp->max_x2) & (ratio_x > p_gp->ratio_th) ){
   							p_gp->max_x2 = ratio_x;
   						}else if ( (ratio_x < (p_gp->min_x2)) & (ratio_x < -(p_gp->ratio_th))){
   							p_gp->min_x2 = ratio_x;
   						}
   						if( (ratio_y > (p_gp->max_y2)) & (ratio_y > (p_gp->ratio_th)) ){
   							p_gp->max_y2 = ratio_y;
   						}else if( (ratio_y < (p_gp->min_y2)) & (ratio_y < -(p_gp->ratio_th))){
   							p_gp->min_y2 = ratio_y;
   						}
   						p_gp->gs_state = STATE_THREE;
   					}

   				break;

   				default:
   				break;
   			}

   			//// Speed Judgement counts////
   			if(p_gp->gs_state > STATE_ONE){
   				p_gp->speed_counts++;
   			}else{
   				p_gp->speed_counts = 0;
   			}

   			return;
  }// End of getDirection()


   // *******************************************************************
   // * Name	: getZoom(Input1, Input2, Input3)
   // * Input	: Input1 = raw_data, Input1 = aoc_data, Input3 = struct gp_params pointer
   // * Output	: Output return value is unsigned 16bits integer = 0x0Z00
   //            Z(4bits from the higer 5th to 8th bit) ; Zoom results
   // * Note	: get the result of Zoom
   // *******************************************************************
   void getZoom(struct gp_params *p_gp, u8 j){
   	const u16 zoom_z_th[5] = {
   		1000,
   		2000,
   		4000,
   	    6000,
   		8000
   	};
   	u8 i;
   	static u8 now_level=6, temp_level=6, prev_level=6;
   	static u8 to_zoom_counts = 0, out_zoom_counts=0, zoom_mode_counts=0;
   	u16 to_zoom_th, out_zoom_th, zoom_mode_th;
   	to_zoom_th = (u16)(p_gp->to_zoom_th)*(u16)gs_sampling_rate/(u16)STANDARD_SAMPLING_RATE;
   	out_zoom_th = (u16)(p_gp->out_zoom_th)*(u16)gs_sampling_rate/(u16)STANDARD_SAMPLING_RATE;
   	zoom_mode_th = (u16)(p_gp->zoom_mode_th)*(u16)gs_sampling_rate/(u16)STANDARD_SAMPLING_RATE;

   	if( (p_gp->zoomModeNow==0) & (to_zoom_counts==0)){
   		p_gp->zoom_aoc[4] = 0;
   		for(i=0;i<4;i++){
   			p_gp->zoom_aoc[i] =  p_gp->aoc_d[i];
   			p_gp->zoom_aoc[4] += p_gp->zoom_aoc[i];
   		}
   	}

   	for(i=0;i<5;i++){
   		if(p_gp->raw_d[j][i] > p_gp->zoom_aoc[i]){
   			p_gp->zoom_d[i] = p_gp->raw_d[j][i] - p_gp->zoom_aoc[i];
   		}else{
   			p_gp->zoom_d[i] = 0;
   		}
   	}

   	if(p_gp->zoomModeNow){
   		clearGSparams(p_gp);
   		if(p_gp->zoom_d[4] > zoom_z_th[4]){
   			temp_level = ZOOM_LEVEL5;
   		}else if(p_gp->zoom_d[4] > zoom_z_th[3]){
   			temp_level = ZOOM_LEVEL4;
   		}else if(p_gp->zoom_d[4] > zoom_z_th[2]){
   			temp_level = ZOOM_LEVEL3;
   		}else if(p_gp->zoom_d[4] > zoom_z_th[1]){
   			temp_level = ZOOM_LEVEL2;
   		}else if(p_gp->zoom_d[4] > zoom_z_th[0]){
   			temp_level = ZOOM_LEVEL1;
   		}else{
   			temp_level = 0;
   		}

   		if((now_level!=temp_level) && (temp_level!=0) &&
   			(temp_level==prev_level)){
   			zoom_mode_counts++;
   		}else{
   			zoom_mode_counts = 0;
   		}

   		if(zoom_mode_counts >= zoom_mode_th){
   			if(zoom_mode_counts >= zoom_mode_th){
   				now_level = temp_level;
   				zoom_mode_counts = 0;
   			}
   			//p_gp->res_gs=0;
   			p_gp->res_gs = ((u16)now_level)<<8;
   		}

   		prev_level = temp_level;
   		if( (temp_level==0) &&
   			(out_zoom_counts >= out_zoom_th)){
   			out_zoom_counts=0;
   			zoom_mode_counts = 0;
   			p_gp->zoomModeNow = 0;
   		}else if(temp_level==0){
   			out_zoom_counts++;
   		}else{
   			out_zoom_counts=0;
   		}
   	}else{
   		if( (p_gp->zoom_d[4] > zoom_z_th[0]) &
   			(to_zoom_counts >= to_zoom_th)){
   			if(p_gp->zoom_d[4] > zoom_z_th[4]){
   				now_level = ZOOM_LEVEL5;
   			}else if(p_gp->zoom_d[4] > zoom_z_th[3]){
   				now_level = ZOOM_LEVEL4;
   			}else if(p_gp->zoom_d[4] > zoom_z_th[2]){
   				now_level = ZOOM_LEVEL3;
   			}else if(p_gp->zoom_d[4] > zoom_z_th[1]){
   				now_level = ZOOM_LEVEL2;
   			}else if(p_gp->zoom_d[4] > zoom_z_th[0]){
   				now_level = ZOOM_LEVEL1;
   			}
   			//p_gp->res_gs=0;
   			p_gp->res_gs = ((u16)now_level)<<8;
   			p_gp->zoomModeNow = 1;
   			to_zoom_counts = 0;
   			clearGSparams(p_gp);
   		}else if((p_gp->zoom_d[4] > zoom_z_th[0])){
   			to_zoom_counts++;
   		}else{
   			to_zoom_counts = 0;
   		}
   	}

   	return ;

   }
/*���Ǻ��?ָ���*/
   void output_gs_level(struct gp_params *p_gp)
   {
//       static u8 open_count=0,stop_count=0,close_count=0,leftright_count=0;
//       if(stop_count == 0)
//       {
//       if((p_gp->res_gs)&0xff00)//stop
//       {
//           stop_count=1;
//           STOPPLUSE_LEVELLOW;
//       }
//       }
//       if(open_count == 0)
//       {
//       if(((p_gp->res_gs)&0x0f)==0x8)//open
//       {
//           open_count=1;
//           OPENPLUSE_LEVELLOW;
//       }
//       }
//       if(close_count == 0)
//       {
//       if(((p_gp->res_gs)&0x0f)==0x4)//close
//       {
//           close_count=1;
//           CLOSEPLUSE_LEVELLOW;
//       }
//       }
//       if(leftright_count==0)
//       {
//    	   if((((p_gp->res_gs)&0x0f)==0x1)||(((p_gp->res_gs)&0x0f)==0x2))//close
//    	         {
//    		        leftright_count=1;
//    	             //CLOSEPLUSE_LEVELLOW;
//    	         }
//       }
//       if(leftright_count)
//       {
//    	   leftright_count++;
//    	   if(leftright_count>= 80)
//    	   {
//    		   leftright_count = 0;
//    		   p_gp->res_gs=0;
//    	   }
//       }
//       if(stop_count)
//       {
//           stop_count++;
//           if(stop_count>=80)//�������
//           {
//                stop_count = 0;
//                STOPPLUSE_LEVELHIGH;
//                p_gp->res_gs=0;
//           }
//       }
//       if(open_count)
//       {
//           open_count++;
//           if(open_count>=80)//�������
//           {
//               open_count = 0;
//               OPENPLUSE_LEVELHIGH;
//               p_gp->res_gs=0;
//           }
//       }
//       if(close_count)
//       {
//          close_count++;
//          if(close_count>=80)//�������
//          {
//              close_count = 0;
//              CLOSEPLUSE_LEVELHIGH;
//              p_gp->res_gs=0;
//          }
//
//       }

	   if(LCD_Count<=1)
	       {
	         if((p_gp->res_gs)&0xff00)//����ط���ֵ����ٽ?���ٽ?�ʱ�?�����ܳ���������� �� �?������
	         {
	        	// STOPPLUSE_LEVELLOW;
	        	// DISP_GUESTER_9_16(4, 12, 32, 45);
	        	 DISP_GUESTER_13_16(2, 14, 32, 45);
	        	 Set_lcdlightON();
	        	 OPENPLUSE_LEVELLOW;
	             LCD_Count=80;
	             LCD_DISP_GUESTURE =1 ;
	             goto flag11;
	            // p_gp->res_gs = p_gp->res_gs&0xff00;
	         }

	         switch((p_gp->res_gs)&0x0f){
	  	   				case 0x1:
	  	   				// DISP_GUESTER(4, 12, 32, 45);
	  	   				 DISP_GUESTER_13_16(2, 14, 32, 45);
	  	   				Set_lcdlightON();
	  	   				OPENPLUSE_LEVELLOW;
	  	   					LCD_Count=80;
	  	   				LCD_DISP_GUESTURE =1 ;
	  	   				break;

	  	   				case 0x2:
	  	   				// DISP_GUESTER(4, 12, 32, 45);
	  	   				 DISP_GUESTER_13_16(2, 14, 32, 45);
	  	   				Set_lcdlightON();
	  	   				OPENPLUSE_LEVELLOW;
	  	   					LCD_Count=80;
	  	   					LCD_DISP_GUESTURE =1 ;

	  	   				break;

	  	   				case 0x4:
	  	   				// DISP_GUESTER(4, 12, 32, 45);
	  	   				 DISP_GUESTER_13_16(2, 14, 32, 45);
	  	   				Set_lcdlightON();
	  	   				OPENPLUSE_LEVELLOW;
	  	   				    LCD_Count=80;
	  	   				LCD_DISP_GUESTURE =1 ;
	                      break;
	  	   				case 0x8:
	  	   				// DISP_GUESTER(4, 12, 32, 45);
	  	   				 DISP_GUESTER_13_16(2, 14, 32, 45);
	  	   				Set_lcdlightON();
	  	   		       OPENPLUSE_LEVELLOW;
	  	   					LCD_Count=80;
	  	   				LCD_DISP_GUESTURE =1 ;
	  	   				break;

	  	   				case 0x03:
	  	   				case 0x05:
	  	   				case 0x06:
	  	   				case 0x07:
	  	   				case 0x09:
	  	   				case 0x0a:
	  	   				case 0x0b:
	  	   				case 0x0c:
	  	   				case 0x0d:
	  	   				case 0x0e:
	  	   				case 0x0f:
	  	   				     LCD_Count=80;
	  	   				break;
	  	   				default:
	  	   				break;
	  	   			}
	       }
flag11:	  	   if(LCD_Count)
	  	             LCD_Count--;
	  	           if(LCD_Count==1)
	  	           {
	  	        	 //GrRectFIllBolymin(4, 12, 32, 45, true, true);
	  	        	// GrRectFIllBolymin(2, 14, 32, 45, true, true);
	  	        	 STOPPLUSE_LEVELHIGH;
	  	         	 OPENPLUSE_LEVELHIGH;
	  	        	 CLOSEPLUSE_LEVELHIGH;
	  	             p_gp->res_gs=0;
	  	             LCD_Count = 0;
	  	           }

   }
/*���Ǻ��?ָ���*/
   void output_gs_lcd(struct gp_params *p_gp)//�����?
   {
     if(LCD_Count<=1)
     {
       if((p_gp->res_gs)&0xff00)//����ط���ֵ����ٽ?���ٽ?�ʱ�?�����ܳ���������� �� �?������
       {
           meun_stop();
           LCD_Count=80;
          // p_gp->res_gs = p_gp->res_gs&0xff00;
       }

       switch((p_gp->res_gs)&0x0f){
	   				case 0x1:
	   					meun_left();
	   					LCD_Count=80;
	   				break;

	   				case 0x2:
	   					meun_right();
	   					LCD_Count=80;

	   				break;

	   				case 0x4:
	   				    meun_bottom();
	   				    LCD_Count=80;
                    break;
	   				case 0x8:
	   					meun_top();
	   					LCD_Count=80;
	   				break;

	   				default:
	   				break;
	   			}
     }
	   if(LCD_Count)
	             LCD_Count--;
	           if(LCD_Count==1)
	           {
	             clear_clear();
	             p_gp->res_gs=0;
	           }
   }
//��ʼ��054A
void GP2AP054A_Init(void)
{
     IniALLparams(&st_gp);//�ԼĴ�����ֵ
    // changeOPmode(&st_gp, INIT_REG_ON);//initialize_register	//ChangeMode054.h
     gs_sampling_rate = 100;//Hz
     gs_set_timer_counts = 65536 - 20000000/4/gs_sampling_rate;

     als_polling_msec = 200;//ms
     als_set_timer_counts = 65536 - 20000000/4/256*als_polling_msec/1000;


     changeStandalone(0, &st_gp);

}


//void Clear_dir(void)
//{
//	st_gp.res_gs=0;
//}


void GP2A054A_calculation(void)
{
	unsigned char j,i;

	getGSdata(&st_gp);//��ȡ�?�
	for(j= now-st_gp.gs_int; j<=now; j++)
	{
					//// Active Offset Calibration ///////////////
					if(st_gp.active_osc_on == 1){
						getActiveOffset(&st_gp, j);		//GSalgolism054.h
					}else{
						for(i=0;i<4;i++){
							st_gp.sub_d[i] =st_gp.raw_d[j][i];
						}
					}
					st_gp.sub_d[4] = 0;
					for(i=0;i<4;i++){
						st_gp.sub_d[4] += st_gp.sub_d[i];
					}
					//// Calculation & get direction results ////

					getDirection(&st_gp);				//GSalgolism054.h
					//// Zoom mode ////
					if(st_gp.zoomFuncOn){
						getZoom(&st_gp, j);				//GSalgolism054.h
					}


					//// Output direction results ////
					//if ((pin_a  & 0x18)==0x18) {		//SW4 High LCD display. Excel is prohibited.
					//	output_gs_lcd(&st_gp);			//OutputFuncPIC054A.h
					//}else if(debug_mode == 1 ){			//1:ON DIRECTION
					//	output_GSresult(st_gp.res_gs);	//OutputFuncPIC054A.h
					//	if(st_gp.res_gs != 0){
					//		printf("\r\n");
					//	}
					//}else if(debug_mode == 2 ||debug_mode == 3||debug_mode==4||debug_mode==6||debug_mode==7){
					//	output_gs_debug(&st_gp, j);		//OutputFuncPIC054A.h
					//}else if(debug_mode == 9){
					//	printf("%4LX", (st_gp.res_gs&0xFF0F));//Masking Speed
					//}
	}
     output_gs_lcd(&st_gp);


}

void disp_D0D1D2D3DATA(void)
{
    unsigned short int tp_res;
    unsigned char tp_i;
    unsigned char tp_bit;

    for(tp_i=0;tp_i<4;tp_i++)
    {
    tp_res = warray_r[tp_i*2]+warray_r[tp_i*2+1]*256;
    //if(tp_res>256) while(1);
                            tp_bit = tp_res%10;
                           display_graphic_8x16(6+16*tp_i,5,&FONT_BUF_8_16[tp_bit*16],0);
                            tp_res = tp_res/10;
                            tp_bit = tp_res%10;
                           display_graphic_8x16(6+16*tp_i,4,&FONT_BUF_8_16[tp_bit*16],0);
                            tp_res = tp_res/10;
                            tp_bit = tp_res%10;
                            display_graphic_8x16(6+16*tp_i,3,&FONT_BUF_8_16[tp_bit*16],0);
                            tp_res = tp_res/10;
                            tp_bit = tp_res%10;
                            display_graphic_8x16(6+16*tp_i,2,&FONT_BUF_8_16[tp_bit*16],0);
                            tp_res = tp_res/10;
                            tp_bit = tp_res%10;
                            display_graphic_8x16(6+16*tp_i,1,&FONT_BUF_8_16[tp_bit*16],0);
    }

}

unsigned char Tp_count=0;
unsigned char Tp_flag=0;
unsigned int Tp_count_over=0;

void getZoom_cyw(struct gp_params *p_gp)
{
    unsigned short int Tp_D0,Tp_D1,Tp_D2,Tp_D3;


       Tp_D0 = (unsigned short int)(warray_r[0]+(unsigned short int)(warray_r[1]*256));
       Tp_D1 = (unsigned short int)(warray_r[2]+(unsigned short int)(warray_r[3]*256));
       Tp_D2 = (unsigned short int)(warray_r[4]+(unsigned short int)(warray_r[5]*256));
       Tp_D3 = (unsigned short int)(warray_r[6]+(unsigned short int)(warray_r[7]*256));

    if(Tp_flag==0)
    {
    if((Tp_D0>4000)&&(Tp_D1>4000)&&(Tp_D2>4000)&&(Tp_D3>4000))
    {
        Tp_count++;
    }
    else
    {
        Tp_count=0;
    }

    if(Tp_count>=20)
    {
        p_gp->res_gs = 1<<8;
        Tp_flag = 1;
        Tp_count_over=0;
        Tp_count=0;
    }
    }

    if(Tp_flag==1)
    {
        p_gp->res_gs = 1<<8;
        //if((Tp_D0<500)&&(Tp_D1<500)&&(Tp_D2<500)&&(Tp_D3<500))
        //if((Tp_D0<800)&&(Tp_D1<800)&&(Tp_D2<800)&&(Tp_D3<800))//������׸Ĳ���
        //{
            Tp_count_over++;
        //}
        if(Tp_count_over>=800)//20151201//500
        {
            Tp_flag=0;
            Tp_count=0;
            Tp_count_over=0;
            clearGSparams(p_gp);
         //   p_gp->gs_state=0;//20151201
            //IniALLparams(&p_gp);//�ԼĴ�����ֵ
        }
    }

}

unsigned char Flag_His_RL=0;
unsigned char Flag_RL=0;
unsigned char Flag_His_TB=0;
unsigned char Flag_TB=0;
unsigned short int Cal_count=0;
//unsigned short int MAX03=0;
//unsigned short int MAX12=0;
//unsigned short int MAX01=0;
//unsigned short int MAX23=0;
void clear_guestram(void)
{
    Cal_count=0;

           Flag_His_RL=0;
           Flag_RL=0;
           Flag_His_TB=0;
           Flag_TB=0;
}
unsigned char getDirection_cyw(void)
{
    unsigned short int Tp_D0,Tp_D1,Tp_D2,Tp_D3;
    unsigned char Tp_Res=0;

    Tp_D0 = warray_r[0]+warray_r[1]*256;
    Tp_D1 = warray_r[2]+warray_r[3]*256;
    Tp_D2 = warray_r[4]+warray_r[5]*256;
    Tp_D3 = warray_r[6]+warray_r[7]*256;

   // if((Tp_D0>4000)&&(Tp_D1>4000)&&(Tp_D2>4000)&&(Tp_D3>4000))
    if((Tp_D0>4050)&&(Tp_D1>4050)&&(Tp_D2>4050)&&(Tp_D3>4050))
    {

        clear_guestram();

        return Tp_Res;
    }

   // if((Tp_D0<220)&&(Tp_D1<220)&&(Tp_D2<220)&&(Tp_D3<220))
    if((Tp_D0<300)&&(Tp_D1<300)&&(Tp_D2<300)&&(Tp_D3<300))
        {

          clear_guestram();
          return Tp_Res;
        }

  //  if(Cal_count>100)//̫��
   //     {

    //    clear_guestram();
    //    return Tp_Res;
    //     }

   // Cal_count++;

    if((Tp_D0+Tp_D3)>=(Tp_D1+Tp_D2))//RIGHT
    {

        if((Flag_His_RL==0)&&(Flag_RL==0))
        {
        if(((Tp_D0+Tp_D3)-(Tp_D1+Tp_D2))>1000)
        {

            Flag_His_RL = 1;
            Flag_RL=1;
        }
        }
    }
    else
    {
        if((Flag_His_RL==0)&&(Flag_RL==0))
        {
        if(((Tp_D1+Tp_D2)-(Tp_D0+Tp_D3))>1000)
         {

                    Flag_His_RL = 2;
                    Flag_RL=2;
          }
        }

    }
    if((Tp_D0+Tp_D1)>=(Tp_D2+Tp_D3))
     {

        if((Flag_His_TB==0)&&(Flag_TB==0))
               {
               if(((Tp_D0+Tp_D1)-(Tp_D2+Tp_D3))>1000)
                {

                           Flag_His_TB = 3;
                           Flag_TB=3;
                 }
               }
     }
     else
     {

         if((Flag_His_TB==0)&&(Flag_TB==0))
               {
                        if(((Tp_D2+Tp_D3)-(Tp_D0+Tp_D1))>1000)
                         {

                                    Flag_His_TB = 4;
                                    Flag_TB=4;
                          }
              }
     }


    if((Flag_His_RL==Flag_RL)&&(Flag_His_RL==1))//֮ǰ��
    {
        if((Tp_D1+Tp_D2)>=(Tp_D0+Tp_D3))
        {
            if(((Tp_D1+Tp_D2)-(Tp_D0+Tp_D3))>1000)
            {
                Flag_RL=2;
            }
        }
    }
    if((Flag_His_RL==Flag_RL)&&(Flag_His_RL==2))//֮ǰ��
        {
            if((Tp_D0+Tp_D3)>=(Tp_D1+Tp_D2))
            {
                if(((Tp_D0+Tp_D3)-(Tp_D1+Tp_D2))>1000)
                {
                    Flag_RL=1;
                }
            }
        }
    if((Flag_His_TB==Flag_TB)&&(Flag_His_TB==3))//֮ǰ��
           {
               if((Tp_D2+Tp_D3)>=(Tp_D0+Tp_D1))
               {
                   if(((Tp_D2+Tp_D3)-(Tp_D0+Tp_D1))>1000)
                   {
                       Flag_TB=4;
                   }
               }
           }
    if((Flag_His_TB==Flag_TB)&&(Flag_His_TB==4))//֮ǰ��
              {
                  if((Tp_D0+Tp_D1)>=(Tp_D2+Tp_D3))
                  {
                      if(((Tp_D0+Tp_D1)-(Tp_D2+Tp_D3))>1000)
                      {
                          Flag_TB=3;
                      }
                  }
              }


    if((Flag_RL!= Flag_His_RL)&&(Flag_RL!=0))
    {
        Tp_Res= Flag_RL;
        clear_guestram();
    }
    if((Flag_TB != Flag_His_TB)&&(Flag_TB!=0))
     {
        Tp_Res= Flag_TB;
        clear_guestram();
     }
   return Tp_Res;
}





/*#define const_timers 20
unsigned char Cal_right[const_timers]={0};
unsigned char Cal_left[const_timers]={0};
unsigned char Cal_top[const_timers]={0};
unsigned char Cal_bottom[const_timers]={0};
unsigned char Cal_count=0;
unsigned char Flag_His_RL=0;
unsigned char Flag_RL=0;
unsigned char Flag_His_TB=0;
unsigned char Flag_TB=0;
void clear_guestram(void)
{
    unsigned char Tp_i=0;
    Cal_count=0;
    Flag_RL=0;
    Flag_His_RL=0;
    Flag_His_TB=0;
    Flag_TB=0;
    for(Tp_i=0;Tp_i<const_timers;Tp_i++)
    {
        Cal_right[Tp_i]=0;
        Cal_left[Tp_i]=0;
        Cal_top[Tp_i]=0;
        Cal_bottom[Tp_i]=0;
    }
}
unsigned char getDirection_cyw(void)
{
    unsigned int Tp_D0,Tp_D1,Tp_D2,Tp_D3;
    unsigned char Tp_Res=0,Tp_i=0,Tp_sumright=0,Tp_sumleft=0,Tp_sumtop=0,Tp_sumbottom=0;

    Tp_D0 = warray_r[0]+warray_r[1]*256;
    Tp_D1 = warray_r[2]+warray_r[3]*256;
    Tp_D2 = warray_r[4]+warray_r[5]*256;
    Tp_D3 = warray_r[6]+warray_r[7]*256;

    if((Tp_D0<220)&&(Tp_D1<220)&&(Tp_D2<220)&&(Tp_D3<220))
    {
        clear_guestram();
        return Tp_Res;
    }

    if((Tp_D0+Tp_D3)>=(Tp_D1+Tp_D2))
    {
        Cal_right[Cal_count]=1;
        Cal_left[Cal_count]=0;
    }
    else
    {
        Cal_right[Cal_count]=0;
        Cal_left[Cal_count]=1;
    }
    if((Tp_D0+Tp_D1)>=(Tp_D2+Tp_D3))
     {
           Cal_top[Cal_count]=1;
           Cal_bottom[Cal_count]=0;
     }
     else
     {
           Cal_top[Cal_count]=0;
           Cal_bottom[Cal_count]=1;
     }

    Cal_count++;
    if(Cal_count>=const_timers)
        Cal_count=0;

    for(Tp_i=0;Tp_i<const_timers;Tp_i++)
    {
        Tp_sumright = Tp_sumright+Cal_right[Tp_i];
        Tp_sumleft = Tp_sumleft+Cal_left[Tp_i];
        Tp_sumtop = Tp_sumtop+Cal_top[Tp_i];
        Tp_sumbottom = Tp_sumbottom+Cal_bottom[Tp_i];
    }

    if(Tp_sumright==const_timers)
    {
        Flag_RL = 1;
    }
    if(Tp_sumleft==const_timers)
    {
         Flag_RL = 2;
    }
    if(Tp_sumtop==const_timers)
       {
           Flag_TB = 3;
       }
       if(Tp_sumbottom==const_timers)
       {
            Flag_TB = 4;
       }

       if((Flag_RL!=0)&&(Flag_RL!=Flag_His_RL))
       {
         if(Flag_His_RL!=0)
         {
             Tp_Res = Flag_RL;
             clear_guestram();
         }
         else
         {
           Flag_His_RL= Flag_RL;
         }

       }
       if((Flag_TB!=0)&&(Flag_TB!=Flag_His_TB))
        {

            if(Flag_His_TB!=0)
             {
                   Tp_Res = Flag_TB;
                    clear_guestram();
             }
             else
             {
                   Flag_His_TB= Flag_TB;
            }
        }
    return Tp_Res;
}*/

void output_lcd_cyw(unsigned short int x_dir)
{
    switch(x_dir){
                      case 0x1:
                        //  meun_left();
                         // meun_right();
                      break;

                      case 0x2:
                         // meun_right();
                         // meun_left();
                      break;

                      case 0x3:
                          //meun_bottom();
                          //  meun_top();
                          break;
                      case 0x4:
                          //meun_top();
                          // meun_bottom();
                      break;

                      default:
                      break;
                  }
}

//���ֶ����?�ȫ�����?
void clear_gs_status(struct gp_params *p_gp)
{
    unsigned char Tp_i;

    for(Tp_i=0;Tp_i<4;Tp_i++){
                p_gp->sub_d[Tp_i]=0;
                p_gp->aoc_d[Tp_i]=0;
                p_gp->raw_d[now][Tp_i]     =0;
                p_gp->raw_d[now][4]     = 0;
                p_gp->sat_flag[now][Tp_i]  = 0;
                p_gp->sat_flag[now][4]  |= 0;

            }
    Tp_i=p_gp->res_gs;
    initGSparams(p_gp);
    p_gp->res_gs=Tp_i;
}
void clear_gs_status_judge(struct gp_params *p_gp)
{
    //
    unsigned short int Tp_D0,Tp_D1,Tp_D2,Tp_D3;


        Tp_D0 = warray_r[0]+warray_r[1]*256;
        Tp_D1 = warray_r[2]+warray_r[3]*256;
        Tp_D2 = warray_r[4]+warray_r[5]*256;
        Tp_D3 = warray_r[6]+warray_r[7]*256;

        if((Tp_D0<200)&&(Tp_D1<200)&&(Tp_D2<200)&&(Tp_D3<200))
            clear_gs_status(p_gp);
}
void GP2AP054A_Init_cyw(void)
{
   //��Щ���ú����տ�������һ���
    unsigned char warray[20]={0x00,0x00,0x00,0x06,0x01,
                                 0x69,0xcd,0xca,0x7c,0x15,
                                 0x70,0x17,0x00,0x00,0x00,
                                 0x00,0x00,0x00,0x00,0x00};
    unsigned char warray2[1]={0x00};
        //unsigned char wsize =1;

    IniALLparams(&st_gp);//�ԼĴ�����ֵ

   // gs_sampling_rate = 100;//Hz 2ms
    //    gs_set_timer_counts = 65536 - 20000000/4/gs_sampling_rate;

    warray2[0]=0;
    seq_i2c_write(0x72, 0, warray2, 1);
    iic_delay;
    iic_delay;iic_delay;iic_delay;
    seq_i2c_write(0x72, 0, warray, 20);
    iic_delay;
    iic_delay;iic_delay;iic_delay;
    warray2[0]=0;
    seq_i2c_write(0x72, 0x41, warray2, 1);
    iic_delay;
    iic_delay;iic_delay;iic_delay;
    warray2[0]=0;
    seq_i2c_write(0x72, 0x02, warray2, 1);
    iic_delay;
    iic_delay;iic_delay;iic_delay;
    warray2[0]=0x80;
    seq_i2c_write(0x72, 0x00, warray2, 1);



}
void GP2AP054A_Init_cyw_debug(void)
{
   // unsigned char warray[20]={0x00,0x00,0x00,0x06,0x01,
   //                           0x69,0xcd,0xca,0x7c,0x15,
   //                           0x70,0x17,0x00,0x00,0x00,
   //                           0x00,0x00,0x00,0x00,0x00};
    unsigned char warray[20]={0x00,0x00,0x00,0x40,0x01,
            0x69,0xcd,0xca,0x7c,0x15,
            0x70,0x17,0x00,0x00,0x00,
                                 0x00,0x00,0x00,0x00,0x00};
    unsigned char warray2[1]={0x00};
        //unsigned char wsize =1;

#ifdef GP2A054Register_DEBUG
    //warray[5] = 0x09;
    warray[6]=0xd8;
#endif


    IniALLparams(&st_gp);//�ԼĴ�����ֵ

   // gs_sampling_rate = 100;//Hz 2ms
    //    gs_set_timer_counts = 65536 - 20000000/4/gs_sampling_rate;

    warray2[0]=0;
    seq_i2c_write(0x72, 0, warray2, 1);
    iic_delay;
    iic_delay;iic_delay;iic_delay;
    seq_i2c_write(0x72, 0, warray, 20);
    iic_delay;
    iic_delay;iic_delay;iic_delay;
    warray2[0]=0;
    seq_i2c_write(0x72, 0x41, warray2, 1);
    iic_delay;
    iic_delay;iic_delay;iic_delay;
    warray2[0]=0;
    seq_i2c_write(0x72, 0x02, warray2, 1);
    iic_delay;
    iic_delay;iic_delay;iic_delay;
    //warray2[0]=0x80;
    warray2[0]=0xa0;
    seq_i2c_write(0x72, 0x00, warray2, 1);



}

void GP2A054A_calculation_cyw(void)
{
    //Read054

    unsigned char tp_i,tp_result=0,j,i;
    static uint8_t guesture_err_timers= 0;
   // unsigned char warray[20]={0,};

    //doWatchdogReset();//cyw add 20150914

    if(error_guesture_cyw == 1)
    {
    	//init054A();
    	if(guesture_err_timers <100)
    	{
    	guesture_err_timers++;
    	GP2AP054A_Init_cyw_debug();
    	}
    	error_guesture_cyw = 0;
    }

    for(tp_i=0;tp_i<8;tp_i++)
    {
        iic_delay;
           warray_r[tp_i] =Read054(tp_i+0x2c);
    }

    //doWatchdogReset();//cyw add 20150914

    if((warray_r[0]==0xff)&&(warray_r[1]==0xff)&&(warray_r[2]==0xff)&&(warray_r[3]==0xff)&&\
            (warray_r[4]==0xff)&&(warray_r[5]==0xff)&&(warray_r[6]==0xff)&&(warray_r[7]==0xff))
    {
        warray_r[0]= warray_r[0];
        return;//iic���˻��б�Ҫ����
    }
#if (defined GUEST_DISP)||(defined GUEST_LEVELOUT)
    getGSdata(&st_gp);

    for(j= now-st_gp.gs_int; j<=now; j++)
        {
                        //// Active Offset Calibration ///////////////

                        if(st_gp.active_osc_on == 1){
                            getActiveOffset(&st_gp, j);     //GSalgolism054.h
                        }else{
                            for(i=0;i<4;i++){
                                st_gp.sub_d[i] =st_gp.raw_d[j][i];
                            }
                        }
                        st_gp.sub_d[4] = 0;
                        for(i=0;i<4;i++){
                            st_gp.sub_d[4] += st_gp.sub_d[i];
                        }
                        //// Calculation & get direction results ////
                     //   if(st_gp.res_gs==0)
                      //  {
                        getDirection(&st_gp);               //GSalgolism054.h

                        //if(st_gp.zoomFuncOn){
                        //getZoom(&st_gp, j);             //GSalgolism054.h
                        //}
                        getZoom_cyw(&st_gp);
                     //   }
       }

#endif


   // tp_result = getDirection_cyw();
#ifdef   GP2A054_DUBUG
    if(!LCD_Count)
    disp_D0D1D2D3DATA();
#endif
    // output_lcd_cyw(tp_result);
#ifdef GUEST_DISP
    output_gs_lcd(&st_gp);
#endif
#ifdef GUEST_LEVELOUT//������˰����Ƶ���������IO ��ִ�����²��?
    if(setting_flag == 0)
    {
       output_gs_level(&st_gp);
    }
#endif
#ifdef CLEAR_RAM_DEBUG
   // clear_gs_status_judge(&st_gp);
#endif
}


void init054A(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
          ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

          ROM_SysCtlDelay(1);


          ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_4);//SCL
          ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, GPIO_PIN_5);//SDA


          ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);//ʹ�ܿ�F ���?���APB������
          ROM_SysCtlDelay(1);
          ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_0);//OPEN LEVEL
          ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);//STOP LEVEL
          ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);//CLOSE LEVEL
          OPENPLUSE_LEVELHIGH;//��ʼΪ��
          STOPPLUSE_LEVELHIGH;//��ʼΪ��
          CLOSEPLUSE_LEVELHIGH;//��ʼΪ��
          //��֪��Ϊʲô��λû�а���DATA�� �����Լ�д�˼Ĵ��?
           HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK)=0X4C4F434B;//���?
           HWREG(GPIO_PORTF_BASE+GPIO_O_CR)=0xff;//����
            HWREG(GPIO_PORTF_BASE+GPIO_O_DEN)=0x07;//��ӦΪ�?ֹ���
}


void guest_reinit(void)
{
	if(setting_flag == 1)
	{
	setting_flag = 0;//���������ò��?
	st_gp.res_gs = 0;
	LCD_Count = 0;
	clearGSparams(&st_gp);
	 STOPPLUSE_LEVELHIGH;
	  	         	 OPENPLUSE_LEVELHIGH;
	  	        	 CLOSEPLUSE_LEVELHIGH;
	}
}
