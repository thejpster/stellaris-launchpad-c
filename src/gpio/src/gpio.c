/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Setup GPIOs for the Launchpad board
*
* Copyright (c) 2012 theJPster (www.thejpster.org.uk)
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
* DEALINGS IN THE SOFTWARE.
*
*****************************************************/

/**************************************************
* Includes
***************************************************/

#include <stdint.h>

#include "misc/misc.h"
#include "gpio/gpio.h"

/**************************************************
* Defines
***************************************************/

/* None */

/**************************************************
* Function Prototypes
**************************************************/

static void enable_gpio_module(gpio_port_t port);

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
    // enable digital for button pins
    gpio_make_input(BUTTON_ONE);
    gpio_make_input(BUTTON_TWO);

    // Enable weak pullups
    GPIO_PORTF_DR2R_R |= GPIO_GET_PIN(BUTTON_ONE) | GPIO_GET_PIN(BUTTON_TWO);
    GPIO_PORTF_PUR_R |= GPIO_GET_PIN(BUTTON_ONE) | GPIO_GET_PIN(BUTTON_TWO);
}

void enable_leds(void)
{
    // set LED pins as outputs (rest are inputs)
    gpio_make_output(LED_RED, 0);
    gpio_make_output(LED_BLUE, 0);
    gpio_make_output(LED_GREEN, 0);
}

void enable_uart(uart_id_t uart_id)
{
    /* Enable clock and alt function for GPIO see [1] p303 / p584 */
    switch (uart_id)
    {
    case UART_ID_0: /* on PA0/PA1 */
        enable_gpio_module(GPIO_PORT_A);
        GPIO_PORTA_AFSEL_R |= (1 << 1) | (1 << 0);
        GPIO_PORTA_DEN_R |= (1 << 1) | (1 << 0);
        GPIO_PORTA_PCTL_R &= ~(GPIO_PCTL_PA0_M | GPIO_PCTL_PA1_M);
        GPIO_PORTA_PCTL_R |= GPIO_PCTL_PA0_U0RX | GPIO_PCTL_PA1_U0TX;
        break;
    case UART_ID_1: /* PB0/1 */
        enable_gpio_module(GPIO_PORT_B);
        GPIO_PORTB_AFSEL_R |= (1 << 1) | (1 << 0);
        GPIO_PORTA_DEN_R |= (1 << 1) | (1 << 0);
        GPIO_PORTB_PCTL_R &= ~(GPIO_PCTL_PB0_M | GPIO_PCTL_PB1_M);
        GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB0_U1RX | GPIO_PCTL_PB1_U1TX;
        break;
    case UART_ID_2: /* On PD6/7 */
        /* This requires GPIO unlocking as PD7 is an NMI pin */
        enable_gpio_module(GPIO_PORT_D);
        GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY; /* Unlock CR  */
        GPIO_PORTD_CR_R |= (1 << 7); /* Allow PD7 to be changed */
        GPIO_PORTD_LOCK_R = 0; /* Lock CR again */
        GPIO_PORTD_AFSEL_R |= (1 << 6) | (1 << 7);
        GPIO_PORTA_DEN_R |= (1 << 6) | (1 << 7);
        GPIO_PORTD_PCTL_R &= ~(GPIO_PCTL_PD6_M | GPIO_PCTL_PD7_M);
        GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD6_U2RX | GPIO_PCTL_PD7_U2TX;
        break;
    case UART_ID_3: /* on PC6/PC7 */
        enable_gpio_module(GPIO_PORT_C);
        GPIO_PORTC_AFSEL_R |= (1 << 7) | (1 << 6);
        GPIO_PORTA_DEN_R |= (1 << 7) | (1 << 6);
        GPIO_PORTC_PCTL_R &= ~(GPIO_PCTL_PC6_M | GPIO_PCTL_PC7_M);
        GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC6_U3RX | GPIO_PCTL_PC7_U3TX;
        break;
    case UART_ID_4: /* on PC4/PC5 */
        enable_gpio_module(GPIO_PORT_C);
        GPIO_PORTC_AFSEL_R |= (1 << 5) | (1 << 4);
        GPIO_PORTA_DEN_R |= (1 << 5) | (1 << 4);
        GPIO_PORTC_PCTL_R &= ~(GPIO_PCTL_PC4_M | GPIO_PCTL_PC5_M);
        GPIO_PORTC_PCTL_R |= GPIO_PCTL_PC4_U4RX | GPIO_PCTL_PC5_U4TX;
        break;
    case UART_ID_5: /* On PE4/PE5 */
        enable_gpio_module(GPIO_PORT_E);
        GPIO_PORTE_AFSEL_R |= (1 << 5) | (1 << 4);
        GPIO_PORTA_DEN_R |= (1 << 5) | (1 << 4);
        GPIO_PORTE_PCTL_R &= ~(GPIO_PCTL_PE4_M | GPIO_PCTL_PE5_M);
        GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE4_U5RX | GPIO_PCTL_PE5_U5TX;
        break;
    case UART_ID_6: /* On PD4/PD5 */
        enable_gpio_module(GPIO_PORT_D);
        GPIO_PORTD_AFSEL_R |= (1 << 5) | (1 << 4);
        GPIO_PORTA_DEN_R |= (1 << 5) | (1 << 4);
        GPIO_PORTD_PCTL_R &= ~(GPIO_PCTL_PD4_M | GPIO_PCTL_PD5_M);
        GPIO_PORTD_PCTL_R |= GPIO_PCTL_PD4_U6RX | GPIO_PCTL_PD5_U6TX;
        break;
    case UART_ID_7: /* On PE0/1 */
        enable_gpio_module(GPIO_PORT_E);
        GPIO_PORTE_AFSEL_R |= (1 << 1) | (1 << 0);
        GPIO_PORTA_DEN_R |= (1 << 1) | (1 << 0);
        GPIO_PORTE_PCTL_R &= ~(GPIO_PCTL_PE0_M | GPIO_PCTL_PE1_M);
        GPIO_PORTE_PCTL_R |= GPIO_PCTL_PE0_U7RX | GPIO_PCTL_PE1_U7TX;
        break;
    default:
        break;
    }

}

