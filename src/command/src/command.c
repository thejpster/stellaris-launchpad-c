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

#include <ctype.h>
#include <string.h>

#include "misc/misc.h"
#include "command/command.h"
#include "lcd/lcd.h"
#include "gpio/gpio.h"

/**************************************************
* Defines
***************************************************/

#define MAX_COMMAND_LINE (80)
#define MAX_ARGS (8)

/**************************************************
* Data Types
**************************************************/

typedef int (*command_fn_t)(unsigned int argc, const char* argv[]);

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

static int fn_help(unsigned int argc, const char* argv[]);
static int fn_lcdcol(unsigned int argc, const char* argv[]);
static int fn_lcdver(unsigned int argc, const char* argv[]);
static int fn_lcdpx(unsigned int argc, const char* argv[]);
static int fn_lcdmode(unsigned int argc, const char* argv[]);
static int fn_lcdperiod(unsigned int argc, const char* argv[]);
static int fn_lcddbc(unsigned int argc, const char* argv[]);
static int fn_lcdrd(unsigned int argc, const char* argv[]);
static int fn_gpio(unsigned int argc, const char* argv[]);

/**************************************************
* Public Data
**************************************************/

static char g_command_buffer[MAX_COMMAND_LINE];
static size_t g_buffer_used = 0;

static const struct command_t g_commands[] = {
    { "help", fn_help, "- Prints help" },
    { "lcdcol", fn_lcdcol, "- Paints a rectangle in the given colour" },
    { "lcdver", fn_lcdver, "- Gets LCD version" },
    { "lcdpx", fn_lcdpx, "- Gets/Sets LCD pixel width" },
    { "lcdmode", fn_lcdmode, "- Gets LCD mode" },
    { "lcdperiod", fn_lcdperiod, "- Gets LCD H/V period" },
    { "lcddbc", fn_lcddbc, "- Gets LCD Dynamic Brightness Control" },
    { "lcdrd", fn_lcdrd, "- Reads framebuffer pixels" },
    { "gpio", fn_gpio, "- Set GPIO" },
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
    const char* argv[MAX_ARGS];
    unsigned int argc = 0;
    bool found = false;

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

static int fn_help(unsigned int argc, const char* argv[])
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

static int fn_lcdcol(unsigned int argc, const char* argv[])
{
    if (argc < 2)
    {
        PRINTF("Not enough arguments\n");
        PRINTF("Type %s <col> [ <x1> <y2> <x2> <y2> ]\n", argv[0]);
        return 1;
    }
    else
    {
        uint32_t colour = 0x00FFFFFF;
        lcd_col_t x1 = 0, x2 = 479;
        lcd_row_t y1 = 0, y2 = 271;
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
}

static int fn_lcdver(unsigned int argc, const char* argv[])
{
    struct lcd_ver_t ver;
    lcd_get_version(&ver);
    PRINTF("Supplier=0x%04x, Product=0x%02x, Rev=0x%02x, Chk=0x%02X\n", ver.supplier_id, ver.product_id, ver.revision, ver.check_value);
    return 0;
}

static int fn_lcdpx(unsigned int argc, const char* argv[])
{
    int result = 0;
    if (argc == 1)
    {
        enum lcd_pixel_width_t width;
        width = lcd_get_pixel_width();
        PRINTF("Pixel width = %u\n", width);
    }
    else if (argc == 2)
    {
        enum lcd_pixel_width_t width = parse_int(argv[1]);
        PRINTF("Pixel width = %u\n", width);
        lcd_set_pixel_width(width);
    }
    else
    {
        PRINTF("Takes 0 or 1 arguments\n");
        result = 1;
    }
    return result;
}

static int fn_lcdmode(unsigned int argc, const char* argv[])
{
    struct lcd_mode_t mode;
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
    return 0;
}

static int fn_lcdperiod(unsigned int argc, const char* argv[])
{
    struct lcd_period_t period;
    lcd_get_horiz_period(&period);
    PRINTF("HT=%u,HPS=%u,HPW=%u,LPS=%u\n", period.total, period.display_start, period.sync_pulse_width, period.sync_pulse_start);
    lcd_get_vert_period(&period);
    PRINTF("VT=%u,VPS=%u,VPW=%u,FPS=%u\n", period.total, period.display_start, period.sync_pulse_width, period.sync_pulse_start);
    return 0;
}

static int fn_lcddbc(unsigned int argc, const char* argv[])
{
    struct lcd_dbc_conf_t conf;
    lcd_get_dbc_conf(&conf);
    PRINTF("manual=%u,transition=%u,mode=%d,enable=%u\n",
           conf.dbc_manual_brightness,
           conf.transition_effect,
           conf.mode,
           conf.master_enable
          );
    return 0;
}

static int fn_lcdrd(unsigned int argc, const char* argv[])
{
    lcd_col_t x1 = 0, x2 = 479;
    lcd_row_t y1 = 0, y2 = 271;
    if (argc >= 2)
    {
        x1 = parse_int(argv[1]);
    }
    if (argc >= 3)
    {
        y1 = parse_int(argv[2]);
    }
    if (argc >= 4)
    {
        x2 = parse_int(argv[3]);
    }
    if (argc >= 5)
    {
        y2 = parse_int(argv[4]);
    }
    PRINTF("%d,%d->%d,%d =>\n", x1,y1,x2,y2);
    uint32_t pixel;
    while((x1 != x2) && (y1 != y2))
    {
        pixel = 0xDEADBF;
        lcd_read_color_rectangle(x1, x2, y1, y2, &pixel, 1);
        PRINTF("%06lx ", pixel);
        x1++;
        if (x1 == x2)
        {
            PRINTF("\n");
            x1 = 0;
            y1++;
        }
    }
    return 0;
}

static int fn_gpio(unsigned int argc, const char* argv[])
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
        uint8_t pin = argv[1][1] - '0';
        char mode = argv[2][0];
        if (port >= GPIO_NUM_PORTS)
        {
            PRINTF("Bad port\n");
            return 2;
        }
        if (pin > 7)
        {
            PRINTF("Bad pin\n");
            return 3;
        }
        switch(mode)
        {
        case 'i':
            gpio_make_input(GPIO_MAKE_IO_PIN(port, pin));
            PRINTF("%s is %d\n", argv[1], gpio_read_input(GPIO_MAKE_IO_PIN(port, pin)));
            break;
        case '0':
        case '1':
            gpio_make_output(GPIO_MAKE_IO_PIN(port, pin), mode - '0');
            PRINTF("%s set to %c\n", argv[1], mode);
            break;
        default:
            PRINTF("Bad mode\n");
            return 4;
        }
        return 0;
    }
}

/**************************************************
* End of file
***************************************************/

