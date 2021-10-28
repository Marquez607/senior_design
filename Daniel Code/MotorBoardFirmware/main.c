//******************************************************************************
//   MSP430FR413x Demo - eUSCI_A0 UART echo at 9600 baud using BRCLK = 8MHz
//
//  Description: This demo echoes back characters received via a PC serial port.
//  SMCLK/ DCO is used as a clock source and the device is put in LPM3
//  The auto-clock enable feature is used by the eUSCI and SMCLK is turned off
//  when the UART is idle and turned on when a receive edge is detected.
//  Note that level shifter hardware is needed to shift between RS232 and MSP
//  voltage levels.
//
//  The example code shows proper initialization of registers
//  and interrupts to receive and transmit data.
//  To test code in LPM3, disconnect the debugger.
//
//  ACLK = REFO = 32768Hz, MCLK = DCODIV = SMCLK = 8MHz.
//
//                MSP430FR4133
//             -----------------
//         /|\|                 |
//          | |                 |
//          --|RST              |
//            |                 |
//            |                 |
//            |     P1.0/UCA0TXD|----> PC (echo)
//            |     P1.1/UCA0RXD|<---- PC
//            |                 |
//
//******************************************************************************

#include <msp430.h>

void Init_GPIO();

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;                // Stop watchdog timer

  // Configure GPIO
  Init_GPIO();
  PM5CTL0 &= ~LOCKLPM5;                    // Disable the GPIO power-on default high-impedance mode
                                           // to activate 1previously configured port settings

  __bis_SR_register(SCG0);                 // disable FLL
  CSCTL3 |= SELREF__REFOCLK;               // Set REFO as FLL reference source
  CSCTL0 = 0;                              // clear DCO and MOD registers
  CSCTL1 &= ~(DCORSEL_7);                  // Clear DCO frequency select bits first
  CSCTL1 |= DCORSEL_3;                     // Set DCO = 8MHz
  CSCTL2 = FLLD_0 + 243;                   // DCODIV = 8MHz
  __delay_cycles(3);
  __bic_SR_register(SCG0);                 // enable FLL
  while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked

  CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                           // default DCODIV as MCLK and SMCLK source

  // Configure UART pins
  P1SEL0 |= BIT0 | BIT1;                    // set 2-UART pin as second function

  // Configure UART
  UCA0CTLW0 |= UCSWRST;
  UCA0CTLW0 |= UCSSEL__SMCLK;

  // Baud Rate calculation
  // 8000000/(16*9600) = 52.083
  // Fractional portion = 0.083
  // User's Guide Table 14-4: UCBRSx = 0x49
  // UCBRFx = int ( (52.083-52)*16) = 1
  UCA0BR0 = 52;                             // 8000000/16/9600
  UCA0BR1 = 0x00;
  UCA0MCTLW = 0x4900 | UCOS16 | UCBRF_1;

  UCA0CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
  UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

  __bis_SR_register(LPM3_bits|GIE);         // Enter LPM3, interrupts enabled
  __no_operation();                         // For debugger
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA0IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
      while(!(UCA0IFG&UCTXIFG));
      UCA0TXBUF = UCA0RXBUF;
      __no_operation();
      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    default: break;
  }
}

void Init_GPIO()
{
    P1DIR = 0xFF; P2DIR = 0xFF; P3DIR = 0xFF; P4DIR = 0xFF;
    P5DIR = 0xFF; P6DIR = 0xFF; P7DIR = 0xFF; P8DIR = 0xFF;
    P1REN = 0xFF; P2REN = 0xFF; P3REN = 0xFF; P4REN = 0xFF;
    P5REN = 0xFF; P6REN = 0xFF; P7REN = 0xFF; P8REN = 0xFF;
    P1OUT = 0x00; P2OUT = 0x00; P3OUT = 0x00; P4OUT = 0x00;
    P5OUT = 0x00; P6OUT = 0x00; P7OUT = 0x00; P8OUT = 0x00;
}
