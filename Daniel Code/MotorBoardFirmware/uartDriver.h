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

void initUart(void);
void sendUart(void);
uint8_t receiveUART(void);



#endif /* UARTDRIVER_H_ */
