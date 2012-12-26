/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Setup GPIOs for the Launchpad board
*
* Copyright (c) 2012 theJPster (www.thejpster.org.uk)
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

#include "uart/uart.h"

/**************************************************
* Public Defines
***************************************************/

/* The LEDs on the Launchpad board are pins GPIO_F1..3 */
#define LED_RED (1<<1)
#define LED_BLUE (1<<2)
#define LED_GREEN (1<<3)
/* The buttons are pins GPIO_F0 and GPIO_F4*/
#define BUTTON_ONE (1<<0)
#define BUTTON_TWO (1<<4)

/**************************************************
* Public Data Types
**************************************************/

/* None */

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
extern void enable_peripherals(void);

/**
 * Enable the two buttons on the launchpad board.
 */
extern void enable_buttons(void);

/**
 * Enable the RGB LED on the launchpad board.
 */
extern void enable_leds(void);

/**
 * Set the GPIO pins so that UART0 can control GPIOA[0:1].
 * You still need to call uart_init().
 */
extern void enable_uart(uart_id_t uart_id);

/*
 * A useful means of conveying errors when you don't have a 
 * working UART.
 */
extern void flash_error(unsigned int pin_a, unsigned int pin_b, unsigned int delay);


#ifdef __cplusplus
}
#endif

#endif /* ndef GPIO_GPIO_H */

/**************************************************
* End of file
***************************************************/

