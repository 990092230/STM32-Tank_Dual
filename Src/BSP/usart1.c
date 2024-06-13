/**
  ******************************************************************************
  * @file    usart1.c
	* @author  Oskar Wei
  * @version V1.0
  * @date    2021-01-26
  * @brief   �����õ�printf���ڣ��ض���printf������
  ******************************************************************************
  * @attention
  *
  * Mail: 990092230@qq.com
  * Blog: www.mindsilicon.com
  *
  ******************************************************************************
  */ 

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "usart1.h"
#include "ff.h"
#include "led.h"
#include "i2c_oled.h"
#include "radio.h"
#include "flash.h"
#include "servo.h"
#include "motor.h"
#include "power_control.h"

/*FreeRTOS���ͷ�ļ�*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


 /**
  * @brief  USART GPIO ����,������������
  * @param  ��
  * @retval ��
  */
void USART1_Init(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// �򿪴���GPIO�ʹ��������ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);  

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

  // ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

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
	USART_Init(USART1, &USART_InitStructure);
	
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure); 

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	

	// ʹ�ܴ���
	USART_Cmd(USART1, ENABLE);

//#ifdef __GNUC__
//	setvbuf(stdout, NULL, _IONBF, 0);
//#endif
}


// ����1���ռ���
uint16_t usart1RecCount = 0;

// ���ڽ��ն���
xQueueHandle  usart1RxQueue;


// ����1�ж�, ���յ��������Ƿ����ͨ��Э������жϷ�������н����ж�, ������ͨ��Э������ݽ�������
void USART1_IRQHandler(void)
{
	// temp���ڻ��浱ǰ��
	uint8_t temp = 0;
	static radioMsg_t usartMsgTemp;
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE)!=RESET)
	{
		temp = USART_ReceiveData(USART1);
		
		if(usart1RecCount == 0)						// ʶ��֡ͷ��һ���ֽ�
		{
			if(temp == 0xAA)
			{
				usart1RecCount++;
			}
		}
		else if(usart1RecCount == 1)			// ʶ��֡ͷ�ڶ����ֽ�
		{
			if(temp == 0xAA)
			{
				usart1RecCount++;
				usartMsgTemp.head = 0xAAAA;
			}
			else														// ֡ͷ������
			{
				usart1RecCount = 0;
			}
		}
		else if(usart1RecCount == 2)			// ͨ�Ŵ���
		{
			usart1RecCount++;
			usartMsgTemp.msgID = temp;
		}
		else if(usart1RecCount == 3)			// ����֡����
		{
			usart1RecCount++;
			if(temp <= 32)
			{
				usartMsgTemp.dataLen = temp;
			}
			else
			{
				usart1RecCount = 0;
				usartMsgTemp.dataLen = 0;
			}
		}
		else if(usart1RecCount < usartMsgTemp.dataLen - 2)		// ���������غ�����
		{
			usartMsgTemp.data[usart1RecCount - 4] = temp;
			usart1RecCount++;
		}
		else if(usart1RecCount == usartMsgTemp.dataLen - 2)		// ����У�����һ���ֽ�
		{
			usartMsgTemp.checksum = temp;
			usart1RecCount++;
		}
		else if(usart1RecCount == usartMsgTemp.dataLen - 1)		// ����У����ڶ����ֽ�
		{
			usartMsgTemp.checksum = usartMsgTemp.checksum | temp << 8;
			xQueueSendFromISR(usart1RxQueue, &usartMsgTemp, 0);
			
			usart1RecCount = 0;
			usartMsgTemp.dataLen = 0;
		}
		else
		{
			usart1RecCount = 0;
			usartMsgTemp.dataLen = 0;
		}
		
		
	}
}




static bool usart1LinkStatus = false;

/*��ȡUSART1����״̬*/
bool Usart1LinkStatus(void)
{
	return usart1LinkStatus;
}


