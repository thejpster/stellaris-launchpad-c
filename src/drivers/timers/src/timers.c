/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2013-2014 theJPster (www.thejpster.org.uk)
*
* This module handles the various counter/timer peripherals
* in the LM4F.
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
* References:
*
*     [1] - Stellaris® LM4F121H5QR Microcontroller
*           Data Sheet.
*           http://www.ti.com/lit/ds/symlink/lm4f120h5qr.pdf
*****************************************************/

/**************************************************
* Includes
***************************************************/

#include "util/util.h"

#include "drivers/misc/misc.h"
#include "drivers/timers/timers.h"

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

typedef struct timer_interrupt_list_t
{
    timer_interrupt_handler_t handler_fn;
    uint32_t n_context;
    void *p_context;
} timer_interrupt_list_t;

/**************************************************
* Function Prototypes
**************************************************/

static void handle_interrupt(timer_module_t timer, timer_ab_t ab);

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Private Data
**************************************************/

static timer_interrupt_list_t interrupt_handlers[TIMER_NUM_TIMERS][2];

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

/* See table 2-9 in [1] */
static const int timer_int_map[TIMER_NUM_TIMERS][2] =
{
    { 19, 20 },
    { 21, 22 },
    { 23, 24 },
    { 35, 36 },
    { 70, 71 },
    { 92, 93 },
    { 94, 95 },
    { 96, 97 },
    { 98, 99 },
    { 100, 101 },
    { 102, 103 },
    { 104, 105 }    
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

    switch (timer)
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
        SET_BITS(p_timer->CFG, 0x01);
        break;
    case TIMER_SPLIT:
        SET_BITS(p_timer->CFG, 0x04);
        break;
    }

    /* Process Timer A config */
    switch (p_config->timer_a.type)
    {
    case TIMER_SPLIT_ONE_SHOT:
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAMR_1_SHOT);
        break;
    case TIMER_SPLIT_PERIODIC:
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAMR_PERIOD);
        break;
    case TIMER_SPLIT_CAPTURE_COUNT:
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAMR_CAP);
        break;
    case TIMER_SPLIT_CAPTURE_TIME:
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAMR_CAP);
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TACMR);
        break;
    case TIMER_SPLIT_PWM:
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAMR_PERIOD);
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAAMS);
        break;
    }

    if (p_config->timer_a.count_up)
    {
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TACDIR);
    }
    if (p_config->timer_a.match_interrupt)
    {
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAMIE);
    }
    if (p_config->timer_a.wait_on_trigger)
    {
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAWOT);
    }
    if (p_config->timer_a.snap_shot_mode)
    {
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TASNAPS);
    }
    if (p_config->timer_a.interval_load_write)
    {
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAILD);
    }
    if (p_config->timer_a.pwm_interrupt)
    {
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAPWMIE);
    }
    if (p_config->timer_a.match_register_update)
    {
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAMRSU);
    }
    if (p_config->timer_a.legacy_operation)
    {
        SET_BITS(p_timer->TAMR, TIMER_TAMR_TAPLO);
    }
    switch (p_config->timer_a.event_mode)
    {
    case TIMER_EVENT_MODE_POSITIVE:
        SET_BITS(p_timer->CTRL, TIMER_CTL_TAEVENT_POS);
        break;
    case TIMER_EVENT_MODE_NEGATIVE:
        SET_BITS(p_timer->CTRL, TIMER_CTL_TAEVENT_NEG);
        break;
    case TIMER_EVENT_MODE_BOTH:
        SET_BITS(p_timer->CTRL, TIMER_CTL_TAEVENT_BOTH);
        break;
    }
    if (p_config->timer_a.stall_enable)
    {
        SET_BITS(p_timer->CTRL, TIMER_CTL_TASTALL);
    }
    if (p_config->timer_a.output_trigger_enable)
    {
        SET_BITS(p_timer->CTRL, TIMER_CTL_TAOTE);
    }
    if (p_config->timer_a.invert_pwm_output)
    {
        SET_BITS(p_timer->CTRL, TIMER_CTL_TAPWML);
    }

    if (p_config->type == TIMER_SPLIT)
    {
        /* Process timer B config */
        switch (p_config->timer_b.type)
        {
        case TIMER_SPLIT_ONE_SHOT:
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBMR_1_SHOT);
            break;
        case TIMER_SPLIT_PERIODIC:
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBMR_PERIOD);
            break;
        case TIMER_SPLIT_CAPTURE_COUNT:
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBMR_CAP);
            break;
        case TIMER_SPLIT_CAPTURE_TIME:
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBMR_CAP);
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBCMR);
            break;
        case TIMER_SPLIT_PWM:
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBMR_PERIOD);
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBAMS);
            break;
        }
        if (p_config->timer_b.count_up)
        {
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBCDIR);
        }
        if (p_config->timer_b.match_interrupt)
        {
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBMIE);
        }
        if (p_config->timer_b.wait_on_trigger)
        {
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBWOT);
        }
        if (p_config->timer_b.snap_shot_mode)
        {
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBSNAPS);
        }
        if (p_config->timer_b.interval_load_write)
        {
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBILD);
        }
        if (p_config->timer_b.pwm_interrupt)
        {
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBPWMIE);
        }
        if (p_config->timer_b.match_register_update)
        {
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBMRSU);
        }
        if (p_config->timer_b.legacy_operation)
        {
            SET_BITS(p_timer->TBMR, TIMER_TBMR_TBPLO);
        }

        switch (p_config->timer_b.event_mode)
        {
        case TIMER_EVENT_MODE_POSITIVE:
            SET_BITS(p_timer->CTRL, TIMER_CTL_TBEVENT_POS);
            break;
        case TIMER_EVENT_MODE_NEGATIVE:
            SET_BITS(p_timer->CTRL, TIMER_CTL_TBEVENT_NEG);
            break;
        case TIMER_EVENT_MODE_BOTH:
            SET_BITS(p_timer->CTRL, TIMER_CTL_TBEVENT_BOTH);
            break;
        }

        if (p_config->timer_b.stall_enable)
        {
            SET_BITS(p_timer->CTRL, TIMER_CTL_TBSTALL);
        }
        if (p_config->timer_b.output_trigger_enable)
        {
            SET_BITS(p_timer->CTRL, TIMER_CTL_TBOTE);
        }
        if (p_config->timer_b.invert_pwm_output)
        {
            SET_BITS(p_timer->CTRL, TIMER_CTL_TBPWML);
        }
    }

    if (p_config->rtc_stall_enable)
    {
        SET_BITS(p_timer->CTRL, TIMER_CTL_RTCEN);
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
        SET_BITS(p_timer->CTRL, (1 << 0));
    }
    else
    {
        SET_BITS(p_timer->CTRL, (1 << 8));
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
        CLEAR_BITS(p_timer->CTRL, (1 << 0));
    }
    else
    {
        CLEAR_BITS(p_timer->CTRL, (1 << 8));
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
    SET_BITS(p_timer->IMR, interrupt);
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
    CLEAR_BITS(p_timer->IMR, interrupt);
}

