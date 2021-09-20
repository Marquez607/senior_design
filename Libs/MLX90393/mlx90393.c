/*********************************************
 * Name: mlx90393.c
 * Author: Marquez Jones
 * Desc: Adapted melexis magnetometer driver to 
 *       be hardware agnostic (I2C only) and pure C
 *
 * ORIGINAL FOUND HERE: https://www.melexis.com/en/product/MLX90393/Triaxis-Micropower-Magnetometer
 *********************************************/
#include "mlx90393.h"

//*************************************** MAIN FUNCTIONS ***************************************

//************************************* COMMUNICATION LEVEL ************************************

static void mlx90393_Send_I2C(char *receiveBuffer, char *sendBuffer, int sendMessageLength, int receiveMessageLength)
{
    char* tempSendBuffer = sendBuffer;
    char* tempReceiveBuffer = receiveBuffer;
    i2c->write(_I2CAddress, tempSendBuffer, sendMessageLength, true);
    i2c->read(_I2CAddress, receiveBuffer, receiveMessageLength);
    receiveBuffer = tempReceiveBuffer;
}

void mlx90393_EX(char *receiveBuffer, int mode)
{
    write_buffer[0] = 0x80;
    if (mode == 1) {
        Send_SPI(receiveBuffer, write_buffer, 1, 1);
    } else {
        Send_I2C(receiveBuffer, write_buffer, 1, 1);
    }
}

void mlx90393_SB(char *receiveBuffer, char zyxt, int mode)
{
    write_buffer[0] = (0x10)|(zyxt);
    if (mode == 1) {
        Send_SPI(receiveBuffer, write_buffer, 1, 1);
    } else {
        Send_I2C(receiveBuffer, write_buffer, 1, 1);
    }
}

void mlx90393_SWOC(char *receiveBuffer, char zyxt, int mode)
{
    write_buffer[0] = (0x20)|(zyxt);
    if (mode == 1) {
        Send_SPI(receiveBuffer, write_buffer, 1, 1);
    } else {
        Send_I2C(receiveBuffer, write_buffer, 1, 1);
    }
}

void mlx90393_SM(char *receiveBuffer, char zyxt, int mode)
{
    write_buffer[0] = (0x30)|(zyxt);
    if (mode == 1) {
        Send_SPI(receiveBuffer, write_buffer, 1, 1);
    } else {
        Send_I2C(receiveBuffer, write_buffer, 1, 1);
    }
}

void mlx90393_RM(char *receiveBuffer, char zyxt, int mode)
{
    write_buffer[0] = (0x40)|(zyxt);
    for(int i=0; i<2*count_set_bits(zyxt); i++) {
        write_buffer[i+2] = 0x00;
    }
    if (mode == 1) {
        Send_SPI(receiveBuffer, write_buffer, 1, 1+2*count_set_bits(zyxt));
    } else {
        Send_I2C(receiveBuffer, write_buffer, 1, 1+2*count_set_bits(zyxt));
    }
}

void mlx90393_RR(char *receiveBuffer, int address, int mode)
{
    write_buffer[0] = 0x50;
    write_buffer[1] = address << 2;
    if (mode == 1) {
        Send_SPI(receiveBuffer, write_buffer, 2, 3);
    } else {
        Send_I2C(receiveBuffer, write_buffer, 2, 3);
    }
}

void mlx90393_WR(char *receiveBuffer, int address, int data, int mode)
{
    write_buffer[0] = 0x60;
    write_buffer[1] = (data&0xFF00)>>8;
    write_buffer[2] = data&0x00FF;
    write_buffer[3] = address << 2;
    if (mode == 1) {
        Send_SPI(receiveBuffer, write_buffer, 4, 1);
    } else {
        Send_I2C(receiveBuffer, write_buffer, 4, 1);
    }
}

void mlx90393_RT(char *receiveBuffer, int mode)
{
    write_buffer[0] = 0xF0;
    if (mode == 1) {
        Send_SPI(receiveBuffer, write_buffer, 1, 1);
    } else {
        Send_I2C(receiveBuffer, write_buffer, 1, 1);
    }
}

void mlx90393_NOP(char *receiveBuffer, int mode)
{
    write_buffer[0] = 0x00;
    if (mode == 1) {
        Send_SPI(receiveBuffer, write_buffer, 1, 1);
    } else {
        Send_I2C(receiveBuffer, write_buffer, 1, 1);
    }
}

void mlx90393_HR(char *receiveBuffer, int mode)
{
    write_buffer[0] = 0xD0;
    if (mode == 1) {
        Send_SPI(receiveBuffer, write_buffer, 1, 1);
    } else {
        Send_I2C(receiveBuffer, write_buffer, 1, 1);
    }
}

void mlx90393_HS(char *receiveBuffer, int mode)
{
    write_buffer[0] = 0xE0;
    if (mode == 1) {
        Send_SPI(receiveBuffer, write_buffer, 1, 1);
    } else {
        Send_I2C(receiveBuffer, write_buffer, 1, 1);
    }
}

//*************************************** EXTRA FUNCTIONS **************************************

int mlx90393_count_set_bits(int N)
{
    int result = 0;
    while(N) {
        result++;
        N &=N-1;
    }
    return result;
}



