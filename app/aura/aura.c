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

/** Current sensor measurement handler
 */
static void cs_meas_timeout_handler(void * p_context)
{
  uint32_t err_code;
  uint16_t len = sizeof(float);
  float cs_value = (float) sensor_get_instant_current(0);

  printf("Current sensor value %f\n", cs_value);

  // Update database
  err_code = sd_ble_gatts_value_set(cs_ss.sensor_value_handles.value_handle,
                                    0, &len, (uint8_t *)&cs_value);
  if (err_code != NRF_SUCCESS)
  {
    printf("Unable to set current sensor value %lx\n", err_code);
    return;
  }

  // Send value if connected and notifying
  if ((cs_ss.conn_handle != BLE_CONN_HANDLE_INVALID) && cs_ss.is_notification_supported)
  {
    ble_gatts_hvx_params_t hvx_params;

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = cs_ss.sensor_value_handles.value_handle;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset = 0;
    hvx_params.p_len  = &len;
    hvx_params.p_data = (uint8_t *)&cs_value;

    err_code = sd_ble_gatts_hvx(cs_ss.conn_handle, &hvx_params);
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
    static int led = 0;
    if (button_action != APP_BUTTON_PUSH)
    {
        return;
    }

    switch (pin_no)
    {
        case AURA_TOUCH_BUTTON:
            /* Do the actual button press handling here. */
            /* For now just turn on an LED */
            if (led) {
                nrf_gpio_pin_clear(AURA_TOUCH_LED);
                led = 0;
            } else {
                nrf_gpio_pin_set(AURA_TOUCH_LED);
                led = 1;
            }
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

    // Configure zero crossing as sense interrupt.
    nrf_gpio_cfg_sense_input(AURA_ZERO_CROSSING_PIN, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);
    app_gpiote_user_register(&zc_gpiote_id, 0, 1 << AURA_ZERO_CROSSING_PIN, zc_event_handler);
    app_gpiote_user_enable(zc_gpiote_id);
    
    buttons_init();

    // Configure triac pin as output.
    nrf_gpio_cfg_output(AURA_TRIAC_ENABLE);
}
