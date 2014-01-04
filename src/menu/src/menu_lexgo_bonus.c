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

#include "../menu.h"

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

static bool test_action(
    const struct menu_t *p_menu,
    const struct menu_item_t *p_menu_item
);

/**************************************************
* Public Data
**************************************************/

const struct menu_item_t settings_menu_items[] =
{
    { "Fangle", MENU_ITEM_TYPE_ACTION, NULL, test_action }
};

const struct menu_t settings_menu =
{
    "Settings Menu",
    NUMELTS(settings_menu_items),
    NULL,
    settings_menu_items
};

const struct menu_item_t top_menu_items[] =
{
    { "Item A", MENU_ITEM_TYPE_ACTION, NULL, test_action },
    { "Item B", MENU_ITEM_TYPE_ACTION, NULL, test_action },
    { "Settings", MENU_ITEM_TYPE_SUBMENU, &settings_menu, NULL },
};

const struct menu_t top_menu =
{
    "Lexgo Bonus",
    NUMELTS(top_menu_items),
    NULL,
    top_menu_items
};

/**************************************************
* Private Data
**************************************************/

/* None */

/**************************************************
* Public Functions
***************************************************/

void menu_lexgo_bonus_init(void)
{
    menu_init(&top_menu);
    menu_redraw(true);
}

/**************************************************
* Private Functions
***************************************************/

static bool test_action(
    const struct menu_t *p_menu,
    const struct menu_item_t *p_menu_item
)
{
    if (p_menu_item->p_label)
    {
        PRINTF("Selected menu item %s/%s\n", p_menu->p_title, p_menu_item->p_label);
    }
    else
    {
        PRINTF("Selected menu item %s/Back\n", p_menu->p_title);
    }
    /* Always redraw menu */
    return true;
}

/**************************************************
* End of file
***************************************************/

