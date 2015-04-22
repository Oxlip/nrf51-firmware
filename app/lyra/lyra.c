#include <stdio.h>
#include <string.h>
#include <nrf_gpio.h>
#include <app_timer.h>
#include <app_gpiote.h>
#include <app_button.h>
#include <ble.h>

#include "platform.h"
#include "board_conf.h"
#include "lyra_devices.h"

void device_timers_init()
{
}

void device_timers_start()
{
}


/**@brief Function for handling button events.
 *
 * @param[in]   pin_no   The pin number of the button pressed.
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    printf("Button %d action: %d\n", pin_no, button_action);
    nrf_gpio_pin_toggle(GREEN_LED);

    /* Send an event notification to HUB */
    switch (pin_no)
    {
        case TOUCH_BUTTON_1:
        case TOUCH_BUTTON_2:
        case TOUCH_BUTTON_3:
            printf("Button %d touched\n", pin_no);
            break;

        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}

/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
    uint32_t err_code;
    static app_button_cfg_t buttons[] =
    {
        {TOUCH_BUTTON_1, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_event_handler},
        {TOUCH_BUTTON_2, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_event_handler},
        {TOUCH_BUTTON_3, APP_BUTTON_ACTIVE_LOW, BUTTON_PULL, button_event_handler},
    };

    APP_BUTTON_INIT(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY, false);

    // Start handling button presses immediately.
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}

void device_init()
{
    // configure LEDs
    nrf_gpio_cfg_output(STATUS_LED_1);
    nrf_gpio_pin_set(STATUS_LED_1);

    nrf_gpio_cfg_output(RED_LED);
    nrf_gpio_cfg_output(GREEN_LED);

    nrf_gpio_pin_clear(RED_LED);
    nrf_gpio_pin_clear(GREEN_LED);

    nrf_gpio_pin_set(GREEN_LED);
    buttons_init();
}

