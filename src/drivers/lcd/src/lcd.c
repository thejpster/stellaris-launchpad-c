/*****************************************************
*
* Stellaris Launchpad LCD Driver
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

/**************************************************
* Includes
***************************************************/

#include "drivers/misc/misc.h"
#include "drivers/gpio/gpio.h"
#include "../lcd.h"

/**************************************************
* Defines
***************************************************/

/* Really conservative default */
#define STROBE_READ_DELAY 100

#define STROBE_WR() \
    do { \
        GPIO_PORTE_DATA_BITS_R[GPIO_GET_PIN(LCD_WR)] = 0x00; \
        GPIO_PORTE_DATA_BITS_R[GPIO_GET_PIN(LCD_WR)] = \
            GPIO_GET_PIN(LCD_WR); \
    } while(0)

#define WRITE_BYTE_FAST(b) \
    do { \
        GPIO_PORTD_DATA_BITS_R[0x03] = b; \
        GPIO_PORTA_DATA_BITS_R[0xFC] = b; \
    } while(0)

#define WRITE_PIXEL_RGB(r, g, b) \
    do { \
        WRITE_BYTE_FAST(r); \
        STROBE_WR(); \
        WRITE_BYTE_FAST(g); \
        STROBE_WR(); \
        WRITE_BYTE_FAST(b); \
        STROBE_WR(); \
    } while(0)

#define SET_COMMAND()  gpio_set_output(LCD_COMMAND_DATA, 0)
#define SET_DATA()     gpio_set_output(LCD_COMMAND_DATA, 1)

#define SET_CS()       gpio_set_output(LCD_CS, 0)
#define CLEAR_CS()     gpio_set_output(LCD_CS, 1)

#define SET_RD()       gpio_set_output(LCD_RD, 0)
#define CLEAR_RD()     gpio_set_output(LCD_RD, 1)

#define SET_RST()       gpio_set_output(LCD_RST, 0)
#define CLEAR_RST()     gpio_set_output(LCD_RST, 1)

#define LCD_CS            GPIO_MAKE_IO_PIN(GPIO_PORT_D, 2)
#define LCD_COMMAND_DATA  GPIO_MAKE_IO_PIN(GPIO_PORT_D, 3)
/* If you change LCD_WR, change STROBE_WR too */
#define LCD_WR            GPIO_MAKE_IO_PIN(GPIO_PORT_E, 2)
/* We don't use LCD_RST or LCD_RD */
#define LCD_DATA0         GPIO_MAKE_IO_PIN(GPIO_PORT_D, 0)
#define LCD_DATA1         GPIO_MAKE_IO_PIN(GPIO_PORT_D, 1)
#define LCD_DATA2         GPIO_MAKE_IO_PIN(GPIO_PORT_A, 2)
#define LCD_DATA3         GPIO_MAKE_IO_PIN(GPIO_PORT_A, 3)
#define LCD_DATA4         GPIO_MAKE_IO_PIN(GPIO_PORT_A, 4)
#define LCD_DATA5         GPIO_MAKE_IO_PIN(GPIO_PORT_A, 5)
#define LCD_DATA6         GPIO_MAKE_IO_PIN(GPIO_PORT_A, 6)
#define LCD_DATA7         GPIO_MAKE_IO_PIN(GPIO_PORT_A, 7)

#define LCD_EN            GPIO_MAKE_IO_PIN(GPIO_PORT_E, 3)

/**************************************************
* Data Types
**************************************************/

