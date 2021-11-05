/*
 * i2c test code
 */

#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/display/Display.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#include "./sensor_drivers/LSM303DLH.h"
#define TASKSTACKSIZE       640
#define MAG_ADDR   0x1E
#define ACCEL_ADDR 0x19
#define MSP_ADDR 0x48
#define PI 3.14159f

static Display_Handle display;

static lsm303_t mag_sensor;
static int i2c_write(uint8_t slave_addr, uint8_t *buffer, uint32_t size);
static int i2c_read(uint8_t slave_addr, uint8_t *buffer, uint32_t size);

static void bus_sniff(void);

static void print_data(float x, float y, float z, float t);

static I2C_Handle      i2c;
static I2C_Params      i2cParams;
static I2C_Transaction i2cTransaction;

static int lcd_reset(void);
static int lcd_write(uint8_t data);
static int lcd_string(char *str);

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{

    /* Call driver init functions */
    Display_init();
//    GPIO_init();
    I2C_init();

    /* Open the UART display for output */
    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL) {
        while (1);
    }

    /* Create I2C for usage */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(CONFIG_I2C_TMP, &i2cParams);
    if (i2c == NULL) {
        Display_printf(display, 0, 0, "Error Initializing I2C\n");
        while (1);
    }
    else {
        Display_printf(display, 0, 0, "I2C Initialized!\n");
    }

    /* init mag sensor params */
    LSM303_struct_init(&mag_sensor);
//    mag_sensor.accel_addr = LSM303_ADDRESS_MAG; //test done on lsm303dlhc which has one i2c line
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


    char str[20];
    /* lcd reset */
    str[0] = 0xFF;
    i2c_write(MSP_ADDR,str,1);
    /* write string to lcd */
    char *pstr = str;
    sprintf(str,"MAG TEST");
    lcd_string(str);


    while(1){

        LSM303_getOrientation(&mag_sensor,&mag_x, &mag_y, &mag_z);
        mag_x += 25.0;
        float heading = (atan2(mag_y,mag_x) * 180) / PI;
        if(heading < 0.0){
            heading += 360.0;
        }

        print_data(mag_x,mag_y,0.0,0.0);

        Display_printf(display,0,0,"Head: %.1f",heading);

        lcd_reset();
        lcd_string(str);
        Task_sleep(500);
    }

//    I2C_close(i2c);
//    Display_printf(display, 0, 0, "I2C closed!");

    return (NULL);
}

static void bus_sniff(void){
    uint8_t i = 0;
    uint8_t test = 0x55;
    int ret = 0;
    while(i<128){
        ret = i2c_write(i,&test,1);
        if(ret < 0){
            Display_printf(display, 0, 0, "NACK : %x",i);
        }
        else{
            Display_printf(display, 0, 0, " ACK : %x",i);
        }
        i++;
    }
}

static int i2c_write(uint8_t slave_addr, uint8_t *buffer, uint32_t size){
    i2cTransaction.writeBuf     = buffer;
    i2cTransaction.writeCount   = size;
    i2cTransaction.readCount    = 0;
    i2cTransaction.slaveAddress = slave_addr;
    if( I2C_transfer(i2c, &i2cTransaction) == false ){
        Display_printf(display,0,0,"Slave NACK");
        return -1;
    }
    return 0;
}

static int i2c_read(uint8_t slave_addr, uint8_t *buffer, uint32_t size){
    i2cTransaction.writeCount   = 0;
    i2cTransaction.readBuf      = buffer;
    i2cTransaction.readCount    = size;
    i2cTransaction.slaveAddress = slave_addr;
    if( I2C_transfer(i2c, &i2cTransaction) == false ){
        Display_printf(display,0,0,"Slave NACK");
        return -1;
    }
    return 0;
}

static void print_data(float x, float y, float z, float t){
    Display_printf(display, 0, 0, "x: %.1f y: %.1f z: %.1f t: %.1f",x,y,z,t);
}
static int lcd_reset(void){
    return lcd_write(0xFF);
}
static int lcd_write(uint8_t data){
    uint8_t buff = data;
    int rc = i2c_write(MSP_ADDR,&buff,1);
    return rc;
}
static int lcd_string(char *str){

    int rc = 0;
    while(*str){
        rc = lcd_write(*str);
        if(rc < 0){
            return rc;
        }
        str++;
    }
    return rc;
}

