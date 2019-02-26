
#include "includes.h"

#define set_bit(x,y) x|=(1<<y) //��X�ĵ�Yλ��1
#define clr_bit(x,y) x&=~(1<<y) //��X�ĵ�Yλ��0
u8 save_BUF[80];//0~9��ID  10�������ϴ�ʱ��  11��ע���־ 20~30�濪��״̬ 79������ID��־  31~36������ʱ
//�豸����״̬
volatile u8 switch_data[9]={0x01,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x19};//���籣��  9��16��������A~D�����byte,���һ��E��1byte��6·�ֿ����������
									         //���ܲ���                               //�ֿ�
volatile u8 uptime=1;//���籣�� ��ʱ�ϴ�ʱ�� 
u8 Device_ID[6]={"123456"};//���籣��
u8 Register_Ack=0x30;//ע���־

u8 event_flag=0;//�¼�������־
u8 key_flag=0;//һ��������־
u8 wifi_connet=0;//wifi�������豸

u8 TX_Buff[20];//MX500s���ڷ�������Buff

struct Show_little
{
	u8  temperature;//�¶�
	u8  temperature_UD;//1Ϊ��0Ϊ��
	u8  humidity;//ʪ��
	u8  PM25;//PM2.5
	u8  formaldehyde;//��ȩ
	u8  Carbon_monoxide;//һ����̼
	u8  air_quality;//���������ȼ� 1��7
	
	u8  Smart_socket_1_4;//���ܲ����ϲ��� 01 02 04 08
	u8  Smart_socket_6_9;//���ܲ����²��� 01 02 04 08
	u8  Smart_socket_5_10;//���ܲ����²��� 01 02 04 08
	
	u8  control;//�ֿ�01 02 04 08 10 20
	u8  Data;//����
	u8  WIFI;//wifi�ź�
	u8  voice;//����
} ;
volatile struct Show_little  Little_view={
	
	25,//�¶�
	1,//1�����϶�
	53,//ʪ��
	100,//PM2.5
	50,
	12,
	5,//���������ȼ�
	1,//���ܲ����ϲ��� 01 02 04 08   1~4
	1,//���ܲ����²��� 01 02 04 08   6~9
	4,//���ܲ����²��� 04 08         5��10
	0x58,//�ֿ�40 10 08 04 02 01 M6~M1
	
	0x4f,//���� 01 02 04 08 10 20 
	
	0x0f,//wifi�ź�01 03 07 0f 1f
	0x0f,//����  0 ��4 ��  01 02 04 08
};

struct Show_VAWK
{
	u8  VAWK_ck;//��ѹ01 ����02  ��03  ����04  ��ʾ�л�
	u32 V_valu;//��������С��λ
	u32 A_valu;//����λΪС��λ
	u32 A_valu_1;//����λΪС��λ
	
	u32 W_valu;//����λΪС��λ
	u32 WH_valu;//����λΪС��λ
	
	u8  A_alarm;//��������
	u8  XL_alarm;//��·����
	u8  SY_alarm;//ˮѹ����
	u8  KY_alarm;//��ȼ������
	
	u8  AF_alarm;//S11 ��������
	u8  ZX_alarm;//���߳���
	u8  YW_alarm;//S15 ������
	u8  FX_alarm;//���߳���
	
	u16 Divid_A[6];//6·��·����
} ;
volatile struct Show_VAWK  VAWK_view={
	1,//��ʼ����ʾ��ѹ
	12345,
	33301,
	33301,
	4510,
	6900,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	{0,0,0,0,0,0},
};

u16 count_time=0;
void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
		count_time++;
		if(count_time<32)
		{
			PM25_LED=1;
		}
		else
		{
			PM25_LED=0;
		}
		if(count_time>500)
		{
			count_time=0;
		}	
	}
}

//8λУ���
u8 voice_SumCheck(u8 *Str,u8 len)
{
	u16 xorsum=0;
	u8 i;
	for(i=0;i<len;i++)
	{
		xorsum=xorsum+(*Str++);
	}
	return ((u8)(xorsum&0x00ff));
}

//mx500s����ģ��ָ����Ŀ����
void voice_set_FileNum(u16 FileNum)
{
//	add_voice();
	voice_speed_NUB(100);
	TX_Buff[0]=0x04;
	TX_Buff[1]=0x02;
	TX_Buff[2]=FileNum%256;
	TX_Buff[3]=FileNum/256;
	TX_Buff[4]=voice_SumCheck(TX_Buff,(TX_Buff[1]+2));
//	voice_speed_NUB(100);
	UART3_Send_Data(TX_Buff,(TX_Buff[1]+3));
}


//��������
void voice_speed_NUB(u8 temp)
{
	while(1)
	{
		delay_ms(temp);
		if(voice_busy==0)
		{
			return;
		}
		
	}
}

void MX500s_voice_speed_NUB()
{
	while(1)
	{
		if(voice_busy==0)
		{
			return;
		}
		
	}
}

//���ó�MP3ģʽ
void SetModeMp3(void)
{
	TX_Buff[0]=0x0E;
	TX_Buff[1]=0x01;
	TX_Buff[2]=0x00;
	TX_Buff[3]=0x0F;
	UART3_Send_Data(TX_Buff,(TX_Buff[1]+3));
}

//ָ��U�̲���
void SendMusicSelSD(void)
{
	TX_Buff[0]=0x0A;
	TX_Buff[1]=0x01;
	TX_Buff[2]=0x01;
	TX_Buff[3]=0x0C;
	UART3_Send_Data(TX_Buff,(TX_Buff[1]+3));
}

//����
void voice_play(void)
{
	TX_Buff[0]=0x01;
	TX_Buff[1]=0x01;
	TX_Buff[2]=0x00;
	TX_Buff[3]=0x02;
	UART3_Send_Data(TX_Buff,(TX_Buff[1]+3));
}

//���������
void add_voice(void)
{
//	voice_speed_NUB(100);
	TX_Buff[0]=0x0B;
	TX_Buff[1]=0x01;
	TX_Buff[2]=0x1E;
	TX_Buff[3]=0x2A;
	UART3_Send_Data(TX_Buff,(TX_Buff[1]+3));
}

void audio_voice(u8 nub1,u8 nub2)
{
	u8 temp[7]={0x7e,0x05,0x41,0x00,0x01,0x45,0xef};
	
	voice_speed_NUB(50);
	
	temp[3]=nub1;
	temp[4]=nub2;
	temp[5]=temp[1]^temp[2]^temp[3]^temp[4];
	UART3_Send_Data(temp,7);
}

void new_audio_voice(void)
{
	MX500s_test_player();
}

void voice_nub(u8 temp)
{
	switch(temp)
	{
		case 0:audio_voice(0,0);break;	
		case 1:audio_voice(0,1);break;
		case 2:audio_voice(0,2);break;
		case 3:audio_voice(0,3);break;
		case 4:audio_voice(0,4);break;
		case 5:audio_voice(0,5);break;
		case 6:audio_voice(0,6);break;
		case 7:audio_voice(0,7);break;
		case 8:audio_voice(0,8);break;
		case 9:audio_voice(0,9);break;
		default :break;
	}
}

//MX500s   voice_nub
void MX500s_voice_nub(u8 temp)
{
	switch(temp)
	{
		case 0:voice_set_FileNum(1);break;	
		case 1:voice_set_FileNum(2);break;
		case 2:voice_set_FileNum(3);break;
		case 3:voice_set_FileNum(4);break;
		case 4:voice_set_FileNum(5);break;
		case 5:voice_set_FileNum(6);break;
		case 6:voice_set_FileNum(7);break;
		case 7:voice_set_FileNum(8);break;
		case 8:voice_set_FileNum(9);break;
		case 9:voice_set_FileNum(10);break;
		default :break;
	}
}

