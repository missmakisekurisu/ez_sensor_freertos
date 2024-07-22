#include "oled_i2c.h"
#include "bmp.h"
#if(USE_REAL_I2C)

#else
#define SET_SCL       GPIO_SetBits(SI2C_PORT, SI2C_SCL_PIN)
#define RESET_SCL     GPIO_ResetBits(SI2C_PORT, SI2C_SCL_PIN)
#define SET_SDA       GPIO_SetBits(SI2C_PORT, SI2C_SDA_PIN)
#define RESET_SDA     GPIO_ResetBits(SI2C_PORT, SI2C_SDA_PIN)
#define READ_SDA      GPIO_ReadInputDataBit(SI2C_PORT, SI2C_SDA_PIN)


#define SI2C_HALF_PERIOD    (2U)
#define SI2C_GENERATE_START {SET_SCL;SET_SDA;delay_us(SI2C_HALF_PERIOD);RESET_SDA;    RESET_SCL;delay_us(SI2C_HALF_PERIOD);}
#define SI2C_GENERATE_STOP  {SET_SCL    ;SET_SDA;delay_us(SI2C_HALF_PERIOD);}
#define SI2C_IGNORE_ACK     {RESET_SDA;SET_SCL;delay_us(SI2C_HALF_PERIOD);RESET_SCL;delay_us(SI2C_HALF_PERIOD);}

static void sim_i2c_gpio_init_transmit(void)
{   
    SI2C_GPIO_APBxClock_FUN(SI2C_GPIO_CLK, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure = {
        .GPIO_Pin = SI2C_SCL_PIN | SI2C_SDA_PIN,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode = GPIO_Mode_Out_PP,
    };
	GPIO_Init(SI2C_PORT, &GPIO_InitStructure);    
}

//used after transmit init
static void sim_i2c_gpio_init_receive(void)
{   
    SI2C_GPIO_APBxClock_FUN(SI2C_GPIO_CLK, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure = {
        .GPIO_Pin = SI2C_SDA_PIN,
        .GPIO_Speed = GPIO_Speed_50MHz,
        .GPIO_Mode = GPIO_Mode_IPU,
    };
	GPIO_Init(SI2C_PORT, &GPIO_InitStructure);
}




static void sim_i2c_send_8bit(uint8_t data){
    for(uint8_t i = 0; i < 8; i++, data <<=1){        
        if(data & 0x80){SET_SDA;}else{RESET_SDA;}         
        SET_SCL;
        delay_us(SI2C_HALF_PERIOD);
        RESET_SCL;
        delay_us(SI2C_HALF_PERIOD);        
    }  
    SI2C_IGNORE_ACK  
}

static uint8_t sim_i2c_receive_8bit(uint8_t data){
    uint8_t recData = 0U;
    sim_i2c_gpio_init_receive();
    for(uint8_t i = 0; i < 8; i++, data <<=1){               
        SET_SCL;
        delay_us(SI2C_HALF_PERIOD);
        recData |= READ_SDA << (7-i);
        RESET_SCL;
        delay_us(SI2C_HALF_PERIOD);        
    }  
    SI2C_IGNORE_ACK  
    return recData;
}

static void sim_i2c_transmit(uint8_t slaveAddr, uint8_t *data, uint8_t size){
    SI2C_GENERATE_START
    sim_i2c_send_8bit(slaveAddr);   
    for(uint8_t i = 0; i < size; i++){
        sim_i2c_send_8bit(data[i]);
    }  
    SI2C_GENERATE_STOP
}

static void sim_i2c_transmit_mem(uint8_t slaveAddr, uint8_t memAddr, uint8_t *data, uint16_t size){
    SI2C_GENERATE_START
    sim_i2c_send_8bit(slaveAddr);   
    sim_i2c_send_8bit(memAddr); 
    for(uint8_t i = 0; i < size; i++){
        sim_i2c_send_8bit(data[i]);
    }  
    SI2C_GENERATE_STOP
}

static void ssd1306_wr_cmd(uint8_t cmd){
    sim_i2c_transmit_mem(SSD1306_ADDR, 0x00, &cmd, 1);
}

static void ssd1306_wr_data(uint8_t data){
    sim_i2c_transmit_mem(SSD1306_ADDR, 0x40,& data, 1);
}


static uint8_t CMD_Data[27]={
0xAE, 0x00, 0x10, 0x40, 0xB0, 0x81, 0xFF, 0xA1, 0xA6, 0xA8, 0x3F,
					
0xC8, 0xD3, 0x00, 0xD5, 0x80, 0xD8, 0x05, 0xD9, 0xF1, 0xDA, 0x12,
					
0xD8, 0x30, 0x8D, 0x14, 0xAF};

static void ssd1306_init(void){
    TIM_delay_ms(800U);
    for(uint8_t i = 0; i < 27U; i++){
        ssd1306_wr_cmd(CMD_Data[i]);
    } 
}

void ssd1306_generate_a_frame(uint8_t *p, uint16_t size){
    ssd1306_wr_cmd(0x20); 
    ssd1306_wr_cmd(0x00);         		
    //column 0~127
    ssd1306_wr_cmd(0x21); 
    ssd1306_wr_cmd(0x00); 
    ssd1306_wr_cmd(0x7F); 
    //page 0~7
    ssd1306_wr_cmd(0x22); 
    ssd1306_wr_cmd(0x00); 
    ssd1306_wr_cmd(0x07);     
    for(uint16_t i = 0; i< size;i++){
        ssd1306_wr_data(p[i]);
    }
}

static void ssd1306_set_s32_pos(uint8_t x, uint8_t y){
    ssd1306_wr_cmd(0x20); 
    ssd1306_wr_cmd(0x00);         		
    //column x~x+32
    ssd1306_wr_cmd(0x21); 
    ssd1306_wr_cmd(x); 
    ssd1306_wr_cmd(x+32); 
    //page 0~7
    ssd1306_wr_cmd(0x22); 
    ssd1306_wr_cmd(y); 
    ssd1306_wr_cmd(y+4);  
}
static void ssd1306_show_my_s32_char(uint8_t charNum){
     for(uint16_t i = 0; i< 128;i++){
        ssd1306_wr_data(hugeFontS32[charNum][i]);
    }
}

void test_oled(void){
    ssd1306_set_s32_pos(47, 1);
    ssd1306_show_my_s32_char(0);
}

void oled_i2c_init(void){
    sim_i2c_gpio_init_transmit();
}
void HAL_I2C_Mem_Write(uint8_t DevAddress, uint8_t MemAddress,uint8_t *pData, uint16_t Size){
    sim_i2c_transmit_mem(DevAddress, MemAddress, pData, Size);
}
#endif
