#ifndef __PID_H
#define __PID_H

#include "stm32f10x.h"                  // Device header

typedef struct
{
	float Kp, Ki, Kd;
	float p_out, i_out, d_out;
//	int present, target;
	float error, last_error, last_error2, last_error3, last_error4, last_error5;
	float output;
	
	
}PID_TypeDef;


void PID_init(float Kp, float Ki, float Kd, PID_TypeDef* PID);

float pid(float present, float target, PID_TypeDef* PID);

int better_PID(int present, u16 target, PID_TypeDef* PID);

int incre_PID(int present, u16 target, PID_TypeDef* PID);

#endif