void voice_data(u32 data)
{
	u8 temp_valu[6];
	temp_valu[0]=data/1000000;
	if(data==0)
	{
		voice_nub(0);
		return;
	}
	if(temp_valu[0]!=0)
	{
		voice_nub(temp_valu[0]);
		audio_voice(0,36);//��
	}
	
	temp_valu[1]=data/100000%10;
	if(temp_valu[1]!=0)
	{
		voice_nub(temp_valu[1]);
		audio_voice(0,12);//ǧ
	}
	else if(temp_valu[0]!=0)
	{
		voice_nub(temp_valu[1]);//0
	}
	
	temp_valu[2]=data/10000%10;
	if(temp_valu[2]!=0)
	{
		voice_nub(temp_valu[2]);
		audio_voice(0,11);//��
	}
	else if(temp_valu[1]!=0)
	{
		voice_nub(temp_valu[2]);//0
	}

	temp_valu[3]=data/1000%10; 
	if(temp_valu[3]!=0)
	{
		voice_nub(temp_valu[3]);
		audio_voice(0,10);//ʮ
	}
	else if(temp_valu[2]!=0)
	{
		voice_nub(temp_valu[3]);//0
	}
	
	temp_valu[4]=data/100%10;
	if(temp_valu[4]!=0)
	{
		voice_nub(temp_valu[4]);
	}
	//С��λ
	temp_valu[5]=data/10%10;
	if(temp_valu[5]!=0)
	{
		audio_voice(0,13);;
		voice_nub(temp_valu[5]);
	}
}

//MX500s���ֲ���
void MX500s_voice_data(u32 data)
{
	u8 temp_valu[6];
	temp_valu[0]=data/1000000;
	if(data==0)
	{
		MX500s_voice_nub(0);
		return;
	}
	if(temp_valu[0]!=0)
	{
		MX500s_voice_nub(temp_valu[0]);
//		audio_voice(0,36);//��
		voice_set_FileNum(37);
	}
	
	temp_valu[1]=data/100000%10;
	if(temp_valu[1]!=0)
	{
		MX500s_voice_nub(temp_valu[1]);
//		audio_voice(0,12);//ǧ
		voice_set_FileNum(13);
	}
	else if(temp_valu[0]!=0)
	{
		MX500s_voice_nub(temp_valu[1]);//0
	}
	
	temp_valu[2]=data/10000%10;
	if(temp_valu[2]!=0)
	{
		MX500s_voice_nub(temp_valu[2]);
//		audio_voice(0,11);//��
		voice_set_FileNum(12);
	}
	else if(temp_valu[1]!=0)
	{
		MX500s_voice_nub(temp_valu[2]);//0
	}

	temp_valu[3]=data/1000%10; 
	if(temp_valu[3]!=0)
	{
		MX500s_voice_nub(temp_valu[3]);
//		audio_voice(0,10);//ʮ
		voice_set_FileNum(11);
	}
	else if(temp_valu[2]!=0)
	{
		MX500s_voice_nub(temp_valu[3]);//0
	}
	
	temp_valu[4]=data/100%10;
	if(temp_valu[4]!=0)
	{
		MX500s_voice_nub(temp_valu[4]);
	}
	//С��λ
	temp_valu[5]=data/10%10;
	if(temp_valu[5]!=0)
	{
//		audio_voice(0,13);;
		voice_set_FileNum(14);
		MX500s_voice_nub(temp_valu[5]);
	}
}

//MX500sһ������
void MX500s_test_player(void)
{
//	voice_set_FileNum(32);
	voice_set_FileNum(16);
//	voice_set_FileNum(1);
//	voice_set_FileNum(2);
//	voice_set_FileNum(3);
//	voice_set_FileNum(4);
//	voice_set_FileNum(5);
//	voice_set_FileNum(6);
//	voice_set_FileNum(7);
//	voice_set_FileNum(8);
//	voice_set_FileNum(9);
//	voice_set_FileNum(10);
//	voice_set_FileNum(11);
//	voice_set_FileNum(12);
//	voice_set_FileNum(13);
//	delay_ms(1000);
//	voice_set_FileNum(32);
//	delay_ms(1000);
//	voice_set_FileNum(34);
//	voice_set_FileNum(16);
//	voice_set_FileNum(34);
//	delay_ms(500);
//	voice_set_FileNum(2);
//	delay_ms(500);
//	voice_set_FileNum(3);
//	voice_set_FileNum(4);
//	voice_set_FileNum(5);
}

//һ������
void voice_player(void)
{
	//�¶�
	audio_voice(0,16);//��ǰ�¶�
	voice_data(Little_view.temperature*100);
	audio_voice(0,17);//���϶�
	//ʪ��
	audio_voice(0,18);//ʪ�Ȱٷ�֮
	voice_data(Little_view.humidity*100);
	delay_ms(500);
	//��ȩ����
	audio_voice(0,19);//��ǰ��ȩ����Ϊ
	voice_data(Little_view.formaldehyde*100);
	audio_voice(0,20);//PPM
	//PM2.5
	audio_voice(0,21);//��ǰPM2.5����
	voice_data(Little_view.PM25*100);
	audio_voice(0,22);//��������
	//��������
	audio_voice(0,23);//��������
	switch(Little_view.air_quality)
	{
		case 6:audio_voice(0,27);//��
		break;
		case 5:
		case 4:audio_voice(0,26);//��
		break;
		case 3:
		case 2:audio_voice(0,25);//��
		break;
		case 1:
		case 0:audio_voice(0,24);//��
		break;
		////////////////////
		default :
			break;
	}
	delay_ms(500);
	//��ѹ
	audio_voice(0,28);//��ǰ��ѹ
	voice_data(VAWK_view.V_valu);
	audio_voice(0,29);//��
	
	delay_ms(500);
	//����
	audio_voice(0,30);//����
	voice_data(VAWK_view.A_valu);
	audio_voice(0,31);//��
	
	delay_ms(500);
	//����
	audio_voice(0,32);//��ǰ����
	voice_data(VAWK_view.W_valu);
	audio_voice(0,33);//��
	
	//�ۼ��õ���
	audio_voice(0,34);//�ۼ��õ���
	voice_data(VAWK_view.WH_valu);
	audio_voice(0,35);//ǧ��ʱ
}


//��һ������
void MX500s_voice_player(void)
{
	//�¶�
//	audio_voice(0,16);//��ǰ�¶�
	voice_set_FileNum(17);
	MX500s_voice_data(Little_view.temperature*100);
//	audio_voice(0,17);//���϶�
	voice_set_FileNum(18);
	//ʪ��
//	audio_voice(0,18);//ʪ�Ȱٷ�֮
	voice_set_FileNum(19);
	MX500s_voice_data(Little_view.humidity*100);
	delay_ms(500);
	//��ȩ����
//	audio_voice(0,19);//��ǰ��ȩ����Ϊ
	voice_set_FileNum(20);
	MX500s_voice_data(Little_view.formaldehyde*100);
//	audio_voice(0,20);//PPM
	voice_set_FileNum(21);
	//PM2.5
//	audio_voice(0,21);//��ǰPM2.5����
	voice_set_FileNum(22);
	MX500s_voice_data(Little_view.PM25*100);
//	audio_voice(0,22);//��������
  voice_set_FileNum(23);
	//��������
//	audio_voice(0,23);//��������
  voice_set_FileNum(24);
	switch(Little_view.air_quality)
	{
		case 6:voice_set_FileNum(28);//��
		break;
		case 5:
		case 4:voice_set_FileNum(27);
		break;
		case 3:
		case 2:voice_set_FileNum(26);
		break;
		case 1:
		case 0:voice_set_FileNum(25);
		break;
		////////////////////
		default :
			break;
	}
	delay_ms(500);
	//��ѹ
//	audio_voice(0,28);//��ǰ��ѹ
	voice_set_FileNum(29);
	MX500s_voice_data(VAWK_view.V_valu);
//	audio_voice(0,29);//��
	voice_set_FileNum(30);
	
	delay_ms(500);
	//����
//	audio_voice(0,30);//����
	voice_set_FileNum(31);
	MX500s_voice_data(VAWK_view.A_valu);
//	audio_voice(0,31);//��
	voice_set_FileNum(32);
	
	delay_ms(500);
	//����
//	audio_voice(0,32);//��ǰ����
	voice_set_FileNum(33);
	MX500s_voice_data(VAWK_view.W_valu);
//	audio_voice(0,33);//��
	voice_set_FileNum(34);
	
	//�ۼ��õ���
//	audio_voice(0,34);//�ۼ��õ���
	voice_set_FileNum(35);
	MX500s_voice_data(VAWK_view.WH_valu);
//	audio_voice(0,35);//ǧ��ʱ
  voice_set_FileNum(36);
}

