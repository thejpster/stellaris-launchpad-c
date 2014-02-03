/*****************************************************
*
* Stellaris Launchpad LCD Driver Simulator
*
* Copyright (c) 2014 theJPster (www.thejpster.org.uk)
*
* Simulates an LCD by writing primitives to a FIFO.
* There is a Python application (lcd_render.py) which
* picks up these primitives and renders them to a 
* window.
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
#include "util/util.h"
#include "../lcd.h"

/**************************************************
* Defines
***************************************************/

/* None */

/**************************************************
* Data Types
**************************************************/

/* None */

/**************************************************
* Function Prototypes
**************************************************/

static void pixel_fn(int x, int y, uint32_t col);

/**************************************************
* Public Data
**************************************************/

extern FILE* f;

/**************************************************
* Private Data
**************************************************/

/* None */

/**************************************************
* Public Functions
***************************************************/

/**
 * Will set up the GPIO for driving the LCD.
 *
 * See header file for pinout.
 *
 */
int lcd_init(void)
{
    fprintf(f, "reset\n");

    PRINTF("Wait...\n");

    delay_ms(1500);

    PRINTF("LCD init...\n");

    lcd_paint_clear_screen();

    lcd_paint_fill_rectangle(LCD_RED,
        LCD_FIRST_COLUMN, LCD_LAST_COLUMN, 0, 20);

    delay_ms(100);

    lcd_paint_fill_rectangle(LCD_BLUE,
        LCD_FIRST_COLUMN, LCD_LAST_COLUMN, 20, 40);

    delay_ms(100);

    lcd_paint_fill_rectangle(LCD_GREEN,
        LCD_FIRST_COLUMN, LCD_LAST_COLUMN, 40, 60);

    delay_ms(100);

    lcd_paint_clear_screen();

    PRINTF("Done!\n");

    return 0;
}

/* Make all pins inputs */
void lcd_deinit(void)
{

}

/**
 * @return data bus width for pixel data
 */
enum lcd_pixel_width_t lcd_get_pixel_width(void)
{
    return LCD_PIXEL_WIDTH_8;
}

/**
 * @param width the new desired bus width (for pixel data only, not commands).
 */
void lcd_set_pixel_width(enum lcd_pixel_width_t width)
{
    /* Nothing */
}

/**
 * @param p_mode Pointer to LCD mode structure which will be filled in
 */
void lcd_get_mode(struct lcd_mode_t *p_mode)
{
    memset(p_mode, 0, sizeof(*p_mode));
}

/**
 * @param p_ver Pointer to LCD version structure which will be filled in
 */
void lcd_get_version(struct lcd_ver_t *p_ver)
{
    memset(p_ver, 0, sizeof(*p_ver));
}

void lcd_get_horiz_period(struct lcd_period_t *p_period)
{
    memset(p_period, 0, sizeof(*p_period));
}

void lcd_get_vert_period(struct lcd_period_t *p_period)
{
    memset(p_period, 0, sizeof(*p_period));
}

void lcd_on(void)
{
    /* Nothing */
}

void lcd_off(void)
{
    /* Nothing */
}

void lcd_get_dbc_conf(struct lcd_dbc_conf_t *p_dbc)
{
    memset(p_dbc, 0, sizeof(*p_dbc));
}

void lcd_set_dbc_conf(const struct lcd_dbc_conf_t *p_dbc)
{
    /* Nothing */
}

/**
 * Gets frame buffer / display mapping (aka 'address mode').
 * @param p_dbc Pointer to data structure which will be filled in.
 * with details of mapping.
 */
void lcd_get_address_mode(struct lcd_address_mode_t *p_mode)
{
    memset(p_mode, 0, sizeof(*p_mode));
}

/**
 * Sets frame buffer / display mapping (aka 'address mode').
 * @param p_dbc Pointer to data structure which will be read and passed to
 * LCD.
 */
void lcd_set_address_mode(const struct lcd_address_mode_t *p_mode)
{
    /* Nothing */
}

void lcd_set_backlight(uint8_t brightness)
{
    /* Nothing */
}

/**
 * Paints a solid rectangle to the LCD in the given colour.
 *
 * @param bg the RGB colour for all the pixels
 * @param x1 the starting column
 * @param x2 the end column
 * @param y1 the starting row
 * @param y2 the end row
 */
void lcd_paint_fill_rectangle(
    uint32_t bg,
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2
)
{
    fprintf(f, "box %d %d %d %d 0x%06lx\n", x1, x2, y1, y2, bg);
    fflush(f);
}

/**
 * Paints a mono rectangle to the LCD in the given colours. This is useful for
 * text.
 *
 * @param fg the RGB colour for set pixels
 * @param bg the RGB colour for unset pixels
 * @param x1 the starting column
 * @param x2 the end column
 * @param y1 the starting row
 * @param y2 the end row
 * @param p_pixels 1bpp data for the given rectangle, length (x2-x1+1)*(y2-y1+1) bits
 */
void lcd_paint_mono_rectangle(
    uint32_t fg,
    uint32_t bg,
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2,
    const uint8_t *p_pixels
)
{
    fprintf(f, "bitmap %d %d %d %d 0x%06lx 0x%06lx ", x1, x2, y1, y2, fg, bg);
    size_t size = (1 + x2 - x1) * (1 + y2 - y1);
    size_t bytes = (size + 7) / 8;
    for(size_t i = 0; i < bytes; i++)
    {
        fprintf(f, "%02X", p_pixels[i]);
    }        
    fprintf(f, "\n");
    fflush(f);
}

/**
 * Paints a full-colour rectangle to the LCD. This is useful for graphics but
 * you need up to 510KB for a full-screen image.
 *
 * @param x1 the starting column
 * @param x2 the end column
 * @param y1 the starting row
 * @param y2 the end row
 * @param p_pixels Run-length encoded pixel values, where the count is in the top byte
 */
void lcd_paint_colour_rectangle(
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2,
    const uint32_t *p_rle_pixels
)
{
    size_t size = (1 + x2 - x1) * (1 + y2 - y1);
    lcd_col_t x = x1;
    lcd_row_t y = y1;
    while (size)
    {
        uint32_t pixel = *p_rle_pixels;
        uint8_t count = (pixel >> 24) & 0xFF;
        size -= count;
        while (count--)
        {
            pixel_fn(x, y, pixel & 0xFFFFFF);
            if (x == x2)
            {
                x = x1;
                y++;
            }
            else
            {
                x++;
            }
        }
        p_rle_pixels++;
    }
}

/**************************************************
* Private Functions
***************************************************/

static void pixel_fn(int x, int y, uint32_t colour)
{
    fprintf(f, "plot %d %d 0x%06lx\n", x, y, colour);
    fflush(f);
}


/**************************************************
* End of file
***************************************************/

