#ifndef _LQOLED_H
#define _LQOLED_H
#include "sys.h"

#define byte u8
#define word u16

#define LCD_CS  PBout(10)//(CS)
#define LCD_RST PBout(7) //	(RST)
#define LCD_RS  PBout(6)//(DC)
#define LCD_SCLK PBout(9)//(A0)
#define LCD_SDIN PBout(8)//(A1)

 extern byte longqiu96x64[768];
 void LCD_Init(void);
 void LCD_CLS(void);
 void LCD_P6x8Str(byte x,byte y,byte ch[]);
 void LCD_P8x16Str(byte x,byte y,byte ch[]);
 void LCD_P8x16StrInvert(byte x,byte y,byte ch[]);
 void LCD_P14x16Str(byte x,byte y,byte ch[]);
 void LCD_Print(byte x, byte y, byte ch[]);
 void LCD_PrintInvert(byte x, byte y, byte ch[]);
 void LCD_PutPixel(byte x,byte y);
 void Draw_BinarySet(byte x,byte y,u8* video,u8 binary);//设定图像某个点
 void Image_set(byte x,byte y);//设定图像某个点
 void Image_clear();//
 void Draw_MyImage(void);
 void LCD_Rectangle(byte x1,byte y1,byte x2,byte y2,byte gif);
 void Draw_LQLogo(void);
 void Draw_LibLogo(void);
 void Draw_BMP(byte x0,byte y0,byte x1,byte y1,byte bmp[]); 
 void LCD_Fill(byte dat);
 void LCD_P6x8Int(u8 x,u8 y,u8 ch[],int num);
#endif

