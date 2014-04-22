/*****************************************************
*
* Stellaris Launchpad LCD Driver
*
* Driver suits SSD1961 with self-initialisation, e.g.
* DisplayTech INT043BTFT.
*
* See https://www.displaytech-us.com/4-3-inch-integrated-tft-driver-boards
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
*****************************************************/

#ifndef LCD_H_
#define LCD_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************
* Includes
***************************************************/

#include "util/util.h"

/**************************************************
* Public Defines
***************************************************/

/* r, b and b should be 0x00..0xFF */
#define MAKE_COLOUR(r, g, b) ( ((r) << 16) | ((g) << 8) | ((b) << 0) )
/* count should also be 0x00..0xFF */
#define MAKE_RLE_COLOUR(count, r, g, b) ( ((count) << 24) | ((r) << 16) | ((g) << 8) | ((b) << 0) )

/* Colours */
#define LCD_BLACK       MAKE_COLOUR(0x00, 0x00, 0x00)
#define LCD_RED         MAKE_COLOUR(0xFF, 0x00, 0x00)
#define LCD_GREEN       MAKE_COLOUR(0x00, 0xFF, 0x00)
#define LCD_BLUE        MAKE_COLOUR(0x00, 0x00, 0xFF)
#define LCD_YELLOW      MAKE_COLOUR(0xFF, 0xFF, 0x00)
#define LCD_CYAN        MAKE_COLOUR(0x00, 0xFF, 0xFF)
#define LCD_MAGENTA     MAKE_COLOUR(0xFF, 0x00, 0xFF)
#define LCD_WHITE       MAKE_COLOUR(0xFF, 0xFF, 0xFF)

#define LCD_RED_DIM     MAKE_COLOUR(0x80, 0x00, 0x00)
#define LCD_GREEN_DIM   MAKE_COLOUR(0x00, 0x80, 0x00)
#define LCD_BLUE_DIM    MAKE_COLOUR(0x00, 0x00, 0x80)
#define LCD_YELLOW_DIM  MAKE_COLOUR(0x80, 0x80, 0x00)
#define LCD_CYAN_DIM    MAKE_COLOUR(0x00, 0x80, 0x80)
#define LCD_MAGENTA_DIM MAKE_COLOUR(0x80, 0x00, 0x80)
#define LCD_GREY        MAKE_COLOUR(0x80, 0x80, 0x80)

/* LCD sizes */
#define LCD_WIDTH 480
#define LCD_HEIGHT 272

#define LCD_FIRST_COLUMN 0
#define LCD_FIRST_ROW 0
#ifdef LCD_ROTATE_DISPLAY
#define LCD_LAST_COLUMN (LCD_HEIGHT-1)
#define LCD_LAST_ROW (LCD_WIDTH-1)
#else
#define LCD_LAST_COLUMN (LCD_WIDTH-1)
#define LCD_LAST_ROW (LCD_HEIGHT-1)
#endif

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

enum lcd_pixel_width_t
{
    LCD_PIXEL_WIDTH_8 = 0,
    LCD_PIXEL_WIDTH_12 = 1,
    LCD_PIXEL_WIDTH_16_888 = 2,
    LCD_PIXEL_WIDTH_16_565 = 3,
    LCD_PIXEL_WIDTH_18 = 4,
    LCD_PIXEL_WIDTH_9 = 6
};

typedef unsigned int lcd_row_t;
typedef unsigned int lcd_col_t;

/* Stores NRGB where N is an RLE number of pixels (optional) */
typedef uint32_t lcd_colour_t;

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
    uint8_t check_value; /* Should be FF */
};

struct lcd_period_t
{
    uint16_t total;
    uint16_t display_start;
    uint8_t sync_pulse_width;
    uint16_t sync_pulse_start;
};

enum lcd_dbc_mode_t
{
    LCD_DBC_MODE_AGGRESSIVE = 3,
    LCD_DBC_MODE_NORMAL = 2,
    LCD_DBC_MODE_CONSERVATIVE = 1,
    LCD_DBC_MODE_OFF = 0
};

struct lcd_dbc_conf_t
{
    bool dbc_manual_brightness;
    bool transition_effect;
    enum lcd_dbc_mode_t mode;
    bool master_enable;
};

/* All fields power on reset to false */
struct lcd_address_mode_t
{
    /* Controls order pixels written to framebuffer. false = top to bottom,
       true = bottom to top */
    bool page_address_order;

    /* Controls order pixels written to framebuffer. false = left to right,
       true = right to left */
    bool column_address_order;

