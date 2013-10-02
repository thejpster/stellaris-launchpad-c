/*****************************************************
*
* Stellaris Launchpad Example Project
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
*
* This module handles the various counter/timer peripherals
* in the LM4F.
*
*****************************************************/

/**************************************************
* Includes
***************************************************/

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "misc/misc.h"
#include "timers/timers.h"

/**************************************************
* Defines
***************************************************/

/* None */

/**************************************************
* Data Types
**************************************************/

typedef struct timer_registers_t
{
    reg_t CFG;
    reg_t TAMR;
    reg_t TBMR;
    reg_t CTRL;
    reg_t SYNC;
    reg_t _PADDING;
    reg_t IMR;
    const reg_t RIS;
    const reg_t MIS;
    reg_t ICR;
    reg_t TAILR;
    reg_t TBILR;
    reg_t TAMATCHR;
    reg_t TBMATCHR;
    reg_t TAPR;
    reg_t TBPR;
    reg_t TAPMR;
    reg_t TBPMR;
    const reg_t TAR;
    const reg_t TBR;
    reg_t TAV;
    reg_t TBV;
    const reg_t RTCPD;
    const reg_t TAPS;
    const reg_t TBPS;
    const reg_t TAPV;
    const reg_t TBPV;
} timer_registers_t;

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

static timer_registers_t *const timers[TIMER_NUM_TIMERS] =
{
    (timer_registers_t *) &TIMER0_CFG_R,
    (timer_registers_t *) &TIMER1_CFG_R,
    (timer_registers_t *) &TIMER2_CFG_R,
    (timer_registers_t *) &TIMER3_CFG_R,
    (timer_registers_t *) &TIMER4_CFG_R,
    (timer_registers_t *) &TIMER5_CFG_R,
    (timer_registers_t *) &WTIMER0_CFG_R,
    (timer_registers_t *) &WTIMER1_CFG_R,
    (timer_registers_t *) &WTIMER2_CFG_R,
    (timer_registers_t *) &WTIMER3_CFG_R,
    (timer_registers_t *) &WTIMER4_CFG_R,
    (timer_registers_t *) &WTIMER5_CFG_R
};

/**************************************************
* Public Functions
***************************************************/

/*
 * Configure a General Purpose Timer Module.
 *
 * @param timer The timer to configure.
 * @param p_config Pointer to a timer configuration structure.
 */
