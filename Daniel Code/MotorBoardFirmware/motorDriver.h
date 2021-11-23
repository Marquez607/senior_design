/*
 * motorDriver.h
 *
 *  Created on: Nov 1, 2021
 *      Author: danie
 */

#ifndef MOTORDRIVER_H_
#define MOTORDRIVER_H_

#include "driverlib.h"
#include "timer_a.h"

#define TIMER_A_PERIOD 0xFFFF


enum motorSelect{
    MOTOR1 = 1,
    MOTOR2 = 2,
};

enum motorDir{
    FORWARD = 1,
    BACKWARD = 2,
};

void initMotors(void);
void enableMotor1(void);
void enableMotor2(void);
void disableMotor1(void);
void disableMotor2(void);
void disableMotors(void);

// Motor is either MOTOR1 or MOTOR2
// dir is either FORWARD OR BACKWARD
// dutyCycle is a percent out of 100
void PWMMotor(uint8_t motor, uint8_t dir, uint16_t dutyCycle, Timer_A_initCompareModeParam* PWMParam);
void turnOffMotor(uint8_t motor);

// Don't care about the rest of this shit
void MOTOR1ForwardFull(void);
void MOTOR1BackwardFull(void);
void MOTOR2ForwardFull(void);
void MOTOR2BackwardFull(void);
void MOTOR1Forward(uint8_t dutyCycle);
void MOTOR1Backward(uint8_t dutyCycle);
void MOTOR2Forward(uint8_t dutyCycle);
void MOTOR2Backward(uint8_t dutyCycle);

#endif /* MOTORDRIVER_H_ */
