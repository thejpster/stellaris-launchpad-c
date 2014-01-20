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

#include "util/util.h"

#include "drivers/misc/misc.h"
#include "drivers/uart/uart.h"
#include "drivers/gpio/gpio.h"
#include "drivers/lcd/lcd.h"
#include "drivers/timers/timers.h"

#include "circbuffer/circbuffer.h"
#include "command/command.h"
#include "font/font.h"
#include "clocks/clocks.h"
#include "menu/menu_lexgo_bonus.h"

#include "main.h"

/**************************************************
* Defines
***************************************************/

#define IN_0 GPIO_MAKE_IO_PIN(GPIO_PORT_D, 6) /* Speed input */
#define IN_1 GPIO_MAKE_IO_PIN(GPIO_PORT_E, 0) /* Tacho input */
#define IN_2 GPIO_MAKE_IO_PIN(GPIO_PORT_E, 4) /* Button input */
#define OUT_0 GPIO_MAKE_IO_PIN(GPIO_PORT_E, 5) /* Tacho output */

/* Delay between screen updates (in timer ticks) */
#define DELAY_TICKS MS_TO_TIMER_TICKS(1000)

/* Size of IRQ to userland ring buffer */
#define MAX_UART_CHARS 16

#define SHORT_PRESS 3
#define LONG_PRESS 8

/**************************************************
* Data Types
**************************************************/

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

static void input_interrupt(
    gpio_io_pin_t pin,
    void *p_context,
    uint32_t n_context
);

static void screen_redraw(void);

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

static uint8_t g_buffer[MAX_UART_CHARS];

static struct circbuffer_t g_uart_cb;

static volatile uint32_t g_overflow_count;

static volatile uint32_t g_output_rate = 0;

static volatile bool g_out_level = false;

static volatile unsigned int g_button_count = 0;

static volatile unsigned int g_short_presses;

static volatile unsigned int g_long_presses;

static bool menu_mode = false;

/**************************************************
* Public Functions
***************************************************/

int main(void)
{
    /* Set system clock to CLOCK_RATE */
    set_clock();

    circbuffer_init(&g_uart_cb, g_buffer, NUMELTS(g_buffer));

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
        gpio_flash_error(LED_RED, LED_GREEN, 250);
    }

    PRINTF("***********************************\n");
    PRINTF("* LCD dashboard demo...           *\n");
    PRINTF("***********************************\n");

    lcd_init();
    clocks_init();

    gpio_make_input(IN_0);
    gpio_make_input(IN_1);
    gpio_make_input(IN_2);
    gpio_make_output(OUT_0, 0);

    gpio_register_handler(IN_0, GPIO_INTERRUPT_MODE_BOTH, input_interrupt, NULL, 0);
    gpio_register_handler(IN_1, GPIO_INTERRUPT_MODE_BOTH, input_interrupt, NULL, 1);

    timer_configure(TIMER_WIDE_0, &g_timer_config);
    timer_register_handler(TIMER_WIDE_0, TIMER_A, timer_interrupt, NULL, 0);
    timer_register_handler(TIMER_WIDE_0, TIMER_B, timer_interrupt, NULL, 0);
    timer_set_interval_load(TIMER_WIDE_0, TIMER_A, 1 << 24); /* 2**24 / 66.67MHz = 0.25 secs between interrupts */

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

    uint32_t then, now, diff;
    then = get_counter();
    while (1)
    {
        now = get_counter();
        diff = now - then;
        if (diff > DELAY_TICKS)
        {
            if (!menu_mode)
            {
                screen_redraw();
            }
            then = now;
        }
        while (!circbuffer_isempty(&g_uart_cb))
        {
            char c = (char) circbuffer_read(&g_uart_cb);
            command_handle_char(c);
        }
        if (g_short_presses)
        {
            g_short_presses--;
            if (menu_mode)
            {
                menu_keypress(MENU_KEYPRESS_DOWN);
            }
        }
        if (g_long_presses)
        {
            g_long_presses--;
            if (!menu_mode)
            {
                menu_lexgo_bonus_init();
                menu_mode = true;
            }
            else
            {
                menu_keypress(MENU_KEYPRESS_ENTER);
            }
        }
        delay_ms(10);
    }

    /* Shouldn't get here */
    return 0;
}

void main_set_tacho(uint32_t timer_ticks)
{
    if (timer_ticks && !g_output_rate)
    {
        PRINTF("Start TW0/B\n");
        g_output_rate = timer_ticks;
        /* Kick it off again as it was stopped */
        timer_set_interval_load(TIMER_WIDE_0, TIMER_B, g_output_rate);
        timer_enable(TIMER_WIDE_0, TIMER_B);
    }
    else
    {
        /* Let it change on the next interrupt */
        g_output_rate = timer_ticks;
    }
}

/**
 * Helper function that deals with the fact our timer regularly overflows. We count the overflows
 * and this function returns that count munged with the current timer value.
 *
 * It's 32-bit @ 1MHz and so wraps every 71.5 minutes.
 */
uint32_t get_counter(void)
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

bool main_menu_close(
    const struct menu_t *p_menu,
    const struct menu_item_t *p_menu_item
)
{
    menu_mode = false;
    lcd_paint_clear_screen();
    return false;
}

void main_fake_short_press(void)
{
    g_short_presses++;
}

void main_fake_long_press(void)
{
    g_long_presses++;
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
            g_button_count++;
            if (g_button_count == LONG_PRESS)
            {
                g_long_presses++;
            }
        }
        else
        {
            if ((g_button_count >= SHORT_PRESS) && (g_button_count < LONG_PRESS))
            {
                /* Enough for a short press but not for a long press */
                g_short_presses;
            }
            g_button_count = 0;
        }
        clocks_timer_tick();
    }
    else
    {
        timer_interrupt_clear(timer, TIMER_B_INTERRUPT_TIMEOUT);
        gpio_set_output(OUT_0, g_out_level);
        g_out_level = !g_out_level;
        if (g_output_rate)
        {
            timer_set_interval_load(TIMER_WIDE_0, TIMER_B, g_output_rate);
        }
        else
        {
            timer_disable(TIMER_WIDE_0, TIMER_B);
        }
    }
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
    if (n_context == 0)
    {
        clocks_speedo_edge();
    }
    else
    {
        clocks_tacho_edge();
    }
}

static void screen_redraw(void)
{
    struct clocks_state_t state;
    char trip_buffer[20];
    lcd_row_t y = 140;
    clocks_get(&state);
    /* Do screen update here */
    font_draw_number_large(10, y, state.current_speed, 4, LCD_BLUE, LCD_BLACK);
    y = y + 110;
    font_draw_number_large(10, y, state.current_revs, 4, LCD_BLUE, LCD_BLACK);
    y = y + 110;
    for (int i = 0; i < CLOCKS_NUM_TRIPS; i++)
    {
        sprintf(trip_buffer, "%.6lu.%u mi",
                state.trip[i] / CLOCKS_DISTANCE_SCALE,
                (int) (state.trip[i] % CLOCKS_DISTANCE_SCALE));
        font_draw_text_small(50, y, trip_buffer, LCD_GREEN, LCD_BLACK);
        y = y + 20;
    }
    /* @todo should set rpm here */
}

/**************************************************
* End of file
***************************************************/

