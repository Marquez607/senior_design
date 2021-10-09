/*
 * Name: shared.c
 * Desc: any data that needs to be shared across multiple threads will go here
 *       proper ipc will be used
 */
#include "shared.h"
#include "pingpong_ipc.h"
#include <stdint.h>

/* data structures that can be global */
uint8_t cam_data[NUM_CAM_BUFFERS][CAM_BUFFER_SIZE];
ppipc_buff_t cam_buffers[NUM_CAM_BUFFERS]; /* we'll use 2 camera buffers */
ppipc_buff_tab_t cam_buff_tab;
