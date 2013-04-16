/*
* Copyright (c) 2012, Mauro Scomparin
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Mauro Scomparin nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Mauro Scomparin ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Mauro Scomparin BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* File:         LM4F_startup.c.
* Author:       Mauro Scomparin <http://scompoprojects.wordpress.com>.
* Version:      1.0.0.
* Description:  LM4F120H5QR startup code.
*/

#include "uart/uart.h"
#include "gpio/gpio.h"
#include "misc/misc.h"

//-----------------------------------------------------------------------------
//                           Functions declarations
//-----------------------------------------------------------------------------

// Main should be defined on your main file so it's extern.
extern int main(void);
// rst_handler contains the code to run on reset.
void rst_handler(void);
// nmi_handler it's the code for an non maskerable interrupt.
void nmi_handler(void);
// this is just the default handler.
void empty_def_handler(void);
// this is the code for an hard fault.
void hardfault_handler(void);

//-----------------------------------------------------------------------------
//                           Variables declarations
//-----------------------------------------------------------------------------

// defined by the linker it's the stack top variable (End of ram)
extern unsigned long _stack_top;
// defined by the liker, this are just start and end marker for each section.
// .text (code)
extern unsigned long _start_text;
extern unsigned long _end_text;
// .data (data to be copied on ram)
extern unsigned long _start_data_flash;
extern unsigned long _start_data;
extern unsigned long _end_data;
// .bss (uninitialized data to set to 0);
extern unsigned long __bss_start__;
extern unsigned long __bss_end__;

