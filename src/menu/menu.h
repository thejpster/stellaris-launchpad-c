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
* Draws a nice menu on the LCD.
*
*****************************************************/

#ifndef MENU_H
#define MENU_H

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

/* None */

/**************************************************
* Public Data Types
**************************************************/

enum menu_keypress_t {
    MENU_KEYPRESS_ENTER,
    MENU_KEYPRESS_UP,
    MENU_KEYPRESS_DOWN
};

enum menu_item_type_t {
    MENU_ITEM_TYPE_SUBMENU,
    MENU_ITEM_TYPE_ACTION,
};

/* Forward declare, to allow mutual reference */
struct menu_item_t;
struct menu_t;

/*
 * @return true if menu should be redraw, false otherwise
 */
typedef bool (*menu_action_t)(
    const struct menu_t *p_menu,
    const struct menu_item_t *p_menu_item
);

struct menu_item_t {
    const char *p_label;
    enum menu_item_type_t type;
    const struct menu_t *p_submenu;
    menu_action_t p_fn;
};

struct menu_t {
    const char* p_title;
    size_t num_items;
    /* Function called when menu closed. */
    menu_action_t p_back;
    const struct menu_item_t* p_menu_items;
};

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Public Function Prototypes
***************************************************/

/*
 * Display the given menu structure, starting with the top item highlighted.
 */
void menu_init(const struct menu_t *p_menu);

/*
 * Feed a keypress into the menu system.
 */
void menu_keypress(enum menu_keypress_t keypress);

/*
 * Redraw the menu in its current state
 */
void menu_redraw(bool blank_screen);

/*
 * Reset the menu's state - back to the top item in the top menu.
 */
void menu_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* ndef MENU_H */

/**************************************************
* End of file
***************************************************/

