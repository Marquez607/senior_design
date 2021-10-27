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

  This library is a port of Adafruit's LSM303 library for Arduino to the Beagle
  Bone Black using the MRAA library written by Intel to handle I2C
  communication.

  This port is written and maintained by Charles Sedgwick. 
  This port retains the licence of the software it is based off of which is
  described below.

  Github: https://github.com/sedgwickc/RoverDaemons/

  Changlog

  Ver    Date       User   Issue #  Change
  -----------------------------------------------------------------------------
  100 25sep2015  cwick              Initial creation. 
  101 25sep2016  cwick     1        Add changelog. Add define for number of mag
                                    registers. Retrieve data from magnetometer
                                    registers. 
 !!!!!!!!!!!!!!!!!!!!!!!!!!!UPDATE VERSION VARIABLE!!!!!!!!!!!!!!!!!!!!!!!!!!!!

 ******************************************************************************
  This is a library for the LSM303 pressure sensor

  Designed specifically to work with the Adafruit LSM303 or LSM303 Breakout 
  ----> http://www.adafruit.com/products/391
  ----> http://www.adafruit.com/products/1603
 
  These displays use I2C to communicate, 2 pins are required to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Kevin Townsend for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include "LSM303DLH.h"
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LSM303_USE_DATASHEET_VALS (0) /* Set to 1 for sanity check */

/**
 * Method to combine two 8-bit registers into a single short, which is 16-bits on the BBB. It shifts
 * the MSB 8-bits to the left and then ORs the result with the LSB.
 * @param msb an unsigned character that contains the most significant uint8_t
 * @param lsb an unsigned character that contains the least significant uint8_t
 */
uint16_t LSM303_combineRegisters(unsigned char msb, unsigned char lsb){
   //shift the MSB left by 8 bits and OR with LSB
   return ((short)msb<<8)|(short)lsb;
}
 
/****************************************************
 Initialize struct for sensor
 user will still need to provide function pointers

 user can change the i2c addresses afterwards if need
 be

 ****************************************************/
void LSM303_struct_init(lsm303_t *sensor){

	sensor->mag_addr = LSM303_ADDRESS_MAG;
	sensor->accel_addr = LSM303_ADDRESS_ACCEL;
    sensor->magGain = LSM303_MAGGAIN_1_3;
    sensor->autoRange = false;

    /* clear data parameters */
    memset(sensor->accelData,0,sizeof(lsm303AccelData));
    memset(sensor->magData,0,sizeof(lsm303MagData));

	sensor->_lsm303Accel_MG_LSB     = 0.001F;   // 1, 2, 4 or 12 mg per lsb
	sensor->_lsm303Mag_Gauss_LSB_XY = 1100.0F;  // Varies with gain
	sensor->_lsm303Mag_Gauss_LSB_Z  = 980.0F;   // Varies with gain

}

/****************************************************
 Initializes sensor assuming valid functions for
 i2c_write and i2c_read pointers
 ****************************************************/
int LSM303_sensor_init(lsm303_t *sensor){
	// Enable the accelerometer (100Hz)
	LSM303_writeCommand(sensor,sensor->accel_addr, LSM303_REGISTER_ACCEL_CTRL_REG1_A, 0x57);
	// FS = 00 (+/- 2g ful scale)
	LSM303_writeCommand(sensor,sensor->accel_addr, LSM303_REGISTER_ACCEL_CTRL_REG4_A, 0x08);

//	// LSM303DLHC has no WHOAMI register so read CTRL_REG1_A back to check
//	// if we are connected or not
//	//Consider chaning this as the MSB may change based on select read rate
//	// if MSBs = 0x5 then data rate is 100Hz
//	uint8_t reg1_a = LSM303_read8(sensor,sensor->accel_addr, LSM303_REGISTER_ACCEL_CTRL_REG1_A);
//	if (reg1_a != 0x57)
//	{
//		return -1;
//	}

	// Enable the magnetometer
	LSM303_writeCommand(sensor,sensor->mag_addr, LSM303_REGISTER_MAG_MR_REG_M, 0x00);

//	// LSM303DLHC has no WHOAMI register so read CRA_REG_M to check
//	// the default value (0b00010000/0x10)
//	uint8_t reg1_m = LSM303_read8(sensor,sensor->mag_addr, LSM303_REGISTER_MAG_CRA_REG_M);
//	if (reg1_m != 0x10)
//	{
//	return -1;
//	}

	// Set the gain to a known level
	LSM303_setMagRate(sensor,LSM303_MAGGAIN_1_3);

	return 0;
}

 /*****************************************************************************
 @brief Reads accelerometer
 *****************************************************************************/
 void LSM303_readAccelerometer(lsm303_t *sensor){
	uint8_t accelDataRaw[NUM_ACCEL_REG];

	uint8_t write_buff[1] = {LSM303_REGISTER_ACCEL_OUT_X_L_A | 0x80};
	sensor->i2c_write(sensor->accel_addr,write_buff,1);
     
 	/* read 6 accel data registers on LSM303 into array */
 	int ret = sensor->i2c_read(sensor->accel_addr,accelDataRaw,6);

	// Shift values to create properly formed integer (low uint8_t first)
	// Shift result by 4 since accellerometer data is only return in 12 MSB
	sensor->accelData.x = (int16_t)(accelDataRaw[0] | (accelDataRaw[1] << 8)) >> 4;
	sensor->accelData.y = (int16_t)(accelDataRaw[2] | (accelDataRaw[3] << 8)) >> 4;
	sensor->accelData.z = (int16_t)(accelDataRaw[4] | (accelDataRaw[5] << 8)) >> 4;
}

