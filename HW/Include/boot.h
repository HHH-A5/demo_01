#ifndef BOOT_H
#define BOOT_H

#include "stdint.h"

typedef void (*load_a)(void);

void BootLoader_Brance(void);
void BootLoader_Clear(void);
void LOAD_A(uint32_t addr);
__asm void MSR_SP(uint32_t addr);
uint8_t BootLoader_Enter(uint8_t time_out);
void BootLoader_Info(void);
#endif

