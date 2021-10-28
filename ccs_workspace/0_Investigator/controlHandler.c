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
const motor_cmd_t TURN_RIGHT = 0x30;
const motor_cmd_t TURN_LEFT  = 0x31;
const motor_cmd_t MOVE_FORW  = 0x32;
const motor_cmd_t MOVE_RVRS  = 0x33;
const motor_cmd_t MOTOR_STOP = 0x34;

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

void controlThread(void *arg0){

    state_t state = ST_WAIT_CMD;
    pdu_cmd_t cmd = STOP;
    while(1){

        switch(state){
        case ST_WAIT_CMD:
            wait_handler(&state,&cmd);
            break;
        case ST_EXE_CMD:
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

        uint8_t test = 0x55;
        uart_write(&test,1);
        Task_sleep(100);
    }
}

/* wait handler */
static void wait_handler(state_t *next, pdu_cmd_t *out_cmd ){

}

/* exe handler */
static void exe_handler(state_t *next, pdu_cmd_t in_cmd ){

}

/* collision handler */
static void col_handler(state_t *next){

}

/* reset handler */
static void reset_handler(state_t *next){

}
