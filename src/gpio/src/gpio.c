/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Setup GPIOs for the Launchpad board
*
* Copyright (c) 2012-2013 theJPster (www.thejpster.org.uk)
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
#include "gpio/gpio_interrupts.h"

/**************************************************
* Defines
***************************************************/

/* None */

/**************************************************
* Data Types
**************************************************/

typedef struct gpio_interrupt_list_t
{
    gpio_interrupt_handler_t handler_fn;
    gpio_io_pin_t pin;
    uint32_t n_context;
    void *p_context;
} gpio_interrupt_list_t;

typedef struct gpio_registers_t
{
    reg_t DATA[255]; /* Data - offset sets pin mask */
    reg_t DATA_R; /* Data register - sets all pins */
    reg_t DIR_R; /* Direction */
    reg_t IS_R; /* Interrupt Sense */
    reg_t IBE_R; /* Interrupt Both Edges */
    reg_t IEV_R; /* Interrupt Event */
    reg_t IM_R; /* Interrupt Mask */
    const reg_t RIS_R; /* Raw Interrupt Status */
    const reg_t MIS_R; /* Masked Interrupt Status */
    reg_t ICR_R; /* Interrupt Clear */
    reg_t AFSEL_R; /* Alternate function Select */
    const reg_t _padding[55];
    reg_t DR2R_R; /* 2mA drive select */
    reg_t DR4R_R; /* 4mA drive select */
    reg_t DR8R_R; /* 8mA drive select */
    reg_t ODR_R; /* Open-drain select */
    reg_t PUR_R; /* Pull-up select */
    reg_t PDR_R; /* Pull-down select */
    reg_t SLR_R; /* Slew-rate control */
    reg_t DEN_R; /* Digital enable */
    reg_t LOCK_R; /* Lock */
    reg_t CR_R; /* Commit */
    reg_t AMSEL_R; /* Analog mode select */
    reg_t PCTL_R; /* Port Control */
    reg_t ADCCTL_R; /* ADC Control */
    reg_t DMACTL_R; /* DMA Control */
    const reg_t _padding2[678];
    const reg_t PeriphID4_R; /* Peripheral ID 4 */
    const reg_t PeriphID5_R; /* Peripheral ID 5 */
    const reg_t PeriphID6_R; /* Peripheral ID 6 */
    const reg_t PeriphID7_R; /* Peripheral ID 7 */
    const reg_t PeriphID0_R; /* Peripheral ID 0 */
    const reg_t PeriphID1_R; /* Peripheral ID 1 */
    const reg_t PeriphID2_R; /* Peripheral ID 2 */
    const reg_t PeriphID3_R; /* Peripheral ID 3 */
    const reg_t PCellD0_R; /* PrimeCell ID 0 */
    const reg_t PCellD1_R; /* PrimeCell ID 1 */
    const reg_t PCellD2_R; /* PrimeCell ID 2 */
    const reg_t PCellD3_R; /* PrimeCell ID 3 */
} gpio_registers_t;

/**************************************************
* Function Prototypes
**************************************************/

static void enable_gpio_module(gpio_port_t port);
static void gpio_interrupt(gpio_port_t port);


/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Private Data
**************************************************/

static gpio_interrupt_list_t interrupt_handlers[GPIO_MAX_INTERRUPT_HANDLERS];

static gpio_registers_t *const register_map[GPIO_NUM_PORTS] =
{
    (gpio_registers_t *) GPIO_PORTA_DATA_BITS_R,
    (gpio_registers_t *) GPIO_PORTB_DATA_BITS_R,
    (gpio_registers_t *) GPIO_PORTC_DATA_BITS_R,
    (gpio_registers_t *) GPIO_PORTD_DATA_BITS_R,
    (gpio_registers_t *) GPIO_PORTE_DATA_BITS_R,
    (gpio_registers_t *) GPIO_PORTF_DATA_BITS_R
};

/* See table 2-9 in [1] */
static const int gpio_int_map[GPIO_NUM_PORTS] =
{
    0, // GPIO PORT A
    1, // GPIO PORT B
    2, // GPIO PORT C
    3, // GPIO PORT D
    4, // GPIO PORT E
    30, // GPIO PORT F
};

/**************************************************
* Public Functions
***************************************************/

void gpio_enable_peripherals(void)
{
    gpio_enable_buttons();
    gpio_enable_leds();
    gpio_enable_uart(UART_ID_0);
}

void gpio_enable_buttons(void)
{
    // enable digital for button pins
    gpio_make_input(BUTTON_ONE);
    gpio_make_input(BUTTON_TWO);

    // Enable weak pullups
    register_map[GPIO_GET_PORT(BUTTON_ONE)]->DR2R_R |= GPIO_GET_PIN(BUTTON_ONE) | GPIO_GET_PIN(BUTTON_TWO);
    register_map[GPIO_GET_PORT(BUTTON_ONE)]->PUR_R |= GPIO_GET_PIN(BUTTON_ONE) | GPIO_GET_PIN(BUTTON_TWO);
}

