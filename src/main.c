/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2013-2014 theJPster (www.thejpster.org.uk)
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

#include "drivers/misc/misc.h"
#include "drivers/uart/uart.h"
#include "drivers/gpio/gpio.h"
#include "drivers/timers/timers.h"

#include "command/command.h"
#include "circbuffer/circbuffer.h"
#include "util/util.h"

/**************************************************
* Defines
***************************************************/

#define ON_MS 100
#define OFF_MS 900

/* Size of IRQ to userland ring buffer */
#define MAX_UART_CHARS 16

#define MS_TO_CLOCKS(x) ((x) * (CLOCK_RATE / 1000UL))

/**************************************************
* Data Types
**************************************************/

/* None */

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
    .type = TIMER_JOINED,
    .timer_a = {
        .count_up = true
    }
};

static struct circbuffer_t g_uart_cb;

static uint8_t g_buffer[MAX_UART_CHARS];

/**************************************************
* Public Functions
***************************************************/

int main(void)
{
    uint32_t last = 0;
    bool on_period = false;

    /* Set system clock to CLOCK_RATE */
    set_clock();

    circbuffer_init(&g_uart_cb, g_buffer, NUMELTS(g_buffer));

    gpio_enable_peripherals();

    timer_configure(TIMER_0, &timer_0_config);
    timer_enable(TIMER_0, TIMER_A);
    timer_set_interval_load(TIMER_0, TIMER_A, 0);

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
        gpio_flash_error(LED_RED, LED_GREEN, 250);
    }

    /* iprintf is a non-float version of printf (it won't print floats).
     * Using the full printf() would double the code size of this small example program. */
    iprintf("Hello %s, %d!\n", "world", 123);

    command_init();

    while (1)
    {
        uint32_t diff, now;

        now = timer_get_value(TIMER_0, TIMER_A);

        diff = now - last;

        /* --+---------+-----------+----
         *   | Off...   | On...    | Off...
         * --+---------+-----------+----
         */

        if (diff > MS_TO_CLOCKS(ON_MS+OFF_MS))
        {
            /* Enter off period */
            gpio_set_output(LED_BLUE, 0);
            gpio_set_output(LED_RED, 0);
            gpio_set_output(LED_GREEN, 0);
            /* Reset LED timer */
            last = now;
            on_period = false;
        }
        else if ((diff > MS_TO_CLOCKS(OFF_MS)) && !on_period)
        {
            /* Enter on period, printing some debug as we do. */
            if (!gpio_read_input(BUTTON_ONE))
            {
                /* Button one pressed as input is low */
                gpio_set_output(LED_BLUE, 1);
            }
            else if (!gpio_read_input(BUTTON_TWO))
            {
                /* Button two pressed as input is low */
                gpio_set_output(LED_RED, 1);
            }
            else
            {
                /* Neither button pressed */
                gpio_set_output(LED_GREEN, 1);
            }
            on_period = true;
        }

        while (!circbuffer_isempty(&g_uart_cb))
        {
            char c = (char) circbuffer_read(&g_uart_cb);
            command_handle_char(c);
        }

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
    while (buffer_size)
    {
        if (!circbuffer_isfull(&g_uart_cb))
        {
            /* Drop chars if buffer full */
            circbuffer_write(&g_uart_cb, *buffer);
        }
        buffer_size--;
        buffer++;
    }

}

/**************************************************
* End of file
***************************************************/

