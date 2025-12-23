## SPI初始化和收发函数

新建spi.c和spi.h文件在HW分组,同时在LIB下添加spi库,因为用的是硬件的spi,同时添加宏

![image-20251218203158031](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251218203158031.png)

```c
#include "gd32f10x.h"
#include "spi.h"
#include "gd32f10x_spi.h"

void SPI0_Init(void)
{
	// 开时钟
	spi_parameter_struct spi_parameter;
	rcu_periph_clock_enable(RCU_SPI0);
	rcu_periph_clock_enable(RCU_GPIOA);
	
	gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_5 | GPIO_PIN_7);   // 输出
	gpio_init(GPIOA,GPIO_MODE_IN_FLOATING,GPIO_OSPEED_50MHZ,GPIO_PIN_6);	        // 输入
	
	spi_i2s_deinit(SPI0);
	spi_parameter.device_mode = SPI_MASTER;
	spi_parameter.trans_mode = SPI_TRANSMODE_FULLDUPLEX;
	spi_parameter.frame_size = SPI_FRAMESIZE_8BIT;
	spi_parameter.nss = SPI_NSS_SOFT;
	spi_parameter.endian = SPI_ENDIAN_MSB;
	spi_parameter.clock_polarity_phase = SPI_CK_PL_LOW_PH_1EDGE;
	spi_parameter.prescale = SPI_PSC_2;

 // spi_struct_para_init(spi_parameter_struct *spi_struct);

  spi_init(SPI0, &spi_parameter);

  spi_enable(SPI0);
}


// 收发一体!!!
uint8_t SPI0_ReadWriteByte(uint8_t txd)
{
	while(spi_i2s_flag_get(SPI0,SPI_FLAG_TBE) != 1);
	spi_i2s_data_transmit(SPI0, txd);
	while(spi_i2s_flag_get(SPI0,SPI_FLAG_RBNE) != 1);
	return spi_i2s_data_receive(SPI0);
}

void SPIO_Write(uint8_t *wdata, uint16_t datalen)
{
	uint16_t i;
	for(i = 0; i < datalen; i++)
	{
		SPI0_ReadWriteByte(wdata[i]);
	}
}


void SPIO_Read(uint8_t *rdata, uint16_t datalen)
{
	uint16_t i;
	for(i = 0; i < datalen; i++)
	{
		rdata[i] = SPI0_ReadWriteByte(0xff);
	}
}
```

```c
#ifndef SPI_H
#define SPI_H

#include "stdint.h"

void SPI0_Init(void);
uint8_t SPI0_ReadWriteByte(uint8_t txd);
void SPIO_Write(uint8_t *wdata, uint16_t datalen);
void SPIO_Read(uint8_t *rdata, uint16_t datalen);

#endif

```



## W25Q64擦,存,取

只有不处于busy的时候进行操作,通过指令的方式

![image-20251218211612845](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251218211612845.png)

![image-20251218211520725](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251218211520725.png)

```c
#include "gd32f10x.h"
#include "w25q64.h"
#include "spi.h"

void W25Q64_Init(void)
{
	// 开时钟
	rcu_periph_clock_enable(RCU_SPI0);
	rcu_periph_clock_enable(RCU_GPIOA);
	
	gpio_init(GPIOA,GPIO_MODE_AF_PP,GPIO_OSPEED_50MHZ,GPIO_PIN_4);   // 输出
	CS_DISENABLE;
	SPI0_Init();

}


void W25Q64_WaitBusy(void)
{
	uint8_t res;
	do{
		CS_ENABLE;
		SPI0_ReadWriteByte(0x05);         // 读取状态寄存器的对应指令
		res = SPI0_ReadWriteByte(0xff);   // 读取状态寄存器的对应指令
		CS_DISENABLE;
		
	}while((res & 0x01) == 0x01);
}

void W25Q64_Enable(void)
{
	W25Q64_WaitBusy();
	CS_ENABLE;
	SPI0_ReadWriteByte(0x06);         // 写入使能的对应指令

	CS_DISENABLE;

}


void W25Q64_Erase64K(uint8_t blockNB)
{
	uint8_t wdata[4];
	
	wdata[0] = 0xD8;
	
	// 计算地址
	wdata[1] = (blockNB * 64 * 1024) >> 16;
	wdata[2] = (blockNB * 64 * 1024) >> 8;
	wdata[3] = (blockNB * 64 * 1024) >> 0;
	
	W25Q64_WaitBusy();
	W25Q64_Enable();
	
	CS_ENABLE;
	SPIO_Write(wdata, 4);         // 写入使能的对应指令
	CS_DISENABLE;
  W25Q64_WaitBusy();
}


void W25Q64_PageWrite(uint8_t *wbuff, uint16_t pageNB)
{
	uint8_t wdata[4];
	
	wdata[0] = 0x02;
	
	// 计算地址
	wdata[1] = (pageNB * 256) >> 16;
	wdata[2] = (pageNB * 256) >> 8;
	wdata[3] = (pageNB * 256) >> 0;
	
	W25Q64_WaitBusy();
	W25Q64_Enable();
	
	CS_ENABLE;
	SPIO_Write(wdata, 4);   
	SPIO_Write(wbuff, 256);  	
	CS_DISENABLE;
 // W25Q64_WaitBusy();
}

void W25Q64_Read(uint8_t *rbuff, uint32_t addr, uint32_t datalen)
{
	uint8_t wdata[4];
	
	wdata[0] = 0x03;
	
	// 计算地址
	wdata[1] = (addr) >> 16;
	wdata[2] = (addr) >> 8;
	wdata[3] = (addr) >> 0;
	
	W25Q64_WaitBusy();
	W25Q64_Enable();
	
	CS_ENABLE;
	SPIO_Write(wdata, 4);   
	SPIO_Read(rbuff, datalen);  	
	CS_DISENABLE;
 // W25Q64_WaitBusy();
}

```

## 整体测试

```c
#include "gd32f10x.h"
#include "usart.h"
#include "delay.h"
#include "iic.h"
#include "m24c02.h"
#include "spi.h"
#include "w25q64.h"


uint16_t i;
uint16_t j;
uint8_t rbuff[256];
uint8_t wdata[256];
uint8_t rdata[256];

int main(void)
{
	Usart0_Init(921600);
	u0_printf("%d %c %x\r\n",0x30,0x30,0x30);
	Delay_Init();
	IIC_Init();
	SPI0_Init();
	W25Q64_Init();
	
	// W25Q64测试代码
	W25Q64_Erase64K(0);
	for(i = 0; i < 256; i++)
	{
		for(j = 0; j < 256; j++)
		{
			wdata[j] = i;
//	  W25Q64_PageWrite(wdata,i);     不应该放在这里!!!!!,会导致写入时间延长!!!!!
		}
		W25Q64_PageWrite(wdata,i);
		u0_printf("addr:%d  data:%d\r\n",i * 256 + j,wdata[j]);
	}
	
	Delay_Ms(100);
	
	for(i = 0; i < 256; i++)
	{
		W25Q64_Read(rdata, i *256, 256);
		for(j = 0; j < 256; j++)
		{
			u0_printf("addr:%d  data:%d\r\n",i * 256 + j,rdata[j]);
		}
	}
```

