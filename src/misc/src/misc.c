/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Setup clocks for the Launchpad board
*
* Copyright (c) 2012 theJPster (www.thejpster.org.uk)
*
*****************************************************/

/**************************************************
* Includes
***************************************************/

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
     *
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
}

/**
 * Rough and ready sleep function. Approx 8 clock ticks elapse
 * per 'delay', although compiler optimisations might 
 * massively reduce this.
 */
void busy_sleep(unsigned long delay)
{
    while(delay--)
    {
        /* Spin */
    }
}

/**************************************************
* Private Functions
***************************************************/

/**************************************************
* End of file
***************************************************/

