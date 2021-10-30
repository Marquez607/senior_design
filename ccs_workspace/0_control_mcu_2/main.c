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
#include <stdbool.h>
#include <stdio.h>

/********************************** defines *****************************/

#define CYCLES_TO_MS 1000 /* 1000 cyles = 1 ms */
#define CYCLES_TO_100US 100
#define US_TO_CM 0.034f

#define ULTRA_PORT GPIO_PORT_P3 //all ultra sonics on same port
#define ULTRA_TRIG0 GPIO_PIN0 /* right */
#define ULTRA_ECHO0 GPIO_PIN1
#define ULTRA_TRIG1 GPIO_PIN2 /* center */
#define ULTRA_ECHO1 GPIO_PIN3
#define ULTRA_TRIG2 GPIO_PIN4 /* left */
#define ULTRA_ECHO2 GPIO_PIN5

#define UALERT_PIN GPIO_PIN4 /* pin o notify launchpad of ultrasonic warnings */
#define UALERT_PORT GPIO_PORT_P1

#define SLAVE_ADDRESS 0x48
#define RESET_LCD 0xFF /* special lcd code */

typedef enum ultra{
    ULTRA_0, /* right */
    ULTRA_1, /* center */
    ULTRA_2  /* left */
}ultra_t;

/********************************** prototypes *****************************/

/* inits */
void gpio_init(void);
void i2c_slave_init();

/* lcd functions */
void delayUS(uint32_t usec);
void lcdWrite(uint8_t bitfield);

/* ultra sonic */
void u_alert_high(void);
void u_alert_low(void);
int16_t check_ultra(ultra_t dev); /* returns the distance in cm of ultra sonic */
void trig_ultra(ultra_t dev);      /* trigger ultra */

/* return -1 on fail */
int8_t check_echo_ultra(ultra_t dev);/* read echo pin  */

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

/* configured over i2c */
static int8_t too_close_cm = 20; /* 10 cm */

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
//    __bis_SR_register(CPUOFF + GIE);        // Enter LPM0 w/ interrupts
    __bis_SR_register(GIE);
//    __no_operation();
    while(1){

        int8_t dist_0 = check_ultra(ULTRA_0);
        int8_t dist_1 = check_ultra(ULTRA_1);
        int8_t dist_2 = check_ultra(ULTRA_2);

        if(dist_0 < too_close_cm){
            u_alert_high();
        }
        else if (dist_1 < too_close_cm){
            u_alert_high();
        }
        else if (dist_2 < too_close_cm){
            u_alert_high();
        }
        else{
            u_alert_low();
        }
    }
}

void u_alert_high(void){
    GPIO_setOutputHighOnPin(UALERT_PORT,UALERT_PIN);
}
void u_alert_low(void){
    GPIO_setOutputLowOnPin(UALERT_PORT,UALERT_PIN);
}

void gpio_init(void){
    /*
    #define ULTRA_PORT GPIO_PORT_P3 //all ultra sonics on same port
    #define ULTRA_TRIG0 GPIO_PIN0
    #define ULTRA_ECHO0 GPIO_PIN1
    #define ULTRA_TRIG1 GPIO_PIN2
    #define ULTRA_ECHO1 GPIO_PIN3
    #define ULTRA_TRIG2 GPIO_PIN4
    #define ULTRA_ECHO2 GPIO_PIN5
    */

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

    /* ultra echo pins */
    GPIO_setAsInputPin(
        ULTRA_PORT,
        ULTRA_ECHO0 + ULTRA_ECHO1 + ULTRA_ECHO2
    );

    /* ultra trig pins */
    GPIO_setAsOutputPin(
        ULTRA_PORT,
        ULTRA_TRIG0 + ULTRA_TRIG1 + ULTRA_TRIG2
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

/*************************** ultra sonics ***************************/

/* returns the distance in cm of ultra sonic */
int16_t check_ultra(ultra_t dev){
    /* note: counts distance in increments of 100 us */

    trig_ultra(dev);
    bool done = false;

    /* wait to go high */
    int8_t res = GPIO_INPUT_PIN_LOW;
    while (res == GPIO_INPUT_PIN_LOW){
        res = check_echo_ultra(dev);
    }

    uint32_t count = 0;
    while(res == GPIO_INPUT_PIN_HIGH){
        res = check_echo_ultra(dev);
        __delay_cycles(CYCLES_TO_100US);
        count++;
    }

    /* convert 100 us to distance in cm */
    float dist_cm = CYCLES_TO_100US * US_TO_CM * (float)count;
    return (int16_t)dist_cm;

}

/* trigger ultra */
/* return -1 on fail */
void trig_ultra(ultra_t dev){
    switch(dev){
    case ULTRA_0:
        GPIO_setOutputHighOnPin(ULTRA_PORT,ULTRA_TRIG0);
        __delay_cycles(10);
        GPIO_setOutputLowOnPin(ULTRA_PORT,ULTRA_TRIG0);
        break;
    case ULTRA_1:
        GPIO_setOutputHighOnPin(ULTRA_PORT,ULTRA_TRIG1);
        __delay_cycles(10);
        GPIO_setOutputLowOnPin(ULTRA_PORT,ULTRA_TRIG1);
        break;
    case ULTRA_2:
        GPIO_setOutputHighOnPin(ULTRA_PORT,ULTRA_TRIG2);
        __delay_cycles(10);
        GPIO_setOutputLowOnPin(ULTRA_PORT,ULTRA_TRIG2);
        break;
    default:
        break;
    }
}

/* read echo pin  */
int8_t check_echo_ultra(ultra_t dev){
    switch(dev){
    case ULTRA_0:
        return (int8_t)GPIO_getInputPinValue(ULTRA_PORT,ULTRA_ECHO0);
    case ULTRA_1:
        return (int8_t)GPIO_getInputPinValue(ULTRA_PORT,ULTRA_ECHO1);
    case ULTRA_2:
        return (int8_t)GPIO_getInputPinValue(ULTRA_PORT,ULTRA_ECHO2);
    default:
        return -1;
    }
}

/*************************** lcd code *******************************/

void delayUS(uint32_t usec){

    usec >> 3;
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
