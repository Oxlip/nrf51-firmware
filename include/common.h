/**
 * Common utility functions and macros.
 */

#ifndef __COMMON__
#define __COMMON__

#include <nrf_delay.h>
#include <nrf_gpio.h>
#include <boards.h>

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
#ifdef LED_1
	blink_led(LED_1, 200, 200, -1);
#endif
}

/**@brief Turns on/off advertisement LED.
 */
static inline void set_advertisement_indicator(uint8_t on) {
#ifdef LED_2
    nrf_gpio_pin_write(LED_2, !on);
#endif
}

/**@brief Turns on/off connection LED.
 */
static inline void set_connection_indicator(uint8_t on) {
#ifdef LED_1
    nrf_gpio_pin_write(LED_1, !on);
#endif
}

/**@brief Turns on/off debug LED.
 */
static inline void set_debug_indicator(uint8_t on) {
#ifdef LED_3
    nrf_gpio_pin_write(LED_3, !on);
#endif
}

#endif