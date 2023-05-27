/**
  ******************************************************************************
  * @file    motor.c
  * @author  Oskar Wei
  * @version V1.0
  * @date    2020-06-06
  * @brief   motor control api
  ******************************************************************************
  * @attention
  *
  * Mail: 990092230@qq.com
  * Shop: www.mindsilicon.com
  *
	* �ó������ѧϰʹ�ã�δ�����������������������κ���;
  ******************************************************************************
  */
	
#include "can.h"
#include "radio.h"
#include "motor.h"
#include "i2c_oled.h"
#include "usart1.h"
#include "stdio.h"
#include "stm32f10x_it.h"

#include "FreeRTOS.h"
#include "task.h"

extern Menu menu;
extern servoPWM_t servoPWM;

motorStatus_t motorStatus = 
{
	.motor1_pwm = 0,					// ���1PWMֵ
	.motor2_pwm = 0,					// ���2PWMֵ
	.target_speed1 = 0,				// ���1Ŀ���ٶ�,����ÿ��,���2λ����Ϊ, ������00:���Ի���,01:��ת,10:��ת,11:����ɲ��,����ο��ֲ���ͨ��Э�鲿��
	.target_speed2 = 0,				// ���2Ŀ���ٶ�,����ÿ��,���2λ����Ϊ, ������00:���Ի���,01:��ת,10:��ת,11:����ɲ��,����ο��ֲ���ͨ��Э�鲿��
	.current_speed1 = 0,			// ���1��ǰ�ٶ�,����ÿ��,���2λ����Ϊ, ������00:���Ի���,01:��ת,10:��ת,11:����ɲ��,����ο��ֲ���ͨ��Э�鲿��
	.current_speed2 = 0,			// ���2��ǰ�ٶ�,����ÿ��,���2λ����Ϊ, ������00:���Ի���,01:��ת,10:��ת,11:����ɲ��,����ο��ֲ���ͨ��Э�鲿��
	.motor1_pulse = 0,				// ���1����������һ��ʱ��Ƭ�ڵ�������,����ǰ������,ÿ���β���֮���ʱ���Ϊ1��ʱ��Ƭ
	.motor2_pulse = 0,				// ���1����������һ��ʱ��Ƭ�ڵ�������,����ǰ������,ÿ���β���֮���ʱ���Ϊ1��ʱ��Ƭ
	.motor1_pulse_total = 0,	// ���1�����ܼ�
	.motor2_pulse_total = 0,	// ���2�����ܼ�
	.motor1_status = 0, 			// ���1״̬,0:����,1:�������쳣
	.motor2_status = 0  			// ���2״̬,0:����,1:�������쳣
};



// ��ʼ�������������1�Ķ�ʱ�����Ա���A4950�������оƬ������ʵ�PWM�ź�
void TIM1_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  // ����Ƚ�ͨ��GPIO ��ʼ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/*--------------------ʱ���ṹ���ʼ��-------------------------*/
	// �������ڣ���������Ϊ1ms
	
	// �Զ���װ�ؼĴ�����ֵ���ۼ�TIM_Period+1��Ƶ�ʺ����һ�����»����ж�
	TIM_TimeBaseStructure.TIM_Period = MOTOR_PWM_RESOLUTION;	
	// ����CNT��������ʱ�� = Fck_int/(psc+1)
	TIM_TimeBaseStructure.TIM_Prescaler = MOTOR_TIM_PSC_APB1;	
	// ʱ�ӷ�Ƶ���ӣ���������ʱ��ʱ��Ҫ�õ�
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;		
	// ����������ģʽ������Ϊ���ϼ���
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;		
	// �ظ���������ֵ��û�õ����ù�
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;	
	// ��ʼ����ʱ��
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/*--------------------����ȽϽṹ���ʼ��-------------------*/	
	
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	// ����ΪPWMģʽ1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	// ���ʹ��
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	// ���ͨ����ƽ��������	
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	// ����Ƚ�ͨ�� 1
	TIM_OCInitStructure.TIM_Pulse = MOTOR_DEFAULT_DUTY;
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
	// ����Ƚ�ͨ�� 4
	TIM_OCInitStructure.TIM_Pulse = MOTOR_DEFAULT_DUTY;
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
	//TIM_ARRPreloadConfig(TIM1, ENABLE);
	// ʹ�ܼ�����
	TIM_Cmd(TIM1, ENABLE);
	
	// �����ʹ�ܣ���ʹ�õ���ͨ�ö�ʱ��ʱ����䲻��Ҫ
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}



