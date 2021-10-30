/*
 * Name: controlHandler.c
 * Desc: has multiple tasks
 *       1) must send updates to client about status updates ie when finishes moving
 *       2) must receive commands and act on them
 *       3) send uart messages to motor board
 *       4)
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

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

typedef uint8_t motor_cmd_t;
const motor_cmd_t TURN_RIGHT = 'R';
const motor_cmd_t TURN_LEFT  = 'L';
const motor_cmd_t MOVE_FORW  = 'F';
const motor_cmd_t MOVE_RVRS  = 'R';
const motor_cmd_t MOTOR_STOP = 'S';

typedef enum state{
    ST_WAIT_CMD, /* wait for command via pipe */
    ST_EXE_CMD,  /* execution state ; will send updates here */
    ST_COL,      /* collision handling ; will send updates here */
    ST_RESET,    /* robot reset operation */
}state_t;

/* wait handler */
static void wait_handler(state_t *next, pdu_cmd_t *out_cmd );

/* exe handler */
static void exe_handler(state_t *next, pdu_cmd_t in_cmd );

/* collision handler */
static void col_handler(state_t *next);

/* reset handler */
static void reset_handler(state_t *next);

/* tell client current pos */
static void send_new_pos(void);

/** global variales **/

static pdu_t rx_pdu;
static pdu_t tx_pdu;

void controlThread(void *arg0){

    state_t state = ST_WAIT_CMD;
    pdu_cmd_t cmd = STOP;
    while(1){

        switch(state){
        case ST_WAIT_CMD:
            Display_printf(display, 0, 0, "WAIT STATE\n");
            wait_handler(&state,&cmd);
            break;
        case ST_EXE_CMD:
            Display_printf(display, 0, 0, "EXE STATE\n");
            exe_handler(&state,cmd);
            break;
        case ST_COL:
            col_handler(&state);
            break;
        case ST_RESET:
            reset_handler(&state);
            break;
        default:
            state = ST_WAIT_CMD;
            break;
        }

//        uint8_t test = 0x55;
//        uart_write(&test,1);
//        Task_sleep(100);
    }
}

/* wait handler */
static void wait_handler(state_t *next, pdu_cmd_t *out_cmd ){

    pdu_fifo_get(&rx_pdu_fifo_g,&rx_pdu);

    switch (input.cmd){
    case MOVE:
        break;
    case STOP:
        break;
    case RESET:
        break;
    default:
        break;
    }

    /* echo the pdu back for now */
    pdu_fifo_put(&tx_pdu_fifo_g,&rx_pdu);


    *next = ST_EXE_CMD;
}

/* exe handler */
static void exe_handler(state_t *next, pdu_cmd_t in_cmd ){

    /* do some work */
    send_new_pos();

    *next = ST_WAIT_CMD;
}

/* tell client current pos */
/* basically uses all global data */
static void send_new_pos(void){

    pdu_t out_pdu;

    out_pdu.cmd = UPDATE;
    get_position(&out_pdu.x,&out_pdu.y);

    sprintf(out_pdu.msg,"POS UPDATE: {%u , %u} ",out_pdu.x,out_pdu.y);
    out_pdu.msg_len = strlen(out_pdu.msg);

    /* send update */
    pdu_fifo_put(&tx_pdu_fifo_g,&out_pdu);

}

/* collision handler */
static void col_handler(state_t *next){

}

/* reset handler */
static void reset_handler(state_t *next){

}
