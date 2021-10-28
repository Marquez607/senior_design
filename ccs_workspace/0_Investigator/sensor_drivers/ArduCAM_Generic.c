/****************************************************************
 * Name: ArduCAM_Generic
 * Author: Marquez Jones
 * Desc: This is a generic abstraction for interfacing 
 *  with the ArduCAM OV2640 board; user provides
 *  function pointers for their hardware and this library
 *  handles the logic layers
 * 
 * CAMERA NOTE: this code is only designed to work on the 
 * OV2640, support for other cameras can be added by looking
 * at the original ArduCAM library and further adapting this
 * version
 * 
 * FIFO NOTE: the added on hardware to the OV2640 camera
 * includes a fifo for images (extra ram for buffering)
 * 
 * fifo operations either configure or read data pertaining
 * to the fifo or read data from the fifo itself
 * 
 * Code this was adapted from
 * https://github.com/ArduCAM/ArduCAM_ESP32S_UNO
 * 
 * NOTE: 
 * SPI functions are writing to the ArduCAM IC (ArduCHIP)
 * I2C functions are writing directly to the OVxxxx (OV2640)
 * 
 ****************************************************************/

#include <stdint.h>
#include "ov2640_regs.h"
#include "sensor_drivers/ArduCAM_Generic.h"
#include <stddef.h>

/*******************************************
 * Desc: write 8 bit value to 8 bit register
 *******************************************/
void ACAM_I2C_write_reg8_8(ArduCam_t *cam,uint8_t addr, uint8_t data){
    static uint8_t buffer[2];
    buffer[0] = addr;
    buffer[1] = data;
    // no clue why I have to shift right by one
    // original driver did this, and I tried this on my 
    // hardware and it worked 
    cam->i2c_write(ACAM_I2C_WRITE_ADDR>>1,buffer,2);
}
/*******************************************
 * Desc: read 8 bit value from 8 bit register
 *******************************************/
uint8_t ACAM_I2C_read_reg8_8(ArduCam_t *cam,uint8_t addr){
    static uint8_t buffer[1];

    // no clue why I have to shift right by one
    // original driver did this, and I tried this on my 
    // hardware and it worked 
    buffer[0] = addr;
    cam->i2c_write(ACAM_I2C_READ_ADDR>>1,buffer,1);
    cam->i2c_read(ACAM_I2C_READ_ADDR>>1,buffer,1);
    return buffer[0];
}

/*******************************************
 * Desc: writes to multiple regs based on 
 *       regList input 
 *******************************************/
void ACAM_I2C_write_multi_regs8_8(ArduCam_t *cam, const struct sensor_reg *regList){

    uint8_t reg_addr = 0;
    uint8_t reg_val = 0;
    const struct sensor_reg *next = regList;
    while ( (reg_addr != 0xFF ) | (reg_val != 0xFF) ){
        reg_addr = (next->reg);
        reg_val = (next->val);
        ACAM_I2C_write_reg8_8(cam,reg_addr,reg_val);
        next++;
    }
}
/*******************************************
 * Desc: write 8 bit data to ArduChip
 *******************************************/
void ACAM_SPI_write_reg(ArduCam_t *cam,uint8_t addr, uint8_t data){
    uint8_t buffer[2];
    buffer[0]= addr|0x80;
    buffer[1] = data; // set bit 7 for write
    cam->spi_cs_low();
    cam->spi_write(buffer,2);
    cam->spi_cs_high();
}

/*******************************************
 * Desc: read 8 bit data from ArduChip
 *******************************************/
uint8_t ACAM_SPI_read_reg(ArduCam_t *cam,uint8_t addr){
    uint8_t buffer[2];
    buffer[0] = addr&0x7F; //clear bit 7 for read
    buffer[1] = 0x00;
//    cam->spi_write(buffer,1);
    cam->spi_cs_low();
    cam->spi_read(buffer,2); //read function should also write
    cam->spi_cs_high();
    return buffer[1];
}

/****************************************
 * get bit from register
 ****************************************/
uint8_t ACAM_get_bit(ArduCam_t *cam, uint8_t addr, uint8_t bit){
  uint8_t temp;
  temp = ACAM_SPI_read_reg(cam,addr);
  temp = temp & bit;
  return temp;
}

/************************************
 * Name: ACAM_Init
 * Desc: initializes arducam with input
 *       parameters; will always use
 *       jpeg compression
 * 
 * Must be called before other API functions
 * BUS: I2C, SPI
 ************************************/
void ACAM_Init(ArduCam_t *cam){

    ACAM_I2C_write_reg8_8(cam,RA_DLMT,0x01);
    ACAM_I2C_write_reg8_8(cam,0x12,0x80); //unnamed reg
    cam->delay_ms(100);

    /* configure for JPEG format */
    ACAM_I2C_write_multi_regs8_8(cam,OV2640_JPEG_INIT);
    ACAM_I2C_write_multi_regs8_8(cam,OV2640_YUV422);
    ACAM_I2C_write_multi_regs8_8(cam,OV2640_JPEG);
    ACAM_I2C_write_reg8_8(cam,RA_DLMT,0x01);
    ACAM_I2C_write_reg8_8(cam,0x15,0x80);   //unnamed reg
    ACAM_I2C_write_multi_regs8_8(cam,OV2640_320x240_JPEG);

}
/************************************
 * Desc: clear out the fifo
 * BUS: SPI
 ************************************/
