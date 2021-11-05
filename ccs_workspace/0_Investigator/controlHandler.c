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
#include "board.h"
#include "sensor_drivers/LSM303DLH.h"

#define ROTATE_TIME_MS 250 /*time between sending a rotate command */
#define FORWARD_TIME_MS 2000/* amount of time that counts as one dist unit */
#define HEADING_ERROR_DEG 5 /* allowed heading error when rotating */

extern Display_Handle display;

typedef uint8_t motor_cmd_t;
const motor_cmd_t MOTOR_TURN_RIGHT = 'R';
const motor_cmd_t MOTOR_TURN_LEFT  = 'L';
const motor_cmd_t MOVE_FORW  = 'F';
const motor_cmd_t MOVE_RVRS  = 'R';
const motor_cmd_t MOTOR_STOP = 'S';

typedef enum state{
    ST_WAIT_CMD, /* wait for command via pipe */
    ST_EXE_CMD,  /* execution state ; will send updates here */
    ST_COL,      /* collision handling ; will send updates here */
    ST_RESET,    /* robot reset operation */
}state_t;


/**************************** STATE HANDLER *************************/

/* wait handler */
static void wait_handler(state_t *next);

/* exe handler */
static void exe_handler(state_t *next);

/* collision handler */
static void col_handler(state_t *next);

/* reset handler */
static void reset_handler(state_t *next);

/**************************** MSG FUNCTIONS *************************/

/* tell client current pos */
static void send_new_pos(void);

/* tell client robot is blocked */
static void send_blocked_msg(void);

/* send some kind of message to the client */
static void send_msg(char *msg);

/**************************** MOVE ROBOT *************************/

/* send command to motor board */
static int send_motor_cmd(motor_cmd_t cmd);

/* rotate to heading */
/* makes robot rotate to new heading by sending motor commands*/
static int rotate_to_heading(float new_heading);

/** global variales **/

static pdu_t rx_pdu;
static pdu_t tx_pdu;

