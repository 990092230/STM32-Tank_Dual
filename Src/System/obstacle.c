/**
  ******************************************************************************
  * @file    obstacle.c
  * @author  Oskar Wei
  * @version V1.0
  * @date    2021-05-12
  * @brief   obstacle avoidance
  ******************************************************************************
  * @attention
  *
  * Mail: 990092230@qq.com
  * Blog: www.mindsilicon.com
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


extern uint16_t ultrasonic_distance1;				// ǰ����������
extern uint16_t ultrasonic_distance2;				// ����������




// �ϰ������㷨
void ObstacleAvoidance(int16_t *pwm1, int16_t *pwm2)
{
	int16_t pwm1_temp, pwm2_temp;
	
	pwm1_temp = *pwm1;
	pwm2_temp = *pwm2;
	
	if(ultrasonic_distance1 < 150)
	{
		if(pwm1_temp > 0)
		{
			pwm1_temp = 0;
			pwm2_temp = 0;
		}
	}
	
	
	if(ultrasonic_distance2 < 150)
	{
		if(pwm1_temp < 0)
		{
			pwm1_temp = 0;
			pwm2_temp = 0;
		}
	}
	
	
	
	*pwm1 = pwm1_temp;
	*pwm2 = pwm2_temp;
	
}




















