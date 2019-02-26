 #include "adc.h"
 #include "delay.h"
#include "GPIO.h"

__IO uint16_t ADCConvertedValue[L][H];
//ADC ��������ݼĴ���
#define ADC1_DR_Address    ((uint32_t)0x4001244C)
//��ʼ��ADC
//�������ǽ��Թ���ͨ��Ϊ��
//����Ĭ�Ͻ�����ͨ��0~3																	   
void  AdcIO_Init(void)
{ 	
//	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_ADC1	, ENABLE );	  //ʹ��ADC1ͨ��ʱ��
 
	
	//PA1 ��Ϊģ��ͨ����������                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	
	//PA1 ��Ϊģ��ͨ����������                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

		//PC ��Ϊģ��ͨ����������                         
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
/*	
	ADC_DeInit(ADC1);  //��λADC1 

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_CC2;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 10;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   

 
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10,1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11,2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12,3, ADC_SampleTime_239Cycles5);
	
	ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����
	
	ADC_DMACmd(ADC1, ENABLE);
	
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);   //�����ⲿ����ģʽʹ�ܣ�������ⲿ����ʵ��������
 
	*/
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������

}	





void TIM2_Configuration(void) 
{  
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
	TIM_OCInitTypeDef TIM_OCInitStructure;    
	TIM_TimeBaseStructure.TIM_Period = 10000;//����100msһ��TIM2�Ƚϵ�����  
	TIM_TimeBaseStructure.TIM_Prescaler = 719;//ϵͳ��Ƶ72M�������Ƶ720���൱��100K�Ķ�ʱ��2ʱ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;  
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
	TIM_TimeBaseInit(TIM2, & TIM_TimeBaseStructure);    
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;//������ϸ˵��  
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;//TIM_OutputState_Disable;  
	TIM_OCInitStructure.TIM_Pulse = 5000;  
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;//�����PWM1ҪΪLow��PWM2��ΪHigh  
	TIM_OC2Init(TIM2, & TIM_OCInitStructure);    
	TIM_Cmd(TIM2, ENABLE);    
	TIM_InternalClockConfig(TIM2);  
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);  
	TIM_UpdateDisableConfig(TIM2, DISABLE);  
	TIM_CtrlPWMOutputs(TIM2,ENABLE);    
} 


//���ADCֵ
//ch:ͨ��ֵ 0~3
u16 Get_Adc(u8 ch)   
{
  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	//�������һ��ADC1�������ת�����
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
//ADC1  DMA ����
void ADC1_DMA_Init(void)
{	
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
//	Adc_Init();
	//����ADC��DMAʱ�ӿ���
	  /* Enable DMA1 clock */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  /* Enable ADC1 and GPIOC GPIOA clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC |  RCC_APB2Periph_GPIOB|  RCC_APB2Periph_GPIOA, ENABLE);
//	NVIC_InitTypeDef NVIC_InitStructure;  


/* DMA1 channel1 configuration ----------------------------------------------*/
  DMA_DeInit(DMA1_Channel1); //ѡ��DMA��ͨ��1
  //�趨��ADC��������ݼĴ�����ADC1_DR_Address��ת�Ƶ��ڴ棨ADCConcertedValue��
  //ÿ�δ����С16λ��ʹ��DMAѭ������ģʽ
  DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADCConvertedValue;//���ݻ������ĵ�ַ
  //����Ϊ����Դ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  //���ݻ���������С2����
  DMA_InitStructure.DMA_BufferSize = H*L;
  // �����ַ�̶�
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  //�ڴ��ַ���ӣ�����adcʱ��ʹ�ܣ����ݴ���ʱ���ڴ�����
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  //����
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  //DMAѭ������
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  //���ȼ���
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  //??
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  //ִ��
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  
  /* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);
  
//  DMA_ITConfig(DMA1_Channel1,DMA_IT_TC, ENABLE);//ʹ�ܴ�������ж� 

//    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);  
//    NVIC_InitStructure.NVIC_IRQChannel =DMA1_Channel1_IRQn;    
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;   
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
//    NVIC_Init(&NVIC_InitStructure);
  /* ADC1 configuration ------------------------------------------------------*/
	
	AdcIO_Init();
  //ADC����ģʽ	 �����˫��ģʽ
  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  //ɨ��ģʽ���ڶ�ͨ���ɼ�
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  //��������ת��ģʽ   ��ת���걾�飨������һ�����������¿�ʼִ��
  //����ڵ���ģʽ��ת��һ�κ�ͽ���
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
  //��ʹ���ⲿ����ת��
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;//ADC_ExternalTrigConv_None;
  //�ɼ������Ҷ���
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  //ת�����ͨ����Ŀ
  ADC_InitStructure.ADC_NbrOfChannel =H;
  //ִ��
  ADC_Init(ADC1, &ADC_InitStructure);
  
  //����ADCʱ�ӣ�ΪPCLK2��8��Ƶ����12MHz
  RCC_ADCCLKConfig(RCC_PCLK2_Div6);
  //Ĭ���飬adc1 ��ͨ��11������Ϊ1,55.5����
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
//ADCin 1��6ͨ�� ���1.5V~2.5V��ѹ
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10,1, ADC_SampleTime_239Cycles5);//PM2.5
  ADC_RegularChannelConfig(ADC1, ADC_Channel_12,2, ADC_SampleTime_239Cycles5); //ADCMQ7 
  ADC_RegularChannelConfig(ADC1, ADC_Channel_13,3, ADC_SampleTime_239Cycles5); //��ȩ	
	
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
	//�ڲ�ʪ��
	ADC_RegularChannelConfig(ADC1, ADC_Channel_16,14, ADC_SampleTime_239Cycles5);
	//�ڲ��ο���ѹ
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17,15, ADC_SampleTime_239Cycles5);




  /* Enable ADC1 DMA */
  //ʹ��ADC_DMA
  ADC_DMACmd(ADC1, ENABLE);
  
  /* Enable ADC1 */
  //ʹ��ADC
  ADC_Cmd(ADC1, ENABLE);
	 //ʹ���¶ȴ��������ڲ��ο���ѹ
  ADC_TempSensorVrefintCmd(ENABLE);   
	
  /* Enable ADC1 reset calibration register */ 
  //ʹ��ADC1�ĸ�λУ׼�Ĵ���  
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  //�ȴ�У׼���
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  //ʹ��ADC1�Ŀ�ʼУ׼�Ĵ���
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  //�ȴ����
  while(ADC_GetCalibrationStatus(ADC1));
     
  /* Start ADC1 Software Conversion */ 
  //ʹ���������������û�в����ⲿ����
  //ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  ADC_ExternalTrigConvCmd(ADC1, ENABLE);
}



//�жϴ�����  
void  DMA1_Channel1_IRQHandler(void)  
{  
 if(DMA_GetITStatus(DMA1_IT_TC1)!=RESET)
{  
   //�Լ����жϴ������ ���Ǽ�ס����Ҫ̫����  ��ò�Ҫ�����ж�ʱ��  
    DMA_ClearITPendingBit(DMA1_IT_TC1);  
 }  
} 











