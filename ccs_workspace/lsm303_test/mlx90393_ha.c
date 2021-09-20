/*
  Elektor project 140555 MLX90393 BoB
  Copyright (c) 2015 Elektor.  All rights reserved.
  Author: CPV, 25/02/2015

  This is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this software; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*
 modifications:
 Removed the need for Elektor_utils.h and added all the helper functions as 
 static functions
 
 Made driver hardware agnostic
 */

#include "mlx90393_ha.h"
#include "stdint.h"

/************************** STATIC FUNCTIONS PROTO START ********************************/

// Generic I2C <start><write><repeated start><read><stop>.
// One buffer is used for TX and RX so you better make it
// large enough for both operations.
static uint8_t i2c_write_read(mlx90393_t *mag,uint8_t i2c_address, uint8_t *p_data, uint8_t tx_size, uint8_t rx_size);

// Assemble an uint16_t from <MSB,LSB>. 
static uint16_t assemble_16(uint8_t *p_data);

// Assemble an uint32_t from <MSB,LSB>. 
static uint32_t assemble_32(uint8_t *p_data);

/************************** STATIC FUNCTIONS PROTO END **********************************/


/********************************
 * send init commands to mlx90939
 * will not initialize hardware
 * interconnect
 ********************************/
int mlx90393_init_struct(mlx90393_t *mag){
  mag->addr = MLX90393_I2C_ADDRESS;
  mag->status = 0;
  mag->x = 0;
  mag->y = 0;
  mag->z = 0;
  mag->t = 0;
}

uint8_t mlx90393_command(mlx90393_t *mag,uint8_t cmd, uint8_t zyxt, uint8_t address, uint16_t data)
{
  uint8_t rx_size = 1;
  uint8_t tx_size = 1;
  cmd &= 0xf0;
  zyxt &= 0x0f;
  address &= 0x3f;

  switch (cmd)
  {
    case MLX90393_CMD_SB:
    case MLX90393_CMD_SWOC:
    case MLX90393_CMD_SM:
      cmd |= zyxt;
      break;

    case MLX90393_CMD_RM:
      cmd |= zyxt;
      if ((zyxt&MLX90393_T)!=0) rx_size += 2;
      if ((zyxt&MLX90393_X)!=0) rx_size += 2;
      if ((zyxt&MLX90393_Y)!=0) rx_size += 2;
      if ((zyxt&MLX90393_Z)!=0) rx_size += 2;
      break;

    case MLX90393_CMD_RR:
      mag->data_buffer[1] = address << 2;
      rx_size += 2;
      tx_size = 2;
      break;

    case MLX90393_CMD_WR:
      mag->data_buffer[1] = (data&0xff00) >> 8;
      mag->data_buffer[2] = data & 0x00ff;
      mag->data_buffer[3] = address << 2;
      tx_size = 4;
      break;

    case MLX90393_CMD_NOP:
    case MLX90393_CMD_EX:
    case MLX90393_CMD_RT:
    case MLX90393_CMD_HR:
    case MLX90393_CMD_HS:
      break;
  }

  mag->data_buffer[0] = cmd;
  return i2c_write_read(mag,mag->addr,(uint8_t*)mag->data_buffer,tx_size,rx_size);
}

void mlx90393_decode(mlx90393_t *mag, uint8_t zyxt)
{
  uint8_t *p = (uint8_t *)mag->data_buffer;
  mag->status = *p;
  p += 1;
  if ((zyxt&MLX90393_T)!=0)
  {
    mag->t = assemble_16(p);
    p += 2;
  }
  if ((zyxt&MLX90393_X)!=0)
  {
    mag->x = assemble_16(p);
    p += 2;
  }
  if ((zyxt&MLX90393_Y)!=0)
  {
    mag->y = assemble_16(p);
    p += 2;
  }
  if ((zyxt&MLX90393_Z)!=0)
  {
    mag->z = assemble_16(p);
    p += 2;
  }
}

uint16_t mlx90393_read_memory_word(mlx90393_t *mag, uint8_t address)
{
  mlx90393_command(mag,MLX90393_CMD_RR,0,address,0);
  // mlx90393_data_buffer[0] contains status code.
  return assemble_16((uint8_t*)&mag->data_buffer[1]);
}

void mlx90393_write_memory_word(mlx90393_t *mag, uint8_t address, uint16_t data)
{
  mlx90393_command(mag,MLX90393_CMD_WR,0,address,data);
}

void mlx90393_read_memory(mlx90393_t *mag, uint8_t *p_dst, uint8_t address, uint8_t size)
{
  uint16_t val;
  while (size!=0)
  {
     val = mlx90393_read_memory_word(mag,address);
     *p_dst = (val>>8)&0xff;
     p_dst += 1;
     *p_dst = val&0xff;
     p_dst += 1;
     address += 1;
     size -= 1;
  }
}

/************************** STATIC FUNCTIONS DEF START ********************************/

// Generic I2C <start><write><repeated start><read><stop>.
// One buffer is used for TX and RX so you better make it
// large enough for both operations.
// returns numbers of bytes read
static uint8_t i2c_write_read(mlx90393_t *mag, uint8_t i2c_address, uint8_t *p_data, uint8_t tx_size, uint8_t rx_size){

  int rc = mag->mag_user_i2c_write(i2c_address,p_data,tx_size);
  if(rc < 0){
    return 0;
  }

  rc = mag->mag_user_i2c_read(i2c_address,p_data,rx_size);
  if(rc < 0){
    return 0;
  }

  return rx_size;
}

// Assemble an uint16_t from <MSB,LSB>. 
static uint16_t assemble_16(uint8_t *p_data){
  uint16_t result = p_data[0];
  result = (result<<8) + p_data[1];
  return result;
}

// Assemble an uint32_t from <MSB,LSB>. 
static uint32_t assemble_32(uint8_t *p_data){
  int i;
  uint32_t result = p_data[0];
  for (i=1; i<4; i++)
  {
    result = (result<<8) + p_data[i];
  }
  return result;
}

/************************** STATIC FUNCTIONS DEF END **********************************/
