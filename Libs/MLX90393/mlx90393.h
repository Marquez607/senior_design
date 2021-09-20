/*********************************************
 * Name: mlx90393.h
 * Author: Marquez Jones
 * Desc: Adapted melexis magnetometer driver to 
 *       be hardware agnostic (I2C only) and pure C
 * ORIGINAL FOUND HERE: https://www.melexis.com/en/product/MLX90393/Triaxis-Micropower-Magnetometer
 *********************************************/
#ifndef MLX90393_H
#define MLX90393_H

// Possible I2C addresses.
#define MLX90393_I2C_ADDRESS  (0x0c)  /* A0=0, A1=0. */

// Key commands.
#define MLX90393_CMD_NOP  (0x00)  /* No OPeration */
#define MLX90393_CMD_SB  (0x10)  /* Start Burst mode */
#define MLX90393_CMD_SWOC  (0x20)  /* Start Wake-up On Change */
#define MLX90393_CMD_SM  (0x30)  /* Start Measurement (polling mode) */
#define MLX90393_CMD_RM  (0x40)  /* Read Measurement */
#define MLX90393_CMD_RR  (0x50)  /* Read from a Register */
#define MLX90393_CMD_WR  (0x60)  /* Write from a Register */
#define MLX90393_CMD_EX  (0x80)  /* EXit */
#define MLX90393_CMD_HR  (0xd0)  /* Memory Recall */
#define MLX90393_CMD_HS  (0xe0)  /* Memory Store */
#define MLX90393_CMD_RT  (0xf0)  /* Reset */

// Flags to use with "zyxt" variables.
#define MLX90393_T  (0x01)  /* Temperature */
#define MLX90393_X  (0x02)  /* X-axis */
#define MLX90393_Y  (0x04)  /* Y-axis */
#define MLX90393_Z  (0x08)  /* Z-axis */

// Memory areas.
#define MLX90393_CUSTOMER_AREA_BEGIN  (0x00)
#define MLX90393_CUSTOMER_AREA_END  (0x1f)
#define MLX90393_MELEXIS_AREA_BEGIN  (0x20)
#define MLX90393_MELEXIS_AREA_END  (0x3f)

/** Send 'exit' command to MLX90393.
* @param *receiveBuffer Pointer to receiveBuffer, will contain response of IC after command is sent.
* @param mode Communication mode (0=I2C, 1=SPI).
* @note The receiveBuffer will contain the status byte only.
*/
void mlx90393_EX(char *receiveBuffer, int mode);


/** Send 'start burst mode' command to MLX90393.
* @param *receiveBuffer Pointer to receiveBuffer, will contain response of IC after command is sent.
* @param mode Communication mode (0=I2C, 1=SPI).
* @note The receiveBuffer will contain the status byte only.
*/
void mlx90393_SB(char *receiveBuffer, char zyxt, int mode);


/** Send 'start wake-up on change mode' command to MLX90393.
* @param *receiveBuffer Pointer to receiveBuffer, will contain response of IC after command is sent.
* @param zyxt Selection of the axes/temperature to which the mode should apply.
* @param mode Communication mode (0=I2C, 1=SPI).
* @note The receiveBuffer will contain the status byte only.
*/
void mlx90393_SWOC(char *receiveBuffer, char zyxt, int mode);


/** Send 'single measurement' command to MLX90393.
* @param *receiveBuffer Pointer to receiveBuffer, will contain response of IC after command is sent.
* @param zyxt Selection of the axes/temperature to be measured.
* @param mode Communication mode (0=I2C, 1=SPI).
* @note The receiveBuffer will contain the status byte only.
*/
void mlx90393_SM(char *receiveBuffer, char zyxt, int mode);


/** Send 'read measurement' command to MLX90393.
* @param *receiveBuffer Pointer to receiveBuffer, will contain response of IC after command is sent.
* @param zyxt Selection of the axes/temperature to be read out.
* @param mode Communication mode (0=I2C, 1=SPI).
* @note The receiveBuffer will contain the status byte, followed by 2 bytes for each T, X, Y and Z (depending on zyxt, some can be left out).
*/
void mlx90393_RM(char *receiveBuffer, char zyxt, int mode);


/** Send 'read register' command to MLX90393.
* @param *receiveBuffer Pointer to receiveBuffer, will contain response of IC after command is sent.
* @param address The register to be read out.
* @param mode Communication mode (0=I2C, 1=SPI).
* @note The receiveBuffer will contain the status byte, followed by 2 bytes for the data at the specific register.
*/
void mlx90393_RR(char *receiveBuffer, int address, int mode);


/** Send 'write register' command to MLX90393.
* @param *receiveBuffer Pointer to receiveBuffer, will contain response of IC after command is sent.
* @param address The register to be written.
* @param data The 16-bit word to be written in the register.
* @param mode Communication mode (0=I2C, 1=SPI).
* @note The receiveBuffer will only contain the status byte.
*/
void mlx90393_WR(char *receiveBuffer, int address, int data, int mode);


/** Send 'reset' command to MLX90393.
* @param *receiveBuffer Pointer to receiveBuffer, will contain response of IC after command is sent.
* @param mode Communication mode (0=I2C, 1=SPI).
* @note The receiveBuffer will contain the status byte only.
*/
void mlx90393_RT(char *receiveBuffer, int mode);


/** Send 'NOP' command to MLX90393.
* @param *receiveBuffer Pointer to receiveBuffer, will contain response of IC after command is sent.
* @param mode Communication mode (0=I2C, 1=SPI).
* @note The receiveBuffer will contain the status byte only.
*/
void mlx90393_NOP(char *receiveBuffer, int mode);


/** Send 'memory recall' command to MLX90393.
* @param *receiveBuffer Pointer to receiveBuffer, will contain response of IC after command is sent.
* @param mode Communication mode (0=I2C, 1=SPI).
* @note The receiveBuffer will contain the status byte only.
*/
void mlx90393_HR(char *receiveBuffer, int mode);

/** Send 'memory store' command to MLX90393.
* @param *receiveBuffer Pointer to receiveBuffer, will contain response of IC after command is sent.
* @param mode Communication mode (0=I2C, 1=SPI).
* @note The receiveBuffer will contain the status byte only.
*/
void mlx90393_HS(char *receiveBuffer, int mode);
int mlx90393_count_set_bits(int N);

//I2C
I2C* i2c;
void Send_I2C(char *receiveBuffer, char *sendBuffer, int sendMessageLength, int receiveMessageLength);
int _I2CAddress;

#endif