void flash_error(gpio_io_pin_t pin_a, gpio_io_pin_t pin_b, unsigned int delay)
{
    while (1)
    {
        gpio_set_output(pin_a, 1);
        gpio_set_output(pin_b, 0);
        busy_sleep(delay);
        gpio_set_output(pin_a, 0);
        gpio_set_output(pin_b, 1);
        busy_sleep(delay);
    }
}

/*
 * Set pin as output (low or high)
 */
void gpio_make_output(gpio_io_pin_t pin, int level)
{
    gpio_port_t port = GPIO_GET_PORT(pin);
    unsigned long mask = GPIO_GET_PIN(pin);
    enable_gpio_module(port);
    switch (port)
    {
    case GPIO_PORT_A:
        GPIO_PORTA_DEN_R |= mask;
        if (level)
        {
            GPIO_PORTA_DATA_R |= mask;
        }
        else
        {
            GPIO_PORTA_DATA_R &= ~mask;
        }
        GPIO_PORTA_DIR_R |= mask;
        break;
    case GPIO_PORT_B:
        GPIO_PORTB_DEN_R |= mask;
        if (level)
        {
            GPIO_PORTB_DATA_R |= mask;
        }
        else
        {
            GPIO_PORTB_DATA_R &= ~mask;
        }
        GPIO_PORTB_DIR_R |= mask;
        break;
    case GPIO_PORT_C:
        GPIO_PORTC_DEN_R |= mask;
        if (level)
        {
            GPIO_PORTC_DATA_R |= mask;
        }
        else
        {
            GPIO_PORTC_DATA_R &= ~mask;
        }
        GPIO_PORTC_DIR_R |= mask;
        break;
    case GPIO_PORT_D:
        GPIO_PORTD_DEN_R |= mask;
        if (level)
        {
            GPIO_PORTD_DATA_R |= mask;
        }
        else
        {
            GPIO_PORTD_DATA_R &= ~mask;
        }
        GPIO_PORTD_DIR_R |= mask;
        break;
    case GPIO_PORT_E:
        GPIO_PORTE_DEN_R |= mask;
        if (level)
        {
            GPIO_PORTE_DATA_R |= mask;
        }
        else
        {
            GPIO_PORTE_DATA_R &= ~mask;
        }
        GPIO_PORTE_DIR_R |= mask;
        break;
    case GPIO_PORT_F:
        GPIO_PORTF_DEN_R |= mask;
        if (level)
        {
            GPIO_PORTF_DATA_R |= mask;
        }
        else
        {
            GPIO_PORTF_DATA_R &= ~mask;
        }
        GPIO_PORTF_DIR_R |= mask;
        break;
    }
}

/*
 * Set pin as input
 */
