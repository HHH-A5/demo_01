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
