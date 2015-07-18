/**
 * Common utility functions and macros.
 */

#ifndef __COMMON__
#define __COMMON__

#include <nrf_delay.h>
#include <nrf_gpio.h>
#include <board_conf.h>

/**@brief Helper function to blink LED.
 *
 * @details Blinks given led for specified number of time.
 */
static inline void blink_led(int led_pin, int on_delay, int off_delay, unsigned int count)
{
    while(count) {
        nrf_gpio_pin_clear(led_pin);
        nrf_delay_ms(on_delay);
        nrf_gpio_pin_set(led_pin);
        nrf_delay_ms(off_delay);
        count--;
    }
}

/**@brief Blink assert LED.
 */
static inline void blink_assert_led()
{
#ifdef ASSERT_LED_PIN_NO
	blink_led(ASSERT_LED_PIN_NO, 200, 200, -1);
#endif
}

/**@brief Turns on/off advertisement LED.
 */
static inline void set_advertisement_indicator(uint8_t on) {
#ifdef ADVERTISING_LED_PIN_NO
    nrf_gpio_pin_write(ADVERTISING_LED_PIN_NO, !on);
#endif
}

/**@brief Turns on/off connection LED.
 */
static inline void set_connection_indicator(uint8_t on) {
#ifdef CONNECTED_LED_PIN_NO
    nrf_gpio_pin_write(CONNECTED_LED_PIN_NO, !on);
#endif
}

/**@brief Turns on/off debug LED.
 */
static inline void set_debug_indicator(uint8_t on) {
#ifdef ASSERT_LED_PIN_NO
    nrf_gpio_pin_write(ASSERT_LED_PIN_NO, !on);
#endif
}

#endif