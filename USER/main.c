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
	
	
		// M24C02测试代码	
//	for(i = 0; i < 256; i++)
//	{
//		M24C02_WriteByte(i,i);
//		Delay_Ms(5);
//	}
//	
//	M24C02_ReadData(0,rbuff,256);
//	
//	for(i = 0; i < 256; i++)
//	{
//		u0_printf("addr:%d  data:%d\r\n",i,rbuff[i]);
//		Delay_Ms(100);
//	}
//	
	
	while(1)
	{
		// 串口测试代码
//		if(U0CB.URxDataOUT != U0CB.URxDataIN)
//		{
//			u0_printf("this time get %d data\r\n",U0CB.URxDataOUT->end - U0CB.URxDataOUT->start +1);
//			for(i = 0; i < U0CB.URxDataOUT->end - U0CB.URxDataOUT->start +1; i++)
//			{
//				u0_printf("%c",U0CB.URxDataOUT->start[i]);
//			}
//			u0_printf("\r\n\n\r");
//			
//			U0CB.URxDataOUT++;
//		  if(U0CB.URxDataOUT == U0CB.URxDataEND)
//		  {
//			  U0CB.URxDataOUT = &U0CB.URxDataPtr[0];
//	  	}
//			
//		}
		
		// 延时测试代码
//		  Delay_Ms(2000);
//			u0_printf("%d Delay succeed\r\n",0x30);
			

	}
	
}