// ����1����,����1��USB�ӿ�
void Usart1Task(void *param)
{
	usart1RxQueue = xQueueCreate(USART1_RX_QUEUE_SIZE, sizeof(radioMsg_t));
	
	radioMsg_t usart_msg;
	
	uint16_t checkSum = 0;
	
	while(1)
	{
		if(xQueueReceive(usart1RxQueue, &usart_msg, 200) == pdTRUE)
		{
			// ����CRC16У��ֵ, ����յ���У��ֵ�Ƚ�, �ж������Ƿ�������
			checkSum = CRC_Table((uint8_t *)&usart_msg, usart_msg.dataLen - 2);
			
			if(usart_msg.checksum == checkSum)		// У��ͨ��
			{
				// printf("Received checksum: %x, and calculated checkSum: %x, are mached!\n", usart_msg.checksum, checkSum);
				ProcessMsg(&usart_msg);
				usart1LinkStatus = true;
			}
			else																	// δͨ��У��
			{
				printf("USART1 Error: Received checksum: %x, and calculated checkSum: %x, are not mached!\n", usart_msg.checksum, checkSum);
			}
		}
		else
		{
			usart1LinkStatus = false;
		}
	}
}


extern systemConfig_t configParam;
extern motorStatus_t motorStatus;
extern servoPWM_t servoPWM;
extern uint8_t powerStatus;
extern uint8_t ak8963_CaliStage;

extern xQueueHandle  radioTxQueue;


