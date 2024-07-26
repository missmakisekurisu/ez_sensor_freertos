#ifndef __OLED_I2C_H
#define __OLED_I2C_H
#include "stm32f1xx_hal.h" 
#include "i2c.h"

#define USE_REAL_I2C    (1)
#if(USE_REAL_I2C)


#else
#define             SI2C_GPIO_CLK                        RCC_APB2Periph_GPIOB
#define             SI2C_GPIO_APBxClock_FUN              RCC_APB2PeriphClockCmd
#define             SI2C_PORT                            GPIOB
#define             SI2C_SCL_PIN                         GPIO_Pin_6
#define             SI2C_SDA_PIN                         GPIO_Pin_7
void sim_i2c_transmit(uint8_t slaveAddr, uint8_t *data, uint8_t size);
void sim_i2c_transmit_mem(uint8_t slaveAddr, uint8_t *data, uint8_t size);

#endif
#define SSD1306_ADDR    	0x78
#define SSD1306_CMD_MEM		0x00
#define SSD1306_DATA_MEM	0x40


void oled_clear(void);
void test_oled(void);

#endif
