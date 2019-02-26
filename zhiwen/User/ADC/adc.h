#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"
						  
//变量定义  ADC通道
#define H  15 // 11个ADC通道
#define L  50//ADC采集值

void Adc_Init(void);
u16  Get_Adc(u8 ch); 
u16 Get_Adc_Average(u8 ch,u8 times); 
u16 Adc_valu_Average(u16 times,u8 nub);
void ADC1_DMA_RCC_Configuration(void);
void ADC1_DMA_Init(void);
#endif 


