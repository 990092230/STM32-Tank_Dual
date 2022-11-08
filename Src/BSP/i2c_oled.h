#ifndef __I2C_OLED_H
#define	__I2C_OLED_H

#include "stm32f10x.h"

//ͨ������0R����,������0x78��0x7A������ַ -- Ĭ��0x78
#define OLED_ADDRESS	0x78

typedef enum
{
	START = 0,
  SHUTDOWN = 1,
	LOWBAT = 2,
	NOTDRAW = 3,
	RUNNING = 4,
	MPU_INIT = 5,
}Menu;

void I2C2_Init(void);
void I2C2_WriteByte(uint8_t addr,uint8_t data);
void OLED_WriteCmd(unsigned char I2C_Command);
void OLED_WriteData(unsigned char I2C_Data);
void OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y);
void OLED_Fill(unsigned char fill_Data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N);
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
void PlayAnimation(void);
void MenuTask(void *param);
void OLED_ShowParm(void);
void OLED_Reset(void);
#endif
