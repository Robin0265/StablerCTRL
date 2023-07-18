#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "Servo.h"
#include "PWM.h"
#include "Camera.h"
#include "OLED.h"
#include "Key.h"
#include "GPIOSection_Init.h"
#include "PID.h"
#include "stdlib.h"

#define TRUE 1
#define FALSE 0

/* 自定义函数定义区 */
float Ave(const float *arr, int n);
void err_store(float *arr, int length, float err_new);

PID_TypeDef PID_Pan, PID_Tilt;

/*数据去毛刺*/
int cnt_T = 0, cnt_F = 0;										// 状态判别计数器																										// 运行状态去毛刺
float ang_err_T_True[2][5] = {0.0};										
/*0 for pan, 1 for tilt*/

/*关键步骤判断标志*/
int blobs_detect =FALSE;
uint8_t blobs_detect_last=FALSE;
int PID_Switch = TRUE;
uint8_t Change_Flag=FALSE;

/*舵机处理相关*/
float pan_ang=90.0, tilt_ang=140.0;
float pan_ang_delta = 0, tilt_ang_delta = 0;
// float err_ang[2][3];

/*
tilt_ang_delta <- Phi_Error
pan_ang_delta <- Theta_Error
*/

/*串口通信相关*/
char CameraData[12];
uint8_t MCU_Cmd;
float Data2MCU[3];
Openmv_DataSet_Typedef Openmv_DataSet;

int main(void)
{	
	
	uint8_t i,j;
	
	PWM_Init();														// PWM输出初始化
	/* PB8 -> SCL, PB9 -> SDA */
	OLED_Init();
	/*舵机角度初始化*/
	Servo_SetAngle_Pan(pan_ang);								
	Servo_SetAngle_Tilt(tilt_ang);
	Camera_Serial_Init();
	MCU_Serial_Init();
//	GPIOA_Init(GPIO_Pin_0,GPIO_Mode_Out_PP);
	PID_init(0.2,0,0.3,&PID_Pan);
	PID_init(0.40,0.002,0.35, &PID_Tilt);
	OLED_Clear();
	OLED_ShowString(1, 1, "Pan_A:");
	OLED_ShowString(2, 1, "Tilt_A:");
	OLED_ShowString(3, 1, "Pan_D:");
	OLED_ShowString(4, 1, "Tilt_D:");
		while(TRUE)
		{			
		/*从OpenMV读取角度数据*/
		// 检查串口通信参数！
			CameraData_Process(&Openmv_DataSet);
			/* blobs_detect 判定 */
			
			if((Openmv_DataSet.Distance==-1)||(Openmv_DataSet.Phi_Error==-181)||(Openmv_DataSet.Theta_Error == -181))
			{
				blobs_detect = FALSE;
				// GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_SET);
			}
			else
			{
				blobs_detect = TRUE;
				// GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_RESET);
			}
			
			
			if(blobs_detect_last!=blobs_detect)//判断是否出现、消失小球
			{
				Change_Flag=1;
			}
			blobs_detect_last=blobs_detect; //储存本次判断		
		
			if(Change_Flag)
			{
				for(i=0;i<2;i++)
				{
					for(j=0;j<5;j++)
					{
						ang_err_T_True[i][j]=0;//当出现突变事件去除上次数据
					}
				}
				Change_Flag=0;
			}
				
			
			
			if(blobs_detect)
			{
				tilt_ang_delta = Openmv_DataSet.Phi_Error;
				pan_ang_delta = Openmv_DataSet.Theta_Error; 
				
				err_store(ang_err_T_True[0],5, pan_ang_delta);
				err_store(ang_err_T_True[1],5, tilt_ang_delta);
	
				
				/*直接控制 or PID控制选择*/
				if(PID_Switch)
				{
					/* PID角度迭代 */	
					pan_ang += pid(-Ave(ang_err_T_True[0],5),0,&PID_Pan);
					tilt_ang -= pid(-Ave(ang_err_T_True[1],5),0,&PID_Tilt);
					OLED_ShowSignedNum(3,7,Openmv_DataSet.Phi_Error,4);
					OLED_ShowSignedNum(4,7,Openmv_DataSet.Theta_Error,4);
					OLED_ShowSignedNum(1,7,(int32_t)pan_ang,4);
					OLED_ShowSignedNum(2,8,(int32_t)tilt_ang,4);
				}
				else
				{/*直接控制*/
					/* 直接角度迭代 */ 
					pan_ang+=Ave(ang_err_T_True[0], 5);
					tilt_ang-=Ave(ang_err_T_True[1], 5);
				}	
					
				{/* 防锁死 */
					if(pan_ang>150)
					{
						pan_ang=150; 
						
					}
					if(pan_ang<30)
					{
						pan_ang=30; 
						
					}
					if(tilt_ang<30)
					{
						tilt_ang=30; 
						
					}
					if(tilt_ang>180)
					{
						tilt_ang=180; 
						
					}	
				}	
					
				// 舵机调整Pan, Tilt角度
				Servo_SetAngle_Pan(pan_ang); 
				Servo_SetAngle_Tilt(tilt_ang);
				Delay_ms(100);//注意使用delay使舵机有足够的时间
				
				
			}
			else
			{
		
				/*计数器判断是否复位，避免反复横跳*/
				{
					cnt_F++;
					if(cnt_F==1500)
					{
						cnt_F = 0;
						pan_ang_delta = 0;
						tilt_ang_delta = 0;
						pan_ang = 90.0;
						tilt_ang = 140.0;
						// 舵机角度调整
						Servo_SetAngle_Pan(pan_ang);
						Servo_SetAngle_Tilt(tilt_ang);
					}
				}
		
			}


			/* 发送数据包 */
			{
				Data2MCU[0] = Openmv_DataSet.Distance;
				Data2MCU[1] = tilt_ang;
				Data2MCU[2] = pan_ang;
				Serial_SendFloat(Data2MCU,3);
			}
		}		
}

/* 自定义函数定义区 */
float Ave(const float *arr, int length)
{
	int i;
	float ave_list = 0;
	
    for(i = 0; i<length; i++)
    {
			ave_list += *(arr+i);
    }
    ave_list /= length;
    return ave_list;
}
void err_store(float *arr, int length, float err_new)
{
    int i;
    for(i=0; i<length;i++)
    {
        arr[i] = arr[i+1];
    }
    arr[i-1] = err_new;
}
