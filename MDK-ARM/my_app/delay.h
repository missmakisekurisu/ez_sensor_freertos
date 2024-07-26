#ifndef _DELAY_H
#define _DELAY_H
#include "stm32f1xx_hal.h"
#include "stdbool.h"

#define TEMPO_TIM                 TIM3
#define TEMPO_TIM_CLK_ENABLE()    __TIM3_CLK_ENABLE()
#define TEMPO_TIM_CLK_DISABLE() 	__TIM3_CLK_DISABLE()

#define TEMPO_TIM_IRQn            TIM3_IRQn
#define TEMPO_TIM_IRQHandler      TIM3_IRQHandler


typedef enum{
	  OLED_FRESH,
    DHT11_TASK,
    TEMPO_COUNT,
}TEMPO_NAME_TYPE;	


typedef struct{
    uint16_t counter;
    uint16_t num; 
    uint16_t pendingTime;
    uint16_t flag;
}TEMPO_TYPE;




void delay_us(uint32_t delay_us);
void PY_usDelayTest(void);
void PY_Delay_us_t(uint32_t Delay);

void tempo_start(void);
void tempo_task(void);

bool get_tempo_flag(TEMPO_NAME_TYPE);
void clear_tempo_flag(TEMPO_NAME_TYPE n);
#endif
