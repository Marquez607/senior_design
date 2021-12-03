/*
 * motorDriver.c
 *
 *  Created on: Nov 1, 2021
 *      Author: danie
 */

#include "motorDriver.h"
#include "timer_a.h"

uint8_t motorStopFlag = 0;

void initMotors(Timer_A_initUpModeParam* motorInitParam, Timer_A_initCompareModeParam* timerCompInitParam){
    // Set up MOTOR1 FWD Pin
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P4,
        GPIO_PIN0,
        GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Set up MOTOR1 BCKWD Pin
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P8,
        GPIO_PIN3,
        GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Set up MOTOR2 FWD Pin
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN7,
        GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Set up MOTOR2 BCKWD Pin
    GPIO_setAsPeripheralModuleFunctionOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN6,
        GPIO_PRIMARY_MODULE_FUNCTION
    );

    // Set up motor enable pins
    GPIO_setAsOutputPin( // Motor 1 Enable Pin
        GPIO_PORT_P2,
        GPIO_PIN6
    );

    GPIO_setAsOutputPin( // Motor 2 Enable Pin
        GPIO_PORT_P2,
        GPIO_PIN7
    );

    // By default enable both motors
    enableMotor1();
    enableMotor2();

    //Start timers

    Timer_A_initUpMode(TIMER_A1_BASE, motorInitParam);
    Timer_A_initUpMode(TIMER_A0_BASE, motorInitParam);

    Timer_A_initCompareMode(TIMER_A1_BASE, timerCompInitParam);
    Timer_A_initCompareMode(TIMER_A0_BASE, timerCompInitParam);

    timerCompInitParam->compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
    Timer_A_initCompareMode(TIMER_A1_BASE, timerCompInitParam);
    Timer_A_initCompareMode(TIMER_A0_BASE, timerCompInitParam);

    Timer_A_stop(TIMER_A1_BASE);
    Timer_A_stop(TIMER_A0_BASE);

    PMM_unlockLPM5();
}

void enableMotor1(void){
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN6);
}

void enableMotor2(void){
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN7);
}

void disableMotor1(void){
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN6);
}

void disableMotor2(void){
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
}

void disableMotors(void){
    disableMotor1();
    disableMotor2();
}

void setMotorStopFlag(void){
    motorStopFlag = 1;
}
void clearMotorStopFlag(void){
    motorStopFlag = 0;
}

//Generate PWM - Timer runs in Up mode
void PWMMotor(uint8_t motor, uint8_t dir, uint16_t dutyCycle, Timer_A_initCompareModeParam* PWMParam){
    if(dutyCycle >= 100){
        dutyCycle = 100;
    }
    PWMParam->compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    PWMParam->compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    PWMParam->compareValue = (TIMER_A_PERIOD/100)*dutyCycle; // Translate percentage to a number relative to period reg

    if(motor == MOTOR1 && dir == FORWARD){
        PWMParam->compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1; // 4.0 is CCR1 TimerA1
        Timer_A_setCompareValue(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_2,
            PWMParam->compareValue
        );
        Timer_A_stop(TIMER_A1_BASE);
        GPIO_setAsOutputPin(GPIO_PORT_P4,GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
        GPIO_setAsPeripheralModuleFunctionOutputPin(
           GPIO_PORT_P8,
           GPIO_PIN3,
           GPIO_PRIMARY_MODULE_FUNCTION
        );
        Timer_A_initCompareMode(TIMER_A1_BASE, PWMParam);
        Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    }
    else if(motor == MOTOR1 && dir == BACKWARD){
        PWMParam->compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2; // 8.3 is CCR2 TimerA1
        Timer_A_stop(TIMER_A1_BASE);
        Timer_A_setCompareValue(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_1,
            PWMParam->compareValue
        );
        GPIO_setAsOutputPin(GPIO_PORT_P8,GPIO_PIN3);
        GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN3);
        GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P4,
            GPIO_PIN0,
            GPIO_PRIMARY_MODULE_FUNCTION
        );
        Timer_A_initCompareMode(TIMER_A1_BASE, PWMParam);
        Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    }
    else if(motor == MOTOR2 && dir == FORWARD){
        PWMParam->compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1; // 1.7 is CCR1 TimerA0

        Timer_A_stop(TIMER_A0_BASE);
        Timer_A_setCompareValue(TIMER_A0_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_1,
            PWMParam->compareValue
        );
        GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN6);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN6);
        GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P1,
            GPIO_PIN7,
            GPIO_PRIMARY_MODULE_FUNCTION
        );
        Timer_A_initCompareMode(TIMER_A0_BASE, PWMParam);
        Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    }
    else if(motor == MOTOR2 && dir == BACKWARD){
        PWMParam->compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2; // 1.6 is CCR2 TimerA0

        Timer_A_stop(TIMER_A0_BASE);
        Timer_A_setCompareValue(TIMER_A0_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_2,
            PWMParam->compareValue
        );
        GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN7);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN7);
        GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P1,
            GPIO_PIN6,
            GPIO_PRIMARY_MODULE_FUNCTION
        );
        Timer_A_initCompareMode(TIMER_A0_BASE, PWMParam);
        Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    }
}

