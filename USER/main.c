#include "gd32f10x.h"
#include "usart.h"
#include "delay.h"
#include "iic.h"

uint16_t i;


int main(void)
{
	Usart0_Init(921600);
	u0_printf("%d %c %x\r\n",0x30,0x30,0x30);
	
	Delay_Init();
	
	while(1)
	{
		// ´®¿Ú²âÊÔ´úÂë
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
		  Delay_Ms(2000);
			u0_printf("%d Delay succeed\r\n",0x30);
		
	}
	
}