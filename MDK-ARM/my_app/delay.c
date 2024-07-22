#include "delay.h"
#include "tim.h"

__IO float usDelayBase;
void PY_usDelayTest(void)
{
  __IO uint32_t firstms, secondms;
  __IO uint32_t counter = 0;

  firstms = HAL_GetTick()+1;
  secondms = firstms+1;

  while(uwTick!=firstms) ;

  while(uwTick!=secondms) counter++;

  usDelayBase = ((float)counter)/1000;
}

void PY_Delay_us_t(uint32_t Delay)
{
  __IO uint32_t delayReg;
  __IO uint32_t usNum = (uint32_t)(Delay*usDelayBase);

  delayReg = 0;
  while(delayReg!=usNum) delayReg++;
}




void delay_us(uint32_t delay_us)
{   
  volatile unsigned int num;
  volatile unsigned int t;


  for (num = 0; num < delay_us; num++)
  {
    t = 6;
    while (t != 0)
    {
      t--;
    }
  }
}




volatile TEMPO_TYPE appTempo[TEMPO_COUNT]={
    {0, 1000U, 100, 0},
    {0, 2000u, 100, 0},
};
    


static void update_app_timer(volatile TEMPO_TYPE *t){
	for(uint8_t i = 0; i < TEMPO_COUNT; i++){
		if(t[i].counter++ >= t[i].num || t[i].flag){
			t[i].flag ++; 
			t[i].counter = 0;    
		}
		if(t[i].flag >= t[i].pendingTime){
			t[i].flag = 0;
		}
	}
}

void tempo_start(void){
	HAL_TIM_Base_Start_IT(&htim3);
	HAL_NVIC_EnableIRQ(TEMPO_TIM_IRQn);
}

void TEMPO_TIM_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);
}


/*put in tim_irqHandler*/
void tempo_task(void){
	update_app_timer(appTempo);
}

bool get_tempo_flag(TEMPO_NAME_TYPE n){
	return (appTempo[n].flag != 0);
}	