// NVIC ISR table
__attribute__ ((section(".nvic_table")))
unsigned long myvectors[] =
{
    // This are the fixed priority interrupts and the stack pointer loaded at startup at R13 (SP).
    //                                              VECTOR N (Check Datasheet)
    // here the compiler it's boring.. have to figure that out
    (unsigned long) &_stack_top,            // stack pointer should be
                                            // placed here at startup.          0
    (unsigned long) rst_handler,            // code entry point                 1
    (unsigned long) nmi_handler,            // NMI handler.                     2
    (unsigned long) hardfault_handler,      // hard fault handler.              3
    // Configurable priority interruts handler start here.
    (unsigned long) empty_def_handler,      // Memory Management Fault          4
    (unsigned long) empty_def_handler,      // Bus Fault                        5
    (unsigned long) empty_def_handler,      // Usage Fault                      6
    0,                                      // Reserved                         7
    0,                                      // Reserved                         8
    0,                                      // Reserved                         9
    0,                                      // Reserved                         10
    (unsigned long) empty_def_handler,      // SV call                          11
    (unsigned long) empty_def_handler,      // Debug monitor                    12
    0,                                      // Reserved                         13
    (unsigned long) empty_def_handler,      // PendSV                           14
    (unsigned long) empty_def_handler,      // SysTick                          15
    // Peripherial interrupts start here.
    (unsigned long) empty_def_handler,      // GPIO Port A                      16
    (unsigned long) empty_def_handler,      // GPIO Port B                      17
    (unsigned long) empty_def_handler,      // GPIO Port C                      18
    (unsigned long) empty_def_handler,      // GPIO Port D                      19
    (unsigned long) empty_def_handler,      // GPIO Port E                      20
    (unsigned long) uart0_irq,              // UART 0                           21
    (unsigned long) uart1_irq,              // UART 1                           22
    (unsigned long) empty_def_handler,      // SSI 0                            23
    (unsigned long) empty_def_handler,      // I2C 0                            24
    0,                                      // Reserved                         25
    0,                                      // Reserved                         26
    0,                                      // Reserved                         27
    0,                                      // Reserved                         28
    0,                                      // Reserved                         29
    (unsigned long) empty_def_handler,      // ADC 0 Seq 0                      30
    (unsigned long) empty_def_handler,      // ADC 0 Seq 1                      31
    (unsigned long) empty_def_handler,      // ADC 0 Seq 2                      32
    (unsigned long) empty_def_handler,      // ADC 0 Seq 3                      33
    (unsigned long) empty_def_handler,      // WDT 0 and 1                      34
    (unsigned long) empty_def_handler,      // 16/32 bit timer 0 A              35
    (unsigned long) empty_def_handler,      // 16/32 bit timer 0 B              36
    (unsigned long) empty_def_handler,      // 16/32 bit timer 1 A              37
    (unsigned long) empty_def_handler,      // 16/32 bit timer 1 B              38
    (unsigned long) empty_def_handler,      // 16/32 bit timer 2 A              39
    (unsigned long) empty_def_handler,      // 16/32 bit timer 2 B              40
    (unsigned long) empty_def_handler,      // Analog comparator 0              41
    (unsigned long) empty_def_handler,      // Analog comparator 1              42
    0,                                      // Reserved                         43
    (unsigned long) empty_def_handler,      // System control                   44
    (unsigned long) empty_def_handler,      // Flash + EEPROM control           45
    (unsigned long) empty_def_handler,      // GPIO Port F                      46
    0,                                      // Reserved                         47
    0,                                      // Reserved                         48
    (unsigned long) uart2_irq,              // UART 2                           49
    (unsigned long) empty_def_handler,      // SSI 1                            50
    (unsigned long) empty_def_handler,      // 16/32 bit timer 3 A              51
    (unsigned long) empty_def_handler,      // 16/32 bit timer 3 B              52
    (unsigned long) empty_def_handler,      // I2C 1                            53
    0,                                      // Reserved                         54
    (unsigned long) empty_def_handler,      // CAN 0                            55
    0,                                      // Reserved                         56
    0,                                      // Reserved                         57
    0,                                      // Reserved                         58
    (unsigned long) empty_def_handler,      // Hibernation module               59
    (unsigned long) empty_def_handler,      // USB                              60
    0,                                      // Reserved                         61
    (unsigned long) empty_def_handler,      // UDMA SW                          62
    (unsigned long) empty_def_handler,      // UDMA Error                       63
    (unsigned long) empty_def_handler,      // ADC 1 Seq 0                      64
    (unsigned long) empty_def_handler,      // ADC 1 Seq 1                      65
    (unsigned long) empty_def_handler,      // ADC 1 Seq 2                      66
    (unsigned long) empty_def_handler,      // ADC 1 Seq 3                      67
    0,                                      // Reserved                         68
    0,                                      // Reserved                         69
    0,                                      // Reserved                         70
    0,                                      // Reserved                         71
    0,                                      // Reserved                         72
    (unsigned long) empty_def_handler,      // SSI 2                            73
    (unsigned long) empty_def_handler,      // SSI 2                            74
    (unsigned long) uart3_irq,              // UART 3                           75
    (unsigned long) uart4_irq,              // UART 4                           76
    (unsigned long) uart5_irq,              // UART 5                           77
    (unsigned long) uart6_irq,              // UART 6                           78
    (unsigned long) uart7_irq,              // UART 7                           79
    0,                                      // Reserved                         80
    0,                                      // Reserved                         81
    0,                                      // Reserved                         82
    0,                                      // Reserved                         83
    (unsigned long) empty_def_handler,      // I2C 2                            84
    (unsigned long) empty_def_handler,      // I2C 4                            85
    (unsigned long) empty_def_handler,      // 16/32 bit timer 4 A              86
    (unsigned long) empty_def_handler,      // 16/32 bit timer 4 B              87
    0,                                      // Reserved                         88
    0,                                      // Reserved                         89
    0,                                      // Reserved                         90
    0,                                      // Reserved                         91
    0,                                      // Reserved                         92
    0,                                      // Reserved                         93
    0,                                      // Reserved                         94
    0,                                      // Reserved                         95
    0,                                      // Reserved                         96
    0,                                      // Reserved                         97
    0,                                      // Reserved                         98
    0,                                      // Reserved                         99
    0,                                      // Reserved                         100
    0,                                      // Reserved                         101
    0,                                      // Reserved                         102
    0,                                      // Reserved                         103
    0,                                      // Reserved                         104
    0,                                      // Reserved                         105
    0,                                      // Reserved                         106
    0,                                      // Reserved                         107
    (unsigned long) empty_def_handler,      // 16/32 bit timer 5 A              108
    (unsigned long) empty_def_handler,      // 16/32 bit timer 5 B              109
    (unsigned long) empty_def_handler,      // 32/64 bit timer 0 A              110
    (unsigned long) empty_def_handler,      // 32/64 bit timer 0 B              111
    (unsigned long) empty_def_handler,      // 32/64 bit timer 1 A              112
    (unsigned long) empty_def_handler,      // 32/64 bit timer 1 B              113
    (unsigned long) empty_def_handler,      // 32/64 bit timer 2 A              114
    (unsigned long) empty_def_handler,      // 32/64 bit timer 2 B              115
    (unsigned long) empty_def_handler,      // 32/64 bit timer 3 A              116
    (unsigned long) empty_def_handler,      // 32/64 bit timer 3 B              117
    (unsigned long) empty_def_handler,      // 32/64 bit timer 4 A              118
    (unsigned long) empty_def_handler,      // 32/64 bit timer 4 B              119
    (unsigned long) empty_def_handler,      // 32/64 bit timer 5 A              120
    (unsigned long) empty_def_handler,      // 32/64 bit timer 5 B              121
    (unsigned long) empty_def_handler,      // System Exception                 122
    0,                                      // Reserved                         123
    0,                                      // Reserved                         124
    0,                                      // Reserved                         125
    0,                                      // Reserved                         126
    0,                                      // Reserved                         127
    0,                                      // Reserved                         128
    0,                                      // Reserved                         129
    0,                                      // Reserved                         130
    0,                                      // Reserved                         131
    0,                                      // Reserved                         132
    0,                                      // Reserved                         133
    0,                                      // Reserved                         134
    0,                                      // Reserved                         135
    0,                                      // Reserved                         136
    0,                                      // Reserved                         137
    0,                                      // Reserved                         138
    0,                                      // Reserved                         139
    0,                                      // Reserved                         140
    0,                                      // Reserved                         141
    0,                                      // Reserved                         142
    0,                                      // Reserved                         143
    0,                                      // Reserved                         144
    0,                                      // Reserved                         145
    0,                                      // Reserved                         146
    0,                                      // Reserved                         147
    0,                                      // Reserved                         148
    0,                                      // Reserved                         149
    0,                                      // Reserved                         150
    0,                                      // Reserved                         151
    0,                                      // Reserved                         152
    0,                                      // Reserved                         153
    0                                       // Reserved                         154
};

