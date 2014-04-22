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
* Utility functions and standard standard libraries.
* 
*****************************************************/

#ifndef UTIL_UTIL_H
#define UTIL_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************
* Includes
***************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/**************************************************
* Public Defines
***************************************************/

#define NUMELTS(x) ( sizeof(x) / sizeof((x)[0]) )

#define CLEAR_BITS(reg, bits) do { reg &= ~(bits); } while (0)

#define SET_BITS(reg, bits) do { reg |= (bits); } while (0)

#ifdef USE_IPRINTF
#define PRINTF(...) iprintf(__VA_ARGS__)
#else
#define PRINTF(...) printf(__VA_ARGS__)
#endif

#define MIN(x,y) ((x) < (y) ? (x) : (y))

/**************************************************
* Public Data Types
**************************************************/

/* None */

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Public Function Prototypes
***************************************************/

/**
 * Calibrated (ish) sleep functions.
 */
void delay_ms(uint32_t delay);
void delay_us(uint32_t delay);

#ifdef __cplusplus
}
#endif

#endif /* ndef UTIL_UTIL_H */

/**************************************************
* End of file
***************************************************/

