/*
 * Name: sensorHandler.c
 * Author: Marquez Jones
 * Desc: i2c sensor handling
 */

/* std lib */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>


/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>
#include <ti/display/Display.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

#include "shared.h"
#include "board.h"
#include "sensor_drivers/LSM303DLH.h"

#define MSP_SLAVE_ADDR 0x48
#define LCD_RESET 0xFF /* lcd reset command to slave */
#define PI 3.14159f

static int lcd_reset(void);
static int lcd_write(uint8_t data);
static int lcd_string(char *str);

extern Display_Handle display;

static lsm303_t mag_sensor;

void *sensorThread(void *arg0)
{
    LSM303_struct_init(&mag_sensor);
    mag_sensor.i2c_read = i2c_read;
    mag_sensor.i2c_write = i2c_write;

    /* need to init sensor after adding function pointers */
    int rc = 0;
    rc = LSM303_sensor_init(&mag_sensor);
    if(rc < 0){
        Display_printf(display, 0, 0, "Error Initializing LM303\n");
        while(1);
    }
    LSM303_setMagRate(&mag_sensor,LSM303_MAGRATE_75);
    float mag_x = 0.0;
    float mag_y = 0.0;
    float mag_z = 0.0;
    char lcd_str[20];
    while(1){

        LSM303_getOrientation(&mag_sensor,&mag_x, &mag_y, &mag_z);
        float heading = (atan2(mag_y,mag_x) * 180) / PI;
        if(heading < 0.0){
            heading += 360.0;
        }

        sprintf(lcd_str,"Head: %.1f",heading);

        lcd_reset();
        lcd_string(lcd_str);
        Task_sleep(1000);

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

