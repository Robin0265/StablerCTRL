#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>
#include "Camera.h"


#define MCU_TX_Pin GPIO_Pin_9
#define MCU_RX_Pin GPIO_Pin_10
/*
主控串口通信:
PA9 -> USART1 TX
PA10 -> USART1 RX
*/

#define Camera_TX_Pin GPIO_Pin_2
#define Camera_RX_Pin GPIO_Pin_3

/*
摄像头串口通信:
PA2 -> USART2 TX 
PA3 -> USART2 RX
*/


extern char CameraData[12];
extern uint8_t MCU_Cmd;
	

void MCU_Serial_Init(void)
{
/***************************************************/
	//开启时钟模块
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
/***************************************************/
	//配置GPIO
	GPIO_InitTypeDef GPIO_InitStruture;
	
	GPIO_InitStruture.GPIO_Mode=GPIO_Mode_AF_PP; //选择模式 复用推挽模式 串口模块接管引脚的高低电平 输出高低电平
	GPIO_InitStruture.GPIO_Pin=MCU_TX_Pin; //选择引脚
	GPIO_InitStruture.GPIO_Speed=GPIO_Speed_50MHz; //选择GPIO的速度
	GPIO_Init(GPIOA,&GPIO_InitStruture);//传递GPIOA的参数
	
	GPIO_InitStruture.GPIO_Mode=GPIO_Mode_IN_FLOATING; //选择模式 浮空输入模式 引脚控制权由对应串口的TX接管 注意TX RX引脚模式不同
	GPIO_InitStruture.GPIO_Pin=MCU_RX_Pin; //选择引脚
	GPIO_InitStruture.GPIO_Speed=GPIO_Speed_50MHz; //选择GPIO的速度
	GPIO_Init(GPIOA,&GPIO_InitStruture);//传递GPIOA的参数
	
/***************************************************/
	//配置串口模块
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate=115200;  //波特率
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None; //数据流设置
	USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx; //串口模式设置 读或写
	USART_InitStructure.USART_Parity=USART_Parity_No; //校验位
	USART_InitStructure.USART_StopBits=USART_StopBits_1; //停止位的宽度
	USART_InitStructure.USART_WordLength=USART_WordLength_8b; //帧的宽度 8bits无校验位 9 bits有1位校验位
	USART_Init(USART1,&USART_InitStructure);
    
/***************************************************/
//	//配置NVIC
//    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE); //开启串口的读取中断
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //NVIC分组方式 全程序只能有一种分组方式
//	NVIC_InitTypeDef NVIC_InitStructure;
//	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;//修改位 配置NVIC接收线
//	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE; //使能
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0; //配置抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;//配置响应优先级
//	NVIC_Init(&NVIC_InitStructure);

/***************************************************/
	//配置DMA DMA+USART自动接收大量数据 无需占用CPU资源
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE); //别玩把这个时钟开启放在最前方
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE); //串口读取寄存器非空触发
	
	DMA_InitTypeDef DMA_InitStruct; 
	//USART1->DR 转到USART1的DR寄存器 &取寄存器的地址
	DMA_InitStruct.DMA_PeripheralBaseAddr=(uint32_t)&USART1->DR; //外设首地址 读取寄存器地址
	DMA_InitStruct.DMA_PeripheralDataSize= DMA_PeripheralDataSize_Byte;//外设数据宽度
	DMA_InitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;//外设指针是否递增
	DMA_InitStruct.DMA_MemoryBaseAddr=(uint32_t)&MCU_Cmd; //内存地址
	DMA_InitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte; //内存数据宽度
	DMA_InitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable ;//内存指针是否递增
	DMA_InitStruct.DMA_DIR=DMA_DIR_PeripheralSRC  ;//传输方向 SRC:外设作为源头 DIS外设作为目的地
	DMA_InitStruct.DMA_BufferSize=1; //传输计数器 传输n次 每次递减 到0停止
	DMA_InitStruct.DMA_Mode=DMA_Mode_Circular;//DMA模式 是否自动重装
	DMA_InitStruct.DMA_M2M=DMA_M2M_Disable;//软件触发还是硬件触发 1是软件触发
	DMA_InitStruct.DMA_Priority=DMA_Priority_Medium;//优先级
	
	
	DMA_Init(DMA1_Channel5,&DMA_InitStruct);//每个硬件触发都有自己对应的channel号
	DMA_Cmd(DMA1_Channel5,ENABLE);
	
	USART_Cmd(USART1,ENABLE); //开启串口

}

