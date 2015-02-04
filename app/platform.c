/* Platform code common to all the devices.
 */

#include <boards.h>
#include <softdevice_handler.h>
#include <app_timer.h>
#include <app_gpiote.h>

#include <board_conf.h>
#include <board_export.h>
#include "simple_uart.h"
#include "boards.h"

#include "platform.h"
#include "app_button.h"
#include "common.h"

/* BLE init routines from ble_common.c */
void ble_init(void);
void ble_late_init(void);

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    nrf_gpio_cfg_output(ADVERTISING_LED_PIN_NO);
    nrf_gpio_cfg_output(CONNECTED_LED_PIN_NO);
    nrf_gpio_cfg_output(ASSERT_LED_PIN_NO);
    nrf_gpio_cfg_output(AURA_TOUCH_LED);

    nrf_gpio_pin_clear(ADVERTISING_LED_PIN_NO);
    nrf_gpio_pin_clear(CONNECTED_LED_PIN_NO);
    nrf_gpio_pin_clear(ASSERT_LED_PIN_NO);
    nrf_gpio_pin_clear(AURA_TOUCH_LED);
}

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    // Initialize timer module, making it use the scheduler
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, true);
}

/**@brief Function for the Event Scheduler initialization.
 */
static void scheduler_init(void)
{
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
}


/**@brief Function for initializing the GPIOTE handler module.
 */
static void gpiote_init(void)
{
    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
}

/**@brief Function for handling button events.
 *
 * @param[in]   pin_no   The pin number of the button pressed.
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    static int led = 0;
    if (button_action == APP_BUTTON_PUSH)
    {
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

/**@brief Function to start timers.
 */
static void timers_start(void)
{

}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

/**@brief Implement _write() std library function.
 *        This is needed for printf().
 */
int _write(int fd, const char * str, int len) __attribute__ ((used));
int _write(int fd, const char * str, int len)
{
#ifdef DEBUG
    for (int i = 0; i < len; i++)
    {
        simple_uart_put(str[i]);
    }
#endif
    return len;
}


/**@brief Implement puts() std library function.
 *        This is needed for printf()(which calls puts() if no argument is passed).
 */
int puts(const char *str)
{
#ifdef DEBUG
    return _write(0, str, __builtin_strlen(str));
#else
    return 0;
#endif
}

int fputc(int ch, FILE * p_file) 
{
    simple_uart_put((uint8_t)ch);
    return 0;
}

/**@brief Initialize debug functionality.
 */
static void debug_init(void)
{
#ifdef DEBUG
    simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, false);
    printf("Firmware Date: %s %s\n", __DATE__, __TIME__);
#endif
}


/**@brief Function for application main entry.
 */
int main(void)
{
    debug_init();

    // Initialize different SOC parts.
    timers_init();
    gpiote_init();
    leds_init();
    buttons_init();

    blink_led(CONNECTED_LED_PIN_NO, 100, 100, 2);

    ble_init();
    scheduler_init();
    ble_late_init();

    // Start execution
    timers_start();

    // Enter main loop
    for (;;)
    {
        app_sched_execute();
        power_manage();
    }
}
