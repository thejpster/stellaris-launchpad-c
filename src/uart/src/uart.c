/*****************************************************
*
* Stellaris Launchpad Example Project
*
* A basic UART driver.
*
* Copyright (c) 2012 theJPster (www.thejpster.org.uk)
*
* See README.md for the license.
*
* References:
*
*     [1] - Stellaris® LM4F121H5QR Microcontroller
*           Data Sheet.
*           http://www.ti.com/lit/ds/symlink/lm4f120h5qr.pdf
*****************************************************/

/**************************************************
* Includes
***************************************************/

#include <stddef.h>
#include <string.h>

#include "misc/misc.h"
#include "uart/uart.h"

/**************************************************
* Defines
***************************************************/

#define FIFO_SIZE 16

/**************************************************
* Function Prototypes
**************************************************/

static void uart_irq(uart_id_t uart_id);

/**************************************************
* Data Types
**************************************************/

typedef struct uart_register_map_t
{
    volatile unsigned long DR_R;             /* 0x4xxxx000 */
    volatile unsigned long RSR_R;            /* 0x4xxxx004 */
    volatile unsigned long padding0[4];      /* 0x4xxxx008, 00C, 010, 014 */
    volatile unsigned long FR_R;             /* 0x4xxxx018 */
    volatile unsigned long padding1[1];      /* 0x4xxxx01C */
    volatile unsigned long ILPR_R;           /* 0x4xxxx020 */
    volatile unsigned long IBRD_R;           /* 0x4xxxx024 */
    volatile unsigned long FBRD_R;           /* 0x4xxxx028 */
    volatile unsigned long LCRH_R;           /* 0x4xxxx02C */
    volatile unsigned long CTL_R;            /* 0x4xxxx030 */
    volatile unsigned long IFLS_R;           /* 0x4xxxx034 */
    volatile unsigned long IM_R;             /* 0x4xxxx038 */
    volatile unsigned long RIS_R;            /* 0x4xxxx03C */
    volatile unsigned long MIS_R;            /* 0x4xxxx040 */
    volatile unsigned long ICR_R;            /* 0x4xxxx044 */
    volatile unsigned long DMACTL_R;         /* 0x4xxxx048 */
} uart_register_map_t;

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Private Data
**************************************************/

static uart_register_map_t * const uart_base[NUM_UARTS] =
{
    (uart_register_map_t*) 0x4000C000,
    (uart_register_map_t*) 0x4000D000,
    (uart_register_map_t*) 0x4000E000,
    (uart_register_map_t*) 0x4000F000,
    (uart_register_map_t*) 0x40010000,
    (uart_register_map_t*) 0x40011000,
    (uart_register_map_t*) 0x40012000,
    (uart_register_map_t*) 0x40013000
};

/* See table 2-9 in [1] */
static const int uart_int_map[NUM_UARTS] =
{
    5, // EN0 UART 0
    6, // EN0 UART 1
    33, // EN1 UART 2
    59, // EN1 UART 3
    60, // EN1 UART 4
    61, // EN1 UART 5
    62, // EN1 UART 6
    63  // EN1 UART 7
};

static uart_callback_fn_t interrupt_fn_table[NUM_UARTS];

/**************************************************
* Public Functions
***************************************************/

/**
 * @param uart_id   The UART to initialise
 * @param cbfn      This function will be called from interrupt context with
 *                  the data received.
 */
int uart_init(
    uart_id_t uart_id,
    uart_baudrate_t baud_rate,
    uart_parity_t parity,
    uart_databits_t databits,
    uart_stopbits_t stopbits,
    uart_callback_fn_t cbfn
    )
{
    if (uart_id >= NUM_UARTS)
    {
        return UART_ERROR_INVALID_ID;
    }

    /* See [1] p812 for these steps */

    /* Actually it should be the GPIO module that does all this.
     * The UART module doesn't care how its pins are muxed, just
     * that they are
     */

    /* Enable UART module in RCGUART register p306 */
    SYSCTL_RCGCUART_R |= 1 << (int) uart_id;

    /* Disable UART and all features */
    uart_base[uart_id]->CTL_R = 0;

    /* Calculate the baud rate values */

    /* baud_div = CLOCK_RATE / (16 * baud_rate); */
    /* baud_int = round(baud_div * 64) */
    unsigned int baud_int = (((CLOCK_RATE * 8) / baud_rate) + 1) / 2;

    /* Store the upper and lower parts of the divider */
    uart_base[uart_id]->IBRD_R = baud_int / 64;
    uart_base[uart_id]->FBRD_R = baud_int % 64;

    /* Calculate the UART Line Control register value */
    unsigned long ctrl = UART_LCRH_FEN;

    switch(parity)
    {
    case UART_PARITY_EVEN:
        ctrl |= UART_LCRH_EPS;
        ctrl |= UART_LCRH_PEN;
        break;
    case UART_PARITY_ODD:
        ctrl |= UART_LCRH_PEN;
        break;
    case UART_PARITY_NONE:
        break;
    default:
        return UART_ERROR_INVALID_PARITY;
    }

    switch(databits)
    {
    case UART_DATABITS_5:
        ctrl |= UART_LCRH_WLEN_5;
        break;
    case UART_DATABITS_6:
        ctrl |= UART_LCRH_WLEN_6;
        break;
    case UART_DATABITS_7:
        ctrl |= UART_LCRH_WLEN_7;
        break;
    case UART_DATABITS_8:
        ctrl |= UART_LCRH_WLEN_8;
        break;
    default:
        return UART_ERROR_INVALID_DATABITS;
    }

    if (stopbits == UART_STOPBITS_2)
    {
        ctrl |= UART_LCRH_STP2;
    }

    uart_base[uart_id]->LCRH_R = ctrl;

    /* Clear the flags */
    uart_base[uart_id]->FR_R = 0;

    /* Clock source is System clock by default */

    /* Set any interrupts */
    if (cbfn)
    {
        interrupt_fn_table[uart_id] = cbfn;
        uart_base[uart_id]->IM_R |= UART_IM_RXIM | UART_IM_RTIM;
        enable_interrupt(uart_int_map[uart_id]);
    }
    else
    {
        disable_interrupt(uart_int_map[uart_id]);
        uart_base[uart_id]->IM_R &= ~(UART_IM_RXIM | UART_IM_RTIM);
        interrupt_fn_table[uart_id] = NULL;
    }

    /* Re-enable UART */
    uart_base[uart_id]->CTL_R |= UART_CTL_RXE | UART_CTL_TXE | UART_CTL_UARTEN;

    return UART_OK;
}