enum ssd1963_cmd_t
{
    CMD_NOP = 0x00,
    CMD_SOFT_RESET = 0x01,
    CMD_GET_PWR_MODE = 0x0A,
    CMD_GET_ADDR_MODE = 0x0B,
    CMD_GET_DISPLAY_MODE = 0x0D,
    CMD_GET_TEAR_EFFECT_STATUS = 0x0E,
    CMD_ENT_SLEEP = 0x10,
    CMD_EXIT_SLEEP = 0x11,
    CMD_ENT_PARTIAL_MODE = 0x12,
    CMD_ENT_NORMAL_MODE = 0x13,
    CMD_EXIT_INVERT_MODE = 0x20,
    CMD_ENT_INVERT_MODE = 0x21,
    CMD_SET_GAMMA = 0x26,
    CMD_BLANK_DISPLAY = 0x28,
    CMD_ON_DISPLAY = 0x29,
    CMD_SET_COLUMN = 0x2A,
    CMD_SET_PAGE = 0x2B,
    CMD_WR_MEMSTART = 0x2C,
    CMD_RD_MEMSTART = 0x2E,
    CMD_SET_PARTIAL_AREA = 0x30,
    CMD_SET_SCROLL_AREA = 0x33,
    CMD_SET_TEAR_OFF = 0x34,
    CMD_SET_TEAR_ON = 0x35,
    CMD_SET_ADDR_MODE = 0x36,
    CMD_SET_SCROLL_START = 0x37,
    CMD_EXIT_IDLE_MODE = 0x38,
    CMD_ENT_IDLE_MODE = 0x39,
    CMD_WR_MEM_AUTO = 0x3C,
    CMD_RD_MEM_AUTO = 0x3E,
    CMD_SET_TEAR_SCANLINE = 0x44,
    CMD_GET_SCANLINE = 0x45,
    CMD_RD_DDB_START = 0xA1,
    CMD_SET_PANEL_MODE = 0xB0,
    CMD_GET_PANEL_MODE = 0xB1,
    CMD_SET_HOR_PERIOD = 0xB4,
    CMD_GET_HOR_PERIOD = 0xB5,
    CMD_SET_VER_PERIOD = 0xB6,
    CMD_GET_VER_PERIOD = 0xB7,
    CMD_SET_GPIO_CONF = 0xB8,
    CMD_GET_GPIO_CONF = 0xB9,
    CMD_SET_GPIO_VAL = 0xBA,
    CMD_GET_GPIO_STATUS = 0xBB,
    CMD_SET_POST_PROC = 0xBC,
    CMD_GET_POST_PROC = 0xBD,
    CMD_SET_PWM_CONF = 0xBE,
    CMD_GET_PWM_CONF = 0xBF,
    CMD_SET_LCD_GEN0 = 0xC0,
    CMD_GET_LCD_GEN0 = 0xC1,
    CMD_SET_LCD_GEN1 = 0xC2,
    CMD_GET_LCD_GEN1 = 0xC3,
    CMD_SET_LCD_GEN2 = 0xC4,
    CMD_GET_LCD_GEN2 = 0xC5,
    CMD_SET_LCD_GEN3 = 0xC6,
    CMD_GET_LCD_GEN3 = 0xC7,
    CMD_SET_GPIO0_ROP = 0xC8,
    CMD_GET_GPIO0_ROP = 0xC9,
    CMD_SET_GPIO1_ROP = 0xCA,
    CMD_GET_GPIO1_ROP = 0xCB,
    CMD_SET_GPIO2_ROP = 0xCC,
    CMD_GET_GPIO2_ROP = 0xCD,
    CMD_SET_GPIO3_ROP = 0xCE,
    CMD_GET_GPIO3_ROP = 0xCF,
    CMD_SET_ABC_DBC_CONF = 0xD0,
    CMD_GET_ABC_DBC_CONF = 0xD1,
    CMD_SET_DBC_THRES = 0xD4,
    CMD_GET_DBC_THRES = 0xD5,
    CMD_PLL_START = 0xE0,
    CMD_SET_PLL_MN = 0xE2,
    CMD_GET_PLL_MN = 0xE3,
    CMD_GET_PLL_STATUS = 0xE4,
    CMD_ENT_DEEP_SLEEP = 0xE5,
    CMD_SET_PCLK = 0xE6,
    CMD_GET_PCLK = 0xE7,
    CMD_SET_DATA_INTERFACE = 0xF0,
    CMD_GET_DATA_INTERFACE = 0xF1,
};

/**************************************************
* Function Prototypes
**************************************************/

static void set_region(
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2
);
static void do_command(enum ssd1963_cmd_t command, const uint8_t *p_data_out, size_t data_out_len, uint8_t *p_data_in, size_t data_in_len);
static void send_command(enum ssd1963_cmd_t command);
static void send_data(uint8_t data);
#ifdef LCD_RD
static uint8_t read_data(void);
#endif
static void make_bus_output(void);
static void make_bus_input(void);

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
    /* Turn on LCD */

    PRINTF("Wait...\n");

    delay_ms(1500);

    PRINTF("LCD init...\n");

    gpio_make_output(LCD_COMMAND_DATA, 1);
    gpio_make_output(LCD_CS, 1);
    gpio_make_output(LCD_WR, 1);
#if defined(LCD_RD)
    gpio_make_output(LCD_RD, 1);
#endif
#if defined(LCD_RST)
    gpio_make_output(LCD_RST, 1);
#endif
    make_bus_output();

    lcd_set_pixel_width(LCD_PIXEL_WIDTH_8);

#ifdef LCD_ROTATE_DISPLAY
    {
        const struct lcd_address_mode_t mode = { 
            /* Swap X and Y */
            .page_column_address_order = true,
            /* Correct inversion caused by X/Y swap */
            .flip_vertical = true
        };
        lcd_set_address_mode(&mode);
    }
