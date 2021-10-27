/*
 * Name: sensorHandler.c
 * Author: Marquez Jones
 * Desc: i2c sensor handling
 */

/* std lib */
#include <stdint.h>
#include <stddef.h>

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>
#include <ti/display/Display.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include "shared.h"
#include <board.h>
#include "sensor_drivers/LSM303DLH.h"

#define MSP_SLAVE_ADDR 0x48
#define LCD_RESET 0xFF /* lcd reset command to slave */

static int lcd_reset(void);
static int lcd_write(uint8_t data);
static int lcd_string(char *str);

extern Display_Handle display;

void *sensorThread(void *arg0)
{
    while(1){
        lcd_reset();
        lcd_string("TEST STRING");
        Task_sleep(500);
    }
}

static int lcd_reset(void){
    return lcd_write(LCD_RESET);
}
static int lcd_write(uint8_t data){
    uint8_t buff = data;
    int rc = i2c_write(MSP_SLAVE_ADDR,&buff,1);
    return rc;
}
static int lcd_string(char *str){

    int rc = 0;
    while(*str){
        rc = lcd_write(*str);
        if(rc < 0){
            return rc;
        }
        str++;
    }
    return rc;
}

