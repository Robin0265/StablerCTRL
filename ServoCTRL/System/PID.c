#include "PID.h"

//0、PID初始化函数，给各参数赋值
//参数(4个)：Kp，Ki，Kd，处理的PID结构体的地址
void PID_init(float Kp, float Ki, float Kd, PID_TypeDef* PID)
{
	PID->Kp = Kp;
	PID->Ki = Ki;
	PID->Kd = Kd;
	PID->p_out = 0.0;
	PID->i_out = 0.0; 
	PID->d_out = 0.0;
	PID->error = 0.0;
	PID->last_error = 0.0;
	PID->last_error2 = 0.0; 
	PID->last_error3 = 0.0;
	PID->last_error4 = 0.0;
	PID->last_error5 = 0.0;
	PID->output = 0.0;
}

//1、位置PID
//参数(3个)：当前位置，目标位置，处理的PID结构体的地址
float pid(float present, float target, PID_TypeDef* PID)
{
	static int32_t count=0;
	
	PID->error = target-present;	//本次误差 = 目标值 - 实际值
	
	PID->p_out = PID->Kp * PID->error;//比例
	PID->i_out += PID->Ki * PID->error;//积分
	PID->d_out = PID->Kd * (PID->error - PID->last_error);//微分
	
	PID->output = PID->p_out + PID->i_out + PID->d_out;//输出
	
	PID->last_error = PID->error;//上次误差 = 本次误差
	
	if(PID->output>20)
	{
		count++;
		if(count<40)
		{
			PID->output=0;
		}
		else
		{
			count=0;
		}

	}
	if(PID->output<-20) //去处一闪而过的影响
	{
		count++;
		if(count<40)
		{
			PID->output=0;
		}
		else
		{
			count=0;
		}
	}
	
	//设置上下限
	if(PID->output>40)
	{
		PID->output=40;
	}
		
	if(PID->output<-40)
	{
		PID->output=-40;
	}
		
	return PID->output;
}


//2、改进版位置PID(对微分项进行改善，考虑历史信息，降噪)
//参数(3个)：当前位置，目标位置，处理的PID结构体的地址
int better_PID(int present, u16 target, PID_TypeDef* PID)
{
	PID->error = target-present;	//本次误差 = 目标值 - 实际值
	
	PID->p_out = PID->Kp * PID->error;//比例
	PID->i_out += PID->Ki * PID->error;//积分
	PID->d_out = PID->Kd * 1/16 * (PID->error + 3*PID->last_error + 2*PID->last_error2 -2*PID->last_error3 - 3*PID->last_error4 - PID->last_error5);//微分
	
	PID->output = PID->p_out + PID->i_out + PID->d_out;//输出
	
	PID->last_error5 = PID->last_error4;//上次误差 = 本次误差
	PID->last_error4 = PID->last_error3;//上次误差 = 本次误差
	PID->last_error3 = PID->last_error2;//上次误差 = 本次误差
	PID->last_error2 = PID->last_error;//上次误差 = 本次误差
	PID->last_error = PID->error;//上次误差 = 本次误差
	
	return PID->output;
}


//3、增量式PID(目前效果不好)
//参数(3个)：当前位置，目标位置，处理的PID结构体的地址
int incre_PID(int present, u16 target, PID_TypeDef* PID)
{
	PID->error = target-present;	//本次误差 = 目标值 - 实际值
	
	PID->p_out = PID->Kp * (PID->error - PID->last_error);//比例
	PID->i_out += PID->Ki * PID->error;//积分
	PID->d_out = PID->Kd * (PID->error - 2*PID->last_error + PID->last_error2);//微分
	
	PID->output += PID->p_out + PID->i_out + PID->d_out;//输出
	
	PID->last_error2 = PID->last_error;
	PID->last_error = PID->error;//上次误差 = 本次误差
	
	return PID->output;
}