/**
 * @return the number of bytes read or, if -ve, an error
 */
ssize_t uart_read(
    uart_id_t uart_id,
    char* buffer,
    size_t buffer_size
    )
{
    if (uart_id >= NUM_UARTS)
    {
        return UART_ERROR_INVALID_ID;
    }

    if (interrupt_fn_table[uart_id])
    {
        /* Can't read from a UART when rx interrupts are enabled */
        return UART_ERROR_INTERRUPT_MODE;
    }

    ssize_t read  = 0;
    while(read < buffer_size)
    {
        if ((uart_base[uart_id]->FR_R & UART_FR_RXFE) == 0)
        {
            buffer[read] = uart_base[uart_id]->DR_R & 0xFF;
            read++;
        }
        else
        {
            break;
        }
    }
    return read;
}

/*
 * This function will block until all the data has
 * been written.
 *
 * @return 0 or an error
 */
int uart_write(
    uart_id_t uart_id,
    const char* buffer,
    size_t buffer_size
    )
{
    if (uart_id >= NUM_UARTS)
    {
        return UART_ERROR_INVALID_ID;
    }

    ssize_t written = 0;
    while(written < buffer_size)
    {
        if ((uart_base[uart_id]->FR_R & UART_FR_TXFF) == 0)
        {
            uart_base[uart_id]->DR_R = buffer[written];
            written++;
        }
    }
    return UART_OK;
}

/*
 * This function will block until all the data has
 * been written.
 */
int uart_write_str(
    uart_id_t uart_id,
    const char* sz_buffer
    )
{
    size_t len = strlen(sz_buffer);
    return uart_write(uart_id, sz_buffer, len);
}

/*
 * This function will try and write as much as it
 * can fit in the buffer and then return.
 */
ssize_t uart_write_nonblock(
    uart_id_t uart_id,
    const char* buffer,
    size_t buffer_size
    )
{
    if (uart_id >= NUM_UARTS)
    {
        return UART_ERROR_INVALID_ID;
    }

    ssize_t written = 0;
    while(written < buffer_size)
    {
        if ((uart_base[uart_id]->FR_R & UART_FR_TXFF) == 0)
        {
            uart_base[uart_id]->DR_R = buffer[written];
            written++;
        }
        else
        {
            break;
        }
    }
    return written;
}

/* These are in the NVIC table in startup.c */
void uart0_irq(void)
{
    uart_irq(UART_ID_0);
}

void uart1_irq(void)
{
    uart_irq(UART_ID_1);
}

void uart2_irq(void)
{
    uart_irq(UART_ID_2);
}

void uart3_irq(void)
{
    uart_irq(UART_ID_3);
}

void uart4_irq(void)
{
    uart_irq(UART_ID_4);
}

void uart5_irq(void)
{
    uart_irq(UART_ID_5);
}

void uart6_irq(void)
{
    uart_irq(UART_ID_6);
}

void uart7_irq(void)
{
    uart_irq(UART_ID_7);
}

/**************************************************
* Private Functions
***************************************************/

void uart_irq(uart_id_t uart_id)
{
    char buffer[FIFO_SIZE];
    size_t num_chars = 0;
    if (interrupt_fn_table[uart_id])
    {
        while ((num_chars < NUMELTS(buffer)) && ((uart_base[uart_id]->FR_R & UART_FR_RXFE) == 0))
        {
            buffer[num_chars] = uart_base[uart_id]->DR_R & 0xFF;
            num_chars++;
        }
        interrupt_fn_table[uart_id](uart_id, buffer, num_chars);
    }
}

/**************************************************
* End of file
***************************************************/