////////��������//////////////////
void  key_check(void)
{
	if(KEY1==0)//һ������
	{
		delay_ms(10);
		if(KEY1==0)
		{
			key_flag=1;//һ������
			event_flag=10;
		}
	}
	else if(KEY2==0)//һ�ָ�����
	{
		delay_ms(10);
		if(KEY2==0)
		{
			switch_data[8]=0xff;
		}
	}
	else if(KEY3==0)//һ����
	{
		delay_ms(10);
		if(KEY3==0)
		{
			MX500s_voice_player();
//			voice_set_FileNum(17);
//			MX500s_voice_nub(0);
//				voice_player();
//			while(1)
//			{
//				LED1=1;
//			LED2=1;
//			}
		}
	}
	else if(KEY4==0)
	{
		delay_ms(10);
		if(KEY4==0)
		{
			voice_set_FileNum(32);
//			while(1)
//			{
//				LED1=1;
//			LED2=1;
//			}
		}
	}
}
///////////TM1621 1����//////////////////////////////////
void SendBit_1621_1(u8 data,u8 cnt)		//data�ĸ�cntλд��HT1621����λ��ǰ
{
	u8 i;
	for(i =0; i <cnt; i ++)
	{
		if((data&0x80)==0) dat=0;
		else dat=1;
		wr=0;
		delay_us(5);
		wr=1;delay_us(5);
		data<<=1;
	}
}

void SendDataBit_1621_1(u8 data,u8 cnt)
{
	u8 i;
	for(i =0; i <cnt; i ++)
	{
		if((data&0x01)==0) dat=0;
		else dat=1;
		wr=0;
		delay_us(5);
		wr=1;delay_us(5);
		data>>=1;
	}	
}


void SendCmd_1(u8 command)
{
	cs=0;delay_us(5);
	SendBit_1621_1(0x80,4);			//д���־��"100"��9λcommand�������
	SendBit_1621_1(command,8); 		//û��ʹ�е�����ʱ����������Ϊ�˱�̷���
	cs=1;	delay_us(5);					//ֱ�ӽ�command�����λд"0"
}

void Write_1621_1(u8 addr,u8 data)
{
	cs=0;delay_us(5);
	SendBit_1621_1(0xa0,3);			//д���־��"101"
	SendBit_1621_1(addr,6);			//д��addr�ĸ�6λ
	SendDataBit_1621_1(data,4);		//д��data�ĵ�4λ
	cs=1;delay_us(5);
}

void WriteAll_1621_1(u8 addr,u8 *p,u8 cnt)
{
	u8 i;
	cs=0;delay_us(5);
	SendBit_1621_1(0xa0,3);			//д���־��"101"
	SendBit_1621_1(addr,6);			//д��addr�ĸ�6λ
	for(i =0; i <cnt; i ++,p++)		//����д������
	{
		SendDataBit_1621_1(0,8);
	}
	cs=1;delay_us(5);
}


void OPLCD_1(void)
{
	u8 a[10];
	a[0]=0x21;a[1]=0x43;a[2]=0x65;a[3]=0x87;a[4]=0xa9;

	SendCmd_1(BIAS);		//����ƫѹ��ռ�ձ�
	SendCmd_1(SYSEN);	//��ϵͳ����
	SendCmd_1(LCDON);	//��LCDƫѹ������
	Write_1621_1(0x24,0x01);	//0x24��(��ַ)�ĸ�6λ��Ч��0x01��(����)�ĵ�4λ��Ч
	WriteAll_1621_1(0,a,32);	//0��(��ʼ��ַ)��6λ��Ч��a��(д�����ݵ���ʼ��ַ)8λ
//����Ч��6��Ϊд����ֽ���������д��"123456789a"
	SendCmd_1(LCDON);	//�ر�LCD��ʾ
}

///////////TM1621 2����//////////////////////////////////

///////////TM1621 2����//////////////////////////////////
void SendBit_2621_2(u8 dat1a,u8 cnt)		//dat1a�ĸ�cntλд��HT1621����λ��ǰ
{
	u8 i;
	for(i =0; i <cnt; i ++)
	{
		if((dat1a&0x80)==0) dat1=0;
		else dat1=1;
		wr1=0;
		delay_us(5);
		wr1=1;delay_us(5);
		dat1a<<=1;
	}
}

void Senddat1aBit_2621_2(u8 dat1a,u8 cnt)
{
	u8 i;
	for(i =0; i <cnt; i ++)
	{
		if((dat1a&0x01)==0) dat1=0;
		else dat1=1;
		wr1=0;
		delay_us(5);
		wr1=1;delay_us(5);
		dat1a>>=1;
	}	
}


void SendCmd_2(u8 command)//д����
{
	cs1=0;delay_us(5);
	SendBit_2621_2(0x80,4);			//д���־��"100"��9λcommand�������
	SendBit_2621_2(command,8); 		//û��ʹ�е�����ʱ����������Ϊ�˱�̷���
	cs1=1;	delay_us(5);					//ֱ�ӽ�command�����λд"0"
}

void wr1ite_2621_2(u8 addr,u8 data)//д����
{
	cs1=0;delay_us(5);
	SendBit_2621_2(0xa0,3);			//д���־��"101"
	SendBit_2621_2(addr,6);			//д��addr�ĸ�6λ
	Senddat1aBit_2621_2(data,4);		//д��dat1a�ĵ�4λ
	cs1=1;delay_us(5);
}

void wr1iteAll_2621_2(u8 addr,u8 *p,u8 cnt)
{
	u8 i;
	cs1=0;delay_us(5);
	SendBit_2621_2(0xa0,3);			//д���־��"101"
	SendBit_2621_2(addr,6);			//д��addr�ĸ�6λ
	for(i =0; i <cnt; i ++,p++)		//����д������
	{
		Senddat1aBit_2621_2(0,8);
	}
	cs1=1;delay_us(5);
}

void OPLCD_2(void)
{
	u8 a[10];
	a[0]=0x21;a[1]=0x43;a[2]=0x65;a[3]=0x87;a[4]=0xa9;

	SendCmd_2(BIAS);		//����ƫѹ��ռ�ձ�
	SendCmd_2(SYSEN);	//��ϵͳ����
	SendCmd_2(LCDON);	//��LCDƫѹ������
	wr1ite_2621_2(0x24,0x01);	//0x24��(��ַ)�ĸ�6λ��Ч��0x01��(����)�ĵ�4λ��Ч
	wr1iteAll_2621_2(0,a,32);	//0��(��ʼ��ַ)��6λ��Ч��a��(д�����ݵ���ʼ��ַ)8λ
//����Ч��6��Ϊд����ֽ���������д��"123456789a"
	SendCmd_2(LCDON);	//�ر�LCD��ʾ
}
/////////////////////////////////////////////////////////
void Write_LCD_zhenmian(u8 addr,u8 data,u8 bit)
{
	cs1=0;delay_us(5);
	SendBit_2621_2(0xa0,3);			//д���־��"101"
	SendBit_2621_2(addr,6);			//д��addr�ĸ�6λ
	Senddat1aBit_2621_2(data,bit);
	cs1=1;delay_us(5);
}

void Write_LCD_beimian(u8 addr,u8 data,u8 bit)
{
	cs=0;delay_us(5);
	SendBit_1621_1(0xa0,3);			//д���־��"101"
	SendBit_1621_1(addr,6);			//д��addr�ĸ�6λ
	SendDataBit_1621_1(data,bit);
	cs=1;delay_us(5);
}


const u8 LED_nub[11]={0xaf,0xa0,0x6d,0xe9,0xe2,0xcb,0xcf,0xa1,0xef,0xeb,0x00}; 


