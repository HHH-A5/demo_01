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
#include "w25q64.h"

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
	u0_printf("!!you are go in bootloder command window\r\n");
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
	int temp,i;
	if(BootStaFlag == 0)    //====================================================================================================
	{
		if((datalen == 1)&&(data[0] == '1'))
		{
			u0_printf("you choose [1]erase A block\r\n");
			GD32_EraseFlash(GD32_A_SPAGE,GD32_A_PAGE_NUM);
		}
		
		if((datalen == 1)&&(data[0] == '2'))
		{
			u0_printf("you choose [2]dowlo A block with Xmodem Communication protocol , bin file !\r\n");
			GD32_EraseFlash(GD32_A_SPAGE,GD32_A_PAGE_NUM);
			BootStaFlag |= IAP_XMODEMC_FLAG;
			BootStaFlag |= IAP_XMODEMD_FLAG;
			UpDataA.XmodemTimer = 0;
			UpDataA.XmodemNB = 0;
		}
		
		if((datalen == 1)&&(data[0] == '3'))
		{
			u0_printf("you choose [3]set A block version !\r\n");
			BootStaFlag |= SET_VERSION_FLAG;
		}
		
		if((datalen == 1)&&(data[0] == '4'))
		{
			u0_printf("you choose [4]get A block version !\r\n");
			M24C02_ReadOTAInfo();
			u0_printf("the newest: %s !\r\n",(char *)OTA_Info.OTA_Ver);   // 加入(char *)才可以正常输出
			BootLoader_Info();
		}
		
		if((datalen == 1)&&(data[0] == '5'))
		{
			BootStaFlag |= CMD_5_FLAG;
			u0_printf("you choose [5]dowlo program to outside flash !\r\n");
			u0_printf("please enter the nume (1-9) of flash !\r\n");
		}

		if((datalen == 1)&&(data[0] == '6'))
		{
			BootStaFlag |= CMD_6_FLAG;
			u0_printf("you choose [6]use program in outside flash !\r\n");
			u0_printf("please enter the nume (1-9) of flash !\r\n");
		}
		
		if((datalen == 1)&&(data[0] == '7'))
		{
			u0_printf("you choose [7]restart !\r\n");
			Delay_Ms(200);
			NVIC_SystemReset();
		}
	}
	else if(BootStaFlag & IAP_XMODEMD_FLAG)     //====================================================================================================
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
					if(BootStaFlag & CMD_5_XMODEM_FLAG)
					{
						for(i = 0; i < 4; i++)
						{
							// 一页写 256字节，64k * 4
							W25Q64_PageWrite(&UpDataA.Updatabuff[i * 256], UpDataA.W25Q64_BlockNB * 64 * 4 + ((UpDataA.XmodemNB + 1)/8 -1) * 4 + i);
						}
					}
					else
					{
						GD32_WriteFlash(GD32_A_SADDR + ((UpDataA.XmodemNB + 1)/(GD32_PAGE_SIZE/128) - 1)* GD32_PAGE_SIZE, (uint32_t *)UpDataA.Updatabuff , GD32_PAGE_SIZE);	
					}
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
				if(BootStaFlag & CMD_5_XMODEM_FLAG)
				{
					for(i = 0; i < 4; i++)
					{
						W25Q64_PageWrite(&UpDataA.Updatabuff[i * 256], UpDataA.W25Q64_BlockNB * 64 * 4 + (UpDataA.XmodemNB/8) * 4 + i);
					}
				}
				else
				{				
					GD32_WriteFlash(GD32_A_SADDR + (UpDataA.XmodemNB /(GD32_PAGE_SIZE/128))* GD32_PAGE_SIZE, (uint32_t *)UpDataA.Updatabuff , ((UpDataA.XmodemNB) % (GD32_PAGE_SIZE/128)) * 128);	
				}
			}
			if(BootStaFlag & CMD_5_XMODEM_FLAG)
			{
				BootStaFlag &= ~IAP_XMODEMD_FLAG;
				BootStaFlag &= ~CMD_5_XMODEM_FLAG;  // 这里不用重启
				OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] = UpDataA.XmodemNB * 128;
				M24C02_WriteOTAInfo();
				Delay_Ms(100);
				BootLoader_Info();
			}
			else
			{
				Delay_Ms(100);
				NVIC_SystemReset();			
			}
		}
	}
  else if(BootStaFlag & SET_VERSION_FLAG)      //====================================================================================================
	{
		u0_printf("datalen is %d\r\n", datalen); 
		if(datalen == 26)
		{	
			if((sscanf((char *)data,"VER-%d.%d.%d-%d/%d/%d-%d:%d",&temp,&temp,&temp,&temp,&temp,&temp,&temp,&temp)) == 8)
			{
				memset(OTA_Info.OTA_Ver, 0 ,32);
				memcpy(OTA_Info.OTA_Ver, data, 27);
				M24C02_WriteOTAInfo();
				u0_printf("new version is ok\r\n"); 
				BootStaFlag &= ~SET_VERSION_FLAG;	
				BootLoader_Info();				
			}
			else
			{
				u0_printf("the type of version is wrong ! ! !\r\n"); 
			}
		}
		else
		{
			u0_printf("the len of version is wrong ! ! !\r\n");  
		}
	}
	else if(BootStaFlag & CMD_5_FLAG)        //====================================================================================================
	{
		if(datalen == 1)
		{
			if((data[0] >= '1') && (data[0] <= '9'))
			{
				UpDataA.W25Q64_BlockNB = data[0] - 0x30;
				BootStaFlag |= IAP_XMODEMC_FLAG;
				BootStaFlag |= IAP_XMODEMD_FLAG;
				BootStaFlag |= CMD_5_XMODEM_FLAG;
				UpDataA.XmodemTimer = 0;
				UpDataA.XmodemNB = 0;
				OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] = 0;
				W25Q64_Erase64K(UpDataA.W25Q64_BlockNB );
				BootStaFlag &=~ CMD_5_FLAG;			
				u0_printf("you choose [5]use program in outside flash ,%d block\r\n",UpDataA.W25Q64_BlockNB);
				u0_printf("with Xmodem Communication protocol , bin file !\r\n");
			}
			else
			{
				u0_printf("the number is wrong ! ! !\r\n");  
			}			
		}
		else
		{
			u0_printf("the len of data is wrong ! ! !\r\n");  
		}
	}
	else if(BootStaFlag & CMD_6_FLAG)        //====================================================================================================
	{
		if(datalen == 1)
		{
			if((data[0] >= '1') && (data[0] <= '9'))
			{
				UpDataA.W25Q64_BlockNB = data[0] - 0x30;
				u0_printf("you choose [6]dowlo program to outside flash ,%d block\r\n",UpDataA.W25Q64_BlockNB);
				u0_printf("	OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] = %d block\r\n",	OTA_Info.Firelen[UpDataA.W25Q64_BlockNB] );
				BootStaFlag |= UODATE_A_FLAG;
				BootStaFlag &=~ CMD_6_FLAG;			
			}
			else
			{
				u0_printf("the number is wrong ! ! !\r\n");  
			}			
		}
		else
		{
			u0_printf("the len of data is wrong ! ! !\r\n");  
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
