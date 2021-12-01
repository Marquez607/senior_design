//  SMCLK = MCLK = BRCLK = DCOCLKDIV = ~1MHz, ACLK = 32.768kHz

#include "driverlib.h"
#include "Board.h"
#include "alarmDriver.h"
#include "uartDriver.h"
#include "motorDriver.h"

uint16_t i;
uint8_t RXData = 0, TXData = 0;
uint8_t triggered = 0;
uint8_t check = 0;

void main(void)
{
    //Stop Watchdog Timer
    WDT_A_hold(WDT_A_BASE);

    initUart();
    initAlarm();

    Timer_A_initUpModeParam motorInitParam = {0};
    motorInitParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    motorInitParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    motorInitParam.timerPeriod = TIMER_A_PERIOD;
    motorInitParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    motorInitParam.captureCompareInterruptEnable_CCR0_CCIE =
        TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE;
    motorInitParam.timerClear = TIMER_A_DO_CLEAR;
    motorInitParam.startTimer = false;

    Timer_A_initCompareModeParam timerCompInitParam = {0};
    timerCompInitParam.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_1;
    timerCompInitParam.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    timerCompInitParam.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;
    timerCompInitParam.compareValue = 0xFFFF*(4/5);
    initMotors(&motorInitParam, &timerCompInitParam);

    enableAlarm();
    for(uint16_t delay=0; delay<UINT16_MAX; delay++);
    disableAlarm();


    // Enable global interrupts
    __enable_interrupt();
    while (1)
    {
        //EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 'f');

        if(RXData != 0){
            for(uint16_t delay=0; delay < UINT16_MAX; delay++);
            if(RXData == MOVE_FORW){
                moveForward(&timerCompInitParam);
            }
            else if(RXData == MOVE_RVRS){
                moveBackward(&timerCompInitParam);
            }
            else if(RXData == MOTOR_TURN_LEFT){
                moveLeft(&timerCompInitParam);
            }
            else if(RXData == MOTOR_TURN_RIGHT){
                moveRight(&timerCompInitParam);
            }
            RXData = 0; // Clear RX Data
        }
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
            if(RXData == MOTOR_STOP){
                turnOffMotor(MOTOR1);
                turnOffMotor(MOTOR2);
            }
            break;
       case USCI_UART_UCTXIFG: break;
       case USCI_UART_UCSTTIFG: break;
       case USCI_UART_UCTXCPTIFG: break;
    }
}

