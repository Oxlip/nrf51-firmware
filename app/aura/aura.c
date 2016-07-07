#include <stdio.h>
#include <string.h>
#include <nrf_gpio.h>
#include <app_timer.h>
#include <app_gpiote.h>
#include <app_button.h>
#include <ble.h>

#include <ble_ss.h>
#include <platform.h>
#include <boards.h>
#include <drivers/cs_78m6610_lmu.h>
#include <common.h>
#include "aura.h"
#include "cs.h"

#define TOUCH_LED       LED_4

static void configure_leds()
{
    // configure LEDs
    nrf_gpio_cfg_output(LED_1);
    nrf_gpio_cfg_output(LED_2);
    nrf_gpio_cfg_output(LED_3);
    nrf_gpio_cfg_output(TOUCH_LED);

    led_on(TOUCH_LED);
}

void device_timers_init()
{
    cs_timers_init();
}

void device_timers_start()
{
    cs_timers_start();
}

#define TOUCH_POWER_BUTTON BUTTON_1

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
        case TOUCH_POWER_BUTTON:
            /* Do the actual button press handling here. */
            led_toggle(TOUCH_LED);
            triac_set(0, TRIAC_OPERATION_TOGGLE);
            break;

        default:
            APP_ERROR_HANDLER(pin_no);
    }
}

/**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)

/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
    uint32_t err_code;
    static app_button_cfg_t buttons[] =
    {
        {TOUCH_POWER_BUTTON, BUTTON_ACTIVE_STATE, BUTTON_PULL, button_event_handler},
    };

    app_button_init(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY);

    // Start handling button presses immediately.
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}

void
triac_set(int triac, triac_operation_t operation)
{
    static uint8_t triac_state = 0;

    printf("Triac state - current %s. Requested op %d\n",
            triac_state ? "On" : "Off", operation);

    if (operation == TRIAC_OPERATION_OFF){
        nrf_gpio_pin_clear(TRIAC_1);
        led_off(TOUCH_LED);
        triac_state = 0;
    } else if (operation == TRIAC_OPERATION_ON){
        nrf_gpio_pin_set(TRIAC_1);
        led_on(TOUCH_LED);
        triac_state = 1;
    } else if (operation == TRIAC_OPERATION_TOGGLE) {
        nrf_gpio_pin_toggle(TRIAC_1);
        led_toggle(TOUCH_LED);
        triac_state = !triac_state;
    }

    dimmer_msg_t msg = {
        .triac = 0,
        .value = triac_state
    };
    ble_dimmer_update_value(&msg);
}

void
device_init()
{
    configure_leds();

#ifdef AURA_CS_RESET
    nrf_gpio_cfg_output(AURA_CS_RESET);
    nrf_gpio_pin_set(AURA_CS_RESET);
#endif

    buttons_init();

    // Configure triac pin as output.
    nrf_gpio_cfg_output(TRIAC_1);

#ifdef BOARD_AURA_V1
    cs_calibrate();
#endif
}
