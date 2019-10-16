#ifndef BACKLIGHT_H_
#define BACKLIGHT_H_


#include <stdint.h>
#include <stdbool.h>
#include <driverlib/gpio.h>

void Set_lcdlightOFF(void);
void Set_lcdlightON(void);
void Out_of_settingmode_cyw(void);
#endif
