/*
 * Copyright (c) 2016, Texas Instruments Incorporated
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
 *  ======== main_tirtos.c ========
 */
#include <stdint.h>

/* POSIX Header files */
#include <pthread.h>

/* RTOS header files */
#include <ti/sysbios/BIOS.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>

/* TI-DRIVERS Header files */

#include "ti_drivers_config.h"

/* investigator includes */
#include "shared.h"
#include "pingpong_ipc.h"
#include "board.h"

extern void * mainThread(void *arg0);

/* application threads */
pthread_t camThreadObj = (pthread_t)NULL;
extern void *camThread(void *arg0);

pthread_t sensorThreadObj = (pthread_t)NULL;
extern void *sensorThread(void *arg0);

pthread_t controlThreadObj = (pthread_t)NULL;
extern void *controlThread(void *arg0);

/* Stack size in bytes */
#define THREADSTACKSIZE    4096

/*
 *  ======== main ========
 */
int main(void)
{
    pthread_t thread;
    pthread_attr_t pAttrs;
    struct sched_param priParam;
    int retc;
    int detachState;

    /* Call board init functions */
    Board_init();

    /* hardware inits */
    gator_board_init();

    /* ipc inits */

    /* wheelson initial position on power up */
    init_position(&position_g,0,0);
    init_heading();

    /* pdu fifo inits */
    pdu_fifo_init(&tx_pdu_fifo_g,tx_pdu_buffer_g,PDU_FIFO_SIZE);
    pdu_fifo_init(&rx_pdu_fifo_g,rx_pdu_buffer_g,PDU_FIFO_SIZE);

    /* cam ipc */
    for(uint32_t i=0;i<NUM_CAM_BUFFERS;i++){
        ppipc_init_buff(&cam_buffers[i],&cam_data[i][0],CAM_BUFFER_SIZE);
    }
    ppipc_init_tab(&cam_buff_tab,cam_buffers,NUM_CAM_BUFFERS);

    /* Set priority and stack size attributes */
    pthread_attr_init(&pAttrs);

    /* network handling */
    priParam.sched_priority = 1;
    detachState = PTHREAD_CREATE_DETACHED;
    retc = pthread_attr_setdetachstate(&pAttrs, detachState);
    pthread_attr_setschedparam(&pAttrs, &priParam);
    retc |= pthread_attr_setstacksize(&pAttrs, THREADSTACKSIZE);
    retc = pthread_create(&thread, &pAttrs, mainThread, NULL);
    if(retc != 0)
    {
        /* pthread_create() failed */
        while(1);
    }

    /* Start the Camera Thread */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = DEFAULT_TASK_PRI;
    retc|= pthread_attr_setschedparam(&pAttrs, &priParam);
    retc |= pthread_attr_setstacksize(&pAttrs, TASK_STACK_SIZE);
    retc = pthread_create(&camThreadObj, &pAttrs, camThread, NULL);
    if(retc)
    {
        while(1);
    }

    /* Start the sensor Handler */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = DEFAULT_TASK_PRI;
    retc|= pthread_attr_setschedparam(&pAttrs, &priParam);
    retc |= pthread_attr_setstacksize(&pAttrs, TASK_STACK_SIZE);
    retc = pthread_create(&sensorThreadObj, &pAttrs, sensorThread, NULL);
    if(retc)
    {
        while(1);
    }

    /* Start the control Handler */
    pthread_attr_init(&pAttrs);
    priParam.sched_priority = DEFAULT_TASK_PRI;
    retc|= pthread_attr_setschedparam(&pAttrs, &priParam);
    retc |= pthread_attr_setstacksize(&pAttrs, TASK_STACK_SIZE);
    retc = pthread_create(&controlThreadObj, &pAttrs, controlThread, NULL);
    if(retc)
    {
        while(1);
    }
    BIOS_start();

    return (0);
}

/*
 *  ======== dummyOutput ========
 *  Dummy SysMin output function needed for benchmarks and size comparison
 *  of FreeRTOS and TI-RTOS solutions.
 */
void dummyOutput(void)
{
}
