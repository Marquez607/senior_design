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
#include <stdlib.h>
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

extern Display_Handle display;

typedef uint8_t motor_cmd_t;
const motor_cmd_t MOTOR_TURN_RIGHT = 'R';
const motor_cmd_t MOTOR_TURN_LEFT  = 'L';
const motor_cmd_t MOVE_FORW  = 'F';
const motor_cmd_t MOVE_RVRS  = 'B';
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

static void send_heading(void);

/**************************** MOVE ROBOT *************************/

/* send command to motor board */
static int send_motor_cmd(motor_cmd_t cmd);

/* rotate to heading */
/* makes robot rotate to new heading by sending motor commands*/
static int rotate_to_heading(float new_heading);

/* figure out heading in order to find next coord */
static float calc_heading(uint8_t new_x, uint8_t new_y);

/*
 * figure out next coord
 * basic pathing algo, go to the next closest grid location
 * will move with new york style coords, no diagonal movements
 */
static int calc_next_coord(uint8_t new_x,
                            uint8_t new_y,
                            uint8_t *next_x,
                            uint8_t *next_y);

/**************************** FAKE HEADING *************************/

/*
 * Fake heading code is for dead reckoning system
 * magnetometer was bad so robot just assumes direction
 * based on number of times it has told motor board to rotate
 * from "north"
 */

float fake_heading;

/*
 *
 */
float get_fake_heading();

/*
 *
 */
void update_fake_heading(float heading);

/**************************** GLOBAL VARIABLES *************************/

static pdu_t rx_pdu;
static pdu_t tx_pdu;

bool blocked_flag = false;


