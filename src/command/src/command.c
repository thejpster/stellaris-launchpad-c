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
*
* This module implements a simple command line harness.
*
*****************************************************/

/**************************************************
* Includes
***************************************************/

#include "util/util.h"

#include <ctype.h>
#include <math.h>

#include "drivers/misc/misc.h"
#include "drivers/lcd/lcd.h"
#include "drivers/gpio/gpio.h"

#include "menu/menu_lexgo_bonus.h"
#include "font/font.h"
#include "main.h"

#include "command/command.h"

/**************************************************
* Defines
***************************************************/

#define MAX_COMMAND_LINE (80)
#define MAX_ARGS (8)

/**************************************************
* Data Types
**************************************************/

typedef int (*command_fn_t)(unsigned int argc, char* argv[]);

/**
 * Declares a command that can be called.
 */
struct command_t
{
    const char* p_command;
    command_fn_t fn;
    const char* p_help;
};

/**************************************************
* Function Prototypes
**************************************************/

static void process_command(void);
static void handle_backspace(void);
static void handle_char(char c);

static void print_prompt(void);

static void beep(void);

static uint32_t parse_int(const char* str);

static int fn_help(unsigned int argc, char* argv[]);
static int fn_lcdcol(unsigned int argc, char* argv[]);
static int fn_lcdpx(unsigned int argc, char* argv[]);
static int fn_gpio(unsigned int argc, char* argv[]);
static int fn_tacho(unsigned int argc, char* argv[]);
static int fn_rpm(unsigned int argc, char* argv[]);
static int fn_digits(unsigned int argc, char* argv[]);
static int fn_text(unsigned int argc, char* argv[]);
static int fn_cal(unsigned int argc, char* argv[]);
static int fn_menu(unsigned int argc, char* argv[]);
static int fn_enter(unsigned int argc, char* argv[]);
static int fn_down(unsigned int argc, char* argv[]);

/**************************************************
* Public Data
**************************************************/

static char g_command_buffer[MAX_COMMAND_LINE];
static size_t g_buffer_used = 0;

static const struct command_t g_commands[] = {
    { "help", fn_help, "- Prints help" },
    { "lcdcol", fn_lcdcol, "- Paints a rectangle in the given colour" },
    { "lcdpx", fn_lcdpx, "- Gets/Sets LCD pixel width" },
    { "gpio", fn_gpio, "- Set GPIO" },
    { "tacho", fn_tacho, "- Set tacho output" },
    { "rpm", fn_rpm, "- Set tacho output in RPM" },
    { "digits", fn_digits, "- Draw some large numbers" },
    { "text", fn_text, "- Draw some text" },
    { "cal", fn_cal, "- Calibrate delay loop" },
    { "menu", fn_menu, "- Open the LCD menu" },
    { "enter", fn_enter, "- Select the LCD menu item" },
    { "down", fn_down, "- Go down one item" },
};

/**************************************************
* Private Data
**************************************************/

/* None */

/**************************************************
* Public Functions
***************************************************/

void command_init(void)
{
    print_prompt();
    g_buffer_used = 0;
    g_command_buffer[0] = '\0';
}

void command_handle_char(char c)
{
    if ((c == '\r') || (c == '\n'))
    {
        process_command();
    }
    else if (c == 0x08)
    {
        handle_backspace();
    }
    else if (isprint((int) c))
    {
        handle_char(c);
    }
    else
    {
        beep();
    }
}

void command_handle_chars(const char* p_str, size_t num_chars)
{
    while(num_chars)
    {
        command_handle_char(*p_str);
        p_str++;
        num_chars--;
    }   
}

/**************************************************
* Private Functions
***************************************************/

static void process_command(void)
{
    char* argv[MAX_ARGS];
    unsigned int argc = 0;
    bool found = false;

    if (!g_buffer_used)
    {
        /* Catch LF after a CR silently */
        return;
    }

    /* If your console is echoing keypresses, you don't need this */
#ifndef BUFFERED_CONSOLE
    PRINTF("\n");
#endif

    /* Terminate the buffer */
    g_command_buffer[g_buffer_used] = '\0';

    argv[argc] = strtok(g_command_buffer, " ");
    while(argv[argc] && (argc < MAX_ARGS))
    {
        argc++;
        argv[argc] = strtok(NULL, " ");
    }

    for(unsigned int i = 0; i < NUMELTS(g_commands); i++)
    {
        const struct command_t * const p = &(g_commands[i]);
        if (strcmp(argv[0], p->p_command) == 0)
        {
            if (p->fn)
            {
                int result = p->fn(argc, argv);
                if (result == 0)
                {
                    PRINTF("Command OK\n");
                }
                else
                {
                    PRINTF("Command error %d\n", result);
                }
            }
            found = true;
            break;
        }
    }

    if (!found)
    {
        PRINTF("Command '%s' not found!\n", argv[0]);
    }

    command_init();
}

