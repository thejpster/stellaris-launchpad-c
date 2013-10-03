/*****************************************************
*
* Stellaris Launchpad "Lexgo Bonus" TFT dash project
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
*
* DESCRIPTION
*
* This is the main module for the Lexgo Bonus TFT dashboard. There is a 4.3"
* TFT connected displaying current speed and trip/odometer readings. A 12v
* input connected to a button on the dash allows the driver to cycle through
* various modes (as yet undefined). In addition to the speed signal (12v
* square wave) there is also a tacho input (again, 12v square wave), the
* frequency of which is measured and trimmed by a specified factor to produce
* a tacho output (12v square wave) for the rev counter. This allows us to
* compensate for changes in wheel size, diff, gearbox, etc.
*
* We create a free-running timer, the value of which is read whenever IN_0 and
* IN_1 change. This gives is the frequency of these two inputs.
*
* IN_2 is a button input which will need de-bouncing in some fashion.
*
* OUT_0 is the tacho output, driven from a second timer. As we haven't
* connected the output to a PWM pin, we'll simply have to set the pin in a
* timer interrupt.
*
* My first-draft guess is that there will be around 14.6ms per speedo
* pulse at 60 mph.
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
#include "lcd/lcd.h"
#include "timers/timers.h"

/**************************************************
* Defines
***************************************************/

#define DELAY (CLOCK_RATE / 64)

#define IN_0 GPIO_MAKE_IO_PIN(GPIO_PORT_D, 6) /* Speed input */
#define IN_1 GPIO_MAKE_IO_PIN(GPIO_PORT_E, 0) /* Tacho input */
#define IN_2 GPIO_MAKE_IO_PIN(GPIO_PORT_E, 4) /* Button input */
#define OUT_0 GPIO_MAKE_IO_PIN(GPIO_PORT_E, 5) /* Tacho output */

/**************************************************
* Data Types
**************************************************/

/* None */

/**************************************************
* Function Prototypes
**************************************************/

static void uart_chars_received(
    uart_id_t uart_id,
    const char* buffer,
    size_t buffer_size
);

static void timer_interrupt(
    timer_module_t timer,
    timer_ab_t ab,
    void* p_context,
    uint32_t n_context
);

static uint32_t get_overflow(void);

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Private Data
**************************************************/

static const timer_config_t timer_0_config =
{
    .type = TIMER_JOINED,
    .timer_a = {
        .type = TIMER_SPLIT_PERIODIC,
        .count_up = true
    }
};

static volatile uint32_t overflow_count;

/**************************************************
* Public Functions
***************************************************/

int main(void)
{
    int pixel_width;
    struct lcd_ver_t ver;
    struct lcd_mode_t mode;

    /* Set system clock to CLOCK_RATE */
    set_clock();

    enable_peripherals();

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

    gpio_make_input(IN_0);
    gpio_make_input(IN_1);
    gpio_make_input(IN_2);
    gpio_make_output(OUT_0, 0);

    lcd_init();

    lcd_get_version(&ver);

    printf("Supplier=0x%04x, Product=0x%02x, Rev=0x%02x\n", ver.supplier_id, ver.product_id, ver.revision);

    lcd_get_mode(&mode);

    printf("colour_enhancement=%c\n", mode.colour_enhancement ? 'Y' : 'N');
    printf("frc=%c\n", mode.frc ? 'Y' : 'N');
    printf("lshift_rising_edge=%c\n", mode.lshift_rising_edge ? 'Y' : 'N');
    printf("horiz_active_high=%c\n", mode.horiz_active_high ? 'Y' : 'N');
    printf("vert_active_high=%c\n", mode.vert_active_high ? 'Y' : 'N');
    printf("tft_type=%x\n", mode.tft_type);
    printf("horiz_pixels=%u\n", mode.horiz_pixels);
    printf("vert_pixels=%u\n", mode.vert_pixels);
    printf("even_sequence=%x\n", mode.even_sequence);
    printf("odd_sequence=%x\n", mode.odd_sequence);

    pixel_width = lcd_get_pixel_width();

    printf("Pixel width is %d\n", pixel_width);

    if (pixel_width != 8)
    {
        lcd_set_pixel_width(8);
        pixel_width = lcd_get_pixel_width();
        printf("Pixel width is now %d (should be 8!)\n", pixel_width);
    }

    timer_configure(TIMER_0, &timer_0_config);
    timer_register_handler(TIMER_0, TIMER_A, timer_interrupt, NULL, 0);
    timer_set_interval_load(TIMER_0, TIMER_A, 1<<24); /* 2**24 / 66.67MHz = 0.25 secs */
    /* We as increment the overflow counter (a uint32_t) every 250 ms or so,
     * the overflow counter will itself overflow after 34 years of continuous running */
    timer_interrupt_enable(TIMER_0, TIMER_A_INTERRUPT_TIMEOUT);
    timer_enable(TIMER_0, TIMER_A);

    while (1)
    {
        busy_sleep(DELAY);

        printf("in_0=%d, in_1=%d, in_2=%d\n",
               gpio_read_input(IN_0),
               gpio_read_input(IN_1),
               gpio_read_input(IN_2));

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        gpio_set_output(LED_RED, 1);
        gpio_set_output(LED_BLUE, 0);
        gpio_set_output(LED_GREEN, 0);

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        /*
         * We should see RED square (top left), BLUE square (top right), GREEN
         * square (down left), in time with the RGB led.
         */

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        lcd_paint_clear_rectangle(0, 0, 479, 271);

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        lcd_paint_fill_rectangle(MAKE_COLOUR(0xFF, 0x00, 0x00), 0, 100, 0, 100);

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        busy_sleep(DELAY);

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        gpio_set_output(LED_RED, 0);
        gpio_set_output(LED_BLUE, 0);
        gpio_set_output(LED_GREEN, 1);

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        lcd_paint_clear_rectangle(0, 0, 479, 271);

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        lcd_paint_fill_rectangle(MAKE_COLOUR(0x00, 0xFF, 0x00), 100, 200, 0, 100);

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        busy_sleep(DELAY);

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        gpio_set_output(LED_RED, 0);
        gpio_set_output(LED_BLUE, 1);
        gpio_set_output(LED_GREEN, 0);

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        lcd_paint_clear_rectangle(0, 0, 479, 271);

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));

        lcd_paint_fill_rectangle(MAKE_COLOUR(0x00, 0x00, 0xFF), 0, 100, 100, 200);

        printf("timer0 = 0x%08lx%06lx\n", get_overflow(), timer_get_value(TIMER_0, TIMER_A));
    }

    /* Shouldn't get here */
    return 0;
}


/**************************************************
* Private Functions
***************************************************/

static void uart_chars_received(
    uart_id_t uart_id,
    const char* buffer,
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

        (void) c;

    }
}

static void timer_interrupt(
    timer_module_t timer,
    timer_ab_t ab,
    void* p_context,
    uint32_t n_context
)
{
    overflow_count++;
    timer_interrupt_clear(TIMER_0, TIMER_A_INTERRUPT_TIMEOUT);
}

static uint32_t get_overflow(void)
{
    uint32_t res;
    disable_interrupts();
    res = overflow_count;
    enable_interrupts();
    return res;
}

/**************************************************
* End of file
***************************************************/

