## 29.回顾功能，规划参数

<img src="C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251221110533067.png" alt="image-20251221110533067" style="zoom:80%;" />

![image-20251221110451870](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251221110451870.png)

#### 新建main.h,定义一些基本参数

```c
#ifndef MAIN_H
#define MAIN_H

#include "stdint.h"

#define GD32_FLASH_SADDR 0x08000000  // flash起始地址
#define GD32_PAGE_SIZE   1024        // 一页1024b,就是1kb
#define GD32_PAGE_NUM    64          // 一共64k,1k1页，一共64页
#define GD32_B_PAGE_NUM  20          // B区的页数大小
#define GD32_A_PAGE_NUM  (GD32_PAGE_NUM - GD32_B_PAGE_NUM)     // A区的页数大小
#define GD32_A_SPAGE     (GD32_B_PAGE_NUM)                     // A区的起始页，就等于B区的页数大小
#define GD32_A_SADDR     (GD32_FLASH_SADDR + GD32_A_SPAGE * GD32_PAGE_SIZE)  // A区的flash起始地址

#endif

```

## 30.OTAflag的定义、读取和 判定

#### 在main.h定义OTA结构体

```
typedef struct
{
	uint32_t OTA_flag;
	
	
}OTA_InfoCB;
```

在在main.c定义这个结构体变量

```
OTA_InfoCB OTA_Info;
```

然后在main.h导出一下

```
extern OTA_InfoCB OTA_Info;
```



#### 在m24c02.c添加新函数

同时记得在m24c02.h中进行声明

```
void M24C02_ReadOTAInfo(void)
{
	memset(&OTA_Info, 0, OTA_INFOCB_SIZE);
	M24C02_ReadData(0, (uint8_t *)&OTA_Info, OTA_INFOCB_SIZE);   // 规定地址0存入的OTA是信息
}
```



#### 新建boot.c和boot.h在HW文件夹里面

```
void BootLoader_Brance(void)
{
	if(OTA_Info.OTA_flag == OTA_SET_FLAG)
	{
		u0_printf("OTA update\r\n");
	}
	else
	{
		u0_printf("jump to A block\r\n");
	}
}
```

同时在main.c测试一下

```
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

OTA_InfoCB OTA_Info;
int main(void)
{
	Usart0_Init(921600);
	Delay_Init();
	IIC_Init();
	M24C02_ReadOTAInfo();
	BootLoader_Brance();
	M24C02_ReadData(0,rbuff,256);
	for(i = 0; i < 6; i++)
	{
		u0_printf("addr:%d  data:%x\r\n",i,rbuff[i]);
		Delay_Ms(1);
	}
	while(1)
	{

	}
	
}
```

## 31.跳转的两大任务

![image-20251223112200084](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251223112200084.png)

#### 设置sp

0x0800 0000

A区 从20页开始，20x1024 = 0x5000

所以0x0800 5000 赋值给sp

#### 设置pc

 A 区起始地址加4 

0x0800 5000  + 4

最后把B区用到的所有外设和寄存器复位



## 32.实现无OTA时跳转A区

#### 回到boot.c,添加一下 代码

但汇编部分有问题，找了一个解决方案

```c


load_a load_A;

void LOAD_A(uint32_t addr);   // 哇趣,这不提前声明会报错

//  阿萨姆 调汇编，  但是会报错！！
//asm void MSP_SP(uint32_t addr)
//{
//	MSR MSP, r0;     // 传入的第一个参数存在r0寄存器
//	BX r14;
//}

void BootLoader_Clear(void)
{
	usart_deinit(USART0);	
	gpio_deinit(GPIOA);
	gpio_deinit(GPIOB);
}

// 20k的RAM 0x2000 0000 - 0x2000 4fff
void MSR_MSP2(uint32_t addr);  // 在启动文件定义了，在这里声明一下
void  LOAD_A(uint32_t addr)
{
	if( (*(uint32_t *)addr >= 0x20000000) && (*(uint32_t *)addr <= 0x20004fff))
	{
		MSR_MSP2(*(uint32_t *)addr);
		// (uint32_t *)(addr + 4) 变为指针
	  // *(uint32_t *)(addr + 4); 取值
	  // (load_a) *(uint32_t *)(addr + 4)强制转换
		load_A =(load_a) *(uint32_t *)(addr + 4);
		BootLoader_Clear();
		load_A();
	}

```

