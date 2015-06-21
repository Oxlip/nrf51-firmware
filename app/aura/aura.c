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

#define CS_MEAS_INTERVAL          APP_TIMER_TICKS(2000, 0) /**< Current sensor measurement interval (ticks). */

app_timer_id_t cs_timer_id;
static int zc_count=0;

/** Binary format to communicate with the mobile app through BLE.
 */
typedef struct {
    uint16_t  current;
    uint16_t  watts;
    uint8_t   volt;
    uint8_t   freq;
} __attribute__((__packed__ )) ble_cs_info;

ble_cs_info cs_info = {0};

/** Current sensor measurement handler
 */
static void cs_meas_timeout_handler(void * p_context)
{
#ifdef BOARD_AURA
    float cs_rms_a, cs_rms_v, cs_active_w, cs_peak_a, cs_peak_v, cs_freq;

    cs_rms_a = (float) cs_get_rms_current(0) * 26;
    cs_rms_v = (float) cs_get_rms_voltage(0) * 700;
    cs_active_w = (float) cs_get_active_watts(0);
    cs_peak_a = (float) cs_get_peak_current(0);
    cs_peak_v = (float) cs_get_peak_voltage(0);
    cs_freq = (float)cs_get_line_frequency();

    cs_info.current = (uint16_t)cs_rms_a;
    cs_info.volt = (uint16_t)cs_rms_v;
    cs_info.watts = (uint8_t)cs_active_w;
    cs_info.freq = (uint8_t)cs_freq;
    printf("RMS Current0 %f RMS Volt0 %f Active Watts0 %f Peak A %f Peak V %f freq %f\n",
            cs_rms_a, cs_rms_v, cs_active_w, cs_peak_a, cs_peak_v, cs_freq);
#else
    cs_info.current = cs_info.current + 1;
    cs_info.volt = 1;
    cs_info.watts = 2;
    cs_info.freq = 3;
#endif

    uint32_t err_code;
    // Update BLE attribute database
    err_code = ble_ss_sensor_value_update(&cs_ss, (uint8_t *)&cs_info, sizeof(cs_info));
    if (err_code != NRF_SUCCESS)
    {
        printf("sd_ble_gatts_value_set failed: errorcode:%#lx.\n", err_code);
        return;
    }
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
            /* For now just turn on an LED */
            printf("Toggling Triac\n");
            nrf_gpio_pin_toggle(AURA_TRIAC_ENABLE);
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

static app_gpiote_user_id_t  zc_gpiote_id;

static void zc_event_handler(uint32_t event_pins_low_to_high, uint32_t event_pins_high_to_low)
{
    zc_count++;
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

    // Configure zero crossing as sense interrupt.
    nrf_gpio_cfg_sense_input(AURA_ZERO_CROSSING_PIN, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);
    app_gpiote_user_register(&zc_gpiote_id, 0, 1 << AURA_ZERO_CROSSING_PIN, zc_event_handler);
    app_gpiote_user_enable(zc_gpiote_id);

    buttons_init();

    // Configure triac pin as output.
    nrf_gpio_cfg_output(AURA_TRIAC_ENABLE);

    cs_calibrate();
}
