/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2012-2013 theJPster (www.thejpster.org.uk)
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
* Functions in main that need to be called elsewhere (like the command harness)
* 
*****************************************************/

#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************
* Includes
***************************************************/

#include "util/util.h"
#include "menu/menu.h"

/**************************************************
* Public Defines
***************************************************/

/* The rate at which main_get_counter() ticks in Hz */
#define TICK_RATE (CLOCK_RATE / 64)

#define TICKS_PER_MS (TICK_RATE / 1000)

#define TIMER_TICKS_TO_MS(timer_ticks) ((unsigned int) ((timer_ticks) / TICKS_PER_MS))

#define MS_TO_TIMER_TICKS(ms) (((unsigned int) (ms)) * TICKS_PER_MS)

/**************************************************
* Public Data Types
**************************************************/

/* None */

/**************************************************
* Public Data
**************************************************/

void main_set_tacho(uint32_t timer_ticks);

uint32_t main_get_counter(void);

bool main_menu_close(
    const struct menu_t *p_menu,
    const struct menu_item_t *p_menu_item
);

void main_lcd_control(bool enabled);

void main_fake_short_press(void);

void main_fake_long_press(void);

/**************************************************
* Public Function Prototypes
***************************************************/

/* None */

#ifdef __cplusplus
}
#endif

#endif /* ndef MAIN_H */

/**************************************************
* End of file
***************************************************/