//-----------------------------------------------------------------------------
//                          Function implementations.
//-----------------------------------------------------------------------------

/*
* System on reset code. NVIC 1
* Here I prepare the memory for the c compiler.
* The stack pointer should be set at the beginning with the NVIC table already.
* Copy the .data segment from flash into ram.
* 0 to the .bss segment
*/

void rst_handler(void)
{
    // Copy the .data section pointers to ram from flash.
    // Look at LD manual (Optional Section Attributes).

    // source and destination pointers
    unsigned long *src;
    unsigned long *dest;

    //this should be good!
    src = &_start_data_flash;
    dest = &_start_data;

    //this too
    while (dest < &_end_data)
    {
        *dest++ = *src++;
    }

    // now set the .bss segment to 0!
    dest = &__bss_start__;
    while (dest < &__bss_end__)
    {
        *dest++ = 0;
    }

    // after setting copying .data to ram and "zero-ing" .bss we are good
    // to start the main() method!
    // There you go!
    main();
}

// NMI Exception handler code NVIC 2
void nmi_handler(void)
{
    // Just loop forever, so if you want to debug the processor it's running.
    while (1)
    {
        flash_error(LED_BLUE, LED_GREEN, CLOCK_RATE);
    }
}

// Hard fault handler code NVIC 3
void hardfault_handler(void)
{
    // Just loop forever, so if you want to debug the processor it's running.
    while (1)
    {
        flash_error(LED_BLUE, LED_RED, CLOCK_RATE);
    }
}

// Empty handler used as default.
void empty_def_handler(void)
{
    // Just loop forever, so if you want to debug the processor it's running.
    while (1)
    {
        flash_error(LED_GREEN, LED_RED, CLOCK_RATE);
    }
}