#else
    {
        const struct lcd_address_mode_t mode = { 0 };
        lcd_set_address_mode(&mode);
    }
#endif

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
    gpio_make_input(LCD_COMMAND_DATA);
    gpio_make_input(LCD_CS);
    gpio_make_input(LCD_WR);
#if defined(LCD_RD)
    gpio_make_input(LCD_RD);
#endif
#if defined(LCD_RST)
    gpio_make_input(LCD_RST);
#endif

    make_bus_input();

    /* Turn off LCD */
    gpio_make_output(LCD_EN, 1);
}

/**
 * @return data bus width for pixel data
 */
enum lcd_pixel_width_t lcd_get_pixel_width(void)
{
    uint8_t data[1];
    do_command(CMD_GET_DATA_INTERFACE, NULL, 0, data, NUMELTS(data));
    return data[0];
}

/**
 * @param width the new desired bus width (for pixel data only, not commands).
 */
void lcd_set_pixel_width(enum lcd_pixel_width_t width)
{
    const uint8_t data[1] = { width };
    do_command(CMD_SET_DATA_INTERFACE, data, NUMELTS(data), NULL, 0);
}

/**
 * @param p_mode Pointer to LCD mode structure which will be filled in
 */
void lcd_get_mode(struct lcd_mode_t *p_mode)
{
    uint8_t data[7];
    do_command(CMD_GET_PANEL_MODE, NULL, 0, data, NUMELTS(data));
    p_mode->colour_enhancement = (data[0] & (1U << 4)) ? 1 : 0;
    p_mode->frc =                (data[0] & (1U << 3)) ? 1 : 0;
    p_mode->lshift_rising_edge = (data[0] & (1U << 2)) ? 1 : 0;
    p_mode->horiz_active_high =  (data[0] & (1U << 1)) ? 1 : 0;
    p_mode->vert_active_high =   (data[0] & (1U << 0)) ? 1 : 0;
    p_mode->tft_type =           (data[1] >> 5) & 0x03;
    p_mode->horiz_pixels =       (data[2] << 8) | data[3];
    p_mode->vert_pixels =        (data[4] << 8) | data[5];
    p_mode->even_sequence =      (data[6] >> 3) & 0x07;
    p_mode->odd_sequence =       (data[6] >> 0) & 0x07;
}

/**
 * @param p_ver Pointer to LCD version structure which will be filled in
 */
void lcd_get_version(struct lcd_ver_t *p_ver)
{
    uint8_t data[5];
    do_command(CMD_RD_DDB_START, NULL, 0, data, NUMELTS(data));
    p_ver->supplier_id = (data[0] << 8) | data[1];
    p_ver->product_id  = data[2];
    p_ver->revision    = data[3];
    p_ver->check_value = data[4];
}

void lcd_get_horiz_period(struct lcd_period_t *p_period)
{
    uint8_t data[8];
    do_command(CMD_GET_HOR_PERIOD, NULL, 0, data, NUMELTS(data));
    p_period->total            = (data[0] << 8) | data[1];
    p_period->display_start    = (data[2] << 8) | data[3];
    p_period->sync_pulse_width = data[4];
    p_period->sync_pulse_start = (data[5] << 8) | data[6];
}

void lcd_get_vert_period(struct lcd_period_t *p_period)
{
    uint8_t data[7];
    do_command(CMD_GET_VER_PERIOD, NULL, 0, data, NUMELTS(data));
    p_period->total            = (data[0] << 8) | data[1];
    p_period->display_start    = (data[2] << 8) | data[3];
    p_period->sync_pulse_width = data[4];
    p_period->sync_pulse_start = (data[5] << 8) | data[6];
}

void lcd_on(void)
{
    do_command(CMD_EXIT_SLEEP, NULL, 0, NULL, 0);
    /* As required by SSD1961 manual (well, it says 5ms) */
    delay_ms(10);
}

void lcd_off(void)
{
    do_command(CMD_ENT_SLEEP, NULL, 0, NULL, 0);
    /* As required by SSD1961 manual (well, it says 5ms) */
    delay_ms(10);
}

void lcd_get_dbc_conf(struct lcd_dbc_conf_t *p_dbc)
{
    uint8_t data[1];
    do_command(CMD_GET_ABC_DBC_CONF, NULL, 0, data, NUMELTS(data));
    p_dbc->dbc_manual_brightness = (data[0] >> 6) & 0x01;
    p_dbc->transition_effect     = (data[0] >> 5) & 0x01;
    p_dbc->mode                  = (data[0] >> 2) & 0x03;
    p_dbc->master_enable         = (data[0] >> 0) & 0x01;
}

