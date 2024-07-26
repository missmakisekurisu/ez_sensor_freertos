#include "oled_i2c.h"
#include "bmp.h"
#include "dht11.h"
#include <stdlib.h>

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

void sim_i2c_transmit(uint8_t slaveAddr, uint8_t *data, uint8_t size){
    SI2C_GENERATE_START
    sim_i2c_send_8bit(slaveAddr);   
    for(uint8_t i = 0; i < size; i++){
        sim_i2c_send_8bit(data[i]);
    }  
    SI2C_GENERATE_STOP
}

void sim_i2c_transmit_mem(uint8_t slaveAddr, uint8_t memAddr, uint8_t *data, uint16_t size){
    SI2C_GENERATE_START
    sim_i2c_send_8bit(slaveAddr);   
    sim_i2c_send_8bit(memAddr); 
    for(uint8_t i = 0; i < size; i++){
        sim_i2c_send_8bit(data[i]);
    }  
    SI2C_GENERATE_STOP
}


#endif

/*frames fresh mode*/
/*hrizontal addressing mode*/


extern const unsigned char F6x8[][6];
extern const unsigned char F8x16[];

uint8_t gRam[8][128] = {0};

static uint8_t c_char[] = {'C','h','e','e','r',' ','u','p','!','\0'};
static uint8_t rh_char[] = {'R','H','\0'};
static uint8_t temp_char[] = {'T','E','M','P','\0'};
	

static void ssd1306_wr_cmd(uint8_t cmd){
	HAL_I2C_Mem_Write(&hi2c1 ,SSD1306_ADDR,SSD1306_CMD_MEM,I2C_MEMADD_SIZE_8BIT,&cmd,1,0x100);
}

static void ssd1306_wr_data(uint8_t data){
	HAL_I2C_Mem_Write(&hi2c1 ,SSD1306_ADDR,SSD1306_DATA_MEM,I2C_MEMADD_SIZE_8BIT,&data,1,0x100);
}

static void ssd1306_wr_data_block(uint8_t* data, uint16_t len){
	HAL_I2C_Mem_Write(&hi2c1 ,SSD1306_ADDR,SSD1306_DATA_MEM,I2C_MEMADD_SIZE_8BIT,data,len,0x100);
}

static uint8_t CMD_Data[27]={
0xAE, 0x00, 0x10, 0x40, 0xB0, 0x81, 0xFF, 0xA1, 0xA6, 0xA8, 0x3F,
					
0xC8, 0xD3, 0x00, 0xD5, 0x80, 0xD8, 0x05, 0xD9, 0xF1, 0xDA, 0x12,
					
0xD8, 0x30, 0x8D, 0x14, 0xAF};

void ssd1306_frame_mode_init(void){
	  HAL_Delay(300U);
    for(uint8_t i = 0; i < 27U; i++){
        ssd1306_wr_cmd(CMD_Data[i]);
    } 
}

static uint8_t gRam_buffer[1024] = {0};
static uint8_t gRam_s32_buffer[128] = {0};
void oled_frame_update(void){
	/*fill data into dma buffer*/
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
	//dma buffer convert to 1 dimension array
	uint8_t i,j;
	for(i = 0; i< 8;i++){
		for(j = 0; j< 128;j++){
				gRam_buffer[i*128+j] = gRam[i][j];
		}
	}
	ssd1306_wr_data_block(gRam_buffer, 1024u);
}


void gRam_clear(void){	 
	uint8_t i,j,k = 0;
	for(i = 0; i< 8;i++){
		for(j = 0; j< 128;j++){
				gRam[i][j] = 0;
		}
	}
}

static void ssd1306_set_s32_pos(uint8_t x, uint8_t y){
    ssd1306_wr_cmd(0x20); 
    ssd1306_wr_cmd(0x00);         		
    //column x~x+32
    ssd1306_wr_cmd(0x21); 
    ssd1306_wr_cmd(x); 
    ssd1306_wr_cmd(x+32); 
    //page y~y+7
    ssd1306_wr_cmd(0x22); 
    ssd1306_wr_cmd(y); 
    ssd1306_wr_cmd(y+4);   //32 = 4*8
}


static void ssd1306_show_s32_char(uint8_t charNum){
//     for(uint16_t i = 0; i< 128;i++){
//        ssd1306_wr_data(hugeFontS32[charNum][i]);
//    }
	for(uint8_t i = 0;i < 128;i++){
		gRam_s32_buffer[i] = hugeFontS32[charNum][i];
	}
	ssd1306_wr_data_block(gRam_s32_buffer,128u);
}

/*
* write 32*32 font in gRam, take 4 page, 32 columns
*/
void gRam_write_s32_char(uint8_t charNum,  uint8_t startPage,uint8_t startColumn){
	uint8_t page, col;
	if(startPage <=7 && startColumn <=127){
		for(page = startPage;page <startPage+4;page++){
			for(col = startColumn;col <startColumn+32;col++){
				gRam[page][col] = hugeFontS32[charNum][32*(page-startPage)+(col-startColumn)];
			}
		}	
	}
}

/*
* write 8*16 font in gRam, take 2 page, 8 columns
*/



void gRam_write_s16_char(uint8_t charNum,uint8_t startPage,uint8_t startColumn){
	uint8_t page, col;
	if(startPage <=7 && startColumn <=127){
		for(page = startPage;page <startPage+2;page++){
			for(col = startColumn;col <startColumn+8;col++){
				gRam[page][col] = F8x16[16*(charNum - 32) + 8*(page-startPage)+(col-startColumn)];
			}
		}
	}		
}

void gRam_write_s16_string(uint8_t* charArr, uint8_t startPage,uint8_t startColumn){
	uint8_t i = 0;
	while(charArr[i] != '\0'){
		gRam_write_s16_char(charArr[i],startPage, startColumn+i*8);
		i++;
	}
}

uint8_t test_itoa;
void gRam_write_s16_number(uint8_t data, uint8_t startPage,uint8_t startColumn){
	test_itoa = (uint8_t)(data/10);
	if(data >= 100){
		gRam_write_s16_char((uint8_t)(data/100) + 48, startPage, startColumn);
	}
	if(data >= 10){
		gRam_write_s16_char((uint8_t)(data/10) + 48, startPage, startColumn+8);
	}
	gRam_write_s16_char((uint8_t)(data%10) + 48, startPage, startColumn+8*2);
}


void DHT11_display(void){
	gRam_write_s16_string(rh_char, 0, 10);
	gRam_write_s16_string(temp_char, 2, 10);	
	gRam_write_s16_number(recDataDHT11[0], 0, 34);
	gRam_write_s16_number(recDataDHT11[2], 2, 50);
}

void test_oled(void){
//	ssd1306_set_s32_pos(47, 1);
//	ssd1306_show_s32_char(0);
	gRam_clear();
	DHT11_display();	
	
//	static uint8_t i = 0;
//	if(i > 2){
//		i = 0;
//	}
//	i++;
//	gRam_write_s32_char(0, 0, 15 + 32*i);
	
//	gRam_write_s32_char(0, 0, 127-32);
	oled_frame_update();
}