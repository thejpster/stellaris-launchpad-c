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

#ifndef GPIO_GPIO_H
#define GPIO_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************
* Includes
***************************************************/

#include "util/util.h"
#include "drivers/uart/uart.h"

/**************************************************
* Public Defines
***************************************************/

/* Macros to pack a port and pin into one value */
#define GPIO_MAKE_IO_PIN(port, pin)  ( (gpio_io_pin_t) ( ((port) << 8U) | (1U<<(pin)) ) )
#define GPIO_GET_PORT(io_pin)  ((io_pin) >> 8U)
#define GPIO_GET_PIN(io_pin)  ((io_pin) & 0xFFU)

/* The LEDs on the Launchpad board are pins GPIO_F1..3 */
#define LED_RED   GPIO_MAKE_IO_PIN(GPIO_PORT_F, 1)
#define LED_BLUE  GPIO_MAKE_IO_PIN(GPIO_PORT_F, 2)
#define LED_GREEN GPIO_MAKE_IO_PIN(GPIO_PORT_F, 3)

/* The buttons are pins GPIO_F0 and GPIO_F4*/
#define BUTTON_ONE GPIO_MAKE_IO_PIN(GPIO_PORT_F, 0)
#define BUTTON_TWO GPIO_MAKE_IO_PIN(GPIO_PORT_F, 4)

#ifndef GPIO_MAX_INTERRUPT_HANDLERS
#define GPIO_MAX_INTERRUPT_HANDLERS 2
#endif

/**************************************************
* Public Data Types
**************************************************/

typedef enum gpio_port_t
{
    GPIO_PORT_A,
    GPIO_PORT_B,
    GPIO_PORT_C,
    GPIO_PORT_D,
    GPIO_PORT_E,
    GPIO_PORT_F,
} gpio_port_t;

#define GPIO_NUM_PORTS (GPIO_PORT_F + 1)

typedef int gpio_io_pin_t;

typedef enum gpio_interrupt_mode_t
{
    GPIO_INTERRUPT_MODE_RISING,
    GPIO_INTERRUPT_MODE_FALLING,
    GPIO_INTERRUPT_MODE_BOTH
} gpio_interrupt_mode_t;

/*
 * Functions matching this prototype can be registered and called when
 * timer interrupts fire.
 */
typedef void (*gpio_interrupt_handler_t)(gpio_io_pin_t pin, void* p_context, uint32_t n_context);

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Public Function Prototypes
***************************************************/

/**
 * Enable the LEDs, buttons and UART0 on the launchpad board.
 */
extern void gpio_enable_peripherals(void);

/**
 * Enable the two buttons on the launchpad board.
 */
extern void gpio_enable_buttons(void);

/**
 * Enable the RGB LED on the launchpad board.
 */
extern void gpio_enable_leds(void);

/**
 * Set the GPIO pins so that UART0 can control GPIOA[0:1].
 * You still need to call uart_init().
 */
extern void gpio_enable_uart(uart_id_t uart_id);

/*
 * A useful means of conveying errors when you don't have a 
 * working UART.
 */
extern void gpio_flash_error(gpio_io_pin_t pin_a, gpio_io_pin_t pin_b, unsigned int delay_ms);

/*
 * Set pin as output (low or high)
 */
extern void gpio_make_output(gpio_io_pin_t pin, int level);

/*
 * Set pin as input
 */
extern void gpio_make_input(gpio_io_pin_t pin);

/*
 * Set pin as input with a pullup.
 */
extern void gpio_make_input_pullup(gpio_io_pin_t pin);

/*
 * Remove any peripheral connected to this pin.
 */
void gpio_force_gpio(gpio_io_pin_t pin);

/*
 * If a pin is already an output, set the level
 */
extern void gpio_set_output(gpio_io_pin_t pin, int level);

/*
 * Set multiple output pins on a port.
 */
extern void gpio_set_outputs(gpio_port_t port, uint8_t outputs, uint8_t mask);

/* Faster versions for specific ports */
#define gpio_set_outputs_porta(outputs, mask) do { GPIO_PORTA_DATA_BITS_R[mask] = outputs; } while(0)
#define gpio_set_outputs_portb(outputs, mask) do { GPIO_PORTB_DATA_BITS_R[mask] = outputs; } while(0)
#define gpio_set_outputs_portc(outputs, mask) do { GPIO_PORTC_DATA_BITS_R[mask] = outputs; } while(0)
#define gpio_set_outputs_portd(outputs, mask) do { GPIO_PORTD_DATA_BITS_R[mask] = outputs; } while(0)
#define gpio_set_outputs_porte(outputs, mask) do { GPIO_PORTE_DATA_BITS_R[mask] = outputs; } while(0)
#define gpio_set_outputs_portf(outputs, mask) do { GPIO_PORTF_DATA_BITS_R[mask] = outputs; } while(0)

/*
 * If a pin is already an input, read the level. 0 for low, 1 for high.
 */
extern uint8_t gpio_read_input(gpio_io_pin_t pin);

/*
 * If a many pins on a port are already inputs, read the levels. 0 for low, 1 for high.
 */
extern uint8_t gpio_read_inputs(gpio_port_t port, uint8_t mask);

/*
 * Register an interrupt handler.
 */
void gpio_register_handler(
    gpio_io_pin_t pin,
    gpio_interrupt_mode_t mode,
    gpio_interrupt_handler_t handler_fn,
    void* p_context,
    uint32_t n_context);

#ifdef __cplusplus
}
#endif

#endif /* ndef GPIO_GPIO_H */

/**************************************************
* End of file
***************************************************/

