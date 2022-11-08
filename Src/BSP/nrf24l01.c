/**
  ******************************************************************************
  * @file    nrf24l01.c
  * @author  Oskar Wei
  * @version V1.0
  * @date    2020-06-01
  * @brief   nrf24l01 driver
  ******************************************************************************
  * @attention
  *
  * Mail: 990092230@qq.com
  * Shop: www.mindsilicon.com
  *
	* �ó������ѧϰʹ�ã�δ�����������������������κ���;
  ******************************************************************************
  */

#include "radio.h"
#include "nrf24l01.h"
#include <stdio.h>

static void (*interruptCb)(void) = 0;

/***************************NRF24L01+��������***********************************/

/* NRF��ʼ����ʹ��STM32��SPI3 */
static void NRF_LowLevel_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE); 

	/* ����SPI3��SCK(PB3),MISO(PB4),MOSI(PB5)���� */ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* ����NRF��CE(PC5),NSS(PA15)���� */ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* ����NRF��IRQ����(PC13) */ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/* ������IRQ�ⲿ�ж� */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);
	EXTI_InitStructure.EXTI_Line = EXTI_Line13;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
  
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  	/* ����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫�� */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;													/* ����SPI����ģʽ:����Ϊ��SPI */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;											/* ����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;														/* ѡ���˴���ʱ�ӵ���̬:ʱ�����յ� */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;													/* ���ݲ����ڵ�һ��ʱ���� */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;															/* NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ���� */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;		/*���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ4 */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;										/* ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ */
	SPI_InitStructure.SPI_CRCPolynomial = 7;															/* CRCֵ����Ķ���ʽ */
	SPI_Init(SPI3, &SPI_InitStructure);  																	/* ����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ��� */
 
	SPI_Cmd(SPI3, ENABLE);	/*ʹ��SPI����*/
	
	SPI3_NSS_H();
	NRF_CE_L();
}

static uint8_t SPI_RWByte(SPI_TypeDef* SPIx , uint8_t TxData)
{			
	/* ͨ������SPIx����һ������ */
	SPI_I2S_SendData(SPIx, TxData);
	/* ���ָ����SPI��־λ�������:���ͻ���ձ�־λ*/
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET);
	
	/* ���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ */
	while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_RXNE) == RESET);
	
	/* ����ͨ��SPIx������յ����� */
	return SPI_I2S_ReceiveData(SPIx); 	
}

/* д�Ĵ��� */
static uint8_t writeReg(uint8_t reg, uint8_t value)
{
	uint8_t status;	
	SPI3_NSS_L();                	 	
	status = SPI_RWByte(NRF_SPI, reg | CMD_W_REG);
	SPI_RWByte(NRF_SPI, value); 	
	SPI3_NSS_H();                 	  
	return status;       					
}

/* ���Ĵ��� */ 
static uint8_t readReg(uint8_t reg)
{
	uint8_t reg_val;	    
 	SPI3_NSS_L();         		 			
	SPI_RWByte(NRF_SPI, reg | CMD_R_REG);
	reg_val = SPI_RWByte(NRF_SPI, 0xA5);
	SPI3_NSS_H();     								    
	return reg_val;    						
}	

/* �������� */
static uint8_t readBuf(uint8_t cmd, uint8_t *pBuf, uint8_t len)
{
	uint8_t status, i;
	SPI3_NSS_L();            
	status = SPI_RWByte(NRF_SPI, cmd);
	
	for(i = 0; i < len; i++)
	{
		pBuf[i] = SPI_RWByte(NRF_SPI, 0XFF);
	}
	SPI3_NSS_H();
	return status;
}

/* д������ */
static uint8_t writeBuf(uint8_t cmd, uint8_t *pBuf, uint8_t len)
{
	uint8_t status, i;	    
	SPI3_NSS_L();          
	status = SPI_RWByte(NRF_SPI, cmd);
	
	for(i = 0; i < len; i++)
	{
		SPI_RWByte(NRF_SPI, *pBuf++);
	}
	SPI3_NSS_H();
	return status;  
}