void lcd_set_dbc_conf(const struct lcd_dbc_conf_t *p_dbc)
{
    uint8_t data[1] = { 0 };
    data[0] |= (p_dbc->dbc_manual_brightness << 6);
    data[0] |= (p_dbc->transition_effect << 5);
    data[0] |= (p_dbc->mode << 2);
    data[0] |= (p_dbc->master_enable << 0);
    do_command(CMD_SET_ABC_DBC_CONF, data, NUMELTS(data), NULL, 0);
}

/**
 * Gets frame buffer / display mapping (aka 'address mode').
 * @param p_dbc Pointer to data structure which will be filled in.
 * with details of mapping.
 */
void lcd_get_address_mode(struct lcd_address_mode_t *p_mode)
{
    uint8_t data[1] = { 0 };
    do_command(CMD_GET_ADDR_MODE, NULL, 0, data, NUMELTS(data));
    p_mode->page_address_order        = (data[0] >> 7) & 0x01;
    p_mode->column_address_order      = (data[0] >> 6) & 0x01;
    p_mode->page_column_address_order = (data[0] >> 5) & 0x01;
    p_mode->line_address_order        = (data[0] >> 4) & 0x01;
    p_mode->bgr_order                 = (data[0] >> 3) & 0x01;
    p_mode->display_data_latch_data   = (data[0] >> 2) & 0x01;
    p_mode->flip_horizontal           = (data[0] >> 1) & 0x01;
    p_mode->flip_vertical             = (data[0] >> 0) & 0x01;
}

/**
 * Sets frame buffer / display mapping (aka 'address mode').
 * @param p_dbc Pointer to data structure which will be read and passed to
 * LCD.
 */
void lcd_set_address_mode(const struct lcd_address_mode_t *p_mode)
{
    uint8_t data[1] = { 0 };
    data[0] |= (p_mode->page_address_order << 7);
    data[0] |= (p_mode->column_address_order << 6);
    data[0] |= (p_mode->page_column_address_order << 5);
    data[0] |= (p_mode->line_address_order << 4);
    data[0] |= (p_mode->bgr_order << 3);
    data[0] |= (p_mode->display_data_latch_data << 2);
    data[0] |= (p_mode->flip_horizontal << 1);
    data[0] |= (p_mode->flip_vertical << 0);
    do_command(CMD_SET_ADDR_MODE, data, NUMELTS(data), NULL, 0);
}

