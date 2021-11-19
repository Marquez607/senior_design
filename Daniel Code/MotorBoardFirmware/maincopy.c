#include "driverlib.h"
#include "Board.h"

#include "alarmDriver.h"
#include "uartDriver.h"
#include "motorDriver.h"

uint16_t i;
uint8_t RXData = 0, TXData = 0;
uint8_t check = 0;

void main(void)
{
    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    // Power on delay for stuff to stabilize
    for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay

    //Set ACLK = REFOCLK with clock divider of 1
    CS_initClockSignal(CS_ACLK,CS_REFOCLK_SELECT,CS_CLOCK_DIVIDER_1);
    //Set SMCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_SMCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);
    //Set MCLK = DCO with frequency divider of 1
    CS_initClockSignal(CS_MCLK,CS_DCOCLKDIV_SELECT,CS_CLOCK_DIVIDER_1);

    initAlarm();
    initUart();
    initMotors();

    //Generate PWM - Timer runs in Up-Down mode
    Timer_A_outputPWMParam PWMParam = {0};
    PWMParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    PWMParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    PWMParam.timerPeriod = TIMER_A_PERIOD;
    PWMParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1; // 4.0 is CCR1 Timer1
    //PWMParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2; // 8.3 is CCR2 Timer1
    PWMParam.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    PWMParam.dutyCycle = 0xFFFF/2;

    Timer_A_outputPWM(TIMER_A1_BASE, &PWMParam);
    //Timer_A_outputPWM(TIMER_A1_BASE, &PWMParam);

    // Enable global interrupts
    __enable_interrupt();
    while (1)
    {
        // UWU
        GPIO_toggleOutputOnPin(GPIO_PORT_P3, GPIO_PIN2);

        //EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 'a');
        PWMParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1; // 4.0 is CCR1 Timer1
        Timer_A_outputPWM(TIMER_A1_BASE, &PWMParam);
        Timer_A_stop(TIMER_A1_BASE);
        Timer_A_clear(TIMER_A1_BASE);

        for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay

        Timer_A_stop(TIMER_A0_BASE);
        Timer_A_clear(TIMER_A0_BASE);
        PWMParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2; // 4.0 is CCR1 Timer1
        Timer_A_outputPWM(TIMER_A0_BASE, &PWMParam);

        for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay
        //Timer_A_stop(TIMER_A1_BASE);
        //Timer_A_clear(TIMER_A1_BASE);
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

