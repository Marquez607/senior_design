/*
 * platform.h
 *
 *  Created on: Oct 7, 2021
 *      Author: Marquez
 */

#ifndef PLATFORM_H_
#define PLATFORM_H_

#define NUM_CAM_BUFFERS 2
/* data structures that can be global */
extern uint8_t cam_data[NUM_CAM_BUFFERS][2048];
extern ppipc_buff_t cam_buffers[NUM_CAM_BUFFERS]; /* we'll use 2 camera buffers */
extern ppipc_buff_tab_t cam_buff_tab;

#endif /* PLATFORM_H_ */
