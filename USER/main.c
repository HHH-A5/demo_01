#include "gd32f10x.h"
#include "usart.h"
#include "delay.h"
#include "iic.h"
#include "m24c02.h"

uint16_t i;
uint8_t rbuff[256];

int main(void)
{
	Usart0_Init(921600);
	u0_printf("%d %c %x\r\n",0x30,0x30,0x30);
	Delay_Init();
	IIC_Init();
	
	for(i = 0; i < 256; i++)
	{
		M24C02_WriteByte(i,i);
		Delay_Ms(5);
	}
	
	M24C02_ReadData(0,rbuff,256);
	
	for(i = 0; i < 256; i++)
	{
		u0_printf("addr:%d  data:%d\r\n",i,rbuff[i]);
		Delay_Ms(100);
	}
	
	
	while(1)
	{
		// ¥Æø⁄≤‚ ‘¥˙¬Î
//		if(U0CB.URxDataOUT != U0CB.URxDataIN)
//		{
//			u0_printf("this time get %d data\r\n",U0CB.URxDataOUT->end - U0CB.URxDataOUT->start +1);
//			for(i = 0; i < U0CB.URxDataOUT->end - U0CB.URxDataOUT->start +1; i++)
//			{
//				u0_printf("%c",U0CB.URxDataOUT->start[i]);
//			}
//			u0_printf("\r\n\n\r");
//			
//			U0CB.URxDataOUT++;
//		  if(U0CB.URxDataOUT == U0CB.URxDataEND)
//		  {
//			  U0CB.URxDataOUT = &U0CB.URxDataPtr[0];
//	  	}
//			
//		}
		
		// —” ±≤‚ ‘¥˙¬Î
//		  Delay_Ms(2000);
//			u0_printf("%d Delay succeed\r\n",0x30);
			

	}
	
}