#ifndef IIC_H
#define IIC_H

#include "stdint.h"


#define IIC_SCL_H  gpio_bit_set(GPIOB,GPIO_PIN_6)
#define IIC_SCL_L  gpio_bit_reset(GPIOB,GPIO_PIN_6)

#define IIC_SDA_H  gpio_bit_set(GPIOB,GPIO_PIN_7)
#define IIC_SDA_L  gpio_bit_reset(GPIOB,GPIO_PIN_7)


void IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
	

#endif
