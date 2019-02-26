#include "ui.h"
#include "sys.h"
//#include "adc.h"
#include "LQ12864.h"
#include "delay.h"
#include "timer.h"
//#include "24l01.h"
#include "led.h"
#include "key.h"
u8 buttonsPressed=0;
extern u8 set_flag;
extern int up_down,left_right,You_Men;
extern u8 tmp_buf[10];
extern u8 mode_flag,tx_flag,rx_flag;
extern u8 tx_buf[10],rx_buf[10];
extern int pwm_limit;
static const char * const mainMenu[] = {
    "  Drift Launch",
    "1. debug   ",
    "2. play    "
};


u8 Menu_active(char **menuText, u8 numItems)
{
    u8 i;
    static u8 position = 0;
	u8 start_position=1;
    u8 lastPosition = 2;
	u8 listItems=3;
    /*if (numItems > 8){                                              // Screen is 8 lines tall = 1
                                                                    // title line + 7 items max
        numItems = 8;
    }*/
    LCD_Print(0,0, (u8 *)menuText[0]); // Print the title
    buttonsPressed = 0;
    while (!buttonsPressed)                                         // Menu active until selection                                                               // is made
    {
		buttonsPressed=KEY_Scan(0);
        if(buttonsPressed==1)
		{
			if(position>0)
				position--;
			buttonsPressed=0;
		}
		else if(buttonsPressed==2)
		{
			position++;
			buttonsPressed=0;
		}
		else if(buttonsPressed==3)
		{
//			set_flag=0;
			buttonsPressed=0;
			break;
		}
		else if(buttonsPressed==4)
		{
			break;
		}
//		else if(KEY4==buttonsPressed)
//			buttonsPressed=0;
        if (position > numItems){
            position = numItems;
        }
        else if (position == 0){
            position = 1;
        }
				if(position>listItems)
				{
					start_position=position-listItems+1;
				}
				else
				{
					start_position=1;
				}
        if (position != lastPosition)                               // Update position if it is
                                                                    // changed 
        {
            for (i = start_position; i < listItems+start_position; i++)                      // Display menu items
            {
                if (i != position){
                    LCD_Print(0, (i-start_position+1)*2, (u8 *)menuText[i]);
                }
                else {
                    // Highlight item at current position
                    LCD_P8x16StrInvert(0, (i-start_position+1)*2, (u8 *)menuText[i]);
                }
            }
            lastPosition = position;
        }
    }
		if(buttonsPressed!=0)
    	return position;
		else
			return 0xFF;
}

//u8 scan_key(void)
//{
////	int up_down,left_right;
//	u8 key_value=0;
//	up_down=Get_Adc(qh);
//	left_right=Get_Adc(zy);
//	delay_ms(10);//È¥¶¶¶¯ 
//	if(up_down<1800) key_value=1;//up
//	else if(up_down>2200) key_value=2;//down
//	else if(left_right<1800) key_value=3;//left
//	else if(left_right>2200) key_value=4;//right
//	return key_value;
//	
//}

//void Packet_Handle()
//{
//	pwm_limit = You_Men*2/100;
//	LCD_P6x8Int(0,2,"DL:",rx_buf[0]);
//	LCD_P6x8Int(0,4,"UD:",up_down);
//	LCD_P6x8Int(0,6,"LR:",left_right);
//	LCD_P6x8Int(70,2,"YM:",pwm_limit);
//}

//void play_Packet_Handle()
//{
//	pwm_limit = You_Men*2/100;
//	LCD_P6x8Int(70,0,"YM:",pwm_limit);
//	LCD_P6x8Int(0,0,"DL:",rx_buf[0]);
//	tx_buf[0] = pwm_limit;
//		if(up_down<2000)
//		{
////			tx_buf[2] = 1;
//			LCD_P8x16StrInvert(50,2,"GO");
//		}
//		else if(up_down>2100)
//		{
////			tx_buf[2] = 2;
//			LCD_P8x16StrInvert(42,6,"BACK");
//		}
//		else if(left_right<2000)
//		{
//			tx_buf[1] = 165;
//			LCD_P8x16StrInvert(0,4,"LEFT");
//		}
//		else if(left_right>2100)
//		{
//			tx_buf[1] = 108;
//			LCD_P8x16StrInvert(85,4,"RIGHT");
//		}
//		else{
//			tx_buf[1] = 135;
////			tx_buf[2] = 0;
//	    LCD_Print(50,2,"GO");
//	    LCD_Print(0,4,"LEFT");
//	    LCD_Print(85,4,"RIGHT");
//	    LCD_Print(42,6,"BACK");
//		}
//}

//void debug_mode(void)
//{
//	TIM_Cmd(TIM3, DISABLE);
//	LCD_CLS();//
//	NRF24L01_RX_Mode();
//	while(NRF24L01_RxPacket(rx_buf))
//	{
//		LCD_Print(5,0,"connect fail!");
//		LCD_Print(0,2, "try to press");
//		LCD_Print(0,4, "down the reset");
//		LCD_Print(0,6, "key!");
//	}
//	LCD_CLS();
//	mode_flag = 0;
//	TIM_Cmd(TIM3, ENABLE);
//	while(1)
//	{
//		LCD_P8x16StrInvert(25,0,"Test Mode");
//	  if(mode_flag)
//	  {
//		  rx_flag = NRF24L01_RxPacket(rx_buf);
//		  NRF24L01_TX_Mode();
//		  tx_flag = NRF24L01_TxPacket(tx_buf);
//		  NRF24L01_RX_Mode();
//		  Packet_Handle();
//		  mode_flag = 0;
//		  while(!mode_flag);
//	  }
//	}
//  
//}

//void play(void)
//{
////	TIM_Cmd(TIM3, DISABLE);
//	LCD_CLS();//
//	NRF24L01_RX_Mode();
//	while(NRF24L01_RxPacket(rx_buf))
//	{
//		LCD_Print(5,0,"connect fail!");
//		LCD_Print(0,2, "try to press");
//		LCD_Print(0,4, "down the reset");
//		LCD_Print(0,6, "key!");
//	}
//	LCD_CLS();
//	mode_flag = 0;
//	TIM_Cmd(TIM3, ENABLE);
//	while(1)
//	{
////		LCD_P8x16StrInvert(25,0,"Test Mode");
//	  if(mode_flag)
//	  {
//		  rx_flag = NRF24L01_RxPacket(rx_buf);
//		  NRF24L01_TX_Mode();
//		  tx_flag = NRF24L01_TxPacket(tx_buf);
//		  NRF24L01_RX_Mode();
//			You_Men=Get_Adc(fire);
//		  up_down=Get_Adc(qh);
//		  left_right=Get_Adc(zy);
//		  play_Packet_Handle();
//		  mode_flag = 0;
//		  while(!mode_flag);
//	  }
//	}
//}

//void mode_play(void)
//{
//	switch(Menu_active((char **)mainMenu, 2))
//	{
//		case 1: debug_mode();break;
//		case 2: play();break;
//		default:break;
//	}
//}	













