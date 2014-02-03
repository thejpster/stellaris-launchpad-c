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
*****************************************************/

#ifndef CLOCKS_H
#define CLOCKS_H

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************
* Includes
***************************************************/

/* None */

/**************************************************
* Public Defines
***************************************************/

#define CLOCKS_DISTANCE_SCALE 10

#define CLOCKS_NUM_TRIPS (CLOCKS_TRIP_RUN + 1)

/**************************************************
* Public Data Types
**************************************************/

/* In units of 1/CLOCKS_DISTANCE_SCALE miles */
typedef uint32_t clocks_distance_t;

/* In mph, i.e. 0..255 mph */
typedef uint8_t clocks_speed_t;

/* In rpm, i.e. 0..65535 rpm */
typedef uint16_t clocks_revs_t;

enum clocks_trip_t {
    CLOCKS_ODOMETER,
    CLOCKS_TRIP_TANK,
    CLOCKS_TRIP_JOURNEY,
    CLOCKS_TRIP_RUN
};

struct clocks_state_t {
    clocks_speed_t current_speed;
    clocks_revs_t current_revs;
    clocks_distance_t trip[CLOCKS_NUM_TRIPS];
};


/**************************************************
* Public Data
**************************************************/

/* None */

/**************************************************
* Public Function Prototypes
***************************************************/

/*
 * Load trips from non-volatile memory.
 */
void clocks_init(void);

/*
 * Store trips in non-volatile memory.
 */
void clocks_save(void);

/*
 * Fetch current status.
 */
void clocks_get(struct clocks_state_t* p_state);

/*
 * This should be called four times per second.
 * It's used to detect stalled inputs (which are normally edge triggered).
 */
void clocks_timer_tick(void);

/*
 * Call this when the speedo input has an edge.
 */
void clocks_speedo_edge(void);

/*
 * Call this when the tacho input has an edge.
 */
void clocks_tacho_edge(void);

/*
 * Reset the trip. You can't reset trip 0.
 */
void clocks_trip_reset(enum clocks_trip_t trip);


#ifdef __cplusplus
}
#endif

#endif /* ndef CLOCKS_H */

/**************************************************
* End of file
***************************************************/

