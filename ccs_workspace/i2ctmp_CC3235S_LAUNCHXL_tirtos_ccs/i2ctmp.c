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

/*
 *  ======== i2ctmp.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/display/Display.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define TASKSTACKSIZE       640

#define MAG_ADDR 0x1E //LSM303
#define ACCEL_ADDR 0x18 //BMI055
#define GYRO_ADDR 0x68 //BMI055

static uint8_t slaveAddress;
static Display_Handle display;

static void i2cErrorHandler(I2C_Transaction *transaction,
    Display_Handle display);

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    uint16_t        sample;
    int16_t         temperature;
    uint8_t         txBuffer[1] = {0x55};
    uint8_t         rxBuffer[2];
    int8_t          i;
    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    I2C_Transaction i2cTransaction;

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

    /* Common I2C transaction setup */
    i2cTransaction.writeBuf   = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf    = rxBuffer;
    i2cTransaction.readCount  = 0;

//    i2cTransaction.slaveAddress = 0x1E;
    i2cTransaction.slaveAddress = 0x30;

    int rc;
    while(1){
        rc = I2C_transfer(i2c, &i2cTransaction);
        if (rc < 0){
            Display_printf(display, 0, 0, "\n I2C Error");
        }
        Display_printf(display, 0, 0, "\n Running I2C Test");
        sleep(1);
    }

    I2C_close(i2c);
    Display_printf(display, 0, 0, "I2C closed!");

    return (NULL);
}

/*
 *  ======== i2cErrorHandler ========
 */
static void i2cErrorHandler(I2C_Transaction *transaction,
    Display_Handle display)
{
    switch (transaction->status) {
    case I2C_STATUS_TIMEOUT:
        Display_printf(display, 0, 0, "I2C transaction timed out!");
        break;
    case I2C_STATUS_CLOCK_TIMEOUT:
        Display_printf(display, 0, 0, "I2C serial clock line timed out!");
        break;
    case I2C_STATUS_ADDR_NACK:
        Display_printf(display, 0, 0, "I2C slave address 0x%x not"
            " acknowledged!", transaction->slaveAddress);
        break;
    case I2C_STATUS_DATA_NACK:
        Display_printf(display, 0, 0, "I2C data byte not acknowledged!");
        break;
    case I2C_STATUS_ARB_LOST:
        Display_printf(display, 0, 0, "I2C arbitration to another master!");
        break;
    case I2C_STATUS_INCOMPLETE:
        Display_printf(display, 0, 0, "I2C transaction returned before completion!");
        break;
    case I2C_STATUS_BUS_BUSY:
        Display_printf(display, 0, 0, "I2C bus is already in use!");
        break;
    case I2C_STATUS_CANCEL:
        Display_printf(display, 0, 0, "I2C transaction cancelled!");
        break;
    case I2C_STATUS_INVALID_TRANS:
        Display_printf(display, 0, 0, "I2C transaction invalid!");
        break;
    case I2C_STATUS_ERROR:
        Display_printf(display, 0, 0, "I2C generic error!");
        break;
    default:
        Display_printf(display, 0, 0, "I2C undefined error case!");
        break;
    }
}
