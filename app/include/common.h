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

#endif