// ��ʼ�������������2�Ķ�ʱ�����Ա���A4950�������оƬ������ʵ�PWM�ź�
void TIM2_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
  // ����Ƚ�ͨ��GPIO ��ʼ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// ������ʱ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/*--------------------ʱ���ṹ���ʼ��-------------------------*/
	// �������ڣ���������Ϊ1ms
	
	// �Զ���װ�ؼĴ�����ֵ���ۼ�TIM_Period+1��Ƶ�ʺ����һ�����»����ж�
	TIM_TimeBaseStructure.TIM_Period = MOTOR_PWM_RESOLUTION;	
	// ����CNT��������ʱ�� = Fck_int/(psc+1)
	TIM_TimeBaseStructure.TIM_Prescaler = MOTOR_TIM_PSC_APB1;	
	// ʱ�ӷ�Ƶ���ӣ���������ʱ��ʱ��Ҫ�õ�
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;		
	// ����������ģʽ������Ϊ���ϼ���
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;		
	// �ظ���������ֵ��û�õ����ù�
	TIM_TimeBaseStructure.TIM_RepetitionCounter=0;	
	// ��ʼ����ʱ��
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/*--------------------����ȽϽṹ���ʼ��-------------------*/	
	
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	// ����ΪPWMģʽ1
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	// ���ʹ��
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	// ���ͨ����ƽ��������	
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	// ����Ƚ�ͨ�� 3
	TIM_OCInitStructure.TIM_Pulse = MOTOR_DEFAULT_DUTY;
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);
	
	// ����Ƚ�ͨ�� 4
	TIM_OCInitStructure.TIM_Pulse = MOTOR_DEFAULT_DUTY;
	TIM_OC4Init(TIM2, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);
	
	//TIM_ARRPreloadConfig(TIM2, ENABLE);
	
	// ʹ�ܼ�����
	TIM_Cmd(TIM2, ENABLE);
}


