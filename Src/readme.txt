�������䣺oskar@mindsilicon.com




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
	motorStatus.motor1_pwm = pwm1;
	motorStatus.motor2_pwm = pwm2;
	
	// data�ĵ�6�͵�7�ֽ�, �������ֽڹ�16λ, ��Ӧ�ֱ���16��ͨ��, 
	// �����λ��16λ����1λ, �����ǣ�L2, L1, LU, LL, LD, LR, SE,
	// ST, RL, RD, RR, RU, R1, R2, R-KEY, L-KEY
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
}





// ��ʾϵͳ���� 
void OLED_ShowParm(void)
{
	char volt1[20],volt2[20],motor1[20],motor2[20];
	
	sprintf(volt2,"Main  Voltage:%5.2f",ADC_ConvertedValueLocal[1]);
	OLED_ShowStr(0,0,(uint8_t*)(&volt2),1);
	
	sprintf(volt1, "Servo Voltage:%5.2f",ADC_ConvertedValueLocal[0]);
	OLED_ShowStr(0,1,(uint8_t*)(&volt1),1);
	
	sprintf(motor1,"motor1_pwm:%5d",motorStatus.motor1_pwm);
	OLED_ShowStr(0,4,(uint8_t*)(&motor1),1);
	
	sprintf(motor2,"motor2_pwm:%5d",motorStatus.motor2_pwm);
	OLED_ShowStr(0,5,(uint8_t*)(&motor2),1);
	
	vTaskDelay(50);	
}