/* �������ݰ�(PTXģʽ) */
void nrf_txPacket(uint8_t *tx_buf, uint8_t len)
{	
	NRF_CE_L();	
	writeBuf(CMD_W_TX_PAYLOAD,tx_buf,32);
	NRF_CE_H();		 
	
	
//	uint8_t reg;
//	uint8_t txdata[32] = "sdfhfgghhgfddfgdfdhggh";
//	reg = readReg(REG_CONFIG);
//	writeReg(REG_CONFIG, reg & ~1 );
//	writeReg(CMD_FLUSH_TX,0xff);		/* ��ϴTX_FIFO */
//	NRF_CE_L();	
//	writeBuf(CMD_W_TX_PAYLOAD,tx_buf,32);
//	NRF_CE_H();	
//	
//	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13));
//	
//	reg = readReg(REG_STATUS);
//	writeReg(REG_STATUS ,reg);
	
}


/* �������ݰ�(PTXģʽ) */
void nrf_txPacket_test(uint8_t *tx_buf, uint8_t len)
{
	uint8_t reg;
	uint8_t txdata[32] = "sdfhfgghhgfddfgdfdhggh";
	reg = readReg(REG_CONFIG);
	writeReg(REG_CONFIG, reg & ~1 );
	writeReg(CMD_FLUSH_TX,0xff);		/* ��ϴTX_FIFO */
	NRF_CE_L();	
	writeBuf(CMD_W_TX_PAYLOAD,txdata,32);
	NRF_CE_H();
	
	while(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13));
	
	reg = readReg(REG_STATUS);
	writeReg(REG_STATUS ,reg);
}




/* ����NO_ACK���ݰ�(PTXģʽ) */
void nrf_txPacketNoACK(uint8_t *tx_buf,uint8_t len)
{	
	NRF_CE_L();		 
	writeBuf(CMD_W_TX_PAYLOAD_NO_ACK,tx_buf,len);
	NRF_CE_H();		
}

/* ����ACK���ݰ�������0ͨ��(PRXģʽ) */
void nrf_txPacket_AP(uint8_t *tx_buf,uint8_t len)
{	
	NRF_CE_L();		 	
	writeBuf(CMD_W_ACK_PAYLOAD(0),tx_buf,len);
	NRF_CE_H();		 
}

/* ����NO_ACK���ݰ�(PTXģʽ) */
void nrf_sendPacketNoACK(uint8_t *sendBuf,uint8_t len)
{	
	while((readReg(REG_STATUS)&0x01)!=0);	/* �ȴ�TX_FIFO��Ϊfull */
	nrf_txPacketNoACK(sendBuf,len);			/* ����NO_ACK���ݰ� */
}

/**************************NRF24L01+���ú���***********************************/
/* ���÷�����յ�ַ�������շ���ַһ�� */
void nrf_setAddress(uint8_t address[5])
{
	writeReg(REG_SETUP_AW, 0x03);											// ���õ�ַ���Ϊ5�ֽ�
	writeBuf(CMD_W_REG | REG_RX_ADDR_P0, address, 5);	// ����ʹ��P0�ڵ�
	writeBuf(CMD_W_REG | REG_TX_ADDR, address, 5); 		
}

/* ����Ƶ��ͨ��,channel:0~125 */
void nrf_setChannel(uint8_t channel)
{
	if(channel <= 125)
	{		
		writeReg(REG_RF_CH, channel);
	}
}

/* ���ô������ʣ�dr:0->250KHz��1->1MHz��2->2MHz�� */
void nrf_setDataRate(enum nrfRate dataRate)
{
	uint8_t reg_rf = readReg(REG_RF_SETUP);
	reg_rf &= ~((1<<5)|(1<<3));/* ���ԭ������ */
	switch(dataRate)
	{
		case DATA_RATE_250K:
			reg_rf |= 0x20;
			break;
		case DATA_RATE_1M:
			reg_rf |= 0x00;
			break;
		case DATA_RATE_2M:
			reg_rf |= 0x08;
			break;	
	}
	writeReg(REG_RF_SETUP,reg_rf); 	
}