void LSM303_getAcceleration(lsm303_t *sensor,float *x, float *y, float *z){

	LSM303_readAccelerometer(sensor);
	// sets x,y,z to values in metres/second
	*x = sensor->accelData.x * sensor->_lsm303Accel_MG_LSB * SENSORS_GRAVITY_STANDARD;
	*y = sensor->accelData.y * sensor->_lsm303Accel_MG_LSB * SENSORS_GRAVITY_STANDARD;
	*z = sensor->accelData.z * sensor->_lsm303Accel_MG_LSB * SENSORS_GRAVITY_STANDARD;
}

/**************************************************************************/
/*!
    @brief  Read mag data from LSM303
*/
/**************************************************************************/
void LSM303_readMagnetometerData(lsm303_t *sensor){
    uint8_t magDataRaw[NUM_MAG_REG];

	// request 6 uint8_ts from LSM303_ADDRESS_MAG
	// request 6 uint8_ts from LSM303_ADDRESS_MAG
	uint8_t write_buff[1] = {LSM303_REGISTER_MAG_OUT_X_H_M | 0x80};
	sensor->i2c_write(sensor->mag_addr,write_buff,1);

	// wait for data to be available
 	int ret = sensor->i2c_read(sensor->mag_addr,magDataRaw, NUM_MAG_REG);

	/* setup indeces for mag data */
	uint8_t xhi = 0;
	uint8_t xlo = 1;
	uint8_t zhi = 2;
	uint8_t zlo = 3;
	uint8_t yhi = 4;
	uint8_t ylo = 5;

	// Shift values to create properly formed integer (low uint8_t first)
	sensor->magData.x = (float)( (int16_t)(magDataRaw[xlo] | ((int16_t)magDataRaw[xhi] << 8)) );
	sensor->magData.y = (float)( (int16_t)(magDataRaw[ylo] | ((int16_t)magDataRaw[yhi] << 8)) );
	sensor->magData.z = (float)( (int16_t)(magDataRaw[zlo] | ((int16_t)magDataRaw[zhi] << 8)) );
}

/* getOrientation(float,float,float)
 * parameters: ponters to where orientation data should be saved 
 */
void LSM303_getOrientation(lsm303_t *sensor,float *x, float *y, float *z){
	bool readingValid = false;
  
	while(!readingValid)
	{

		uint8_t reg_mg = LSM303_read8(sensor,sensor->mag_addr, LSM303_REGISTER_MAG_SR_REG_Mg);
		if (!(reg_mg & 0x1)) {
			return;
		}

		LSM303_readMagnetometerData(sensor);
		
		if(!sensor->autoRange){
		    readingValid = true;
		}
		/* removed auto range code for now */
//    	else //removed autoRange code since I couldn't figure out how it was enabled
//    	{
//
//      	  /* Check if the sensor is saturating or not */
//      	  if ( (sensor->magData.x >= 2040) | (sensor->magData.x <= -2040) |
//           	   (sensor->magData.y >= 2040) | (sensor->magData.y <= -2040) |
//           	   (sensor->magData.z >= 2040) | (sensor->magData.z <= -2040) )
//      	  {
//        	/* Saturating .... increase the range if we can */
//        	switch(sensor->magGain)
//        	{
//          	  case LSM303_MAGGAIN_5_6:
//            	LSM303_setMagGain(sensor,LSM303_MAGGAIN_8_1);
//            	readingValid = false;
//
//            	break;
//          	  case LSM303_MAGGAIN_4_7:
//            	LSM303_setMagGain(sensor,LSM303_MAGGAIN_5_6);
//            	readingValid = false;
//
//            	break;
//          	  case LSM303_MAGGAIN_4_0:
//            	LSM303_setMagGain(sensor,LSM303_MAGGAIN_4_7);
//            	readingValid = false;
//
//            	break;
//          	  case LSM303_MAGGAIN_2_5:
//            	LSM303_setMagGain(sensor,LSM303_MAGGAIN_4_0);
//            	readingValid = false;
//
//            	break;
//          	  case LSM303_MAGGAIN_1_9:
//            	LSM303_setMagGain(sensor,LSM303_MAGGAIN_2_5);
//            	readingValid = false;
//
//            	break;
//          	  case LSM303_MAGGAIN_1_3:
//            	LSM303_setMagGain(sensor,LSM303_MAGGAIN_1_9);
//            	readingValid = false;
//
//            	break;
//          	  default:
//            	readingValid = true;
//            	break;
//        	}
//      	  }
//      	  else
//      	  {
//        	/* All values are withing range */
//        	readingValid = true;
//      	  }
//		}
	}
	*x = sensor->magData.x / sensor->_lsm303Mag_Gauss_LSB_XY * (float)SENSORS_GAUSS_TO_MICROTESLA;
	*y = sensor->magData.y / sensor->_lsm303Mag_Gauss_LSB_XY * (float)SENSORS_GAUSS_TO_MICROTESLA;
	*z = sensor->magData.z / sensor->_lsm303Mag_Gauss_LSB_Z * (float)SENSORS_GAUSS_TO_MICROTESLA;
}

