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
*****************************************************/

#ifndef CIRCBUFFER_H
#define CIRCBUFFER_H

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

/* Circular buffer object */
struct circbuffer_t
{
    size_t    size;   /* maximum number of elements           */
    size_t    start;  /* index of oldest element              */
    size_t    end;    /* index at which to write new element  */
    uint8_t   *elems;  /* vector of elements                   */
};

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Public Function Prototypes
***************************************************/

void circbuffer_init(struct circbuffer_t *cb, uint8_t *p_buffer, size_t buffer_len);
bool circbuffer_isfull(struct circbuffer_t *cb);
bool circbuffer_isempty(struct circbuffer_t *cb);
void circbuffer_write(struct circbuffer_t *cb, uint8_t elem);
uint8_t circbuffer_read(struct circbuffer_t *cb);

#ifdef __cplusplus
}
#endif

#endif /* ndef CIRCBUFFER_H */

/**************************************************
* End of file
***************************************************/

