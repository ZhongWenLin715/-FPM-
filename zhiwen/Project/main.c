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
#define usart2_baund  115200//����2�����ʣ�����ָ��ģ�鲨���ʸ���

SysPara AS608Para;//ָ��ģ��AS608����
u16 ValidN;//ģ������Чָ�Ƹ���
u8** kbd_tbl;


int main(void)
{    
	u8 ensure;
	char *str;	
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init();  	//��ʼ����ʱ����
	uart_init(115200);	//��ʼ������1������Ϊ115200������֧��USMART
	usart2_init(usart2_baund);//��ʼ������2,������ָ��ģ��ͨѶ
	PS_StaGPIO_Init();	//��ʼ��FR��״̬����
//	BEEP_Init();  			//��ʼ��������
	KEY_Init();					//������ʼ�� 	
	LED_Init();
//	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
//	exfuns_init();			//Ϊfatfs��ر��������ڴ�  
// 	f_mount(fs[1],"1:",1);  //����FLASH.
	
	/*����ָ��ʶ��ʵ�����*/
	printf("\r\nָ��ʶ��ģ����Գ���\r\n");
	printf("\r\nģ������....\r\n");
	while(PS_HandShake(&AS608Addr))//��AS608ģ������
	{
		delay_ms(400);
		printf("\r\nδ��⵽ģ��!!!\r\n");
		delay_ms(800);
		printf("\r\n��������ģ��...\r\n");
	}
	printf("\r\nͨѶ�ɹ�!!!\r\n");
//	str=mymalloc(SRAMIN,30);
//	sprintf(str,"������:%d   ��ַ:%x",usart2_baund,AS608Addr);
	printf("\r\nadd:%x\r\n",AS608Addr);
	ensure=PS_ValidTempleteNum(&ValidN);//����ָ�Ƹ���
	if(ensure!=0x00)
		ShowErrMessage(ensure);//��ʾȷ���������Ϣ	
	ensure=PS_ReadSysPara(&AS608Para);  //������ 
	if(ensure==0x00)
	{
		printf("\r\n��ȡָ�Ƹ�����ȷ\r\n");
//		mymemset(str,0,50);
//		sprintf(str,"������:%d     �Աȵȼ�: %d",AS608Para.PS_max-ValidN,AS608Para.PS_level);
	}
	else
		ShowErrMessage(ensure);	
//	myfree(SRAMIN,str);
	while(1)
	{
		
		if(PS_Sta)	 //���PS_Sta״̬���������ָ����
		{
			press_FR();//ˢָ��			
//			Add_FR();//ע��ָ��
		}
    else {
//			printf("û�а���\r\n");
		}			
	} 	
}