////���8����ʾ
void Show_big_LED(void)
{
	u8 temp_valu[6];
	u32 show_NUB=0;//��ʾ����
	u8 S5,S6,S7,S8;//��ʾ��λ
	u8 S_temp=0;
	
	switch(VAWK_view.VAWK_ck)   //��ʾ��λ
	{
		case 1:show_NUB=VAWK_view.V_valu;
					S5=0x10;S6=0;S7=0;S8=0;
					Write_LCD_zhenmian(0,0x01,4);
			break;
		case 2:show_NUB=VAWK_view.A_valu;
					S5=0;S6=0x10;S7=0;S8=0;
					Write_LCD_zhenmian(0,0x02,4);
			break;
		case 3:show_NUB=VAWK_view.W_valu;
					S5=0;S6=0;S7=0x01;S8=0;
					Write_LCD_zhenmian(0,0x04,4);
			break;
		case 4:show_NUB=VAWK_view.WH_valu;
					S5=0;S6=0;S7=0;S8=0x02;
					Write_LCD_zhenmian(0,0x08,4);
			break;
		default :show_NUB=VAWK_view.V_valu;
					S5=1;S6=0;S7=0;S8=0;
					Write_LCD_zhenmian(0,0x01,4);
			break;
	}
	//����λ
	if(show_NUB>1000000)
	{
		show_NUB=show_NUB/10;
	}
	temp_valu[0]=show_NUB/100000;
	if(temp_valu[0]==0){temp_valu[0]=10;}
	temp_valu[1]=show_NUB/10000%10;
	if((temp_valu[0]==10)&&(temp_valu[1]==0)){temp_valu[1]=10;}
	temp_valu[2]=show_NUB/1000%10;
	if((temp_valu[0]==10)&&(temp_valu[1]==10)&&(temp_valu[2]==0)){temp_valu[2]=10;}
	temp_valu[3]=show_NUB/100%10;
	//С��λ
	temp_valu[4]=show_NUB/10%10;
	temp_valu[5]=show_NUB%10;
	if(temp_valu[5]==0){temp_valu[5]=10;}
	
	//��ʾ����λ
	Write_LCD_zhenmian(4,LED_nub[temp_valu[0]],8);
	Write_LCD_zhenmian(12,LED_nub[temp_valu[1]],8);
	Write_LCD_zhenmian(20,LED_nub[temp_valu[2]],8);
	Write_LCD_zhenmian(28,LED_nub[temp_valu[3]],8);
	//��ʾС��λ
	Write_LCD_zhenmian(40,LED_nub[temp_valu[4]]|S6,8);
	Write_LCD_zhenmian(48,LED_nub[temp_valu[5]]|S5,8);
	
	//��ʾС���� ����,������
	if(VAWK_view.AF_alarm==1)
	{
		S_temp|=0x04;//
		event_flag+=1;
	}
	if(VAWK_view.YW_alarm==1)
	{
		S_temp|=0x08;
		event_flag+=1;
	}
	S_temp|=0x02;
	Write_LCD_zhenmian(36,S_temp,4);
	//��ʾ �� ���ʵ�λ  ��· ���߳���
	S_temp=0;
	if(VAWK_view.ZX_alarm==1)
	{
		S_temp|=0x04;//
		event_flag+=1;
	}
	if(VAWK_view.FX_alarm==1)
	{
		S_temp|=0x08;
		event_flag+=1;
	}
	S_temp=S_temp|S7|S8;
	Write_LCD_zhenmian(57,S_temp,4);
	
	
//	u8  A_alarm;//��������
//	u8  XL_alarm;//��·����
//	u8  SY_alarm;//ˮѹ����
//	u8  KY_alarm;//��ȼ������
//���һ�������쳣������
	S_temp=0;
	if(VAWK_view.KY_alarm==1)
	{
		S_temp|=0x01;//
		event_flag+=1;
	}

	if(VAWK_view.SY_alarm==1)
	{
		S_temp|=0x02;
		event_flag+=1;
	}

	if(VAWK_view.XL_alarm==1)
	{
		S_temp|=0x04;//
		event_flag+=1;
	}

	if(VAWK_view.A_alarm==1)
	{
		S_temp|=0x08;
		event_flag+=1;
	}

	Write_LCD_zhenmian(124,S_temp,4);
}



const u8 LED_nub2[11]={0xf5,0x05,0xb6,0x97,0x47,0xd3,0xf3,0x85,0xf7,0xd7,0x00};
void Show_little_LED(void)//
{
	u8 temp_valu[6];
	//�¶���ʾ
	temp_valu[0]=Little_view.temperature/10%10;
	if(temp_valu[0]==0){temp_valu[0]=10;}
	temp_valu[1]=Little_view.temperature%10;
	Write_LCD_beimian(72,LED_nub[temp_valu[0]]|0x10,8);
	Write_LCD_beimian(80,LED_nub[temp_valu[1]]|0x10,8);
	if(Little_view.temperature_UD==0)//��ʾ��
	{
		Write_LCD_beimian(88,0x0f,4);
	}
	else
	{
		Write_LCD_beimian(88,0x0b,4);
	}
	
	//ʪ����ʾ
	temp_valu[0]=Little_view.humidity/10%10;
	if(temp_valu[0]==0){temp_valu[0]=10;}
	temp_valu[1]=Little_view.humidity%10;
	Write_LCD_beimian(37,LED_nub2[temp_valu[0]]|0x08,8);
	Write_LCD_beimian(29,LED_nub2[temp_valu[1]]|0x08,8);

	//PM2.5
	temp_valu[0]=Little_view.PM25/100;
	if(temp_valu[0]==0){temp_valu[0]=10;}
	temp_valu[1]=Little_view.PM25/10%10;
	if((temp_valu[0]==10)&&(temp_valu[1]==0)){temp_valu[1]=10;}
	temp_valu[2]=Little_view.PM25%10;
	Write_LCD_beimian(5,LED_nub[temp_valu[0]]|0x10,8);
	Write_LCD_beimian(13,LED_nub[temp_valu[1]]|0x10,8);
	Write_LCD_beimian(21,LED_nub[temp_valu[2]]|0x10,8);
	
	//��ȩ
	temp_valu[0]=Little_view.formaldehyde/10%10;
//	if(temp_valu[0]==0){temp_valu[0]=10;}
	temp_valu[1]=Little_view.formaldehyde%10;
	Write_LCD_beimian(54,LED_nub2[temp_valu[0]]|0x08,8);
	Write_LCD_beimian(46,LED_nub2[temp_valu[1]]|0x08,8);
	
	////���������ȼ�
	switch(Little_view.air_quality)
	{
		case 6:
						Write_LCD_beimian(60,0x09,4);
						Write_LCD_beimian(64,0x01,4);
						Write_LCD_beimian(68,0x00,4);
		break;
		case 5:
						Write_LCD_beimian(60,0x0b,4);
						Write_LCD_beimian(64,0x02,4);
						Write_LCD_beimian(68,0x00,4);
		break;
		case 4:
						Write_LCD_beimian(60,0x0f,4);
						Write_LCD_beimian(64,0x02,4);
						Write_LCD_beimian(68,0x00,4);//////////////////////
		break;
		case 3:
						Write_LCD_beimian(60,0x0f,4);
						Write_LCD_beimian(64,0x04,4);
						Write_LCD_beimian(68,0x08,4);
		break;
		case 2:
						Write_LCD_beimian(60,0x0f,4);
						Write_LCD_beimian(64,0x04,4);
						Write_LCD_beimian(68,0x0c,4);
		break;
		case 1:
						Write_LCD_beimian(60,0x0f,4);
						Write_LCD_beimian(64,0x08,4);
						Write_LCD_beimian(68,0x0e,4);
		break;
		case 0:
						Write_LCD_beimian(60,0x0f,4);
						Write_LCD_beimian(64,0x08,4);
						Write_LCD_beimian(68,0x0f,4);
		break;
		
		////////////////////
		default :
						Write_LCD_beimian(60,0x09,4);
						Write_LCD_beimian(64,0x01,4);
						Write_LCD_beimian(68,0x00,4);
			break;
	}
	//���ܲ���
	Little_view.Smart_socket_1_4=switch_data[1];
	Little_view.Smart_socket_6_9=switch_data[1]>>5|((switch_data[0]&0x01)<<3);
	if((switch_data[1]&0x10)==0x10)
	{
		Little_view.Smart_socket_5_10=0x04;
	}
	else
	{
		Little_view.Smart_socket_5_10=0x0;
	}
	if((switch_data[0]&0x02)==0x02)
	{
		Little_view.Smart_socket_5_10+=0x08;
	}
	else
	{
		Little_view.Smart_socket_5_10+=0x0;
	}		
	
	Write_LCD_beimian(97,Little_view.Smart_socket_1_4,4);
	Write_LCD_beimian(93,Little_view.Smart_socket_6_9,4);
	if((Little_view.control&0x01)==0x01)//T5  T10 ��ֿ� M1
	{
		Write_LCD_beimian(101,Little_view.Smart_socket_5_10|0x03,4);
	}
	else
	{
		Write_LCD_beimian(101,Little_view.Smart_socket_5_10|0x02,4);
	}
	
	//�ֿ�control
	if((switch_data[8]&0x20)==0x20)
	{
		Little_view.control=switch_data[8]+0x40;
	}
	else
	{
		Little_view.control=switch_data[8];
	}
	Write_LCD_beimian(105,(Little_view.control>>1),4);
	if((Little_view.control&0x40)==0x40)//M6
	{
			Write_LCD_beimian(0,0x0f,4);
	}
	else
	{
		 Write_LCD_beimian(0,0x0B,4);
	}
	
	//data
	Write_LCD_beimian(113,Little_view.Data,4);
	if((Little_view.WIFI&0x10)==0x10)//��wifi����źŹ���
	{
		Write_LCD_beimian(109,(0x09+(Little_view.Data>>4)),4);
	}
	else
	{
		Write_LCD_beimian(109,(0x08+(Little_view.Data>>4)),4);
	}
	
	
	//wifi
	if(wifi_connet==1)
	{
		Write_LCD_beimian(116,Little_view.WIFI,4);
	}
	else
	{
		Write_LCD_beimian(116,0,4);
	}
	//voice

	Write_LCD_beimian(121,Little_view.voice,4);
	
	
}


