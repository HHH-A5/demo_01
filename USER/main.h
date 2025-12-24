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

#define OTA_SET_FLAG  0xAABB122
#define UODATE_A_FLAG  0x00000001

typedef struct
{
	uint32_t OTA_flag;
	uint32_t Firelen[11]; // 0成员固定对于OTA的大小
}OTA_InfoCB;
#define OTA_INFOCB_SIZE sizeof(OTA_InfoCB)
	
// 每次1024个字节
typedef struct
{
	uint8_t Updatabuff[GD32_PAGE_SIZE];
	uint32_t W25Q64_BlockNB;  // 记录取的是哪一块
	
}UpDataA_CB;


extern OTA_InfoCB OTA_Info;
extern UpDataA_CB UpDataA;
extern uint32_t BootStaFlag; 
#endif
