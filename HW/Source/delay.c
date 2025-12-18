//#include "gd32f10x.h"
//#include "delay.h"

//void Delay_Init(void)
//{
//  systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);

//}

//void Delay_Us(uint16_t us)
//{
//    SysTick->LOAD = us * (SystemCoreClock/1000000U);   // 108 MHz 下 1 μs 对应 108 个时钟
//    SysTick->VAL  = 0;              // 清空当前值
//    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk;          // 启用，不开启中断
//    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) { /* 等待 */ }
//   SysTick->CTRL &=~ SysTick_CTRL_ENABLE_Msk;  // 关闭 SysTick
//}
//void Delay_Ms(uint16_t ms)
//{
//	while(ms--)
//	{
//		Delay_Us(1000);
//	}
//}

#include "gd32f10x.h"
#include "delay.h"

void Delay_Init(void){
	systick_clksource_set(SYSTICK_CLKSOURCE_HCLK);    // 不分频,108Mhz
}

void Delay_Us(uint16_t us){
	SysTick->LOAD = us*108;      // 加载倒计数值
	SysTick->VAL = 0x00;         //  当前计数值
	
	// 或上就是置1,SysTick_CTRL_ENABLE_Msk就是那一位是1
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;   
	
	// SysTick->CTRL是一个寄存器,与上 SysTick_CTRL_COUNTFLAG_Msk 查看第16位的数值
	while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));  
	SysTick->CTRL &=~ SysTick_CTRL_ENABLE_Msk;
}

void Delay_Ms(uint16_t ms)
{
	while(ms--){
		Delay_Us(1000);
	}
}