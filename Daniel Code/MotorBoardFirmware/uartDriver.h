/*
 * uartDriver.h
 *
 *  Created on: Nov 1, 2021
 *      Author: danie
 */

#ifndef UARTDRIVER_H_
#define UARTDRIVER_H_

#include "driverlib.h"
#include "Board.h"

enum motor_cmd_t{
    MOTOR_TURN_RIGHT = 'R',
    MOTOR_TURN_LEFT  = 'L',
    MOVE_FORW  = 'F',
    MOVE_RVRS  = 'R',
    MOTOR_STOP = 'S',
};

void initUart(void);
void sendUart(void);
uint8_t receiveUART(void);



#endif /* UARTDRIVER_H_ */