void gpio_enable_leds(void)
{
    // set LED pins as outputs (rest are inputs)
    gpio_make_output(LED_RED, 0);
    gpio_make_output(LED_BLUE, 0);
    gpio_make_output(LED_GREEN, 0);
}

void gpio_enable_uart(uart_id_t uart_id)
{
    /* Enable clock and alt function for GPIO see [1] p303 / p584 */
    switch (uart_id)
    {
    case UART_ID_0: /* on PA0/PA1 */
        enable_gpio_module(GPIO_PORT_A);
        register_map[GPIO_PORT_A]->AFSEL_R |= (1 << 1) | (1 << 0);
        register_map[GPIO_PORT_A]->DEN_R |= (1 << 1) | (1 << 0);
        register_map[GPIO_PORT_A]->PCTL_R &= ~(GPIO_PCTL_PA0_M | GPIO_PCTL_PA1_M);
        register_map[GPIO_PORT_A]->PCTL_R |= GPIO_PCTL_PA0_U0RX | GPIO_PCTL_PA1_U0TX;
        break;
    case UART_ID_1: /* PB0/1 */
        enable_gpio_module(GPIO_PORT_B);
        register_map[GPIO_PORT_B]->AFSEL_R |= (1 << 1) | (1 << 0);
        register_map[GPIO_PORT_B]->DEN_R |= (1 << 1) | (1 << 0);
        register_map[GPIO_PORT_B]->PCTL_R &= ~(GPIO_PCTL_PB0_M | GPIO_PCTL_PB1_M);
        register_map[GPIO_PORT_B]->PCTL_R |= GPIO_PCTL_PB0_U1RX | GPIO_PCTL_PB1_U1TX;
        break;
    case UART_ID_2: /* On PD6/7 */
        /* This requires GPIO unlocking as PD7 is an NMI pin */
        enable_gpio_module(GPIO_PORT_D);
        register_map[GPIO_PORT_D]->LOCK_R = GPIO_LOCK_KEY; /* Unlock CR  */
        register_map[GPIO_PORT_D]->CR_R |= (1 << 7); /* Allow PD7 to be changed */
        register_map[GPIO_PORT_D]->LOCK_R = 0; /* Lock CR again */
        register_map[GPIO_PORT_D]->AFSEL_R |= (1 << 6) | (1 << 7);
        register_map[GPIO_PORT_D]->DEN_R |= (1 << 6) | (1 << 7);
        register_map[GPIO_PORT_D]->PCTL_R &= ~(GPIO_PCTL_PD6_M | GPIO_PCTL_PD7_M);
        register_map[GPIO_PORT_D]->PCTL_R |= GPIO_PCTL_PD6_U2RX | GPIO_PCTL_PD7_U2TX;
        break;
    case UART_ID_3: /* on PC6/PC7 */
        enable_gpio_module(GPIO_PORT_C);
        register_map[GPIO_PORT_C]->AFSEL_R |= (1 << 7) | (1 << 6);
        register_map[GPIO_PORT_C]->DEN_R |= (1 << 7) | (1 << 6);
        register_map[GPIO_PORT_C]->PCTL_R &= ~(GPIO_PCTL_PC6_M | GPIO_PCTL_PC7_M);
        register_map[GPIO_PORT_C]->PCTL_R |= GPIO_PCTL_PC6_U3RX | GPIO_PCTL_PC7_U3TX;
        break;
    case UART_ID_4: /* on PC4/PC5 */
        enable_gpio_module(GPIO_PORT_C);
        register_map[GPIO_PORT_C]->AFSEL_R |= (1 << 5) | (1 << 4);
        register_map[GPIO_PORT_C]->DEN_R |= (1 << 5) | (1 << 4);
        register_map[GPIO_PORT_C]->PCTL_R &= ~(GPIO_PCTL_PC4_M | GPIO_PCTL_PC5_M);
        register_map[GPIO_PORT_C]->PCTL_R |= GPIO_PCTL_PC4_U4RX | GPIO_PCTL_PC5_U4TX;
        break;
    case UART_ID_5: /* On PE4/PE5 */
        enable_gpio_module(GPIO_PORT_E);
        register_map[GPIO_PORT_E]->AFSEL_R |= (1 << 5) | (1 << 4);
        register_map[GPIO_PORT_E]->DEN_R |= (1 << 5) | (1 << 4);
        register_map[GPIO_PORT_E]->PCTL_R &= ~(GPIO_PCTL_PE4_M | GPIO_PCTL_PE5_M);
        register_map[GPIO_PORT_E]->PCTL_R |= GPIO_PCTL_PE4_U5RX | GPIO_PCTL_PE5_U5TX;
        break;
    case UART_ID_6: /* On PD4/PD5 */
        enable_gpio_module(GPIO_PORT_D);
        register_map[GPIO_PORT_D]->AFSEL_R |= (1 << 5) | (1 << 4);
        register_map[GPIO_PORT_D]->DEN_R |= (1 << 5) | (1 << 4);
        register_map[GPIO_PORT_D]->PCTL_R &= ~(GPIO_PCTL_PD4_M | GPIO_PCTL_PD5_M);
        register_map[GPIO_PORT_D]->PCTL_R |= GPIO_PCTL_PD4_U6RX | GPIO_PCTL_PD5_U6TX;
        break;
    case UART_ID_7: /* On PE0/1 */
        enable_gpio_module(GPIO_PORT_E);
        register_map[GPIO_PORT_E]->AFSEL_R |= (1 << 1) | (1 << 0);
        register_map[GPIO_PORT_E]->DEN_R |= (1 << 1) | (1 << 0);
        register_map[GPIO_PORT_E]->PCTL_R &= ~(GPIO_PCTL_PE0_M | GPIO_PCTL_PE1_M);
        register_map[GPIO_PORT_E]->PCTL_R |= GPIO_PCTL_PE0_U7RX | GPIO_PCTL_PE1_U7TX;
        break;
    default:
        break;
    }

}

