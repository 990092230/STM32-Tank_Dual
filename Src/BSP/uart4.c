/**
  ******************************************************************************
  * @file    uart4.c
	* @author  Oskar Wei
  * @version V1.0
  * @date    2021-01-26
  * @brief   
  ******************************************************************************
  * @attention
  *
  * Mail: 990092230@qq.com
  * Blog: www.mindsilicon.com
  *
  ******************************************************************************
  */ 

#include "usart1.h"
#include "uart4.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>
#include "led.h"
#include "i2c_oled.h"
#include "radio.h"
#include "flash.h"

/*FreeRTOS���ͷ�ļ�*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"



 /**
  * @brief  UART GPIO ����,������������
  * @param  ��
  * @retval ��
  */
void UART4_Init(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// �򿪴���GPIO�ʹ��������ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

  // ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = baudrate;
	// ���� �������ֳ�
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// ����ֹͣλ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// ����У��λ
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// ����Ӳ��������
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// ���ù���ģʽ���շ�һ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// ��ɴ��ڵĳ�ʼ������
	USART_Init(UART4, &USART_InitStructure);
	
	USART_ClockInitStructure.USART_Clock = USART_Clock_Disable;          // SCLKʱ������(ͬ��ģʽ����Ҫ)
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_Low;                // ʱ�Ӽ���(ͬ��ģʽ����Ҫ)
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_2Edge;              // ʱ����λ(ͬ��ģʽ����Ҫ)
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Disable;      // ���һλʱ������(ͬ��ģʽ����Ҫ)
	USART_ClockInit(UART4, &USART_ClockInitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure); 

	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);

	// ʹ�ܴ���
	USART_Cmd(UART4, ENABLE);
	
//	USART_SendData(UART4, 0x3A);
}


// ����4���ռ���
uint16_t uart4RecCount = 0;

// ����4���ն���
xQueueHandle uart4RxQueue;

// ����4�ж�, ���յ��������Ƿ����ͨ��Э������жϷ�������н����ж�, ������ͨ��Э������ݽ�������
//void UART4_IRQHandler(void)
//{
//	// temp���ڻ��浱ǰ��
//	uint8_t temp = 0;
//	static radioMsg_t usartMsgTemp;
//	
//	if(USART_GetITStatus(UART4, USART_IT_RXNE)!=RESET)
//	{
//		temp = USART_ReceiveData(UART4);
//		
//		if(uart4RecCount == 0)						// ʶ��֡ͷ��һ���ֽ�
//		{
//			if(temp == 0xAA)
//			{
//				uart4RecCount++;
//			}
//		}
//		else if(uart4RecCount == 1)			// ʶ��֡ͷ�ڶ����ֽ�
//		{
//			if(temp == 0xAA)
//			{
//				uart4RecCount++;
//				usartMsgTemp.head = 0xAAAA;
//			}
//			else														// ֡ͷ������
//			{
//				uart4RecCount = 0;
//			}
//		}
//		else if(uart4RecCount == 2)			// ͨ�Ŵ���
//		{
//			uart4RecCount++;
//			usartMsgTemp.msgID = temp;
//		}
//		else if(uart4RecCount == 3)			// ����֡����
//		{
//			uart4RecCount++;
//			if(temp <= 32)
//			{
//				usartMsgTemp.dataLen = temp;
//			}
//			else
//			{
//				uart4RecCount = 0;
//				usartMsgTemp.dataLen = 0;
//			}
//		}
//		else if(uart4RecCount < usartMsgTemp.dataLen - 2)		// ���������غ�����
//		{
//			usartMsgTemp.data[uart4RecCount - 4] = temp;
//			uart4RecCount++;
//		}
//		else if(uart4RecCount == usartMsgTemp.dataLen - 2)		// ����У�����һ���ֽ�
//		{
//			usartMsgTemp.checksum = temp;
//			uart4RecCount++;
//		}
//		else if(uart4RecCount == usartMsgTemp.dataLen - 1)		// ����У����ڶ����ֽ�
//		{
//			usartMsgTemp.checksum = usartMsgTemp.checksum | temp << 8;
//			xQueueSendFromISR(uart4RxQueue, &usartMsgTemp, 0);
//			
//			uart4RecCount = 0;
//			usartMsgTemp.dataLen = 0;
//		}
//		else
//		{
//			uart4RecCount = 0;
//			usartMsgTemp.dataLen = 0;
//		}
//		
//		
//	}
//}


//void Uart4Task(void *param)
//{
//	uart4RxQueue = xQueueCreate(UART4_RX_QUEUE_SIZE, sizeof(radioMsg_t));
//	
//	radioMsg_t usart_msg;
//	
//	uint16_t checkSum = 0;
//	
//	while(1)
//	{
//		if(xQueueReceive(uart4RxQueue, &usart_msg, 10) == pdTRUE)
//		{
//			// ����CRC16У��ֵ, ����յ���У��ֵ�Ƚ�, �ж������Ƿ�������
//			checkSum = CRC_Table((uint8_t *)&usart_msg, usart_msg.dataLen - 2);
//			
//			if(usart_msg.checksum == checkSum)		// У��ͨ��
//			{
////				printf("Received checksum: %x, and calculated checkSum: %x, are mached!\n", usart_msg.checksum, checkSum);
////				ProcessMsg(&usart_msg);
//			}
//			else																	// δͨ��У��
//			{
//				printf("UART4 Error: Received checksum: %x, and calculated checkSum: %x, are not mached!\n", usart_msg.checksum, checkSum);
//			}
//		}
//	}
//}



// ���ڱ��泬��������,��λ����
uint16_t ultrasonic_distance1 = 0;

// ���ڱ������״̬, 0:���ֽ�, 1:β�ֽ�, 2:�������
uint8_t ultrasonic_state1 = 0;


void Uart4Task(void *param)
{
	vTaskDelay(3000);
	
	
	// ��US100������ģ�鷢�Ͳ���ָ��0x55
	while(USART_GetFlagStatus(UART4, USART_FLAG_TXE)== RESET)
	{

	}
	USART_SendData(UART4, 0x55);
	ultrasonic_state1 = 0;
	
	while(1)
	{
		if(ultrasonic_state1 == 2)
		{
			printf("ultrasonic_distance: %d mm\n", ultrasonic_distance1);
			
			// ��US100������ģ�鷢�Ͳ���ָ��0x55
			while(USART_GetFlagStatus(UART4, USART_FLAG_TXE)== RESET)
			{

			}
			USART_SendData(UART4, 0x55);
			ultrasonic_state1 = 0;
		}
		
		// ÿ�����10��
		vTaskDelay(100);
	}
}



// ����4�ж�,
void UART4_IRQHandler(void)
{
	static uint16_t temp = 0;
	
	if(USART_GetITStatus(UART4, USART_IT_RXNE)!=RESET)
	{
		if(ultrasonic_state1 == 0)
		{
			temp = USART_ReceiveData(UART4) << 8;
			ultrasonic_state1 ++;
		}
		else if(ultrasonic_state1 == 1)
		{
			temp |= USART_ReceiveData(UART4);
			ultrasonic_distance1 = temp;
			ultrasonic_state1 ++;
		}
		else
		{
			USART_ReceiveData(UART4);
		}
		
	}
}