void controlThread(void *arg0){

    fake_heading = HEAD_N;
    lcd_reset();
    state_t state = ST_WAIT_CMD;
    pdu_cmd_t cmd = PDU_STOP;

    while(1){

        Display_printf(display,0,0,"NEW COMMAND");
        if(blocked_flag == true){
            state = ST_COL;
        }
        switch(state){
        case ST_WAIT_CMD:
            lcd_reset();
            lcd_string("WAIT STATE");
            wait_handler(&state);

            break;
        case ST_EXE_CMD:
            lcd_reset();
            lcd_string("EXE STATE");
//            Display_printf(display,0,0,"EXE STATE");
            exe_handler(&state);
            break;

        case ST_COL:
            lcd_reset();
            lcd_string("COLLIDE STATE");
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

void safetyHandler(uint_least8_t index){

    bool val = read_u_alert();
    if(val){
        Display_printf(display,0,0,"U_ALERT: HIGH");
        send_motor_cmd(MOTOR_STOP);
        blocked_flag = true;
        turn_on_speaker();
    }
    else{
        Display_printf(display,0,0,"U_ALERT: LOW");
        blocked_flag = false;
        turn_off_speaker();
    }

    return;
}

/* wait handler */
static void wait_handler(state_t *next){

    /* stop motor when in this state */
    send_motor_cmd(MOTOR_STOP);

    pdu_fifo_get(&rx_pdu_fifo_g,&rx_pdu);
//    pdu_fifo_put(&tx_pdu_fifo_g,&rx_pdu);

    switch (rx_pdu.cmd){
    case PDU_MOVE:
//        Display_printf(display, 0, 0,"MOVE CMD");
        *next = ST_EXE_CMD;
        break;
    case PDU_RESET:
        *next = ST_RESET;
        break;

    /* direct control */
    case PDU_GO_FORWARD:
        send_motor_cmd(MOVE_FORW);
        *next = ST_WAIT_CMD;
        break;
    case PDU_GO_REV:
        send_motor_cmd(MOVE_RVRS);
        *next = ST_WAIT_CMD;
        break;
    case PDU_TURN_LEFT:
        send_motor_cmd(MOTOR_TURN_LEFT);
        *next = ST_WAIT_CMD;
        break;
    case PDU_TURN_RIGHT:
        send_motor_cmd(MOTOR_TURN_RIGHT);
        *next = ST_WAIT_CMD;
        break;
    case PDU_STOP:
        send_motor_cmd(MOTOR_STOP);
        *next = ST_WAIT_CMD;
        break;
    default:
        Display_printf(display, 0, 0, "Garbage");
        break;
    }

    send_new_pos();

}

/* exe handler */
static void exe_handler(state_t *next){
    /*
     * note , robot will only move in either x or y direction
     * at a time
     */

    /* parse pdu data to figure out what to do */
    bool done = false;

    uint8_t new_x = rx_pdu.x; /* dest */
    uint8_t new_y = rx_pdu.y;

    uint8_t next_x;
    uint8_t next_y;

    float new_heading;

    do{
        if(blocked_flag){

            *next = ST_COL;
            return;
        }
        if(rx_pdu.cmd == PDU_MOVE){
            uint8_t curr_x;
            uint8_t curr_y;

            get_position(&curr_x, &curr_y);

//            Display_printf(display, 0, 0,"E X E \n");


            /* we've reached the correct location */
            if(curr_x == new_x && curr_y == new_y){
                done = true; /* done with moving */
            }

//            /* get next coord */
            calc_next_coord(new_x,new_y,&next_x,&next_y);

            /* get heading based on coord */
            new_heading = calc_heading(new_x,new_y);

            /* rotate robot to heading */
            rotate_to_heading(new_heading);

//            Display_printf(display, 0, 0,"M O V I N G\n");
            if(blocked_flag){
                *next = ST_COL;
                return;
            }

            /* move for hard-coded amount of time */
            send_motor_cmd(MOVE_FORW);
            Task_sleep(FORWARD_TIME_MS);
            send_motor_cmd(MOTOR_STOP);

            /* update current position */
            change_position(next_x,next_y);

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
    uint8_t data = (uint8_t)cmd;
    return uart_write(&data,1); /* send one byte for now */
}

static void send_heading(void){
    char msg[32];
    float heading = get_heading();
    sprintf(msg,"HEADING: %.1f",heading);
    send_msg(msg);
}

/* collision handler */
static void col_handler(state_t *next){

    /* while u_alert is in effect, stay here */
    while(read_u_alert()){
        send_motor_cmd(MOTOR_STOP);
        send_blocked_msg(); /* tell client we're blocked */
        turn_on_speaker();
        Task_sleep(1000);
        turn_off_speaker();
    }

    turn_off_speaker();
    *next = ST_EXE_CMD; /* try to resume previous command */
}

/* reset handler */
static void reset_handler(state_t *next){
    *next = ST_WAIT_CMD;
//    send_motor_cmd(MOTOR_STOP);
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

    float curr_heading = get_fake_heading();

//    while(curr_heading > new_heading + HEADING_ERROR_DEG ||
//          curr_heading < new_heading - HEADING_ERROR_DEG){

//        Display_printf(display, 0, 0, "CURR HEADING : %.1f",curr_heading);
//        Display_printf(display, 0, 0, "TAR HEADING : %.1f",new_heading);

        int res = (curr_heading - new_heading)/90;

        if (res == -3){ /* account for turning 90 deg left */
            res = -1;
        }

        int num_turns = abs(res);

        bool turn_left = false;
        if (res < 0){
            turn_left = true;
        }

        /* if heading to right ,rotate right */
        for(int i=0;i<num_turns;i++){ //just turn right regardless
//            Display_printf(display, 0, 0, "MOVING RIGHT");
            if(turn_left){
                send_motor_cmd(MOTOR_TURN_LEFT);
            }
            else{
                send_motor_cmd(MOTOR_TURN_RIGHT);
            }
            Task_sleep(ROTATE_TIME_MS);
            send_motor_cmd(MOTOR_STOP);
        }

        update_fake_heading(new_heading);
        curr_heading = new_heading;

        /* if heading left, rotate left */
//        if(curr_heading > new_heading + HEADING_ERROR_DEG){
//            Display_printf(display, 0, 0, "MOVING LEFT");
//            send_motor_cmd(MOTOR_TURN_RIGHT);
//            Task_sleep(ROTATE_TIME_MS);
//            send_motor_cmd(MOTOR_STOP);
//
//        }

//        curr_heading = get_heading();
//        send_heading();
//    }

//    Display_printf(display, 0, 0, "ROTATION DONE");

    return 0;
}

/* figure out heading in order to find next coord */
static float calc_heading(uint8_t new_x, uint8_t new_y){
    /* y axis is inverted from what you'd expect
     * positive values are south of 0
     * 0,0 is most "north" and most "west" point
     */

    float curr_heading = get_heading();
    uint8_t curr_x,curr_y;
    get_position(&curr_x,&curr_y);

    /* go x first */
    if(new_x > curr_x){
        return HEAD_S;
    }
    else if(new_x < curr_x){
        return HEAD_N;
    }
    /* y cases */
    else if(new_y > curr_y){
        return HEAD_E;
    }
    else if(new_y < curr_y){
        return HEAD_W;
    }
    else{ /* if new == curr coords then heading doesn't matter*/
        return -1;
    }
}

/*
 * figure out next coord
 * basic pathing algo, go to the next closest grid location
 * will move with new york style coords, no diagonal movements
 */
static int calc_next_coord(uint8_t new_x,
                            uint8_t new_y,
                            uint8_t *next_x,
                            uint8_t *next_y){

    uint8_t curr_x,curr_y;
    get_position(&curr_x,&curr_y);

    /* prioritize x first arbitrarily */
    /* if new_x == next_x, move in y direction */
    /* we can't move diagonally for now */
    if(curr_x < new_x){
        *next_x = curr_x+1;
        *next_y = curr_y;
        return 0;
    }
    else if(curr_x > new_x){
        *next_x = curr_x-1;
        *next_y = curr_y;
        return 0;
    }
    else if(curr_y < new_y){
        *next_x = curr_x;
        *next_y = curr_y+1;
        return 0;
    }
    else if(curr_y > new_y){
        *next_x = curr_x;
        *next_y = curr_y-1;
        return 0;
    }
    else{ /* already at location */
        return -1;
    }

}

float get_fake_heading(){
    return fake_heading;
}
void update_fake_heading(float heading){
    fake_heading = heading;
}
