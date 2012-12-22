/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2012 theJPster (www.thejpster.org.uk)
*
*****************************************************/

/**************************************************
* Includes
***************************************************/

#include "inc/lm4f120h5qr.h"

/**************************************************
* Defines
***************************************************/

/* These are the LEDs on the Launchpad board (on GPIO_F) */
#define LED_RED 0x2
#define LED_BLUE 0x4
#define LED_GREEN 0x8

/* So we can see the LEDs flashing */
#define DELAY 1000000UL

/**************************************************
* Function Prototypes
**************************************************/

static void busy_sleep(unsigned long wait_for);

/**************************************************
* Public Functions
***************************************************/

int main(void)
{
    // enable PORT F GPIO peripheral
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;
    // set LED PORT F pins as outputs
    GPIO_PORTF_DIR_R = LED_RED | LED_BLUE | LED_GREEN;
    // enable digital for LED PORT F pins
    GPIO_PORTF_DEN_R = LED_RED | LED_BLUE | LED_GREEN;

    while (1)
    {
        GPIO_PORTF_DATA_R = LED_RED;
        busy_sleep(DELAY);
        GPIO_PORTF_DATA_R = LED_BLUE;
        busy_sleep(DELAY);
        GPIO_PORTF_DATA_R = LED_GREEN;
        busy_sleep(DELAY);
    }

    /* Shouldn't get here */
    return 0;
}

/**************************************************
* Private Functions
***************************************************/

void busy_sleep(unsigned long wait_for)
{
    while (wait_for--)
    {
        /* Spin */
    }
}

/**************************************************
* End of file
***************************************************/
