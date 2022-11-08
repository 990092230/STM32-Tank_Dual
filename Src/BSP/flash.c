/**
  ******************************************************************************
  * @file    flash.c
  * @author  Oskar Wei
  * @version V1.0
  * @date    2020-08-23
  * @brief   embedded flash
  ******************************************************************************
  * @attention
  *
  * Mail: 990092230@qq.com
  * Shop: www.mindsilicon.com
  *
	* �ó������ѧϰʹ�ã�δ�����������������������κ���;
  ******************************************************************************
  */


#include "flash.h"
#include "radio.h"
#include "key.h"
#include "utils.h"
#include <stdio.h>

// ϵͳ����
extern systemConfig_t configParam;
extern systemConfig_t configParamDefault;


/**
  * @brief  Load configurations from flash memory.
  * @param  None
  * @retval bool
  */
void LoadConfigData(void)
{
	uint32_t address = 0x00;				//��¼д��ĵ�ַ
	uint32_t data = 0x0;						//��¼д�������
	uint8_t i= 0;
	uint16_t checkSum = 0;
	
	uint8_t config[CONFIG_DATA_SIZE] = {0};
	
	// ��ȡFlash����
  address = WRITE_START_ADDR;
	
	for(i = 0; i < CONFIG_DATA_SIZE; i += 4)
	{
		data = *(__IO uint32_t*) address;
		config[i]			= data;
		config[i + 1] = data >> 8;
		config[i + 2] = data >> 16;
		config[i + 3] = data >> 24;
		
		address += 4;
	}
	
	checkSum = CRC_Table(config, CONFIG_DATA_SIZE - 2);
	
	if((config[10] != (checkSum & 0xFF)) || (config[11] != (checkSum >> 8)))
	{
		printf("Error: Loading config data from flash failed!\nLoading default parameters and saving them to flash!");
		configParam = configParamDefault;
		SaveConfigData();
	}
	else
	{
		// ���ߵ�ַ
		configParam.radio.addr[0] = config[0];
		configParam.radio.addr[1] = config[1];
		configParam.radio.addr[2] = config[2];
		configParam.radio.addr[3] = config[3];
		configParam.radio.addr[4] = config[4];
		
		// ����Ƶ��
		configParam.radio.channel = config[5];
		
		configParam.radio.dataRate = RADIO_DATARATE;
		
		// �ļ�ϵͳ��־
		configParam.fs_mark = config[6];
		configParam.fs_mark = config[7] << 8;
		configParam.fs_mark = config[8] << 16;
		configParam.fs_mark = config[9] << 24;
		
	}
	
}


/**
  * @brief  Save configurations to flash memory.
  * @param  None
  * @retval bool
  */
bool SaveConfigData(void)
{
	uint32_t address = 0x00;				//��¼д��ĵ�ַ
	uint32_t data = 0x0;						//��¼д�������
	uint8_t i= 0;
	uint16_t checkSum = 0;
	
	FLASH_Status FLASHStatus = FLASH_COMPLETE;	//��¼ÿ�β����Ľ��	
	bool programStatus = true;									//��¼�������Խ��
	
	
	uint8_t config[CONFIG_DATA_SIZE] = {0};
	
	// ���ߵ�ַ
	config[0] = configParam.radio.addr[0];
	config[1] = configParam.radio.addr[1];
	config[2] = configParam.radio.addr[2];
	config[3] = configParam.radio.addr[3];
	config[4] = configParam.radio.addr[4];
	
	// ����Ƶ��
	config[5] = configParam.radio.channel;
	
	// �ļ�ϵͳ��־
	config[6] = configParam.fs_mark;
	config[7] = configParam.fs_mark >> 8;
	config[8] = configParam.fs_mark >> 16;
	config[9] = configParam.fs_mark >> 24;
	
	checkSum = CRC_Table(config, CONFIG_DATA_SIZE - 2);
	config[10] = checkSum;
	config[11] = checkSum >> 8;
	

  /* ���� */
  FLASH_Unlock();

  /* ������б�־λ */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

  /* ��ҳ����*/
	FLASHStatus = FLASH_ErasePage(WRITE_START_ADDR);
  
  /* ���ڲ�FLASHд������ */
  address = WRITE_START_ADDR;
	i = 0;

  while((i < CONFIG_DATA_SIZE) && (FLASHStatus == FLASH_COMPLETE))
  {
		data = 0;
		data = config[i] | (config[i + 1] << 8) | (config[i + 2] << 16) | (config[i + 3] << 24);
    FLASHStatus = FLASH_ProgramWord(address, data);
    address += 4;
		i += 4;
  }

  FLASH_Lock();
  
  /* ���д��������Ƿ���ȷ */
  address = WRITE_START_ADDR;
	i = 0;

  while((i < CONFIG_DATA_SIZE) && (programStatus != false))
  {
		data = config[i] | (config[i + 1] << 8) | (config[i + 2] << 16) | (config[i + 3] << 24);
		
    if((*(__IO uint32_t*) address) != data)
    {
      programStatus = false;
    }
    address += 4;
		i += 4;
  }
	
	return programStatus;
	
}
