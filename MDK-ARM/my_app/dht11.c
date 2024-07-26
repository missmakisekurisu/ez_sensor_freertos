#include  "dht11.h"
#include  "delay.h"

uint8_t recDataDHT11[5] = {0};

static void master_output_init(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DHT11_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_SDA_PORT, &GPIO_InitStruct);
}


static void master_receive_init(void){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DHT11_SDA_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_SDA_PORT, &GPIO_InitStruct);
}	


static uint8_t read_byte_DHT11(void){
    uint8_t data = 0U;
    for(int i = 0;i < 8;i++){
        //transfer start with 50us RESET
        while(READ_BIT_DHT11 == GPIO_PIN_RESET);
        //SET signal duration = 26-28us: this bit = 0; 
        //SET signal duration = 70us: this bit = 1;
        PY_Delay_us_t(45); 
        if(READ_BIT_DHT11 == GPIO_PIN_SET){
            data += (1 <<(7-i));
        }
        //if bit = 1, need to wait utill SET signal end
        while(READ_BIT_DHT11 == GPIO_PIN_SET);
    }
    return data;
}



static void DHT11_read_task(void){
    uint8_t data[5] = {0};
		/*transfer trigger*/
    master_output_init();  //SDA idle: set Pin 
    SET_PIN_DHT11;
		HAL_Delay(10u);
		
    
    RESET_PIN_DHT11; //master send trigger signal: reset Pin for required at least 18ms
		HAL_Delay(20u);
   
    master_receive_init(); //master shift to receive mode: IPU   
		PY_Delay_us_t(50);//30   
    while(READ_BIT_DHT11 == GPIO_PIN_RESET); //wait DHT11 response in 20-40us, it will reset Pin for 80us    
    while(READ_BIT_DHT11 == GPIO_PIN_SET); //DHT11 response end, it will set Pin for 80us
		
    /*transfer start, 5 bytes*/		
    for(int i = 0;i < 5;i++){
        data[i] =  read_byte_DHT11();      
    }
    //wait for DHT11 end transfer: reset Pin for 50us
    while(READ_BIT_DHT11 == GPIO_PIN_RESET);
    //wire recover to idle
    master_output_init();
    SET_PIN_DHT11;
		
    /*check sum*/
    uint16_t sum = 0;
    for(int j = 0;j <4;j++){
        sum += data[j];
    }
   if(data[4] == (uint8_t)sum){
        recDataDHT11[0] = data[0];
        recDataDHT11[1] = data[1];
        recDataDHT11[2] = data[2];
        recDataDHT11[3] = data[3];
    }

}

void DHT11_main_task(void){
    if(get_tempo_flag(DHT11_TASK)){
        DHT11_read_task();
				clear_tempo_flag(DHT11_TASK);
		}		
}	

