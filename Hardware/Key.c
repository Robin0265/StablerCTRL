#include "stm32f10x.h"                  // Device header
#include "Delay.h"

uint8_t Key_GetNum(uint16_t Pin1,uint16_t Pin2)
{
	uint8_t KeyNum=0;
	
		if(GPIO_ReadInputDataBit(GPIOB,Pin1)==0)
		{
			Delay_ms(50);
			while(GPIO_ReadInputDataBit(GPIOB,Pin1)==0);
			Delay_ms(50);
			KeyNum=1;
		}
		if(GPIO_ReadInputDataBit(GPIOB,Pin2)==0)
		{
			Delay_ms(50);
			while(GPIO_ReadInputDataBit(GPIOB,Pin2)==0);
			Delay_ms(50);
			KeyNum=2;
		}
	
	return KeyNum;
}
