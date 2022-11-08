/**
  ******************************************************************************
  * @file    main.c
  * @author: Oskar Wei
  * @version 2.1
  * @date    2020-05-17
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * Mail: 990092230@qq.com
  * Shop: www.mindsilicon.com
  *
	* �ó������ѧϰʹ�ã�δ�����������������������κ���;
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include <stdio.h>
#include "led.h"
#include "usart1.h"
#include "uart4.h"
#include "uart5.h"
#include "spi_flash.h"
#include "ff.h"
#include "string.h"
#include "key.h"
#include "i2c_oled.h"
#include "delay.h"
#include "buzzer.h"
#include "watchdog.h"
#include "delay.h"
#include "sensors.h"
#include "utils.h"
#include "power_control.h"
#include "adc.h"
#include "can.h"
#include "servo.h"
#include "motor.h"
#include "radio.h"
#include "flash.h"

#include "FreeRTOS.h"	// �˴�"x"��ʾ,����ΪKeil��,������޹أ�û��Ӱ��
#include "task.h"

#include "i2c_device.h"

/* Private define ------------------------------------------------------------*/
/* Extern define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/




TaskHandle_t startTaskHandle;
extern xTaskHandle radiolinkTaskHandle;

static void startTask(void *arg);

uint8_t can_buf[8] = {'H','i','!','C','A','N','!','\n'};

int main(void)
{
	// �ж����ȼ���������
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	// ��ʱ��ʼ��
	Delay_Init(72);
	
	// ��������ʼ��
	Buzzer_Init();
	
	// �û�����ʼ��
	UserKey_Init();
	
	LED_Init();
	
	USART1_Init(115200);
	printf("Hello  world !\r\n");
	UART4_Init(9600);
	UART5_Init(9600);
	
	i2cdevInit(I2C2_DEV);
	
	// ����STM32Ƭ��Flash�洢���б������������,������ַ��Ƶ�ʡ�ҡ��У׼����
	LoadConfigData();
	
	FileSystem_Init();
	
	ADCx_Init();
	
	Servo_Init();
	
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS1_4tq,CAN_BS1_4tq,4,CAN_Mode_Normal);//CAN��ʼ������ģʽ,������1Mbps
		
	// ��ʼ�����Ź�
//	watchdogInit(WATCHDOG_RELOAD_MS);
	
	xTaskCreate(startTask, "START_TASK", 300, NULL, 1, &startTaskHandle);	/*������ʼ����*/

	vTaskStartScheduler();	/*�����������*/
	
	
	/* ����ʹ���ļ�ϵͳ,ȡ�������ļ�ϵͳ */
	f_mount(NULL,"1:",1);
	LED_TOGGLE;
}


// ��������
void startTask(void *arg)
{
	// �����ٽ���
	taskENTER_CRITICAL();
	
	printf("Free heap before starting: %d bytes\n", xPortGetFreeHeapSize());
	
	xTaskCreate(SensorTask, "SENSORS", 300, NULL, 5, NULL);			        /*������������������*/
	
	xTaskCreate(MenuTask, "MENU", 1600, NULL, 5, NULL);										/*������ʾ��������*/
	
//	xTaskCreate(PlayMusic, "PlayMusic", 200, NULL, 3, NULL);			      /*�������ִ�������*/
	
	xTaskCreate(PowerTask, "POWER", 100, NULL, 5, NULL);
	
	xTaskCreate(MotorTask, "MOTOR", 100, NULL, 5, NULL);
	
	xTaskCreate(RadioTask, "RADIO", 100, NULL, 5, NULL);
	
	xTaskCreate(Usart1Task, "USART1", 200, NULL, 5, NULL);
	
	xTaskCreate(Uart4Task, "UART4", 100, NULL, 5, NULL);
	
	xTaskCreate(Uart5Task, "UART5", 100, NULL, 5, NULL);
	
	// ��ӡʣ���ջ��С
	printf("Free heap after starting: %d bytes\n", xPortGetFreeHeapSize());
	
	// ɾ����ʼ����
	vTaskDelete(startTaskHandle);
	
	// �˳��ٽ���
	taskEXIT_CRITICAL();
} 




// ��������,CPU����ʱִ��
void vApplicationIdleHook( void )
{
//	static u32 tickWatchdogReset = 0;
//	static char pWriteBuffer[2024];

	portTickType tickCount = getSysTickCnt();

//	if (tickCount - tickWatchdogReset > WATCHDOG_RESET_MS)
	{
//		tickWatchdogReset = tickCount;
		watchdogReset();
	}
	
//	vTaskList((char *)&pWriteBuffer);
//	printf("task_name  task_state  priority  stack  tasK_num\n");
//	printf("%s\n", pWriteBuffer);
	
	CanSendMsg(can_buf,8);//����8���ֽ�
	
	__WFI();	/*����͹���ģʽ*/
}





void vApplicationMallocFailedHook( void )
{
	portDISABLE_INTERRUPTS();
	printf("\nMalloc failed!\n");
	LED_TOGGLE;
	while(1);
}

#if (configCHECK_FOR_STACK_OVERFLOW == 1)
void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed portCHAR *pcTaskName)
{
	portDISABLE_INTERRUPTS();
	printf("\nStack overflow!\n");
	LED_TOGGLE;
	while(1);
}
#endif








#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
