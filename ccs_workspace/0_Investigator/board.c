/*
 * Name: board.c
 * Author: Marquez Jones
 * Desc: all io init and use functions
 *       including necessary mutex/semaphores
 */

#include "board.h"

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/SPI.h>
#include <ti/display/Display.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/* driver config */
#include "ti_drivers_config.h"

I2C_Handle      i2c;
I2C_Params      i2cParams;
I2C_Transaction i2cTransaction;
sem_t i2c_mutex;

SPI_Handle      spi;
SPI_Params      spiParams;
SPI_Transaction spiTransaction;
//sem_t spi_mutex;

UART_Handle uart;
UART_Params uartParams;
sem_t uart_mutex;

Display_Handle display;

/**************************
 * Name: gator_board_init()
 * Desc: initialize project IO
 * SPI
 * I2C
 * UART
 * Will also populate handles
 **************************/
void gator_board_init(void){

    /* Call driver init functions */
    Display_init();
    GPIO_init();
    I2C_init();
    SPI_init();
    UART_init();

    /* sem init */
    sem_init(&i2c_mutex, 0, 1);
    sem_init(&uart_mutex, 0, 1);

    /* config GPIO */
//    CAM_CS_HIGH();
    acam_spi_cs_high();

    /* display init */
    /* Open the UART display for output */
    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
//        Display_printf(display, 0, 0, "Error Initializing Display\n");
        while (1);
    }

    /* i2c init */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(CONFIG_I2C_0, &i2cParams);
    if (i2c == NULL) {
//        Display_printf(display, 0, 0, "Error Initializing I2C\n");
        while (1);
    }
    else {
//        Display_printf(display, 0, 0, "I2C Initialized!\n");
    }

    /* spi init */
    SPI_Params_init(&spiParams);
    spiParams.frameFormat = SPI_POL0_PHA0;
    spiParams.bitRate = 4000000; //acam maxes out at 8 MHz
    spi = SPI_open(CONFIG_SPI_0, &spiParams);
    if (spi == NULL) {
//        Display_printf(display, 0, 0, "Error initializing master SPI\n");
        while (1);
    }
    else {
//        Display_printf(display, 0, 0, "Master SPI initialized\n");
    }

    /* uart init */
    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_FULL;
    uartParams.baudRate = 115200;
    uart = UART_open(CONFIG_UART_0, &uartParams);

    if (uart == NULL) {
        /* UART_open() failed */
//        Display_printf(display, 0, 0, "UART FAILED\n");
        while (1);
    }
    Display_printf(display, 0, 0, "Hardware initialized\n");
}
/*
 * In compliance with AdruCAM_Generic
 *
 * Functions designed to work with ArduCAM
 * These should also include necessary mutex/semaphore code
 */
int acam_spi_write(uint8_t *data, uint32_t data_len){
//    CAM_CS_LOW();
    spiTransaction.count = data_len;
    spiTransaction.txBuf = data;
    spiTransaction.rxBuf = data;
    if( !SPI_transfer(spi, &spiTransaction) ){
        return -1;
    }

//    Display_printf(display, 0, 0, "SPI write called %d bytes\n" , data_len);
//    CAM_CS_HIGH();
    return 0;
}
int acam_spi_read(uint8_t *data, uint32_t data_len){
//    CAM_CS_LOW();
    spiTransaction.count = data_len;
    spiTransaction.txBuf = data;
    spiTransaction.rxBuf = data;
    if( !SPI_transfer(spi, &spiTransaction) ){
        return -1;
    }
//    CAM_CS_HIGH();
    return 0;
}

void acam_spi_cs_high(void){
    GPIO_write(CAM_SPI_SS, 1);
}

void acam_spi_cs_low(void){
    GPIO_write(CAM_SPI_SS, 0);
}

int i2c_write(uint8_t slave_addr,uint8_t *data,uint32_t data_len){
    sem_wait(&i2c_mutex);
    i2cTransaction.writeBuf = data;
    i2cTransaction.writeCount = data_len;
    i2cTransaction.readCount = 0;
    i2cTransaction.slaveAddress = slave_addr;
    if( I2C_transfer(i2c,&i2cTransaction) ){
        sem_post(&i2c_mutex);
        return 0;
    }
    sem_post(&i2c_mutex);
    return -1;
}
int i2c_read(uint8_t slave_addr,uint8_t *data,uint32_t data_len){
    sem_wait(&i2c_mutex);
    i2cTransaction.readBuf = data;
    i2cTransaction.readCount = data_len;
    i2cTransaction.writeCount = 0;
    i2cTransaction.slaveAddress = slave_addr;
    if( I2C_transfer(i2c,&i2cTransaction) ){
        sem_post(&i2c_mutex);
        return 0;
    }
    sem_post(&i2c_mutex);
    return -1;
}
void acam_delay_ms(uint32_t ms){
    Task_sleep(ms); // yield for x amount of ms
}

int uart_write(uint8_t *data,uint32_t size){
    sem_wait(&uart_mutex);
    UART_write(uart, data, size);
    sem_post(&uart_mutex);
}
int uart_read(uint8_t *data,uint32_t size){
    sem_wait(&uart_mutex);
    UART_read(uart, data, size);
    sem_post(&uart_mutex);
}


