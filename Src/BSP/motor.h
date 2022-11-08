#ifndef __MOTOR_H
#define	__MOTOR_H

#include "servo.h"


/*
* ���PWM��ʱ����������
* APB1 prescaler = 2, APB1 Frequency = 36MHz, APB1 TIMER Frequency = 72MHz
* APB2 prescaler = 1, APB2 Frequency = 72MHz, APB2 TIMER Frequency = 72MHz
*/
#define MOTOR_PWM_FREQUENCE  1000            // 1000Hz    
#define MOTOR_PWM_RESOLUTION 1000		         // 1ms = 1000us
#define MOTOR_DEFAULT_DUTY 	 0      				 // 0us
#define MOTOR_TIM_PSC_APB1 ((APB1_TIMER_CLOCKS/MOTOR_PWM_FREQUENCE)/MOTOR_PWM_RESOLUTION-1)

// Բ����
#define MOTOR_PI 3.1415926535
// ����ֱ��,��λ����
#define WHEEL_DIAMETER 65
// �����ܳ�
#define WHEEL_PERIMETER (MOTOR_PI*WHEEL_DIAMETER)
// ���ÿ�ܵ�������
#define MOTOR_PULSE 330
// ����ջ�����ʱ��Ƭ,��λ����
#define MOTOR_TIME_SLICE 20
// ���PID�ջ�����P����
#define MOTOR_PID_P 20
// ���PID�ջ�����d����
#define MOTOR_PID_D 15

/*���״̬�ṹ��*/
typedef struct
{
	int16_t motor1_pwm;						// ���1PWMֵ
	int16_t motor2_pwm;						// ���2PWMֵ
	uint16_t target_speed1;				// ���1Ŀ���ٶ�,����ÿ��,���2λ����Ϊ, ������00:���Ի���,01:��ת,10:��ת,11:����ɲ��,����ο��ֲ���ͨ��Э�鲿��
	uint16_t target_speed2;				// ���2Ŀ���ٶ�,����ÿ��,���2λ����Ϊ, ������00:���Ի���,01:��ת,10:��ת,11:����ɲ��,����ο��ֲ���ͨ��Э�鲿��
	uint16_t current_speed1;			// ���1��ǰ�ٶ�,����ÿ��,���2λ����Ϊ, ������00:���Ի���,01:��ת,10:��ת,11:����ɲ��,����ο��ֲ���ͨ��Э�鲿��
	uint16_t current_speed2;			// ���2��ǰ�ٶ�,����ÿ��,���2λ����Ϊ, ������00:���Ի���,01:��ת,10:��ת,11:����ɲ��,����ο��ֲ���ͨ��Э�鲿��
	int16_t motor1_pulse;					// ���1����������һ��ʱ��Ƭ�ڵ�������,����ǰ������,ÿ���β���֮���ʱ���Ϊ1��ʱ��Ƭ
	int16_t motor2_pulse;					// ���1����������һ��ʱ��Ƭ�ڵ�������,����ǰ������,ÿ���β���֮���ʱ���Ϊ1��ʱ��Ƭ
	int32_t motor1_pulse_total;		// ���1�����ܼ�
	int32_t motor2_pulse_total;		// ���2�����ܼ�
	uint8_t motor1_status; 				// ���1״̬,0:����,1:�������쳣
	uint8_t motor2_status; 				// ���2״̬,0:����,1:�������쳣
}motorStatus_t;




/**************************��������********************************/
//void Motor_PWM_Duty(uint8_t channel, uint16_t duty);	// ��ֹ�ⲿ����
void Motor_Init(void);
//void Motor1_Brake(void);															// ��ֹ�ⲿ����
//void Motor2_Brake(void);															// ��ֹ�ⲿ����
//void Set_Motor_PWM(int16_t motor1, int16_t motor2);		// ��ֹ�ⲿ����
void MotorTask(void *param);

#endif /* __MOTOR_H */

