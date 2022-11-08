/**
  ******************************************************************************
  * @file    radio.c
  * @author  Oskar Wei
  * @version V1.0
  * @date    2020-06-09
  * @brief   radio api
  ******************************************************************************
  * @attention
  *
  * Mail: 990092230@qq.com
  * Shop: www.mindsilicon.com
  *
	* �ó������ѧϰʹ�ã�δ�����������������������κ���;
  ******************************************************************************
  */

#include "obstacle.h"
#include "motor.h"
#include "led.h"
#include "flash.h"
#include "nrf24l01.h"
#include "radio.h" 
#include "i2c_oled.h"
#include "usart1.h"
#include "servo.h"
#include <stdio.h>
#include <string.h>

/*FreeRTOS���ͷ�ļ�*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"





/*���ն�����Ϣ����*/
#define  RADIOLINK_TX_QUEUE_SIZE  3

xTaskHandle radiolinkTaskHandle;
xQueueHandle  radioTxQueue;
//xQueueHandle  radioRxQueue;
static xSemaphoreHandle nrfIT;

static bool radioConnectStatus = false;
static uint16_t failReceiveCount = 0;

// ϵͳ����
systemConfig_t configParam;



systemConfig_t configParamDefault=
{
	.version = VERSION,
	.radio.channel = RADIO_CHANNEL,
	.radio.dataRate = RADIO_DATARATE,
	.radio.addr = RADIO_ADDRESS,
	.fs_mark = 0xFFFFFFFF,
};

/*nrf�ⲿ�жϻص�����*/
static void nrf_interruptCallback(void)
{
	portBASE_TYPE  xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(nrfIT, &xHigherPriorityTaskWoken);
}

/*�������ó�ʼ������ַ��ͨ�������ʣ�*/
static void radioInit(void)
{
	// ����STM32Ƭ��Flash�洢���б������������,������ַ��Ƶ�ʡ�ҡ��У׼����
	LoadConfigData();
	
	
	if(nrf_check() == SUCCESS)
	{
		NRF_Init(PRX_MODE);
		nrf_setIterruptCallback(nrf_interruptCallback);
	}
	else
	{
		printf("Radio check fail!/n");
		while(1);
	}
}


/*�������ӳ�ʼ��*/
void radiolinkInit(void)
{
	radioInit();
	
	radioTxQueue = xQueueCreate(RADIOLINK_TX_QUEUE_SIZE, sizeof(radioMsg_t));
	ASSERT(radioTxQueue);
	
//	radioRxQueue = xQueueCreate(RADIOLINK_RX_QUEUE_SIZE, sizeof(radioMsg_t));
//	ASSERT(radioRxQueue);
	
	nrfIT = xSemaphoreCreateBinary();
}



/*������������*/
void RadioTask(void* param)
{
	radioMsg_t rx_p;
	uint16_t checkSum;
	uint8_t temp[sizeof(radioMsg_t)];

	radiolinkInit();	/*����ͨ�ų�ʼ��*/
	
	// RX active
	NRF_CE_H();
	
	while(1)
	{
//		printf("Radio Task\n");
		LED_TOGGLE;
		
		if(xQueueReceive(radioTxQueue, &rx_p, 0) == pdTRUE)
		{
			memcpy(temp, (uint8_t *)&rx_p, rx_p.dataLen - 2);
			temp[rx_p.dataLen - 2] = rx_p.checksum;
			temp[rx_p.dataLen - 1] = rx_p.checksum >> 8;
			nrf_txPacket_AP(temp, rx_p.dataLen);
		}
		
		xSemaphoreTake(nrfIT, 100);
		
		nrfEvent_e status = nrf_checkEventandRxPacket((uint8_t *)&rx_p, &(rx_p.dataLen));
		rx_p.checksum = ((uint8_t *)&rx_p)[rx_p.dataLen - 2] | ((uint8_t *)&rx_p)[rx_p.dataLen - 1] << 8;
		
		// ���δ�յ�����
		if(status == IDLE)
		{
			failReceiveCount ++;
			if(failReceiveCount > 2)
			{
				radioConnectStatus = false;
			}
			continue;
		}
		
		checkSum = CRC_Table((uint8_t *)&rx_p, rx_p.dataLen - 2);
		if((rx_p.dataLen <= RADIO_MSG_MAX_SIZE) && ((rx_p.checksum & 0xFF) == (checkSum & 0xFF)) && ((rx_p.checksum >> 8) == (checkSum >> 8)))
		{
			ParseRadioMsg(&rx_p);
			failReceiveCount = 0;
			radioConnectStatus = true;
		}
		else
		{
			failReceiveCount ++;
			if(failReceiveCount > 2)
			{
				radioConnectStatus = false;
			}
			continue;
		}
		
	}
}


