/*
 * alarmDriver.c
 *
 *  Created on: Nov 1, 2021
 *      Author: danie
 */

#include "alarmDriver.h"

void initAlarm(void){
    //Setup Alarm Enable Pin PA.x output
    GPIO_setAsOutputPin(
        GPIO_PORT_P3,
        GPIO_PIN2
        );

    PMM_unlockLPM5();

    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN2);
};

void enableAlarm(void){
    GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN2);
};

void disableAlarm(void){
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN2);
};

void toggleAlarm(void){
    GPIO_toggleOutputOnPin(GPIO_PORT_P3, GPIO_PIN2);
};

void pulseAlarm(uint8_t duration){

};
