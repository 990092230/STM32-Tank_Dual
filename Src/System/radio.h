#ifndef __RADIO_H
#define	__RADIO_H

#include "stm32f10x.h"
#include <stdbool.h>
#include "nrf24l01.h"



/* Ĭ�����ò��� */
#define  VERSION	11		/*��ʾ�汾ΪV1.1*/
#define  DISPLAY_LANGUAGE	ENGLISH

#define  RADIO_CHANNEL 		0
#define  RADIO_DATARATE 	DATA_RATE_250K
#define  RADIO_ADDRESS 		{0x11, 0x22, 0x33, 0x44, 0x55}

enum language
{
	SIMPLE_CHINESE,
	ENGLISH,
	COMPLEX_CHINESE,
};

/*�������ýṹ*/
typedef struct
{
	uint8_t channel;		
	enum nrfRate dataRate;
	uint8_t addr[5];
}radioConfig_t;


/*��������ṹ*/
typedef struct
{
	uint8_t version;				/*����汾��*/
	enum language language;	/*��ʾ����*/
	radioConfig_t radio;		/*�������ò���*/
	uint8_t cksum;					/*У��*/
	uint32_t fs_mark;				// file system mark, �ļ�ϵͳ���, �����ж�����spi flash���Ƿ��Ѿ��������ļ�ϵͳ
													// ����Ѿ��������ļ�ϵͳ, fs_mark�ᱻ����Ϊ0xAABBCCDD, ��ʱ
													// Ӧ�ý�ֹ��ʽ��spi flashоƬ, ���ⶪʧоƬ�ڵ���Դ�ļ�
}systemConfig_t;


 /*
 ͨ�������ʽ
 ֡ͷ��2�ֽڣ� ͨ�Ŵ��루1�ֽڣ� ���ݳ��ȣ�����֡ͷ��ͨ�Ŵ����У�飩 �������ݣ������ֽڣ� У�飨2�ֽڣ�
 ����
 0xAA 0xAA 0x01 0x08 nByteData 0x__ 0x__
 */
#define RADIO_MSG_MAX_SIZE 32

#define HEADER_BYTE1 0xAA
#define HEADER_BYTE2 0xAA


