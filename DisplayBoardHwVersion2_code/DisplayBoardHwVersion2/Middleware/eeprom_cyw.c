#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <driverlib/rom.h>
#include <driverlib/rom_map.h>
#include <driverlib/sw_crc.h>
#include "Middleware/eeprom.h"
#include "Middleware/uartstdio.h"
#include "paramdatabase.h"
#include "Application/interTaskCommunication.h"
#include "Gesture_Sensor/ram_cyw.h"
#include <driverlib/eeprom.h>
#include "Gesture_Sensor/GP2AP054A_cyw.h"

void EEPROM_Gesture_Sava_cyw(void)
{

    uint32_t Tp_data_xx=0;
    uint32_t *Tp_p_xx;

    if(menu_gesture_flag_cyw>=2)
                menu_gesture_flag_cyw = 0;
    Tp_data_xx = menu_gesture_flag_cyw;
    Tp_p_xx = &Tp_data_xx;


    EEPROMProgram(Tp_p_xx,890,4);

}

void EEPROM_Gesture_Load_cyw(void)
{
    //uint32_t *Tp_p;
    //uint32_t Tp_data=0;
    uint32_t Tp_data_xx=0;
    uint32_t *Tp_p_xx;


    Tp_p_xx = &Tp_data_xx;
    EEPROMRead(Tp_p_xx, 890,4);
    if(Tp_data_xx >=2)//出错了再试一次
    {
        Tp_data_xx =0;
        menu_gesture_flag_cyw = (uint8_t)Tp_data_xx ;
        EEPROM_Gesture_Sava_cyw();
    }
    menu_gesture_flag_cyw = (uint8_t)Tp_data_xx ;
}
