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

/* The LEDs on the Launchpad board are pins GPIO_F1..3 */
#define LED_RED (1<<1)
#define LED_BLUE (1<<2)
#define LED_GREEN (1<<3)
/* The buttons are pins GPIO_F0 and GPIO_F4*/
#define BUTTON_ONE (1<<0)
#define BUTTON_TWO (1<<4)

/* So we can see the LEDs flashing */
#define DELAY 300000UL

/**************************************************
* Function Prototypes
**************************************************/

static void busy_sleep(unsigned long wait_for);

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

int main(void)
{
    // enable PORT F GPIO peripheral
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;
    // The datasheet says wait after enabling GPIO
    busy_sleep(10);

    /* The GPIO for button one is multiplexed with NMI so we
     * have to 'unlock' it before we can use it
     */
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; /* Unlock CR  */
    GPIO_PORTF_CR_R |= BUTTON_ONE; /* Allow F0 to be changed */
    GPIO_PORTF_LOCK_R = 0; /* Lock CR again */

    // enable digital for LED PORT F pins
    GPIO_PORTF_DEN_R = LED_RED | LED_BLUE | LED_GREEN | BUTTON_ONE | BUTTON_TWO;
    // set LED PORT F pins as outputs (rest are inputs)
    GPIO_PORTF_DIR_R = LED_RED | LED_BLUE | LED_GREEN;
    // Enable weak pullups
    GPIO_PORTF_DR2R_R = BUTTON_ONE | BUTTON_TWO;
    GPIO_PORTF_PUR_R = BUTTON_ONE | BUTTON_TWO;

    while (1)
    {
        GPIO_PORTF_DATA_R = 0;
        busy_sleep(DELAY);

        unsigned int buttons = GPIO_PORTF_DATA_R;
        if ((buttons & BUTTON_ONE) == 0)
        {
            /* Button one pressed as input is low */
            GPIO_PORTF_DATA_R = LED_BLUE;
        }
        else if ((buttons & BUTTON_TWO) == 0)
        {
            /* Button two pressed as input is low */
            GPIO_PORTF_DATA_R = LED_GREEN;
        }
        else
        {
            /* Neither button pressed */
            GPIO_PORTF_DATA_R = LED_RED;
        }
        busy_sleep(DELAY);
    }

    /* Shouldn't get here */
    return 0;
}

/*
 * To use the arm-none-eabi C library, we need to supply this
 */
void _exit(void)
{
    while(1)
    {
        /* Do nothing */
    }
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

