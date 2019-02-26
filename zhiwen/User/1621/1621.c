
#include "includes.h"

#define set_bit(x,y) x|=(1<<y) //将X的第Y位置1
#define clr_bit(x,y) x&=~(1<<y) //将X的第Y位清0
u8 save_BUF[80];//0~9存ID  10存数据上传时间  11存注册标志 20~30存开关状态 79存设置ID标志  31~36保存瓦时
//设备开关状态
volatile u8 switch_data[9]={0x01,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x19};//掉电保存  9个16进制数，A~D组各两byte,最后一级E，1byte是6路分控制输出控制
									         //智能插座                               //分控
volatile u8 uptime=1;//掉电保存 定时上传时间 
u8 Device_ID[6]={"123456"};//掉电保存
u8 Register_Ack=0x30;//注册标志

u8 event_flag=0;//事件报警标志
u8 key_flag=0;//一键报警标志
u8 wifi_connet=0;//wifi连接上设备

u8 TX_Buff[20];//MX500s串口发送数据Buff

struct Show_little
{
	u8  temperature;//温度
	u8  temperature_UD;//1为正0为负
	u8  humidity;//湿度
	u8  PM25;//PM2.5
	u8  formaldehyde;//甲醛
	u8  Carbon_monoxide;//一氧化碳
	u8  air_quality;//空气质量等级 1到7
	
	u8  Smart_socket_1_4;//智能插座上部分 01 02 04 08
	u8  Smart_socket_6_9;//智能插座下部分 01 02 04 08
	u8  Smart_socket_5_10;//智能插座下部分 01 02 04 08
	
	u8  control;//分控01 02 04 08 10 20
	u8  Data;//数据
	u8  WIFI;//wifi信号
	u8  voice;//音量
} ;
volatile struct Show_little  Little_view={
	
	25,//温度
	1,//1正摄氏度
	53,//湿度
	100,//PM2.5
	50,
	12,
	5,//空气质量等级
	1,//智能插座上部分 01 02 04 08   1~4
	1,//智能插座下部分 01 02 04 08   6~9
	4,//智能插座下部分 04 08         5和10
	0x58,//分控40 10 08 04 02 01 M6~M1
	
	0x4f,//数据 01 02 04 08 10 20 
	
	0x0f,//wifi信号01 03 07 0f 1f
	0x0f,//音量  0 到4 级  01 02 04 08
};

struct Show_VAWK
{
	u8  VAWK_ck;//电压01 电流02  功03  功率04  显示切换
	u32 V_valu;//后两们来小数位
	u32 A_valu;//后两位为小数位
	u32 A_valu_1;//后两位为小数位
	
	u32 W_valu;//后两位为小数位
	u32 WH_valu;//后两位为小数位
	
	u8  A_alarm;//电流报警
	u8  XL_alarm;//线路报警
	u8  SY_alarm;//水压报警
	u8  KY_alarm;//可燃气报警
	
	u8  AF_alarm;//S11 安防报警
	u8  ZX_alarm;//总线超载
	u8  YW_alarm;//S15 烟雾报警
	u8  FX_alarm;//分线超载
	
	u16 Divid_A[6];//6路分路电流
} ;
volatile struct Show_VAWK  VAWK_view={
	1,//初始化显示电压
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
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
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

//8位校验和
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

//mx500s语音模块指定曲目播放
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


//语音播放
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

//设置成MP3模式
void SetModeMp3(void)
{
	TX_Buff[0]=0x0E;
	TX_Buff[1]=0x01;
	TX_Buff[2]=0x00;
	TX_Buff[3]=0x0F;
	UART3_Send_Data(TX_Buff,(TX_Buff[1]+3));
}

//指定U盘播放
void SendMusicSelSD(void)
{
	TX_Buff[0]=0x0A;
	TX_Buff[1]=0x01;
	TX_Buff[2]=0x01;
	TX_Buff[3]=0x0C;
	UART3_Send_Data(TX_Buff,(TX_Buff[1]+3));
}

//播放
void voice_play(void)
{
	TX_Buff[0]=0x01;
	TX_Buff[1]=0x01;
	TX_Buff[2]=0x00;
	TX_Buff[3]=0x02;
	UART3_Send_Data(TX_Buff,(TX_Buff[1]+3));
}

//音量加最大
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
		audio_voice(0,36);//万
	}
	
	temp_valu[1]=data/100000%10;
	if(temp_valu[1]!=0)
	{
		voice_nub(temp_valu[1]);
		audio_voice(0,12);//千
	}
	else if(temp_valu[0]!=0)
	{
		voice_nub(temp_valu[1]);//0
	}
	
	temp_valu[2]=data/10000%10;
	if(temp_valu[2]!=0)
	{
		voice_nub(temp_valu[2]);
		audio_voice(0,11);//百
	}
	else if(temp_valu[1]!=0)
	{
		voice_nub(temp_valu[2]);//0
	}

	temp_valu[3]=data/1000%10; 
	if(temp_valu[3]!=0)
	{
		voice_nub(temp_valu[3]);
		audio_voice(0,10);//十
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
	//小数位
	temp_valu[5]=data/10%10;
	if(temp_valu[5]!=0)
	{
		audio_voice(0,13);;
		voice_nub(temp_valu[5]);
	}
}