void turnOffMotor(uint8_t motor){
    if(motor == MOTOR1){
        Timer_A_stop(TIMER_A0_BASE);
        GPIO_setAsOutputPin(GPIO_PORT_P4,GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
        GPIO_setAsOutputPin(GPIO_PORT_P8,GPIO_PIN3);
        GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN3);
    }
    else if(motor == MOTOR2){
        Timer_A_stop(TIMER_A0_BASE);
        GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN7);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN7);
        GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN6);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN6);
    }
}

// Motor2 Forward Motor1 Backward
void moveRight(Timer_A_initCompareModeParam* PWMParam){
    PWMMotor(MOTOR2, FORWARD, TURN_POWER, PWMParam);
    PWMMotor(MOTOR1, BACKWARD, TURN_POWER, PWMParam);

    for(int loop2=0;loop2<10;loop2++){
        for(int duration=0;duration<TURN_DURATION;duration++){if(motorStopFlag == 1){return;}}
    }

    turnOffMotor(MOTOR1);
    turnOffMotor(MOTOR2);
}

// Motor2 Backward Motor1 Forward
void moveLeft(Timer_A_initCompareModeParam* PWMParam){
    PWMMotor(MOTOR2, BACKWARD, TURN_POWER, PWMParam);
    PWMMotor(MOTOR1, FORWARD, TURN_POWER, PWMParam);

    for(int loop2=0;loop2<10;loop2++){
        for(int duration=0;duration<TURN_DURATION;duration++){if(motorStopFlag == 1){return;}}
    }

    turnOffMotor(MOTOR1);
    turnOffMotor(MOTOR2);
}

// Both Motors Forward
void moveForward(Timer_A_initCompareModeParam* PWMParam){
    PWMMotor(MOTOR1, FORWARD, TURN_POWER, PWMParam);
    PWMMotor(MOTOR2, FORWARD, TURN_POWER, PWMParam);

    for(int loop2=0;loop2<10;loop2++){
        for(int duration=0;duration<TURN_DURATION;duration++){if(motorStopFlag == 1){return;}}
    }

    turnOffMotor(MOTOR1);
    turnOffMotor(MOTOR2);
}

// Both Motors Backward
void moveBackward(Timer_A_initCompareModeParam* PWMParam){
    PWMMotor(MOTOR1, BACKWARD, TURN_POWER, PWMParam);
    PWMMotor(MOTOR2, BACKWARD, TURN_POWER, PWMParam);

    for(int loop2=0;loop2<10;loop2++){
        for(int duration=0;duration<TURN_DURATION;duration++){if(motorStopFlag == 1){return;}}
    }

    turnOffMotor(MOTOR1);
    turnOffMotor(MOTOR2);
}

