//  SMCLK = MCLK = BRCLK = DCOCLKDIV = ~1MHz, ACLK = 32.768kHz

#include "driverlib.h"
#include "Board.h"
#include "alarmDriver.h"
#include "uartDriver.h"

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
    enableAlarm();
    for(uint16_t  delay=0;delay<UINT16_MAX;delay++);
    disableAlarm();

    // Motor Init
    GPIO_setAsOutputPin(GPIO_PORT_P4,GPIO_PIN0);
    GPIO_setAsOutputPin(GPIO_PORT_P8,GPIO_PIN3);
    GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P1,GPIO_PIN6);

    // Enable global interrupts
    __enable_interrupt();
    while (1)
    {
        EUSCI_A_UART_transmitData(EUSCI_A0_BASE, 'f');
        if(RXData != 0){
            if(RXData == MOTOR_STOP){
                GPIO_toggleOutputOnPin(GPIO_PORT_P4,GPIO_PIN0);
            }
            else if(RXData == MOVE_FORW){
                GPIO_toggleOutputOnPin(GPIO_PORT_P8,GPIO_PIN3);
            }
            else if(RXData == MOVE_RVRS){
                GPIO_toggleOutputOnPin(GPIO_PORT_P1,GPIO_PIN7);
            }
            else if(RXData == MOTOR_TURN_LEFT){
                GPIO_toggleOutputOnPin(GPIO_PORT_P1,GPIO_PIN6);
            }
            else if(RXData == MOTOR_TURN_RIGHT){
                GPIO_toggleOutputOnPin(GPIO_PORT_P4,GPIO_PIN0);
            }
            RXData = 0; // Clear RX Data
        }
        for(uint16_t delay2=0; delay2 < 10; delay2++){ // Software Delay
            for(uint16_t delay=0; delay < UINT16_MAX; delay++); // Software Delay
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
            triggered = 1;
            break;
       case USCI_UART_UCTXIFG: break;
       case USCI_UART_UCSTTIFG: break;
       case USCI_UART_UCTXCPTIFG: break;
    }
}

