/*****************************************************
*
* Stellaris Launchpad Example Project
*
* A basic UART driver.
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

#ifndef UART_UART_H_
#define UART_UART_H_

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

#define UART_OK                       0
#define UART_ERROR_INVALID_ID        -1
#define UART_ERROR_INVALID_BAUDRATE  -2
#define UART_ERROR_UNKNOWN           -3
#define UART_ERROR_INVALID_PARITY    -4
#define UART_ERROR_INVALID_DATABITS  -5
#define UART_ERROR_INVALID_STOPPBITS -6
#define UART_ERROR_INTERRUPT_MODE    -7

/**************************************************
* Public Data Types
**************************************************/

typedef enum uart_id_t
{
    UART_ID_0,
    UART_ID_1,
    UART_ID_2,
    UART_ID_3,
    UART_ID_4,
    UART_ID_5,
    UART_ID_6,
    UART_ID_7,
    NUM_UARTS
} uart_id_t;

typedef enum uart_parity_t
{
    UART_PARITY_NONE,
    UART_PARITY_EVEN,
    UART_PARITY_ODD
} uart_parity_t;

typedef enum uart_databits_t
{
    UART_DATABITS_5,
    UART_DATABITS_6,
    UART_DATABITS_7,
    UART_DATABITS_8
} uart_databits_t;

typedef enum uart_stopbits_t
{
    UART_STOPBITS_1,
    UART_STOPBITS_2
} uart_stopbits_t;

typedef unsigned int uart_baudrate_t;

typedef void (*uart_callback_fn_t)(uart_id_t uart_id, const char* buffer, size_t buffer_size);

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Public Function Prototypes
***************************************************/

/**
 * @param uart_id   The UART to initialise
 * @param cbfn      This function will be called from interrupt context with
 *                  the data received.
 */
extern int uart_init(
    uart_id_t uart_id,
    uart_baudrate_t baud_rate,
    uart_parity_t parity,
    uart_databits_t databits,
    uart_stopbits_t stopbits,
    uart_callback_fn_t cbfn
    );

/**
 * @return the number of bytes written or, if -ve, an error
 */
extern ssize_t uart_read(
    uart_id_t uart_id,
    char* buffer,
    size_t buffer_size
    );

/*
 * This function will block until all the data has
 * been written.
 */
extern int uart_write(
    uart_id_t uart_id,
    const char* buffer,
    size_t buffer_size
    );

/*
 * This function will block until all the data has
 * been written.
 */
extern int uart_write_str(
    uart_id_t uart_id,
    const char* sz_buffer
    );

/*
 * This function will try and write as much as it
 * can fit in the buffer and then return.
 */
extern ssize_t uart_write_nonblock(
    uart_id_t uart_id,
    const char* buffer,
    size_t buffer_size
    );

extern void uart0_irq(void);
extern void uart1_irq(void);
extern void uart2_irq(void);
extern void uart3_irq(void);
extern void uart4_irq(void);
extern void uart5_irq(void);
extern void uart6_irq(void);
extern void uart7_irq(void);

#ifdef __cplusplus
}
#endif

#endif /* ndef UART_UART_H */

/**************************************************
* End of file
***************************************************/