void gpio_make_input(gpio_io_pin_t pin)
{
    gpio_port_t port = GPIO_GET_PORT(pin);
    unsigned long mask = GPIO_GET_PIN(pin);
    enable_gpio_module(port);
    switch (port)
    {
    case GPIO_PORT_A:
        GPIO_PORTA_DEN_R |= mask;
        GPIO_PORTA_DIR_R &= ~mask;
        break;
    case GPIO_PORT_B:
        GPIO_PORTB_DEN_R |= mask;
        GPIO_PORTB_DIR_R &= ~mask;
        break;
    case GPIO_PORT_C:
        GPIO_PORTC_DEN_R |= mask;
        GPIO_PORTC_DIR_R &= ~mask;
        break;
    case GPIO_PORT_D:
        GPIO_PORTD_DEN_R |= mask;
        GPIO_PORTD_DIR_R &= ~mask;
        break;
    case GPIO_PORT_E:
        GPIO_PORTE_DEN_R |= mask;
        GPIO_PORTE_DIR_R &= ~mask;
        break;
    case GPIO_PORT_F:
        /* The GPIO for button one is multiplexed with NMI so we
         * have to 'unlock' it before we can use it
         */
        if (GPIO_GET_PIN(pin) == 1)
        {
            GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; /* Unlock CR  */
            GPIO_PORTF_CR_R |= 1; /* Allow F0 to be changed */
            GPIO_PORTF_LOCK_R = 0; /* Lock CR again */
        }
        GPIO_PORTF_DEN_R |= mask;
        GPIO_PORTF_DIR_R &= ~mask;
        break;
    }
}

/*
 * If a pin is already an output, set the level
 */
void gpio_set_output(gpio_io_pin_t pin, int level)
{
    gpio_port_t port = GPIO_GET_PORT(pin);
    unsigned long mask = GPIO_GET_PIN(pin);
    enable_gpio_module(port);
    if (level)
    {
        switch (port)
        {
        case GPIO_PORT_A:
            GPIO_PORTA_DATA_R |= mask;
            break;
        case GPIO_PORT_B:
            GPIO_PORTB_DATA_R |= mask;
            break;
        case GPIO_PORT_C:
            GPIO_PORTC_DATA_R |= mask;
            break;
        case GPIO_PORT_D:
            GPIO_PORTD_DATA_R |= mask;
            break;
        case GPIO_PORT_E:
            GPIO_PORTE_DATA_R |= mask;
            break;
        case GPIO_PORT_F:
            GPIO_PORTF_DATA_R |= mask;
            break;
        }
    }
    else
    {
        switch (port)
        {
        case GPIO_PORT_A:
            GPIO_PORTA_DATA_R &= ~mask;
            break;
        case GPIO_PORT_B:
            GPIO_PORTB_DATA_R &= ~mask;
            break;
        case GPIO_PORT_C:
            GPIO_PORTC_DATA_R &= ~mask;
            break;
        case GPIO_PORT_D:
            GPIO_PORTD_DATA_R &= ~mask;
            break;
        case GPIO_PORT_E:
            GPIO_PORTE_DATA_R &= ~mask;
            break;
        case GPIO_PORT_F:
            GPIO_PORTF_DATA_R &= ~mask;
            break;
        }
    }
}

/*
 * If a pin is already an input, read the level. 0 for low, 1 for high.
 */
int gpio_read_input(gpio_io_pin_t pin)
{
    int result = 0;
    gpio_port_t port = GPIO_GET_PORT(pin);
    unsigned long mask = GPIO_GET_PIN(pin);
    switch (port)
    {
    case GPIO_PORT_A:
        result = (GPIO_PORTA_DATA_R & mask) ? 1 : 0;
        break;
    case GPIO_PORT_B:
        result = (GPIO_PORTB_DATA_R & mask) ? 1 : 0;
        break;
    case GPIO_PORT_C:
        result = (GPIO_PORTC_DATA_R & mask) ? 1 : 0;
        break;
    case GPIO_PORT_D:
        result = (GPIO_PORTD_DATA_R & mask) ? 1 : 0;
        break;
    case GPIO_PORT_E:
        result = (GPIO_PORTE_DATA_R & mask) ? 1 : 0;
        break;
    case GPIO_PORT_F:
        result = (GPIO_PORTF_DATA_R & mask) ? 1 : 0;
        break;
    }
    return result;
}

/**************************************************
* Private Functions
***************************************************/

static void enable_gpio_module(gpio_port_t port)
{
    unsigned long mask = 1 << port;
    if ((SYSCTL_RCGCGPIO_R & mask) == 0)
    {
        SYSCTL_RCGCGPIO_R |= mask;
        busy_sleep(10);
    }
}

/**************************************************
* End of file
***************************************************/

