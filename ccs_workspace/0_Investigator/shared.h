/*
 * Name: shared.c
 * Author: Marquez Jones
 * Desc: any data that needs to be shared across multiple threads will go here
 *       proper ipc will be used
 */

#ifndef SHARED_H_
#define SHARED_H_

#include "pingpong_ipc.h"
#include <stdint.h>
#include <semaphore.h>

#define TASK_STACK_SIZE                       (2048)
#define DEFAULT_TASK_PRI                      (3)
#define NUM_CAM_BUFFERS                       (2)
#define CAM_BUFFER_SIZE                       (10 * 1024)

/************************** WHEELSON LOCATION **********************************/

/* approximate bearings */
/* the robot compass is nowhere near this accurate
 * will need to allow generous tolerances like +/- 30 deg
 *
 * or hand calibrate directions
 *
 * make sure robot is aligned to either N,S,E,W
 * 0,0 is most "north" and most "west" point
 */
/* NOTE: polar mapping */
extern uint16_t HEAD_N;
extern uint16_t HEAD_S;
extern uint16_t HEAD_E;
extern uint16_t HEAD_W;

extern uint16_t FORWARD_TIME_MS;
extern uint16_t ROTATE_TIME_MS;
extern uint16_t HEADING_ERROR_DEG;

//#define ROTATE_TIME_MS 250 /*time between sending a rotate command */
//#define FORWARD_TIME_MS 2000/* amount of time that counts as one dist unit */
//#define HEADING_ERROR_DEG 5 /* allowed heading error when rotating */


/* " compass " bearing */
typedef struct wheelson_heading{
    float heading;
    sem_t mutex;
}wheelson_heading_t;

/* which way wheelson is facing */
extern wheelson_heading_t heading_g;

/*
 * initialize the heading semaphore
 */
void init_heading(void);

/*
 * change current heading
 */
void update_heading(float heading);

/*
 * get current heading
 */
float get_heading(void);

/* current position */
/* NOTE: this is how wheelson knows where it is */
/* this should be routinely synced with client software */
typedef struct wheelson_pos{
    sem_t mutex;
    uint8_t x;
    uint8_t y;
}wheelson_pos_t;

extern wheelson_pos_t position_g;

/* only needs to get called on boot up or whenever client decides
 * to reset relative coordinate system
 */
void init_position(wheelson_pos_t *pos,uint8_t x, uint8_t y);

/*
 * get current global position variable
 */
void get_position(uint8_t *x, uint8_t *y);

/*
 * update current global position variable
 */
void change_position(uint8_t x, uint8_t y);

/*********************************** MESSAGE SYSTEM ********************************/

#define PDU_FIFO_SIZE 64
#define PDU_SIZE sizeof(pdu_t)
#define PDU_START_BYTE0 0x5E
#define PDU_START_BYTE1 0xFF

/* PDU Codes */
/* wanted to ensure byte sized commands */
//typedef uint8_t pdu_cmd_t;
//extern const pdu_cmd_t C_MOVE;  /* can be queued up */
//extern const pdu_cmd_t C_STOP;  /* should be heeded immediately */
//extern const pdu_cmd_t C_RESET; /* reset orientation and stop , can also change position here*/
//extern const pdu_cmd_t C_BLOCK; /* robot has encountered obstacle */
//extern const pdu_cmd_t C_UPDATE;/* robot sending update to client */

typedef enum pdu_cmd{
    PDU_MOVE,
    PDU_STOP,
    PDU_RESET,
    PDU_BLOCK,
    PDU_UPDATE,
    PDU_CHANGE_HEAD, /* change heading configurations in msg portion */
    PDU_GO_FORWARD,
    PDU_GO_REV,
    PDU_TURN_LEFT,
    PDU_TURN_RIGHT
}pdu_cmd_t;

/* PDU */
/* protocol data unit for exchanging messages between robot and client */
typedef struct pdu{
    pdu_cmd_t cmd;
    uint8_t x;
    uint8_t y;
    uint8_t msg_len; /* any additional data */
    char msg[32];       /* C string */
}pdu_t;

/* circular buffer */
typedef struct pdu_fifo{
    pdu_t *buffer; /* buffer array */
    uint32_t size;
    uint32_t avail; /* avail buffers to read */
    uint32_t tail; /* next readable*/
    uint32_t head; /*next writeable */
    sem_t full; /* posts when not full */
    sem_t empty; /* posts when not empty */
    sem_t mutex; /* table lock */
}pdu_fifo_t;

extern pdu_t rx_pdu_buffer_g[PDU_FIFO_SIZE];
extern pdu_t tx_pdu_buffer_g[PDU_FIFO_SIZE];
extern pdu_fifo_t tx_pdu_fifo_g; /* for transmit */
extern pdu_fifo_t rx_pdu_fifo_g; /* for receive */

/* initialize ipc at start */
/* uses available global buffer */
void pdu_fifo_init(pdu_fifo_t *fifo,pdu_t *buffer,uint32_t size);

/*
 * copies user input pdu to fifo
 * returns -1 on failure, 0 on success
 */
int pdu_fifo_put(pdu_fifo_t *fifo,pdu_t *input);

/*
 * copies pdu data to user out pointer
 * returns -1 on failure, 0 on success
 */
int pdu_fifo_get(pdu_fifo_t *fifo,pdu_t *out);

/*
 * fills buffer with pdu
 */
int pdu_fill_buffer(uint8_t *out_buff,pdu_t *in_pdu);

/*
 * populates pdu with buffer data
 */
int pdu_read_buffer(uint8_t *in_buff,pdu_t *out_pdu);

/********************************** OTHER IPC DATA **********************************/

/* data structures that can be global */
extern uint8_t cam_data[NUM_CAM_BUFFERS][CAM_BUFFER_SIZE];
extern ppipc_buff_t cam_buffers[NUM_CAM_BUFFERS]; /* we'll use 2 camera buffers */
extern ppipc_buff_tab_t cam_buff_tab;



#endif /* SHARED_H_ */
