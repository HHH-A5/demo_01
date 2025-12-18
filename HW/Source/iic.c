#include "gd32f10x.h"
#include "iic.h"
#include "delay.h"

void IIC_Init(void)
{
	rcu_periph_clock_enable(RCU_GPIOB);
	
	gpio_init(GPIOA,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_6);
	gpio_init(GPIOA,GPIO_MODE_OUT_OD,GPIO_OSPEED_50MHZ,GPIO_PIN_7);	
	
	
	// 初始总线都是高电平
	IIC_SCL_H;
	IIC_SDA_H;
}




void IIC_Start(void)
{
	IIC_SCL_H;
	IIC_SDA_H;
	Delay_Us(2);
	IIC_SDA_L;
	Delay_Us(2);
	IIC_SCL_L;     // 立刻把 SCL 也拉低，锁定总线，准备发送/接收数据。
}


void IIC_Stop(void)
{
	IIC_SCL_H;
	IIC_SDA_L;
	Delay_Us(2);
	IIC_SDA_H;
}
