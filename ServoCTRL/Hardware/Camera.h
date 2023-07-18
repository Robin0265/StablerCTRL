#ifndef _CAMERA_SERIAL_H
#define _CAMERA_SERIAL_H
#include "stm32f10x.h"                  // Device header
typedef struct{float Distance;float Phi_Error;float Theta_Error;} Openmv_DataSet_Typedef; 
void MCU_Serial_Init(void);
void Camera_Serial_Init(void);
void CameraData_Process(Openmv_DataSet_Typedef* Openmv_DataSet);
void Serial_SendByte(uint8_t Data);
void Serial_SendArray(uint8_t *Array_Address,uint16_t Length);
void Serial_SendString(char* String_Address);
uint32_t Pow(uint32_t X,uint32_t Y);
void Serial_SendNum(uint32_t Number,uint8_t Length);
void Serial_Printf(char *format,...);
void Serial_SendFloat(float* DataSet,uint8_t Length);
#endif
