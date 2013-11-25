/*****************************************************
*
* Stellaris Launchpad LCD Driver
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

/**************************************************
* Includes
***************************************************/

#include "misc/misc.h"
#include "gpio/gpio.h"
#include "../lcd.h"

/**************************************************
* Defines
***************************************************/

/* Really conservative default */
#define STROBE_DELAY 10 /* 100 = 28.1 us */
#define STROBE_READ_DELAY 1000

/* WR = pin E2 */
#define STROBE_WR() do { \
        CLEAR_BITS(GPIO_PORTE_DATA_R, (1 << 2)); \
        busy_sleep(STROBE_DELAY); \
        SET_BITS(GPIO_PORTE_DATA_R, (1 << 2)); \
    } while(0)

#define SET_COMMAND()  CLEAR_BITS(GPIO_PORTD_DATA_R, (1 << 3))
#define SET_DATA()     SET_BITS(GPIO_PORTD_DATA_R, (1 << 3))

#define SET_CS()       CLEAR_BITS(GPIO_PORTD_DATA_R, (1 << 2))
#define CLEAR_CS()     SET_BITS(GPIO_PORTD_DATA_R, (1 << 2))

#define SET_RD()       CLEAR_BITS(GPIO_PORTE_DATA_R, (1 << 1))
#define CLEAR_RD()     SET_BITS(GPIO_PORTE_DATA_R, (1 << 1))

/**************************************************
* Data Types
**************************************************/

/* None */

/**************************************************
* Function Prototypes
**************************************************/

static void set_region(
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2
);
static void send_command(uint8_t command);
static void send_data(uint8_t data);
static uint8_t read_data(void);
static void send_byte(uint8_t byte);

/**************************************************
* Public Data
**************************************************/

/* None */

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
    /* /CS and /RS */
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_D, 2), 1);
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_D, 3), 1);
    /* /RD and /WR and /RST */
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_E, 1), 1);
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_E, 2), 1);
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_E, 3), 1);

    /* 8-bit data bus */
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_D, 0), 1);
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_D, 1), 1);
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_A, 2), 1);
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_A, 3), 1);
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_A, 4), 1);
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_A, 5), 1);
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_A, 6), 1);
    gpio_make_output(GPIO_MAKE_IO_PIN(GPIO_PORT_A, 7), 1);

    /* @todo Do an LCD comms test here? */

    return 0;
}

/**
 * @return width in bits, i.e. 8, 9, 12, 16 or 18
 */
int lcd_get_pixel_width(void)
{
    int result = 0;
    SET_CS();
    send_command(0xF1);
    result = read_data();
    CLEAR_CS();
    switch (result)
    {
    case 0x00:
        result = 8;
        break;
    case 0x01:
        result = 12;
        break;
    case 0x02:
        result = -16;
        break;
    case 0x03:
        result = 16;
        break;
    case 0x04:
        result = 18;
        break;
    case 0x06:
        result = 9;
        break;
    default:
        result = -1;
        break;
    }
    return result;
}

/**
 * @param width should be in bits, i.e. 8, 9, 12, 16 or 18
 */
void lcd_set_pixel_width(int width)
{
    SET_CS();
    send_command(0xF0);
    switch (width)
    {
    case 18:
        send_data(0x04);
        break;
    case 16:
        send_data(0x03);
        break;
    case 12:
        send_data(0x01);
        break;
    case 9:
        send_data(0x06);
        break;
    default:
    case 8:
        send_data(0x00);
        break;
    }
    CLEAR_CS();
}

/**
 * @param p_mode Pointer to LCD mode structure which will be filled in
 */
void lcd_get_mode(struct lcd_mode_t *p_mode)
{
    uint8_t temp = 0;
    SET_CS();
    send_command(0xB1);

    temp = read_data();
    p_mode->colour_enhancement = (temp & (1 << 4)) ? 1 : 0;
    p_mode->frc = (temp & (1 << 3)) ? 1 : 0;
    p_mode->lshift_rising_edge = (temp & (1 << 2)) ? 1 : 0;
    p_mode->horiz_active_high = (temp & (1 << 1)) ? 1 : 0;
    p_mode->vert_active_high = (temp & (1 << 0)) ? 1 : 0;

    temp = read_data();
    p_mode->tft_type = ((temp >> 5) & 0x03);

    temp = read_data();
    p_mode->horiz_pixels = temp << 8;
    temp = read_data();
    p_mode->horiz_pixels |= temp;

    temp = read_data();
    p_mode->vert_pixels = temp << 8;
    temp = read_data();
    p_mode->vert_pixels |= temp;

    temp = read_data();
    p_mode->even_sequence = (temp >> 3) & 0x07;
    p_mode->odd_sequence = (temp >> 0) & 0x07;

    CLEAR_CS();
}

