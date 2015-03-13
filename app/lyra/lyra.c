#include <stdio.h>
#include <string.h>
#include <nrf_gpio.h>
#include <app_timer.h>
#include <app_gpiote.h>
#include <app_button.h>
#include <ble.h>

#include "platform.h"
#include "board_conf.h"

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
    if (button_action != APP_BUTTON_PUSH)
    {
        return;
    }

    switch (pin_no)
    {
        printf("Button %d touched\n", pin_no);
        case TOUCH_BUTTON_1:
        case TOUCH_BUTTON_2:
        case TOUCH_BUTTON_3:
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

    buttons_init();
}

