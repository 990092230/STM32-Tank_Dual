#ifndef __LED_H
#define	__LED_H


#include "stm32f10x.h"


/* ����LED���ӵ�GPIO�˿�, �û�ֻ��Ҫ�޸�����Ĵ��뼴�ɸı�LED���ŵ����� */


// B-��ɫ
#define LED_GPIO_PORT    	GPIOC			              /* GPIO�˿� */
#define LED_GPIO_CLK 	    RCC_APB2Periph_GPIOC		/* GPIO�˿�ʱ�� */
#define LED_GPIO_PIN			GPIO_Pin_4			        /* GPIO���� */


/** the macro definition to trigger the led on or off 
  * 1 - off
  *0 - on
  */
#define ON  0
#define OFF 1

/* ʹ�ñ�׼�Ĺ̼������IO*/
#define LED(a)	if (a)	\
					GPIO_SetBits(LED_GPIO_PORT, LED_GPIO_PIN);\
					else		\
					GPIO_ResetBits(LED_GPIO_PORT, LED_GPIO_PIN)


/* ֱ�Ӳ����Ĵ����ķ�������IO */
#define GPIO_Toggle(p,i) {p->ODR ^=i;} //�����ת״̬


/* �������IO�ĺ� */
#define LED_TOGGLE		 GPIO_Toggle(LED_GPIO_PORT,LED_GPIO_PIN)
#define LED_OFF		   	 GPIO_SetBits(LED_GPIO_PORT, LED_GPIO_PIN)
#define LED_ON			   GPIO_ResetBits(LED_GPIO_PORT, LED_GPIO_PIN)



void LED_Init(void);

#endif /* __LED_H */