void gpio_flash_error(gpio_io_pin_t pin_a, gpio_io_pin_t pin_b, unsigned int delay)
{
    while (1)
    {
        gpio_set_output(pin_a, 1);
        gpio_set_output(pin_b, 0);
        delay_ms(delay);
        gpio_set_output(pin_a, 0);
        gpio_set_output(pin_b, 1);
        delay_ms(delay);
    }
}

/*
 * Set pin as output (low or high)
 */
void gpio_make_output(gpio_io_pin_t pin, int level)
{
    gpio_port_t port = GPIO_GET_PORT(pin);
    reg_t mask = GPIO_GET_PIN(pin);
    enable_gpio_module(port);
    gpio_force_gpio(pin);
    register_map[port]->DATA[mask] = level ? 0xFF : 0x00;
    register_map[port]->DIR_R |= mask;
    register_map[port]->DEN_R |= mask;
}

void gpio_force_gpio(gpio_io_pin_t pin)
{
    gpio_port_t port = GPIO_GET_PORT(pin);
    reg_t mask = GPIO_GET_PIN(pin);
    register_map[port]->AFSEL_R &= ~mask;
    switch(mask)
    {
    case 0x01:
        register_map[port]->PCTL_R &= ~(0x07 << 0);
        break;
    case 0x02:
        register_map[port]->PCTL_R &= ~(0x07 << 4);
        break;
    case 0x04:
        register_map[port]->PCTL_R &= ~(0x07 << 8);
        break;
    case 0x08:
        register_map[port]->PCTL_R &= ~(0x07 << 12);
        break;
    case 0x10:
        register_map[port]->PCTL_R &= ~(0x07 << 16);
        break;
    case 0x20:
        register_map[port]->PCTL_R &= ~(0x07 << 20);
        break;
    case 0x40:
        register_map[port]->PCTL_R &= ~(0x07 << 24);
        break;
    case 0x80:
        register_map[port]->PCTL_R &= ~(0x07 << 28);
        break;
    }
}

/*
 * Set pin as input
 */
void gpio_make_input(gpio_io_pin_t pin)
{
    gpio_port_t port = GPIO_GET_PORT(pin);
    reg_t mask = GPIO_GET_PIN(pin);
    enable_gpio_module(port);
    if (pin == GPIO_MAKE_IO_PIN(GPIO_PORT_F, 0))
    {
        /* The GPIO for button one is multiplexed with NMI so we
         * have to 'unlock' it before we can use it
         */
        register_map[GPIO_PORT_F]->LOCK_R = GPIO_LOCK_KEY; /* Unlock CR  */
        register_map[GPIO_PORT_F]->CR_R |= GPIO_GET_PIN(pin); /* Allow F0 to be changed */
        register_map[GPIO_PORT_F]->LOCK_R = 0; /* Lock CR again */
    }
    register_map[port]->DEN_R |= mask;
    register_map[port]->DIR_R &= ~mask;
    gpio_force_gpio(pin);
}

/*
 * If a pin is already an output, set the level
 */
void gpio_set_output(gpio_io_pin_t pin, int level)
{
    gpio_port_t port = GPIO_GET_PORT(pin);
    reg_t mask = GPIO_GET_PIN(pin);
    register_map[port]->DATA[mask] = level ? 0xFF : 00;
}

