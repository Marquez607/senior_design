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

#define TASK_STACK_SIZE                       (2048)
#define DEFAULT_TASK_PRI                      (3)

#define NUM_CAM_BUFFERS 2
#define CAM_BUFFER_SIZE 20 * 1024
/* data structures that can be global */
extern uint8_t cam_data[NUM_CAM_BUFFERS][CAM_BUFFER_SIZE];
extern ppipc_buff_t cam_buffers[NUM_CAM_BUFFERS]; /* we'll use 2 camera buffers */
extern ppipc_buff_tab_t cam_buff_tab;

#endif /* SHARED_H_ */
