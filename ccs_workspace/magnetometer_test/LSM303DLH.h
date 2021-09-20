/******************************************************************************
  Name: LSM303DLH
  Author: Marquez Jones
  Date: 09/14/2021
  Desc: heavy modification of repo found at 
		https://github.com/sedgwickc/BBB_LSM303

  This version of the code was designed to be hardware agnostic where the user 
  inputs a function pointer to the i2c related functions; version is also written
  in pure C as opposed to C++

  This port maintains original license

  version deployed in TI CC3235S

  ******************************************************************************
  This is a Beaglebone Black/Green Rev. C C++ library for the LSM303DLHC sensor

  This code is based off of the Ardiuno driver written by Adafruit for their 
  10DOF breakout board. This code retains the license used by Adafruit's 
  original code. 

  Github: https://github.com/sedgwickc/RoverDaemons/

  Changlog

  Ver    Date       User   Issue #  Change
  --------------------------------------------------------------------------------
  100 25sep2015  cwick              Initial creation. 
  101 25sep2016  cwick     1        Add changelog. Add define for number of mag
                                    registers

  *****************************************************************************

  Designed specifically to work with the Adafruit LSM303 or LSM303 Breakout 
  ----> http://www.adafruit.com/products/391
  ----> http://www.adafruit.com/products/1603

  These displays use I2C to communicate, 2 pins are required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Kevin Townsend for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 *****************************************************************************/
#ifndef __LSM303_H__
#define __LSM303_H__

#include <stdint.h>
#include <stdbool.h>

/*=========================================================================
I2C ADDRESS/BITS
-----------------------------------------------------------------------*/
#define LSM303_ADDRESS_ACCEL          (0x19)         // 0011001x
#define LSM303_ADDRESS_MAG            (0x1E)         // 0011110x
/*=========================================================================*/

/*========================================================================*/
#define NUM_ACCEL_REG 6
#define NUM_MAG_REG 6

/*=========================================================================
Constants
-----------------------------------------------------------------------*/
#define SENSORS_GAUSS_TO_MICROTESLA (100) //Gauss to micro-Tesla multiplier
#define SENSORS_GRAVITY_EARTH       (9.80665F) // Earth's gravity in m/s^2 
#define SENSORS_GRAVITY_STANDARD	(SENSORS_GRAVITY_EARTH)
#define SCALE 						(2)
/*=========================================================================*/


/*=========================================================================
REGISTERS
-----------------------------------------------------------------------*/
typedef enum
{                                                     // DEFAULT    TYPE
  LSM303_REGISTER_ACCEL_CTRL_REG1_A         = 0x20,   // 00000111   rw
  LSM303_REGISTER_ACCEL_CTRL_REG2_A         = 0x21,   // 00000000   rw
  LSM303_REGISTER_ACCEL_CTRL_REG3_A         = 0x22,   // 00000000   rw
  LSM303_REGISTER_ACCEL_CTRL_REG4_A         = 0x23,   // 00000000   rw
  LSM303_REGISTER_ACCEL_CTRL_REG5_A         = 0x24,   // 00000000   rw
  LSM303_REGISTER_ACCEL_CTRL_REG6_A         = 0x25,   // 00000000   rw
  LSM303_REGISTER_ACCEL_REFERENCE_A         = 0x26,   // 00000000   r
  LSM303_REGISTER_ACCEL_STATUS_REG_A        = 0x27,   // 00000000   r
  LSM303_REGISTER_ACCEL_OUT_X_L_A           = 0x28,
  LSM303_REGISTER_ACCEL_OUT_X_H_A           = 0x29,
  LSM303_REGISTER_ACCEL_OUT_Y_L_A           = 0x2A,
  LSM303_REGISTER_ACCEL_OUT_Y_H_A           = 0x2B,
  LSM303_REGISTER_ACCEL_OUT_Z_L_A           = 0x2C,
  LSM303_REGISTER_ACCEL_OUT_Z_H_A           = 0x2D,
  LSM303_REGISTER_ACCEL_FIFO_CTRL_REG_A     = 0x2E,
  LSM303_REGISTER_ACCEL_FIFO_SRC_REG_A      = 0x2F,
  LSM303_REGISTER_ACCEL_INT1_CFG_A          = 0x30,
  LSM303_REGISTER_ACCEL_INT1_SOURCE_A       = 0x31,
  LSM303_REGISTER_ACCEL_INT1_THS_A          = 0x32,
  LSM303_REGISTER_ACCEL_INT1_DURATION_A     = 0x33,
  LSM303_REGISTER_ACCEL_INT2_CFG_A          = 0x34,
  LSM303_REGISTER_ACCEL_INT2_SOURCE_A       = 0x35,
  LSM303_REGISTER_ACCEL_INT2_THS_A          = 0x36,
  LSM303_REGISTER_ACCEL_INT2_DURATION_A     = 0x37,
  LSM303_REGISTER_ACCEL_CLICK_CFG_A         = 0x38,
  LSM303_REGISTER_ACCEL_CLICK_SRC_A         = 0x39,
  LSM303_REGISTER_ACCEL_CLICK_THS_A         = 0x3A,
  LSM303_REGISTER_ACCEL_TIME_LIMIT_A        = 0x3B,
  LSM303_REGISTER_ACCEL_TIME_LATENCY_A      = 0x3C,
  LSM303_REGISTER_ACCEL_TIME_WINDOW_A       = 0x3D
} lsm303AccelRegisters_t;

