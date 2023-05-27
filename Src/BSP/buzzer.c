/**
  ******************************************************************************
  * @file    buzzer.c
  * @author  Oskar Wei
  * @version V1.0
  * @date    2020-05-02
  * @brief   buzzer low level init ��������������
  ******************************************************************************
  * @attention
  *
  * Mail: 990092230@qq.com
  * Shop: www.mindsilicon.com
  *
  ******************************************************************************
  */
  
#include "buzzer.h"   
#include "delay.h"
#include "led.h"
#include <stdio.h>


/*FreeRTOS���ͷ�ļ�*/
#include "FreeRTOS.h"
#include "task.h"



void TIM6_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	// ������ʱ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	/*--------------------ʱ���ṹ���ʼ��-------------------------*/
	// �������ڣ���������Ϊ1K
	
	// �Զ���װ�ؼĴ�����ֵ���ۼ�TIM_Period+1��Ƶ�ʺ����һ�����»����ж�
	TIM_TimeBaseStructure.TIM_Period = 30;	
	// ����CNT��������ʱ�� = Fck_int/(psc+1)
	TIM_TimeBaseStructure.TIM_Prescaler = 7200-1;	
	// ʱ�ӷ�Ƶ���� ����������ʱ��ʱ��Ҫ�õ�
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		
	// ����������ģʽ������Ϊ���ϼ���
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		
	// �ظ���������ֵ��û�õ����ù�
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;	
	// ��ʼ����ʱ��
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ClearFlag(TIM6, TIM_FLAG_Update);//���TIM�ĸ��±�־λ
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	
	//Reset counter
  TIM_SetCounter(TIM6, 0);
	TIM_Cmd(TIM6, DISABLE);
}


void TIM6_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM6,TIM_IT_Update)!=RESET)
	{
		TIM_ClearITPendingBit(TIM6,TIM_IT_Update); // ����жϱ�־λ
		TOGGLE_BUZZER;
	}
}




// ��ʼ��������
void Buzzer_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	BUZZER_OFF;
	
	TIM6_Init();
}

// ��ǰ��������
uint8_t currMusic = MUSIC_MUTE;
									
void PlayMusic(void *param)
{
	// ��������ʼ��
	Buzzer_Init();
	//TIM_Cmd(TIM6, ENABLE);
	
	while(1)
	{
//		printf(">>PlayMusic Task...\r\n");
		if(currMusic == MUSIC_MUTE)
		{
			vTaskDelay(20);
		}
		else
		{
			TIM_Cmd(TIM6, ENABLE);
			vTaskDelay(100);
			TIM_Cmd(TIM6, DISABLE);
			vTaskDelay(100);
			
			currMusic = MUSIC_MUTE;
			vTaskDelay(50);
		}
		
		
	}
	
}