extern motorStatus_t motorStatus;
extern servoPWM_t servoPWM;

// ������������
void ParseRadioMsg(radioMsg_t *msg)
{
	int16_t pwm1 = 0, pwm2 = 0;
	static float duty[8] = {0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5};
	uint16_t keys = 0;
	
	// �ȼ�������PWMֵ, data[4]���Ҳ�ҡ�˴�ֱ����, ��������ǰ���˶�
	pwm1 = -(msg->data[4] - 0x7F) * 8;
	pwm2 = pwm1;
	
	// data[3]���Ҳ�ҡ��ˮƽ����, ������������ת��ʱ����Ĳ���
	pwm1 += (msg->data[3] - 0x7F) * 5;
	pwm2 -= (msg->data[3] - 0x7F) * 5;
	
	//ObstacleAvoidance(&pwm1, &pwm2);
	
	motorStatus.motor1_pwm = pwm1;
	motorStatus.motor2_pwm = pwm2;
	
	// 1�Ŷ������, data[1]�����ҡ��ˮƽ����, ��������ת����
	duty[0] = -(float)(msg->data[1] - 0x7F) / 0x7F;
	// ����6�ǿ��ƶ��ת������, ����0.5ʹҡ�˻���
	duty[0] = duty[0] / 7 + 0.5;
	// ת��������1�Ŷ���ӿ�
	PWM(&duty[0]);
	servoPWM.servo1 = Duty_to_PWM(duty[0]);
	
	// ��������Ŀ���, �ð�������, �ֱ�Ϊdata[5]��data[6]
	// data�ĵ�6�͵�7�ֽ�, �������ֽڹ�16λ, ��Ӧ�ֱ���16��ͨ��, 
	// �����λ��16λ����1λ, �����ǣ�L2, L1, LU, LL, LD, LR, SE, ST, RL, RD, RR, RU, R1, R2, R-KEY, L-KEY
	// ����R-KEY��L-KEY�ֱ�������ҡ�����°��¶�Ӧ�İ���
	keys = (msg->data[5] << 8) | msg->data[6];
	
	// 2�Ŷ������, ��L1��R1��������ת�ͷ�ת, 
	if((keys & (1 << 14)) && (!(keys & (1 << 3))))				// L1������R1�ɿ�ʱ
	{
		duty[1] -= 0.005;
		PWM(&duty[1]);
		servoPWM.servo2 = Duty_to_PWM(duty[1]);
	}
	else if((!(keys & (1 << 14))) && (keys & (1 << 3)))		// L1�ɿ���R1����ʱ
	{
		duty[1] += 0.005;
		PWM(&duty[1]);
		servoPWM.servo2 = Duty_to_PWM(duty[1]);
	}
	
	// 3�Ŷ������, ��L2��R2��������ת�ͷ�ת, 
	if((keys & (1 << 15)) && (!(keys & (1 << 2))))				// L2������R2�ɿ�ʱ
	{
		duty[2] -= 0.005;
		PWM(&duty[2]);
		if(duty[2] < 0.1)		// ��ֹ��̨��ֱ���ת���Ƕȹ��󣬶����¶�ת
		{
			duty[2] = 0.1;
		}
		servoPWM.servo3 = Duty_to_PWM(duty[2]);
	}
	else if((!(keys & (1 << 15))) && (keys & (1 << 2)))		// L2�ɿ���R2����ʱ
	{
		duty[2] += 0.005;
		PWM(&duty[2]);
		if(duty[2] > 0.9)		// ��ֹ��̨��ֱ���ת���Ƕȹ��󣬶����¶�ת
		{
			duty[2] = 0.9;
		}
		servoPWM.servo3 = Duty_to_PWM(duty[2]);
	}
	
	// 4�Ŷ������, ��LL��LR��������ת�ͷ�ת, 
	if((keys & (1 << 12)) && (!(keys & (1 << 10))))				// LL������LR�ɿ�ʱ
	{
		duty[3] -= 0.005;
		PWM(&duty[3]);
		servoPWM.servo4 = Duty_to_PWM(duty[3]);
	}
	else if((!(keys & (1 << 12))) && (keys & (1 << 10)))		// LL�ɿ���LR����ʱ
	{
		duty[3] += 0.005;
		PWM(&duty[3]);
		servoPWM.servo4 = Duty_to_PWM(duty[3]);
	}
	
	// 5�Ŷ������, ��LU��LD��������ת�ͷ�ת, 
	if((keys & (1 << 13)) && (!(keys & (1 << 11))))				// LU������LD�ɿ�ʱ
	{
		duty[4] -= 0.005;
		PWM(&duty[4]);
		servoPWM.servo5 = Duty_to_PWM(duty[4]);
	}
	else if((!(keys & (1 << 13))) && (keys & (1 << 11)))		// LU�ɿ���LD����ʱ
	{
		duty[4] += 0.005;
		PWM(&duty[4]);
		servoPWM.servo5 = Duty_to_PWM(duty[4]);
	}
	
	// 6�Ŷ������, ��RL��RR��������ת�ͷ�ת, 
	if((keys & (1 << 7)) && (!(keys & (1 << 5))))				// RL������RR�ɿ�ʱ
	{
		duty[5] -= 0.005;
		PWM(&duty[5]);
		servoPWM.servo6 = Duty_to_PWM(duty[5]);
	}
	else if((!(keys & (1 << 7))) && (keys & (1 << 5)))		// RL�ɿ���RR����ʱ
	{
		duty[5] += 0.005;
		PWM(&duty[5]);
		servoPWM.servo6 = Duty_to_PWM(duty[5]);
	}
	
	// 7�Ŷ������, ��RU��RD��������ת�ͷ�ת, 
	if((keys & (1 << 4)) && (!(keys & (1 << 6))))				// RU������RD�ɿ�ʱ
	{
		duty[6] -= 0.005;
		PWM(&duty[6]);
		servoPWM.servo7 = Duty_to_PWM(duty[6]);
	}
	else if((!(keys & (1 << 4))) && (keys & (1 << 6)))		// RU�ɿ���RD����ʱ
	{
		duty[6] += 0.005;
		PWM(&duty[6]);
		servoPWM.servo7 = Duty_to_PWM(duty[6]);
	}
	
	// 8�Ŷ������, ��SE��ST��������ת�ͷ�ת, 
	if((keys & (1 << 9)) && (!(keys & (1 << 8))))					// SE������ST�ɿ�ʱ
	{
		duty[7] -= 0.005;
		PWM(&duty[7]);
		servoPWM.servo8 = Duty_to_PWM(duty[7]);
	}
	else if((!(keys & (1 << 9))) && (keys & (1 << 8)))		// SE�ɿ���ST����ʱ
	{
		duty[7] += 0.005;
		PWM(&duty[7]);
		servoPWM.servo8 = Duty_to_PWM(duty[7]);
	}
	
}


/*��ȡ��������*/
uint16_t radioinkFailRxcount(void)
{
	return failReceiveCount;
}

/*��ȡ��������״̬*/
bool radiolinkConnectStatus(void)
{
	return radioConnectStatus;
}

/*ʹ��radiolink*/
void radiolinkEnable(FunctionalState state)
{
	if(state == ENABLE)
	{
		vTaskResume(radiolinkTaskHandle);
	}
	else
	{
		vTaskSuspend(radiolinkTaskHandle);
	}
}







