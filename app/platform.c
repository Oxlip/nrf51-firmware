/* Platform code common to all the devices.
 */

#include <boards.h>
#include <softdevice_handler.h>
#include <app_timer.h>
#include <app_gpiote.h>
#include <twi_master.h>

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

    nrf_gpio_pin_clear(ADVERTISING_LED_PIN_NO);
    nrf_gpio_pin_clear(CONNECTED_LED_PIN_NO);
    nrf_gpio_pin_clear(ASSERT_LED_PIN_NO);
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
    simple_uart_config(-1, UART_TX_PIN_NUMBER, -1, UART_RX_PIN_NUMBER, false);
    printf("Firmware Date: %s %s\n", __DATE__, __TIME__);
#endif
}


/**@brief Function for application main entry.
 */
int main(void)
{
    debug_init();

    // Initialize different SOC parts.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, true);
    device_timers_init();

    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);
    leds_init();

    ble_init();
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    ble_late_init();

    if (!twi_master_init()) {
        printf("Could not init i2c bus\n");
        return false;
    }

    device_init();

    // Start execution
    device_timers_start();

    // Enter main loop
    for (;;)
    {
        app_sched_execute();
#ifdef POWER_MANAGE_ENABLED
        power_manage();
#endif
    }
}
