#ifndef __PWM_H
#define __PWM_H
#include "stm32f10x.h"                  // Device header
void PWM_Init(void);

void PWM_SetCompare_Pan(uint16_t Compare);
void PWM_SetCompare_Tilt(uint16_t Compare);

#endif
