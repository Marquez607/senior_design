/*****************************************************************************
 * Name: main.c
 * Author: Marquez Jones
 * Desc: main program for MCU 2 on Control board
 *       for now will act as LCD driver
 *****************************************************************************/
#include "driverlib.h"
#include "ANY_LCD.h"
#include "Board.h"
#include <stdint.h>

/********************************** defines *****************************/

#define MCU_CLK 1E6 /* default clk should be 1MHz */
#define UALERT_PIN GPIO_PIN4 /* pin o notify launchpad of ultrasonic warnings */
#define UALERT_PORT GPIO_PORT_P1
#define SLAVE_ADDRESS 0x48
#define RESET_LCD 0xFF /* special lcd code */

/********************************** prototypes *****************************/

void gpio_init(void);
void delayUS(uint32_t usec);
void lcdWrite(uint8_t bitfield);
void u_alert_high(void);
void u_alert_low(void);
void i2c_slave_init();

/*
 * Name: i2c_data_handler
 * Desc: decodes i2c data
 *       for now will just handle the LCD
 */
void i2c_data_handler(uint8_t data);

/********************************** data ***********************************/

static const uint8_t reverse_nibble_lookup[8] = {
0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe};
static uint8_t RXData;
static any_lcd_t lcd;

void main (void)
{
    //Stop WDT
    WDT_A_hold(WDT_A_BASE);

    gpio_init();

    i2c_slave_init();

    /****LCD INIT*****/
    lcd.delay_us  = &delayUS;
    lcd.lcd_write = &lcdWrite;

    LCD_Init(lcd); //will initialize lcd "logically" ie send the correct commands for 4 bit mode
    LCD_WriteString("ANY_LCD",lcd);


    /* leave this for now */
    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
    __no_operation();
    while(1);
}

void u_alert_high(void){
    GPIO_setOutputHighOnPin(UALERT_PORT,UALERT_PIN);
}
void u_alert_low(void){
    GPIO_setOutputLowOnPin(UALERT_PORT,UALERT_PIN);
}

void gpio_init(void){

    /* data pins on Port 7 */
    GPIO_setAsOutputPin(
        GPIO_PORT_P7,
        GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + GPIO_PIN3
    );

    /* control signal */
    GPIO_setAsOutputPin(
        GPIO_PORT_P1,
        GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2 + UALERT_PIN
    );

    /* i2c pins */
    GPIO_setAsPeripheralModuleFunctionInputPin(
        GPIO_PORT_P5,
        GPIO_PIN2 + GPIO_PIN3,
        GPIO_PRIMARY_MODULE_FUNCTION
    );

  /*
   * Disable the GPIO power-on default high-impedance mode to activate
   * previously configured port settings
   */
    PMM_unlockLPM5();
}

void i2c_slave_init(){
    // eUSCI configuration
    EUSCI_B_I2C_initSlaveParam param = {0};
    param.slaveAddress = SLAVE_ADDRESS;
    param.slaveAddressOffset = EUSCI_B_I2C_OWN_ADDRESS_OFFSET0;
    param.slaveOwnAddressEnable = EUSCI_B_I2C_OWN_ADDRESS_ENABLE;
    EUSCI_B_I2C_initSlave(EUSCI_B0_BASE, &param);

    EUSCI_B_I2C_enable(EUSCI_B0_BASE);

    EUSCI_B_I2C_clearInterrupt(EUSCI_B0_BASE,
                EUSCI_B_I2C_RECEIVE_INTERRUPT0
                );

    EUSCI_B_I2C_enableInterrupt(EUSCI_B0_BASE,
                EUSCI_B_I2C_RECEIVE_INTERRUPT0
                );

}

/*
 * Name: i2c_data_handler
 * Desc: decodes i2c data
 *       for now will just handle the LCD
 */
void i2c_data_handler(uint8_t data){

    if(data == RESET_LCD){
        LCD_Reset(lcd);
    }
    else if(data == '\0'){
        /* do nothing */
    }
    else{
        /* char handling */
        LCD_WriteData(data,lcd);
    }

}

/*************************** lcd code *******************************/

void delayUS(uint32_t usec){

    for(uint32_t i=0;i<usec;i++){
        __delay_cycles(1);
    }
}
void lcdWrite(uint8_t bitfield){
    /*
     * LCD DATA 7-4 -> P7.3 - P7.0
     * LCD E        -> P1.0
     * LCD RW       -> P1.1
     * LCD RS       -> P1.2
     */

    /* I needed to reverse the control nibble */
    uint8_t ctrl_bits = reverse_nibble_lookup[bitfield&0x07] >> 1;

    /* P1 control signals */
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, ( ~ctrl_bits ) & 0x07);
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, ctrl_bits & 0x07);

    /* P7 data signals */
    GPIO_setOutputLowOnPin(GPIO_PORT_P7, (~bitfield >> 4) & 0x0F );
    GPIO_setOutputHighOnPin(GPIO_PORT_P7, (bitfield >> 4) & 0x0F );
}

/***************************** INTERRUPTS ********************************/

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_B0_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(USCI_B0_VECTOR)))
#endif
void USCIB0_ISR(void)
{
    switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
    {
        case USCI_NONE:             // No interrupts break;
            break;
        case USCI_I2C_UCALIFG:      // Arbitration lost
            break;
        case USCI_I2C_UCNACKIFG:    // NAK received (master only)
            break;
        case USCI_I2C_UCSTTIFG:     // START condition detected with own address (slave mode only)
            break;
        case USCI_I2C_UCSTPIFG:     // STOP condition detected (master & slave mode)
            break;
        case USCI_I2C_UCRXIFG3:     // RXIFG3
            break;
        case USCI_I2C_UCTXIFG3:     // TXIFG3
            break;
        case USCI_I2C_UCRXIFG2:     // RXIFG2
            break;
        case USCI_I2C_UCTXIFG2:     // TXIFG2
            break;
        case USCI_I2C_UCRXIFG1:     // RXIFG1
            break;
        case USCI_I2C_UCTXIFG1:     // TXIFG1
            break;
        case USCI_I2C_UCRXIFG0:     // RXIFG0
            RXData = EUSCI_B_I2C_slaveGetData(EUSCI_B0_BASE);
            i2c_data_handler(RXData);
            break;
        case USCI_I2C_UCTXIFG0:     // TXIFG0
            break;
        case USCI_I2C_UCBCNTIFG:    // Byte count limit reached (UCBxTBCNT)
            break;
        case USCI_I2C_UCCLTOIFG:    // Clock low timeout - clock held low too long
            break;
        case USCI_I2C_UCBIT9IFG:    // Generated on 9th bit of a transmit (for debugging)
            break;
        default:
            break;
    }
}
