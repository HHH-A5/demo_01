#ifndef BOOT_H
#define BOOT_H

#include "stdint.h"

typedef void (*load_a)(void);

void BootLoader_Brance(void);
	
#endif

