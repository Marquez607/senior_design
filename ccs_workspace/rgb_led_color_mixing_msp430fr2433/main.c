/* --COPYRIGHT--,BSD
 * Copyright (c) 2020, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//******************************************************************************
//  MSP430FR2433 Demo - UART RGB LED Color Mixing
//
//  Description: This example sets up Timer A to output PWM waveforms with
//  variable duty cycles based on UART byte values. These PWM waveforms control
//  the color of an external RGB LED. The PWM values are updated after every 3rd
//  byte received via UART.
//  ACLK = default REFO ~32768Hz
//  SMCLK = MCLK = DCO + FLL + 32KHz REFO REF = 1MHz
//
//           MSP430FR2433
//         ---------------
//     /|\|               |
//      | |               |
//      --|RST            |
//        |          P2.5 |<--- UART RX
//        |               |
//        |          P2.6 |---> UART TX
//        |               |
//        |          P1.2 |---> TA0.2 (Red)
//        |          P1.5 |---> TA1.1 (Green)
//        |          P1.4 |---> TA1.2 (Blue)
//
//   Aaron Barrera
//   Texas Instruments Inc.
//   October 2020
//   Built with Code Composer Studio v9.2 and IAR 7.20
//******************************************************************************

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

unsigned int valueIntoCCR(unsigned char colorVal);
#define GPIO_ALL        BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7
#define MCLK_FREQ_MHZ 1                                          // MCLK = 1MHz

// Declare global variables
volatile uint8_t uartByteNum;                                    // 0 for high, 1 for low

 // These variables are initialized in callbacks.h, so must be global here to use for UART
 volatile uint8_t redVal;       //Value of red
 volatile uint8_t greenVal;     //Value of green
 volatile uint8_t blueVal;      //Value of blue

// main.c
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                                     // Stop watchdog timer

    __bis_SR_register(SCG0);                                      // disable FLL

    // Initialize Clock System
    // SMCLK = MCLK = DCO + FLL + 32KHz REFO REF = 1MHz
    CSCTL3 |= SELREF__REFOCLK;                                    // Set REFO as FLL reference source
    CSCTL1 = DCOFTRIMEN | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0;      // DCOFTRIM=3, DCO Range = 1MHz
    CSCTL2 = FLLD_0 + 30;                                         // DCODIV = 1MHz
    __delay_cycles(3);
    __bic_SR_register(SCG0);                                      // enable FLL

    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;                    // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                                                  // default DCODIV as MCLK and SMCLK source
    // Initialize globals
    uartByteNum = 0;
    redVal = 0;
    greenVal = 0;
    blueVal = 0;

    //RED = P1.2 = TA0.2
    //GREEN = P1.4 = TA1.2
    //BLUE = P1.5 = TA1.1

    // Initialize ports
    P1OUT = 0;                                                    // Set all pins low
    P1DIR = GPIO_ALL;

    P1SEL1 |= BIT2 | BIT4 | BIT5;                                 // P1.1, P1.2, P1.5 options select

    P2OUT = 0;                                                    // Set all pins low
    P2DIR = GPIO_ALL;                                             // Set all pins as outputs

    P3OUT = 0;                                                    // Set all pins low
    P3DIR = GPIO_ALL;                                             // Set all pins as outputs

    P2SEL0 |= BIT5 | BIT6;                                        // Configure P1.4 and P1.5 for UART
    // Disable the GPIO power-on default high-impedance
    // mode to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure UART 9600 baud
    UCA1CTLW0 |= UCSWRST;                                         // eUSCI_A logic held in reset state.
    UCA1CTLW0 |= UCSSEL__SMCLK;                                   // One stop bit, no parity, LSB first, 8-bit data as the default setting
    UCA1BR0 = 6;                                                  // 1000000/16/9600
    UCA1BR1 = 0x00;
    UCA1MCTLW = 0x2000 | UCOS16 | UCBRF_8;
    UCA1CTLW0 &= ~UCSWRST;                                        // eUSCI_A reset released for operation
    UCA1IE = UCRXIE;                                              // Enable USCI_A0 RX interrupt

    // Initialize Timer_A
    TA0CCR0 = 1000-1;                       // PWM Period for TA0
    TA0CCTL2 = OUTMOD_7;                    // CCR2 reset/set
    TA0CCR2 = 0;                            // CCR2 PWM duty cycle (RED)

    TA1CCR0 = 1000-1;                       // PWM Period for TA1
    TA1CCTL2 = OUTMOD_7;                    // CCR1 reset/set
    TA1CCR2 = 0;                            // CCR1 PWM duty cycle (GREEN)
    TA1CCTL1 = OUTMOD_7;                    // CCR2 reset/set
    TA1CCR1 = 0;                            // CCR2 PWM duty cycle (BLUE)

    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR;  // SMCLK, up mode, clear TA0R
    TA1CTL = TASSEL__SMCLK | MC__UP | TACLR;  // SMCLK, up mode, clear TA1R

    __bis_SR_register(LPM0_bits | GIE);                       // Go to LPM0 with interrupts
    __no_operation();
}

// eUSCI interrupt service routine
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
    {
        case USCI_UART_UCRXIFG:
            SYSCFG0 = FRWPPW;                    // FRAM write enable
            if(uartByteNum == 0)                 //Set redVal to first UART byte
            {
                redVal = UCA1RXBUF;
            }
            else if (uartByteNum == 1)           //Set greenVal to second UART byte
            {
                greenVal = UCA1RXBUF;
            }
            else if (uartByteNum == 2)           //Set blueVal to third UART byte
            {                                    //Update LED color after 3 bytes of data
                blueVal = UCA1RXBUF;

                TA0CCR2 = redVal;
                TA1CCR2 = greenVal;
                TA1CCR1 = blueVal;
            }

            uartByteNum++;

            if (uartByteNum == 3)
            {
                uartByteNum = 0;
            }

            SYSCFG0 = FRWPPW | PFWP;             // FRAM write disable
            break;
        default: break;
    }
}

unsigned int valueIntoCCR(unsigned char colorVal)                 //Function to convert 16-bit value to CCR value
{
    unsigned int CCR = (unsigned int)(colorVal) * 3.91765;        //Factor = TAxCCR0/255 = 999/255 = 3.91765
    return (CCR);
}
