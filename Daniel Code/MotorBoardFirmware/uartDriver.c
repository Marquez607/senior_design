/*
 * uartDriver.c
 *
 *  Created on: Nov 1, 2021
 *      Author: danie
 */

#include "uartDriver.h"

void initUart(void){
    //Configure UART pins
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_UCA0TXD,
        GPIO_PIN_UCA0TXD,
        GPIO_FUNCTION_UCA0TXD
    );
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_UCA0RXD,
        GPIO_PIN_UCA0RXD,
        GPIO_FUNCTION_UCA0RXD
    );

    /*
    * Disable the GPIO power-on default high-impedance mode to activate
    * previously configured port settings
    */
   PMM_unlockLPM5();

   //Configure UART
   //SMCLK = 1MHz, Baudrate = 115200
   //UCBRx = 8, UCBRFx = 0, UCBRSx = 0xD6, UCOS16 = 0
   EUSCI_A_UART_initParam param = {0};
   param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
   param.clockPrescalar = 8;
   param.firstModReg = 0;
   param.secondModReg = 0xD6;
   param.parity = EUSCI_A_UART_NO_PARITY;
   param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
   param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
   param.uartMode = EUSCI_A_UART_MODE;
   param.overSampling = EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;

   if (STATUS_FAIL == EUSCI_A_UART_init(EUSCI_A0_BASE, &param)) {
       return;
   }

   EUSCI_A_UART_enable(EUSCI_A0_BASE);

   EUSCI_A_UART_clearInterrupt(EUSCI_A0_BASE,
       EUSCI_A_UART_RECEIVE_INTERRUPT);

   // Enable USCI_A0 RX interrupt
   EUSCI_A_UART_enableInterrupt(EUSCI_A0_BASE,
   EUSCI_A_UART_RECEIVE_INTERRUPT);
}

void sendUart(void){

}

uint8_t receiveUART(void){

}