// ��Ϣ������, ͨ�Ŵ���ɲ���radio.h�еĶ���
void ProcessMsg(radioMsg_t *msg)
{
	radioMsg_t ackMsg;
	
	// ����ͨ�Ŵ���Ĳ�ͬ��������ͬ����Ӧ
	if(msg->msgID == CMD_APPTCB1)				// ����λ�����͵����ư������1, ��ȡ��ַ��Ƶ��
	{
		ackMsg.head = 0xAAAA;
		ackMsg.msgID = RES_APPTCB1;				// ���ư��APPTCB1����Ӧ, ���ص�ַ��Ƶ��
		ackMsg.dataLen = 0x0C;
		
		// ���ߵ�ַ
		ackMsg.data[0] = configParam.radio.addr[0];
		ackMsg.data[1] = configParam.radio.addr[1];
		ackMsg.data[2] = configParam.radio.addr[2];
		ackMsg.data[3] = configParam.radio.addr[3];
		ackMsg.data[4] = configParam.radio.addr[4];
		
		// ����Ƶ��
		ackMsg.data[5] = configParam.radio.channel;
		
		// CRC16У���
		ackMsg.checksum = CRC_Table((uint8_t *)&ackMsg, ackMsg.dataLen - 2);
		
		// ��������
		for(int i = 0; i < ackMsg.dataLen - 2; i++)
		{
			while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
			{
				
			}
			USART_SendData(USART1, ((uint8_t *)&ackMsg)[i]);
		}
		
		// ��ΪSTM32��Ƭ����С��ģʽ, ����У��ֵ�ĵ��ֽڴ洢�ڵ͵�ַ, �ȷ���У��ĵ��ֽ�
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
		{

		}
		USART_SendData(USART1, ackMsg.checksum);
		
		// ����У��ĸ��ֽ�
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
		{

		}
		USART_SendData(USART1, ackMsg.checksum >> 8);
		
	}
	else if(msg->msgID == CMD_APPTCB2)				// ����λ�����͵����ư������2, д���ַ
	{
		configParam.radio.addr[0] = msg->data[0];
		configParam.radio.addr[1] = msg->data[1];
		configParam.radio.addr[2] = msg->data[2];
		configParam.radio.addr[3] = msg->data[3];
		configParam.radio.addr[4] = msg->data[4];
		
		if(SaveConfigData())
		{
			// д��ɹ�����1, д��ʧ�ܷ���0
			ackMsg.data[0] = 0x01;
		}
		else
		{
			ackMsg.data[0] = 0x0;
		}
		
		ackMsg.head = 0xAAAA;
		ackMsg.msgID = RES_APPTCB2;				// ���ư��APPTCB2����Ӧ, ����д��״̬
		ackMsg.dataLen = 0x07;
		
		// CRC16У���
		ackMsg.checksum = CRC_Table((uint8_t *)&ackMsg, ackMsg.dataLen - 2);
		
		// ��������
		for(int i = 0; i < ackMsg.dataLen - 2; i++)
		{
			while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
			{
				
			}
			USART_SendData(USART1, ((uint8_t *)&ackMsg)[i]);
		}
		
		// ��ΪSTM32��Ƭ����С��ģʽ, ����У��ֵ�ĵ��ֽڴ洢�ڵ͵�ַ, �ȷ���У��ĵ��ֽ�
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
		{

		}
		USART_SendData(USART1, ackMsg.checksum);
		
		// ����У��ĸ��ֽ�
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
		{

		}
		USART_SendData(USART1, ackMsg.checksum >> 8);
		
	}
	else if(msg->msgID == CMD_APPTCB3)				// ����λ�����͵����ư������3, д��Ƶ��
	{
		configParam.radio.channel = msg->data[0];
		
		if(SaveConfigData())
		{
			// д��ɹ�����1, д��ʧ�ܷ���0
			ackMsg.data[0] = 0x01;
		}
		else
		{
			ackMsg.data[0] = 0x0;
		}
		
		ackMsg.head = 0xAAAA;
		ackMsg.msgID = RES_APPTCB3;				// ���ư��APPTCB3����Ӧ, ����д��״̬
		ackMsg.dataLen = 0x07;
		
		// CRC16У���
		ackMsg.checksum = CRC_Table((uint8_t *)&ackMsg, ackMsg.dataLen - 2);
		
		// ��������
		for(int i = 0; i < ackMsg.dataLen - 2; i++)
		{
			while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
			{
				
			}
			USART_SendData(USART1, ((uint8_t *)&ackMsg)[i]);
		}
		
		// ��ΪSTM32��Ƭ����С��ģʽ, ����У��ֵ�ĵ��ֽڴ洢�ڵ͵�ַ, �ȷ���У��ĵ��ֽ�
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
		{

		}
		USART_SendData(USART1, ackMsg.checksum);
		
		// ����У��ĸ��ֽ�
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
		{

		}
		USART_SendData(USART1, ackMsg.checksum >> 8);
		
	}
	else if(msg->msgID == CMD_APPTCB5) // ����λ�����͵����ư������5, ģ�������ֱ�����, ��λ�������ֱ��ĸ�ʽ�������ݸ�С��
	{
		ParseRadioMsg(msg);
	}
	else if(msg->msgID == CMD_APPTCB6) // ����λ�����͵����ջ�������6,����8·���PWMֵ�͵��ת��,���ת�ٻ���ÿ��,����2·��Դ�������
	{
		// �����ʽʾ��:AA AA 16 1B 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 5E A6
		// ���ö��
		servoPWM.servo1 = (msg->data[1] << 8) | msg->data[0];
		servoPWM.servo2 = (msg->data[3] << 8) | msg->data[2];
		servoPWM.servo3 = (msg->data[5] << 8) | msg->data[4];
		servoPWM.servo4 = (msg->data[7] << 8) | msg->data[6];
		servoPWM.servo5 = (msg->data[9] << 8) | msg->data[8];
		servoPWM.servo6 = (msg->data[11] << 8) | msg->data[10];
		servoPWM.servo7 = (msg->data[13] << 8) | msg->data[12];
		servoPWM.servo8 = (msg->data[15] << 8) | msg->data[14];
		
		// ���õ��
		motorStatus.target_speed1 = (msg->data[16] << 8) | msg->data[17];
		motorStatus.target_speed2 = (msg->data[18] << 8) | msg->data[19];
		
		// ���õ�Դ�������
		powerStatus = msg->data[20];
		
		// �������ݸ�ʽʾ��:AA AA 1E 11 00 00 00 00 00 00 00 00 00 00 00 7F F6
		ackMsg.head = 0xAAAA;
		ackMsg.msgID = RES_APPTCB6;				// ���ư��APPTCB3����Ӧ, ����д��״̬
		ackMsg.dataLen = 0x11;
		
		ackMsg.data[0] = 0;
		
		ackMsg.data[1] = motorStatus.current_speed1 >> 8;
		ackMsg.data[2] = motorStatus.current_speed1;
		
		ackMsg.data[3] = motorStatus.current_speed2 >> 8;
		ackMsg.data[4] = motorStatus.current_speed2;

//		ackMsg.data[1] = motorStatus.motor1_pulse >> 8;
//		ackMsg.data[2] = motorStatus.motor1_pulse;
//		
//		ackMsg.data[3] = motorStatus.motor2_pulse >> 8;
//		ackMsg.data[4] = motorStatus.motor2_pulse;
		
		// CRC16У���
		ackMsg.checksum = CRC_Table((uint8_t *)&ackMsg, ackMsg.dataLen - 2);
		
		// ��������
		for(int i = 0; i < ackMsg.dataLen - 2; i++)
		{
			while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
			{

			}
			USART_SendData(USART1, ((uint8_t *)&ackMsg)[i]);
		}

		// ��ΪSTM32��Ƭ����С��ģʽ, ����У��ֵ�ĵ��ֽڴ洢�ڵ͵�ַ, �ȷ���У��ĵ��ֽ�
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
		{

		}
		USART_SendData(USART1, ackMsg.checksum);

		// ����У��ĸ��ֽ�
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE)== RESET)
		{

		}
		USART_SendData(USART1, ackMsg.checksum >> 8);
	}
	else if(msg->msgID == CMD_APPTCB9)	// ����λ�����͵��ֱ�������,У׼��������ƴ�����,֪ͨ��λ������ԭʼ����, ����У׼�������λ��
	{
		// ����3������׶�,
		// ��һ�׶��յ���λ����ȡ���������ݵ�ָ��, Ȼ���������λ�����ʹ���������
		// �ڶ����׶�, ��λ���Ѿ��ռ������㹻������, ����ָ��Ҫ����λ��ֹͣ��������
		// �������׶�, ��λ���Ѿ��������У׼����, ������λ������У׼��������λ��, ��λ�����������
		// ����ͨ��״̬�������жϵ�ǰУ׼�Ľ׶�, 1:��һ�׶�, 2:�ڶ��׶�, 3:�����׶�
		// �����ʽʾ��:AA AA 21 len status _ _
		if(msg->data[0] ==  1)
		{
			ak8963_CaliStage = 1;
		}
		else if(msg->data[0] ==  2)
		{
			ak8963_CaliStage = 2;
		}
		else if(msg->data[0] == 3)
		{
			ak8963_CaliStage = 3;
		}
		else
		{
			ak8963_CaliStage = 0;
		}
		
	}
	else if((msg->msgID == RES_RTCB1) || (msg->msgID == RES_APPTCB5)) // �ش����ݸ��ֱ�, ͸������, ���ջ��������ݸ���λ��
	{
		xQueueSend(radioTxQueue, msg, 0);
	}
	
}



