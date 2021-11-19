#include "driverlib.h"
#include "Board.h"

#include "alarmDriver.h"
#include "uartDriver.h"
#include "motorDriver.h"

#define TIMER_PERIOD 0xFFFF
#define TEST_DUTY 0xFF00

uint16_t i;
uint8_t RXData = 0, TXData = 0;
uint8_t check = 0;

void main(void)
{
    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    //Set ACLK = REFOCLK with clock divider of 1
    CS_initClockSignal(CS_ACLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
    //Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);
    //Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);

    GPIO_setAsOutputPin(GPIO_PORT_P4,GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P8,GPIO_PIN3);
    PMM_unlockLPM5();

    // Enable global interrupts
    __enable_interrupt();


    //Initialize compare mode to generate PWM1
    while (1)
    {
        // UWU
        GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN3);
        GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);

        for(uint16_t delay2 = 0; delay2 < 5; delay2++){
            for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay
        }

        GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
        GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN3);

        for(uint16_t delay2 = 0; delay2 < 5; delay2++){
            for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay
        }
    }

}
