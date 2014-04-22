/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2013-2014 theJPster (www.thejpster.org.uk)
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
* This module handles the various counter/timer peripherals
* in the LM4F.
*
*****************************************************/

#ifndef TIMER_INTERRUPTS_H_
#define TIMER_INTERRUPTS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************
* Includes
***************************************************/

/* None */

/**************************************************
* Public Defines
***************************************************/

/* None */

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
extern void timer_0a_interrupt(void);
extern void timer_1a_interrupt(void);
extern void timer_2a_interrupt(void);
extern void timer_3a_interrupt(void);
extern void timer_4a_interrupt(void);
extern void timer_5a_interrupt(void);
extern void timer_0b_interrupt(void);
extern void timer_1b_interrupt(void);
extern void timer_2b_interrupt(void);
extern void timer_3b_interrupt(void);
extern void timer_4b_interrupt(void);
extern void timer_5b_interrupt(void);
extern void timer_w0a_interrupt(void);
extern void timer_w1a_interrupt(void);
extern void timer_w2a_interrupt(void);
extern void timer_w3a_interrupt(void);
extern void timer_w4a_interrupt(void);
extern void timer_w5a_interrupt(void);
extern void timer_w0b_interrupt(void);
extern void timer_w1b_interrupt(void);
extern void timer_w2b_interrupt(void);
extern void timer_w3b_interrupt(void);
extern void timer_w4b_interrupt(void);
extern void timer_w5b_interrupt(void);

#ifdef __cplusplus
}
#endif

#endif /* ndef TIMER_INTERRUPTS_H_ */

/**************************************************
* End of file
***************************************************/