/*
 * Register a function to be called when an interrupt fires. There is one interrupt per timer,
 * so the handler will need to check what caused the interrupt if multiple interrupts have been enabled.
 *
 * @param timer - A timer module (e.g. TIMER_0)
 * @param ab - Select timer A or timer B.
 * @param handler - A function to call (in interrupt context)
 * @param p_context - A pointer argument supplied to the handler
 * @param n_context - An integer argument supplied to the handler
 */
void timer_register_handler(
    timer_module_t timer,
    timer_ab_t ab,
    timer_interrupt_handler_t handler,
    void *p_context, uint32_t n_context)
{
    timer_interrupt_list_t *p = &interrupt_handlers[timer][ab];
    p->handler_fn = handler;
    p->p_context = p_context;
    p->n_context = n_context;
    enable_interrupt(timer_int_map[timer][ab]);
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

void timer_0a_interrupt(void)
{
    handle_interrupt(TIMER_0, TIMER_A);
}

void timer_1a_interrupt(void)
{
    handle_interrupt(TIMER_1, TIMER_A);
}

void timer_2a_interrupt(void)
{
    handle_interrupt(TIMER_2, TIMER_A);
}

void timer_3a_interrupt(void)
{
    handle_interrupt(TIMER_3, TIMER_A);
}

void timer_4a_interrupt(void)
{
    handle_interrupt(TIMER_4, TIMER_A);
}

void timer_5a_interrupt(void)
{
    handle_interrupt(TIMER_5, TIMER_A);
}

void timer_0b_interrupt(void)
{
    handle_interrupt(TIMER_0, TIMER_B);
}

void timer_1b_interrupt(void)
{
    handle_interrupt(TIMER_1, TIMER_B);
}

void timer_2b_interrupt(void)
{
    handle_interrupt(TIMER_2, TIMER_B);
}

void timer_3b_interrupt(void)
{
    handle_interrupt(TIMER_3, TIMER_B);
}

void timer_4b_interrupt(void)
{
    handle_interrupt(TIMER_4, TIMER_B);
}

void timer_5b_interrupt(void)
{
    handle_interrupt(TIMER_5, TIMER_B);
}

void timer_w0a_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_0, TIMER_A);
}

void timer_w1a_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_1, TIMER_A);
}

void timer_w2a_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_2, TIMER_A);
}

void timer_w3a_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_3, TIMER_A);
}

void timer_w4a_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_4, TIMER_A);
}

void timer_w5a_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_5, TIMER_A);
}

void timer_w0b_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_0, TIMER_B);
}

void timer_w1b_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_1, TIMER_B);
}

void timer_w2b_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_2, TIMER_B);
}

void timer_w3b_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_3, TIMER_B);
}

void timer_w4b_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_4, TIMER_B);
}

void timer_w5b_interrupt(void)
{
    handle_interrupt(TIMER_WIDE_5, TIMER_B);
}

/**************************************************
* Private Functions
***************************************************/

static void handle_interrupt(timer_module_t timer, timer_ab_t ab)
{
    timer_interrupt_list_t *p = &interrupt_handlers[timer][ab];
    if (p->handler_fn)
    {
        p->handler_fn(timer, ab, p->p_context, p->n_context);
    }
}

/**************************************************
* End of file
***************************************************/

