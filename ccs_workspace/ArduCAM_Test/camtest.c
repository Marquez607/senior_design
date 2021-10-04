/*

*/

/* application headers */
#include <ArduCAM_Generic.h>
#include <board.h>

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

#define TASKSTACKSIZE       1024
#define CAM_BUFF_SIZE 2048

uint8_t cam_buffer[CAM_BUFF_SIZE]; //we'll grab 4k at a time

int camCapture(ArduCam_t *cam);

 /*  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    board_init();

    ArduCam_t acam;
    acam.spi_write = acam_spi_write;
    acam.spi_read = acam_spi_read;
    acam.spi_cs_low = acam_spi_cs_low;
    acam.spi_cs_high = acam_spi_cs_high;
    acam.i2c_write = acam_i2c_write;
    acam.i2c_read = acam_i2c_read;
    acam.delay_ms = acam_delay_ms;

    ACAM_Init(&acam);
    ACAM_OV2640_set_JPEG_size(&acam,OV2640_320x240);

    Display_printf(display,0,0,"Starting ArduCam Test\n");
    ACAM_clear_fifo_flag(&acam);



    uint32_t avail = 0;
    while(1){
        ACAM_clear_fifo_flag(&acam);
        ACAM_start_capture(&acam);

        /* wait for a capture to finish */
        while(!ACAM_get_bit(&acam,ARDUCHIP_TRIG, CAP_DONE_MASK));
        Display_printf(display,0,0,"Capture Finished");
        avail = ACAM_get_fifo_length(&acam);
        camCapture(&acam);
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
int camCapture(ArduCam_t *cam){

    /* TODO: chuck data to TCP ipc */

    uint32_t len = ACAM_get_fifo_length(cam);
    uint8_t temp=0,temp_last=0;
    bool is_header = false;
    int i = 0;
    while(len--){
        temp_last = temp;
        temp = ACAM_read_fifo(cam);

        /* check for end of frame */
        if ( (temp==0xD9) && (temp_last==0xFF) ){
            cam_buffer[i++] = temp;
            /* dump data to ipc if tcp still up */

            is_header = false;
            i=0;
        }
        /* check if header already passed (body of image) */
        if (is_header==true){
            if( i < CAM_BUFF_SIZE ){ /* wait for buffer to fill */
                cam_buffer[i++] = temp; /* add to buff */
            }
            else{
                /* dump data to ipc if tcp still up */

                i=0;
                cam_buffer[i++] = temp;
//                cam->spi_cs_low();
                ACAM_set_fifo_burst(cam);
            }
        }
        /* look for header */
        else if ((temp==0xD8) && (temp_last==0xFF)){
            is_header = true;
            cam_buffer[i++] = temp_last;
            cam_buffer[i++] = temp;
        }
    }
}


