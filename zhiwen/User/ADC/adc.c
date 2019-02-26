 #include "adc.h"
 #include "delay.h"
#include "GPIO.h"

__IO uint16_t ADCConvertedValue[L][H];
//ADC 外设的数据寄存器
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
//初始化ADC
//这里我们仅以规则通道为例
//我们默认将开启通道0~3																	   
void  AdcIO_Init(void)
{ 	
//	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1	, ENABLE );	  //使能ADC1通道时钟
 
	
	//PA1 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//PA1 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

		//PC 作为模拟通道输入引脚                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
/*	
	ADC_DeInit(ADC1);  //复位ADC1 

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 10;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

 
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10,1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11,2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12,3, ADC_SampleTime_239Cycles5);
	
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
	
	ADC_DMACmd(ADC1, ENABLE);
	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);   //设置外部触发模式使能（这个“外部“其实仅仅是相
 
	*/
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能

}	





void TIM2_Configuration(void) 
{  
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
	TIM_OCInitTypeDef TIM_OCInitStructure;    
	TIM_TimeBaseStructure.TIM_Period = 10000;//设置100ms一次TIM2比较的周期  
	TIM_TimeBaseStructure.TIM_Prescaler = 719;//系统主频72M，这里分频720，相当于100K的定时器2时钟  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;  
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM2, & TIM_TimeBaseStructure);    
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//下面详细说明  
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//TIM_OutputState_Disable;  
	TIM_OCInitStructure.TIM_Pulse = 5000;  
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;//如果是PWM1要为Low，PWM2则为High  
	TIM_OC2Init(TIM2, & TIM_OCInitStructure);    
	TIM_Cmd(TIM2, ENABLE);    
	TIM_InternalClockConfig(TIM2);  
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);  
	TIM_UpdateDisableConfig(TIM2, DISABLE);  
	TIM_CtrlPWMOutputs(TIM2,ENABLE);    
} 


//获得ADC值
//ch:通道值 0~3
u16 Get_Adc(u8 ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}


u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val=0;
	u16 t;
	for(t=0;t<times;t++)
	{
		temp_val+=Get_Adc(ch);
		delay_us(28);
	}
	return temp_val/times;
} 	 

u16 Adc_valu_Average(u16 times,u8 nub)
{
	u32 temp_val=0;
	u16 t;
	for(t=0;t<times;t++)
	{
		temp_val+=ADCConvertedValue[t][nub];
	}
	return temp_val/times;
}

//===============================================================================================
//ADC1  DMA 配置
void ADC1_DMA_Init(void)
{	
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
//	Adc_Init();
	//外设ADC，DMA时钟开启
	  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  /* Enable ADC1 and GPIOC GPIOA clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC |  RCC_APB2Periph_GPIOB|  RCC_APB2Periph_GPIOA, ENABLE);
//	NVIC_InitTypeDef NVIC_InitStructure;  


/* DMA1 channel1 configuration ----------------------------------------------*/
  DMA_DeInit(DMA1_Channel1); //选择DMA的通道1
  //设定从ADC外设的数据寄存器（ADC1_DR_Address）转移到内存（ADCConcertedValue）
  //每次传输大小16位，使用DMA循环传输模式
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADCConvertedValue;//数据缓冲区的地址
  //外设为数据源
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  //数据缓冲区，大小2半字
  DMA_InitStructure.DMA_BufferSize = H*L;
  // 外设地址固定
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  //内存地址增加，多组adc时，使能，数据传输时，内存增加
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  //半字
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  //DMA循环传输
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  //优先级高
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  //??
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  //执行
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
//  DMA_ITConfig(DMA1_Channel1,DMA_IT_TC, ENABLE);//使能传输完成中断 

//    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);  
//    NVIC_InitStructure.NVIC_IRQChannel =DMA1_Channel1_IRQn;    
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;   
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
//    NVIC_Init(&NVIC_InitStructure);
  /* ADC1 configuration ------------------------------------------------------*/
	
	AdcIO_Init();
  //ADC独立模式	 相对于双重模式
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  //扫描模式用于多通道采集
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  //开启连续转换模式   当转换完本组（可能是一个）继续重新开始执行
  //相对于单次模式：转换一次后就结束
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  //不使用外部触发转换
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;//ADC_ExternalTrigConv_None;
  //采集数据右对齐
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  //转换组的通道数目
  ADC_InitStructure.ADC_NbrOfChannel =H;
  //执行
  ADC_Init(ADC1, &ADC_InitStructure);
  
  //配置ADC时钟，为PCLK2的8分频，即12MHz
  RCC_ADCCLKConfig(RCC_PCLK2_Div6);
  //默认组，adc1 ，通道11，排序为1,55.5周期
//				case 0:channel=10;break;
//				case 1:channel=11;break;
//				case 2:channel=12;break;
//				case 3:channel=13;break;
//				case 4:channel=0;break;
//				case 5:channel=1;break;
//				case 6:channel=2;break;
//				case 7:channel=3;break;
//				case 8:channel=4;break;
//				case 9:channel=5;break;
//				case 10:channel=6;break;
//				case 11:channel=7;break;
//				case 12:channel=14;break;
//				case 13:channel=15;break;
//				case 14:channel=8;break;
//				case 15:channel=9;break;
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_4,1, ADC_SampleTime_239Cycles5);
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_5,2, ADC_SampleTime_239Cycles5);
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_6,3, ADC_SampleTime_239Cycles5);
//ADCin 1到6通道 检测1.5V~2.5V电压
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10,1, ADC_SampleTime_239Cycles5);//PM2.5
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12,2, ADC_SampleTime_239Cycles5); //ADCMQ7 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_13,3, ADC_SampleTime_239Cycles5); //甲醛	
	
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0,13, ADC_SampleTime_239Cycles5);//1
  ADC_RegularChannelConfig(ADC1, ADC_Channel_1,12, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_4,11, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_5,10, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6,9, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7,8, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_14,7, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_15,6, ADC_SampleTime_239Cycles5);//8
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8,5, ADC_SampleTime_239Cycles5);
  ADC_RegularChannelConfig(ADC1, ADC_Channel_9,4, ADC_SampleTime_239Cycles5);//10
	//内部湿度
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16,14, ADC_SampleTime_239Cycles5);
	//内部参考电压
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17,15, ADC_SampleTime_239Cycles5);




  /* Enable ADC1 DMA */
  //使能ADC_DMA
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  //使能ADC
  ADC_Cmd(ADC1, ENABLE);
	 //使能温度传感器和内部参考电压
  ADC_TempSensorVrefintCmd(ENABLE);   
	
  /* Enable ADC1 reset calibration register */ 
  //使能ADC1的复位校准寄存器  
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  //等待校准完成
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  //使能ADC1的开始校准寄存器
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  //等待完成
  while(ADC_GetCalibrationStatus(ADC1));
     
  /* Start ADC1 Software Conversion */ 
  //使用软件触发，由于没有采用外部触发
  //ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  ADC_ExternalTrigConvCmd(ADC1, ENABLE);
}



//中断处理函数  
void  DMA1_Channel1_IRQHandler(void)  
{  
 if(DMA_GetITStatus(DMA1_IT_TC1)!=RESET)
{  
   //自己的中断处理代码 但是记住程序不要太复杂  最好不要超过中断时间  
    DMA_ClearITPendingBit(DMA1_IT_TC1);  
 }  
} 











