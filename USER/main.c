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

uint8_t crc[5] = {1,2,3,4,5};
int main(void)
{
	Usart0_Init(921600);
	Delay_Init();
	IIC_Init();
	
	OTA_Info.OTA_flag = 0xAABB1111;
	OTA_Info.Firelen[0] = 85;      // 第一个应用程序大小

	u0_printf("CRC result:%x\r\n",Xmodem_CRC(crc,5));
	
	//while(1);
	
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
		
		// 两者不等代表接收到数据
		if(U0CB.URxDataOUT != U0CB.URxDataIN)
		{
			BootLoader_Even(U0CB.URxDataOUT->start, U0CB.URxDataOUT->end - U0CB.URxDataOUT->start +1);
		
			U0CB.URxDataOUT++;
		  if(U0CB.URxDataOUT == U0CB.URxDataEND)
		  {
			  U0CB.URxDataOUT = &U0CB.URxDataPtr[0];
	  	}
		}
		
		if(BootStaFlag & IAP_XMODEMC_FLAG)
		{
			Delay_Ms(10);
			if(UpDataA.XmodemTimer >= 100)
			{
				u0_printf("C");
				UpDataA.XmodemTimer = 0;
			}
			UpDataA.XmodemTimer++;
		}
	}
}
