/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2012 theJPster (www.thejpster.org.uk)
*
*****************************************************/

/**************************************************
* Includes
***************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "misc/misc.h"
#include "uart/uart.h"
#include "gpio/gpio.h"

/**************************************************
* Defines
***************************************************/

/* So we can see the LEDs flashing. Given the arithmetic in the 
* busy_sleep loop, this is about 1 second */
#define DELAY (CLOCK_RATE / 16)

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

int main(void)
{
    /* Set system clock to 16MHz */
    set_clock();

    enable_peripherals();

    int res = uart_init(
        UART_ID_0,
        115200,
        UART_PARITY_NONE,
        UART_DATABITS_8,
        UART_STOPBITS_1,
        NULL
    );

    if (res != 0)
    {
        flash_error(LED_RED, LED_GREEN, CLOCK_RATE * 1);
    }

    printf("Hello %d!\n", 123);

    while (1)
    {
        GPIO_PORTF_DATA_R = 0;
        busy_sleep(DELAY);

        unsigned int buttons = GPIO_PORTF_DATA_R;
        if ((buttons & BUTTON_ONE) == 0)
        {
            /* Button one pressed as input is low */
            GPIO_PORTF_DATA_R = LED_BLUE;
            uart_write_str(UART_ID_0, "blue\n");
        }
        else if ((buttons & BUTTON_TWO) == 0)
        {
            /* Button two pressed as input is low */
            GPIO_PORTF_DATA_R = LED_RED;
            uart_write_str(UART_ID_0, "red\n");
        }
        else
        {
            /* Neither button pressed */
            GPIO_PORTF_DATA_R = LED_GREEN;
            uart_write_str(UART_ID_0, "green\n");
        }
        busy_sleep(DELAY);
    }

    /* Shouldn't get here */
    return 0;
}


/**************************************************
* Private Functions
***************************************************/

/* None */

/**************************************************
* End of file
***************************************************/

