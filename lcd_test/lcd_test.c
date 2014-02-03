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
#include "drivers/lcd/lcd.h"

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

/* None */

/**************************************************
* Public Data
**************************************************/

FILE* f;

/**************************************************
* Private Data
**************************************************/

/* None */

/**************************************************
* Public Functions
***************************************************/

int main(int argc, char* argv[])
{
	f = fopen("lcd_fifo", "w");
	lcd_init();
	for(int i = 0; i < 100; i++)
	{
		font_draw_number_large(
			0, 0,
			i,
			3,
			LCD_BLUE,
			LCD_BLACK
			);
		delay_ms(250);
	}
}

void delay_ms(uint32_t milliseconds)
{
	struct timeval t;
	t.tv_sec = milliseconds / 1000;
	t.tv_usec = (milliseconds % 1000) * 1000;
	select(0, NULL, NULL, NULL, &t);
}

/**************************************************
* Private Functions
***************************************************/

/* None */

/**************************************************
* End of file
***************************************************/

