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
#include <inttypes.h>
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

static uint32_t get_counter(void);

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Private Data
**************************************************/

static const timer_config_t g_timer_config =
{
    .type = TIMER_SPLIT,
    .timer_a = {
        .type = TIMER_SPLIT_PERIODIC,
        .count_up = true
    },
    .timer_b = {
        .type = TIMER_SPLIT_PERIODIC,
        .count_up = true
    }
};

static volatile uint32_t g_overflow_count;

static volatile uint32_t g_led_factor = 65536;

static volatile uint32_t g_char_count = 0;

static volatile uint32_t g_led_count = 0;

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

    timer_configure(TIMER_WIDE_0, &g_timer_config);
    timer_register_handler(TIMER_WIDE_0, TIMER_A, timer_interrupt, NULL, 0);
    timer_register_handler(TIMER_WIDE_0, TIMER_B, timer_interrupt, NULL, 0);
    timer_set_interval_load(TIMER_WIDE_0, TIMER_A, 1<<24); /* 2**24 / 66.67MHz = 0.25 secs between interrupts */
    timer_set_interval_load(TIMER_WIDE_0, TIMER_B, g_led_factor);

    /*
     * We as increment the overflow counter (a uint32_t) every 250 ms or so,
     * the overflow counter will itself overflow after 34 years of continuous running
     */

    /*
     * The get_counter() function makes a single 32-bit value out of the timer
     * value and the overflow counter. That effectively runs at 66.67MHz / 64
     * = ~1MHz and will wrap every 68 minutes, roughly.
     */
    timer_interrupt_enable(TIMER_WIDE_0, TIMER_A_INTERRUPT_TIMEOUT);
    timer_interrupt_enable(TIMER_WIDE_0, TIMER_B_INTERRUPT_TIMEOUT);
    timer_enable(TIMER_WIDE_0, TIMER_A);
    timer_enable(TIMER_WIDE_0, TIMER_B);

    while (1)
    {
        uint32_t then, now;

        then = get_counter();
        busy_sleep(DELAY);
        now = get_counter();
        iprintf("Delay of %u took %" PRIu32 " 'timer' ticks\n", DELAY, now - then);

        printf("in_0=%d, in_1=%d, in_2=%d, time=0x%08"PRIx32", cc=%"PRIu32", gf=0x%08"PRIx32", gc=0x%08"PRIx32"\n",
               gpio_read_input(IN_0),
               gpio_read_input(IN_1),
               gpio_read_input(IN_2),
               get_counter(),
               g_char_count,
               g_led_factor,
               g_led_count);

        gpio_set_output(LED_RED, 1);

        /*
         * We should see RED square (top left), BLUE square (top right), GREEN
         * square (down left), in time with the RGB led.
         */

        busy_sleep(DELAY);

#if 0
        then = get_counter();
        lcd_paint_clear_rectangle(0, 0, 479, 271);
        lcd_paint_fill_rectangle(MAKE_COLOUR(0xFF, 0x00, 0x00), 0, 100, 0, 100);
        now = get_counter();
        iprintf("LCD took %" PRIu32 " 'timer' ticks\n", now - then);
#endif

        busy_sleep(DELAY);

        gpio_set_output(LED_RED, 0);

        //lcd_paint_clear_rectangle(0, 0, 479, 271);

        //lcd_paint_fill_rectangle(MAKE_COLOUR(0x00, 0xFF, 0x00), 100, 200, 0, 100);

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
        if (c == '1')
        {
            if (g_led_factor >= 2)
            {
                g_led_factor = g_led_factor / 2;
            }
        }
        else if (c == '2')
        {
            if (g_led_factor <= (1U<<31)-1)
            {
                g_led_factor = g_led_factor * 2;
            }
        }
        g_char_count++;
    }
}

static void timer_interrupt(
    timer_module_t timer,
    timer_ab_t ab,
    void* p_context,
    uint32_t n_context
)
{
    if (ab == TIMER_A)
    {
        g_overflow_count++;
        timer_interrupt_clear(timer, TIMER_A_INTERRUPT_TIMEOUT);
    }
    else
    {
        gpio_set_output(LED_BLUE, g_led_count & 1);
        g_led_count++;
        timer_set_interval_load(TIMER_WIDE_0, TIMER_B, g_led_factor);
        timer_interrupt_clear(timer, TIMER_B_INTERRUPT_TIMEOUT);
    }
}

static uint32_t get_counter(void)
{
    uint32_t timer_val, overflow_val;

    disable_interrupts();
    timer_val = timer_get_value(TIMER_WIDE_0, TIMER_A);
    overflow_val = g_overflow_count;
    enable_interrupts();

    /*
     * Chop 6 bits off to make a 1MHz free-running timer @ ~1Mhz. Combine the
     * 18 bit free-running ~1MHz timer with the overflow counter to produce a
     * 32-bit value
     */
    timer_val >>= 6;
    timer_val |= (overflow_val << 18);

    return timer_val;
}

/**************************************************
* End of file
***************************************************/

