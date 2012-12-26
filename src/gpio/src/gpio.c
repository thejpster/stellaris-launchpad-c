/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Setup GPIOs for the Launchpad board
*
* Copyright (c) 2012 theJPster (www.thejpster.org.uk)
*
*****************************************************/

/**************************************************
* Includes
***************************************************/

#include "misc/misc.h"

#include "gpio/gpio.h"

/**************************************************
* Defines
***************************************************/

/* None */

/**************************************************
* Function Prototypes
**************************************************/

 /* None */

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Private Data
**************************************************/

/* None */

/**************************************************
* Public Functions
***************************************************/

void enable_peripherals(void)
{
    enable_buttons();
    enable_leds();
    enable_uart(UART_ID_0);
}

void enable_buttons(void)
{
    // enable PORT F GPIO peripheral using
    // run-time clock gating control register 2
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    // The datasheet says wait after enabling GPIO
    busy_sleep(10);

    /* The GPIO for button one is multiplexed with NMI so we
     * have to 'unlock' it before we can use it
     */
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; /* Unlock CR  */
    GPIO_PORTF_CR_R |= BUTTON_ONE; /* Allow F0 to be changed */
    GPIO_PORTF_LOCK_R = 0; /* Lock CR again */

    // enable digital for button pins
    GPIO_PORTF_DEN_R |= BUTTON_ONE | BUTTON_TWO;

    // set pins to inputs
    GPIO_PORTF_DIR_R &= (BUTTON_ONE | BUTTON_TWO);

    // Enable weak pullups
    GPIO_PORTF_DR2R_R |= BUTTON_ONE | BUTTON_TWO;
    GPIO_PORTF_PUR_R |= BUTTON_ONE | BUTTON_TWO;
}

void enable_leds(void)
{
    // enable PORT F GPIO peripheral using
    // run-time clock gating control register 2
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    // The datasheet says wait after enabling GPIO
    busy_sleep(10);

    // enable digital for LED PORT F pins
    GPIO_PORTF_DEN_R |= LED_RED | LED_BLUE | LED_GREEN;
    // set LED PORT F pins as outputs (rest are inputs)
    GPIO_PORTF_DIR_R |= LED_RED | LED_BLUE | LED_GREEN;
}

void enable_uart(uart_id_t uart_id)
{
    /* Enable clock and alt function for GPIO see [1] p303 / p584 */
    switch(uart_id)
    {
    case UART_ID_0: /* on PA0/PA1 */
        SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R0;
        GPIO_PORTA_AFSEL_R |= (1 << 1) | (1 << 0);
        GPIO_PORTA_DEN_R |= (1 << 1) | (1 << 0);
        GPIO_PORTA_PCTL_R &= ~(GPIO_PCTL_PA0_M | GPIO_PCTL_PA1_M);
        GPIO_PORTA_PCTL_R |= GPIO_PCTL_PA0_U0RX | GPIO_PCTL_PA1_U0TX;  
        break;
    case UART_ID_1: /* PB0/1 */
        SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
        GPIO_PORTB_AFSEL_R |= (1 << 1) | (1 << 0);
        GPIO_PORTA_DEN_R |= (1 << 1) | (1 << 0);
        GPIO_PORTB_PCTL_R &= ~(GPIO_PCTL_PB0_M | GPIO_PCTL_PB1_M);
        GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB0_U1RX | GPIO_PCTL_PB1_U1TX;  
        break;
    case UART_ID_2: /* On PD6/7 */
        /* This requires GPIO unlocking as PD7 is an NMI pin */
        SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3;
        GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY; /* Unlock CR  */
        GPIO_PORTD_CR_R |= (1 << 7); /* Allow PD7 to be changed */
        GPIO_PORTD_LOCK_R = 0; /* Lock CR again */
        GPIO_PORTD_AFSEL_R |= (1 << 6) | (1 << 7);
        GPIO_PORTA_DEN_R |= (1 << 6) | (1 << 7);
        GPIO_PORTD_PCTL_R &= ~(GPIO_PCTL_PD6_M | GPIO_PCTL_PD7_M);
        GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD6_U2RX | GPIO_PCTL_PD7_U2TX;  
        break;
    case UART_ID_3: /* on PC6/PC7 */
        SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;
        GPIO_PORTC_AFSEL_R |= (1 << 7) | (1 << 6);
        GPIO_PORTA_DEN_R |= (1 << 7) | (1 << 6);
        GPIO_PORTC_PCTL_R &= ~(GPIO_PCTL_PC6_M | GPIO_PCTL_PC7_M);
        GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC6_U3RX | GPIO_PCTL_PC7_U3TX;  
        break;
    case UART_ID_4: /* on PC4/PC5 */
        SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R2;
        GPIO_PORTC_AFSEL_R |= (1 << 5) | (1 << 4);
        GPIO_PORTA_DEN_R |= (1 << 5) | (1 << 4);
        GPIO_PORTC_PCTL_R &= ~(GPIO_PCTL_PC4_M | GPIO_PCTL_PC5_M);
        GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC4_U4RX | GPIO_PCTL_PC5_U4TX;  
        break;
    case UART_ID_5: /* On PE4/PE5 */
        SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
        GPIO_PORTE_AFSEL_R |= (1 << 5) | (1 << 4);
        GPIO_PORTA_DEN_R |= (1 << 5) | (1 << 4);
        GPIO_PORTE_PCTL_R &= ~(GPIO_PCTL_PE4_M | GPIO_PCTL_PE5_M);
        GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE4_U5RX | GPIO_PCTL_PE5_U5TX;  
        break;
    case UART_ID_6: /* On PD4/PD5 */
        SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R3;
        GPIO_PORTD_AFSEL_R |= (1 << 5) | (1 << 4);
        GPIO_PORTA_DEN_R |= (1 << 5) | (1 << 4);
        GPIO_PORTD_PCTL_R &= ~(GPIO_PCTL_PD4_M | GPIO_PCTL_PD5_M);
        GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD4_U6RX | GPIO_PCTL_PD5_U6TX;  
        break;
    case UART_ID_7: /* On PE0/1 */
        SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;
        GPIO_PORTE_AFSEL_R |= (1 << 1) | (1 << 0);
        GPIO_PORTA_DEN_R |= (1 << 1) | (1 << 0);
        GPIO_PORTE_PCTL_R &= ~(GPIO_PCTL_PE0_M | GPIO_PCTL_PE1_M);
        GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE0_U7RX | GPIO_PCTL_PE1_U7TX;  
        break;
    default:
        break;
    }

}

void flash_error(unsigned int pin_a, unsigned int pin_b, unsigned int delay)
{
    while(1)
    {
        GPIO_PORTF_DATA_R = pin_a;
        busy_sleep(delay);
        GPIO_PORTF_DATA_R = pin_b;
        busy_sleep(delay);
    }
}

/**************************************************
* Private Functions
***************************************************/

/**************************************************
* End of file
***************************************************/

