#ifndef __1621_H
#define __1621_H	 

#include "sys.h"
#define cs  PEout(15)// MT
#define wr  PEout(14)// 
#define dat PEout(13)// 

#define cs1  PEout(12)// MT
#define wr1  PEout(8)// 
#define dat1 PEout(7)// 

//4路按键
#define KEY1   PEin(2)
#define KEY2   PEin(3)
#define KEY3   PEin(4)
#define KEY4   PEin(5)

#define People_check   PCin(15)

#define voice_busy   PBin(12)

#define CTRL_MQ7 PCout(1)// 
//6路继电器控制
#define switch1  PCout(8)// MT
#define switch2  PCout(7)// 
#define switch3  PCout(6)// 
#define switch4  PDout(15)// MT
#define switch5  PDout(14)// 
#define switch6  PDout(13)// 

#define PM25_LED PAout(12)// 
////#define cs  PDout(3)// MT
////#define wr  PDout(4)// 
////#define dat PDout(5)// 

////#define cs1  PDout(2)// MT
////#define wr1  PDout(1)// 
////#define dat1 PDout(0)//

#define LED1  PDout(6)// 
#define LED2  PDout(7)// 
#define LCD_light  PCout(9)// 

#define BIAS 0x52	
#define SYSEN 0x02
#define LCDON 0x06
#define LCDOFF 0x04



///////////TM1621 1驱动//////////////////////////////////
void SendBit_1621_1(u8 data,u8 cnt);		//data的高cnt位写入HT1621，高位在前
void SendDataBit_1621_1(u8 data,u8 cnt);
void SendCmd_1(u8 command);
void Write_1621_1(u8 addr,u8 data);
void WriteAll_1621_1(u8 addr,u8 *p,u8 cnt);
void OPLCD_1(void);

///////////TM1621 2驱动//////////////////////////////////
void SendBit_1621_2(u8 data,u8 cnt);		//data的高cnt位写入HT1621，高位在
void SendDataBit_1621_2(u8 data,u8 cnt);
void SendCmd_2(u8 command);
void Write_1621_2(u8 addr,u8 data);
void WriteAll_1621_2(u8 addr,u8 *p,u8 cnt);
void OPLCD_2(void);

void LCD_init(void);
void LCD_show(void);

void sensor_data(void);


void send_data_wifi(void);
void send_event(void)	;

void read_data(void);//
void send_MSG(void);
void switch_6_control(void);

void  key_check(void);
void  audio_voice(u8 nub1,u8 nub2);
void  new_audio_voice(void);
void MX500s_test_player(void);
void MX500s_voice_player(void);
void MX500s_test_player(void);
void voice_speed_NUB(u8 temp);
void MX500s_voice_speed_NUB();
void add_voice(void);
void voice_play(void);
void back_light(void);  //500MS
#endif



