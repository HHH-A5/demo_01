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

uint32_t BootStaFlag;   // 状态机变量
int main(void)
{
	Usart0_Init(921600);
	Delay_Init();
	IIC_Init();
	
	OTA_Info.OTA_flag = 0xAABB122;
	OTA_Info.Firelen[0] = 85;      // 第一个应用程序大小

	M24C02_WriteOTAInfo();
	Delay_Ms(5);
	M24C02_ReadOTAInfo();

	BootLoader_Brance();

	while(1)
	{
		if(BootStaFlag & UODATE_A_FLAG)
		{
			// 更新A区
			u0_printf(" code size : %d\r\n", OTA_Info.Firelen[UpDataA.W25Q64_BlockNB]);
			if(OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] % 4 == 0)
			{
				GD32_EraseFlash(GD32_A_SPAGE,GD32_A_PAGE_NUM);
				// 每次从w25q6读取1k的数据出来
				for(i = 0; i < OTA_Info.Firelen[UpDataA.W25Q64_BlockNB]/GD32_PAGE_SIZE; i++)
				{
					// UpDataA.W25Q64_BlockNB * 64 * 1024 代表偏移64k
					W25Q64_Read(UpDataA.Updatabuff, UpDataA.W25Q64_BlockNB * 64 * 1024 +i*1024, GD32_PAGE_SIZE);
					GD32_WriteFlash(0x08000000 + i*GD32_PAGE_SIZE, (uint32_t *)UpDataA.Updatabuff , GD32_PAGE_SIZE);
				}
				
				if(OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] % 1024 != 0)
				{
					W25Q64_Read(UpDataA.Updatabuff, UpDataA.W25Q64_BlockNB * 64 * 1024 +i*1024, OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] % 1024);
					GD32_WriteFlash(0x08000000 + i*GD32_PAGE_SIZE, (uint32_t *)UpDataA.Updatabuff , OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] % 1024);					
				}
				
				// 清除OTA标志位
				if(UpDataA.W25Q64_BlockNB == 0)
				{
					OTA_Info.OTA_flag = 0;
					M24C02_WriteOTAInfo();
				}
				NVIC_SystemReset();
			}
			else
			{
				u0_printf(" code size error\r\n");
				BootStaFlag &= ~UODATE_A_FLAG;
			}
		}
	}
}
