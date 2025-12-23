#include "gd32f10x.h"
#include "usart.h"
#include "delay.h"
#include "iic.h"
#include "m24c02.h"
#include "spi.h"
#include "w25q64.h"
#include "fmc.h"
#include "main.h"
#include "boot.h"

uint8_t rbuff[256];
uint16_t i;
uint8_t ret;

OTA_InfoCB OTA_Info;
UpDataA_CB UpDataA;
int main(void)
{
	Usart0_Init(921600);
	Delay_Init();
	IIC_Init();
//	Delay_Ms(5);
//	M24C02_WriteByte(0x00,0xAA);
//	Delay_Ms(5);
//	M24C02_WriteByte(0x01,0xBB);
//	Delay_Ms(5);
//	M24C02_WriteByte(0x02,0x11);
//	Delay_Ms(5);
//	M24C02_WriteByte(0x03,0x22);
//	Delay_Ms(6);
	M24C02_ReadOTAInfo();
	ret += 1;
	if(ret <  2)
	{
	    BootLoader_Brance();
	}
	M24C02_ReadData(0,rbuff,6);
	
	for(i = 0; i < 6; i++)
	{
		u0_printf("addr:%d  data:%x\r\n",i,rbuff[i]);
		Delay_Ms(1);
	}

//	uint8_t tmp;
//M24C02_WriteByte(0x1C, 0x02);
//Delay_Ms(6);                    // ±ØÐë ¡Ý5 ms
//M24C02_ReadData(0x1C, &tmp, 1);
//u0_printf("read back: 0x%02X\r\n", tmp);
	while(1)
	{

	}

}
