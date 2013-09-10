/*****************************************************
*
* Stellaris Launchpad LCD Driver
*
* Driver suits SSD1961 with self-initialisation, e.g.
* DisplayTech INT043BTFT.
*
* See https://www.displaytech-us.com/4-3-inch-integrated-tft-driver-boards
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
*****************************************************/

#ifndef LCD_H_
#define LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************
* Includes
***************************************************/

#include <stdint.h>
#include <stdbool.h>

/**************************************************
* Public Defines
***************************************************/

/* None */

/**************************************************
* Public Data Types
**************************************************/

enum lcd_tft_type_t
{
    LCD_TFT_TYPE_TFTA = 0x00,
    LCD_TFT_TYPE_TFTB = 0x01,
    LCD_TFT_TYPE_SERIAL_RGB = 0x02,
    LCD_TFT_TYPE_SERIAL_RGB_DUMMY = 0x03
};

enum lcd_tft_rgb_sequence_t
{
    LCD_TFT_RGB = 0x00,
    LCD_TFT_RBG = 0x01,
    LCD_TFT_GRB = 0x02,
    LCD_TFT_GBR = 0x03,
    LCD_TFT_BRG = 0x04,
    LCD_TFT_BGR = 0x05
};

typedef unsigned int lcd_row_t;
typedef unsigned int lcd_col_t;

struct lcd_mode_t
{
    bool colour_enhancement;
    bool frc; /* true = FRC, false = dither */
    bool lshift_rising_edge;
    bool horiz_active_high;
    bool vert_active_high;
    enum lcd_tft_type_t tft_type;
    lcd_col_t horiz_pixels;
    lcd_row_t vert_pixels;
    enum lcd_tft_rgb_sequence_t even_sequence;
    enum lcd_tft_rgb_sequence_t odd_sequence;
};

struct lcd_ver_t
{
    uint16_t supplier_id;
    uint8_t product_id;
    uint8_t revision;
};


/* r, b and b should be 0x00..0xFF */
#define MAKE_COLOUR(r, g, b) ( ((r) << 16) | ((g) << 8) | ((b) << 0) )
/* count should also be 0x00..0xFF */
#define MAKE_RLE_COLOUR(count, r, g, b) ( ((count) << 24) | ((r) << 16) | ((g) << 8) | ((b) << 0) )

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Public Function Prototypes
***************************************************/

/**
 * Will set up the GPIO for driving the LCD.
 *
 * The pinout is:
 *
 * /CS  = D2 (J3.05)
 * /RS  = D3 (J3.06)
 * /RD  = E1 (J3.07)
 * /WR  = E2 (J3.08)
 * /RST = E3 (J3.09)
 *
 * DATA00 = B0 (J1.03)
 * DATA01 = B1 (J1.04)
 * DATA02 = A2 (J2.10)
 * DATA03 = A3 (J2.09)
 * DATA04 = A4 (J2.08)
 * DATA05 = A5 (J1.08)
 * DATA06 = A6 (J1.09)
 * DATA07 = A7 (J1.10)
 *
 * In the future, I might add 16-bit mode:
 * DATA08 = F1 (J3.10)
 * DATA09 = F2 (J4.01)
 * DATA10 = F3 (J4.02)
 * DATA11 = F4 (J4.10)
 * DATA12 = C4 (J4.04)
 * DATA13 = C5 (J4.05)
 * DATA14 = C6 (J4.06)
 * DATA15 = C7 (J4.07)
 *
 */
extern int lcd_init(void);

/**
 * @return width in bits, i.e. 8, 9, 12, 16 or 18
 */
extern int lcd_get_pixel_width(void);

/**
 * @param width should be in bits, i.e. 8, 9, 12, 16 or 18
 */
extern void lcd_set_pixel_width(int width);

/**
 * @param p_mode Pointer to LCD mode structure which will be filled in
 */
extern void lcd_get_mode(struct lcd_mode_t *p_mode);

/**
 * @param p_ver Pointer to LCD version structure which will be filled in
 */
extern void lcd_get_version(struct lcd_ver_t *p_ver);

/**
 * Paints a solid rectangle to the LCD in black.
 *
 * @param x1 the starting column
 * @param x2 the end column
 * @param y1 the starting row
 * @param y2 the end row
 */
extern void lcd_paint_clear_rectangle(
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2
);

/**
 * Paints a solid rectangle to the LCD in the given colour.
 *
 * @param bg the RGB colour for all the pixels
 * @param x1 the starting column
 * @param x2 the end column
 * @param y1 the starting row
 * @param y2 the end row
 */
extern void lcd_paint_fill_rectangle(
    uint32_t bg,
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2
);

#define lcd_paint_pixel(col, x, y) lcd_paint_fill_rectangle(col, x, x, y, y)

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
extern void lcd_paint_mono_rectangle(
    uint32_t fg,
    uint32_t bg,
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2,
    const uint8_t *p_pixels
);

/**
 * Paints a full-colour rectangle to the LCD. This is useful for graphics but
 * you need 510KB for a full-screen image.
 * 
 * @param x1 the starting column
 * @param x2 the end column
 * @param y1 the starting row
 * @param y2 the end row
 * @param p_pixels Run-length encoded pixel values, where the count is in the top byte
 */
extern void lcd_paint_colour_rectangle(
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2,
    const uint32_t *p_rle_pixels
);

#ifdef __cplusplus
}
#endif

#endif /* ndef LCD_H_ */

/**************************************************
* End of file
***************************************************/

