/*
 * motorDriver.c
 *
 *  Created on: Nov 1, 2021
 *      Author: danie
 */

#include "motorDriver.h"
#include "timer_a.h"

void initMotors(void){
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
    Timer_A_initUpModeParam param = {0};
    param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    param.timerPeriod = TIMER_A_PERIOD;
    param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    param.captureCompareInterruptEnable_CCR0_CCIE =
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE;
    param.timerClear = TIMER_A_DO_CLEAR;
    param.startTimer = false;
    Timer_A_initUpMode(TIMER_A1_BASE, &param);
    Timer_A_initUpMode(TIMER_A0_BASE, &param);

    Timer_A_initCompareModeParam initComp1Param = {0};
    initComp1Param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    initComp1Param.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    initComp1Param.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    initComp1Param.compareValue = 0xFFF;
    Timer_A_initCompareMode(TIMER_A1_BASE, &initComp1Param);
    Timer_A_initCompareMode(TIMER_A0_BASE, &initComp1Param);

    initComp1Param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
    Timer_A_initCompareMode(TIMER_A1_BASE, &initComp1Param);
    Timer_A_initCompareMode(TIMER_A0_BASE, &initComp1Param);

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


//Generate PWM - Timer runs in Up mode
void PWMMotor(uint8_t motor, uint8_t dir, uint16_t dutyCycle, Timer_A_initCompareModeParam* PWMParam){
    if(dutyCycle >= 100){
        dutyCycle = 100;
    }
    PWMParam->compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    PWMParam->compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    PWMParam->compareValue = dutyCycle; // Translate percentage to a number relative to period reg

    if(motor == MOTOR1 && dir == FORWARD){
        PWMParam->compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1; // 4.0 is CCR1 TimerA1
        Timer_A_setCompareValue(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_2,
            TIMER_A_PERIOD
        );
        Timer_A_stop(TIMER_A1_BASE);
        GPIO_setAsOutputPin(GPIO_PORT_P4,GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
        GPIO_setAsPeripheralModuleFunctionOutputPin(
           GPIO_PORT_P8,
           GPIO_PIN3,
           GPIO_PRIMARY_MODULE_FUNCTION
        );
        for(uint16_t delay=0; delay < UINT16_MAX/4; delay++); // Delay to not accidentally short H-Bridge
        Timer_A_initCompareMode(TIMER_A1_BASE, &PWMParam);
        Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    }
    else if(motor == MOTOR1 && dir == BACKWARD){
        PWMParam->compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2; // 8.3 is CCR2 TimerA1
        Timer_A_stop(TIMER_A1_BASE);
        Timer_A_setCompareValue(TIMER_A1_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_1,
            TIMER_A_PERIOD
        );
        GPIO_setAsOutputPin(GPIO_PORT_P8,GPIO_PIN3);
        GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN3);
        GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P4,
            GPIO_PIN0,
            GPIO_PRIMARY_MODULE_FUNCTION
        );
        for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Delay to not accidentally short H-Bridge
        Timer_A_initCompareMode(TIMER_A1_BASE, &PWMParam);
        Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    }
    else if(motor == MOTOR2 && dir == FORWARD){
        PWMParam->compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1; // 1.7 is CCR1 TimerA0

        Timer_A_stop(TIMER_A0_BASE);
        Timer_A_setCompareValue(TIMER_A0_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_2,
            TIMER_A_PERIOD
        );
        GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN6);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN6);
        GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P1,
            GPIO_PIN7,
            GPIO_PRIMARY_MODULE_FUNCTION
        );
        for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Delay to not accidentally short H-Bridge
        Timer_A_initCompareMode(TIMER_A0_BASE, &PWMParam);
        Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE);
    }
    else if(motor == MOTOR2 && dir == BACKWARD){
        PWMParam->compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2; // 1.6 is CCR2 TimerA0

        Timer_A_stop(TIMER_A0_BASE);
        Timer_A_setCompareValue(TIMER_A0_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_1,
            TIMER_A_PERIOD
        );
        GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN7);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN7);
        GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P1,
            GPIO_PIN6,
            GPIO_PRIMARY_MODULE_FUNCTION
        );
        for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Delay to not accidentally short H-Bridge
        Timer_A_initCompareMode(TIMER_A0_BASE, &PWMParam);
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

