/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2014 theJPster (www.thejpster.org.uk)
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

#include "util/util.h"
#include <sys/select.h>
#include "font/font.h"
#include "menu/menu_lexgo_bonus.h"
#include "clocks/clocks.h"
#include "drivers/lcd/lcd.h"

/**************************************************
* Defines
***************************************************/

#define MIN(a,b) ((a) < (b) ? (a) : (b))

/**************************************************
* Data Types
**************************************************/

/* None */

/**************************************************
* Function Prototypes
**************************************************/

static void screen_furniture(void);
static void screen_redraw(void);

/**************************************************
* Public Data
**************************************************/

FILE* f;

/**************************************************
* Private Data
**************************************************/

static struct clocks_state_t state;

/**************************************************
* Public Functions
***************************************************/

int main(int argc, char* argv[])
{
    f = fopen("lcd_fifo", "w");
    lcd_init();
    screen_furniture();
    state.current_speed = 123;
    state.current_revs = 4000;
    state.trip[CLOCKS_ODOMETER] = 1000000;
    state.trip[CLOCKS_TRIP_TANK] = 3000;
    state.trip[CLOCKS_TRIP_JOURNEY] = 2567;
    state.trip[CLOCKS_TRIP_RUN] = 1234;
    for(int i = 0; i < 150; i+=1)
    {
        state.current_speed = i;
        state.current_revs-= 10;
        state.trip[CLOCKS_ODOMETER]++;
        state.trip[CLOCKS_TRIP_TANK]++;
        state.trip[CLOCKS_TRIP_JOURNEY]++;
        state.trip[CLOCKS_TRIP_RUN]++;
        screen_redraw();
        delay_ms(250);
    }
    menu_lexgo_bonus_init();
}

void delay_ms(uint32_t milliseconds)
{
    struct timeval t;
    t.tv_sec = milliseconds / 1000;
    t.tv_usec = (milliseconds % 1000) * 1000;
    select(0, NULL, NULL, NULL, &t);
}

void clocks_trip_reset(enum clocks_trip_t trip)
{

}

bool main_menu_close(
    const struct menu_t *p_menu,
    const struct menu_item_t *p_menu_item
)
{

}

/**************************************************
* Private Functions
***************************************************/

#define ORANGE 0xe09512
#define GREEN 0xe09512
#define BLUE 0x0072ee

#define GRAPH_BORDER 0xaaaaaa
#define GRAPH_TEXT 0xcccccc

#define SPEED_BAR_HEIGHT 20
#define SPEED_BAR_WIDTH 230
#define SPEED_BAR_INNER_BORDER 2
#define SPEED_BAR_EDGE 1
#define SPEED_BAR_X 15
#define SPEED_BAR_Y 248
#define SPEED_BAR_X2 (SPEED_BAR_X + SPEED_BAR_EDGE + SPEED_BAR_WIDTH + SPEED_BAR_EDGE - 1)
#define SPEED_BAR_Y2 (SPEED_BAR_Y + SPEED_BAR_EDGE + SPEED_BAR_HEIGHT + SPEED_BAR_EDGE - 1)
#define SPEED_BAR_MAX_SPEED 120


