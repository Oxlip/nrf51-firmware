/**
 * Board specific configurations.
 */

#ifndef __BOARD_CONF__
#define __BOARD_CONF__

#define DEVICE_FW_REVISION_ID           BUILD_TIME

#ifdef BOARD_LYRA
/**< Name of device. Will be included in the advertising data. */
#define DEVICE_NAME                     "Lyra"
#define DEVICE_HW_REVISION_ID           "0.1"

#define UART_RX_PIN_NUMBER              23
#define UART_TX_PIN_NUMBER              22

#define TOUCH_BUTTON_1                  28
#define TOUCH_BUTTON_2                  24
#define TOUCH_BUTTON_3                  8

#define RED_LED                         9
#define GREEN_LED                       12

#define BUTTON_ACTIVE_STATE             APP_BUTTON_ACTIVE_HIGH
#define BUTTON_PIN_PULL                 NRF_GPIO_PIN_PULLDOWN

#elif BOARD_DEV2

/**< Name of device. Will be included in the advertising data. */
#define DEVICE_NAME                     "Lyra Nrf51DK"
#define DEVICE_HW_REVISION_ID           "0.1"

#define ASSERT_LED_PIN_NO               23

#define UART_RX_PIN_NUMBER              11
#define UART_TX_PIN_NUMBER              9

#define TOUCH_BUTTON_1                  17
#define TOUCH_BUTTON_2                  18
#define TOUCH_BUTTON_3                  19

#define RED_LED                         21
#define GREEN_LED                       22

#define BUTTON_ACTIVE_STATE             APP_BUTTON_ACTIVE_LOW
#define BUTTON_PIN_PULL                 NRF_GPIO_PIN_PULLUP

#else

#error "Board type not defined"

#endif

/**< Enable power management. */
#define POWER_MANAGE_ENABLED

/**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)

#endif /* __BOARD_CONF__ */