/* ���÷��书��,power: 0->-18dB  1->-12dB  2->-6dB  3->0dB */
void nrf_setPower(enum nrfPower power)
{
	uint8_t reg_rf = readReg(REG_RF_SETUP);
	reg_rf &= 0xF8;/* ���ԭ�蹦�� */
	switch(power)
	{
		case POWER_M18dBm:
			reg_rf |= 0x00;
			break;
		case POWER_M12dBm:
			reg_rf |= 0x02;
			break;
		case POWER_M6dBm:
			reg_rf |= 0x04;
			break;
		case POWER_0dBm:
			reg_rf |= 0x07;
			break;	
	}
	writeReg(REG_RF_SETUP,reg_rf);
}

/* �����ط�ʱ�������������ʼ��շ��ֽڴ�С���� */
/* ��ϸ˵���ο�nrf24l01.datasheet��P34. */
void nrf_setArd(void)
{
	uint8_t reg_rf, reg_retr;
	reg_rf = readReg(REG_RF_SETUP);
	reg_retr = readReg(REG_SETUP_RETR);
	
	if(!(reg_rf&0x20))	/* ���ʲ���250K(�Ĵ���0x20) */
	{
		reg_retr|= 1<<4;/* (1+1)*250=500us,�ڽ���32�ֽ�ʱ */
	}
	else
	{
		reg_retr|= 5<<4;/* (5+1)*250=1500us,�ڽ���32�ֽ�ʱ */
	}
	
	writeReg(REG_SETUP_RETR,reg_retr);
}

/* �����ط�������arc:0~15 */
void nrf_setArc(uint8_t arc)
{
	uint8_t reg_retr;
	if(arc > 15)
	{
		return;
	}
	
	reg_retr = readReg(REG_SETUP_RETR);
	reg_retr |= arc;
	writeReg(REG_SETUP_RETR, reg_retr);
}

/* ��ȡ���չ��ʼ�� */
uint8_t nrf_getRpd(void)
{
   return readReg(REG_RPD);
}

/* ��ȡ�ط�ʧ�ܴ��� */
uint8_t nrf_getTxRetry(void)
{
   return readReg(REG_OBSERVE_TX)&0x0F;
}


extern systemConfig_t configParam;

/* ��ʼ��NRF24L01���� */
/* model: PTX_MODE��PRX_MODE */
void NRF_Init(enum nrfMode model)
{
	NRF_LowLevel_Init();
	nrf_setAddress(configParam.radio.addr);
	nrf_setChannel(configParam.radio.channel);
	nrf_setDataRate(configParam.radio.dataRate);
	nrf_setPower(RADIO_POWER);		// ���书��
	nrf_setArd();									// �ط�ʱ��������
	nrf_setArc(3);								// �ط�����
	
	if(model == PRX_MODE)
	{
		writeReg(REG_CONFIG, 0x0f);   	/* IRQ�շ�����жϿ���,16λCRC,PRX */
		writeReg(REG_DYNPD,0x01);				/* ʹ��RX_P0��̬����PLAYLOAD */
		writeReg(REG_FEATURE,0x06);			/* ʹ�ܶ�̬����PLAYLOAD������ACK PLAYLOAD */
		
		writeReg(REG_EN_AA,0x01); 			/* ʹ��ͨ��0���Զ�Ӧ�� */	
		
		writeReg(CMD_FLUSH_TX,0xff);		/* ��ϴTX_FIFO */
		writeReg(CMD_FLUSH_RX,0xff);
	}
	else							 	
	{
		writeReg(REG_CONFIG, 0x0E);			// TX Mode, Power ON, 2Bytes CRC, Enable CRC, IRQ �շ�����жϿ���
		writeReg(REG_DYNPD, 0x01);				// ʹ��RX_P0��̬����PLAYLOAD
		writeReg(REG_FEATURE, 0x06);		// ʹ�ܶ�̬���ȡ�ACK PLAYLOAD���͡�W_TX_PAYLOAD_NOACK
		
		writeReg(CMD_FLUSH_TX, 0xFF);		// ��ϴTX_FIFO
		writeReg(CMD_FLUSH_RX, 0xFF);
	}										
}

