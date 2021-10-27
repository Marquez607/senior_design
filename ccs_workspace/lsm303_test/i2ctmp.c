/*
 * Copyright (c) 2018-2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/display/Display.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#include "LSM303DLH.h"

#define TASKSTACKSIZE       640

static Display_Handle display;

static lsm303_t mag_sensor;
static int mag_i2c_write(uint8_t slave_addr, uint8_t *buffer, uint32_t size);
static int mag_i2c_read(uint8_t slave_addr, uint8_t *buffer, uint32_t size);

static print_data(float x, float y, float z, float t);

static I2C_Handle      i2c;
static I2C_Params      i2cParams;
static I2C_Transaction i2cTransaction;

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{

    /* Call driver init functions */
    Display_init();
//    GPIO_init();
    I2C_init();

    /* Open the UART display for output */
    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        while (1);
    }

    /* Create I2C for usage */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(CONFIG_I2C_TMP, &i2cParams);
    if (i2c == NULL) {
        Display_printf(display, 0, 0, "Error Initializing I2C\n");
        while (1);
    }
    else {
        Display_printf(display, 0, 0, "I2C Initialized!\n");
    }

    /* init mag sensor params */
    LSM303_struct_init(&mag_sensor);
    mag_sensor.accel_addr = LSM303_ADDRESS_MAG; //test done on lsm303dlhc which has one i2c line
    mag_sensor.i2c_read = mag_i2c_read;
    mag_sensor.i2c_write = mag_i2c_write;

    /* need to init sensor after adding function pointers */
    int rc = 0;
    rc = LSM303_sensor_init(&mag_sensor);
    while(rc < 0){
        rc = LSM303_sensor_init(&mag_sensor);
        Display_printf(display, 0, 0, "Error Initializing LM303\n");
    }

    float mag_x = 0.0;
    float mag_y = 0.0;
    float mag_z = 0.0;

    LSM303_getOrientation(&mag_sensor,&mag_x,&mag_y,&mag_z);

    while(1){
        LSM303_getOrientation(&mag_sensor,&mag_x, &mag_y, &mag_z);
        print_data(mag_x,mag_y,mag_z,0.0);
        Task_sleep(500);
    }

    I2C_close(i2c);
    Display_printf(display, 0, 0, "I2C closed!");

    return (NULL);
}

static int mag_i2c_write(uint8_t slave_addr, uint8_t *buffer, uint32_t size){
    i2cTransaction.writeBuf     = buffer;
    i2cTransaction.writeCount   = size;
    i2cTransaction.readCount    = 0;
    i2cTransaction.slaveAddress = slave_addr;
    if( I2C_transfer(i2c, &i2cTransaction) < 0 ){
        return -1;
    }
    return 0;
}

static int mag_i2c_read(uint8_t slave_addr, uint8_t *buffer, uint32_t size){
    i2cTransaction.writeCount   = 0;
    i2cTransaction.readBuf      = buffer;
    i2cTransaction.readCount    = size;
    i2cTransaction.slaveAddress = slave_addr;
    if( I2C_transfer(i2c, &i2cTransaction) < 0 ){
        return -1;
    }
    return 0;
}

static print_data(float x, float y, float z, float t){
    Display_printf(display, 0, 0, "x: %.1f y: %.1f z: %.1f t: %.1f",x,y,z,t);
}

