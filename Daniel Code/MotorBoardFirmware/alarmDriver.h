/*
 * alarmDriver.h
 *
 *  Created on: Nov 1, 2021
 *      Author: danie
 */

#ifndef ALARMDRIVER_H_
#define ALARMDRIVER_H_

#include "driverlib.h"

#define alarmPort GPIO_PORT_P3
#define alarmPin GPIO_PIN2

/* Initialize Pin 3.2 as GPIO for working with the Alaram
 *
 */
void initAlarm(void);

/* Turn on the alarm via reset pin
 *
 */
void enableAlarm(void);

/* Turn off the alarm via reset pin
 *
 */
void disableAlarm(void);

/* Turn off the alarm via reset pin
 *
 */
void toggleAlarm(void);

void startupSound(void);

/*  Pulse the alarm for duration in 0.065535s increments
 *
 */
void pulseAlarm(uint8_t duration);

#endif /* ALARMDRIVER_H_ */
