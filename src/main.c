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
#include <inttypes.h>
#include <ctype.h>
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

/* A nice pause (for busy_sleep) */
#define DELAY (CLOCK_RATE / 64) /* = 220ms */

#define IN_0 GPIO_MAKE_IO_PIN(GPIO_PORT_D, 6) /* Speed input */
#define IN_1 GPIO_MAKE_IO_PIN(GPIO_PORT_E, 0) /* Tacho input */
#define IN_2 GPIO_MAKE_IO_PIN(GPIO_PORT_E, 4) /* Button input */
#define OUT_0 GPIO_MAKE_IO_PIN(GPIO_PORT_E, 5) /* Tacho output */

#define BUTTON_DEBOUNCE_COUNT 3 /* Number of 250 ms overflows required to register button */

/* Magic value which indicates speed is approximately zero */
#define STALLED 0xFFFFFFFF

/* The rate at which get_counter() ticks in Hz */
#define TICK_RATE (CLOCK_RATE / 64)

/* Number of counter ticks elapsed before we declare speed to be zero. */
#define MAX_PERIOD TICK_RATE /* One second */

/**************************************************
* Data Types
**************************************************/

typedef struct waveform_t
{
    uint32_t period;
    uint32_t last_seen;
} waveform_t;

/**************************************************
* Function Prototypes
**************************************************/

static void uart_chars_received(
    uart_id_t uart_id,
    const char *buffer,
    size_t buffer_size
);

static void timer_interrupt(
    timer_module_t timer,
    timer_ab_t ab,
    void *p_context,
    uint32_t n_context
);

static uint32_t get_counter(void);

static void input_interrupt(gpio_io_pin_t pin, void *p_context, uint32_t n_context);

/**************************************************
* Public Data
**************************************************/

extern unsigned int g_int_count;

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

static volatile uint32_t g_output_rate = 65536;

static volatile uint32_t g_new_output_rate = 0;

static volatile uint32_t g_char_count = 0;

static volatile bool g_out_level = false;

static volatile bool g_button_count = 3;

static volatile bool g_start = false;

static volatile waveform_t speedo = { .last_seen = 0, .period = STALLED };
static volatile waveform_t tacho  = { .last_seen = 0, .period = STALLED };

/**************************************************
* Public Functions
***************************************************/

int main(void)
{
    int pixel_width;
    struct lcd_ver_t ver;
    struct lcd_mode_t mode;
    uint32_t old_char_count;

    /* Set system clock to CLOCK_RATE */
    set_clock();

    gpio_enable_peripherals();

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
        gpio_flash_error(LED_RED, LED_GREEN, DELAY / 4);
    }

    gpio_make_input(IN_0);
    gpio_make_input(IN_1);
    gpio_make_input(IN_2);
    gpio_make_output(OUT_0, 0);

    gpio_register_handler(IN_0, GPIO_INTERRUPT_MODE_BOTH, input_interrupt, (void *) &speedo, 0);
    gpio_register_handler(IN_1, GPIO_INTERRUPT_MODE_BOTH, input_interrupt, (void *) &tacho, 0);

    timer_configure(TIMER_WIDE_0, &g_timer_config);
    timer_register_handler(TIMER_WIDE_0, TIMER_A, timer_interrupt, NULL, 0);
    timer_register_handler(TIMER_WIDE_0, TIMER_B, timer_interrupt, NULL, 0);
    timer_set_interval_load(TIMER_WIDE_0, TIMER_A, 1 << 24); /* 2**24 / 66.67MHz = 0.25 secs between interrupts */
    timer_set_interval_load(TIMER_WIDE_0, TIMER_B, g_output_rate);

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

    PRINTF("Pausing...\n");

    /* Give the board a chance to boot up */

#if 0
    while(get_counter() < 2000000)
    {

    }
#else
    while(!g_start)
    {

    }
    old_char_count = g_char_count;
