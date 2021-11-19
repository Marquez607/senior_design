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

    initAlarm();
    initUart();
    initMotors();

    disableAlarm();

    // Power on delay for stuff to stabilize
    for(uint16_t delay2=0; delay2 < 15; delay2++){
        for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay
    }

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
    // Need the struct to be allocated within main
    Timer_A_initCompareModeParam PWMParam = {0};
    disableAlarm();
    while (1)
    {
        // UWU
        //toggleAlarm();
        //GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN2);
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 'a');
        GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN3);
        for(uint16_t delay=0; delay < UINT16_MAX; delay++);
        GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN0);

        for(uint16_t delay2 = 0; delay2 < 10; delay2++){
            for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay
        }

        GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN0);
        for(uint16_t delay=0; delay < UINT16_MAX; delay++);
        GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN3);

        for(uint16_t delay2 = 0; delay2 < 10; delay2++){
            for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay
        }
        /*
        PWMMotor(MOTOR1, FORWARD, TEST_DUTY, &PWMParam);
        for(uint16_t delay2 = 0; delay2 < 10; delay2++){
            for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay
        }

        PWMMotor(MOTOR1, BACKWARD, TEST_DUTY, &PWMParam);
        for(uint16_t delay2 = 0; delay2 < 10; delay2++){
            for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay
        }

        PWMMotor(MOTOR2, FORWARD, TEST_DUTY, &PWMParam);
        for(uint16_t delay2 = 0; delay2 < 10; delay2++){
            for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay
        }

        PWMMotor(MOTOR2, BACKWARD, TEST_DUTY, &PWMParam);
        for(uint16_t delay2 = 0; delay2 < 10; delay2++){
            for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay
        }
    */
    }

}
//******************************************************************************
//
//This is the USCI_A0 interrupt vector service routine.
//
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_A0_VECTOR)))
#endif
void EUSCI_A0_ISR(void)
{
    switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
    {
        case USCI_NONE: break;
        case USCI_UART_UCRXIFG:
            RXData = EUSCI_A_UART_receiveData(EUSCI_A0_BASE);
            //EUSCI_A_UART_transmitData(EUSCI_A0_BASE, RXData);
            break;
       case USCI_UART_UCTXIFG: break;
       case USCI_UART_UCSTTIFG: break;
       case USCI_UART_UCTXCPTIFG: break;
    }
}

