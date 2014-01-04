/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2012-2014 theJPster (www.thejpster.org.uk)
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
* Chip specific utility functions that don't fit in to any other 'driver'
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

#include "util/util.h"
#include "drivers/lm4f120h5qr.h"

/**************************************************
* Public Defines
***************************************************/

/* None */

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
 * Rough and ready sleep function that just
 * spins in a loop.
 */
extern void busy_sleep(uint32_t delay);

/**
 * Calibrated (ish) sleep functions.
 */
void delay_ms(uint32_t delay);
void delay_us(uint32_t delay);

/**
 * Enable an interrupt. See table 2-9 in [1].
 */
extern void enable_interrupt(unsigned int interrupt_id);

/**
 * Disable an interrupt. See table 2-9 in [1].
 */
extern void disable_interrupt(unsigned int interrupt_id);

/*
 * Disable all interrupts.
 *
 * Use this to create interrupt-safe critical sections. Disable interrupts for only
 * the shortest possible period of time.
 */
#define disable_interrupts() __asm("cpsid i")

/*
 * Enable interrupts again.
 */
#define enable_interrupts() __asm("cpsie i")

#ifdef __cplusplus
}
#endif

#endif /* ndef MISC_MISC_H_ */

/**************************************************
* End of file
***************************************************/

