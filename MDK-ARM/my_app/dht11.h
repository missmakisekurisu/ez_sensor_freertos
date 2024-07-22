#ifndef __DHT11_H
#define __DHT11_H
#include "stm32f1xx_hal.h"               

#define DHT11_SDA_PORT           GPIOA
#define DHT11_SDA_PIN            GPIO_PIN_7
#define DHT11_GPIO_CLK_ENABLE()    __GPIOA_CLK_ENABLE()


#define SET_PIN_DHT11   HAL_GPIO_WritePin(DHT11_SDA_PORT,GPIO_PIN_7,GPIO_PIN_SET)
#define RESET_PIN_DHT11 HAL_GPIO_WritePin(DHT11_SDA_PORT,GPIO_PIN_7,GPIO_PIN_RESET)
#define READ_BIT_DHT11  HAL_GPIO_ReadPin(DHT11_SDA_PORT, DHT11_SDA_PIN)

void DHT11_main_task(void);
extern uint8_t recDataDHT11[5];
#endif


