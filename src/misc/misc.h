/*****************************************************
*
* Stellaris Launchpad Example Project
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
* References:
*
*     [1] - Stellaris® LM4F121H5QR Microcontroller
*           Data Sheet
*****************************************************/

#ifndef MISC_MISC_H_
#define MISC_MISC_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************
* Includes
***************************************************/

#include <stdint.h>
#include "misc/lm4f120h5qr.h"

/**************************************************
* Public Defines
***************************************************/

#define NUMELTS(x) ( sizeof(x) / sizeof((x)[0]) )

#define CLEAR_BITS(reg, bits) do { reg &= ~(bits); } while (0)

#define SET_BITS(reg, bits) do { reg |= (bits); } while (0)

#define printf iprintf

/**************************************************
* Public Data Types
**************************************************/

typedef volatile unsigned long reg_t;

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Public Function Prototypes
***************************************************/

/**
 * Set system clock to a CLOCK_RATE
 */
extern void set_clock(void);

/**
 * Rough and ready sleep function.
 */
extern void busy_sleep(uint32_t delay);

/**
 * Enable an interrupt. See table 2-9 in [1].
 */
extern void enable_interrupt(unsigned int interrupt_id);

/**
 * Disable an interrupt. See table 2-9 in [1].
 */
extern void disable_interrupt(unsigned int interrupt_id);

#ifdef __cplusplus
}
#endif

#endif /* ndef MISC_MISC_H_ */

/**************************************************
* End of file
***************************************************/