void timer_configure(timer_module_t timer, const timer_config_t *p_config)
{
    timer_registers_t *p_timer = timers[timer];

    switch(timer)
    {
    case TIMER_0:
        SYSCTL_RCGCTIMER_R = SYSCTL_RCGCTIMER_R0;
        break;
    case TIMER_1:
        SYSCTL_RCGCTIMER_R = SYSCTL_RCGCTIMER_R1;
        break;
    case TIMER_2:
        SYSCTL_RCGCTIMER_R = SYSCTL_RCGCTIMER_R2;
        break;
    case TIMER_3:
        SYSCTL_RCGCTIMER_R = SYSCTL_RCGCTIMER_R3;
        break;
    case TIMER_4:
        SYSCTL_RCGCTIMER_R = SYSCTL_RCGCTIMER_R4;
        break;
    case TIMER_5:
        SYSCTL_RCGCTIMER_R = SYSCTL_RCGCTIMER_R5;
        break;
    case TIMER_WIDE_0:
        SYSCTL_RCGCWTIMER_R = SYSCTL_RCGCWTIMER_R0;
        break;
    case TIMER_WIDE_1:
        SYSCTL_RCGCWTIMER_R = SYSCTL_RCGCWTIMER_R1;
        break;
    case TIMER_WIDE_2:
        SYSCTL_RCGCWTIMER_R = SYSCTL_RCGCWTIMER_R2;
        break;
    case TIMER_WIDE_3:
        SYSCTL_RCGCWTIMER_R = SYSCTL_RCGCWTIMER_R3;
        break;
    case TIMER_WIDE_4:
        SYSCTL_RCGCWTIMER_R = SYSCTL_RCGCWTIMER_R4;
        break;
    case TIMER_WIDE_5:
        SYSCTL_RCGCWTIMER_R = SYSCTL_RCGCWTIMER_R5;
        break;
    }

    /* Can't configure timers while they are running */
    timer_disable(timer, TIMER_A);
    timer_disable(timer, TIMER_B);

    /* Set the defaults */
    p_timer->TAMR = 0;
    p_timer->TBMR = 0;
    p_timer->CFG = 0;
    p_timer->CTRL = 0;
    switch (p_config->type)
    {
    case TIMER_JOINED:
        /* The default */
        break;
    case TIMER_RTC:
        p_timer->CFG |= 0x01;
        break;
    case TIMER_SPLIT:
        p_timer->CFG |= 0x04;
        break;
    }

    /* Process Timer A config */
    switch (p_config->timer_a.type)
    {
    case TIMER_SPLIT_ONE_SHOT:
        p_timer->TAMR |= TIMER_TAMR_TAMR_1_SHOT;
        break;
    case TIMER_SPLIT_PERIODIC:
        p_timer->TAMR |= TIMER_TAMR_TAMR_PERIOD;
        break;
    case TIMER_SPLIT_CAPTURE_COUNT:
        p_timer->TAMR |= TIMER_TAMR_TAMR_CAP;
        break;
    case TIMER_SPLIT_CAPTURE_TIME:
        p_timer->TAMR |= TIMER_TAMR_TAMR_CAP;
        p_timer->TAMR |= TIMER_TAMR_TACMR;
        break;
    case TIMER_SPLIT_PWM:
        p_timer->TAMR |= TIMER_TAMR_TAMR_PERIOD;
        p_timer->TAMR |= TIMER_TAMR_TAAMS;
        break;
    }

    if (p_config->timer_a.count_up)
    {
        p_timer->TAMR |= TIMER_TAMR_TACDIR;
    }
    if (p_config->timer_a.match_interrupt)
    {
        p_timer->TAMR |= TIMER_TAMR_TAMIE;
    }
    if (p_config->timer_a.wait_on_trigger)
    {
        p_timer->TAMR |= TIMER_TAMR_TAWOT;
    }
    if (p_config->timer_a.snap_shot_mode)
    {
        p_timer->TAMR |= TIMER_TAMR_TASNAPS;
    }
    if (p_config->timer_a.interval_load_write)
    {
        p_timer->TAMR |= TIMER_TAMR_TAILD;
    }
    if (p_config->timer_a.pwm_interrupt)
    {
        p_timer->TAMR |= TIMER_TAMR_TAPWMIE;
    }
    if (p_config->timer_a.match_register_update)
    {
        p_timer->TAMR |= TIMER_TAMR_TAMRSU;
    }
    if (p_config->timer_a.legacy_operation)
    {
        p_timer->TAMR |= TIMER_TAMR_TAPLO;
    }
    switch (p_config->timer_a.event_mode)
    {
    case TIMER_EVENT_MODE_POSITIVE:
        p_timer->CTRL |= TIMER_CTL_TAEVENT_POS;
        break;
    case TIMER_EVENT_MODE_NEGATIVE:
        p_timer->CTRL |= TIMER_CTL_TAEVENT_NEG;
        break;
    case TIMER_EVENT_MODE_BOTH:
        p_timer->CTRL |= TIMER_CTL_TAEVENT_BOTH;
        break;
    }
    if (p_config->timer_a.stall_enable)
    {
        p_timer->CTRL |= TIMER_CTL_TASTALL;
    }
    if (p_config->timer_a.output_trigger_enable)
    {
        p_timer->CTRL |= TIMER_CTL_TAOTE;
    }
    if (p_config->timer_a.invert_pwm_output)
    {
        p_timer->CTRL |= TIMER_CTL_TAPWML;
    }

    if (p_config->type == TIMER_SPLIT)
    {
        /* Process timer B config */
        switch (p_config->timer_b.type)
        {
        case TIMER_SPLIT_ONE_SHOT:
            p_timer->TBMR |= TIMER_TBMR_TBMR_1_SHOT;
            break;
        case TIMER_SPLIT_PERIODIC:
            p_timer->TBMR |= TIMER_TBMR_TBMR_PERIOD;
            break;
        case TIMER_SPLIT_CAPTURE_COUNT:
            p_timer->TBMR |= TIMER_TBMR_TBMR_CAP;
            break;
        case TIMER_SPLIT_CAPTURE_TIME:
            p_timer->TBMR |= TIMER_TBMR_TBMR_CAP;
            p_timer->TBMR |= TIMER_TBMR_TBCMR;
            break;
        case TIMER_SPLIT_PWM:
            p_timer->TBMR |= TIMER_TBMR_TBMR_PERIOD;
            p_timer->TBMR |= TIMER_TBMR_TBAMS;
            break;
        }
        if (p_config->timer_b.count_up)
        {
            p_timer->TBMR |= TIMER_TBMR_TBCDIR;
        }
        if (p_config->timer_b.match_interrupt)
        {
            p_timer->TBMR |= TIMER_TBMR_TBMIE;
        }
        if (p_config->timer_b.wait_on_trigger)
        {
            p_timer->TBMR |= TIMER_TBMR_TBWOT;
        }
        if (p_config->timer_b.snap_shot_mode)
        {
            p_timer->TBMR |= TIMER_TBMR_TBSNAPS;
        }
        if (p_config->timer_b.interval_load_write)
        {
            p_timer->TBMR |= TIMER_TBMR_TBILD;
        }
        if (p_config->timer_b.pwm_interrupt)
        {
            p_timer->TBMR |= TIMER_TBMR_TBPWMIE;
        }
        if (p_config->timer_b.match_register_update)
        {
            p_timer->TBMR |= TIMER_TBMR_TBMRSU;
        }
        if (p_config->timer_b.legacy_operation)
        {
            p_timer->TBMR |= TIMER_TBMR_TBPLO;
        }

        switch (p_config->timer_b.event_mode)
        {
        case TIMER_EVENT_MODE_POSITIVE:
            p_timer->CTRL |= TIMER_CTL_TBEVENT_POS;
            break;
        case TIMER_EVENT_MODE_NEGATIVE:
            p_timer->CTRL |= TIMER_CTL_TBEVENT_NEG;
            break;
        case TIMER_EVENT_MODE_BOTH:
            p_timer->CTRL |= TIMER_CTL_TBEVENT_BOTH;
            break;
        }

        if (p_config->timer_b.stall_enable)
        {
            p_timer->CTRL |= TIMER_CTL_TBSTALL;
        }
        if (p_config->timer_b.output_trigger_enable)
        {
            p_timer->CTRL |= TIMER_CTL_TBOTE;
        }
        if (p_config->timer_b.invert_pwm_output)
        {
            p_timer->CTRL |= TIMER_CTL_TBPWML;
        }
    }

    if (p_config->rtc_stall_enable)
    {
        p_timer->CTRL |= TIMER_CTL_RTCEN;
    }
}

