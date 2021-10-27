/*
 * Name: pingpong.c
 * Author: Marquez Jones
 * Desc: ping pong buffering system that allows our camera and tcp to both
 * operate simultaneously with less blocking
 *
 * TCP will iterate through buffers and transmit data available while camera populates
 * other buffers
 *
 * DOES NOT WORK WITH MULTIPLE READERS/WRITERS
 * NOT CURRENTLY CONCERNED WITH THAT FEATURE
 * ADDING A MUTEX IN THE BUFFEF STRUCT MAY ALLOW IT TO WORK WITH
 * MULTIPLE READER/WRITERS */

#include "pingpong_ipc.h"
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>

/******************************* func def start ***********************************/

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
                     uint32_t size){
    buffer->data = data;
    buffer->size = size;
}

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
                    uint32_t num_buffs){

    table->buffers = buffers;
    table->num_buffs = num_buffs;

    /* sem inits */
    sem_init(&table->empty, 0, 0); /* no state changes occured */
    sem_init(&table->full, 0, 0); /* no state changes occured */
    sem_init(&table->mutex, 0, 1); /* unlocked at start */
}

/*
 * Desc: returns pointer to available buffer struct
 * don't update avail until data actually retrieved/written
 * params
 * table -> pointer to table
 * RnW -> 0 block on writing, 1 block on reading
 *
 * return buffer pointer
 */
ppipc_buff_t *ppipc_get_buffer(ppipc_buff_tab_t *table, bool RnW){

    ppipc_buff_t *ret = NULL;
    sem_wait(&table->mutex);
    if(RnW){ /* if reading */
        while(table->avail == 0){
            sem_post(&table->mutex); /* release */
            sem_wait(&table->empty); /* block on empty */
            sem_wait(&table->mutex);
        }
        ret = &table->buffers[table->tail];
    }
    else{ /* if writing */
        while(table->avail == table->num_buffs){
            sem_post(&table->mutex); /* release */
            sem_wait(&table->full); /* block on full */
            sem_wait(&table->mutex);
        }
        ret = &table->buffers[table->head];
    }
    sem_post(&table->mutex);
    return ret;
}

/*
 * Desc: update data structure with new avail value/post appropriate sems
 * because the data should have been written or read, we update avail here
 * params
 * table -> table pointer
 * RnW ->
 *      0 post threads waiting to read, inc avail (you were writing)
 *      1 post threads waiting to write, dec avail (you were reading)
 */
void ppipc_update_buffer(ppipc_buff_tab_t *table, bool RnW){

    sem_wait(&table->mutex);
    if(RnW){ /* if done reading */
        table->avail--;
        table->tail++;
        table->tail %= table->num_buffs;

        /*post thread waiting on full buffer only if one spot left */
        if(table->avail == table->num_buffs-1){
            sem_post(&table->full);
        }
    }
    else{ /* if done writing */
        table->avail++;
        table->head++;
        table->head %= table->num_buffs;

        /* post threads waiting only if one avail */
        if(table->avail == 1){
            sem_post(&table->empty);
        }

    }
    sem_post(&table->mutex);
}

/******************************* func def end *************************************/
