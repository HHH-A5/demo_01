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
	
	OTA_Info.OTA_flag = 0xAABB122;
	
	for(i = 0; i < 11; i++)
	{
		OTA_Info.Firelen[i] = i;
	}
	M24C02_WriteOTAInfo();
	Delay_Ms(5);
	M24C02_ReadOTAInfo();
	u0_printf("%x\r\n", OTA_Info.OTA_flag);
	for(i = 0; i < 11; i++)
	{
		u0_printf("%x\r\n", OTA_Info.Firelen[i]);
	}
	BootLoader_Brance();

	while(1)
	{

	}

}
