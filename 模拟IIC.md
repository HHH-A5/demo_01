## 利用systick做延时函数

在HW文件夹新建delay.c和delay.h,并将.c文件添加到HW分组中

```c
#include "gd32f10x.h"
#include "delay.h"

void Delay_Init(void){
	systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);    // 不分频,108Mhz
}

void Delay_Us(uint16_t us){
	SysTick->LOAD = us*108;      // 加载倒计数值
	SysTick->VAL = 0x00;         //  当前计数值
	
	// 或上就是置1,SysTick_CTRL_ENABLE_Msk就是那一位是1
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;   
	
	// SysTick->CTRL是一个寄存器,与上 SysTick_CTRL_COUNTFLAG_Msk 查看第16位的数值
	while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));  
	SysTick->CTRL &=~ SysTick_CTRL_ENABLE_Msk;
}

void Delay_Ms(uint16_t ms)
{
	while(ms--){
		Delay_Us(1000);
	}
}
```

## 起始和结束信号

```c
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

```

I²C 的“开始条件（Start）”定义为：
**SCL 保持高电平期间，SDA 出现高→低跳变**。
一旦这个跳变完成，总线就算被主机“占用”了。

但 Start 只负责“通知所有器件：新的传输马上开始”，它**并不自带时钟**。
接下来主机要往总线上送第一个 bit（地址的 MSB），必须先把时钟线 SCL 握在自己手里——也就是拉低——然后才能在 SDA 上改变数据，而不违反 I²C “SCL 高电平时 SDA 必须稳定”的规矩。

因此时序图里总是：

1. SDA 高→低（Start 形成）
2. 立刻把 SCL 也拉低，**锁定总线**，准备发送/接收数据。

你代码里最后一步

c

复制

```c
IIC_SCL_L;
```

正是为了“抢时钟”，告诉从机：
“起始标志已打完，现在由我主机掌控时钟，接下来我要开始发数据了。”

## 发送一字节数据

```c
void IIC_Send_Byte(uint8_t txd)
{
	int8_t i;
	
	for(i = 7; i >= 0; i--)
	{
		IIC_SCL_L;
		if(txd & BIT(i))  // BIT(x) 与 0x00001 << x位
		{
			IIC_SDA_H;
		}
		else
		{
			IIC_SDA_L;
		}
		Delay_Us(2);
		IIC_SCL_H;
		Delay_Us(2);
	}
	IIC_SCL_L;
	IIC_SDA_H;
	
}
```

## 等待从机应答信号

![image-20251218151049061](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251218151049061.png)

```c
uint8_t IIC_Wait_Ack(int16_t time_out)
{
	do
	{	
		time_out--;
		Delay_Us(2);
	}while((READ_SDA) & time_out);
	if(time_out < 0) return 1;     // 超时了
	
	IIC_SCL_H;
	Delay_Us(2);
	if(READ_SDA != 0) return 2;     // 没有保持稳定的低电平
	
	IIC_SCL_L;    // 防止误操作,拉低
	Delay_Us(2);
	return 0;
}
```
## 接受一字节数据

```c
uint8_t IIC_Read_Byte(uint8_t ack)
{
	int8_t i;
	uint8_t rxd;
	
	rxd = 0;
	for(i = 7; i >= 0; i--)
	{
		IIC_SCL_L;
		Delay_Us(2);
		IIC_SCL_H;
		
		if(READ_SDA)
		{
			rxd |= BIT(i);
		}
		Delay_Us(2);
	}
	
	IIC_SCL_L;
	Delay_Us(2);   // 此时从机释放总线
	if(ack)
	{
		IIC_SDA_L;
		IIC_SCL_H;
		Delay_Us(2);
	  IIC_SCL_L;
		IIC_SDA_H;
		Delay_Us(2);
	}
	else 
	{
		IIC_SDA_H;
		IIC_SCL_H;
		Delay_Us(2);
	  IIC_SCL_L;
		Delay_Us(2);
	}
	return rxd;
}

```

其中在iic.h中宏定义

```c
#define READ_SDA   gpio_input_bit_get(GPIOB,GPIO_PIN_7)
```

## 向m24c02写入数据

```c
#include "gd32f10x.h"
#include "iic.h"
#include "m24c02.h"

uint8_t M24C02_WriteByte(uint8_t addr, uint8_t wdata)
{
	// 开始,写等,写地址等,写数据等,结束
	IIC_Start();
	IIC_Send_Byte(M24C02_WRITE_ADDR);
	if(IIC_Wait_Ack(100) != 0)return 1;
	
	IIC_Send_Byte(addr);
	if(IIC_Wait_Ack(100) != 0)return 2;
	
	IIC_Send_Byte(wdata);
	if(IIC_Wait_Ack(100) != 0)return 3;
	
	IIC_Stop();
	
	return 0;
}

uint8_t M24C02_WritePage(uint8_t addr, uint8_t *wdata)
{
	// 开始,写等,写地址等,写数据等,结束
	
	uint8_t i;
	IIC_Start();
	IIC_Send_Byte(M24C02_WRITE_ADDR);
	if(IIC_Wait_Ack(100) != 0)return 1;
	
	IIC_Send_Byte(addr);
	if(IIC_Wait_Ack(100) != 0)return 2;
	
	for(i = 0; i < 16; i++)
	{
		IIC_Send_Byte(wdata[i]);
		if(IIC_Wait_Ack(100) != 0)return 3+i;
	}

	IIC_Stop();
	
	return 0;
}
```

## 从m24c02读取数据

```c
uint8_t M24C02_ReadData(uint8_t addr, uint8_t *rdata, uint8_t datalen)
{
	uint8_t i;
	
	IIC_Start();
	IIC_Send_Byte(M24C02_WRITE_ADDR);
	if(IIC_Wait_Ack(100) != 0)return 1;
	
	IIC_Send_Byte(addr);
	if(IIC_Wait_Ack(100) != 0)return 2;
	
	IIC_Start();
	IIC_Send_Byte(M24C02_READ_ADDR);
	if(IIC_Wait_Ack(100) != 0)return 3;
	
	for(i = 0; i < datalen - 1; i++)
	{
		rdata[i] = IIC_Read_Byte(1);  // 传入1 代表要应答
	}
	rdata[i] = IIC_Read_Byte(0);  // 传入0代表不要应答
	
	IIC_Stop();
	
	return 0;
}

```

主函数添加测试代码

```c
{
	Usart0_Init(921600);
	u0_printf("%d %c %x\r\n",0x30,0x30,0x30);
	Delay_Init();
	IIC_Init();
	
	for(i = 0; i < 256; i++)
	{
		M24C02_WriteByte(i,i);
		Delay_Ms(5);
	}
	
	M24C02_ReadData(0,rbuff,256);
	
	for(i = 0; i < 256; i++)
	{
		u0_printf("addr:%d  data:%d\r\n",i,rbuff[i]);
		Delay_Ms(100);
	}
	
```

