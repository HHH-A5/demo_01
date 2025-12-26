

## 38.实现进入串口交互式命令行

boot.c新增函数

```c
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
	u0_printf("[6]restart !\r\n");
}

```

同时在原来的BootLoader_Brance中调用

```
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
	else 
	{
		u0_printf("!!you are go inbootloder command window\r\n");
		BootLoader_Info();
	}
}
```

## 39.实现交互式命令[1]擦除A区[7]重启

新增函数

```c
void BootLoader_Even(uint8_t *data, uint16_t datalen)
{
	if((datalen == 1)&&(data[0] == '1'))
	{
		u0_printf("you choose [1]erase A block\r\n");
		GD32_EraseFlash(GD32_A_SPAGE,GD32_A_PAGE_NUM);
	}
	if((datalen == 1)&&(data[0] == '7'))
	{
		u0_printf("you choose [7]restart !\r\n");
		Delay_Ms(200);
		NVIC_SystemReset();
	}
}
```

![image-20251224222700845](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251224222700845.png)

![image-20251224222908845](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251224222908845.png)

在main.c的大循环里加入串口接受数据处理

```c
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
```

## 40.实现串口IAP功能的关键点总结

1：先发C，500ms/1s

2：SOH = 0x01

   ACK = 0x06

   NCK = 0x15

   EOT = 0x04

3：有效数据 128 把它写入A区

   整包数据 128 + 5 = 133

4：C8T6硬件CRC32位的，要自己做一个CRC16校验

![Xmodem基本流程](D:\桌面\GD32F103开发文件夹\GD32 + 4G开发板 资料包\(1)上篇章 手把手写Bootloader程序教程 配套资料+程序\(1)上篇章 手把手写Bootloader程序教程 配套资料+程序\【第40节视频】实现串口IAP功能的关键点总结\Xmodem基本流程.png)

## 41.实现CRC16校验功能

初始值，预算用的多项式

模2除法，对应C语言就是异或预算

按位运算，判断最高位是1还是0

1：异或多项式

0：不异或多项式

在boot.c添加一个校验函数

[CRC（循环冗余校验）在线计算_ip33.com](https://ip33.com/crc.html)

![image-20251225163306122](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251225163306122.png)

boot.c添加新函数

```c
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
			if(CRCinit & 0x8000)
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
```

main.c测试一下

```c
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

uint8_t crc[5] = {1,2,3,4,5};
int main(void)
{
	Usart0_Init(921600);
	Delay_Init();
	IIC_Init();

	u0_printf("CRC result:%x\r\n",Xmodem_CRC(crc,5));   // 如果是8028则计算正确
	
	while(1);
}
```

## 42.Xmdoem协议串口IAP第一步发送C

在main.h定义

#define IAP_XMODEMC_FLAG 0x00000002 

在boot.c里

```c
void BootLoader_Even(uint8_t *data, uint16_t datalen)
{
	if(BootStaFlag == 0)
	{
		if((datalen == 1)&&(data[0] == '1'))
		{
			u0_printf("you choose [1]erase A block\r\n");
			GD32_EraseFlash(GD32_A_SPAGE,GD32_A_PAGE_NUM);
		}
        // 添加：
		if((datalen == 1)&&(data[0] == '2'))
		{
			u0_printf("you choose dowlo A block with Xmodem Communication protocol , bin file !\r\n");
			GD32_EraseFlash(GD32_A_SPAGE,GD32_A_PAGE_NUM);
			BootStaFlag |= IAP_XMODEMC_FLAG;
			UpDataA.XmodemTimer = 0;
		}
		if((datalen == 1)&&(data[0] == '7'))
		{
			u0_printf("you choose [7]restart !\r\n");
			Delay_Ms(200);
			NVIC_SystemReset();
		}
	}
}

```

在main.c的while(1)里添加，这样每隔1秒发一个大写字母C

```c
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
```

## 43.完成整个Xmodem协议串口IAP功能工程源码

main.h新增一个个状态变量，C的上次添加了

```c
#define IAP_XMODEMC_FLAG 0x00000002     // C的意思就是发送大写C
#define IAP_XMODEMD_FLAG 0x00000004     // D的意思就是发送data数据
```

然后boot.c添加处理函数

```c
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
	
	// 新增
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
                    // 每8包写入一次，第一个8包不需要偏移，所以有个减1
						GD32_WriteFlash(GD32_A_SADDR + ((UpDataA.XmodemNB + 1)/(GD32_PAGE_SIZE/128) - 1)* GD32_PAGE_SIZE, (uint32_t *)UpDataA.Updatabuff , GD32_PAGE_SIZE);	
				}
				// 我是后加的，和up主的有区别
				UpDataA.XmodemNB++;    // ++后的值才代表接受到的包数
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
```

这是生成bin的指令，因为我们使用Xmodem传输的是二进制文件，而正常编译产生的是hex文件

G:\Applications\MDK_541\code\ARM\ARMCC\bin\fromelf.exe --bin -o  ..\USER\Objects\A_block.bin  ..\USER\Objects\A_block.axf

其中G:\Applications\MDK_541\code\ARM\ARMCC\bin是keil的安装包下的bin路径

\USER\Objects\A_block.axf是工程生成这个axf的路径

<img src="C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251225231646438.png" alt="image-20251225231646438" style="zoom:67%;" />

然后将这个命令输入到：

![image-20251225231836565](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251225231836565.png)

## 44.完成设置以及查询OTA版本号功能

格式：VER-1.1.0-2023/02/20-12:00

main.h,添加新变量OTA_Ver

```c
typedef struct
{
	uint32_t OTA_flag;
	uint32_t Firelen[11]; // 0成员固定对于OTA的大小
	uint8_t OTA_Ver[32];   
}OTA_InfoCB;

```



在even函数里面添加新的处理函数

```c
 else if(BootStaFlag & SET_VERSION_FLAG)
	{
		u0_printf("datalen is %d\r\n", datalen); 
		if(datalen == 26)
		{	
			if((sscanf((char *)data,"VER-%d.%d.%d-%d/%d/%d-%d:%d",&temp,&temp,&temp,&temp,&temp,&temp,&temp,&temp)) == 8)
			{
				memset(OTA_Info.OTA_Ver, 0 ,32);
				memcpy(OTA_Info.OTA_Ver, data, 26);
				M24C02_WriteOTAInfo();
				u0_printf("new version is ok\r\n"); 
				BootStaFlag &= ~SET_VERSION_FLAG;				
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
```

不要直接输入,先复制，再到2那里粘贴，不然一次只能输入一个字符；或者右键选择粘贴

<img src="C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251226112027813.png" alt="image-20251226112027813" style="zoom:67%;" />

查询功能,直接处理

```c
		if((datalen == 1)&&(data[0] == '4'))
		{
			u0_printf("you choose [4]get A block version !\r\n");
			M24C02_ReadOTAInfo();
			u0_printf("the newest: %s !\r\n",(char *)OTA_Info.OTA_Ver);   // 加入(char *)才可以正常输出
			BootLoader_Info();
		}
```

## 45.完成从外部Flash更新A区工程

直接在even函数原有的框架上添加的一下代码

```c
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
```



```c
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
```

然后对xmodem传输逻辑进行一下优化

```
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
```



![image-20251226182308618](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251226182308618.png)

然后记得在main.c添加W25Q64_Init();

因为我是后++的，所以代码里面是：((UpDataA.XmodemNB + 1)/8 -1)

<img src="C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251226163041184.png" alt="image-20251226163041184" style="zoom:67%;" />
