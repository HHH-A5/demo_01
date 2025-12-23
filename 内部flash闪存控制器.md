## 内部flash闪存控制器

新建fmc.c和fmc.h文件,因为闪存控制器就是FMC,然后添加库文件,同时打开宏开关。一个扇区是1k,所以扇区1地址0-1023,扇区2地址1024-2047依次推理，通常页和扇区是同一个概念。

![image-20251219160234578](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251219160234578.png)

![image-20251219160105895](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251219160105895.png)

**同时注意起始地址不是0,而是0x0800 0000!!!**

![image-20251219161625018](C:\Users\Acer\AppData\Roaming\Typora\typora-user-images\image-20251219161625018.png)

## 编写底层代码

```c
#include "gd32f10x.h"
#include "fmc.h"


void GD32_EraseFlash(uint16_t start, uint16_t num)
{
	uint16_t i;
	fmc_unlock();
	
	for(i = 0; i < num; i++)
	{
		fmc_page_erase( 0x08000000 + start * 1024 + i * 1024);
	}
	
	fmc_lock();
}

// 按字写入,一次写入四字节,所以 uint32_t *wdata
void GD32_WriteFlash(uint32_t saddr, uint32_t *wdata, uint32_t wnum)
{
	fmc_unlock();
	while(wnum)
	{
		fmc_word_program(saddr, *wdata);
		wnum -= 4;
		saddr += 4;
		wdata++;   // 本身是32为的,所以不用加4
	}
}
```

编写测试代码

```c
#include "gd32f10x.h"
#include "usart.h"
#include "delay.h"
#include "iic.h"
#include "m24c02.h"
#include "spi.h"
#include "w25q64.h"
#include "fmc.h"

uint16_t i;
uint16_t j;
uint8_t rbuff[256];
uint8_t wdata[256];
uint8_t rdata[256];

uint32_t wbuff[1024];


int main(void)
{
	Usart0_Init(921600);
//	u0_printf("%d %c %x\r\n",0x30,0x30,0x30);
	Delay_Init();
	IIC_Init();
	SPI0_Init();
	W25Q64_Init();
	
	//  flash测试代码
	for(i = 0; i < 1024; i++)
	{
		wbuff[i] = 0x11118888;
	}
	
	GD32_EraseFlash(60,4);   // 擦除后面的4个扇区
	
	GD32_WriteFlash(0x08000000 + 60 * 1024, wbuff, 1024 * 4);
	
	for(i = 0; i < 1024; i++)
	{
		u0_printf("%x\r\n",* (uint32_t *)(0x08000000 + 60 * 1024 + i * 4));
	}
	
```

