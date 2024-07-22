#ifndef __OLED_I2C_H
#define __OLED_I2C_H
#include "stm32f1xx_hal.h" 
#include "delay.h"

#define USE_REAL_I2C    (1)
#if(USE_REAL_I2C)


#else
#define             SI2C_GPIO_CLK                        RCC_APB2Periph_GPIOB
#define             SI2C_GPIO_APBxClock_FUN              RCC_APB2PeriphClockCmd
#define             SI2C_PORT                            GPIOB
#define             SI2C_SCL_PIN                         GPIO_Pin_6
#define             SI2C_SDA_PIN                         GPIO_Pin_7

#define SSD1306_ADDR    0x78
void ssd1306_generate_a_frame(uint8_t *p, uint16_t size);

void test_oled(void);
#endif



#endif
