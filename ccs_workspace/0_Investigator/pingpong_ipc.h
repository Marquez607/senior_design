/*
 * Name: pingpong.h
 * Author: Marquez Jones
 * Desc: ping pong buffering system that allows our camera and tcp to both
 * operate simultaneously with less blocking
 *
 * TCP will iterate through buffers and transmit data available while camera populates
 * other buffers
 *
 * DOES NOT WORK WITH MULTIPLE READERS/WRITERS
 * NOT CURRENTLY CONCERNED WITH THAT FEATURE
 */

#ifndef PINGPONG_IPC_H_
#define PINGPONG_IPC_H_

#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

/******************************* structs start ***********************************/

typedef struct ppipc_buff{
    uint32_t size; /* size of each buffer */
    uint8_t *data;
}ppipc_buff_t;

/* use avail_rd and avail_wr to figure out which buffer to use */
typedef struct ppipc_buff_tab{
    ppipc_buff_t *buffers; /* buffer array */
    uint32_t num_buffs; /* number of buffers */
    uint32_t avail; /* avail buffers to read */
    uint32_t tail; /* next readable*/
    uint32_t head; /*next writeable */
    sem_t full; /* posts when not full */
    sem_t empty; /* posts when not empty */
    sem_t mutex; /* table lock */
}ppipc_buff_tab_t;

typedef enum ppipc_RnW{
    PPIPC_READ = true,
    PPIPC_WRITE = false
}ppipc_RnW_t;

/******************************* structs end *************************************/

/******************************* func proto start ***********************************/

/*
 * Desc: initialize buffer table
 *
 * Params:
 * buffer -> buffer struct
 * data -> data array
 * buff_size -> size of the buffer
 */
void ppipc_init_buff(ppipc_buff_t *buffer,
                     uint8_t *data,
                     uint32_t size);

/*
 * Desc: initialize buffer table
 *
 * Params:
 * table -> pointer to table struct
 * buffers -> array of buffer objects
 * num_buffs -> number of buffers
 * buff_size -> size of the buffers
 */
void ppipc_init_tab(ppipc_buff_tab_t *table,
                    ppipc_buff_t *buffers,
                    uint32_t num_buffs);

/*
 * Desc: returns pointer to available buffer struct
 * don't update avail until data actually retrieved/written
 * params
 * table -> pointer to table
 * RnW -> 0 block on writing, 1 block on reading
 *
 * return buffer pointer
 */
ppipc_buff_t *ppipc_get_buffer(ppipc_buff_tab_t *table,bool RnW);

/*
 * Desc: update data structure with new avail value/post appropriate sems
 * params
 * table -> table pointer
 * RnW ->
 *      0 post threads waiting to read, inc avail
 *      1 post threads waiting to write, dec avail
 *
 */
void ppipc_update_buffer(ppipc_buff_tab_t *table, bool RnW);

/******************************* func proto end *************************************/

#endif /* PINGPONG_IPC_H_ */
