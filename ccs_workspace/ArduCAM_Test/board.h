/*
 * Name: board.h
 * Author: Marquez Jones
 * Desc: all io init and use functions
 *       including necessary mutex/semaphores
 */

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>
#include <ti/display/Display.h>

#ifndef BOARD_H_
#define BOARD_H_

extern I2C_Handle      i2c;
extern I2C_Params      i2cParams;
extern I2C_Transaction i2cTransaction;

extern SPI_Handle      spi;
extern SPI_Params      spiParams;
extern SPI_Transaction spiTransaction;

extern Display_Handle display;

//#define CAM_CS_LOW() GPIO_write(CAM_SPI_SS, 0)
//#define CAM_CS_HIGH() GPIO_write(CAM_SPI_SS, 1)

/**************************
 * Name: board_init()
 * Desc: initialize project IO
 * SPI
 * I2C
 * UART
 *
 * Will also populate handles
 **************************/
void board_init(void);

/*
 * In compliance with AdruCAM_Generic
 *
 * Functions designed to work with ArduCAM
 * These should also include necessary mutex/semaphore code
 */
int acam_spi_write(uint8_t *data, uint32_t data_len);
int acam_spi_read(uint8_t *data, uint32_t data_len);
int acam_i2c_write(uint8_t slave_addr,uint8_t *data,uint32_t data_len);
int acam_i2c_read(uint8_t slave_addr,uint8_t *data,uint32_t data_len);
void acam_delay_ms(uint32_t ms);
void acam_spi_cs_high(void);
void acam_spi_cs_low(void);

#endif /* BOARD_H_ */
