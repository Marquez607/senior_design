/*
 * Name: shared.c
 * Desc: any data that needs to be shared across multiple threads will go here
 *       proper ipc will be used
 */
#include "shared.h"
#include "pingpong_ipc.h"
#include <stdint.h>
#include <semaphore.h>
#include <string.h>

#include <ti/display/Display.h>

extern Display_Handle display;

/************************** WHEELSON LOCATION **********************************/


/* where wheelson is currently in our "world" */
wheelson_pos_t position_g;

/* " compass " bearing */
uint32_t heading_g;

/* maybe we'll need to set a relative "north" for ease of use */
uint32_t heading_offset_g;

/* only needs to get called on boot up or whenever client decides
 * to reset relative coordinate system
 */
void init_position(wheelson_pos_t *pos,uint8_t x, uint8_t y){
    pos->x = x;
    pos->y = y;
    sem_init(&pos->mutex, 0, 1); /* unlocked at start */
}

/*
 * get current global position variable
 */
void get_position(uint8_t *x, uint8_t *y){
    sem_wait(&position_g.mutex);
    *x = position_g.x;
    *y = position_g.y;
    sem_post(&position_g.mutex);
}

/*
 * update current global position variable
 */
void change_position(uint8_t x, uint8_t y){
    sem_wait(&position_g.mutex);
    position_g.x = x;
    position_g.y = y;
    sem_post(&position_g.mutex);
}


/*********************************** MESSAGE SYSTEM ********************************/

/* const variables */
const pdu_cmd_t MOVE = 0;  /* can be queued up */
const pdu_cmd_t STOP = 1;  /* should be heeded immediately */
const pdu_cmd_t RESET = 2; /* reset orientation and stop */
const pdu_cmd_t BLOCK = 3; /* robot has encountered obstacle */
const pdu_cmd_t UPDATE = 4;/* robot sending update to client */

pdu_t rx_pdu_buffer_g[PDU_FIFO_SIZE];
pdu_t tx_pdu_buffer_g[PDU_FIFO_SIZE];
pdu_fifo_t tx_pdu_fifo_g; /* for transmit */
pdu_fifo_t rx_pdu_fifo_g; /* for receive */

/* initialize ipc at start */
/* uses available global buffer */
void pdu_fifo_init(pdu_fifo_t *fifo,pdu_t *buffer,uint32_t size){
    fifo->buffer = buffer;
    fifo->size = size;
    fifo->avail = 0;
    fifo->tail = 0;
    fifo->head = 0;

    sem_init(&fifo->empty, 0, 0); /* no state changes occured */
    sem_init(&fifo->full, 0, 0); /* no state changes occured */
    sem_init(&fifo->mutex, 0, 1); /* unlocked at start */
}

/*
 * copies user input pdu to fifo
 * blocks if full
 * returns -1 on failure, 0 on success
 */
int pdu_fifo_put(pdu_fifo_t *fifo,pdu_t *input){

    sem_wait(&fifo->mutex);
    while(fifo->avail == fifo->size){
        sem_post(&fifo->mutex);
        sem_wait(&fifo->full);
        sem_wait(&fifo->mutex);
    }

    /* copy operation */
    memcpy(&fifo->buffer[fifo->head],input,sizeof(pdu_t));
    fifo->head++;
    fifo->head%=fifo->size;
    fifo->avail++;

    /* post threads blocked on reading */
    if(fifo->avail == 1){
        sem_post(&fifo->empty);
    }
    sem_post(&fifo->mutex);

    return 0;
}

/*
 * copies pdu data to user out pointer
 * blocks if empty
 * returns -1 on failure, 0 on success
 */
int pdu_fifo_get(pdu_fifo_t *fifo,pdu_t *out){

    sem_wait(&fifo->mutex);
    while(fifo->avail < 1){
        sem_post(&fifo->mutex);
        sem_wait(&fifo->empty);
        sem_wait(&fifo->mutex);
    }

    /* copy operation */
    memcpy(out,&fifo->buffer[fifo->tail],sizeof(pdu_t));
    fifo->tail++;
    fifo->tail%=fifo->size;
    fifo->avail--;

    /* post threads blocked on writing */
    if(fifo->avail == fifo->size-1){
        sem_post(&fifo->full);
    }
    sem_post(&fifo->mutex);

    return 0;
}


/*
 * fills buffer with pdu
 */
int pdu_fill_buffer(uint8_t *out_buff,pdu_t *in_pdu){

    out_buff[0] = PDU_START_BYTE0;
    out_buff[1] = PDU_START_BYTE1;
    memcpy(out_buff+2,in_pdu,PDU_SIZE);
    return 0;
}

/*
 * populates pdu with buffer data
 */
int pdu_read_buffer(uint8_t *in_buff,pdu_t *out_pdu){

    if(in_buff[0] != PDU_START_BYTE0 || in_buff[1] != PDU_START_BYTE1){
        Display_printf(display, 0, 0, "BAD PDU");
        return -1;
    }
    memcpy((uint8_t*)out_pdu,in_buff+2,PDU_SIZE);
    return 0;
}

/********************************** OTHER IPC DATA **********************************/

/* data structures that can be global */
uint8_t cam_data[NUM_CAM_BUFFERS][CAM_BUFFER_SIZE];
ppipc_buff_t cam_buffers[NUM_CAM_BUFFERS]; /* we'll use 2 camera buffers */
ppipc_buff_tab_t cam_buff_tab;