[keil 中 __asm void MSR_MSP(u32 addr) 编译器报错的问题_msr 编译失败-CSDN博客](https://blog.csdn.net/weixin_44619232/article/details/114922357?spm=1001.2101.3001.6650.2&utm_medium=distribute.pc_relevant.none-task-blog-2~default~BlogCommendFromBaidu~Ctr-2-114922357-blog-118649854.235^v43^pc_blog_bottom_relevance_base6&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2~default~BlogCommendFromBaidu~Ctr-2-114922357-blog-118649854.235^v43^pc_blog_bottom_relevance_base6&utm_relevant_index=5)

![image-20251223163451201](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251223163451201.png)

最后回到跳转函数

```c
void BootLoader_Brance(void)
{
	if(OTA_Info.OTA_flag == OTA_SET_FLAG)
	{
		u0_printf("OTA update\r\n");
	}
	else
	{
		u0_printf("jump to A block\r\n");
		LOAD_A(GD32_A_SADDR);    // 添加！！
	}
}
```

#### A区程序修改位置,一共两处

<img src="C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251223165343411.png" alt="image-20251223165343411" style="zoom: 67%;" />

![image-20251223165246180](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251223165246180.png)

#### 同时，发现了以前串口打印函数的问题，把TBE 改为TC

<img src="C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251223165820992.png" alt="image-20251223165820992" style="zoom:67%;" />

## 33.一些OTA细节

#### 1：谁将OTA_flag变成对勾？

A区

#### 2：什么时候将OTA_flag变成对勾？

A区下载完毕之后

#### 3：OTA时，最新版本的程序文件下载到哪？

分片下载，[256],W25Q64中

#### 4：OTA时，最新版本的程序文件如何下载？下载多少？

服务器下发告诉我们上传的新版本程序的大小，字节数

4+：下载多少这个变量用不用保存？

用，保存到24C02

#### 5：发生OTA事件时，B区如何更新A区？

根据保存在24C02中的下载量，拿数据，写到A区

## 34.向工程添加变量

先到main.h中补充

#### uint32_t Firelen[11]

#### UpDataA_CB

```c
#ifndef MAIN_H
#define MAIN_H

#include "stdint.h"

#define GD32_FLASH_SADDR 0x08000000  // flash起始地址
#define GD32_PAGE_SIZE   1024        // 一页1024b,就是1kb
#define GD32_PAGE_NUM    64          // 一共64k,1k1页，一共64页
#define GD32_B_PAGE_NUM  20          // B区的页数大小
#define GD32_A_PAGE_NUM  (GD32_PAGE_NUM - GD32_B_PAGE_NUM)    // A区的页数大小
#define GD32_A_SPAGE     (GD32_B_PAGE_NUM)                     // A区的起始页，就等于B区的页数大小
#define GD32_A_SADDR     (GD32_FLASH_SADDR + GD32_A_SPAGE * GD32_PAGE_SIZE)  // A区的flash起始地址

#define OTA_SET_FLAG  0x2211BBAA

typedef struct
{
	uint32_t OTA_flag;
添加：	uint32_t Firelen[11]; // 0成员固定对于OTA的大小
}OTA_InfoCB;
#define OTA_INFOCB_SIZE sizeof(OTA_InfoCB)
	
添加：
// 每次1024个字节
typedef struct
{
	uint8_t Updatabuff[GD32_PAGE_SIZE];
	uint32_t W25Q64_BlockNB;  // 记录取的是哪一块
	
}UpDataA_CB;

extern OTA_InfoCB OTA_Info;
添加：extern UpDataA_CB UpDataA;   // 记得在main.c定义
#endif

```

## 35.向24C02写入OTA信息结构体数

#### 新加一个写入函数

```c

void M24C02_WriteOTAInfo(void)
{
	uint8_t i;
	uint8_t *wptr;
	
	wptr = (uint8_t *)&OTA_Info;
	for(i = 0; i < OTA_INFOCB_SIZE/16; i++)
	{
		M24C02_WritePage(i * 16, wptr + i * 16);
		Delay_Ms(6);
	}
}
```

#### 在main.c加入测试函数

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
	
	OTA_Info.OTA_flag = 0x11223344;
	
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

```

## 36.B区从W25Q64中更新A区工程

#### 先定义状态机变量

在main.c,  uint32_t BootStaFlag;   // 状态机变量

```
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

```

![image-20251224202909131](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251224202909131.png)



![image-20251224202828368](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251224202828368.png)

![image-20251224203027652](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251224203027652.png)
