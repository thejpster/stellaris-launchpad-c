/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Setup clocks for the Launchpad board
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

/**************************************************
* Includes
***************************************************/

#include "gpio/gpio.h"
#include "misc/misc.h"

/**************************************************
* Defines
***************************************************/

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

/**
 * Set system clock to a hard-coded value of 16 MHz.
 */
void set_clock(void)
{
    /*
     * There are four clock sources. The Precsision Internal Oscillator (PIOSC),
     * the Main Oscillator (MOSC), the internal 30kHz clock and the hibernation clock.
     *
     * The MOSC is connected to a 16MHz crystal and the Hibernation Clock is connected
     * to a 32.768 kHz crystal.
     *
     * You can run off any of these clock sources directly, or use them to drive the 400MHz
     * Phase-locked-loop (PLL) and run off a division of that.
     */

#if (CLOCK_RATE == 16000000) || (CLOCK_RATE == 66666666)
    /*
     * We're going to run at 16 MHz, which is a ratio of 1:1 with the MOSC.
     * This is the fastest we can run with the 16MHz crystal without using the PLL.
     *
     * If you change this, change CLOCK_RATE in misc.h
     *
     * The register we need to set is the Run-time Clock Config register (RCC).
     */
    unsigned long rcc = SYSCTL_RCC_R;
    // RCC SYSDIV field = 0x0
    CLEAR_BITS(rcc, SYSCTL_RCC_SYSDIV_M);
    // XTAL field = 0x15 (=> 16MHz)
    CLEAR_BITS(rcc, SYSCTL_RCC_XTAL_M);
    SET_BITS(rcc, SYSCTL_RCC_XTAL_16MHZ);
    // Set BYPASS bit
    SET_BITS(rcc, SYSCTL_RCC_BYPASS);
    // Set to MOSC
    CLEAR_BITS(rcc, SYSCTL_RCC_OSCSRC_M);
    SET_BITS(rcc, SYSCTL_RCC_OSCSRC_MAIN);
    // Disable PIOSC
    SET_BITS(rcc, SYSCTL_RCC_IOSCDIS);
    // Enable MOSC (i.e. don't disable)
    CLEAR_BITS(rcc, SYSCTL_RCC_MOSCDIS);
    // Write to register
    SYSCTL_RCC_R = rcc;

    /* Let it stabilise */
    busy_sleep(600000);
#else
#error CLOCK_RATE must be 16000000 or 66666666
#endif

#if CLOCK_RATE == 66666666
    /*
     * Now we're going to run at 66.67 MHz which is a ratio of 1:6 with the 400MHz PLL.
     * As the PLL is divided down by two, we need a divisor of 3.
     *
     * We could get 80MHz if we danced with RCC2 instead and got 400MHz / 5.
     */
    /* Clear PLL lock status */
    /* NB: MISC = Masked Interrupt State, not miscellaneous */
    SYSCTL_MISC_R = SYSCTL_MISC_PLLLMIS;

    /* Enable the PLL. We're OK, BYPASS is still set */
    CLEAR_BITS(rcc, SYSCTL_RCC_PWRDN);
    SYSCTL_RCC_R = rcc;

    while((SYSCTL_RIS_R & SYSCTL_MISC_PLLLMIS) == 0)
    {
        /* Wait for PLL to lock */
    }

    /* Set up a /3 divider (by putting 0x02 in the bitfield) */
    CLEAR_BITS(rcc, SYSCTL_RCC_SYSDIV_M);
    SET_BITS(rcc, 2 << SYSCTL_RCC_SYSDIV_S);
    /* Enable use of divider */
    SET_BITS(rcc, SYSCTL_RCC_USESYSDIV);
    /* Switch to PLL */
    CLEAR_BITS(rcc, SYSCTL_RCC_BYPASS);
    SYSCTL_RCC_R = rcc;

#endif
}

/**
 * Rough and ready sleep function.
 */
void busy_sleep(unsigned long delay)
{
    while(delay--)
    {
        __asm("");
    }
}

/*
 * Enable an interrupt in the NIC.
 * Writing a 0 to the enable register has no
 * effect so we don't need to do read-modify-write.
 */
void enable_interrupt(int interrupt_id)
{
    int register_bank = interrupt_id / 32;
    int bit_field = 1 << (interrupt_id % 32);
    switch(register_bank)
    {
    case 0:
        NVIC_EN0_R = bit_field; 
        break;
    case 1:
        NVIC_EN1_R = bit_field; 
        break;
    case 2:
        NVIC_EN2_R = bit_field; 
        break;
    case 3:
        NVIC_EN3_R = bit_field; 
        break;
    case 4:
        NVIC_EN4_R = bit_field; 
        break;
    default:
        flash_error(LED_RED, LED_BLUE, CLOCK_RATE / 32);
        break;
    }
}

/*
 * Disable an interrupt in the NIC.
 * Writing a 0 to the disable register has no
 * effect so we don't need to do read-modify-write.
 */
void disable_interrupt(int interrupt_id)
{
    int register_bank = interrupt_id / 32;
    int bit_field = 1 << (interrupt_id % 32);
    switch(register_bank)
    {
    case 0:
        NVIC_DIS0_R = bit_field; 
        break;
    case 1:
        NVIC_DIS1_R = bit_field; 
        break;
    case 2:
        NVIC_DIS2_R = bit_field; 
        break;
    case 3:
        NVIC_DIS3_R = bit_field; 
        break;
    case 4:
        NVIC_DIS4_R = bit_field; 
        break;
    default:
        flash_error(LED_RED, LED_BLUE, CLOCK_RATE / 32);
        break;
    }
}

/**************************************************
* Private Functions
***************************************************/

/**************************************************
* End of file
***************************************************/