/* ���MCU��24l01�Ƿ�ͨѶ���� */
/* ������д�������ַ�Ƿ�һ�� */

ErrorStatus nrf_check(void)
{ 
	uint8_t addr[5] = {0x11, 0x22, 0x33, 0x44, 0x55}, read_addr[5], i;
	
	NRF_LowLevel_Init();
	writeBuf(CMD_W_REG | REG_TX_ADDR, addr, 5); 
	readBuf(CMD_R_REG | REG_TX_ADDR, read_addr, 5); 
	
	for( i = 0; i < 5; i++ )
	{
		if( addr[ i ] != read_addr[ i ] )
		{
			return ERROR;
		}	
	} 
	
	return SUCCESS;
}

/* �������ݰ������ذ�����len */
uint8_t nrf_rxPacket(uint8_t *rx_buf)
{	
	uint8_t rx_len = readReg(CMD_RX_PL_WID);
	if(rx_len>0 && rx_len<33)
	{
		NRF_CE_L();	
		readBuf(CMD_R_RX_PAYLOAD,rx_buf,rx_len);
		NRF_CE_H();
	}
	else 
		rx_len = 0;
	writeReg(CMD_FLUSH_RX,0xff);/* ��ϴRX_FIFO */
	return rx_len;		
}

/* ��ѯ�¼����������ݰ� */
nrfEvent_e nrf_checkEventandRxPacket(uint8_t *ackBuf, uint8_t *acklen)
{
	nrfEvent_e nrfEvent = IDLE;
	*acklen = 0;
	uint8_t status = readReg(REG_STATUS);/*���¼���־�Ĵ���*/
	
//	printf("Radio task nrfEvent_e: %d\n", status);
	
	if(status&BIT_MAX_RT)/*�ط�ʧ��*/
	{
		writeReg(CMD_FLUSH_TX,0xff);
		nrfEvent =  MAX_RT;
	}
	else if(status&BIT_RX_DR)/*�������ݵ�RX_FIFO*/
	{
		*acklen =  nrf_rxPacket(ackBuf);
		nrfEvent = RX_DR;
	}
	else if(status&BIT_TX_DS)/*����������TX_FIFO�ɹ�*/
	{
		nrfEvent = TX_DS;
	}
	writeReg(REG_STATUS,0x70);/*�����־*/
	uint8_t status1 = readReg(REG_STATUS);/*���¼���־�Ĵ���*/
	status1 = status1;
	return nrfEvent;
}

/* �������ݰ������ȴ�����ACK(PTXģʽ) */
/* ����ֵ��1�ɹ���0ʧ��*/
uint8_t nrf_sendPacketWaitACK(uint8_t *sendBuf, uint8_t len, uint8_t *ackBuf, uint8_t *acklen)
{ 
	if(len == 0) return 0;
	nrf_txPacket(sendBuf,len);
	while((readReg(REG_STATUS)&0x70) == 0);/* �ȴ��¼� */
	nrfEvent_e nrfEvent = nrf_checkEventandRxPacket(ackBuf, acklen);
	if(nrfEvent == MAX_RT)
		return 0;
	return 1;
}

/*����nrf�жϻص�����*/
void nrf_setIterruptCallback(void(*cb)(void))
{
	interruptCb = cb;
}

/*�ⲿ�жϷ�����*/
void EXTI15_10_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line13) == SET)
	{
		if(interruptCb)
		{
			interruptCb();
		}
		EXTI_ClearITPendingBit(EXTI_Line13);
	}
}

