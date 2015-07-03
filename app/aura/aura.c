#include <stdio.h>
#include <string.h>
#include <nrf_gpio.h>
#include <app_timer.h>
#include <app_gpiote.h>
#include <app_button.h>
#include <ble.h>

#include "ble_ss.h"
#include "platform.h"
#include "board_conf.h"
#include "sensor.h"
#include "aura.h"

#define CS_MEAS_INTERVAL          APP_TIMER_TICKS(3000, APP_TIMER_PRESCALER) /**< Current sensor measurement interval (ticks). */

app_timer_id_t cs_timer_id;

cs_info_t cs_info = {0};

#define CS_RMS_A_MULTIPLIER     22.6f
#define CS_RMS_V_MULTIPLIER     700
#define CS_ACTIVE_W_MULTIPLIER  15.6f

#define MILLI                   1000

/** Current sensor measurement handler
 */
static void cs_meas_timeout_handler(void * p_context)
{
#ifndef BOARD_AURA
    static
#endif
    float cs_rms_a=0.0026f, cs_rms_v=0.346f, cs_active_w=0.0009f, cs_peak_a, cs_peak_v, cs_freq;

#ifdef BOARD_AURA
    cs_rms_a = (float) cs_get_rms_current(0);
    cs_rms_v = (float) cs_get_rms_voltage(0);
    cs_active_w = (float) cs_get_active_watts(0);
    cs_peak_a = (float) cs_get_peak_current(0);
    cs_peak_v = (float) cs_get_peak_voltage(0);
    cs_freq = (float)cs_get_line_frequency();
#else
    cs_rms_a += 0.001f;
#endif

    cs_info.current = (uint16_t)(cs_rms_a * CS_RMS_A_MULTIPLIER * MILLI);
    cs_info.watts = (uint16_t)(cs_active_w * CS_ACTIVE_W_MULTIPLIER * MILLI);
    cs_info.volt = (uint8_t)(cs_rms_v * CS_RMS_V_MULTIPLIER);
    cs_info.freq = (uint8_t)cs_freq;

    printf("RMS Current0 %f RMS Volt0 %f Active Watts0 %f Peak A %f Peak V %f freq %f\n",
            cs_rms_a, cs_rms_v, cs_active_w, cs_peak_a, cs_peak_v, cs_freq);

    ble_cs_update_value(&cs_info);
}

void device_timers_init()
{
    uint32_t err_code;
    err_code = app_timer_create(&cs_timer_id, APP_TIMER_MODE_REPEATED, cs_meas_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

void device_timers_start()
{
    uint32_t err_code;
    err_code = app_timer_start(cs_timer_id, CS_MEAS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling button events.
 *
 * @param[in]   pin_no   The pin number of the button pressed.
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    if (button_action != APP_BUTTON_PUSH)
    {
        return;
    }

    switch (pin_no)
    {
        case AURA_TOUCH_BUTTON:
            /* Do the actual button press handling here. */
            triac_set(0, TRIAC_OPERATION_TOGGLE);
            break;

        default:
            APP_ERROR_HANDLER(pin_no);
    }
}


/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
    uint32_t err_code;
    static app_button_cfg_t buttons[] =
    {
        {AURA_TOUCH_BUTTON, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_event_handler},
    };

    APP_BUTTON_INIT(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY, false);

    // Start handling button presses immediately.
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}


void
triac_set(int triac, triac_operation_t operation)
{
    if (operation == TRIAC_OPERATION_OFF){
        nrf_gpio_pin_clear(TRIAC_1_PIN);
    } else if (operation == TRIAC_OPERATION_ON){
        nrf_gpio_pin_set(TRIAC_1_PIN);
    } else if (operation == TRIAC_OPERATION_TOGGLE) {
        nrf_gpio_pin_toggle(TRIAC_1_PIN);
    }
}


void
device_init()
{
    // configure LEDs
    nrf_gpio_cfg_output(AURA_TOUCH_LED);
    nrf_gpio_pin_set(AURA_TOUCH_LED);

#ifdef AURA_CS_RESET
    nrf_gpio_cfg_output(AURA_CS_RESET);
    nrf_gpio_pin_set(AURA_CS_RESET);
#endif

    buttons_init();

    // Configure triac pin as output.
    nrf_gpio_cfg_output(TRIAC_1_PIN);

#if BOARD_AURA
    cs_calibrate();
#endif
}
