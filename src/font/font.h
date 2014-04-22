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
* Code for displaying text/numbers on the LCD.
* 
*****************************************************/

#ifndef FONT_H
#define FONT_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************
* Includes
***************************************************/

#include "drivers/lcd/lcd.h"

/**************************************************
* Public Defines
***************************************************/

/* None */

/**************************************************
* Public Data Types
**************************************************/

/* None */

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Public Function Prototypes
***************************************************/

void font_draw_number_large(
    lcd_row_t x, lcd_col_t y,
    uint16_t number,
    unsigned int pad_width,
    lcd_colour_t fg,
    lcd_colour_t bg
);

void font_draw_text_small(
    lcd_row_t x, lcd_col_t y,
    const char* p_message,
    lcd_colour_t fg,
    lcd_colour_t bg,
    bool monospace
);

size_t font_draw_text_small_len(
    const char* p_message,
    bool monospace
);

void font_glyph_width_small(char x);

#ifdef __cplusplus
}
#endif

#endif /* ndef FONT_H */

/**************************************************
* End of file
***************************************************/