static void handle_backspace(void)
{
    if (g_buffer_used)
    {
        /* Erase the previous character and go back one */
        PRINTF("\b \b");
        /* One less in the buffer */
        g_buffer_used--;
    }
    else
    {
        beep();
    }
}

static void handle_char(char c)
{
    if (g_buffer_used != (MAX_COMMAND_LINE-1))
    {
        g_command_buffer[g_buffer_used] = c;
        g_buffer_used++;
#ifndef BUFFERED_CONSOLE
        /* If your console is echoing keypresses, you don't need this */
        PRINTF("%c", c);
#endif
    }
}


static void print_prompt(void)
{
    PRINTF("\r> ");
}

static void beep(void)
{
    PRINTF("\a");
}

static uint32_t parse_int(const char* str)
{
    uint32_t result = 0;
    unsigned int factor = 10;
    if (strlen(str) >= 2)
    {
        if ((str[0] == '0') && (str[1] == 'x'))
        {
            factor = 16;
            str = str + 2;
        }
    }
    while(*str)
    {
        const char c = *str;
        result *= factor;
        if (isdigit((int) c))
        {
            result += c - '0';
        }
        else if ((c >= 'A') && (c <= 'F') && (factor == 16))
        {
            result += 10 + (c - 'A');
        }
        else if ((c >= 'a') && (c <= 'f') && (factor == 16))
        {
            result += 10 + (c - 'a');
        }
        str++;
    }
    return result;
}

/* Commands */

static int fn_help(unsigned int argc, char* argv[])
{
    size_t max_len = 0;
    PRINTF("Command list:\n");
    /* Find the longest command, so the padding is neat */
    for(unsigned int i = 0; i < NUMELTS(g_commands); i++)
    {
        size_t this_len = strlen(g_commands[i].p_command);
        if (this_len > max_len)
        {
            max_len = this_len;
        }
    }
    for(unsigned int i = 0; i < NUMELTS(g_commands); i++)
    {
        const struct command_t * const p = &(g_commands[i]);
        PRINTF("\t%-*s %s\n", (int) max_len, p->p_command, p->p_help);
    }
    /* Help is always successful */
    return 0;
}

static int fn_lcdcol(unsigned int argc, char* argv[])
{
    uint32_t colour = 0x00FFFFFF;
    lcd_col_t x1 = LCD_FIRST_COLUMN, x2 = LCD_LAST_COLUMN;
    lcd_row_t y1 = LCD_FIRST_ROW, y2 = LCD_LAST_ROW;
    if (argc >= 2)
    {
        colour = parse_int(argv[1]);
    }
    if (argc >= 3)
    {
        x1 = parse_int(argv[2]);
    }
    if (argc >= 4)
    {
        y1 = parse_int(argv[3]);
    }
    if (argc >= 5)
    {
        x2 = parse_int(argv[4]);
    }
    if (argc >= 6)
    {
        y2 = parse_int(argv[5]);
    }
    PRINTF("%d,%d->%d,%d => %06lx\n", x1,y1,x2,y2,colour);
    lcd_paint_fill_rectangle(colour, x1, x2, y1, y2);
    return 0;
}

static int fn_lcdpx(unsigned int argc, char* argv[])
{
    int result = 0;
    if (argc == 2)
    {
        enum lcd_pixel_width_t width = parse_int(argv[1]);
        PRINTF("Pixel width = %u\n", width);
        lcd_set_pixel_width(width);
    }
    else
    {
        PRINTF("Takes 1 argument\n");
        result = 1;
    }
    return result;
}

static int fn_gpio(unsigned int argc, char* argv[])
{
    /* portpin i/0/1 */
    /* e.g. E1 0 */
    /* e.g. A3 i */
    if (argc != 3)
    {
        PRINTF("Call %s A3 i to set A3 as input\n", argv[0]);
        PRINTF("Call %s E1 0 to set E1 as output low\n", argv[0]);
        PRINTF("Call %s F2 1 to set F2 as output high\n", argv[0]);
        return 1;
    }
    else
    {
        gpio_port_t port = argv[1][0] - 'A';
        uint8_t pin_no = argv[1][1] - '0';
        char mode = argv[2][0];
        gpio_io_pin_t pin = GPIO_MAKE_IO_PIN(port, pin_no);
        if (port >= GPIO_NUM_PORTS)
        {
            PRINTF("Bad port\n");
            return 2;
        }
        if (pin_no > 7)
        {
            PRINTF("Bad pin\n");
            return 3;
        }
        switch(mode)
        {
        case 'i':
            gpio_make_input(pin);
            PRINTF("%s (0x%04X) is %d\n", argv[1], pin, gpio_read_input(GPIO_MAKE_IO_PIN(port, pin)));
            break;
        case '0':
        case '1':
            gpio_make_output(pin, mode - '0');
            PRINTF("%s (0x%04X) set to %d\n", argv[1], pin, mode - '0');
            break;
        default:
            PRINTF("Bad mode\n");
            return 4;
        }
        return 0;
    }
}

