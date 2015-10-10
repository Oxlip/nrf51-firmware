/* Platform code common to all the devices.
 */

#include <softdevice_handler.h>
#include <app_timer.h>
#include <app_gpiote.h>
#include <app_uart.h>
#include <app_trace.h>
#include <app_scheduler.h>
#include <app_timer_appsh.h>
#include <twi_master.h>

#include <boards.h>
#include <board_export.h>

#include "platform.h"
#include "common.h"

#define UART_RX_BUF_SIZE 16
#define UART_TX_BUF_SIZE 128

bool scan_i2c_bus(void)
{
  uint8_t addr = 0, data = 0, data_lenght = 1;
  int found = 0;
  for (addr = 0; addr < 128; addr++) {
    if (twi_master_transfer(addr << 1 | TWI_READ_BIT, &data, data_lenght, false)) {
      found = 1;
      printf("Found device on address %u(%#x)\n", addr, addr);
    }
  }
  if (!found) {
    printf("Failed to find any devices on the bus!\n");
  }
  return found;
}


/* BLE init routines from ble_common.c */
void ble_init(void);
void ble_late_init(void);

/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
#ifdef CONNECTED_LED_PIN_NO
    nrf_gpio_cfg_output(ADVERTISING_LED_PIN_NO);
    nrf_gpio_cfg_output(CONNECTED_LED_PIN_NO);
    set_advertisement_indicator(0);
    set_connection_indicator(0);
#endif

#ifdef ASSERT_LED_PIN_NO
    nrf_gpio_cfg_output(ASSERT_LED_PIN_NO);
    set_debug_indicator(0);
#endif
}

/**@brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_communication);
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
        APP_ERROR_HANDLER(p_event->data.error_code);
    }
}

/**@brief  Function for initializing the UART module.
 */
/**@snippet [UART Initialization] */
static void uart_init(void)
{
    uint32_t                     err_code;
    const app_uart_comm_params_t comm_params =
    {
        RX_PIN_NUMBER,
        TX_PIN_NUMBER,
        -1,
        -1,
        false,
        false,
        UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_error_handle,
                       APP_IRQ_PRIORITY_LOW,
                       err_code);
    APP_ERROR_CHECK(err_code);
}

/**@brief Initialize debug functionality.
 */
void debug_init(void)
{
#ifdef DEBUG
#ifdef BOOTLOADER
    const char app[] = "Bootloader";
#else
    const char app[] = "Firmware";
#endif
    uart_init();
    app_trace_init();

    printf("%s (%s) Date: %s %s\n", app, DEVICE_NAME, __DATE__, __TIME__);
#endif
}


/**@brief Function for application main entry.
 */
int main(void)
{
    debug_init();

    // Initialize different SOC parts.
    APP_TIMER_APPSH_INIT(APP_TIMER_PRESCALER, APP_TIMER_MAX_TIMERS, APP_TIMER_OP_QUEUE_SIZE, true);
    device_timers_init();

    APP_GPIOTE_INIT(APP_GPIOTE_MAX_USERS);

    leds_init();

    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);

    ble_init();
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
