/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2013 theJPster (www.thejpster.org.uk)
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
* Circular buffer example from Wikipedia (http://en.wikipedia.org/wiki/Circular_buffer).
* Keeps one slot open.
*
*****************************************************/

/**************************************************
* Includes
***************************************************/

#include "util/util.h"

#include "circbuffer/circbuffer.h"

/**************************************************
* Defines
***************************************************/

#define INC_AND_WRAP(val, size) (((val) + 1) % (size))

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

/* None */

/**************************************************
* Private Data
**************************************************/

/* None */

/**************************************************
* Public Functions
***************************************************/

/*
 * Will be able to store buffer_len-1 characters.
 */
void circbuffer_init(struct circbuffer_t *cb, uint8_t *p_buffer, size_t buffer_len)
{
    cb->size  = buffer_len; /* include empty elem */
    cb->start = 0;
    cb->end   = 0;
    cb->elems = p_buffer;
}

bool circbuffer_isfull(struct circbuffer_t *cb)
{
    return INC_AND_WRAP(cb->end, cb->size) == cb->start;
}

bool circbuffer_isempty(struct circbuffer_t *cb)
{
    return (cb->end == cb->start);
}

/* Write an element, overwriting oldest element if buffer is full. App can
   choose to avoid the overwrite by checking circbuffer_isfull(). */
void circbuffer_write(struct circbuffer_t *cb, uint8_t elem)
{
    cb->elems[cb->end] = elem;
    cb->end = INC_AND_WRAP(cb->end, cb->size);
    if (cb->end == cb->start)
    {
        cb->start = INC_AND_WRAP(cb->start, cb->size); /* full, overwrite */
    }
}

/* Read oldest element. App must ensure !circbuffer_isempty() first. */
uint8_t circbuffer_read(struct circbuffer_t *cb)
{
    uint8_t elem = cb->elems[cb->start];
    cb->start = INC_AND_WRAP(cb->start, cb->size);
    return elem;
}

/**************************************************
* Private Functions
***************************************************/

/* None */

/**************************************************
* End of file
***************************************************/

