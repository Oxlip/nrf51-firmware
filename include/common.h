/**
 * Common utility functions and macros.
 */

#ifndef __COMMON__
#define __COMMON__

#include <nrf_delay.h>
#include <nrf_gpio.h>
#include <boards.h>


static inline void led_on(int pin)
{
#ifdef BOARD_PCA10028
    nrf_gpio_pin_set(pin);
#else
    nrf_gpio_pin_clear(pin);
#endif
}

static inline void led_off(int pin)
{
#ifdef BOARD_PCA10028
    nrf_gpio_pin_clear(pin);
#else
    nrf_gpio_pin_set(pin);
#endif
}

static inline void led_toggle(int pin)
{
    nrf_gpio_pin_toggle(pin);
}

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
static inline void set_advertisement_indicator(uint8_t on)
{
#ifdef ADVERTISING_LED_PIN_NO
    if (on) {
        led_on(ADVERTISING_LED_PIN_NO);
    } else {
        led_off(ADVERTISING_LED_PIN_NO);
    }
#endif
}

/**@brief Turns on/off connection LED.
 */
static inline void set_connection_indicator(uint8_t on)
{
#ifdef CONNECTED_LED_PIN_NO
    if (on) {
        led_on(CONNECTED_LED_PIN_NO);
    } else {
        led_off(CONNECTED_LED_PIN_NO);
    }
#endif
}

/**@brief Turns on/off debug LED.
 */
static inline void set_debug_indicator(uint8_t on)
{
#ifdef ASSERT_LED_PIN_NO
    if (on) {
        led_on(ASSERT_LED_PIN_NO);
    } else {
        led_off(ASSERT_LED_PIN_NO);
    }
#endif
}

#endif