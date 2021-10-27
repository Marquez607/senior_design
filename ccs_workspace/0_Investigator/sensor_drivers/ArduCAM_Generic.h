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

#ifndef ARDUCAM_GENERIC_H
#define ARDUCAM_GENERIC_H
#include <stdint.h>
#include <stddef.h>

struct sensor_reg {
    uint8_t reg;
    uint8_t val;
};

typedef struct ArduCam {
    /* 
       NOTE: external software responsible for 
       initializing hardware peripherals

       the function pointers are functions the external software
       should provide in order to interface with the ArduCAM module
    */
    /*
        Application software should not handle CS
        the ArduCAM driver will maninpulate it as necessary

        NOTE: even on a spi_read, the driver may populate the buffer
              ensure provided functions don't manupulate buffer data
              on spi_read

        data -> buffer for data to either retrieve or transmit 
        data_len -> length of buffer

        pseudo code:
        write:

            for(int i=0;i<data_len;i++){
                spi_write(data[i])
            }

        read:

            for(int i=0;i<data_len;i++){
                data[i] = spi_read();
            }

     */
    int (*spi_write)(uint8_t *data, uint32_t data_len);
    int (*spi_read)(uint8_t *data, uint32_t data_len);
    void (*spi_cs_low)(void);
    void (*spi_cs_high)(void);

    /*
      NOTE: the ArduCAM has fixed addresses of 0x60 and 0x61
      the address here is the register address on the module
      
      slave_addr -> chip address 
      data -> driver will pass in a pointer to a data buffer
      data_len -> driver will specify how many bytes to read/write

      return -1 on failure
     */
    int (*i2c_write)(uint8_t slave_addr,uint8_t *data,uint32_t data_len);
    int (*i2c_read)(uint8_t slave_addr,uint8_t *data,uint32_t data_len);


    /**************************
     * functions just needs to be
     * able to delay an ms
     **************************/
    void (*delay_ms)(uint32_t ms); 

}ArduCam_t;


/****************************************************/
/* Sensor related definition 												*/
/****************************************************/
#define BMP 	0
#define JPEG	1

#define ACAM_I2C_WRITE_ADDR 0x60
#define ACAM_I2C_READ_ADDR 0x61 

#define OV7670		0	
#define MT9D111_A	1
#define OV7675		2
#define OV5642		3
#define OV3640  	4
#define OV2640  	5
#define OV9655		6
#define MT9M112		7
#define OV7725		8
#define OV7660		9
#define MT9M001 	10
#define OV5640 		11
#define MT9D111_B	12
#define OV9650		13
#define MT9V111		14
#define MT9T112		15
#define MT9D112		16
#define MT9V034 	17

#define OV2640_160x120 		0	//160x120
#define OV2640_176x144 		1	//176x144
#define OV2640_320x240 		2	//320x240
#define OV2640_352x288 		3	//352x288
#define OV2640_640x480		4	//640x480
#define OV2640_800x600 		5	//800x600
#define OV2640_1024x768		6	//1024x768
#define OV2640_1280x1024	7	//1280x1024
#define OV2640_1600x1200	8	//1600x1200


/****************************************************/
/* I2C Control Definition 													*/
/****************************************************/
#define I2C_ADDR_8BIT 0
#define I2C_ADDR_16BIT 1
#define I2C_REG_8BIT 0
#define I2C_REG_16BIT 1
#define I2C_DAT_8BIT 0
#define I2C_DAT_16BIT 1

/* Register initialization tables for SENSORs */
/* Terminating list entry for reg */
#define SENSOR_REG_TERM_8BIT                0xFF
#define SENSOR_REG_TERM_16BIT               0xFFFF
/* Terminating list entry for val */
#define SENSOR_VAL_TERM_8BIT                0xFF
#define SENSOR_VAL_TERM_16BIT               0xFFFF

//Define maximum frame buffer size
#define MAX_FIFO_SIZE		0x5FFFF			//384KByte

/****************************************************/
/* ArduChip registers definition 											*/
/****************************************************/
#define RWBIT									0x80  //READ AND WRITE BIT IS BIT[7]
#define RA_DLMT                 0xFF

#define ARDUCHIP_TEST1       	0x00  //TEST register

#define ARDUCHIP_FRAMES			0x01  //FRAME control register, Bit[2:0] = Number of frames to be captured																		//On 5MP_Plus platforms bit[2:0] = 7 means continuous capture until frame buffer is full

#define ARDUCHIP_MODE      		0x02  //Mode register
#define MCU2LCD_MODE       		0x00
#define CAM2LCD_MODE       		0x01
#define LCD2MCU_MODE       		0x02