///���������ݲɼ�////////////////////////////////////
//��ѹ�������
//�¶�ֵ(��Χ:0~50��)
//ʪ��ֵ(��Χ:20%~90%)
void Read_DHT11_Data(void)    
{        
 	u8 buf[5];
	u8 i;
	DHT11_Rst();
	if(DHT11_Check()==0)
	{
		for(i=0;i<5;i++)//��ȡ40λ����
		{
			buf[i]=DHT11_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
		{
			Little_view.temperature=buf[2]-3;//����
			Little_view.humidity=buf[0];
		}
	}  
}
u16 ADC_TempSensor=0;
u16 ADC_Vrefint;
u16 V[16];
u16 data[16];
void sensor_data(void)
{
	u8 i;
	u32 V_valu=0;
	
	CTRL_MQ7=1;	
	DMA_Cmd(DMA1_Channel1, ENABLE);

	if(DMA_GetFlagStatus(DMA1_FLAG_TC1)!=RESET)	//�ж�ͨ��4�������
	{
		//PM25_LED=0;
		//DMA_Cmd(DMA1_Channel1, DISABLE);
		DMA_ClearFlag(DMA1_FLAG_TC1);//���ͨ��4������ɱ�־		
		
		ADC_TempSensor=Adc_valu_Average(L,H-2);//�ڲ��¶�
		ADC_Vrefint=Adc_valu_Average(L,H-1)>>2;//�ڲ��ο���ѹ
		ADC_Vrefint=12000/ADC_Vrefint;
		
		for(i=0;i<H;i++)
		{
			data[i]=Adc_valu_Average(L,i)>>2;
			//V_valu=((3300*data[i])/1023);
			V_valu=(ADC_Vrefint*data[i])/10;
			//V_valu=(V_valu*1062)/51;//mv
			V[i]=V_valu;//ת��MV	
		}

		
		//////���ߵ�ѹ/////////////////////////////////////
	  //	VAWK_view.V_valu=V[0];
		
		if(V[3]<1000)//С��1V ����Ϊ0
		{
			VAWK_view.V_valu=0;
		}
		else if(V[3]>2500)
		{
			VAWK_view.V_valu=24000+((V[3]-2500)*100)/6.25;//1��2.5V���� 0��240V  2.5-1=1.5=1500MA    1500/240=6.25
			VAWK_view.ZX_alarm=1;//���ߵ�ѹ����240V ����
		}
		else
		{
			VAWK_view.V_valu=((2500-V[3])*100)/6.25;//1��2.5V���� 0��240V 2.5-1=1.5=1500MA
			VAWK_view.ZX_alarm=0;//���ߵ�ѹ����240V ����
		}
		
		//////���ߵ���1/////////////////////////////////////
		if(V[4]<1000)//С��1V ����Ϊ0
		{
			VAWK_view.A_valu=0;
			VAWK_view.A_alarm=0;//��������
		}
		else if(V[4]>2500)
		{
			VAWK_view.A_valu=8000+((V[4]-2500)*100)/18.75;//1��2.5V���� 0��80A  2.5-1=1.5=1500MA  1500/80=18.75
			VAWK_view.A_alarm=1;//��������
			event_flag+=1;
		}
		else
		{
			VAWK_view.A_alarm=0;//��������
			VAWK_view.A_valu=((2500-V[4])*100)/18.75;//1��2.5V���� 0��80A  2.5-1=1.5=1500MA
		}
		//////���ߵ���2/////////////////////////////////////
		if(V[5]<1000)//С��1V ����Ϊ0
		{
			VAWK_view.A_valu_1=0;
			VAWK_view.A_alarm=0;//��������
		}
		else if(V[5]>2500)
		{
			VAWK_view.A_valu_1=8000+((V[5]-2500)*100)/18.75;//1��2.5V���� 0��80A  2.5-1=1.5=1500MA  1500/80=18.75
			VAWK_view.A_alarm=1;//��������
		}
		else
		{
			VAWK_view.A_alarm=0;//��������
			VAWK_view.A_valu_1=((2500-V[5])*100)/18.75;//1��2.5V���� 0��80A  2.5-1=1.5=1500MA
		}
		//�����·������· ��1�ͷ�2������100MA ���ж����е�Դ
		if(VAWK_view.A_valu_1>VAWK_view.A_valu)
		{
			if(VAWK_view.A_valu_1-VAWK_view.A_valu>10)//100MA
			{
				switch_data[8]=0x00;
				VAWK_view.A_alarm=1;//��������
				event_flag+=1;
			}
		}
		else
		{
			if(VAWK_view.A_valu-VAWK_view.A_valu_1>10)//100MA
			{
				switch_data[8]=0x00;
				VAWK_view.A_alarm=1;//��������
				event_flag+=1;
			}
		}
		
	//////6·��·����2/////////////////////////////////////
		for(i=0;i<6;i++)
		{
				if(V[6+i]<1000)//С��1V ����Ϊ0
				{
					VAWK_view.A_valu_1=0;
					VAWK_view.Divid_A[i]=0;//��������
				}
				else if(V[6+i]>2500)
				{
					VAWK_view.Divid_A[i]=1500+((V[6+i]-2500)*100)/100;//1��2.5V���� 0��80A  2.5-1=1.5=1500MA  1500/15=100
					VAWK_view.A_alarm=1;//��������
					clr_bit(switch_data[8],i);//����15A �رյ�Դ
					
					VAWK_view.FX_alarm=1;//���߱���
					event_flag+=1;
				}
				else
				{
					VAWK_view.A_alarm=0;//��������
					VAWK_view.Divid_A[i]=((2500-V[6+i])*100)/100;//1��2.5V���� 0��80A  2.5-1=1.5=1500MA
					if(VAWK_view.Divid_A[i]>1000)
					{
						clr_bit(switch_data[8],i);//����10A �رյ�Դ
						VAWK_view.FX_alarm=1;//���߱���
						event_flag+=1;
					}
				}
		}
		//////////////�ܹ�
		VAWK_view.W_valu=(VAWK_view.A_valu * VAWK_view.V_valu)/100;
		
	//��ȡ��ȩ ����0.1mg/����  =0.0746ppm
		if(V[2]>1300)//����1V
		{
			Little_view.formaldehyde=(V[2]-1300)*100/235;
		}
		else
		{
			Little_view.formaldehyde=0;
		}
		//��ȡPM25
		Little_view.PM25=V[0]/10;
		if(Little_view.PM25>50)
		{
			VAWK_view.YW_alarm=1;
			event_flag+=1;//������
		}
		//һ����̼
		Little_view.Carbon_monoxide=V[1]/10;
	}
	
	Little_view.air_quality=Little_view.formaldehyde + (Little_view.PM25/20);//���������ɼ�ȩ(0~99)��7��+PM2.5(0~999)��7���ó�
	//��ȡ��ʪ��ֵ	
	Read_DHT11_Data();
	CTRL_MQ7=0;		
	
}
////////////////////////////////////////
void LCD_init(void)
{
	OPLCD_1();
	OPLCD_2();
	LCD_light=0;
}



void LCD_show(void)
{
		Show_big_LED();
		Show_little_LED();
		VAWK_view.VAWK_ck++;
		if(VAWK_view.VAWK_ck>4)
		{
			VAWK_view.VAWK_ck=1;
		}
//		send_data_wifi();
//		send_event();
}

///6·�̵�������/////////////////////////////////////////////////
void switch_6_control(void)
{
	if((switch_data[8]&0x01)==0x01)
	{
		switch1=1;
	}
	else
	{
		switch1=0;
	}
	
	if((switch_data[8]&0x02)==0x02)
	{
		switch2=1;
	}
	else
	{
		switch2=0;
	}
	
		if((switch_data[8]&0x04)==0x04)
	{
		switch3=1;
	}
	else
	{
		switch3=0;
	}
	
		if((switch_data[8]&0x08)==0x08)
	{
		switch4=1;
	}
	else
	{
		switch4=0;
	}
	
		if((switch_data[8]&0x10)==0x10)
	{
		switch5=1;
	}
	else
	{
		switch5=0;
	}
	
		if((switch_data[8]&0x20)==0x20)
	{
		switch6=1;
	}
	else
	{
		switch6=0;
	}
}
//{id:"xxxx��, 
//Heartbeat�� vv:"xx", iv:" xx", pv:" xx", ev:" xx", tv:" xx", hv:"xx",pmv:"xx",fv:"xx��,
//wpadd:��xx��,badd:��xx��,tadd:��xx��, padd:��xx��, madd:��xx��, oadd:��xx��,sadd:��xx��, 
//eadd:��xx��, buadd:��xx��, uadd:��xx��,value1:��xx�� ,value2:��xx��,value3:��xx��,
//value4:��xx�� ,value5:��xx�� ,value6:��xx��,�� ADstate:9��16������}

//u8 s[]={"wpadd:\"0\",badd:\"0\",tadd:\"0\",padd:\"0\",madd:\"0\",oadd:\"0\",sadd:\"0\",eadd:\"0\",buadd:\"0\",uadd:\"0\",value1:\"0\",value2:\"0\",value3:\"0\",value4:\"0\",value5:\"0\",value6:\"0\""};
void send_data_wifi(void)
{
	u8 count=0;
	u8 i;
	u32 show_NUB=0;//��ʾ����
	u8 buf[6];
	u8 send_wifi_buf[250];
//	strcpy((char *)&send_wifi_buf[0],(char const *)"{id:\"");//5
//	strcpy((char *)&send_wifi_buf[5],(char const *)&Device_ID[0]);//6
//	strcpy((char *)&send_wifi_buf[12],(char const *)"\",Heartbeat:");//count=22
	
	count=0;
	for(i=0;i<4;i++)
	{
		switch(i)
		{
			case 0:show_NUB=VAWK_view.V_valu;
			send_wifi_buf[count++]='v';
			send_wifi_buf[count++]='v';
			send_wifi_buf[count++]=':';
			send_wifi_buf[count++]='"';
			break;
		case 1:show_NUB=VAWK_view.A_valu;
			send_wifi_buf[count++]='i';
			send_wifi_buf[count++]='v';
			send_wifi_buf[count++]=':';
			send_wifi_buf[count++]='"';
			break;
		case 2:show_NUB=VAWK_view.W_valu;
			send_wifi_buf[count++]='p';
			send_wifi_buf[count++]='v';
			send_wifi_buf[count++]=':';
			send_wifi_buf[count++]='"';
			break;
		case 3:show_NUB=VAWK_view.WH_valu;
			send_wifi_buf[count++]='e';
			send_wifi_buf[count++]='v';
			send_wifi_buf[count++]=':';
			send_wifi_buf[count++]='"';
			break;
			
		default :break;	
		}
			
		buf[0]=show_NUB/100000+0x30;
		if(buf[0]==0x30){count=count;}
		else{send_wifi_buf[count++]=buf[0];}
		
		buf[1]=show_NUB/10000%10+0x30;
		if((buf[1]==0x30)&&(buf[1]==0x30)){count=count;}
		else{send_wifi_buf[count++]=buf[1];}
		
		buf[2]=show_NUB/1000%10+0x30;
		if((send_wifi_buf[0]==0x30)&&(send_wifi_buf[1]==0x30)&&(send_wifi_buf[2]==0x30)){count=count;}
		else{send_wifi_buf[count++]=buf[2];}
			
		buf[3]=show_NUB/100%10+0x30;
		send_wifi_buf[count++]=buf[3];
		//С��λ
		send_wifi_buf[count++]='.';
		send_wifi_buf[count++]=show_NUB/10%10+0x30;
		send_wifi_buf[count++]='"';
		send_wifi_buf[count++]=',';
	}
	
		//�¶�
	send_wifi_buf[count++]='t';
	send_wifi_buf[count++]='v';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	
	send_wifi_buf[count++]=Little_view.temperature/10%10+0x30;
	if(send_wifi_buf[count]==0x30){count--;}
	send_wifi_buf[count++]=Little_view.temperature%10+0x30;
	send_wifi_buf[count++]='"';
	send_wifi_buf[count++]=',';
	
	//ʪ��
	send_wifi_buf[count++]='h';
	send_wifi_buf[count++]='v';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	
	send_wifi_buf[count++]=Little_view.humidity/10%10+0x30;
	if(send_wifi_buf[count]==0x30){count--;}
	send_wifi_buf[count++]=Little_view.humidity%10+0x30;
	send_wifi_buf[count++]='"';
	send_wifi_buf[count++]=',';

	//PM2.5
	send_wifi_buf[count++]='p';
	send_wifi_buf[count++]='m';
	send_wifi_buf[count++]='v';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	
	buf[0]=Little_view.PM25/100;
	if(buf[0]!=0){send_wifi_buf[count++]=buf[0]+0x30;}
	buf[1]=Little_view.PM25/10%10;
	if((buf[0]==0)&&(buf[1]==0)){count=count;}
	else{send_wifi_buf[count++]=buf[1]+0x30;}
	send_wifi_buf[count++]=Little_view.PM25%10+0x30;
	send_wifi_buf[count++]='"';
	send_wifi_buf[count++]=',';


	
	//��ȩ
	send_wifi_buf[count++]='f';
	send_wifi_buf[count++]='v';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	send_wifi_buf[count++]=Little_view.formaldehyde/10%10+0x30;
	send_wifi_buf[count++]=Little_view.formaldehyde%10+0x30;
	send_wifi_buf[count++]='"';
	send_wifi_buf[count++]=',';
	
	UART2_Send_Data("{id:\"",5);//5
	UART2_Send_Data(&Device_ID[0],6);//6
	UART2_Send_Data("\",Heartbeat:",12);//count=18
	UART2_Send_Data(&send_wifi_buf[0],count);

	UART2_Send_Data("wpadd:\"0\",badd:\"0\",tadd:\"0\",padd:\"0\",madd:\"0\",oadd:\"0\",sadd:\"0\",eadd:\"0\",buadd:\"0\",uadd:\"0\",value1:\"0\",value2:\"0\",value3:\"0\",value4:\"0\",value5:\"0\",value6:\"0\",",159);
	UART2_Send_Data("ADstate:",8);
	UART2_Send_Data(switch_data,9);//�ϴ�����״̬
	UART2_Send_Data("}",1);
}
	
//{id:��xxxx��, Event��ba:�� xx��, la:�� xx��, ca:�� xx��, sa:�� xx��, sma:��xx��, lka:�� xx��, apa:�� xx��, akca:��xx��}		
//id: �豸id, event:�¼��ϴ����ba: ���߱��� , la: ���߱���, ca:��ȼ���壬 sa: ���������� sma: �̸л𾯣�lka��©�籨����apa: ˮѹ�쳣, akca:һ������,
//	u8  A_alarm;//��������
//	u8  XL_alarm;//��·����
//	u8  SY_alarm;//ˮѹ����
//	u8  KY_alarm;//��ȼ������
//	
//	u8  AF_alarm;//S11 ��������
//	u8  ZX_alarm;//���߳���
//	u8  YW_alarm;//S15 ������
//	u8  FX_alarm;//���߳���
//�¼��ϴ�			
void send_event(void)	
{
	u8 count=0;
	u8 send_wifi_buf[250];

	count=0;
	////��ȼ������
	send_wifi_buf[count++]='b';
	send_wifi_buf[count++]='a';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	if(VAWK_view.KY_alarm==1)
	{
		send_wifi_buf[count++]='1';
	}
	else
	{
		send_wifi_buf[count++]='0';
	}
	send_wifi_buf[count++]='"';
	
		//���߱���
	send_wifi_buf[count++]='b';
	send_wifi_buf[count++]='a';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	if(VAWK_view.ZX_alarm==1)
	{
		send_wifi_buf[count++]='1';
	}
	else
	{
		send_wifi_buf[count++]='0';
	}
	send_wifi_buf[count++]='"';
	
	//���߱���
	send_wifi_buf[count++]='l';
	send_wifi_buf[count++]='a';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	if(VAWK_view.FX_alarm==1)
	{
		send_wifi_buf[count++]='1';
	}
	else
	{
		send_wifi_buf[count++]='0';
	}
	send_wifi_buf[count++]='"';
	
	
	//��ȼ����   �޼��
	send_wifi_buf[count++]='c';
	send_wifi_buf[count++]='a';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
//	if(VAWK_view.FX_alarm==1)
//	{
//		send_wifi_buf[count++]='1';
//	}
//	else
//	{
		send_wifi_buf[count++]='0';
//	}
	send_wifi_buf[count++]='"';
	
	
	// ��������
	send_wifi_buf[count++]='s';
	send_wifi_buf[count++]='a';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	if(VAWK_view.AF_alarm==1)
	{
		send_wifi_buf[count++]='1';
	}
	else
	{
		send_wifi_buf[count++]='0';
	}
	send_wifi_buf[count++]='"';
	
	
	//������
	send_wifi_buf[count++]='s';
	send_wifi_buf[count++]='m';
	send_wifi_buf[count++]='a';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	if(VAWK_view.YW_alarm==1)
	{
		send_wifi_buf[count++]='1';
	}
	else
	{
		send_wifi_buf[count++]='0';
	}
	send_wifi_buf[count++]='"';
	
	
	//��������
	send_wifi_buf[count++]='l';
	send_wifi_buf[count++]='k';
	send_wifi_buf[count++]='a';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	if(VAWK_view.A_alarm==1)
	{
		send_wifi_buf[count++]='1';
	}
	else
	{
		send_wifi_buf[count++]='0';
	}
	send_wifi_buf[count++]='"';

	//ˮѹ���� �޼��
	send_wifi_buf[count++]='a';
	send_wifi_buf[count++]='p';
	send_wifi_buf[count++]='a';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	if(VAWK_view.SY_alarm==1)
	{
		send_wifi_buf[count++]='1';
	}
	else
	{
		send_wifi_buf[count++]='0';
	}
	send_wifi_buf[count++]='"';
	
//	��·���� ©�籨��
//	send_wifi_buf[count++]='c';
//	send_wifi_buf[count++]='a';
//	send_wifi_buf[count++]=':';
//	send_wifi_buf[count++]='"';
//	if(VAWK_view.XL_alarm==1)
//	{
//		send_wifi_buf[count++]='1';
//	}
//	else
//	{
//		send_wifi_buf[count++]='0';
//	}
//	send_wifi_buf[count++]='"';
	
		//һ������
	send_wifi_buf[count++]='a';
	send_wifi_buf[count++]='c';
	send_wifi_buf[count++]='k';
	send_wifi_buf[count++]='a';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	if(key_flag==1)
	{
		send_wifi_buf[count++]='1';
	}
	else
	{
		send_wifi_buf[count++]='0';
	}
	send_wifi_buf[count++]='"';
	
	UART2_Send_Data("{id:\"",5);//5
	UART2_Send_Data(&Device_ID[0],6);//6
	UART2_Send_Data("\",Event:",8);//count=18
	UART2_Send_Data(&send_wifi_buf[0],count);
	UART2_Send_Data("}",1);
}



u8 send_time_sec=0;
u8 send_time_min=0;
u8 save_kvalu_time=0;
u8 send_event_time=0;
u32 WH_count=0;
void send_MSG(void)
{
	u8 i;
	send_time_sec++;
	send_event_time++;
	if(send_time_sec>60)
	{
		//������ʱ
		save_kvalu_time++;
		
		WH_count+=VAWK_view.W_valu/60;//60S����һ��WH �Ŵ�10��
		
		if(save_kvalu_time>10)//10���ӱ���һ��
		{			
			
			for(i=0;i<9;i++)//���濪��״̬
			{
				save_BUF[20+i]=switch_data[i];
			}
			
			if(WH_count>100000)//�ۼƳ���KWH
			{
				WH_count=WH_count/100000;//��С100��
				
				VAWK_view.WH_valu+=WH_count;//  KWH
				save_BUF[31]=VAWK_view.WH_valu/100000;
				save_BUF[32]=VAWK_view.WH_valu/10000%10;
				save_BUF[33]=VAWK_view.WH_valu/1000%10;
				save_BUF[34]=VAWK_view.WH_valu/100%10;
				save_BUF[35]=VAWK_view.WH_valu/10%10;
				save_BUF[36]=VAWK_view.WH_valu%10;
				WH_count=0;//������
			}
			STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//��������
			
		}

		send_time_sec=0;
		send_time_min++;
		if(send_time_min>=uptime)//��ʱ�ϴ���Ϣ����
		{
			send_time_min=0;
			send_data_wifi();
		}
	}
	///�¼���Ϣ�ϱ�
	if((event_flag>5)||(key_flag==1))
	{
		if(send_event_time>5)//5���ϴ�һ��
		{
			send_event_time=0;
			send_event();
		}
	}
	if(Register_Ack==0x30)
	{
		UART2_Send_Data("{id:\"",5);//5
		UART2_Send_Data(&Device_ID[0],6);//6
		UART2_Send_Data(",Register}",10);//count=18
	}
}

/////wifi�������ݴ���/////////////////////////////////

void wifi_cmd(void)
{
	u8 uart2_rxbuf[200];     //���ջ���
	volatile u8 temp=0;
	u16 wait=0;
	u16 uart2_count=0;	
	u8 bit_temp;
	    
	if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET)
	{
		USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);	        
		while(wait++ < 5536)	   			    
		{
			if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE))						    
			{
			    USART_ClearITPendingBit(USART2,USART_IT_RXNE);	       	
			    uart2_rxbuf[uart2_count++]=USART_ReceiveData(USART2); 	
			    wait=0;					
			}	
		}
	} 
	if(strncmp((char const *)(&uart2_rxbuf[0]),(char const *)"Set ID:",7)==0)
	{
		for(wait=0;wait<6;wait++)
		{
			Device_ID[wait]=uart2_rxbuf[wait+7];
			save_BUF[wait]=Device_ID[wait];
			
		}
		save_BUF[79]=0x31;
		UART2_Send_Data("SetID OK,ID is:",15);//5
		UART2_Send_Data(&Device_ID[0],6);//5
		STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//��������
	}
	else if(strncmp((char const *)(&uart2_rxbuf[0]),(char const *)"Reset all",9)==0)
	{
		for(wait=0;wait<80;wait++)
		{
			save_BUF[wait]=0xff;
		}
		UART2_Send_Data("Reset OK",8);//5

		STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//��������
		delay_ms(200);
		NVIC_SystemReset();//ϵͳ��λ
	}
	else if(strncmp((char const *)(&uart2_rxbuf[0]),(char const *)"{id:\"",5)==0)//֡ͷ
	{
			if(strncmp((char const *)(&uart2_rxbuf[5]),(char const *)Device_ID,6)==0)//�豸ID
			{
					wifi_connet=1;
					////////////////////////////////////ע��ָ��
					if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"RegisterAck",11)==0)//�����ϴ�Ӧ�� 
					{
						Register_Ack=uart2_rxbuf[25];
						save_BUF[11]=Register_Ack;
						delay_ms(5);
						STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//��������
					}
					
					if(Register_Ack==0x30)//����豸ûע�� �����з��²���
					{
						UART2_Send_Data("{id:\"",5);//5
						UART2_Send_Data(&Device_ID[0],6);//6
						UART2_Send_Data(",Device No Register}\r\n}",21);  //count=18
						return;
					}
					
					//ȫ������
					if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"control:",8)==0)
					{
						for(wait=0;wait<9;wait++)
						{
							switch_data[wait]=uart2_rxbuf[21+wait];
						}	
						
						for(wait=0;wait<9;wait++)
						{
							save_BUF[20+wait]=switch_data[wait];
						}
						delay_ms(10);
						STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//��������
						UART2_Send_Data("{id:\"",5);//5
						UART2_Send_Data(&Device_ID[0],6);//6
						UART2_Send_Data("\",controlack,flag:\"00\",msg:\"success\"}",37);//count=18
					}
					
					else if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"control_singl:",14)==0)//��������
					{
//#define set_bit(x,y) x|=(1<<y) //��X�ĵ�Yλ��1
//#define clr_bit(x,y) x&=~(1<<y) //��X�ĵ�Yλ��0
						bit_temp=(uart2_rxbuf[29]-0x30)*10+(uart2_rxbuf[30]-0x30);
						if(bit_temp<1)
						{
							UART2_Send_Data("{id:\"",5);//5
							UART2_Send_Data(&Device_ID[0],6);//6
							UART2_Send_Data("\",control_singlack,flag:\"01\",msg:\"data error\"}",46);//count=18
						}
						else if((uart2_rxbuf[32]=='O')||(uart2_rxbuf[32]=='C'))
						{
								bit_temp-=1;
								switch(uart2_rxbuf[27])
								{
									case 'A':
										if(uart2_rxbuf[32]=='O')//��
										{
											if(bit_temp<9)
											{
												set_bit(switch_data[1],bit_temp);
											}
											else
											{
												set_bit(switch_data[0],bit_temp-8);
											}
										}
										else if(uart2_rxbuf[32]=='C')//�ر�
										{
											if(bit_temp<9)
											{
												clr_bit(switch_data[1],bit_temp);
											}
											else
											{
												clr_bit(switch_data[0],bit_temp-8);
											}
										}
										
									break;
									case 'B':
										if(uart2_rxbuf[32]=='O')//��
										{
											if(bit_temp<9)
											{
												set_bit(switch_data[3],bit_temp);
											}
											else
											{
												set_bit(switch_data[2],bit_temp-8);
											}
										}
										else if(uart2_rxbuf[32]=='C')//�ر�
										{
											if(bit_temp<9)
											{
												clr_bit(switch_data[3],bit_temp);
											}
											else
											{
												clr_bit(switch_data[2],bit_temp-8);
											}
										}
									break;
									case 'C':
										if(uart2_rxbuf[32]=='O')//��
										{
											if(bit_temp<9)
											{
												set_bit(switch_data[5],bit_temp);
											}
											else
											{
												set_bit(switch_data[4],bit_temp-8);
											}
										}
										else if(uart2_rxbuf[32]=='C')//�ر�
										{
											if(bit_temp<9)
											{
												clr_bit(switch_data[5],bit_temp);
											}
											else
											{
												clr_bit(switch_data[4],bit_temp-8);
											}
										}
									break;
									case 'D':
										switch_data[6]=uart2_rxbuf[29];
										switch_data[7]=uart2_rxbuf[30];
									
									break;
									case 'E':
										if(uart2_rxbuf[32]=='O')//��
										{
												set_bit(switch_data[8],bit_temp);
										}
										else if(uart2_rxbuf[32]=='C')//�ر�
										{
												clr_bit(switch_data[8],bit_temp);
										}
									break;
									default:break;
								}
									for(wait=0;wait<9;wait++)
									{
										save_BUF[20+wait]=switch_data[wait];
									}
								delay_ms(10);
								STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//��������
								UART2_Send_Data("{id:\"",5);//5
								UART2_Send_Data(&Device_ID[0],6);//6
								UART2_Send_Data("\",control_singlack,flag:\"00\",msg:\"success\"}",43);//count=18
						}
						
					}
					else if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"check",5)==0)//��ѯ
					{
						UART2_Send_Data("{id:\"",5);//5
						UART2_Send_Data(&Device_ID[0],6);//6
						UART2_Send_Data("\",checkack,flag:\"00\",msg:\"",26);//count=18
						UART2_Send_Data(&switch_data[0],9);//6
						UART2_Send_Data("\"}",2);//5
					}
					else if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"uptime",6)==0)//�ϴ�ʱ�� ����
					{
						for(wait=21;wait<30;wait++)
						{
							if(uart2_rxbuf[wait]=='"')
							{
								bit_temp=wait-21;
								wait=40;
							
								if(bit_temp==3)
								{
									uptime=(uart2_rxbuf[21]-0x30)*100+(uart2_rxbuf[22]-0x30)*10+(uart2_rxbuf[23]-0x30);
								}
								else if(bit_temp==2)
								{
									uptime=(uart2_rxbuf[21]-0x30)*10+(uart2_rxbuf[22]-0x30);
								}
								else if(bit_temp==1)
								{
									uptime=uart2_rxbuf[21]-0x30;
								}
								else
								{
									UART2_Send_Data("{id:\"",5);//5
									UART2_Send_Data(&Device_ID[0],6);//6
									UART2_Send_Data("\",uptimeack,flag:\"01\",msg:\"Unlawful data\"}",42);//count=18
									return;
								}
							}
						}
						save_BUF[10]=uptime;
						delay_ms(5);
						STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//��������
						
						UART2_Send_Data("{id:\"",5);//5
						UART2_Send_Data(&Device_ID[0],6);//6
						UART2_Send_Data("\",uptimeack,flag:\"00\",msg:\"success\"}",36);//count=18
						
					}
					///////////////////////////////�����ϴ�Ӧ��
					else if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"HeartbeatAck",12)==0)//�����ϴ�Ӧ�� 
					{
						
					}
					else if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"EventAck",8)==0)//�¼��ϴ�Ӧ��
					{
						
					}

			}
	}
