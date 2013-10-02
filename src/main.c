/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2013 theJPster (www.thejpster.org.uk)
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "misc/misc.h"
#include "uart/uart.h"
#include "gpio/gpio.h"
#include "timers/timers.h"

/**************************************************
* Defines
***************************************************/

/* So we can see the LEDs flashing. Given our busy_sleep
* implementation, this is about 0.5 seconds */
#define DELAY (CLOCK_RATE / 32)

/**************************************************
* Data Types
**************************************************/

typedef enum button_uart_override_t
{
    BUTTON_UART_OVERRIDE_NONE,
    BUTTON_UART_OVERRIDE_ONE,
    BUTTON_UART_OVERRIDE_TWO
} button_uart_override_t;

/**************************************************
* Function Prototypes
**************************************************/

static void uart_chars_received(
    uart_id_t uart_id,
    const char *buffer,
    size_t buffer_size
);

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Private Data
**************************************************/

static const timer_config_t timer_0_config = {
    .type = TIMER_SPLIT,
    .timer_a = {
        .type = TIMER_SPLIT_PERIODIC,
        .count_up = false
    }
};

static button_uart_override_t g_override = BUTTON_UART_OVERRIDE_NONE;

/**************************************************
* Public Functions
***************************************************/

int main(void)
{
    button_uart_override_t override = g_override;

    /* Set system clock to CLOCK_RATE */
    set_clock();

    enable_peripherals();

    timer_configure(TIMER_0, &timer_0_config);
    timer_enable(TIMER_0, TIMER_A);

    int res = uart_init(
                  UART_ID_0,
                  115200,
                  UART_PARITY_NONE,
                  UART_DATABITS_8,
                  UART_STOPBITS_1,
                  uart_chars_received
              );

    if (res != 0)
    {
        /* Warn user UART failed to init */
        flash_error(LED_RED, LED_GREEN, DELAY / 4);
    }

    /* iprintf is a non-float version of printf (it won't print floats).
     * Using the full printf() would double the code size of this small example program. */
    iprintf("Hello %s, %d!\n", "world", 123);

    while (1)
    {
        gpio_set_output(LED_BLUE, 0);
        gpio_set_output(LED_RED, 0);
        gpio_set_output(LED_GREEN, 0);

        busy_sleep(DELAY);

        printf("timer0 = 0x%08lx\n", timer_get_value(TIMER_0, TIMER_A));

        if (override != g_override)
        {
            const char *msg;
            override = g_override;
            switch (override)
            {
            case BUTTON_UART_OVERRIDE_NONE:
                msg = "off";
                break;
            case BUTTON_UART_OVERRIDE_ONE:
                msg = "Button #1";
                break;
            case BUTTON_UART_OVERRIDE_TWO:
                msg = "Button #2";
                break;
            default:
                msg = "Unknown";
                break;
            }
            iprintf("Override is now %s\n", msg);
        }

        if (!gpio_read_input(BUTTON_ONE) || (override == BUTTON_UART_OVERRIDE_ONE))
        {
            /* Button one pressed as input is low */
            gpio_set_output(LED_BLUE, 1);
            /* We could also use iprintf instead */
            uart_write_str(UART_ID_0, "blue\n");
        }
        else if (!gpio_read_input(BUTTON_TWO) || (override == BUTTON_UART_OVERRIDE_TWO))
        {
            /* Button two pressed as input is low */
            gpio_set_output(LED_RED, 1);
            uart_write_str(UART_ID_0, "red\n");
        }
        else
        {
            /* Neither button pressed */
            gpio_set_output(LED_GREEN, 1);
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

void uart_chars_received(
    uart_id_t uart_id,
    const char *buffer,
    size_t buffer_size
)
{
    /* Don't do any printf here - we're in an interrupt and we
     * need to get out of it as quickly as possible.
     */
    size_t i;
    for (i = 0; i < buffer_size; i++)
    {
        /* Deal with received characters */
        char c = buffer[i];
        switch (c)
        {
        case '1':
            /* Pretend button one is pressed */
            g_override = BUTTON_UART_OVERRIDE_ONE;
            break;
        case '2':
            /* Pretend button two is pressed */
            g_override = BUTTON_UART_OVERRIDE_TWO;
            break;
        default:
            /* Stop pretending anything's pressed */
            g_override = BUTTON_UART_OVERRIDE_NONE;
            break;
        }
    }
}

/**************************************************
* End of file
***************************************************/

