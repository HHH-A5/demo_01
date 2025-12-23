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

void BootLoader_Clear(void){
	usart_deinit(USART0);
	gpio_deinit(GPIOA);
	gpio_deinit(GPIOB);
}
__asm void MSR_SP(uint32_t addr){
	MSR MSP, r0
	BX r14
}

void LOAD_A(uint32_t addr){
	if((*(uint32_t *)addr>=0x20000000)&&(*(uint32_t *)addr<=0x20004FFF)){
		MSR_SP(*(uint32_t *)addr);
		load_A = (load_a)*(uint32_t *)(addr+4);
		BootLoader_Clear();
		load_A();
	}
}


void BootLoader_Brance(void){
	if(OTA_Info.OTA_flag == OTA_SET_FLAG){
		u0_printf("OTA更新\r\n");
	}else{
		u0_printf("go to A block\r\n");
		LOAD_A(GD32_A_SADDR);
	}
}