//	UART1_Send_Data(&uart1_rxbuf[0],uart1_count);  											 
} 

void USART1_IRQHandler(void)
{
	u16 wait;
	OSIntEnter();
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET)
	{
		USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);	        
		while(wait++ < 5536)	   			    
		{
			if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE))						    
			{
			    USART_ClearITPendingBit(USART1,USART_IT_RXNE);	       	
				
			}	
		}
	} 
	OSIntExit();
}


void USART2_IRQHandler(void)                	//����1�жϷ������
{
	OSIntEnter();
	wifi_cmd();
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	
	OSIntExit();	
} 

//u8 save_BUF[80];//0~9��ID  10�������ϴ�ʱ��  11��ע���־ 20~30�濪��״̬ 79������ID��־
////�豸����״̬
//volatile u8 switch_data[9]={0x01,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x19};//���籣��
//									//���ܲ���                               //�ֿ�
//volatile u8 uptime=1;//���籣�� ��ʱ�ϴ�ʱ�� 
//u8 Device_ID[6]={"123456"};//���籣��
//u8 Register_Ack=0x30;//ע���־

void read_data(void)//
{
	u8 i;
	LED2=1;LED1=1;

	STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//��������

	while(save_BUF[79]!=0x31)
	{
		UART2_Send_Data("Set 6S ID Please\r\n",18);
		LED2=0;LED1=0;delay_ms(500);
		LED2=1;LED1=1;delay_ms(500);
		STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//��������
		LED2=0;LED1=0;delay_ms(500);
		LED2=1;LED1=1;delay_ms(500);
	}
	for(i=0;i<6;i++)
	{
		Device_ID[i]=save_BUF[i];
	}
	uptime=save_BUF[10];
	if(uptime==0xff)
	{
		uptime=1;
	}
	Register_Ack=save_BUF[11];
	if(Register_Ack==0xff)
	{
		Register_Ack=0x30;
	}
	for(i=0;i<9;i++)
	{
		switch_data[i]=save_BUF[20+i];
		if(switch_data[i]==0xff)
		{
			switch_data[i]=0;
		}
	}
	if(save_BUF[31]==0xff)
	{
		VAWK_view.WH_valu=0;
	}
	else
	{
		VAWK_view.WH_valu=save_BUF[31]*100000+save_BUF[32]*10000+save_BUF[33]*1000+save_BUF[34]*100+save_BUF[35]*10+save_BUF[36];
	}
}

u8 back_time=0;
void back_light(void)  //500MS
{
		if(back_time>1)
		{
			back_time--;
			return;
		}
		if(People_check==1)
		{
			LCD_light=0;
			back_time=60;//30s
		}
		else
		{
			LCD_light=1;
		}

}