void controlThread(void *arg0){

    lcd_reset();
    state_t state = ST_WAIT_CMD;
    pdu_cmd_t cmd = PDU_STOP;

    float new_headings[4] = {HEAD_N,HEAD_S,HEAD_E,HEAD_W};
    while(1){
        Display_printf(display,0,0,"NEW HEADING");
        for(int i=0;i<4;i++){
            rotate_to_heading(new_headings[i]);
            Task_sleep(1);
        }
    }


    while(1){
        Display_printf(display,0,0,"NEW COMMAND");
        switch(state){
        case ST_WAIT_CMD:
            lcd_reset();
            lcd_string("WAIT STATE");
            wait_handler(&state);
            break;
        case ST_EXE_CMD:
            lcd_reset();
            lcd_string("EXE STATE");
            exe_handler(&state);
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
    }
}

/* wait handler */
static void wait_handler(state_t *next){

    /* stop motor when in this state */
    send_motor_cmd(MOTOR_STOP);

    pdu_fifo_get(&rx_pdu_fifo_g,&rx_pdu);

    switch (rx_pdu.cmd){
    case PDU_MOVE:
        *next = ST_EXE_CMD;
        break;
    case PDU_STOP:
        *next = ST_EXE_CMD;
        break;
    case PDU_RESET:
        *next = ST_RESET;
        break;
    default:
        break;
    }

//    /* echo the pdu back for now */
//    pdu_fifo_put(&tx_pdu_fifo_g,&rx_pdu);

    *next = ST_EXE_CMD;
}

/* exe handler */
static void exe_handler(state_t *next){
    /*
     * note , robot will only move in either x or y direction
     * at a time
     */

    /* parse pdu data to figure out what to do */
    bool done = false;

    do{
        if(rx_pdu.cmd = PDU_MOVE){
            uint8_t new_x = rx_pdu.x; /* dest */
            uint8_t new_y = rx_pdu.y;
            uint8_t curr_x;
            uint8_t curr_y;

            get_position(&curr_x, &curr_y);

            if(curr_x == new_x && curr_y == new_y){
                done = true; /* done with moving */
            }

            /* move x */
            /* get current x */
//            get_position(&curr_x, &curr_y);

            /* move y */
            /* get current y */
//            get_position(&curr_x, &curr_y);

        }
        else{
            send_motor_cmd(MOTOR_STOP);
            done = true;
        }

        /* send update */
        send_new_pos();
    }while(!done);

    send_msg("CMD FINISHED");

    *next = ST_WAIT_CMD;
}

/* tell client current pos */
/* basically uses all global data */
static void send_new_pos(){

    pdu_t out_pdu;

    out_pdu.cmd = PDU_UPDATE;
    get_position(&out_pdu.x,&out_pdu.y);

    sprintf(out_pdu.msg,"POS UPDATE: {%u , %u} ",out_pdu.x,out_pdu.y);
    out_pdu.msg_len = strlen(out_pdu.msg);

    /* send update */
    pdu_fifo_put(&tx_pdu_fifo_g,&out_pdu);

}

/* send some kind of message to the client */
static void send_msg(char *msg){

    pdu_t out_pdu;

    out_pdu.cmd = PDU_UPDATE;
    get_position(&out_pdu.x,&out_pdu.y);

    sprintf(out_pdu.msg,"%s",msg);
    out_pdu.msg_len = strlen(out_pdu.msg);

    /* send update */
    pdu_fifo_put(&tx_pdu_fifo_g,&out_pdu);

}


/* send command to motor board */
static int send_motor_cmd(motor_cmd_t cmd){
//    uint8_t data = (uint8_t)cmd;
//    return uart_write(&data,1); /* send one byte for now */
}

/* collision handler */
static void col_handler(state_t *next){

    /* while u_alert is in effect, stay here */
    while(read_u_alert()){
        send_motor_cmd(MOTOR_STOP);
        send_blocked_msg(); /* tell client we're blocked */
        Task_sleep(1000);
    }
}

/* reset handler */
static void reset_handler(state_t *next){
    send_motor_cmd(MOTOR_STOP);
}

/* tell client robot is blocked */
static void send_blocked_msg(void){
    pdu_t out_pdu;

    out_pdu.cmd = PDU_BLOCK;
    get_position(&out_pdu.x,&out_pdu.y);

    sprintf(out_pdu.msg,"ROBOT BLOCK: {%u , %u} ",out_pdu.x,out_pdu.y);
    out_pdu.msg_len = strlen(out_pdu.msg);

    /* send update */
    pdu_fifo_put(&tx_pdu_fifo_g,&out_pdu);

}

/* makes robot rotate to new heading by sending motor commands*/
static int rotate_to_heading(float new_heading){

    if(new_heading > 360 || new_heading < 0){
//        Display_printf(display, 0, 0, "BAD");
        return -1;
    }

    send_motor_cmd(MOTOR_STOP); /* stop robot */

    float curr_heading = get_heading();

    while(curr_heading > new_heading + HEADING_ERROR_DEG ||
          curr_heading < new_heading - HEADING_ERROR_DEG){

        Display_printf(display, 0, 0, "CURR HEADING : %.1f",curr_heading);
        Display_printf(display, 0, 0, "TAR HEADING : %.1f",new_heading);

        /* if heading to right ,rotate right */
        if(curr_heading < new_heading - HEADING_ERROR_DEG){
            Display_printf(display, 0, 0, "MOVING RIGHT");
            send_motor_cmd(MOTOR_TURN_RIGHT);
            Task_sleep(ROTATE_TIME_MS);
            send_motor_cmd(MOTOR_STOP);
        }

        curr_heading = get_heading();

        /* if heading left, rotate left */
        if(curr_heading > new_heading + HEADING_ERROR_DEG){
            Display_printf(display, 0, 0, "MOVING LEFT");
            send_motor_cmd(MOTOR_TURN_RIGHT);
            Task_sleep(ROTATE_TIME_MS);
            send_motor_cmd(MOTOR_STOP);

        }

        curr_heading = get_heading();
    }

    Display_printf(display, 0, 0, "ROTATION DONE");

    return 0;
}