void ACAM_flush_fifo(ArduCam_t *cam){
    ACAM_SPI_write_reg(cam,ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}
/************************************
 * Desc: begin capturing frame to buffer
 * BUS: SPI
 ************************************/
void ACAM_start_capture(ArduCam_t *cam){
    ACAM_SPI_write_reg(cam,ARDUCHIP_FIFO, FIFO_START_MASK);
}
/************************************
 * Desc: 
 * BUS: SPI
 ************************************/
void ACAM_clear_fifo_flag(ArduCam_t *cam){
    ACAM_SPI_write_reg(cam,ARDUCHIP_FIFO, FIFO_CLEAR_MASK);
}
/************************************
 * Desc: get length of fifo
 * BUS: SPI
 ************************************/
uint32_t ACAM_get_fifo_length(ArduCam_t *cam){
    uint32_t len1,len2,len3,length=0;
    len1 = ACAM_SPI_read_reg(cam,FIFO_SIZE1);
    len2 = ACAM_SPI_read_reg(cam,FIFO_SIZE2);
    len3 = ACAM_SPI_read_reg(cam,FIFO_SIZE3);// & 0x7f;
    length = ( (len3 << 16) | (len2 << 8) | (len1) ) & 0x07fffff;
    return length;
}
/************************************
 * Desc: 
 * does not enable CS
 * BUS: SPI
 ************************************/
void ACAM_set_fifo_burst(ArduCam_t *cam){
    static uint8_t buffer[1] = {BURST_FIFO_READ};
    cam->spi_write(buffer,1);
    buffer[0] = 0x00; /* send buffer byte */
    cam->spi_write(buffer,1);
}
/**************************************
 * Desc: reads up to max_len bytes from 
 *       fifo in one operation
 * returns: number of bytes read, -1 on failure
 * max_length must be at least 3
 * 0 for cmd, 1 for dummy, 2 for actual data
 * BUS: SPI 
 **************************************/
int ACAM_set_fifo_burst_read(ArduCam_t *cam,uint8_t *buffer, uint32_t max_len){

    if(buffer == NULL || max_len < 1){
        return -1;
    }

    uint32_t avail = ACAM_get_fifo_length(cam);
    if(avail < 1){
        return -1; //nothing to read
    }

    // only read up to buffer size
    uint32_t read_count = avail;
    if(avail > max_len-2){
        read_count = max_len;
    }
    buffer[0] = BURST_FIFO_READ;
    buffer[1] = 0x00;
//    cam->spi_write(cmd,2);
    cam->spi_read(buffer,read_count);
    return read_count;
}
/************************************
 * Desc: Read 1 byte of data from fifo
 * BUS: SPI 
 ************************************/
uint8_t ACAM_read_fifo(ArduCam_t *cam){
    return ACAM_SPI_read_reg(cam,SINGLE_FIFO_READ);
}
/************************************
 * Desc: set the jpeg size
 * BUS: I2C
 ************************************/
void ACAM_OV2640_set_JPEG_size(ArduCam_t *cam,uint8_t size){
	switch(size)
	{
		case OV2640_160x120:
		    ACAM_I2C_write_multi_regs8_8(cam,OV2640_160x120_JPEG);
			break;
		case OV2640_176x144:
		    ACAM_I2C_write_multi_regs8_8(cam,OV2640_176x144_JPEG);
			break;
		case OV2640_320x240:
		    ACAM_I2C_write_multi_regs8_8(cam,OV2640_320x240_JPEG);
			break;
		case OV2640_352x288:
		    ACAM_I2C_write_multi_regs8_8(cam,OV2640_352x288_JPEG);
			break;
		case OV2640_640x480:
		    ACAM_I2C_write_multi_regs8_8(cam,OV2640_640x480_JPEG);
			break;
		case OV2640_800x600:
		    ACAM_I2C_write_multi_regs8_8(cam,OV2640_800x600_JPEG);
			break;
		case OV2640_1024x768:
		    ACAM_I2C_write_multi_regs8_8(cam,OV2640_1024x768_JPEG);
			break;
		case OV2640_1280x1024:
		    ACAM_I2C_write_multi_regs8_8(cam,OV2640_1280x1024_JPEG);
			break;
		case OV2640_1600x1200:
		    ACAM_I2C_write_multi_regs8_8(cam,OV2640_1600x1200_JPEG);
			break;
		default:
		    ACAM_I2C_write_multi_regs8_8(cam,OV2640_320x240_JPEG);
			break;
	}
}