/**************************************************************************/
/*!
    @brief  Sets the magnetometer's gain
*/
/**************************************************************************/
void LSM303_setMagGain(lsm303_t *sensor,lsm303MagGain gain)
{
  LSM303_writeCommand(sensor,sensor->mag_addr, LSM303_REGISTER_MAG_CRB_REG_M, (uint8_t)gain);
  
  sensor->magGain = gain;
 
  switch(gain)
  {
    case LSM303_MAGGAIN_1_3:
      sensor->_lsm303Mag_Gauss_LSB_XY = 1100;
      sensor->_lsm303Mag_Gauss_LSB_Z  = 980;
      break;
    case LSM303_MAGGAIN_1_9:
      sensor->_lsm303Mag_Gauss_LSB_XY = 855;
      sensor->_lsm303Mag_Gauss_LSB_Z  = 760;
      break;
    case LSM303_MAGGAIN_2_5:
      sensor->_lsm303Mag_Gauss_LSB_XY = 670;
      sensor->_lsm303Mag_Gauss_LSB_Z  = 600;
      break;
    case LSM303_MAGGAIN_4_0:
      sensor->_lsm303Mag_Gauss_LSB_XY = 450;
      sensor->_lsm303Mag_Gauss_LSB_Z  = 400;
      break;
    case LSM303_MAGGAIN_4_7:
      sensor->_lsm303Mag_Gauss_LSB_XY = 400;
      sensor->_lsm303Mag_Gauss_LSB_Z  = 355;
      break;
    case LSM303_MAGGAIN_5_6:
      sensor->_lsm303Mag_Gauss_LSB_XY = 330;
      sensor->_lsm303Mag_Gauss_LSB_Z  = 295;
      break;
    case LSM303_MAGGAIN_8_1:
      sensor->_lsm303Mag_Gauss_LSB_XY = 230;
      sensor->_lsm303Mag_Gauss_LSB_Z  = 205;
      break;
  } 
}

/**************************************************************************/
/*!
    @brief  Sets the magnetometer's update rate
*/
/**************************************************************************/
void LSM303_setMagRate(lsm303_t *sensor,lsm303MagRate rate)
{
	uint8_t reg_m = ((uint8_t)rate & 0x07) << 2;
  	LSM303_writeCommand(sensor,sensor->mag_addr, LSM303_REGISTER_MAG_CRA_REG_M, reg_m);
}

/**************************************************************************/
/*!
    @brief  Writes an 8 bit value over I2C
*/
/**************************************************************************/
void LSM303_writeCommand(lsm303_t *sensor,unsigned int address,unsigned int reg, unsigned char value)
{
	uint8_t buffer[2];
	buffer[0] = reg;
	buffer[1] = value;
	sensor->i2c_write(address,buffer,2);
}

/**************************************************************************/
/*!
    @brief  Reads an 8 bit value (byte) over I2C
*/
/**************************************************************************/
uint8_t LSM303_read8(lsm303_t *sensor,unsigned int address, unsigned int reg)
{
	uint8_t buffer[1];
	buffer[0] = reg;
	sensor->i2c_write(address,buffer,1); //write reg address
	sensor->i2c_read(address,buffer,1);  //read byte
	return buffer[0];
}	
