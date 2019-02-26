#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "BF5325A.h"
//#include "lcd.h"
#include "key.h"
//#include "beep.h"	  
//#include "malloc.h"   
//#include "sdio_sdcard.h"       
//#include "ff.h"  
//#include "exfuns.h"    
//#include "AS608.h"
//#include "timer.h"
#include "led.h"
#define usart2_baund  115200//串口2波特率，根据指纹模块波特率更改

SysPara AS608Para;//指纹模块AS608参数
u16 ValidN;//模块内有效指纹个数
u8** kbd_tbl;


int main(void)
{    
	u8 ensure;
	char *str;	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init();  	//初始化延时函数
	uart_init(115200);	//初始化串口1波特率为115200，用于支持USMART
	usart2_init(usart2_baund);//初始化串口2,用于与指纹模块通讯
	PS_StaGPIO_Init();	//初始化FR读状态引脚
//	BEEP_Init();  			//初始化蜂鸣器
	KEY_Init();					//按键初始化 	
	LED_Init();
//	my_mem_init(SRAMIN);		//初始化内部内存池 
//	exfuns_init();			//为fatfs相关变量申请内存  
// 	f_mount(fs[1],"1:",1);  //挂载FLASH.
	
	/*加载指纹识别实验界面*/
	printf("\r\n指纹识别模块测试程序\r\n");
	printf("\r\n模块握手....\r\n");
	while(PS_HandShake(&AS608Addr))//与AS608模块握手
	{
		delay_ms(400);
		printf("\r\n未检测到模块!!!\r\n");
		delay_ms(800);
		printf("\r\n尝试连接模块...\r\n");
	}
	printf("\r\n通讯成功!!!\r\n");
//	str=mymalloc(SRAMIN,30);
//	sprintf(str,"波特率:%d   地址:%x",usart2_baund,AS608Addr);
	printf("\r\nadd:%x\r\n",AS608Addr);
	ensure=PS_ValidTempleteNum(&ValidN);//读库指纹个数
	if(ensure!=0x00)
		ShowErrMessage(ensure);//显示确认码错误信息	
	ensure=PS_ReadSysPara(&AS608Para);  //读参数 
	if(ensure==0x00)
	{
		printf("\r\n读取指纹个数正确\r\n");
//		mymemset(str,0,50);
//		sprintf(str,"库容量:%d     对比等级: %d",AS608Para.PS_max-ValidN,AS608Para.PS_level);
	}
	else
		ShowErrMessage(ensure);	
//	myfree(SRAMIN,str);
	while(1)
	{
		
		if(PS_Sta)	 //检测PS_Sta状态，如果有手指按下
		{
			press_FR();//刷指纹			
//			Add_FR();//注册指纹
		}
    else {
//			printf("没有按下\r\n");
		}			
	} 	
}