//MX500s数字播报
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
//		audio_voice(0,36);//万
		voice_set_FileNum(37);
	}
	
	temp_valu[1]=data/100000%10;
	if(temp_valu[1]!=0)
	{
		MX500s_voice_nub(temp_valu[1]);
//		audio_voice(0,12);//千
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
//		audio_voice(0,11);//百
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
//		audio_voice(0,10);//十
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
	//小数位
	temp_valu[5]=data/10%10;
	if(temp_valu[5]!=0)
	{
//		audio_voice(0,13);;
		voice_set_FileNum(14);
		MX500s_voice_nub(temp_valu[5]);
	}
}

//MX500s一键播放
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

//一键播放
void voice_player(void)
{
	//温度
	audio_voice(0,16);//当前温度
	voice_data(Little_view.temperature*100);
	audio_voice(0,17);//摄氏度
	//湿度
	audio_voice(0,18);//湿度百分之
	voice_data(Little_view.humidity*100);
	delay_ms(500);
	//甲醛含量
	audio_voice(0,19);//当前甲醛含量为
	voice_data(Little_view.formaldehyde*100);
	audio_voice(0,20);//PPM
	//PM2.5
	audio_voice(0,21);//当前PM2.5含量
	voice_data(Little_view.PM25*100);
	audio_voice(0,22);//立方厘米
	//空气气量
	audio_voice(0,23);//空气质量
	switch(Little_view.air_quality)
	{
		case 6:audio_voice(0,27);//差
		break;
		case 5:
		case 4:audio_voice(0,26);//中
		break;
		case 3:
		case 2:audio_voice(0,25);//良
		break;
		case 1:
		case 0:audio_voice(0,24);//优
		break;
		////////////////////
		default :
			break;
	}
	delay_ms(500);
	//电压
	audio_voice(0,28);//当前电压
	voice_data(VAWK_view.V_valu);
	audio_voice(0,29);//伏
	
	delay_ms(500);
	//电流
	audio_voice(0,30);//电流
	voice_data(VAWK_view.A_valu);
	audio_voice(0,31);//安
	
	delay_ms(500);
	//功率
	audio_voice(0,32);//当前功率
	voice_data(VAWK_view.W_valu);
	audio_voice(0,33);//瓦
	
	//累计用电量
	audio_voice(0,34);//累计用电量
	voice_data(VAWK_view.WH_valu);
	audio_voice(0,35);//千瓦时
}


