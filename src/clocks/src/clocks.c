/*****************************************************
*
* Stellaris Launchpad Example Project
*
* Copyright (c) 2014 theJPster (www.thejpster.org.uk)
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
* By 'clocks', I mean Rev Counters (showing revs/minute)
* and Speedometers (showing miles/hour) - typically represented
* by needles that rotate on a shaft through about 270 degrees.
*
* In our case, there is one 'clock' showing revs/minute
* and a TFT showing speed.
*
* This code also handles the trip computer, retaining the number
* of miles travelled
*
* a) ever
* b) since the last trip reset
* c) since the car started
*
*****************************************************/

/**************************************************
* Includes
***************************************************/

#include "util/util.h"
#include "drivers/misc/misc.h"
#include "main.h"

#include "../clocks.h"

/**************************************************
* Defines
***************************************************/

/* Magic value which indicates speed is approximately zero */
#define STALLED 0xFFFFFFFF

/* Number of counter ticks elapsed before we declare speed to be zero. */
#define MAX_PERIOD (TICK_RATE/2) /* half a second */

/* 2000 edges per second, assuming 8 edges per rotation, is about 10,000 rpm */
#define MIN_PERIOD (TICK_RATE / 2000)

/* We add to the trip in units of 0.1 miles */
#define MM_PER_REVOLUTION          1985
#define EDGES_PER_REVOLUTION       8
#define MM_PER_MILE                1609344
#define MM_PER_TENTH_MILE          (MM_PER_MILE/10)
#define REVOLUTIONS_PER_TENTH_MILE (MM_PER_TENTH_MILE / MM_PER_REVOLUTION)
#define EDGES_PER_TENTH_MILE       (int) ((REVOLUTIONS_PER_TENTH_MILE * EDGES_PER_REVOLUTION))

/**************************************************
* Data Types
**************************************************/

typedef struct waveform_t
{
    uint32_t period;
    uint32_t last_seen;
} waveform_t;

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

static clocks_distance_t trip[CLOCKS_NUM_TRIPS];
static uint16_t trip_edges[CLOCKS_NUM_TRIPS];

static volatile waveform_t speedo = { .last_seen = 0, .period = STALLED };

static volatile waveform_t tacho  = { .last_seen = 0, .period = STALLED };

static const double speed_factor = ((double) TICK_RATE * 3600 * MM_PER_REVOLUTION) / (EDGES_PER_REVOLUTION *MM_PER_MILE);
static const double tacho_factor = ((double) TICK_RATE * 60) / (EDGES_PER_REVOLUTION);

/**************************************************
* Public Functions
***************************************************/

/*
 * Load trips from non-volatile memory.
 */
void clocks_init(void)
{
    PRINTF("Edges per 0.1 mi = %u\n", EDGES_PER_TENTH_MILE);
    /* When these hit zero, clock up another 0.1 miles */
    for (int i = 0; i < CLOCKS_NUM_TRIPS; i++)
    {
        trip_edges[i] = EDGES_PER_TENTH_MILE;
        /* @todo these are placeholders. Write this. */
        trip[i] = (i * 1000) * CLOCKS_DISTANCE_SCALE;
    }
}

/*
 * Store trips in non-volatile memory.
 */
void clocks_save(void)
{
    /* @todo write this */
}

/*
 * Fetch current status.
 */
void clocks_get(struct clocks_state_t *p_state)
{
    /* Need to stop interrupts to get consistent state */
    uint32_t cache_speedo;
    uint32_t cache_tacho;

    disable_interrupts();
    for (int i = 0; i < CLOCKS_NUM_TRIPS; i++)
    {
        p_state->trip[i] = trip[i];
    }
    cache_speedo = speedo.period;
    cache_tacho = tacho.period;
    enable_interrupts();

    /* convert speedo.period to mph */
    if (cache_speedo == STALLED)
    {
        p_state->current_speed = 0;
    }
    else
    {
        p_state->current_speed = (clocks_speed_t) (speed_factor / cache_speedo);
    }
    /* convert tacho.period to rpm */
    if (cache_tacho == STALLED)
    {
        p_state->current_revs = 0;
    }
    else
    {
        p_state->current_revs = (clocks_revs_t) (tacho_factor / cache_tacho);
    }
}

/*
 * This should be called four times per second.
 * It's used to detect stalled inputs (which are normally edge triggered).
 * We don't knobble the edge counter, so we're ready to restart.
 */
void clocks_timer_tick(void)
{
    /* Zero stalled input readings */
    uint32_t now = get_counter();
    if ((now - speedo.last_seen) > MAX_PERIOD)
    {
        speedo.period = STALLED;
        speedo.last_seen = now;
    }
    if ((now - tacho.last_seen) > MAX_PERIOD)
    {
        tacho.period = STALLED;
        tacho.last_seen = now;
    }
}

/*
 * Call this when the speedo input has an edge. It's done under
 * interrupt so be quick about it.
 */
void clocks_speedo_edge(void)
{
    uint32_t now = get_counter();
    speedo.period = now - speedo.last_seen;
    if (speedo.period < MIN_PERIOD)
    {
        /* Set upper limit on speed for sanity */
        speedo.period = MIN_PERIOD;
    }
    speedo.last_seen = now;
    /* Add a fixed amout to all trips */
    for (int i = 0; i < CLOCKS_NUM_TRIPS; i++)
    {
        trip_edges[i]--;
        if (trip_edges[i] == 0)
        {
            trip_edges[i] = EDGES_PER_TENTH_MILE;
            trip[i]++;
        }
    }
}

/*
 * Call this when the tacho input has an edge.
 */
void clocks_tacho_edge(void)
{
    uint32_t now = get_counter();
    tacho.period = now - tacho.last_seen;
    if (tacho.period < MIN_PERIOD)
    {
        /* Set upper limit on tacho for sanity */
        tacho.period = MIN_PERIOD;
    }
    tacho.last_seen = now;
}

/*
 * Reset the trip. You can't reset trip CLOCKS_ODOMETER.
 */
void clocks_trip_reset(enum clocks_trip_t trip_index)
{
    if (trip_index < CLOCKS_NUM_TRIPS)
    {
        if (trip_index != CLOCKS_ODOMETER)
        {
            disable_interrupts();
            trip[trip_index] = 0;
            trip_edges[trip_index] = EDGES_PER_TENTH_MILE;
            enable_interrupts();
        }
    }
}

/**************************************************
* Private Functions
***************************************************/

/* None */

/**************************************************
* End of file
***************************************************/

