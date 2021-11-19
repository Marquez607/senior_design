/*
 * Name: sensorHandler.c
 * Author: Marquez Jones
 * Desc: i2c sensor handling
 */

/* std lib */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* POSIX Header files */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <math.h>


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

#define PI 3.14159f

extern Display_Handle display;

static lsm303_t mag_sensor;

/* send some kind of message to the client */
static void send_msg(char *msg);

static void send_heading(void);

void *sensorThread(void *arg0)
{
    LSM303_struct_init(&mag_sensor);
    mag_sensor.i2c_read = i2c_read;
    mag_sensor.i2c_write = i2c_write;

    /* need to init sensor after adding function pointers */
    int rc = 0;
    rc = LSM303_sensor_init(&mag_sensor);
    if(rc < 0){
        Display_printf(display, 0, 0, "Error Initializing LM303\n");
        while(1);
    }
    LSM303_setMagRate(&mag_sensor,LSM303_MAGRATE_75);
    float mag_x = 0.0;
    float mag_y = 0.0;
    float mag_z = 0.0;

    uint8_t count = 0;
    while(1){

        LSM303_getOrientation(&mag_sensor,&mag_x, &mag_y, &mag_z);
        mag_x += 25;
        float heading = (atan2(mag_y,mag_x) * 180) / PI;
        if(heading < 0.0){
            heading += 360.0;
        }

        update_heading(heading);

        if(count < 40){
            count++;
        }else{
//            Display_printf(display, 0, 0, "Send Heading.\n");
            count = 0;
            send_heading(); /* send current heading every second */
        }

        Task_sleep(50); /* read at 20 hz */

    }
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

static void send_heading(void){
    char msg[32];
    float heading = get_heading();
    sprintf(msg,"HEADING: %.1f",heading);
    send_msg(msg);
}


