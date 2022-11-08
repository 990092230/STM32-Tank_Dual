/**
  ******************************************************************************
  * @file    power_control.c
  * @author  Oskar Wei
  * @version V1.0
  * @date    2020-06-01
  * @brief   output power control
  ******************************************************************************
  * @attention
  *
  * Mail: 990092230@qq.com
  * Shop: www.mindsilicon.com
  *
	* �ó������ѧϰʹ�ã�δ�����������������������κ���;
  ******************************************************************************
  */

#include "i2c_oled.h"
#include "power_control.h"
#include "key.h"
#include "adc.h"
#include "motor.h"

#include "FreeRTOS.h"
#include "task.h"

 /**
  * @brief  ��ʼ�����ƿɵ���Դ������
  * @param  ��
  * @retval ��
  */
void ADJ_Outpt_Init(void)
{		
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*����LED��ص�GPIO����ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	/*ѡ��Ҫ���Ƶ�GPIO����*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;	

	/*��������ģʽΪͨ���������*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

	/*������������Ϊ50MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 

	/*���ÿ⺯������ʼ��GPIO*/
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	/* �ر�ADJ��Դ���	*/
	GPIO_ResetBits(GPIOC, GPIO_Pin_15);
}



 /**
  * @brief  ��ʼ������Դ������
  * @param  ��
  * @retval ��
  */
void MAIN_Output_Init(void)
{		
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*����LED��ص�GPIO����ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	/*ѡ��Ҫ���Ƶ�GPIO����*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;	

	/*��������ģʽΪͨ���������*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

	/*������������Ϊ50MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 

	/*���ÿ⺯������ʼ��GPIO*/
	GPIO_Init(GPIOC, &GPIO_InitStructure);	

	/* �ر�����Դ���	*/
	GPIO_ResetBits(GPIOC, GPIO_Pin_14);
}


 /**
  * @brief  ��ʼ����Դ��
  * @param  ��
  * @retval ��
  */
void PowerControl_Init(void)
{		
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*����LED��ص�GPIO����ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	/*ѡ��Ҫ���Ƶ�GPIO����*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	

	/*��������ģʽΪͨ���������*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

	/*������������Ϊ50MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*���ÿ⺯������ʼ��GPIO*/
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
//	GPIO_SetBits(GPIOC, GPIO_Pin_0);
}


extern Menu menu;

/* ��Դ״̬��1���ֽڱ�ʾ, 0:�ر���ѹ���/�ر�����Դ���; 1:������ѹ���/�ر�����Դ���;
 * 2:�ر���ѹ���/��������Դ���; 3:������ѹ���/��������Դ���
 */
uint8_t powerStatus = 0;


// ��Դ�������ػ�����
void PowerTask(void *param)
{
	ADJ_Outpt_Init();
	MAIN_Output_Init();
	PowerControl_Init();
	
	PowerKey_Init();
	
HERE:
	
	// ��������1�뿪��
	vTaskDelay(1000);
	if(!POWER_KEY_STATUS)
	{
		POWER_ON;
	}
	else
	{
		goto HERE;
//		while(1)
//		{
//			vTaskDelay(100);
//		}
	}
	
	ADJ_OUTPUT_ON;
	MAIN_OUTPUT_ON;
	
	
	// ����Դ��״̬
	while(!POWER_KEY_STATUS)      // �������Դ��δ�ɿ�
	{
		vTaskDelay(100);
	}
	
	while(menu == START)
	{
		vTaskDelay(100);
	}
	
	while(1)
	{
		if(!POWER_KEY_STATUS)
		{
			vTaskDelay(1500);
			if(!POWER_KEY_STATUS)
			{
				menu = SHUTDOWN;
				vTaskDelay(6000);		// �ȴ��ػ���������,�ȴ�ת���еĵ��ֹͣ,��ֹ�������ЧӦ�������½��뿪������
				while(!POWER_KEY_STATUS);
				POWER_OFF;
			}
		}
		else
		{
			if(powerStatus == 0x00)
			{
				ADJ_OUTPUT_OFF;
				MAIN_OUTPUT_OFF;
			}
			else if(powerStatus == 0x01)
			{
				ADJ_OUTPUT_ON;
				MAIN_OUTPUT_OFF;
			}
			else if(powerStatus == 0x02)
			{
				ADJ_OUTPUT_OFF;
				MAIN_OUTPUT_ON;
			}
			else if(powerStatus == 0x03)
			{
				ADJ_OUTPUT_ON;
				MAIN_OUTPUT_ON;
			}
			
			// ����֮ǰADC�Ĳ�������������Դ��ѹ
			ADC_ConvertedValueLocal[0] = ADC_ConvertedValueLocal[0] * (0.8) + (float) ADC_ConvertedValue[0]/4096*3.3*11 * (0.2);
			
			// ��������Դ��ѹ
			ADC_ConvertedValueLocal[1] = ADC_ConvertedValueLocal[1] * (0.8) + (float) ADC_ConvertedValue[1]/4096*3.3*20.608 *(0.2);
			vTaskDelay(20);
		}
	}
	
}


/*********************************************END OF FILE**********************/
