#ifndef __FLASH_H
#define	__FLASH_H


#include "stm32f10x.h"
#include <stdbool.h>


/* STM32��������Ʒÿҳ��С2KByte���С�С������Ʒÿҳ��С1KByte */
#if defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_CL) || defined (STM32F10X_XL)
  #define FLASH_PAGE_SIZE    ((uint16_t)0x800)	//2048
#else
  #define FLASH_PAGE_SIZE    ((uint16_t)0x400)	//1024
#endif

//д�����ʼ��ַ�������ַ
#define WRITE_START_ADDR  ((uint32_t)0x0803F800)
#define WRITE_END_ADDR    ((uint32_t)0x0803FFFF)

#define CONFIG_DATA_SIZE 12


void LoadConfigData(void);
bool SaveConfigData(void);


#endif /* __FLASH_H */