typedef enum
{
  LSM303_REGISTER_MAG_CRA_REG_M             = 0x00,
  LSM303_REGISTER_MAG_CRB_REG_M             = 0x01,
  LSM303_REGISTER_MAG_MR_REG_M              = 0x02,
  LSM303_REGISTER_MAG_OUT_X_H_M             = 0x03,
  LSM303_REGISTER_MAG_OUT_X_L_M             = 0x04,
  LSM303_REGISTER_MAG_OUT_Z_H_M             = 0x05,
  LSM303_REGISTER_MAG_OUT_Z_L_M             = 0x06,
  LSM303_REGISTER_MAG_OUT_Y_H_M             = 0x07,
  LSM303_REGISTER_MAG_OUT_Y_L_M             = 0x08,
  LSM303_REGISTER_MAG_SR_REG_Mg             = 0x09,
  LSM303_REGISTER_MAG_IRA_REG_M             = 0x0A,
  LSM303_REGISTER_MAG_IRB_REG_M             = 0x0B,
  LSM303_REGISTER_MAG_IRC_REG_M             = 0x0C,
  LSM303_REGISTER_MAG_TEMP_OUT_H_M          = 0x31,
  LSM303_REGISTER_MAG_TEMP_OUT_L_M          = 0x32
} lsm303MagRegisters_t;
/*=======================================================================*/

/*=======================================================================
MAGNETOMETER GAIN SETTINGS
-----------------------------------------------------------------------*/
typedef enum
{
  LSM303_MAGGAIN_1_3                        = 0x20,  // +/- 1.3
  LSM303_MAGGAIN_1_9                        = 0x40,  // +/- 1.9
  LSM303_MAGGAIN_2_5                        = 0x60,  // +/- 2.5
  LSM303_MAGGAIN_4_0                        = 0x80,  // +/- 4.0
  LSM303_MAGGAIN_4_7                        = 0xA0,  // +/- 4.7
  LSM303_MAGGAIN_5_6                        = 0xC0,  // +/- 5.6
  LSM303_MAGGAIN_8_1                        = 0xE0   // +/- 8.1
} lsm303MagGain;	
/*=======================================================================*/

/*=======================================================================
MAGNETOMETER UPDATE RATE SETTINGS
-----------------------------------------------------------------------*/
typedef enum
{
  LSM303_MAGRATE_0_7                        = 0x00,  // 0.75 Hz
  LSM303_MAGRATE_1_5                        = 0x01,  // 1.5 Hz
  LSM303_MAGRATE_3_0                        = 0x62,  // 3.0 Hz
  LSM303_MAGRATE_7_5                        = 0x03,  // 7.5 Hz
  LSM303_MAGRATE_15                         = 0x04,  // 15 Hz
  LSM303_MAGRATE_30                         = 0x05,  // 30 Hz
  LSM303_MAGRATE_75                         = 0x06,  // 75 Hz
  LSM303_MAGRATE_220                        = 0x07   // 200 Hz
} lsm303MagRate;	
/*=======================================================================*/

/*=======================================================================
INTERNAL MAGNETOMETER DATA TYPE
-----------------------------------------------------------------------*/
typedef struct lsm303MagData_s
{
    float x;
    float y;
    float z;
} lsm303MagData;
/*=======================================================================*/

/*=======================================================================
INTERNAL ACCELERATION DATA TYPE
-----------------------------------------------------------------------*/
typedef struct lsm303AccelData_s
{
  int16_t x;
  int16_t y;
  int16_t z;
} lsm303AccelData;
/*=======================================================================*/

typedef struct accelDataRaw_s
{
  int8_t xlo;
  int8_t xhi;
  int8_t ylo;
  int8_t yhi;
  int8_t zlo;
  int8_t zhi;
} accelDataRaw;
/*=========================================================================
CHIP ID-----------------------------------------------------------------------*/
#define LSM303_ID                     (0b11010100)
/*=========================================================================*/

typedef struct lsm303{
  lsm303AccelData accelData;   // Last read accelerometer data will be available here
  lsm303MagGain   magGain;
  lsm303MagData   magData;     // Last read magnetometer data will be available here
  bool autoRange;
  float _lsm303Accel_MG_LSB;
  float _lsm303Mag_Gauss_LSB_Z;
  float _lsm303Mag_Gauss_LSB_XY;
  uint8_t accel_addr; //on some lsm303s the accel and mag addresses will differ
  uint8_t mag_addr; 
  int (*i2c_write)(uint8_t slave_addr,uint8_t *buffer, uint32_t size);
  /*
    data read should be returned to the buffer
   */
  int (*i2c_read)(uint8_t slave_addr,uint8_t *buffer, uint32_t size);
}lsm303_t;

/****************************************************
 Initialize struct for sensor
 user will still need to provide function pointers

 user can change the i2c addresses afterwards if need
 be

 ****************************************************/
void LSM303_struct_init(lsm303_t *sensor);

/****************************************************
 Initializes sensor assuming valid functions for
 i2c_write and i2c_read pointers
 ****************************************************/
int LSM303_sensor_init(lsm303_t *sensor);

void LSM303_getOrientation(lsm303_t *sensor,float*x,float*y,float*z);
void LSM303_getAcceleration(lsm303_t *sensor,float*x,float*y,float*z);

/****************************************************
 * write command to LSM303
 ****************************************************/
void LSM303_writeCommand(lsm303_t *sensor,unsigned int address, unsigned int reg, unsigned char value);

uint8_t LSM303_read8(lsm303_t *sensor,unsigned int address, unsigned int reg);
uint16_t LSM303_combineRegisters(unsigned char, unsigned char);

void LSM303_readAccelerometer(lsm303_t *sensor);
void LSM303_readMagnetometerData(lsm303_t *sensor);

void LSM303_setMagRate(lsm303_t *sensor,lsm303MagRate rate);
void LSM303_setMagGain(lsm303_t *sensor,lsm303MagGain gain);

#endif
