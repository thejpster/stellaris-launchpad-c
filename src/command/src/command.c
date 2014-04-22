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

#include "drivers/gpio/gpio.h"

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

#define COMMAND_DEFINITIONS \
    X("help", fn_help, "- Prints help") \
    X("gpio", fn_gpio, "- Set GPIO")

/**************************************************
* Function Prototypes
**************************************************/

static void process_command(void);
static void handle_backspace(void);
static void handle_char(char c);

static void print_prompt(void);

static void beep(void);

static uint32_t parse_int(const char* str);

#define X(label, fun, help) \
    static int fun(unsigned int argc, char* argv[]);
COMMAND_DEFINITIONS
#undef X

/**************************************************
* Public Data
**************************************************/

static char g_command_buffer[MAX_COMMAND_LINE];
static size_t g_buffer_used = 0;

#define X(label, fun, help) \
    { label, fun, help },
static const struct command_t g_commands[] = {
    COMMAND_DEFINITIONS
};
#undef X

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
        fflush(stdout);
#endif
    }
}


static void print_prompt(void)
{
    PRINTF("\r> ");
    fflush(stdout);
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
        else
        {
            break;
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

/**************************************************
* End of file
***************************************************/

