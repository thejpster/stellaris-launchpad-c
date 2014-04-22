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
#include "drivers/lcd/lcd.h"
#include "font/font.h"
#include "menu/menu.h"

/**************************************************
* Defines
***************************************************/

/* Levels of nested menus allowed */
#define MENU_DEPTH 3

/* Offset of menu text from screen edge */
#define MENU_INSET 10

/**************************************************
* Data Types
**************************************************/

/* None */

/**************************************************
* Function Prototypes
**************************************************/

static bool handle_enter(void);

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Private Data
**************************************************/

static const struct menu_t *menu_levels[MENU_DEPTH] = { 0 };
static size_t current_level;
static int current_item;

/**************************************************
* Public Functions
***************************************************/

/*
 * Display the given menu structure, starting with the top item highlighted.
 */
void menu_init(const struct menu_t *p_menu)
{
    memset(menu_levels, 0, sizeof(menu_levels));
    menu_levels[0] = p_menu;
    current_level = 0;
    current_item = 0;
}

/*
 * Feed a keypress into the menu system.
 */
void menu_keypress(enum menu_keypress_t keypress)
{
    bool redraw_required = true;
    bool blank_required = false;
    const struct menu_t *p_menu = menu_levels[current_level];
    /* All menus have one more item than actually specified - the 'Back'
     * menu item */
    switch(keypress)
    {
    case MENU_KEYPRESS_UP:
        if (current_item == 0)
        {
            current_item = p_menu->num_items;
        }
        else
        {
            current_item -= 1;
        }
        break;
    case MENU_KEYPRESS_DOWN:
        if (current_item == p_menu->num_items)
        {
            /* Wrap to top */
            current_item = 0;
        }
        else
        {
            current_item += 1;
        }
        break;
    case MENU_KEYPRESS_ENTER:
        redraw_required = handle_enter();
        blank_required = true;
        break;
    }
    if (redraw_required)
    {
        menu_redraw(blank_required);
    }
}

/*
 * Redraw the menu in its current state
 */
void menu_redraw(bool blank_screen)
{
    const struct menu_t *p_menu = menu_levels[current_level];
    lcd_row_t y = 0;
    PRINTF("Drawing menu '%s'\n", p_menu->p_title);
    if (blank_screen)
    {
        lcd_paint_clear_screen();
    }
    font_draw_text_small(MENU_INSET, y, p_menu->p_title, LCD_BLUE, LCD_BLACK, false);
    lcd_paint_fill_rectangle(LCD_BLUE, LCD_FIRST_COLUMN, LCD_LAST_COLUMN, 20, 20);
    y += 24;
    for(size_t draw_item = 0; draw_item < p_menu->num_items; draw_item++)
    {
        const struct menu_item_t *p_menu_item = &(p_menu->p_menu_items[draw_item]);
        PRINTF("%c %s\n", (draw_item == current_item) ? '*' : ' ', p_menu_item->p_label);
        if (draw_item == current_item)
        {
            font_draw_text_small(MENU_INSET, y, p_menu_item->p_label, LCD_BLACK, LCD_BLUE, false);
        }
        else
        {
            font_draw_text_small(MENU_INSET, y, p_menu_item->p_label, LCD_BLUE, LCD_BLACK, false);
        }
        y += 20;
    }
    PRINTF("%c Back\n", (p_menu->num_items == current_item) ? '*' : ' ');
    if (p_menu->num_items == current_item)
    {
        font_draw_text_small(MENU_INSET, y, "Back", LCD_BLACK, LCD_BLUE, false);
    }
    else
    {
        font_draw_text_small(MENU_INSET, y, "Back", LCD_BLUE, LCD_BLACK, false);
    }
}

/*
 * Reset the menu's state - back to the top item in the top menu.
 */
void menu_reset(void)
{
    current_level = 0;
    current_item = 0;
    menu_redraw(true);
}

/**************************************************
* Private Functions
***************************************************/

static bool handle_enter(void)
{
    bool redraw_required = true;
    const struct menu_t *p_menu = menu_levels[current_level];
    const struct menu_item_t *p_menu_item = NULL;

    if (current_item < p_menu->num_items)
    {
        p_menu_item = &(p_menu->p_menu_items[current_item]);
        switch(p_menu_item->type)
        {
        case MENU_ITEM_TYPE_SUBMENU:
            if (current_level < MENU_DEPTH)
            {
                current_level +=1;
                current_item = 0;
                menu_levels[current_level] = p_menu_item->p_submenu;
            }
            break;
        case MENU_ITEM_TYPE_ACTION:
            if (p_menu_item->p_fn)
            {
                redraw_required = p_menu_item->p_fn(p_menu, p_menu_item);
            }
            break;
        }
    }
    else
    {
        /* 'Back' was selected */
        if (current_level)
        {
            current_level--;
            redraw_required = true;
        }
        else
        {
            redraw_required = false;
        }
        current_item = 0;
        if (p_menu->p_back)
        {
            /* Back function can prevent redraw, but not force it */
            bool result = p_menu->p_back(p_menu, NULL);
            redraw_required = redraw_required && result;
        }
    }
    return redraw_required;
}

/**************************************************
* End of file
***************************************************/