/**
 * @param p_ver Pointer to LCD version structure which will be filled in
 */
void lcd_get_version(struct lcd_ver_t *p_ver)
{
    uint8_t temp = 0;
    SET_CS();
    send_command(0xA1); /* Read Data Descriptor Block / DDB */

    temp = read_data();
    p_ver->supplier_id = temp << 8;
    temp = read_data();
    p_ver->supplier_id |= temp;

    temp = read_data();
    p_ver->product_id = temp;

    temp = read_data();
    p_ver->revision = temp;

    /* Should be 0xFF */
    read_data();

    CLEAR_CS();
}

/**
 * Paints a solid rectangle to the LCD in black.
 *
 * @param x1 the starting column
 * @param x2 the end column
 * @param y1 the starting row
 * @param y2 the end row
 */
void lcd_paint_clear_rectangle(
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2
)
{
    SET_CS();
    set_region(x1, x2, y1, y2);
    CLEAR_CS();
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
    size_t size = (1 + x2 - x1) * (1 + y2 - y1);
    SET_CS();
    set_region(x1, x2, y1, y2);
    while (size--)
    {
        /* This is for 8 bit mode */
        send_data(bg >> 16);
        send_data(bg >> 8);
        send_data(bg >> 0);
    }
    CLEAR_CS();
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
    size_t size = (1 + x2 - x1) * (1 + y2 - y1);
    uint8_t temp = *p_pixels;
    int count = 0;
    SET_CS();
    set_region(x1, x2, y1, y2);
    while (size--)
    {
        if (temp & 0x80)
        {
            send_data(fg >> 16);
            send_data(fg >> 8);
            send_data(fg >> 0);
        }
        else
        {
            send_data(bg >> 16);
            send_data(bg >> 8);
            send_data(bg >> 0);
        }
        if (count == 7)
        {
            /* Clocked out last pixel in this byte */
            p_pixels++;
            temp = *p_pixels;
            count = 0;
        }
        else
        {
            count += 1;
            temp <<= 1;
        }
    }
    CLEAR_CS();
}

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
void lcd_paint_colour_rectangle(
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2,
    const uint32_t *p_rle_pixels
)
{
    size_t size = (1 + x2 - x1) * (1 + y2 - y1);
    SET_CS();
    set_region(x1, x2, y1, y2);
    while (size)
    {
        uint32_t pixel = *p_rle_pixels;
        uint8_t count = pixel >> 24;
        size -= count;
        while (count--)
        {
            send_data(pixel >> 16);
            send_data(pixel >> 8);
            send_data(pixel >> 0);
        }
        p_rle_pixels++;
    }
    CLEAR_CS();
}

/**************************************************
* svate Functions
***************************************************/

static void set_region(
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2
)
{
    /* set column */
    send_command(0x2a);
    send_data(x1 >> 8);
    send_data(x1);
    send_data(x2 >> 8);
    send_data(x2);
    /* set row */
    send_command(0x2b);
    send_data(y1 >> 8);
    send_data(y1);
    send_data(y2 >> 8);
    send_data(y2);
    /* start write */
    send_command(0x2c);
}

static void send_command(
    uint8_t command
)
{
    SET_COMMAND();
    send_byte(command);
    STROBE_WR();
}

static void send_data(
    uint8_t data
)
{
    SET_DATA();
    send_byte(data);
    STROBE_WR();
}

static uint8_t read_data(
    void
)
{
    uint8_t result = 0;
    SET_DATA();
    SET_RD();
    busy_sleep(STROBE_READ_DELAY);
    result = (GPIO_PORTD_DATA_R & 0x03);
    result |= (GPIO_PORTA_DATA_R & 0xFC);
    CLEAR_RD();
    return result;
}

static void send_byte(
    uint8_t byte
)
{
    GPIO_PORTD_DATA_R = (GPIO_PORTD_DATA_R & 0xFC) | (byte & 0x03);
    GPIO_PORTA_DATA_R = (byte & 0xFC) | (GPIO_PORTA_DATA_R & 0x03);
}

/**************************************************
* End of file
***************************************************/