typedef enum
{
	CMD_UNDEF = 0x00,		// δ��������,Ԥ��
	// Control Board�����ư�,��Ϊ���ջ��ܶ�ʱ��ʱװ�ڻ��߼����ڿ��ư��ϵ�,���Գ���Contol Boardָ�����ջ�Receiver
	
	/* ***********�ֱ��ͽ��ջ�֮���ͨ��********** */
	// Command, Remote Controller to Control Board (Receiver),���ֱ��������ջ�����������
	CMD_RTCB1 = 0x01,		// ��ң���ֱ����͵����ջ�������1, ���ڷ�������ҡ�˺Ͱ�����״̬����
	CMD_RTCB2 = 0x02,		// ��ң���ֱ����͵����ջ�������2
	CMD_RTCB3 = 0x03,		// ��ң���ֱ����͵����ջ�������3
	CMD_RTCB4 = 0x04,		// ��ң���ֱ����͵����ջ�������4
	CMD_RTCB5 = 0x05,		// ��ң���ֱ����͵����ջ�������5
	CMD_RTCB6 = 0x06,		// ��ң���ֱ����͵����ջ�������6
	CMD_RTCB7 = 0x07,		// ��ң���ֱ����͵����ջ�������7
	CMD_RTCB8 = 0x08,		// ��ң���ֱ����͵����ջ�������8
	
	// Response, Control Board (Receiver) to Remote Controller,�ӽ��ջ������ֱ��ķ�������
	RES_RTCB1 = 0x09,		// ���ջ���RTCB1����Ӧ, ���ڻش����ݴ�����
	RES_RTCB2 = 0x0A,		// ���ջ���RTCB2����Ӧ, ���ڷ������ݸ���λ��
	RES_RTCB3 = 0x0B,		// ���ջ���RTCB3����Ӧ
	RES_RTCB4 = 0x0C,		// ���ջ���RTCB4����Ӧ
	RES_RTCB5 = 0x0D,		// ���ջ���RTCB5����Ӧ
	RES_RTCB6 = 0x0E,		// ���ջ���RTCB6����Ӧ
	RES_RTCB7 = 0x0F,		// ���ջ���RTCB7����Ӧ
	RES_RTCB8 = 0x10,		// ���ջ���RTCB8����Ӧ
	
	
	/* ***********��������,������λ�����ý��ջ�,�ֱ�����ư�,����ʱ�����õ��豸��Ҫ��USB����λ��ֱ��********** */
	// Command, APP to Control Board (Receiver), ����λ�����͵����ջ�,�ֱ����߿��ư�
	CMD_APPTCB1 = 0x11,	// ����λ�����͵����ջ����ֱ�������1, ��ȡ��ַ��Ƶ��
	CMD_APPTCB2 = 0x12,	// ����λ�����͵����ջ����ֱ�������2, д���ַ
	CMD_APPTCB3 = 0x13,	// ����λ�����͵����ջ����ֱ�������3, д��Ƶ��
	CMD_APPTCB4 = 0x14,	// ����λ�����͵�        �ֱ�������4, У׼�ֱ�, ����У׼��һ�׶�, ����У׼�ڶ��׶�, ����У׼������˳�У׼
	CMD_APPTCB5 = 0x15,	// ����λ�����͵����ջ����ֱ�������5, ͸������, ��λ���������ݸ����ջ�
	CMD_APPTCB6 = 0x16,	// ����λ�����͵����ջ�      ������6, ����8·���PWMֵ�͵��ת��,���ת�ٻ���ÿ��,����2·��Դ�������
	CMD_APPTCB7 = 0x17,	// ����λ�����͵����ջ�      ������7, ��ȡ״̬��Ϣ, ����:8·���PWMֵ�͵��ת��(rad/s), 
	CMD_APPTCB8 = 0x18,	// ����λ�����͵����ջ����ֱ�������8, У׼������ٶȴ�����,֪ͨ��λ������ԭʼ����, ����У׼�������λ��
	
	// Response, Control Board (Receiver) to APP, ���ջ�,�ֱ����߿��ư巴������λ��������
	RES_APPTCB1 = 0x19,	// ���ջ����ֱ���APPTCB1����Ӧ, ���ص�ַ��Ƶ��
	RES_APPTCB2 = 0x1A,	// ���ջ����ֱ���APPTCB2����Ӧ, ���ص�ַд��״̬
	RES_APPTCB3 = 0x1B,	// ���ջ����ֱ���APPTCB3����Ӧ, ����Ƶ��д��״̬
	RES_APPTCB4 = 0x1C,	//         �ֱ���APPTCB4����Ӧ, �޷���
	RES_APPTCB5 = 0x1D,	// ���ջ����ֱ���APPTCB5����Ӧ, ͸������, ���ջ��������ݸ���λ��
	RES_APPTCB6 = 0x1E,	// ���ջ�      ��APPTCB6����Ӧ, �������ý��
	RES_APPTCB7 = 0x1F,	// ���ջ�      ��APPTCB7����Ӧ, ���ض�ȡ״̬��Ϣ, ����:8·���PWMֵ�͵��ת��(rad/s), 
	RES_APPTCB8 = 0x20,	// ���ջ����ֱ���APPTCB8����Ӧ, ����ԭʼ���ٶȴ��������ݸ���λ��, ����У׼״̬
	
	// Command, APP to Control Board (Receiver), ����λ�����͵����ջ�,�ֱ����߿��ư�
	CMD_APPTCB9  = 0x21,	// ����λ�����͵����ջ����ֱ�������, У׼��������ƴ�����,֪ͨ��λ������ԭʼ����, ����У׼�������λ��
	CMD_APPTCB10 = 0x22,	// ����λ�����͵����ջ����ֱ�������, У׼���������Ǵ�����,֪ͨ��λ������ԭʼ����, ����У׼�������λ��
	CMD_APPTCB11 = 0x23,	// ����λ�����͵����ջ����ֱ�������, 
	CMD_APPTCB12 = 0x24,	// ����λ�����͵����ջ����ֱ�������, 
	CMD_APPTCB13 = 0x25,	// ����λ�����͵����ջ����ֱ�������, 
	CMD_APPTCB14 = 0x26,	// ����λ�����͵����ջ�      ������, 
	CMD_APPTCB15 = 0x27,	// ����λ�����͵����ջ�      ������, 
	CMD_APPTCB16 = 0x28,	// ����λ�����͵����ջ����ֱ�������,
	
	// Response, Control Board (Receiver) to APP, ���ջ�,�ֱ����߿��ư巴������λ��������
	RES_APPTCB9  = 0x29,	// ���ջ����ֱ���APPTCB1����Ӧ, ����ԭʼ�����ƴ��������ݸ���λ��, ����У׼״̬
	RES_APPTCB10 = 0x2A,	// ���ջ����ֱ���APPTCB2����Ӧ, ����ԭʼ�����Ǵ��������ݸ���λ��, ����У׼״̬
	RES_APPTCB11 = 0x2B,	// ���ջ����ֱ���APPTCB3����Ӧ, 
	RES_APPTCB12 = 0x2C,	// ���ջ����ֱ���APPTCB4����Ӧ, 
	RES_APPTCB13 = 0x2D,	// ���ջ����ֱ���APPTCB5����Ӧ, 
	RES_APPTCB14 = 0x2E,	// ���ջ�      ��APPTCB6����Ӧ, 
	RES_APPTCB15 = 0x2F,	// ���ջ�      ��APPTCB7����Ӧ, 
	RES_APPTCB16 = 0x30,	// ���ջ����ֱ���APPTCB8����Ӧ
	
}msgID_e;

/*ͨѶ���ݽṹ radio message*/
typedef struct
{
	uint16_t head;
	uint8_t msgID;
	uint8_t dataLen;
	uint8_t data[RADIO_MSG_MAX_SIZE];
	uint16_t checksum;
}radioMsg_t;


void radiolinkInit(void);
bool radiolinkSendPacket(const radioMsg_t *p);
bool radiolinkSendPacketBlocking(const radioMsg_t *p);
bool radiolinkReceivePacket(radioMsg_t *p);
bool radiolinkReceivePacketBlocking(radioMsg_t *p);
void RadioTask(void* param);
uint16_t radioinkFailRxcount(void);
bool radiolinkConnectStatus(void);
void radiolinkEnable(FunctionalState state);


void configParamInit(void);
void configParamTask(void* param);
void writeConfigParamToFlash(void);
void configParamReset(void);

void ParseRadioMsg(radioMsg_t *msg);

#endif /* __RADIO_H */
