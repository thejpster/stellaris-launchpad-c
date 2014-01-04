/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Stubs required to make libc link. See
* ../../gcc-arm-none-eabi-4_6-2012q4/share/doc/html/libc/Stubs.html#Stubs
*
* Copyright (c) 2012 theJPster (www.thejpster.org.uk)
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

#include <sys/stat.h>

#include "drivers/misc/misc.h"
#include "drivers/gpio/gpio.h"
#include "drivers/uart/uart.h"

/**************************************************
* Defines
***************************************************/

/* None */

/**************************************************
* Function Prototypes
**************************************************/

/* None */

/**************************************************
* Data Types
**************************************************/

/* None */

/**************************************************
* Public Data
**************************************************/

extern char _heap_bottom;       /* Defined by the linker */
extern char _heap_top;          /* Defined by the linker */

/**************************************************
* Private Data
**************************************************/

char *heap_end;

/**************************************************
* Public Functions
***************************************************/

/*
 * To use the arm-none-eabi C library, we need to supply this
 */
void _exit(int status)
{
    while(1)
    {
        /* Do nothing */
    }
}

/*
 * To use the arm-none-eabi C library, we need to supply this.
 * It allows the heap to grow and shrink.
 */
caddr_t _sbrk(int incr) {
    char *prev_heap_end;

    if (heap_end == 0) {
      heap_end = &_heap_bottom;
    }
    prev_heap_end = heap_end;
    if ((heap_end + incr) > &_heap_top) {
        /* Flash the red LED to signify heap exhaustion */
        gpio_flash_error(LED_RED, 0, 500);
    }

    heap_end += incr;
    return (caddr_t) prev_heap_end;
}

/*
 * Allows us to use printf and write to stdout.
 */
int _write(int file, char *ptr, int len) {
	if (file == 1)
	{
		uart_write(UART_ID_0, ptr, len);
	}
	return len;
}

int _isatty(int file) {
	return 1;
}

int _open(const char *name, int flags, int mode) {
	return -1;
}

int _close(int file)
{
	return -1;
}

int _read(int file, char *ptr, int len) {
	return 0;
}

int _lseek(int file, int ptr, int dir) {
	return 0;
}

int _fstat(int file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

/**************************************************
* Private Functions
***************************************************/

/* None */

/**************************************************
* End of file
***************************************************/

