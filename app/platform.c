/* Platform code common to all the devices.
 */

#include <boards.h>
#include <softdevice_handler.h>
#include <app_timer.h>
#include <app_gpiote.h>

#include <board_conf.h>
#include <board_export.h>

#include "platform.h"

/* common init routines defined in ble_common.c */
void gap_params_init(void);
void advertising_init(void);
void sec_params_init(void);
void conn_params_init(void);
void ble_stack_init(void);

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    nrf_gpio_cfg_output(ADVERTISING_LED_PIN_NO);
    nrf_gpio_cfg_output(CONNECTED_LED_PIN_NO);
    nrf_gpio_pin_clear(CONNECTED_LED_PIN_NO);
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

/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize
    timers_init();
    gpiote_init();
    leds_init();

    ble_stack_init();
    scheduler_init();

    gap_params_init();
    services_init();
    conn_params_init();
    sec_params_init();
    advertising_init();

    // Start execution
    timers_start();

    // Enter main loop
    for (;;)
    {
        power_manage();
    }
}