static int fn_tacho(unsigned int argc, char* argv[])
{
    int result = 0;
    /* Sets the tacho output */
    if (argc != 2)
    {
        uint32_t speed = main_read_tacho();
        PRINTF("Tacho in is %08lx (%lu)\n", speed, speed);
        speed = main_read_speedo();
        PRINTF("Speedo in is %08lx (%lu)\n", speed, speed);
    }
    else
    {
        uint32_t speed = parse_int(argv[1]);
        PRINTF("Set tacho out to %08lx (%lu)\n", speed, speed);
        main_set_tacho(speed);
    }
    return result;
}

static int fn_rpm(unsigned int argc, char* argv[])
{
    const double tacho_power = 1.0906;
    const double tacho_ratio = 1679251179;
    int result = 0;
    if (argc != 2)
    {
        PRINTF("Give an rpm\n");
        result = 1;
    }
    else
    {
        uint32_t rpm = parse_int(argv[1]);
        uint32_t speed;
        if (rpm > 0)
        {
            double p = pow(rpm, tacho_power);
            double inter = tacho_ratio / p;
            speed = (uint32_t) inter;
        }
        else
        {
            speed = 0;
        }
        PRINTF("Set tacho out to %lu rpm, %08lx (%lu)\n", rpm, speed, speed);
        main_set_tacho(speed);
    }
    return result;
}

static int fn_digits(unsigned int argc, char* argv[])
{
    int result = 0;
    if (argc != 5)
    {
        PRINTF("Call %s <x> <y> <colour> <number>\n", argv[0]);
        PRINTF("\te.g. %s 100 100 0xFFFF00 1234\n", argv[0]);
        result = 1;
    }
    else
    {
        uint32_t now, then;
        lcd_col_t x = parse_int(argv[1]);
        lcd_row_t y = parse_int(argv[2]);
        lcd_colour_t col = parse_int(argv[3]);
        uint16_t number = parse_int(argv[4]);
        PRINTF("Drawing %u @ %u,%u in %06lx\n", number, x, y, col);
        then = get_counter();
        font_draw_number_large(x, y, number, col, LCD_BLACK);
        now = get_counter();
        PRINTF("Took %u ms\n", TIMER_TICKS_TO_MS(now - then));
    }
    return result;
}

static int fn_text(unsigned int argc, char* argv[])
{
    int result = 0;
    if (argc < 5)
    {
        PRINTF("Call %s <x> <y> <colour> <message>\n", argv[0]);
        PRINTF("\te.g. %s 100 100 0xFF0000 Hello_world!\n", argv[0]);
        PRINTF("\tNote: _ are swapped for spaces\n");
        result = 1;
    }
    else
    {
        uint32_t now, then;
        lcd_col_t x = parse_int(argv[1]);
        lcd_row_t y = parse_int(argv[2]);
        lcd_colour_t col = parse_int(argv[3]);
        char* p_message = argv[4];
        while(*p_message)
        {
            if (*p_message == '_')
            {
                *p_message = ' ';  
            }
            p_message++;
        }
        p_message = argv[4];
        PRINTF("Drawing '%s' @ %u,%u in %06lx\n", p_message, x, y, col);
        then = get_counter();
        font_draw_text_small(x, y, p_message, col, LCD_BLACK);
        now = get_counter();
        PRINTF("Took %u ms\n", TIMER_TICKS_TO_MS(now - then));
    }
    return result;
}

static int fn_cal(unsigned int argc, char* argv[])
{
    int result = 0;
    if (argc < 2)
    {
        PRINTF("Call %s <delay>\n", argv[0]);
        result = 1;
    }
    else
    {
        uint32_t now, then;
        uint32_t delay;
        delay = parse_int(argv[1]);
        PRINTF("Delaying 0x%08" PRIx32 " (%" PRIu32 ") milliseconds\n", delay, delay);
        then = get_counter();
        delay_ms(delay);
        now = get_counter();
        PRINTF("Took %u ms\n", TIMER_TICKS_TO_MS(now - then));
    }
    return result;
}

static int fn_menu(unsigned int argc, char* argv[])
{
    menu_lexgo_bonus_init();
    return 0;
}

static int fn_enter(unsigned int argc, char* argv[])
{
    menu_keypress(MENU_KEYPRESS_ENTER);
    return 0;
}

static int fn_down(unsigned int argc, char* argv[])
{
    menu_keypress(MENU_KEYPRESS_DOWN);
    return 0;
}

/**************************************************
* End of file
***************************************************/