#define ARDUCHIP_TIM       		0x03  //Timming control
#define HREF_LEVEL_MASK    		0x01  //0 = High active , 		1 = Low active
#define VSYNC_LEVEL_MASK   		0x02  //0 = High active , 		1 = Low active
#define LCD_BKEN_MASK      		0x04  //0 = Enable, 					1 = Disable
#define PCLK_DELAY_MASK  		0x08  //0 = data no delay,		1 = data delayed one PCLK
#define MODE_MASK          		0x10  //0 = LCD mode, 				1 = FIFO mode
#define FIFO_PWRDN_MASK	   		0x20  	//0 = Normal operation, 1 = FIFO power down
#define LOW_POWER_MODE			  0x40  	//0 = Normal mode, 			1 = Low power mode

#define ARDUCHIP_FIFO      		0x04  //FIFO and I2C control
#define FIFO_CLEAR_MASK    		0x01
#define FIFO_START_MASK    		0x02
#define FIFO_RDPTR_RST_MASK     0x10
#define FIFO_WRPTR_RST_MASK     0x20

#define ARDUCHIP_GPIO			  0x06  //GPIO Write Register
#if !(defined (ARDUCAM_SHIELD_V2) || defined (ARDUCAM_SHIELD_REVC))
#define GPIO_RESET_MASK			0x01  //0 = Sensor reset,							1 =  Sensor normal operation
#define GPIO_PWDN_MASK			0x02  //0 = Sensor normal operation, 	1 = Sensor standby
#define GPIO_PWREN_MASK			0x04	//0 = Sensor LDO disable, 			1 = sensor LDO enable
#endif

#define BURST_FIFO_READ			0x3C  //Burst FIFO read operation
#define SINGLE_FIFO_READ		0x3D  //Single FIFO read operation

#define ARDUCHIP_REV       		0x40  //ArduCHIP revision
#define VER_LOW_MASK       		0x3F
#define VER_HIGH_MASK      		0xC0

#define ARDUCHIP_TRIG      		0x41  //Trigger source
#define VSYNC_MASK         		0x01
#define SHUTTER_MASK       		0x02
#define CAP_DONE_MASK      		0x08

#define FIFO_SIZE1				0x42  //Camera write FIFO size[7:0] for burst to read
#define FIFO_SIZE2				0x43  //Camera write FIFO size[15:8]
#define FIFO_SIZE3				0x44  //Camera write FIFO size[18:16]

/****************************************
 * get bit from register
 ****************************************/
uint8_t ACAM_get_bit(ArduCam_t *cam,uint8_t addr, uint8_t bit);

/*******************************************
 * Desc: write 8 bit value to 8 bit register
 *******************************************/
void ACAM_I2C_write_reg8_8(ArduCam_t *cam,uint8_t addr, uint8_t data);

/*******************************************
 * Desc: read 8 bit value from 8 bit register
 *******************************************/
uint8_t ACAM_I2C_read_reg8_8(ArduCam_t *cam,uint8_t addr);

/*******************************************
 * Desc: writes to multiple regs based on
 *       regList input
 *******************************************/
void ACAM_I2C_write_multi_regs8_8(ArduCam_t *cam, const struct sensor_reg *regList);

/*******************************************
 * Desc: write 8 bit data to ArduChip
 *******************************************/
void ACAM_SPI_write_reg(ArduCam_t *cam,uint8_t addr, uint8_t data);

/*******************************************
 * Desc: read 8 bit data from ArduChip
 *******************************************/
uint8_t ACAM_SPI_read_reg(ArduCam_t *cam,uint8_t addr);

/************************************
 * Name: ACAM_Init
 * Desc: initializes arducam with input
 *       parameters; will always use
 *       jpeg compression
 * 
 *       Defaults resolution to 320x240
 * 
 * Must be called before other API functions
 * BUS: I2C
 ************************************/
void ACAM_Init(ArduCam_t *cam);

/************************************
 * Desc: clear out the fifo
 * BUS: SPI
 ************************************/
void ACAM_flush_fifo(ArduCam_t *cam);

/************************************
 * Desc: begin capturing frame to buffer
 * BUS: SPI
 ************************************/
void ACAM_start_capture(ArduCam_t *cam);

/************************************
 * Desc: 
 * BUS: SPI
 ************************************/
void ACAM_clear_fifo_flag(ArduCam_t *cam);

/************************************
 * Desc: get length of fifo
 * BUS: SPI
 ************************************/
uint32_t ACAM_get_fifo_length(ArduCam_t *cam);

/************************************
 * Desc: 
 * BUS: SPI
 ************************************/
void ACAM_set_fifo_burst(ArduCam_t *cam);

/**************************************
 * Desc: reads up to max_len bytes from 
 *       fifo in one operation
 * returns: number of bytes read, -1 on failure
 * BUS: SPI 
 **************************************/
int ACAM_set_fifo_burst_read(ArduCam_t *cam,uint8_t *buffer, uint32_t max_len);

/************************************
 * Desc: Read 1 byte of data from fifo
 * BUS: SPI 
 ************************************/
uint8_t ACAM_read_fifo(ArduCam_t *cam);

/************************************
 * Desc: set the jpeg size
 * BUS: I2C
 ************************************/
void ACAM_OV2640_set_JPEG_size(ArduCam_t *cam,uint8_t size);

#endif /*ARDUCAM_GENERIC_H */