void lcd_set_backlight(uint8_t brightness)
{
    uint8_t data[6] = { 0 };
    data[0] = 0x06; /* 0x0e in SSD1963.c example from Microchip, 0x06 from UTFT */
    data[1] = brightness;
    data[2] = 0x01; /* PWM enabled */
    /* Ignore other three fields */
    do_command(CMD_SET_PWM_CONF, data, NUMELTS(data), NULL, 0);
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
    uint8_t r, g, b;
    SET_CS();
    set_region(x1, x2, y1, y2);
    send_command(CMD_WR_MEMSTART);
    r = (bg >> 16) & 0xFF;    
    g = (bg >> 8) & 0xFF;    
    b = (bg >> 0) & 0xFF;    
    while (size--)
    {
        WRITE_PIXEL_RGB(r, g, b);
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
    uint8_t f_r, f_g, f_b;
    uint8_t b_r, b_g, b_b;
    size_t bytes = size / 8;
    size_t remainder = size - (bytes * 8);

    f_r = (fg >> 16) & 0xFF;
    f_g = (fg >> 8) & 0xFF;
    f_b = (fg >> 0) & 0xFF;
    b_r = (bg >> 16) & 0xFF;
    b_g = (bg >> 8) & 0xFF;
    b_b = (bg >> 0) & 0xFF;

    SET_CS();
    set_region(x1, x2, y1, y2);
    send_command(CMD_WR_MEMSTART);
    while(bytes--)
    {
        uint8_t temp = *p_pixels;
        p_pixels++;
        for(uint8_t bit = 0; bit < 8; bit++)
        {
            if (temp & 0x80)
            {
                WRITE_PIXEL_RGB(f_r, f_g, f_b);
            }
            else
            {
                WRITE_PIXEL_RGB(b_r, b_g, b_b);
            }
            temp <<= 1;
        }
    }
    if (remainder)
    {
        uint8_t temp = *p_pixels;
        p_pixels++;
        for(uint8_t bit = 0; bit < remainder; bit++)
        {
            if (temp & 0x80)
            {
                WRITE_PIXEL_RGB(f_r, f_g, f_b);
            }
            else
            {
                WRITE_PIXEL_RGB(b_r, b_g, b_b);
            }
            temp <<= 1;
        }
    }
    CLEAR_CS();
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
    SET_CS();
    set_region(x1, x2, y1, y2);
    send_command(CMD_WR_MEMSTART);
    while (size)
    {
        uint32_t pixel = *p_rle_pixels;
        uint8_t count = (pixel >> 24) & 0xFF;
        uint8_t r = (pixel >> 16) & 0xFF;
        uint8_t g = (pixel >> 8) & 0xFF;
        uint8_t b = (pixel >> 0) & 0xFF;
        size -= count;
        while (count--)
        {
            WRITE_PIXEL_RGB(r,g,b);
        }
        p_rle_pixels++;
    }
    CLEAR_CS();
}

#ifdef LCD_RD
void lcd_read_color_rectangle(
    lcd_col_t x1,
    lcd_col_t x2,
    lcd_row_t y1,
    lcd_row_t y2,
    uint32_t *p_pixels,
    size_t pixel_len
    )
{
    size_t size = (1 + x2 - x1) * (1 + y2 - y1);
    if (size > pixel_len)
    {
        size = pixel_len;
    }
    SET_CS();
    set_region(x1, x2, y1, y2);
    send_command(CMD_RD_MEMSTART);
    while (size && pixel_len)
    {
        uint32_t pixel = 0;
        pixel = read_data() << 16;
        pixel |= read_data() << 8;
        pixel |= read_data();
        *p_pixels = pixel;
        size--;
        pixel_len--;
        p_pixels++;
    }
    CLEAR_CS();
}
#endif

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
#ifdef LCD_ROTATE_DISPLAY
    lcd_col_t temp1 = x1;
    lcd_col_t temp2 = x2;
    x1 = y1;
    x2 = y2;
    y1 = temp1;
    y2 = temp2;
#endif    
    /* set column */
    send_command(CMD_SET_COLUMN);
    send_data(x1 >> 8);
    send_data(x1);
    send_data(x2 >> 8);
    send_data(x2);
    /* set row */
    send_command(CMD_SET_PAGE);
    send_data(y1 >> 8);
    send_data(y1);
    send_data(y2 >> 8);
    send_data(y2);
}

static void do_command(
    uint8_t command,
    const uint8_t *p_data_out,
    size_t data_out_len,
    uint8_t *p_data_in,
    size_t data_in_len
)
{
    SET_CS();
    send_command(command);
    if (data_out_len)
    {
        while (data_out_len)
        {
            send_data(*p_data_out);
            data_out_len--;
            p_data_out++;
        }
    }

#ifdef LCD_RD
    if (data_in_len)
    {
        make_bus_input();
        while (data_in_len)
        {
            *p_data_in = read_data();
            data_in_len--;
            p_data_in++;
        }
        make_bus_output();
    }
#endif

    CLEAR_CS();
}

static void send_command(
    uint8_t command
)
{
    SET_COMMAND();
    WRITE_BYTE_FAST(command);
    STROBE_WR();
    SET_DATA();
}

static void send_data(
    uint8_t data
)
{
    WRITE_BYTE_FAST(data);
    STROBE_WR();
}

#ifdef LCD_RD
static uint8_t read_data(
    void
)
{
    uint8_t result = 0;
    SET_RD();
    busy_sleep(STROBE_READ_DELAY);
    result = gpio_read_inputs(GPIO_PORT_D, 0x03);
    result |= gpio_read_inputs(GPIO_PORT_A, 0xFC);
    CLEAR_RD();
    return result;
}
#endif

static void make_bus_output(void)
{
    /* 8-bit data bus */
    gpio_make_output(LCD_DATA0, 1);
    gpio_make_output(LCD_DATA1, 1);
    gpio_make_output(LCD_DATA2, 1);
    gpio_make_output(LCD_DATA3, 1);
    gpio_make_output(LCD_DATA4, 1);
    gpio_make_output(LCD_DATA5, 1);
    gpio_make_output(LCD_DATA6, 1);
    gpio_make_output(LCD_DATA7, 1);
}

static void make_bus_input(void)
{
    /* 8-bit data bus */
    gpio_make_input(LCD_DATA0);
    gpio_make_input(LCD_DATA1);
    gpio_make_input(LCD_DATA2);
    gpio_make_input(LCD_DATA3);
    gpio_make_input(LCD_DATA4);
    gpio_make_input(LCD_DATA5);
    gpio_make_input(LCD_DATA6);
    gpio_make_input(LCD_DATA7);
}

/**************************************************
* End of file
***************************************************/