void Camera_Serial_Init(void)
{
/***************************************************/
	//开启时钟模块
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
/***************************************************/
	//配置GPIO
	GPIO_InitTypeDef GPIO_InitStruture;
	
	GPIO_InitStruture.GPIO_Mode=GPIO_Mode_AF_PP; //选择模式 复用推挽模式 串口模块接管引脚的高低电平 输出高低电平
	GPIO_InitStruture.GPIO_Pin=Camera_TX_Pin; //选择引脚
	GPIO_InitStruture.GPIO_Speed=GPIO_Speed_50MHz; //选择GPIO的速度
	GPIO_Init(GPIOA,&GPIO_InitStruture);//传递GPIOA的参数
	
	GPIO_InitStruture.GPIO_Mode=GPIO_Mode_IN_FLOATING; //选择模式 浮空输入模式 引脚控制权由对应串口的TX接管 注意TX RX引脚模式不同
	GPIO_InitStruture.GPIO_Pin=Camera_RX_Pin; //选择引脚
	GPIO_InitStruture.GPIO_Speed=GPIO_Speed_50MHz; //选择GPIO的速度
	GPIO_Init(GPIOA,&GPIO_InitStruture);//传递GPIOA的参数
	
/***************************************************/
	//配置串口模块
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate=115200;  //波特率
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None; //数据流设置
	USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx; //串口模式设置 读或写
	USART_InitStructure.USART_Parity=USART_Parity_No; //校验位
	USART_InitStructure.USART_StopBits=USART_StopBits_1; //停止位的宽度
	USART_InitStructure.USART_WordLength=USART_WordLength_8b; //帧的宽度 8bits无校验位 9 bits有1位校验位
	USART_Init(USART2,&USART_InitStructure);
    
/***************************************************/
//	//配置NVIC
//    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE); //开启串口的读取中断
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //NVIC分组方式 全程序只能有一种分组方式
//	NVIC_InitTypeDef NVIC_InitStructure;
//	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;//修改位 配置NVIC接收线
//	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE; //使能
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0; //配置抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;//配置响应优先级
//	NVIC_Init(&NVIC_InitStructure);

/***************************************************/
	//配置DMA DMA+USART自动接收大量数据 无需占用CPU资源
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE); //别玩把这个时钟开启放在最前方
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE); //串口读取寄存器非空触发
	
	DMA_InitTypeDef DMA_InitStruct; 
	//USART1->DR 转到USART1的DR寄存器 &取寄存器的地址
	DMA_InitStruct.DMA_PeripheralBaseAddr=(uint32_t)&USART2->DR; //外设首地址 读取寄存器地址
	DMA_InitStruct.DMA_PeripheralDataSize= DMA_PeripheralDataSize_Byte;//外设数据宽度
	DMA_InitStruct.DMA_PeripheralInc=DMA_PeripheralInc_Disable;//外设指针是否递增
	DMA_InitStruct.DMA_MemoryBaseAddr=(uint32_t)&CameraData; //内存地址
	DMA_InitStruct.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte; //内存数据宽度
	DMA_InitStruct.DMA_MemoryInc=DMA_MemoryInc_Enable ;//内存指针是否递增
	DMA_InitStruct.DMA_DIR=DMA_DIR_PeripheralSRC  ;//传输方向 SRC:外设作为源头 DIS外设作为目的地
	DMA_InitStruct.DMA_BufferSize=12; //传输计数器 传输n次 每次递减 到0停止
	DMA_InitStruct.DMA_Mode=DMA_Mode_Circular;//DMA模式 是否自动重装
	DMA_InitStruct.DMA_M2M=DMA_M2M_Disable;//软件触发还是硬件触发 1是软件触发
	DMA_InitStruct.DMA_Priority=DMA_Priority_Medium;//优先级
	
	
	DMA_Init(DMA1_Channel6,&DMA_InitStruct);//每个硬件触发都有自己对应的channel号
	DMA_Cmd(DMA1_Channel6,ENABLE);
	
	USART_Cmd(USART2,ENABLE); //开启串口

}


void CameraData_Process(Openmv_DataSet_Typedef* Openmv_DataSet)
{

    //!!!!!!!!!!!!!!!1注意这里变量的排序需要核对
		Openmv_DataSet->Distance=*((float*)CameraData+0);
    Openmv_DataSet->Phi_Error=*((float*)CameraData+1);
    Openmv_DataSet->Theta_Error=*((float*)CameraData+2);
}







void Serial_SendByte(uint8_t Data) //发送一个字节
{
	USART_SendData(USART1,Data); //赋值给DR寄存器 传给发送移位寄存器逐位进行发送
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET); 
	/*
	检测TX寄存器空信号 如果非空 FLAG=RESET 说明存在TX未进入发送移位寄存器
	需要等待FlAG=SET 确保TX已经进入移位寄存器进行发送 防止数据在传输前就被覆盖
	当TX被写入后 FLAG=RESET 被硬件自动清位
	
	此处逻辑即 写入TX寄存器后等待其移位到移位寄存器进行发送 TXE=SET
	进而确保每次写入TX寄存器的时候里面都是空的 不会有数据被覆盖
	*/
}

void Serial_SendArray(uint8_t *Array_Address,uint16_t Length)
{
	uint16_t i;
	
	for(i=0;i<Length;i++)
	{
		Serial_SendByte(*(Array_Address+i));
	}
}

void Serial_SendString(char* String_Address) //传递一个字符类型指针 同时字符串自带截止位'\0'
{
	uint16_t i;
	
	for(i=0;String_Address[i]!='\0';i++) 
	{
		Serial_SendByte(*(String_Address+i));
	}
		
}

uint32_t Pow(uint32_t X,uint32_t Y)
{
	uint32_t Result=1;
	
	while(Y--)
	{
		Result=Result*X;
	}
	
	return Result;
		
}

void Serial_SendNum(uint32_t Number,uint8_t Length)
{
	uint8_t i;
	
	for(i=0;i<Length;i++)
	{
		Serial_SendByte(Number/Pow(10,Length-i-1)%10+'0');
	}
}

//int fputc(int ch,FILE* f)
//{
//	Serial_SendByte(ch); //将printf的方向重定位到串口进行输出
//	return ch;
//}


void Serial_Printf(char *format,...) //可变参数的子函数定义
{
	char String[200];
		
	va_list arg;
	va_start(arg,format);
	vsprintf(String,format,arg);
	va_end(arg);
	Serial_SendString(String);
	
}


void Serial_SendFloat(float* DataSet,uint8_t Length)
{
	  uint8_t Tem[4],i,j; //将浮点数字的4字节内存依次发送

    for(i=0;i<Length;i++)
    {
        for(j=0;j<4;j++)
        {
            Tem[j]=*((uint8_t*)DataSet+4*i+j); //Data浮点数指针转为字节型依次赋值发送
            Serial_SendByte(Tem[j]);
        }

    }
}



