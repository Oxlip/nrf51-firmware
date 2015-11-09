/** Current sensor data gathering interface.
 *  This file implements sensor value gathering and storing them in RAM for
 *  predefined number samples.
 *
 *  Current sensor values are measured every few seconds and
 *  stored in a circular array. There are different arrays for each time
 *  unit(seconds, minutes etc). When the lowest unit completes full cycle,
 *  the upper units entry will be created.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <app_timer.h>
#include <platform.h>
#include <boards.h>
#include <drivers/cs_78m6610_lmu.h>
#include "aura.h"

/* Every 3 seconds measure current sensor value. 3 is chosen just arbitrarily since
 * 1 or 2 causes too much BLE traffic which results in BLE timeout(may be
 * problem in phone), and anything greater than 5 seconds seems to be too laggy
 * in UI.
 */
#define CS_MEAS_INTERVAL_IN_SEC   3
#define CS_MEAS_INTERVAL          APP_TIMER_TICKS(CS_MEAS_INTERVAL_IN_SEC * 1000, APP_TIMER_PRESCALER) /**< Current sensor measurement interval (ticks). */

/* Constant defining how many samples to store.
 * Note - the current implementation relies on the phone to store in persistent
 * storage.
 */
#define CS_DATA_SAMPLE_COUNT_SECONDS    (60 / CS_MEAS_INTERVAL_IN_SEC)  //1 minute
#define CS_DATA_SAMPLE_COUNT_MINUTES    (60)                            //1 hour
#define CS_DATA_SAMPLE_COUNT_HOURS      (24)                            //1 day
#define CS_DATA_SAMPLE_COUNT_DAYS       (30)                            //1 month

/* Meta data to manage circular array of cs_info_t */
typedef struct cs_info_carray {
    uint8_t last;       //index of the last entry - insert happens here.
    uint8_t total;      //total number of entries.
    cs_info_t array[0]; //actual array will follow this structure.
} cs_info_carray_t;

/* Expands a short name to actual cs_info_xxx_array at compile time. */
#define CS_ARRAY(_name) cs_info_##_name##_array

/* Defines cs_info array with given number of entries(total) */
#define CS_ARRAY_DEFINE(_name, _total)                          \
struct {                                                        \
    cs_info_carray_t ca_info;                                   \
    cs_info_t array[_total];                                    \
} CS_ARRAY(_name) = {                                           \
    .ca_info = {                                                \
        .last = 0,                                              \
        .total = _total                                         \
    }                                                           \
}

/* Insert given cs_info into given array.
 */
#define CS_ARRAY_INSERT(_name, _info)                           \
do {                                                            \
    cs_info_carray_t *ca_info = &CS_ARRAY(_name).ca_info;       \
    memcpy(&CS_ARRAY(_name).array[ca_info->last],               \
           &_info, sizeof(cs_info_t));                          \
    ca_info->last++;                                            \
    if (ca_info->last >= ca_info->total) {                      \
        ca_info->last = 0;                                      \
    }                                                           \
} while(0)

/* If lower_name array is full, calculate average and insert the
 * entry into given name array.
 */
#define CS_ARRAY_INSERT_IF_NEEDED(_name, _lower_name)           \
do {                                                            \
    cs_info_carray_t *ca_info = &CS_ARRAY(_lower_name).ca_info; \
    if (ca_info->last == 0) {                                   \
        cs_info_t avg;                                          \
        cs_calc_average(ca_info->array, ca_info->total, &avg);  \
        CS_ARRAY_INSERT(_name, avg);                            \
    }                                                           \
} while(0)

/* Insert given info into seconds and propagate changes
 * if needed into upper array.
 */
#define CS_INSERT(_info)                                        \
do {                                                            \
    CS_ARRAY_INSERT(seconds, _info);                            \
    CS_ARRAY_INSERT_IF_NEEDED(minutes, seconds);                \
    CS_ARRAY_INSERT_IF_NEEDED(hours, minutes);                  \
    CS_ARRAY_INSERT_IF_NEEDED(days, hours);                     \
} while(0)

/* Define all the arrays */
CS_ARRAY_DEFINE(seconds, CS_DATA_SAMPLE_COUNT_SECONDS);
CS_ARRAY_DEFINE(minutes, CS_DATA_SAMPLE_COUNT_MINUTES);
CS_ARRAY_DEFINE(hours, CS_DATA_SAMPLE_COUNT_HOURS);
CS_ARRAY_DEFINE(days, CS_DATA_SAMPLE_COUNT_DAYS);

/** Calculate average values of all fields in the given cs_info array.
 */
static inline void
cs_calc_average(cs_info_t *cs_info, uint8_t total, cs_info_t *result)
{
    int i;

    memset(result, 0, sizeof(cs_info_t));

    for(i = 0; i < total; i++) {
        result->current += cs_info[i].current;
        result->watts += cs_info[i].watts;
        result->volt += cs_info[i].volt;
        result->freq += cs_info[i].freq;
    }
    result->current /= total;
    result->watts /= total;
    result->volt /= total;
    result->freq /= total;
}

/** Current sensor measurement handler.
 *   1. Reads the value from current sensor.
 *   2. Converts it into metric.
 *   3. Update the arrays.
 *   4. Post BLE notification.
 */
static void cs_measurement_handler(void * p_context)
{
    cs_info_t cs_info;

#ifdef BOARD_PCA10028
    cs_info.watts = 250 + (rand() % 10);
    cs_info.volt = 230 + (rand() % 10);
    cs_info.current = (cs_info.watts * 1000) / cs_info.volt;
    cs_info.freq = 47 + (rand() % 5);

    printf("current %d %d %d %d\n", cs_info.current, cs_info.watts, cs_info.volt, cs_info.freq);
#else
    float cs_rms_a, cs_rms_v, cs_active_w, cs_freq;

    cs_rms_a = (float) cs_get_rms_current(0);
    cs_rms_v = (float) cs_get_rms_voltage(0);
    cs_active_w = (float) cs_get_active_watts(0);
    cs_freq = (float) cs_get_line_frequency();

    /* Constants required for current sensor value conversion
     * from bits to metric.
     */
    #define CS_RMS_A_MULTIPLIER     22.6f
    #define CS_RMS_V_MULTIPLIER     700
    #define CS_ACTIVE_W_MULTIPLIER  15.6f

    #define MILLI                   1000

    cs_info.current = (uint16_t)(cs_rms_a * CS_RMS_A_MULTIPLIER * MILLI);
    cs_info.watts = (uint16_t)(cs_active_w * CS_ACTIVE_W_MULTIPLIER * MILLI);
    cs_info.volt = (uint8_t)(cs_rms_v * CS_RMS_V_MULTIPLIER);
    cs_info.freq = (uint8_t)cs_freq;
#endif

    CS_INSERT(cs_info);

    ble_cs_update_value(&cs_info);
}

static app_timer_id_t cs_timer_id;
void cs_timers_init()
{
    uint32_t err_code;
    err_code = app_timer_create(&cs_timer_id, APP_TIMER_MODE_REPEATED, cs_measurement_handler);
    APP_ERROR_CHECK(err_code);
}

void cs_timers_start()
{
    uint32_t err_code;
    err_code = app_timer_start(cs_timer_id, CS_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}
