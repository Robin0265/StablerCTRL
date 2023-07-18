/*电机基础调节*/
#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Servo_Init(void)
{
	PWM_Init();
}
/*In degrees*/
void Servo_SetAngle_Pan(float Angle)
{
	PWM_SetCompare_Pan((uint16_t)(Angle / 180 * 2000 + 500));
}
/*In degrees*/
void Servo_SetAngle_Tilt(float Angle)
{
	PWM_SetCompare_Tilt((uint16_t)(Angle / 180 * 2000 + 500));
}
void Servo_SetAngle_Unit_Pan(float Angle)
{
	;
}
void Servo_SetAngle_Unit_Tilt(float Angle)
{
	;
}