// ��ʼ�����1�ı�����,��������TIM5
void Motor1_Encoder_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
  TIM_ICInitTypeDef TIM_ICInitStructure;  
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);	// ʹ�ܶ�ʱ��5��ʱ��
	
	/*ʹ��GPIOA��AFIO����ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	
	/*��ʼ��PA0��PA1�˿�ΪIN_FLOATINGģʽ*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Prescaler = 0;			// Ԥ��Ƶ�� 
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;		// �趨�������Զ���װֵ
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;				// ѡ��ʱ�ӷ�Ƶ������Ƶ
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		// TIM���ϼ���  
  TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
  TIM_EncoderInterfaceConfig(TIM5, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);	// ʹ�ñ�����ģʽ
	
	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInit(TIM5, &TIM_ICInitStructure);
	
  TIM_ClearFlag(TIM5, TIM_FLAG_Update);				// ���TIM�ĸ��±�־λ
  TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	
  TIM_SetCounter(TIM5, 0);		// ���������
  TIM_Cmd(TIM5, ENABLE); 
}


// ��ʼ�����2�ı�����,��������TIM4
void Motor2_Encoder_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
  TIM_ICInitTypeDef TIM_ICInitStructure;  
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);	// ʹ�ܶ�ʱ��5��ʱ��
	
	/*ʹ��GPIOB��AFIO����ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	
	/*��ʼ��PB6��PB7�˿�ΪIN_FLOATINGģʽ*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Prescaler = 0;			// Ԥ��Ƶ�� 
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;		// �趨�������Զ���װֵ
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;				// ѡ��ʱ�ӷ�Ƶ������Ƶ
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;		// TIM���ϼ���  
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  TIM_EncoderInterfaceConfig(TIM4, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);	// ʹ�ñ�����ģʽ
	
	TIM_ICStructInit(&TIM_ICInitStructure);
	TIM_ICInitStructure.TIM_ICFilter = 10;
	TIM_ICInit(TIM4, &TIM_ICInitStructure);
	
  TIM_ClearFlag(TIM4, TIM_FLAG_Update);				// ���TIM�ĸ��±�־λ
  TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	
  TIM_SetCounter(TIM4, 0);		// ���������
  TIM_Cmd(TIM4, ENABLE); 
}

int16_t motor1_cnt = 0, motor2_cnt = 0;

// �������ܣ�TIM5�жϷ�����
void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5, TIM_IT_Update)!=RESET)
	{
		TIM_ClearITPendingBit(TIM5, TIM_IT_Update);  // ����жϱ�־λ
	}	    
}


// �������ܣ�TIM4�жϷ�����
void TIM4_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update)!=RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  // ����жϱ�־λ
	}
}


// �������ܣ���ȡ��λʱ���ڵı���������,���ڼ�����ת��
void Read_Encoder(void)
{
	// ��ȡ���1������,ע����������short,����������ת���������
	motorStatus.motor1_pulse = -(short)TIM5 -> CNT;
	TIM5 -> CNT = 0;
	
	motorStatus.motor2_pulse = (short)TIM4 -> CNT;
	TIM4 -> CNT = 0;
}




// �����PWMֵӦ�õ���ʱ��, ʹ��ʱ�����Ŀ��PWM����
void Motor_PWM_Duty(uint8_t channel, uint16_t duty)
{
	if(channel > 4 || channel < 1)
	{
		return;
	}
	
	if(duty > 1000)
	{
		return;
	}
	
	switch(channel)
	{
		case 1: TIM1->CCR1 = duty; break;
		case 2: TIM1->CCR4 = duty; break;
		case 3: TIM2->CCR3 = duty; break;
		case 4: TIM2->CCR4 = duty; break;
	}
}


// ���õ��1��PWMֵ��-1000~1000, PWMֵΪ������ת,Ϊ����ת,Ϊ0��ͣת(����)
void Set_Motor1_PWM(int16_t motor1)
{
	if(motor1 > 1000)
	{
		motor1 = 1000;
	}
	else if(motor1 < -1000)
	{
		motor1 = -1000;
	}
	
	// �����
	if(motor1 >= 0)				// ��ת��ͣת
	{
		Motor_PWM_Duty(1, motor1);
		Motor_PWM_Duty(2, 0);
	}
	else									// ��ת
	{
		Motor_PWM_Duty(1, 0);
		Motor_PWM_Duty(2, -motor1);
	}
}


// ���õ��2��PWMֵ��-1000~1000, PWMֵΪ������ת,Ϊ����ת,Ϊ0��ͣת(����)
void Set_Motor2_PWM(int16_t motor2)
{
	if(motor2 > 1000)
	{
		motor2 = 1000;
	}
	else if(motor2 < -1000)
	{
		motor2 = -1000;
	}
	
	// �����
	if(motor2 >= 0)				// ��ת��ͣת
	{
		Motor_PWM_Duty(3, 0);
		Motor_PWM_Duty(4, motor2);
	}
	else									// ��ת
	{
		Motor_PWM_Duty(3, -motor2);
		Motor_PWM_Duty(4, 0);
	}
}



// �����ʼ��
void Motor_Init(void)
{
	// ��ʼ�����PWM��ʱ��
	TIM1_Init();
	TIM2_Init();
	
	// �رյ��1�͵��2,ʹ�䴦�ڹ��Ի���״̬
	Set_Motor1_PWM(0);
	Set_Motor2_PWM(0);
	
	// ��ʼ�����1������
	Motor1_Encoder_Init();
	
	// ��ʼ�����2������
	Motor2_Encoder_Init();

}


// ����ɲ��, ���÷���ЧӦʹ�������
void Motor1_Brake(void)
{
	Motor_PWM_Duty(1, MOTOR_PWM_RESOLUTION);
	Motor_PWM_Duty(2, MOTOR_PWM_RESOLUTION);
}

// ����ɲ��, ���÷���ЧӦʹ�������
void Motor2_Brake(void)
{
	Motor_PWM_Duty(3, MOTOR_PWM_RESOLUTION);
	Motor_PWM_Duty(4, MOTOR_PWM_RESOLUTION);
}

/*
 * �����ٶȼ�����PWMֵ,�ٶȵ�λ:����ÿ��
 * ������Ҫ�õ�PID�ջ������㷨,�Ե�����ٶȽ��п���
 *
 */