/*
 * Set multiple output pins on a port.
 */
void gpio_set_outputs(gpio_port_t port, uint8_t outputs, uint8_t mask)
{
    register_map[port]->DATA[mask] = outputs;
}


/*
 * If a pin is already an input, read the level. 0 for low, 1 for high.
 */
uint8_t gpio_read_input(gpio_io_pin_t pin)
{
    uint8_t result = 0;
    gpio_port_t port = GPIO_GET_PORT(pin);
    reg_t mask = GPIO_GET_PIN(pin);
    result = (register_map[port]->DATA[mask]) ? 1 : 0;
    return result;
}

/*
 * If a many pins on a port are already inputs, read the levels. 0 for low, 1 for high.
 */
uint8_t gpio_read_inputs(gpio_port_t port, uint8_t mask)
{
    return (uint8_t) register_map[port]->DATA[mask];
}

/*
 * Register an interrupt handler in an empty slot.
 */
void gpio_register_handler(
    gpio_io_pin_t pin,
    gpio_interrupt_mode_t mode,
    gpio_interrupt_handler_t handler_fn,
    void *p_context,
    uint32_t n_context)
{
    gpio_port_t port = GPIO_GET_PORT(pin);
    reg_t mask = GPIO_GET_PIN(pin);
    gpio_registers_t * const p_gpio = register_map[port];

    for (unsigned int i = 0; i < NUMELTS(interrupt_handlers); i++)
    {
        gpio_interrupt_list_t *const p = &interrupt_handlers[i];
        if (!p->handler_fn)
        {
            /* Enable the GPIO port interrupt in the NVIC */
            enable_interrupt(gpio_int_map[port]);
            p->handler_fn = handler_fn;
            p->pin = pin;
            p->p_context = p_context;
            p->n_context = n_context;
            /* Detect edges */
            CLEAR_BITS(p_gpio->IS_R, mask);
            switch(mode)
            {
            case GPIO_INTERRUPT_MODE_RISING:
                CLEAR_BITS(p_gpio->IBE_R, mask);
                SET_BITS(p_gpio->IEV_R, mask);
                break;
            case GPIO_INTERRUPT_MODE_FALLING:
                CLEAR_BITS(p_gpio->IBE_R, mask);
                CLEAR_BITS(p_gpio->IEV_R, mask);
                break;
            case GPIO_INTERRUPT_MODE_BOTH:
                SET_BITS(p_gpio->IBE_R, mask);
                break;
            default:
                gpio_flash_error(LED_RED, LED_GREEN, 500);
                break;
            }
            /* Enable the pin interrupt in the mask register */
            SET_BITS(p_gpio->IM_R, mask);
            break;
        }
    }
}

void gpioA_interrupt(void)
{
    gpio_interrupt(GPIO_PORT_A);
}

void gpioB_interrupt(void)
{
    gpio_interrupt(GPIO_PORT_B);
}

void gpioC_interrupt(void)
{
    gpio_interrupt(GPIO_PORT_C);
}

void gpioD_interrupt(void)
{
    gpio_interrupt(GPIO_PORT_D);
}

void gpioE_interrupt(void)
{
    gpio_interrupt(GPIO_PORT_E);
}

void gpioF_interrupt(void)
{
    gpio_interrupt(GPIO_PORT_F);
}

/**************************************************
* Private Functions
***************************************************/

static void enable_gpio_module(gpio_port_t port)
{
    reg_t mask = 1 << port;
    if ((SYSCTL_RCGCGPIO_R & mask) == 0)
    {
        SYSCTL_RCGCGPIO_R |= mask;
        busy_sleep(10);
    }
}


/*
 * Cycle through all the pins in this port with an interrupt flagged and call
 * any registered handlers.
 */
static void gpio_interrupt(gpio_port_t port)
{
    /* Capture the current enabled & active interrupts */
    uint8_t active_interrupts = register_map[port]->MIS_R;

    /* Clear only those interrupts we captured */
    register_map[port]->ICR_R = active_interrupts;

    for (unsigned int pin = 0; pin < 8; pin++)
    {
        /* Is there an interrupt on this pin? */
        if (active_interrupts & (1 << pin))
        {
            /* Now cycle through all the registered handlers */
            for (unsigned int i = 0; i < NUMELTS(interrupt_handlers); i++)
            {
                const gpio_interrupt_list_t *const p = &interrupt_handlers[i];
                if (p->handler_fn && (p->pin == GPIO_MAKE_IO_PIN(port, pin)))
                {
                    /* Call registered handler */
                    p->handler_fn(p->pin, p->p_context, p->n_context);
                }
            }
        }
    }
}

/**************************************************
* End of file
***************************************************/