/*
 * Enable a timer.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
void timer_enable(timer_module_t timer, timer_ab_t ab)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        p_timer->CTRL |= (1 << 0);
    }
    else
    {
        p_timer->CTRL |= (1 << 8);
    }
}

/*
 * Disable a timer.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
void timer_disable(timer_module_t timer, timer_ab_t ab)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        p_timer->CTRL &= ~(1 << 0);
    }
    else
    {
        p_timer->CTRL &= ~(1 << 8);
    }
}

/*
 * Disable an interrupt.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
void timer_interrupt_enable(timer_module_t timer, timer_interrupt_t interrupt)
{
    timer_registers_t *p_timer = timers[timer];
    p_timer->IMR |= interrupt;
}

/*
 * Enable an interrupt.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
void timer_interrupt_disable(timer_module_t timer, timer_interrupt_t interrupt)
{
    timer_registers_t *p_timer = timers[timer];
    p_timer->IMR &= ~interrupt;
}

/*
 * Check if an interrupt event has actually occured.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
bool timer_interrupt_raw_status(timer_module_t timer, timer_interrupt_t interrupt)
{
    timer_registers_t *p_timer = timers[timer];
    return (p_timer->RIS & interrupt);
}

/*
 * Check if an activated interrupt event has occured.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
bool timer_interrupt_masked_status(timer_module_t timer, timer_interrupt_t interrupt)
{
    timer_registers_t *p_timer = timers[timer];
    return (p_timer->MIS & interrupt);
}

/*
 * Clear an interrupt event flag.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
void timer_interrupt_clear(timer_module_t timer, timer_interrupt_t interrupt)
{
    timer_registers_t *p_timer = timers[timer];
    p_timer->ICR = interrupt;
}

/*
 * When counting down, this is the start value. When counting up, this is the upper bound.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
uint32_t timer_get_interval_load(timer_module_t timer, timer_ab_t ab)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        return p_timer->TAILR;
    }
    else
    {
        return p_timer->TBILR;
    }
}

/*
 * When counting down, this is the start value. When counting up, this is the upper bound.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
void timer_set_interval_load(timer_module_t timer, timer_ab_t ab, uint32_t value)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        p_timer->TAILR = value;
    }
    else
    {
        p_timer->TBILR = value;
    }
}

/*
 * An interrupt can be generated on match when config value match_interrupt is
 * true. This gets the match value.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
uint32_t timer_get_match(timer_module_t timer, timer_ab_t ab)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        return p_timer->TAMATCHR;
    }
    else
    {
        return p_timer->TBMATCHR;
    }
}

/*
 * An interrupt can be generated on match when config value match_interrupt is
 * true. This sets the match value.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
void timer_set_match(timer_module_t timer, timer_ab_t ab, uint32_t value)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        p_timer->TAMATCHR = value;
    }
    else
    {
        p_timer->TBMATCHR = value;
    }
}

/*
 * Prescale allows you to extend a split timer by 16 bits. In one-shot or
 * periodic down modes, they form the bottom 16 bits. In all other modes they
 * form the top 16 bits.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
uint16_t timer_get_prescale(timer_module_t timer, timer_ab_t ab)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        return p_timer->TAPR;
    }
    else
    {
        return p_timer->TBPR;
    }
}

/*
 * Prescale allows you to extend a split timer by 16 bits. In one-shot or
 * periodic down modes, they form the bottom 16 bits. In all other modes they
 * form the top 16 bits.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
void timer_set_prescale(timer_module_t timer, timer_ab_t ab, uint16_t value)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        p_timer->TAPR = value;
    }
    else
    {
        p_timer->TBPR = value;
    }
}

/*
 * Prescale allows you to extend a split timer by 16 bits. This gets the 16
 * bits that go with timer_get_match().
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
uint16_t timer_get_prescale_match(timer_module_t timer, timer_ab_t ab)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        return p_timer->TAPMR;
    }
    else
    {
        return p_timer->TBPMR;
    }
}

/* 
 * Prescale allows you to extend a split timer by 16 bits. This sets the 16
 * bits that go with timer_set_match().
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
void timer_set_prescale_match(timer_module_t timer, timer_ab_t ab, uint16_t value)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        p_timer->TAPMR = value;
    }
    else
    {
        p_timer->TBPMR = value;
    }
}

/*
 * Get the current value of timer A or B.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
uint32_t timer_get_value(timer_module_t timer, timer_ab_t ab)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        return p_timer->TAR;
    }
    else
    {
        return p_timer->TBR;
    }
}

/*
 * Get the free-running value of timer A or B.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
uint32_t timer_get_free_run_value(timer_module_t timer, timer_ab_t ab)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        return p_timer->TAV;
    }
    else
    {
        return p_timer->TBV;
    }
}

/*
 * Set the value of the free running timer. This is loaded into the timer at the next tick.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
void timer_set_free_run_value(timer_module_t timer, timer_ab_t ab, uint32_t value)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        p_timer->TAV = value;
    }
    else
    {
        p_timer->TBV = value;
    }
}

/*
 * Get the pre-divide used in RTC mode.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
uint16_t timer_get_rtc_predivide(timer_module_t timer)
{
    timer_registers_t *p_timer = timers[timer];
    return p_timer->RTCPD;
}

/*
 * Get the current value of the timer prescaler. Used with timer_get_value().
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
uint16_t timer_get_prescale_snapshot(timer_module_t timer, timer_ab_t ab)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        return p_timer->TAPS;
    }
    else
    {
        return p_timer->TBPS;
    }
}

/*
 * Get the free-running current value of the timer prescaler. Use with
 * timer_get_free_running_value.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 */
uint16_t timer_get_prescale_value(timer_module_t timer, timer_ab_t ab)
{
    timer_registers_t *p_timer = timers[timer];
    if (ab == TIMER_A)
    {
        return p_timer->TAPV;
    }
    else
    {
        return p_timer->TBPV;
    }
}

/**************************************************
* Private Functions
***************************************************/

/* None */

/**************************************************
* End of file
***************************************************/