void Motor1_Speed_Control(void)
{
	// �ϴεĵ������ƫ��
	static int16_t motor1_pulse_last_bias = 0;
	
	// ���εĵ������ƫ��,��������Ŀ���������͵�ǰ��������ƫ��
	int16_t motor1_pulse_bias = 0, int16_temp = 0;
	
	/* 
	 * ����Ŀ�����ٶȼ���Ŀ��ת��,Ҳ����һ��ʱ��Ƭ����Ҫ�ﵽ��������,
	 * ��Ϊ���PID�㷨���Ƶ�Ŀ��������ǵ����һ��ʱ��Ƭ����Ҫת����������
	 * ���ٶȵ�λ����ÿ��,ת�ٵ�λΪ������ÿʱ��Ƭ
	 */
	int16_t motor1_pulse_target = 0;
	
	// �ж��Ƿ�Ҫ������Ի��л�������ɲ��״̬
	// target_speed�����2λ��������Ϊ, ������00:���Ի���,01:��ת,10:��ת,11:����ɲ��,����ο��ֲ���ͨ��Э�鲿��
	uint8_t motor1_mode = 0;
	int8_t motor1_sign = 1;
	
	motor1_mode = motorStatus.target_speed1 >> 14;
	
	// ���ݲ�ͬ״̬������Ӧ����
	if(motor1_mode == 0)				// ���Ի���
	{
		motorStatus.motor1_pwm = 0;
		Set_Motor1_PWM(motorStatus.motor1_pwm);
		return;
	}
	else if(motor1_mode == 3)		// ����ɲ��
	{
		motorStatus.motor1_pwm = 0;
		Motor1_Brake();
		return;
	}
	else if(motor1_mode == 1)		// ��ת
	{
		motor1_sign = 1;
	}
	else if(motor1_mode == 2)		// ��ת
	{
		motor1_sign = -1;
	}
	
	// Ŀ��������
	motor1_pulse_target = motor1_sign * ((motorStatus.target_speed1 & 0x3FFF) / WHEEL_PERIMETER * MOTOR_PULSE) / (1000 / MOTOR_TIME_SLICE);
	
//	printf("t:%d, p:%d\n", motor1_pulse_target, motorStatus.motor1_pulse);
	
	// ��������ƫ��
	motor1_pulse_bias = motor1_pulse_target - motorStatus.motor1_pulse;
	
	// ����PWMֵ
	int16_temp = motorStatus.motor1_pwm + motor1_pulse_bias * MOTOR_PID_P - motor1_pulse_last_bias * MOTOR_PID_D;
	if(int16_temp > 1000)
	{
		int16_temp = 1000;
	}
	else if(int16_temp < -1000)
	{
		int16_temp = -1000;
	}
	
	motorStatus.motor1_pwm = int16_temp;
	
	motor1_pulse_last_bias = motor1_pulse_bias;
	
	Set_Motor1_PWM(motorStatus.motor1_pwm);
}

/*
 * �����ٶȼ�����PWMֵ,�ٶȵ�λ:����ÿ��
 * ������Ҫ�õ�PID�ջ������㷨,�Ե�����ٶȽ��п���
 *
 */
void Motor2_Speed_Control(void)
{
	// �ϴεĵ������ƫ��
	static int16_t motor2_pulse_last_bias = 0;
	
	// ���εĵ������ƫ��,��������Ŀ���������͵�ǰ��������ƫ��
	int16_t motor2_pulse_bias = 0, int16_temp = 0;
	
	/* 
	 * ����Ŀ�����ٶȼ���Ŀ��ת��,Ҳ����һ��ʱ��Ƭ����Ҫ�ﵽ��������,
	 * ��Ϊ���PID�㷨���Ƶ�Ŀ��������ǵ����һ��ʱ��Ƭ����Ҫת����������
	 * ���ٶȵ�λ����ÿ��,ת�ٵ�λΪ������ÿʱ��Ƭ
	 */
	int16_t motor2_pulse_target = 0;
	
	// �ж��Ƿ�Ҫ������Ի��л�������ɲ��״̬
	// target_speed�����2λ����Ϊ, ������00:���Ի���,01:��ת,10:��ת,11:����ɲ��,����ο��ֲ���ͨ��Э�鲿��
	uint8_t motor2_mode = 0;
	int8_t motor2_sign = 1;
	
	motor2_mode = motorStatus.target_speed2 >> 14;
	
	// ���ݲ�ͬ״̬������Ӧ����
	if(motor2_mode == 0)				// ���Ի���
	{
		motorStatus.motor2_pwm = 0;
		Set_Motor2_PWM(motorStatus.motor2_pwm);
		return;
	}
	else if(motor2_mode == 3)		// ����ɲ��
	{
		motorStatus.motor2_pwm = 0;
		Motor2_Brake();
		return;
	}
	else if(motor2_mode == 1)		// ��ת
	{
		motor2_sign = 1;
	}
	else if(motor2_mode == 2)		// ��ת
	{
		motor2_sign = -1;
	}
	
	// Ŀ��������
	motor2_pulse_target = motor2_sign * ((motorStatus.target_speed2 & 0x3FFF) / WHEEL_PERIMETER * MOTOR_PULSE) / (1000 / MOTOR_TIME_SLICE);
	
//	printf("t:%d, p:%d\n", motor2_pulse_target, motorStatus.motor2_pulse);
	
	// ��������ƫ��
	motor2_pulse_bias = motor2_pulse_target - motorStatus.motor2_pulse;
	
	// ����PWMֵ
	int16_temp = motorStatus.motor2_pwm + motor2_pulse_bias * MOTOR_PID_P - motor2_pulse_last_bias * MOTOR_PID_D;
	if(int16_temp > 1000)
	{
		int16_temp = 1000;
	}
	else if(int16_temp < -1000)
	{
		int16_temp = -1000;
	}
	
	motorStatus.motor2_pwm = int16_temp;
	
	motor2_pulse_last_bias = motor2_pulse_bias;
	
	Set_Motor2_PWM(motorStatus.motor2_pwm);
}

