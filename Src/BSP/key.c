/**
  ******************************************************************************
  * @file    key.c
  * @author  Oskar Wei
  * @version V1.0
  * @date    2020-05-02
  * @brief   key low level init
  ******************************************************************************
  * @attention
  *
  * Mail: 990092230@qq.com
  * Shop: www.mindsilicon.com
  *
  ******************************************************************************
  */
  
#include "key.h"   

 /**
  * @brief  ��ʼ���û���
  * @param  ��
  * @retval ��
  */
void UserKey_Init(void)
{		
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*����LED��ص�GPIO����ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/*ѡ��Ҫ���Ƶ�GPIO����*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	

	/*��������ģʽΪͨ���������*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;   

	/*������������Ϊ50MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*���ÿ⺯������ʼ��GPIO*/
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
}


 /**
  * @brief  ��ʼ����Դ��
  * @param  ��
  * @retval ��
  */
void PowerKey_Init(void)
{		
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;

	/*����LED��ص�GPIO����ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
	/*ѡ��Ҫ���Ƶ�GPIO����*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	

	/*��������ģʽΪͨ���������*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	
	/*������������Ϊ50MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	
	/*���ÿ⺯������ʼ��GPIO*/
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
}



/*********************************************END OF FILE**********************/
