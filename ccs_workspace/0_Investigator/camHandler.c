/*
 * Name: camHandler
 * Author: Marquez Jones
 * Desc: camera reading and interface with camera tcp
 *       will read data from the camera and output to wifi
 */

/* application headers */
#include "sensor_drivers/ArduCAM_Generic.h"
#include "board.h"

/* std lib */
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

/* Driver configuration */
#include "ti_drivers_config.h"

/* investigator includes */
#include "shared.h"
#include "pingpong_ipc.h"

extern Display_Handle display;

void camCapture(ArduCam_t *cam);

 /*  ======== mainThread ========
 */
void *camThread(void *arg0)
{

    ArduCam_t acam;
    acam.spi_write = acam_spi_write;
    acam.spi_read = acam_spi_read;
    acam.spi_cs_low = acam_spi_cs_low;
    acam.spi_cs_high = acam_spi_cs_high;
    acam.i2c_write = i2c_write;
    acam.i2c_read = i2c_read;
    acam.delay_ms = acam_delay_ms;

    ACAM_Init(&acam);
    ACAM_OV2640_set_JPEG_size(&acam,OV2640_320x240);

//    Display_printf(display,0,0,"Starting ArduCam Test\n");
    ACAM_clear_fifo_flag(&acam);

    while(1){
        ACAM_clear_fifo_flag(&acam);
        ACAM_start_capture(&acam);

        /* wait for a capture to finish */
        while(!ACAM_get_bit(&acam,ARDUCHIP_TRIG, CAP_DONE_MASK));
//        Display_printf(display,0,0,"Capture Finished");
        camCapture(&acam);
        Task_sleep(1);
    }

}

/*
 * read and send full image from camera
 * roughly based on esp32 camCapture example
 *
 * uses single fifo read so is technically
 * half the possible bandwidth
 *
 */
void camCapture(ArduCam_t *cam){

    uint32_t len = ACAM_get_fifo_length(cam);
    uint8_t temp=0,temp_last=0;
    bool is_header = false;
    int i = 0;

    /* IPC */
    ppipc_buff_t *cam_buffer = ppipc_get_buffer(&cam_buff_tab,PPIPC_WRITE);

    while(len--){
        temp_last = temp;
        temp = ACAM_read_fifo(cam);

        /* check for end of frame */
        if ( (temp==0xD9) && (temp_last==0xFF) ){
            cam_buffer->data[i++] = temp;
            is_header = false;
            i=0;
        }
        /* check if header already passed (body of image) */
        if (is_header==true){
            if( i < cam_buffer->size ){ /* wait for buffer to fill */
                cam_buffer->data[i++] = temp; /* add to buff */
            }
            else{
                /* dump data to ipc if tcp still up */
                ppipc_update_buffer(&cam_buff_tab,PPIPC_WRITE); /* flush to tcp */
                cam_buffer = ppipc_get_buffer(&cam_buff_tab,PPIPC_WRITE); /* get next buffer */
                i=0;
                cam_buffer->data[i++] = temp;
                ACAM_set_fifo_burst(cam);
            }
        }
        /* look for header */
        else if ((temp==0xD8) && (temp_last==0xFF)){
            is_header = true;
            cam_buffer->data[i++] = temp_last;
            cam_buffer->data[i++] = temp;
        }
    }
    ppipc_update_buffer(&cam_buff_tab,PPIPC_WRITE); /* flush buffer to tcp */
}


