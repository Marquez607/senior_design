/*
 * Copyright (c) 2018-2019, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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
 *  ======== spimaster.c ========
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/display/Display.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerMSP432.h>

#define THREADSTACKSIZE (1024)

#define SPI_MSG_LENGTH  (30)
#define MASTER_MSG      ("Hello from master, msg#: ")

#define MAX_LOOP        (10)

static Display_Handle display;

unsigned char masterRxBuffer[SPI_MSG_LENGTH];
unsigned char masterTxBuffer[SPI_MSG_LENGTH];

/* Semaphore to block master until slave is ready for transfer */
sem_t masterSem;

/*
 *  ======== slaveReadyFxn ========
 *  Callback function for the GPIO interrupt on CONFIG_SPI_SLAVE_READY.
 */
void slaveReadyFxn(uint_least8_t index)
{
    sem_post(&masterSem);
}

/*
 *  ======== masterThread ========
 *  Master SPI sends a message to slave while simultaneously receiving a
 *  message from the slave.
 */
void *masterThread(void *arg0)
{
    SPI_Handle      masterSpi;
    SPI_Params      spiParams;
    SPI_Transaction transaction;
    uint32_t        i;
    bool            transferOK;
    int32_t         status;


    /* Open SPI as master (default) */
    SPI_Params_init(&spiParams);
    spiParams.frameFormat = SPI_POL0_PHA0;
    spiParams.bitRate = 4000000;
    masterSpi = SPI_open(CONFIG_SPI_MASTER, &spiParams);
    if (masterSpi == NULL) {
        Display_printf(display, 0, 0, "Error initializing master SPI\n");
        while (1);
    }
    else {
        Display_printf(display, 0, 0, "Master SPI initialized\n");
    }

    /* Copy message to transmit buffer */
    strncpy((char *) masterTxBuffer, MASTER_MSG, SPI_MSG_LENGTH);

    while(1){

            /* Initialize master SPI transaction structure */
            GPIO_write(CONFIG_GPIO_0, 0); //CS low
            masterTxBuffer[1] = 0x55;
            masterTxBuffer[0] = 0x80;
            transaction.count = 2;
            transaction.txBuf = (void *) masterTxBuffer;
            transaction.rxBuf = (void *) masterRxBuffer;

            /* Perform SPI transfer */
            transferOK = SPI_transfer(masterSpi, &transaction);
            if (transferOK) {
//                Display_printf(display, 0, 0, "\nMaster received: %d", masterRxBuffer[1]);
            }
            else {
                Display_printf(display, 0, 0, "Unsuccessful master SPI transfer");
            }
            GPIO_write(CONFIG_GPIO_0, 1); //CS high

            /* Initialize master SPI transaction structure */
            GPIO_write(CONFIG_GPIO_0, 0); //CS low
            masterTxBuffer[1] = 0x00;
            masterTxBuffer[0] = 0x00;
            transaction.count = 2;
            transaction.txBuf = (void *) masterTxBuffer;
            transaction.rxBuf = (void *) masterRxBuffer;

            /* Perform SPI transfer */
            transferOK = SPI_transfer(masterSpi, &transaction);
            if (transferOK) {
                Display_printf(display, 0, 0, "\nMaster received: %d", masterRxBuffer[1]);
            }
            else {
                Display_printf(display, 0, 0, "Unsuccessful master SPI transfer");
            }
            GPIO_write(CONFIG_GPIO_0, 1); //CS high

    }

    SPI_close(masterSpi);

    Display_printf(display, 0, 0, "\nDone");

    return (NULL);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    pthread_t           thread0;
    pthread_attr_t      attrs;
    struct sched_param  priParam;
    int                 retc;
    int                 detachState;

    /* Call driver init functions. */
    Display_init();
    GPIO_init();
    SPI_init();

//    /* Configure the LED pins */
//    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
//    GPIO_setConfig(CONFIG_GPIO_LED_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Set performance level to allow custom SPI Frequency */
    Power_releaseConstraint(PowerMSP432_DISALLOW_PERFLEVEL_4);
    Power_setPerformanceLevel(4);

    /* Open the display for output */
    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        /* Failed to open display driver */
        while (1);
    }

//    /* Turn on user LED */
    GPIO_write(CONFIG_GPIO_0, 1);

    Display_printf(display, 0, 0, "Starting the SPI master example");
    Display_printf(display, 0, 0, "This example requires external wires to be "
        "connected to the header pins. Please see the Board.html for details.\n");

    /* Create application threads */
    pthread_attr_init(&attrs);

    detachState = PTHREAD_CREATE_DETACHED;
    /* Set priority and stack size attributes */
    retc = pthread_attr_setdetachstate(&attrs, detachState);
    if (retc != 0) {
        /* pthread_attr_setdetachstate() failed */
        while (1);
    }

    retc |= pthread_attr_setstacksize(&attrs, THREADSTACKSIZE);
    if (retc != 0) {
        /* pthread_attr_setstacksize() failed */
        while (1);
    }

    /* Create master thread */
    priParam.sched_priority = 1;
    pthread_attr_setschedparam(&attrs, &priParam);

    retc = pthread_create(&thread0, &attrs, masterThread, NULL);
    if (retc != 0) {
        /* pthread_create() failed */
        while (1);
    }

    return (NULL);
}