// ��������ǰ������ٶ�,��λ����ÿ��
void Motor_Speed_Calculate(void)
{
	int16_t m = 0;
	m = ((float)(motorStatus.motor1_pulse * (1000 / MOTOR_TIME_SLICE)) / MOTOR_PULSE * WHEEL_PERIMETER);
	if(m > 0)
	{
		motorStatus.current_speed1 = (1 << 14) | (m & 0x3FFF);
	}
	else if(m < 0)
	{
		motorStatus.current_speed1 = (2 << 14) | (m & 0x3FFF);
	}
	else if(m == 0)
	{
		motorStatus.current_speed1 = motorStatus.target_speed1 & 0xC000;
	}
	
	m = ((float)(motorStatus.motor2_pulse * (1000 / MOTOR_TIME_SLICE)) / MOTOR_PULSE * WHEEL_PERIMETER);
	if(m > 0)
	{
		motorStatus.current_speed2 = (1 << 14) | (m & 0x3FFF);
	}
	else if(m < 0)
	{
		motorStatus.current_speed2 = (2 << 14) | (m & 0x3FFF);
	}
	else if(m == 0)
	{
		motorStatus.current_speed2 = motorStatus.target_speed2 & 0xC000;
	}
	
}

extern uint8_t can_buf[8];

void MotorTask(void *param)
{
	while(menu == RUNNING)
	{
		vTaskDelay(100);
	}
	
//	Motor_Init();
	
	while(1)
	{
		if(menu == MPU_INIT || menu == SHUTDOWN || menu == START)
		{
			Set_Motor1_PWM(0);
			Set_Motor2_PWM(0);
			vTaskDelay(50);
		}
		else
		{
			Read_Encoder();
			Motor_Speed_Calculate();
			
			if(radiolinkConnectStatus())		// ����Ƿ����ֱ���������,����ֱ��Ѿ�����,�����ֱ�ָ������,������������
			{
				Set_Motor1_PWM(motorStatus.motor1_pwm);
				Set_Motor2_PWM(motorStatus.motor2_pwm);
				Set_Servo_PWM(1, servoPWM.servo1);
				Set_Servo_PWM(2, servoPWM.servo2);
				Set_Servo_PWM(3, servoPWM.servo3);
				Set_Servo_PWM(4, servoPWM.servo4);
				Set_Servo_PWM(5, servoPWM.servo5);
				Set_Servo_PWM(6, servoPWM.servo6);
				Set_Servo_PWM(7, servoPWM.servo7);
				Set_Servo_PWM(8, servoPWM.servo8);
			}
			else if(Usart1LinkStatus())			// ���USART1�Ƿ���������λ������������,�������ִ����λ��ָ��
			{
				Motor1_Speed_Control();
				Motor2_Speed_Control();
				Set_Servo_PWM(1, servoPWM.servo1);
				Set_Servo_PWM(2, servoPWM.servo2);
				Set_Servo_PWM(3, servoPWM.servo3);
				Set_Servo_PWM(4, servoPWM.servo4);
				Set_Servo_PWM(5, servoPWM.servo5);
				Set_Servo_PWM(6, servoPWM.servo6);
				Set_Servo_PWM(7, servoPWM.servo7);
				Set_Servo_PWM(8, servoPWM.servo8);
			}
			else														// ���ߺ���λ���������Ӷ��ѶϿ�,�رյ�����
			{
				motorStatus.motor1_pwm = 0;
				motorStatus.motor2_pwm = 0;
				Set_Motor1_PWM(0);
				Set_Motor2_PWM(0);
			}
			vTaskDelay(MOTOR_TIME_SLICE);
		}
		
		CanSendMsg(can_buf,8);//����8���ֽ�
		
	}
}

/*********************************************END OF FILE**********************/