static void screen_furniture(void)
{
    font_draw_text_small(5, 120, "Current speed:", BLUE, LCD_BLACK, false);
    lcd_paint_fill_rectangle(
        ORANGE,
        LCD_FIRST_COLUMN, LCD_LAST_COLUMN,
        330, 331
        );
    font_draw_text_small(5, 340, "Trips:", ORANGE, LCD_BLACK, false);
    font_draw_text_small(5, 360, " Odo", ORANGE, LCD_BLACK, true);
    font_draw_text_small(5, 380, "Tank", ORANGE, LCD_BLACK, true);
    font_draw_text_small(5, 400, "Trip", ORANGE, LCD_BLACK, true);
    font_draw_text_small(5, 420, "Jrny", ORANGE, LCD_BLACK, true);
    font_draw_text_small(210, 220, "mph", BLUE, LCD_BLACK, false);

    lcd_paint_fill_rectangle(GRAPH_BORDER,
        SPEED_BAR_X, SPEED_BAR_X2,
        SPEED_BAR_Y, SPEED_BAR_Y2
        );
    lcd_paint_fill_rectangle(LCD_BLACK,
        SPEED_BAR_X + SPEED_BAR_EDGE, SPEED_BAR_X2 - SPEED_BAR_EDGE,
        SPEED_BAR_Y + SPEED_BAR_EDGE, SPEED_BAR_Y2 - SPEED_BAR_EDGE
        );

    for(unsigned int i = 0; i <= 120; i+= 10)
    {
        char buf[4];
        lcd_col_t offset = (i * SPEED_BAR_WIDTH) / SPEED_BAR_MAX_SPEED;
        lcd_paint_fill_rectangle(GRAPH_BORDER,
            SPEED_BAR_X + SPEED_BAR_EDGE + offset - 1,
            SPEED_BAR_X + SPEED_BAR_EDGE + offset,
            SPEED_BAR_Y2,
            SPEED_BAR_Y2 + 4
            );
    }
    for(unsigned int i = 0; i <= 120; i+= 30)
    {
        char buf[4];
        lcd_col_t offset = (i * SPEED_BAR_WIDTH) / SPEED_BAR_MAX_SPEED;
        sprintf(buf, "%u", i);
        size_t label_width = strlen(buf) * 16;
        font_draw_text_small(
            SPEED_BAR_X + SPEED_BAR_EDGE + offset - (label_width/2),
            SPEED_BAR_Y2 + 10,
            buf,
            GRAPH_TEXT, LCD_BLACK, false
            );
    }
}

static void screen_redraw(void)
{
    char trip_buffer[20];
    /* Do screen update here */
    font_draw_number_large(10, 140, state.current_speed, 3, LCD_WHITE, LCD_BLACK);
    for (int i = 0; i < CLOCKS_NUM_TRIPS; i++)
    {
        sprintf(trip_buffer, "%6.0lu.%u mi",
                state.trip[i] / CLOCKS_DISTANCE_SCALE,
                (int) (state.trip[i] % CLOCKS_DISTANCE_SCALE));
        font_draw_text_small(
            85, 360 + (20*i),
            trip_buffer,
            LCD_WHITE, LCD_BLACK,
            true);
    }
    lcd_col_t split = (MIN(state.current_speed, SPEED_BAR_MAX_SPEED) * SPEED_BAR_WIDTH) / SPEED_BAR_MAX_SPEED;
    uint32_t c;
    if (state.current_speed > 70)
    {
        /* Over the limit..*/
        c = 0xFF0000;
    }
    else if (state.current_speed > 50)
    {
        /* Amber..*/
        c = 0xFFFF00;
    }
    else if (state.current_speed > 30)
    {
        /* Green..*/
        c = 0x00FF00;
    }
    else
    {
        /* White..*/
        c = 0xFFFFFF;
    }
    printf("split = %u, col=%06x\n", split, c);
    lcd_paint_fill_rectangle(c,
        SPEED_BAR_X + SPEED_BAR_EDGE + SPEED_BAR_INNER_BORDER, SPEED_BAR_X + SPEED_BAR_EDGE + split - 1,
        SPEED_BAR_Y + SPEED_BAR_EDGE + SPEED_BAR_INNER_BORDER, SPEED_BAR_Y2 - SPEED_BAR_EDGE - SPEED_BAR_INNER_BORDER
        );
    lcd_paint_fill_rectangle(LCD_BLACK,
        SPEED_BAR_X + SPEED_BAR_EDGE + split, SPEED_BAR_X2 - SPEED_BAR_EDGE - SPEED_BAR_INNER_BORDER,
        SPEED_BAR_Y + SPEED_BAR_EDGE + SPEED_BAR_INNER_BORDER, SPEED_BAR_Y2 - SPEED_BAR_EDGE - SPEED_BAR_INNER_BORDER
        );
    /* @todo should set rpm here */
}

/**************************************************
* End of file
***************************************************/

