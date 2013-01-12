/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2012 theJPster (www.thejpster.org.uk)
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

#include "misc/lm4f120h5qr.h"

/**************************************************
* Public Defines
***************************************************/

#define NUMELTS(x) ( sizeof(x) / sizeof((x)[0]) )

#define CLOCK_RATE 16000000 /* see set_clock() */

#define CLEAR_BITS(reg, bits) do { reg &= ~(bits); } while (0)

#define SET_BITS(reg, bits) do { reg |= (bits); } while (0)

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
 * Set system clock to a hard-coded value of 16 MHz.
 */
extern void set_clock(void);

/**
 * Rough and ready sleep function. Approx 8 clock ticks elapse
 * per 'delay', although compiler optimisations might 
 * massively reduce this.
 */
extern void busy_sleep(unsigned long delay);

extern void enable_interrupt(int interrupt_id);

extern void disable_interrupt(int interrupt_id);

#ifdef __cplusplus
}
#endif

#endif /* ndef MISC_MISC_H_ */

/**************************************************
* End of file
***************************************************/

