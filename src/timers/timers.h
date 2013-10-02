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
* This module handles the various counter/timer peripherals
* in the LM4F.
*
*****************************************************/

#ifndef TIMERS_H_
#define TIMERS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************
* Includes
***************************************************/

#include <stdbool.h>

/**************************************************
* Public Defines
***************************************************/

/* None */

/**************************************************
* Public Data Types
**************************************************/

typedef enum timer_module_t
{
    TIMER_0,
    TIMER_1,
    TIMER_2,
    TIMER_3,
    TIMER_4,
    TIMER_5,
    TIMER_WIDE_0,
    TIMER_WIDE_1,
    TIMER_WIDE_2,
    TIMER_WIDE_3,
    TIMER_WIDE_4,
    TIMER_WIDE_5,
    
} timer_module_t;

#define TIMER_NUM_TIMERS (TIMER_WIDE_5 + 1)

typedef enum timer_ab_t
{
    TIMER_A,
    TIMER_B
} timer_ab_t;

typedef enum timer_type_t
{
    TIMER_JOINED,
    TIMER_RTC,
    TIMER_SPLIT
} timer_type_t;

typedef enum timer_split_type_t
{
    TIMER_SPLIT_ONE_SHOT,
    TIMER_SPLIT_PERIODIC,
    TIMER_SPLIT_CAPTURE_COUNT,
    TIMER_SPLIT_CAPTURE_TIME,
    TIMER_SPLIT_PWM
} timer_split_type_t;

typedef enum timer_event_mode_t
{
    TIMER_EVENT_MODE_POSITIVE = 0x0,
    TIMER_EVENT_MODE_NEGATIVE = 0x1,
    TIMER_EVENT_MODE_BOTH = 0x03
} timer_event_mode_t;

typedef enum timer_interrupt_t
{
    TIMER_A_INTERRUPT_TIMEOUT = (1<<0),
    TIMER_A_INTERRUPT_CAPTURE_MODE_MATCH = (1<<1),
    TIMER_A_INTERRUPT_CAPTURE_MODE_EVENT = (1<<2),
    TIMER_RTC_INTERRUPT_MATCH = (1<<3),
    TIMER_A_INTERRUPT_MATCH = (1<<4),
    TIMER_B_INTERRUPT_TIMEOUT = (1<<8),
    TIMER_B_INTERRUPT_CAPTURE_MODE_MATCH = (1<<9),
    TIMER_B_INTERRUPT_CAPTURE_MODE_EVENT = (1<<10),
    TIMER_B_INTERRUPT_MATCH =  (1<<11)
} timer_interrupt_t;

typedef struct timer_split_config_t
{
    /* These values go into GPTMTxMR */
    timer_split_type_t type;
    bool count_up; /* Ignored in PWM and RTC */
    bool match_interrupt; /* Valid in ONE_SHOT and PERIODIC */
    bool wait_on_trigger; /* For cascading timers. Invalid for TIMER_0_A. */
    bool snap_shot_mode;
    bool interval_load_write;
    bool pwm_interrupt;
    bool match_register_update;
    bool legacy_operation;

    /* These values go into CPTMTCL */
    bool stall_enable;
    timer_event_mode_t event_mode;
    bool output_trigger_enable;
    bool invert_pwm_output;
} timer_split_config_t;

typedef struct timer_config_t
{
    timer_type_t type;
    timer_split_config_t timer_a;
    /* Only valid when type == TIMER_SPLIT */
    timer_split_config_t timer_b;

    bool rtc_stall_enable;
} timer_config_t;

/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Public Function Prototypes
***************************************************/

void timer_configure(timer_module_t timer, const timer_config_t* p_config);

void timer_enable(timer_module_t timer, timer_ab_t ab);

void timer_disable(timer_module_t timer, timer_ab_t ab);

void timer_interrupt_enable(timer_module_t timer, timer_interrupt_t interrupt);

void timer_interrupt_disable(timer_module_t timer, timer_interrupt_t interrupt);

bool timer_interrupt_raw_status(timer_module_t timer, timer_interrupt_t interrupt);

bool timer_interrupt_masked_status(timer_module_t timer, timer_interrupt_t interrupt);

void timer_interrupt_clear(timer_module_t timer, timer_interrupt_t interrupt);

uint32_t timer_get_interval_load(timer_module_t timer, timer_ab_t ab);

void timer_set_interval_load(timer_module_t timer, timer_ab_t ab, uint32_t value);

uint32_t timer_get_match(timer_module_t timer, timer_ab_t ab);

void timer_set_match(timer_module_t timer, timer_ab_t ab, uint32_t value);

uint16_t timer_get_prescale(timer_module_t timer, timer_ab_t ab);

void timer_set_prescale(timer_module_t timer, timer_ab_t ab, uint16_t value);

uint16_t timer_get_prescale_match(timer_module_t timer, timer_ab_t ab);

void timer_set_prescale_match(timer_module_t timer, timer_ab_t ab, uint16_t value);

uint32_t timer_get_value(timer_module_t timer, timer_ab_t ab);

uint32_t timer_get_free_run_value(timer_module_t timer, timer_ab_t ab);

void timer_set_free_run_value(timer_module_t timer, timer_ab_t ab, uint32_t value);

uint16_t timer_get_rtc_predivide(timer_module_t timer);

uint16_t timer_get_prescale_snapshot(timer_module_t timer, timer_ab_t ab);

uint16_t timer_get_prescale_value(timer_module_t timer, timer_ab_t ab);

#ifdef __cplusplus
}
#endif

#endif /* ndef TIMERS_H_ */

/**************************************************
* End of file
***************************************************/

