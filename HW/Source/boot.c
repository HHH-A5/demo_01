//#include "gd32f10x.h"
//#include "usart.h"
//#include "delay.h"
//#include "iic.h"
//#include "m24c02.h"
//#include "spi.h"
//#include "w25q64.h"
//#include "fmc.h"
//#include "main.h"
//#include "boot.h"

//load_a load_A;

//void LOAD_A(uint32_t addr);   // 哇趣,这不提前声明会报错
//void BootLoader_Brance(void)
//{
//	
//	Delay_Ms(500);
//	if(OTA_Info.OTA_flag == OTA_SET_FLAG)
//	{
//		u0_printf("OTA update\r\n");
//	}
//	else
//	{
//		u0_printf("jump to A block\r\n");
//		LOAD_A(GD32_A_SADDR);
//	}
//}

//// 阿萨姆 调汇编，  但是会报错！！
////asm void MSP_SP(uint32_t addr)
////{
////	MSR MSP, r0;     // 传入的第一个参数存在r0寄存器
////	BX r14;
////}

//void BootLoader_Clear(void)
//{
//	usart_deinit(USART0);	
//	gpio_deinit(GPIOA);
//	gpio_deinit(GPIOB);
//}

//// 20k的RAM 0x2000 0000 - 0x2000 4fff
//void MSR_MSP2(uint32_t addr);  // 在启动文件定义了，在这里声明一下
//void  LOAD_A(uint32_t addr)
//{
//	if( (*(uint32_t *)addr >= 0x20000000) && (*(uint32_t *)addr <= 0x20004fff))
//	{
//		MSR_MSP2(*(uint32_t *)addr);
//		// (uint32_t *)(addr + 4) 变为指针
//	  // *(uint32_t *)(addr + 4); 取值
//	  // (load_a) *(uint32_t *)(addr + 4)强制转换
//		load_A =(load_a) *(uint32_t *)(addr + 4);
//		BootLoader_Clear();
//		load_A();
//	}
//}

#include "gd32f10x.h"
#include "boot.h"
#include "main.h"
#include "usart.h"
#include "delay.h"
#include "fmc.h"
#include "iic.h"
#include "m24c02.h"

load_a load_A;

void BootLoader_Clear(void)
{
	usart_deinit(USART0);
	gpio_deinit(GPIOA);
	gpio_deinit(GPIOB);
}

__asm void MSR_SP(uint32_t addr)
{
	MSR MSP, r0
	BX r14
}

void LOAD_A(uint32_t addr)
{
	if((*(uint32_t *)addr>=0x20000000)&&(*(uint32_t *)addr<=0x20004FFF))
	{
		MSR_SP(*(uint32_t *)addr);
		load_A = (load_a)*(uint32_t *)(addr+4);
		BootLoader_Clear();
		load_A();
	}
	else
	{
		u0_printf("go to A block error !\r\n");
	}
}

void BootLoader_Brance(void)
{
	if(BootLoader_Enter(20) == 0)
	{
		if(OTA_Info.OTA_flag == OTA_SET_FLAG)
		{
			u0_printf("OTA update\r\n");
			BootStaFlag |= UODATE_A_FLAG;
			UpDataA.W25Q64_BlockNB = 0;
		}else
		{
			u0_printf("go to A block\r\n");
			LOAD_A(GD32_A_SADDR);
		}
	}
	// 如果跳转失败，也进入命令行
	u0_printf("!!you are go inbootloder command window\r\n");
	BootLoader_Info();

}

uint8_t BootLoader_Enter(uint8_t time_out)
{
	u0_printf("%dms, please enter :w, for bootloder command window\r\n", time_out * 100);
	while(time_out--)
	{
		Delay_Ms(100);
		if(U0_RxBuff[0] == 'w')
		{
			return 1;  // 进入命令行
		}
	}
	return 0;  // 不进入命令行
}