#endif

    lcd_init();

    while (1)
    {
        uint32_t then, now;

        lcd_get_version(&ver);

        PRINTF("Supplier=0x%04x, Product=0x%02x, Rev=0x%02x, Chk=0x%02X\n", ver.supplier_id, ver.product_id, ver.revision, ver.check_value);

        lcd_get_mode(&mode);

        PRINTF("colour_enhancement=%c\n", mode.colour_enhancement ? 'Y' : 'N');
        PRINTF("frc=%c\n", mode.frc ? 'Y' : 'N');
        PRINTF("lshift_rising_edge=%c\n", mode.lshift_rising_edge ? 'Y' : 'N');
        PRINTF("horiz_active_high=%c\n", mode.horiz_active_high ? 'Y' : 'N');
        PRINTF("vert_active_high=%c\n", mode.vert_active_high ? 'Y' : 'N');
        PRINTF("tft_type=%x\n", mode.tft_type);
        PRINTF("horiz_pixels=%u\n", mode.horiz_pixels);
        PRINTF("vert_pixels=%u\n", mode.vert_pixels);
        PRINTF("even_sequence=%x\n", mode.even_sequence);
        PRINTF("odd_sequence=%x\n", mode.odd_sequence);

        lcd_get_version(&ver);

        PRINTF("Supplier=0x%04x, Product=0x%02x, Rev=0x%02x, Chk=0x%02X\n", ver.supplier_id, ver.product_id, ver.revision, ver.check_value);

        pixel_width = lcd_get_pixel_width();

        PRINTF("Pixel width is %d\n", pixel_width);

        if (pixel_width != 8)
        {
            lcd_set_pixel_width(8);
            pixel_width = lcd_get_pixel_width();
            PRINTF("Pixel width is now %d (should be 8!)\n", pixel_width);
        }


        PRINTF("in=%d%d%d, time=0x%08"PRIx32", or=%"PRIu32", button=%c\n",
               gpio_read_input(IN_0),
               gpio_read_input(IN_1),
               gpio_read_input(IN_2),
               get_counter(),
               g_output_rate,
               (g_button_count == 0) ? 'Y' : 'N');

        PRINTF("Speedo = %"PRIu32" (0x%08"PRIx32"), Tacho = %"PRIu32" (0x%08"PRIx32")\n", speedo.period, speedo.last_seen, tacho.period, tacho.last_seen);

        /*
         * We should see RED square (top left), BLUE square (top right), GREEN
         * square (down left), in time with the RGB led.
         */

        busy_sleep(DELAY);

        then = get_counter();
        lcd_paint_clear_rectangle(LCD_FIRST_COLUMN, LCD_FIRST_ROW, LCD_LAST_COLUMN, LCD_LAST_ROW);
        lcd_paint_fill_rectangle(MAKE_COLOUR(0xFF, 0x00, 0x00), LCD_FIRST_COLUMN, 100, LCD_FIRST_ROW, 100);
        now = get_counter();
        PRINTF("LCD took %"PRIu32" 'timer' ticks\n", now - then);

        busy_sleep(DELAY);

        lcd_paint_clear_rectangle(LCD_FIRST_COLUMN, LCD_FIRST_ROW, LCD_LAST_COLUMN, LCD_LAST_ROW);

        lcd_paint_fill_rectangle(MAKE_COLOUR(0x00, 0xFF, 0x00), 100, 200, 0, 100);

        while(g_char_count == old_char_count)
        {
            
        }

        old_char_count = g_char_count;

    }

    /* Shouldn't get here */
    return 0;
}


/**************************************************
* Private Functions
***************************************************/

/**
 * Called when characters arrive on the UART.
 */
static void uart_chars_received(
    uart_id_t uart_id,
    const char *buffer,
    size_t buffer_size
)
{
    /* Don't do any printf here - we're in an interrupt and we
     * need to get out of it as quickly as possible.
     */
    for (size_t i = 0; i < buffer_size; i++)
    {
        /* Deal with received characters */
        char c = buffer[i];
        if (isdigit((int)c))
        {
            g_new_output_rate *= 10;
            g_new_output_rate += (c - '0');
        }
        else if ((c == '\r') || (c == '\n'))
        {
            if (!g_output_rate)
            {
                /* Kick it off again as it was stopped */
                timer_set_interval_load(TIMER_WIDE_0, TIMER_B, g_new_output_rate);
            }

            g_output_rate = g_new_output_rate;
            g_new_output_rate = 0;
        }
        else if (c == 'q')
        {
            g_start = true;
        }
        else
        {
            g_new_output_rate = 0;
        }
        g_char_count++;
    }
}

/**
 * Handles all the timer overflow interrupts.
 *
 * Registered on both A and B timers on TIMER_WIDE_0. Timer A is our master
 * timing reference, used for measuring elapsed time between GPIO interrupts.
 * Timer B is to drive the output pin at the specified rate.
 */
static void timer_interrupt(
    timer_module_t timer,
    timer_ab_t ab,
    void *p_context,
    uint32_t n_context
)
{
    if (ab == TIMER_A)
    {
        g_overflow_count++;
        /* Blink on 1 overflow in 4, or 250ms per 1000ms */
        gpio_set_output(LED_RED, ((g_overflow_count & 0x03) == 0) ? 1 : 0);
        timer_interrupt_clear(timer, TIMER_A_INTERRUPT_TIMEOUT);
        /* Debounce the button */
        if (gpio_read_input(IN_2))
        {
            if (g_button_count)
            {
                /* When this hits zero, the button has been satisfactorily pressed */
                g_button_count--;
            }
        }
        else
        {
            g_button_count = BUTTON_DEBOUNCE_COUNT;
        }
        /* Zero stalled input readings */
        uint32_t now = get_counter();
        if ((now - speedo.last_seen) > MAX_PERIOD)
        {
            speedo.period = STALLED;
            speedo.last_seen = now;
        }
        if ((now - tacho.last_seen) > MAX_PERIOD)
        {
            tacho.period = STALLED;
            tacho.last_seen = now;
        }
    }
    else
    {
        timer_interrupt_clear(timer, TIMER_B_INTERRUPT_TIMEOUT);
        if (g_output_rate)
        {
            timer_set_interval_load(TIMER_WIDE_0, TIMER_B, g_output_rate);
            gpio_set_output(OUT_0, g_out_level);
            g_out_level = !g_out_level;
        }
    }
}

/**
 * Helper function that deals with the fact our timer regularly overflows. We count the overflows
 * and this function returns that count munged with the current timer value.
 *
 * It's 32-bit @ 1MHz and so wraps every 71.5 minutes.
 */
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

/*
 * Called when one of our input pins changes state
 */
static void input_interrupt(
    gpio_io_pin_t pin,
    void *p_context,
    uint32_t n_context
)
{
    /* Don't care which pin - just use the given context */
    volatile waveform_t *p = (waveform_t *) p_context;
    uint32_t now = get_counter();
    p->period = now - p->last_seen;
    p->last_seen = now;
}

/**************************************************
* End of file
***************************************************/