//新一键播放
void MX500s_voice_player(void)
{
	//温度
//	audio_voice(0,16);//当前温度
	voice_set_FileNum(17);
	MX500s_voice_data(Little_view.temperature*100);
//	audio_voice(0,17);//摄氏度
	voice_set_FileNum(18);
	//湿度
//	audio_voice(0,18);//湿度百分之
	voice_set_FileNum(19);
	MX500s_voice_data(Little_view.humidity*100);
	delay_ms(500);
	//甲醛含量
//	audio_voice(0,19);//当前甲醛含量为
	voice_set_FileNum(20);
	MX500s_voice_data(Little_view.formaldehyde*100);
//	audio_voice(0,20);//PPM
	voice_set_FileNum(21);
	//PM2.5
//	audio_voice(0,21);//当前PM2.5含量
	voice_set_FileNum(22);
	MX500s_voice_data(Little_view.PM25*100);
//	audio_voice(0,22);//立方厘米
  voice_set_FileNum(23);
	//空气气量
//	audio_voice(0,23);//空气质量
  voice_set_FileNum(24);
	switch(Little_view.air_quality)
	{
		case 6:voice_set_FileNum(28);//差
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
	//电压
//	audio_voice(0,28);//当前电压
	voice_set_FileNum(29);
	MX500s_voice_data(VAWK_view.V_valu);
//	audio_voice(0,29);//伏
	voice_set_FileNum(30);
	
	delay_ms(500);
	//电流
//	audio_voice(0,30);//电流
	voice_set_FileNum(31);
	MX500s_voice_data(VAWK_view.A_valu);
//	audio_voice(0,31);//安
	voice_set_FileNum(32);
	
	delay_ms(500);
	//功率
//	audio_voice(0,32);//当前功率
	voice_set_FileNum(33);
	MX500s_voice_data(VAWK_view.W_valu);
//	audio_voice(0,33);//瓦
	voice_set_FileNum(34);
	
	//累计用电量
//	audio_voice(0,34);//累计用电量
	voice_set_FileNum(35);
	MX500s_voice_data(VAWK_view.WH_valu);
//	audio_voice(0,35);//千瓦时
  voice_set_FileNum(36);
}

////////触摸按键//////////////////
void  key_check(void)
{
	if(KEY1==0)//一键呼叫
	{
		delay_ms(10);
		if(KEY1==0)
		{
			key_flag=1;//一键呼叫
			event_flag=10;
		}
	}
	else if(KEY2==0)//一恢复供电
	{
		delay_ms(10);
		if(KEY2==0)
		{
			switch_data[8]=0xff;
		}
	}
	else if(KEY3==0)//一播报
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
///////////TM1621 1驱动//////////////////////////////////
void SendBit_1621_1(u8 data,u8 cnt)		//data的高cnt位写入HT1621，高位在前
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
	SendBit_1621_1(0x80,4);			//写入标志码"100"和9位command命令，由于
	SendBit_1621_1(command,8); 		//没有使有到更改时钟输出等命令，为了编程方便
	cs=1;	delay_us(5);					//直接将command的最高位写"0"
}

void Write_1621_1(u8 addr,u8 data)
{
	cs=0;delay_us(5);
	SendBit_1621_1(0xa0,3);			//写入标志码"101"
	SendBit_1621_1(addr,6);			//写入addr的高6位
	SendDataBit_1621_1(data,4);		//写入data的低4位
	cs=1;delay_us(5);
}

void WriteAll_1621_1(u8 addr,u8 *p,u8 cnt)
{
	u8 i;
	cs=0;delay_us(5);
	SendBit_1621_1(0xa0,3);			//写入标志码"101"
	SendBit_1621_1(addr,6);			//写入addr的高6位
	for(i =0; i <cnt; i ++,p++)		//连续写入数据
	{
		SendDataBit_1621_1(0,8);
	}
	cs=1;delay_us(5);
}


void OPLCD_1(void)
{
	u8 a[10];
	a[0]=0x21;a[1]=0x43;a[2]=0x65;a[3]=0x87;a[4]=0xa9;

	SendCmd_1(BIAS);		//设置偏压和占空比
	SendCmd_1(SYSEN);	//打开系统振荡器
	SendCmd_1(LCDON);	//打开LCD偏压发生器
	Write_1621_1(0x24,0x01);	//0x24：(地址)的高6位有效，0x01：(数据)的低4位有效
	WriteAll_1621_1(0,a,32);	//0：(起始地址)高6位有效，a：(写入数据的起始地址)8位
//都有效，6：为写入的字节数。连续写入"123456789a"
	SendCmd_1(LCDON);	//关闭LCD显示
}

///////////TM1621 2驱动//////////////////////////////////

///////////TM1621 2驱动//////////////////////////////////
void SendBit_2621_2(u8 dat1a,u8 cnt)		//dat1a的高cnt位写入HT1621，高位在前
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


void SendCmd_2(u8 command)//写命令
{
	cs1=0;delay_us(5);
	SendBit_2621_2(0x80,4);			//写入标志码"100"和9位command命令，由于
	SendBit_2621_2(command,8); 		//没有使有到更改时钟输出等命令，为了编程方便
	cs1=1;	delay_us(5);					//直接将command的最高位写"0"
}

void wr1ite_2621_2(u8 addr,u8 data)//写数据
{
	cs1=0;delay_us(5);
	SendBit_2621_2(0xa0,3);			//写入标志码"101"
	SendBit_2621_2(addr,6);			//写入addr的高6位
	Senddat1aBit_2621_2(data,4);		//写入dat1a的低4位
	cs1=1;delay_us(5);
}

void wr1iteAll_2621_2(u8 addr,u8 *p,u8 cnt)
{
	u8 i;
	cs1=0;delay_us(5);
	SendBit_2621_2(0xa0,3);			//写入标志码"101"
	SendBit_2621_2(addr,6);			//写入addr的高6位
	for(i =0; i <cnt; i ++,p++)		//连续写入数据
	{
		Senddat1aBit_2621_2(0,8);
	}
	cs1=1;delay_us(5);
}

void OPLCD_2(void)
{
	u8 a[10];
	a[0]=0x21;a[1]=0x43;a[2]=0x65;a[3]=0x87;a[4]=0xa9;

	SendCmd_2(BIAS);		//设置偏压和占空比
	SendCmd_2(SYSEN);	//打开系统振荡器
	SendCmd_2(LCDON);	//打开LCD偏压发生器
	wr1ite_2621_2(0x24,0x01);	//0x24：(地址)的高6位有效，0x01：(数据)的低4位有效
	wr1iteAll_2621_2(0,a,32);	//0：(起始地址)高6位有效，a：(写入数据的起始地址)8位
//都有效，6：为写入的字节数。连续写入"123456789a"
	SendCmd_2(LCDON);	//关闭LCD显示
}
/////////////////////////////////////////////////////////
void Write_LCD_zhenmian(u8 addr,u8 data,u8 bit)
{
	cs1=0;delay_us(5);
	SendBit_2621_2(0xa0,3);			//写入标志码"101"
	SendBit_2621_2(addr,6);			//写入addr的高6位
	Senddat1aBit_2621_2(data,bit);
	cs1=1;delay_us(5);
}

void Write_LCD_beimian(u8 addr,u8 data,u8 bit)
{
	cs=0;delay_us(5);
	SendBit_1621_1(0xa0,3);			//写入标志码"101"
	SendBit_1621_1(addr,6);			//写入addr的高6位
	SendDataBit_1621_1(data,bit);
	cs=1;delay_us(5);
}


const u8 LED_nub[11]={0xaf,0xa0,0x6d,0xe9,0xe2,0xcb,0xcf,0xa1,0xef,0xeb,0x00}; 


////大号8字显示
void Show_big_LED(void)
{
	u8 temp_valu[6];
	u32 show_NUB=0;//显示数据
	u8 S5,S6,S7,S8;//显示单位
	u8 S_temp=0;
	
	switch(VAWK_view.VAWK_ck)   //显示单位
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
	//整数位
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
	//小数位
	temp_valu[4]=show_NUB/10%10;
	temp_valu[5]=show_NUB%10;
	if(temp_valu[5]==0){temp_valu[5]=10;}
	
	//显示整数位
	Write_LCD_zhenmian(4,LED_nub[temp_valu[0]],8);
	Write_LCD_zhenmian(12,LED_nub[temp_valu[1]],8);
	Write_LCD_zhenmian(20,LED_nub[temp_valu[2]],8);
	Write_LCD_zhenmian(28,LED_nub[temp_valu[3]],8);
	//显示小数位
	Write_LCD_zhenmian(40,LED_nub[temp_valu[4]]|S6,8);
	Write_LCD_zhenmian(48,LED_nub[temp_valu[5]]|S5,8);
	
	//显示小数点 安防,烟雾报警
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
	//显示 功 功率单位  总路 分线超载
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
	
	
//	u8  A_alarm;//电流报警
//	u8  XL_alarm;//线路报警
//	u8  SY_alarm;//水压报警
//	u8  KY_alarm;//可燃气报警
//随便一个出现异常都报警
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
	//温度显示
	temp_valu[0]=Little_view.temperature/10%10;
	if(temp_valu[0]==0){temp_valu[0]=10;}
	temp_valu[1]=Little_view.temperature%10;
	Write_LCD_beimian(72,LED_nub[temp_valu[0]]|0x10,8);
	Write_LCD_beimian(80,LED_nub[temp_valu[1]]|0x10,8);
	if(Little_view.temperature_UD==0)//显示负
	{
		Write_LCD_beimian(88,0x0f,4);
	}
	else
	{
		Write_LCD_beimian(88,0x0b,4);
	}
	
	//湿度显示
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
	
	//甲醛
	temp_valu[0]=Little_view.formaldehyde/10%10;
//	if(temp_valu[0]==0){temp_valu[0]=10;}
	temp_valu[1]=Little_view.formaldehyde%10;
	Write_LCD_beimian(54,LED_nub2[temp_valu[0]]|0x08,8);
	Write_LCD_beimian(46,LED_nub2[temp_valu[1]]|0x08,8);
	
	////空气质量等级
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
	//智能插座
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
	if((Little_view.control&0x01)==0x01)//T5  T10 与分控 M1
	{
		Write_LCD_beimian(101,Little_view.Smart_socket_5_10|0x03,4);
	}
	else
	{
		Write_LCD_beimian(101,Little_view.Smart_socket_5_10|0x02,4);
	}
	
	//分控control
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
	if((Little_view.WIFI&0x10)==0x10)//与wifi最高信号共用
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


///传感器数据采集////////////////////////////////////
//电压检测任务
//温度值(范围:0~50°)
//湿度值(范围:20%~90%)
void Read_DHT11_Data(void)    
{        
 	u8 buf[5];
	u8 i;
	DHT11_Rst();
	if(DHT11_Check()==0)
	{
		for(i=0;i<5;i++)//读取40位数据
		{
			buf[i]=DHT11_Read_Byte();
		}
		if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
		{
			Little_view.temperature=buf[2]-3;//修正
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

	if(DMA_GetFlagStatus(DMA1_FLAG_TC1)!=RESET)	//判断通道4传输完成
	{
		//PM25_LED=0;
		//DMA_Cmd(DMA1_Channel1, DISABLE);
		DMA_ClearFlag(DMA1_FLAG_TC1);//清除通道4传输完成标志		
		
		ADC_TempSensor=Adc_valu_Average(L,H-2);//内部温度
		ADC_Vrefint=Adc_valu_Average(L,H-1)>>2;//内部参考电压
		ADC_Vrefint=12000/ADC_Vrefint;
		
		for(i=0;i<H;i++)
		{
			data[i]=Adc_valu_Average(L,i)>>2;
			//V_valu=((3300*data[i])/1023);
			V_valu=(ADC_Vrefint*data[i])/10;
			//V_valu=(V_valu*1062)/51;//mv
			V[i]=V_valu;//转成MV	
		}

		
		//////总线电压/////////////////////////////////////
	  //	VAWK_view.V_valu=V[0];
		
		if(V[3]<1000)//小于1V 电流为0
		{
			VAWK_view.V_valu=0;
		}
		else if(V[3]>2500)
		{
			VAWK_view.V_valu=24000+((V[3]-2500)*100)/6.25;//1到2.5V代表 0到240V  2.5-1=1.5=1500MA    1500/240=6.25
			VAWK_view.ZX_alarm=1;//总线电压大于240V 报警
		}
		else
		{
			VAWK_view.V_valu=((2500-V[3])*100)/6.25;//1到2.5V代表 0到240V 2.5-1=1.5=1500MA
			VAWK_view.ZX_alarm=0;//总线电压大于240V 报警
		}
		
		//////总线电流1/////////////////////////////////////
		if(V[4]<1000)//小于1V 电流为0
		{
			VAWK_view.A_valu=0;
			VAWK_view.A_alarm=0;//电流报警
		}
		else if(V[4]>2500)
		{
			VAWK_view.A_valu=8000+((V[4]-2500)*100)/18.75;//1到2.5V代表 0到80A  2.5-1=1.5=1500MA  1500/80=18.75
			VAWK_view.A_alarm=1;//电流报警
			event_flag+=1;
		}
		else
		{
			VAWK_view.A_alarm=0;//电流报警
			VAWK_view.A_valu=((2500-V[4])*100)/18.75;//1到2.5V代表 0到80A  2.5-1=1.5=1500MA
		}
		//////总线电流2/////////////////////////////////////
		if(V[5]<1000)//小于1V 电流为0
		{
			VAWK_view.A_valu_1=0;
			VAWK_view.A_alarm=0;//电流报警
		}
		else if(V[5]>2500)
		{
			VAWK_view.A_valu_1=8000+((V[5]-2500)*100)/18.75;//1到2.5V代表 0到80A  2.5-1=1.5=1500MA  1500/80=18.75
			VAWK_view.A_alarm=1;//电流报警
		}
		else
		{
			VAWK_view.A_alarm=0;//电流报警
			VAWK_view.A_valu_1=((2500-V[5])*100)/18.75;//1到2.5V代表 0到80A  2.5-1=1.5=1500MA
		}
		//检测两路电流回路 分1和分2相差大于100MA 则切断所有电源
		if(VAWK_view.A_valu_1>VAWK_view.A_valu)
		{
			if(VAWK_view.A_valu_1-VAWK_view.A_valu>10)//100MA
			{
				switch_data[8]=0x00;
				VAWK_view.A_alarm=1;//电流报警
				event_flag+=1;
			}
		}
		else
		{
			if(VAWK_view.A_valu-VAWK_view.A_valu_1>10)//100MA
			{
				switch_data[8]=0x00;
				VAWK_view.A_alarm=1;//电流报警
				event_flag+=1;
			}
		}
		
	//////6路分路电流2/////////////////////////////////////
		for(i=0;i<6;i++)
		{
				if(V[6+i]<1000)//小于1V 电流为0
				{
					VAWK_view.A_valu_1=0;
					VAWK_view.Divid_A[i]=0;//电流报警
				}
				else if(V[6+i]>2500)
				{
					VAWK_view.Divid_A[i]=1500+((V[6+i]-2500)*100)/100;//1到2.5V代表 0到80A  2.5-1=1.5=1500MA  1500/15=100
					VAWK_view.A_alarm=1;//电流报警
					clr_bit(switch_data[8],i);//大于15A 关闭电源
					
					VAWK_view.FX_alarm=1;//分线报警
					event_flag+=1;
				}
				else
				{
					VAWK_view.A_alarm=0;//电流报警
					VAWK_view.Divid_A[i]=((2500-V[6+i])*100)/100;//1到2.5V代表 0到80A  2.5-1=1.5=1500MA
					if(VAWK_view.Divid_A[i]>1000)
					{
						clr_bit(switch_data[8],i);//大于10A 关闭电源
						VAWK_view.FX_alarm=1;//分线报警
						event_flag+=1;
					}
				}
		}
		//////////////总功
		VAWK_view.W_valu=(VAWK_view.A_valu * VAWK_view.V_valu)/100;
		
	//获取甲醛 国标0.1mg/立方  =0.0746ppm
		if(V[2]>1300)//大于1V
		{
			Little_view.formaldehyde=(V[2]-1300)*100/235;
		}
		else
		{
			Little_view.formaldehyde=0;
		}
		//获取PM25
		Little_view.PM25=V[0]/10;
		if(Little_view.PM25>50)
		{
			VAWK_view.YW_alarm=1;
			event_flag+=1;//烟雾报警
		}
		//一氧化碳
		Little_view.Carbon_monoxide=V[1]/10;
	}
	
	Little_view.air_quality=Little_view.formaldehyde + (Little_view.PM25/20);//空气质量由甲醛(0~99)分7级+PM2.5(0~999)分7级得出
	//读取温湿度值	
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

///6路继电器控制/////////////////////////////////////////////////
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
//{id:"xxxx”, 
//Heartbeat： vv:"xx", iv:" xx", pv:" xx", ev:" xx", tv:" xx", hv:"xx",pmv:"xx",fv:"xx”,
//wpadd:”xx”,badd:”xx”,tadd:”xx”, padd:”xx”, madd:”xx”, oadd:”xx”,sadd:”xx”, 
//eadd:”xx”, buadd:”xx”, uadd:”xx”,value1:”xx” ,value2:”xx”,value3:”xx”,
//value4:”xx” ,value5:”xx” ,value6:”xx”,” ADstate:9个16进制数}

//u8 s[]={"wpadd:\"0\",badd:\"0\",tadd:\"0\",padd:\"0\",madd:\"0\",oadd:\"0\",sadd:\"0\",eadd:\"0\",buadd:\"0\",uadd:\"0\",value1:\"0\",value2:\"0\",value3:\"0\",value4:\"0\",value5:\"0\",value6:\"0\""};
void send_data_wifi(void)
{
	u8 count=0;
	u8 i;
	u32 show_NUB=0;//显示数据
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
		//小数位
		send_wifi_buf[count++]='.';
		send_wifi_buf[count++]=show_NUB/10%10+0x30;
		send_wifi_buf[count++]='"';
		send_wifi_buf[count++]=',';
	}
	
		//温度
	send_wifi_buf[count++]='t';
	send_wifi_buf[count++]='v';
	send_wifi_buf[count++]=':';
	send_wifi_buf[count++]='"';
	
	send_wifi_buf[count++]=Little_view.temperature/10%10+0x30;
	if(send_wifi_buf[count]==0x30){count--;}
	send_wifi_buf[count++]=Little_view.temperature%10+0x30;
	send_wifi_buf[count++]='"';
	send_wifi_buf[count++]=',';
	
	//湿度
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


	
	//甲醛
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
	UART2_Send_Data(switch_data,9);//上传开关状态
	UART2_Send_Data("}",1);
}
	
//{id:”xxxx”, Event：ba:” xx”, la:” xx”, ca:” xx”, sa:” xx”, sma:”xx”, lka:” xx”, apa:” xx”, akca:”xx”}		
//id: 设备id, event:事件上传标记ba: 总线报警 , la: 分线报警, ca:可燃气体， sa: 安防报警， sma: 烟感火警，lka：漏电报警，apa: 水压异常, akca:一键呼叫,
//	u8  A_alarm;//电流报警
//	u8  XL_alarm;//线路报警
//	u8  SY_alarm;//水压报警
//	u8  KY_alarm;//可燃气报警
//	
//	u8  AF_alarm;//S11 安防报警
//	u8  ZX_alarm;//总线超载
//	u8  YW_alarm;//S15 烟雾报警
//	u8  FX_alarm;//分线超载
//事件上传			
void send_event(void)	
{
	u8 count=0;
	u8 send_wifi_buf[250];

	count=0;
	////可燃气报警
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
	
		//总线报警
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
	
	//分线报警
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
	
	
	//可燃气体   无检测
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
	
	
	// 安防报警
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
	
	
	//烟雾报警
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
	
	
	//电流报警
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

	//水压报警 无检测
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
	
//	线路报警 漏电报警
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
	
		//一键呼叫
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
		//计算瓦时
		save_kvalu_time++;
		
		WH_count+=VAWK_view.W_valu/60;//60S计算一次WH 放大10倍
		
		if(save_kvalu_time>10)//10分钟保存一次
		{			
			
			for(i=0;i<9;i++)//保存开关状态
			{
				save_BUF[20+i]=switch_data[i];
			}
			
			if(WH_count>100000)//累计超过KWH
			{
				WH_count=WH_count/100000;//缩小100倍
				
				VAWK_view.WH_valu+=WH_count;//  KWH
				save_BUF[31]=VAWK_view.WH_valu/100000;
				save_BUF[32]=VAWK_view.WH_valu/10000%10;
				save_BUF[33]=VAWK_view.WH_valu/1000%10;
				save_BUF[34]=VAWK_view.WH_valu/100%10;
				save_BUF[35]=VAWK_view.WH_valu/10%10;
				save_BUF[36]=VAWK_view.WH_valu%10;
				WH_count=0;//从新算
			}
			STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//保存数据
			
		}

		send_time_sec=0;
		send_time_min++;
		if(send_time_min>=uptime)//定时上传信息数据
		{
			send_time_min=0;
			send_data_wifi();
		}
	}
	///事件信息上报
	if((event_flag>5)||(key_flag==1))
	{
		if(send_event_time>5)//5秒上传一次
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

/////wifi接收数据处理/////////////////////////////////

void wifi_cmd(void)
{
	u8 uart2_rxbuf[200];     //接收缓冲
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
		STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//保存数据
	}
	else if(strncmp((char const *)(&uart2_rxbuf[0]),(char const *)"Reset all",9)==0)
	{
		for(wait=0;wait<80;wait++)
		{
			save_BUF[wait]=0xff;
		}
		UART2_Send_Data("Reset OK",8);//5

		STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//保存数据
		delay_ms(200);
		NVIC_SystemReset();//系统复位
	}
	else if(strncmp((char const *)(&uart2_rxbuf[0]),(char const *)"{id:\"",5)==0)//帧头
	{
			if(strncmp((char const *)(&uart2_rxbuf[5]),(char const *)Device_ID,6)==0)//设备ID
			{
					wifi_connet=1;
					////////////////////////////////////注册指令
					if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"RegisterAck",11)==0)//数据上传应答 
					{
						Register_Ack=uart2_rxbuf[25];
						save_BUF[11]=Register_Ack;
						delay_ms(5);
						STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//保存数据
					}
					
					if(Register_Ack==0x30)//如果设备没注册 不进行发下操作
					{
						UART2_Send_Data("{id:\"",5);//5
						UART2_Send_Data(&Device_ID[0],6);//6
						UART2_Send_Data(",Device No Register}\r\n}",21);  //count=18
						return;
					}
					
					//全部控制
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
						STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//保存数据
						UART2_Send_Data("{id:\"",5);//5
						UART2_Send_Data(&Device_ID[0],6);//6
						UART2_Send_Data("\",controlack,flag:\"00\",msg:\"success\"}",37);//count=18
					}
					
					else if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"control_singl:",14)==0)//独立控制
					{
//#define set_bit(x,y) x|=(1<<y) //将X的第Y位置1
//#define clr_bit(x,y) x&=~(1<<y) //将X的第Y位清0
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
										if(uart2_rxbuf[32]=='O')//打开
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
										else if(uart2_rxbuf[32]=='C')//关闭
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
										if(uart2_rxbuf[32]=='O')//打开
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
										else if(uart2_rxbuf[32]=='C')//关闭
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
										if(uart2_rxbuf[32]=='O')//打开
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
										else if(uart2_rxbuf[32]=='C')//关闭
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
										if(uart2_rxbuf[32]=='O')//打开
										{
												set_bit(switch_data[8],bit_temp);
										}
										else if(uart2_rxbuf[32]=='C')//关闭
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
								STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//保存数据
								UART2_Send_Data("{id:\"",5);//5
								UART2_Send_Data(&Device_ID[0],6);//6
								UART2_Send_Data("\",control_singlack,flag:\"00\",msg:\"success\"}",43);//count=18
						}
						
					}
					else if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"check",5)==0)//查询
					{
						UART2_Send_Data("{id:\"",5);//5
						UART2_Send_Data(&Device_ID[0],6);//6
						UART2_Send_Data("\",checkack,flag:\"00\",msg:\"",26);//count=18
						UART2_Send_Data(&switch_data[0],9);//6
						UART2_Send_Data("\"}",2);//5
					}
					else if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"uptime",6)==0)//上传时间 设置
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
						STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//保存数据
						
						UART2_Send_Data("{id:\"",5);//5
						UART2_Send_Data(&Device_ID[0],6);//6
						UART2_Send_Data("\",uptimeack,flag:\"00\",msg:\"success\"}",36);//count=18
						
					}
					///////////////////////////////主动上传应答
					else if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"HeartbeatAck",12)==0)//数据上传应答 
					{
						
					}
					else if(strncmp((char const *)(&uart2_rxbuf[13]),(char const *)"EventAck",8)==0)//事件上传应答
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


void USART2_IRQHandler(void)                	//串口1中断服务程序
{
	OSIntEnter();
	wifi_cmd();
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);	
	OSIntExit();	
} 

//u8 save_BUF[80];//0~9存ID  10存数据上传时间  11存注册标志 20~30存开关状态 79存设置ID标志
////设备开关状态
//volatile u8 switch_data[9]={0x01,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x19};//掉电保存
//									//智能插座                               //分控
//volatile u8 uptime=1;//掉电保存 定时上传时间 
//u8 Device_ID[6]={"123456"};//掉电保存
//u8 Register_Ack=0x30;//注册标志

void read_data(void)//
{
	u8 i;
	LED2=1;LED1=1;

	STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//读出数据

	while(save_BUF[79]!=0x31)
	{
		UART2_Send_Data("Set 6S ID Please\r\n",18);
		LED2=0;LED1=0;delay_ms(500);
		LED2=1;LED1=1;delay_ms(500);
		STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)save_BUF,80);//读出数据
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