void BootLoader_Info(void)
{
	u0_printf("\r\n");
	u0_printf("[1]erase A block\r\n");
	u0_printf("[2]dowlo A block\r\n");
	u0_printf("[3]set A block version\r\n");
	u0_printf("[4]get A block version\r\n");
	u0_printf("[5]dowlo program to outside flash\r\n");
	u0_printf("[6]use program in outside flash\r\n");
	u0_printf("[7]restart !\r\n");
}

void BootLoader_Even(uint8_t *data, uint16_t datalen)
{
	if(BootStaFlag == 0)
	{
		if((datalen == 1)&&(data[0] == '1'))
		{
			u0_printf("you choose [1]erase A block\r\n");
			GD32_EraseFlash(GD32_A_SPAGE,GD32_A_PAGE_NUM);
		}
		if((datalen == 1)&&(data[0] == '2'))
		{
			u0_printf("you choose dowlo A block with Xmodem Communication protocol , bin file !\r\n");
			GD32_EraseFlash(GD32_A_SPAGE,GD32_A_PAGE_NUM);
			BootStaFlag |= IAP_XMODEMC_FLAG;
			BootStaFlag |= IAP_XMODEMD_FLAG;
			UpDataA.XmodemTimer = 0;
			UpDataA.XmodemNB = 0;
		}
		if((datalen == 1)&&(data[0] == '7'))
		{
			u0_printf("you choose [7]restart !\r\n");
			Delay_Ms(200);
			NVIC_SystemReset();
		}
	}
	
	if(BootStaFlag & IAP_XMODEMD_FLAG)
	{
		if((datalen == 133)&&(data[0] == 0x01))
		{
			BootStaFlag &= ~IAP_XMODEMC_FLAG;
			UpDataA.XmodemCRC =  Xmodem_CRC(&data[3], 128);
			if(UpDataA.XmodemCRC == data[131]*256 + data[132])
			{
				memcpy(&UpDataA.Updatabuff[(UpDataA.XmodemNB % (GD32_PAGE_SIZE/128))*128], &data[3], 128);
				if(((UpDataA.XmodemNB + 1) % (GD32_PAGE_SIZE/128)) == 0)
				{
						GD32_WriteFlash(GD32_A_SADDR + ((UpDataA.XmodemNB + 1)/(GD32_PAGE_SIZE/128) - 1)* GD32_PAGE_SIZE, (uint32_t *)UpDataA.Updatabuff , GD32_PAGE_SIZE);	
				}
				// 我是后加的，和up主的有区别
				UpDataA.XmodemNB++;
				u0_printf("\x06");   // ACK		
			}
			else
			{	
				u0_printf("\x15");	 // NACK	 		
			}
		}	

		if((datalen == 1)&&(data[0] == 0x04))
		{	
			u0_printf("\x06");   // ACK					
			if(((UpDataA.XmodemNB) % (GD32_PAGE_SIZE/128)) != 0)
			{
					GD32_WriteFlash(GD32_A_SADDR + (UpDataA.XmodemNB /(GD32_PAGE_SIZE/128))* GD32_PAGE_SIZE, (uint32_t *)UpDataA.Updatabuff , ((UpDataA.XmodemNB) % (GD32_PAGE_SIZE/128)) * 128);	
			}
			Delay_Ms(100);
			NVIC_SystemReset();
		}
	}
}

uint16_t Xmodem_CRC(uint8_t *data, uint16_t datalen)
{
	uint8_t i;
	// 这两个值是规定的
	uint16_t CRCinit = 0x0000;
	uint16_t CRCpoly = 0x1021;
	
	while(datalen--)
	{
		CRCinit = (*data << 8) ^ CRCinit;   // *data代表取值
		for(i = 0; i < 8; i++)
		{
			if(CRCinit & 0x8000)   // 判断最高位是不是1
			{
				CRCinit = (CRCinit << 1) ^ CRCpoly;
			}
			else
			{
				CRCinit = (CRCinit << 1);
			}
		}
		data++;
	}
	return CRCinit;
}