    /* Controls order pixels written to framebuffer. If true, swaps both X
       and Y */
    bool page_column_address_order;

    /* This bit controls the display panel’s horizontal line refresh order.
       The image shown on the display panel is unaffected, regardless of the
       bit setting. */
    bool line_address_order;

    /* This bit controls the RGB data order transferred from the SSD1961’s
       frame buffer to the display panel. */
    bool bgr_order;

    /* This bit controls the display panel’s vertical line data latch order.
    */
    bool display_data_latch_data;

    /* This bit flips the image shown on the display panel left to right. No
       change is made to the frame buffer. */
    bool flip_horizontal;

    /* This bit flips the image shown on the display panel top to bottom. No
       change is made to the frame buffer. */
    bool flip_vertical;
};

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
 * DATA00 = D0 (J3.03)
 * DATA01 = D1 (J3.04)
 * DATA02 = A2 (J2.10)
 * DATA03 = A3 (J2.09)
 * DATA04 = A4 (J2.08)
 * DATA05 = A5 (J1.08)
 * DATA06 = A6 (J1.09)
 * DATA07 = A7 (J1.10)
 *
 * In the future, I might add 16-bit mode.
 *
 */
extern int lcd_init(void);

/* Make all pins inputs */
extern void lcd_deinit(void);

/**
 * @return width data bus width for pixel data
 */
extern enum lcd_pixel_width_t lcd_get_pixel_width(void);

/**
 * @param width the new desired bus width (for pixel data only, not commands).
 */
extern void lcd_set_pixel_width(enum lcd_pixel_width_t width);

/**
 * @param p_mode Pointer to LCD mode structure which will be filled in
 */
extern void lcd_get_mode(struct lcd_mode_t *p_mode);

/**
 * @param p_ver Pointer to LCD version structure which will be filled in
 */
extern void lcd_get_version(struct lcd_ver_t *p_ver);

/**
 * @param p_period Pointer to LCD period structure which will be filled in
 * with details of horizontal frame sync.
 */
extern void lcd_get_horiz_period(struct lcd_period_t *p_period);

/**
 * @param p_period Pointer to LCD period structure which will be filled in
 * with details of vertical frame sync.
 */
extern void lcd_get_vert_period(struct lcd_period_t *p_period);

/**
 * Enables LCD display.
 */
extern void lcd_on(void);

/**
 * Disables LCD display.
 */
extern void lcd_off(void);

/**
 * Gets Dynamic Brightness Control settings.
 * @param p_dbc Pointer to DBC structure which will be filled in.
 * with details of DBC.
 */
extern void lcd_get_dbc_conf(struct lcd_dbc_conf_t *p_dbc);

/**
 * Sets Dynamic Brightness Control settings.
 * @param p_dbc Pointer to DBC structure which will be read and passed
 * to LCD.
 */
extern void lcd_set_dbc_conf(const struct lcd_dbc_conf_t *p_dbc);

/**
 * Gets frame buffer / display mapping (aka 'address mode').
 * @param p_dbc Pointer to data structure which will be filled in.
 * with details of mapping.
 */
extern void lcd_get_address_mode(struct lcd_address_mode_t *p_mode);

/**
 * Sets frame buffer / display mapping (aka 'address mode').
 * @param p_dbc Pointer to data structure which will be read and passed to
 * LCD.
 */
extern void lcd_set_address_mode(const struct lcd_address_mode_t *p_mode);

/**
 * Sets the LCD backlight brightness.
 * @param brightness 0xFF is maximum brightness, 0x00 is minimum.
 */
extern void lcd_set_backlight(uint8_t brightness);

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
    lcd_colour_t bg,
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2
);

#define lcd_paint_pixel(col, x, y) lcd_paint_fill_rectangle(col, x, x, y, y)

#define lcd_paint_clear_screen() lcd_paint_fill_rectangle( \
        LCD_BLACK, \
        LCD_FIRST_COLUMN, LCD_LAST_COLUMN, \
        LCD_FIRST_ROW, LCD_LAST_ROW)

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
    lcd_colour_t fg,
    lcd_colour_t bg,
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
    const lcd_colour_t *p_rle_pixels
);

extern void lcd_read_color_rectangle(
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2,
    lcd_colour_t *p_rle_pixels,
    size_t pixel_len
);

#ifdef __cplusplus
}
#endif

#endif /* ndef LCD_H_ */

/**************************************************
* End of file
***************************************************/