#ifdef __GNUC__


#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int _isatty(int fd)
{
	if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
		return 1;

	errno = EBADF;
	return 0;
}

int _getpid()
{
	return 0;
}

int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}


int _write(int fd, char *ptr, int len)
{
	for(uint16_t t=0; t<len; t++)
	{
		//while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
		USART_SendData(USART1, ptr[t]);
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
	return len;
}

int _read(int fd, char* ptr, int len)
{
	for(int t=0; t<len; t++)
	{
		/* �ȴ������������� */
		while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

		ptr[t] = (char)USART_ReceiveData(USART1);
	}
	return len;
}


int _close(int fd)
{
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
    return 0;

  errno = EBADF;
  return -1;
}


int _lseek(int fd, int ptr, int dir)
{
  (void) fd;
  (void) ptr;
  (void) dir;

  errno = EBADF;
  return -1;
}

int _fstat(int fd, struct stat* st)
{
  if (fd >= STDIN_FILENO && fd <= STDERR_FILENO)
  {
    st->st_mode = S_IFCHR;
    return 0;
  }

  errno = EBADF;
  return 0;
}



#elif

///�ض���c�⺯��printf�����ڣ��ض�����ʹ��printf����
int fputc(int ch, FILE *f)
{
		/* ����һ���ֽ����ݵ����� */
		USART_SendData(USART1, (uint8_t) ch);

		/* �ȴ�������� */
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

		return (ch);
}

///�ض���c�⺯��scanf�����ڣ���д����ʹ��scanf��getchar�Ⱥ���
int fgetc(FILE *f)
{
		/* �ȴ������������� */
		while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);

		return (char)USART_ReceiveData(USART1);
}

#endif






