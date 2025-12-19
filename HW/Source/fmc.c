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



