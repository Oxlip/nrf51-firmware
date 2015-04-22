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

/**< Button used for deleting all stored bond information. */
#define BOND_DELETE_ALL_BUTTON_ID       15
/**< Button used to wake up the application. */
#define WAKEUP_BUTTON_PIN               16
/**< Button used to turn on/off the attached device. */
#define POWER_BUTTON_PIN                5

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          0
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            1
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               2

#define UART_RX_PIN_NUMBER              23
#define UART_TX_PIN_NUMBER              22

#define TOUCH_BUTTON_1                  28
#define TOUCH_BUTTON_2                  24
#define TOUCH_BUTTON_3                  8
#define TOUCH_BUTTON_4                  11

#define RED_LED                         9
#define GREEN_LED                       12

#define STATUS_LED_1                    9

#elif BOARD_DEV1

/**< Name of device. Will be included in the advertising data. */
#define DEVICE_NAME                     "Lyra Nrf51EK"
#define DEVICE_HW_REVISION_ID           "0.1"

/**< Button used for deleting all stored bond information. */
#define BOND_DELETE_ALL_BUTTON_ID       15
/**< Button used to wake up the application. */
#define WAKEUP_BUTTON_PIN               16
/**< Button used to turn on/off the attached device. */
#define POWER_BUTTON_PIN                17

/**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          18
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            19
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               18

#define UART_RX_PIN_NUMBER              28
#define UART_TX_PIN_NUMBER              29

#define TOUCH_BUTTON_1                  17
#define TOUCH_BUTTON_2                  18
#define TOUCH_BUTTON_3                  19
#define TOUCH_BUTTON_4                  20

#define STATUS_LED_1                    21

#elif BOARD_DEV2

/**< Name of device. Will be included in the advertising data. */
#define DEVICE_NAME                     "Lyra Nrf51DK"
#define DEVICE_HW_REVISION_ID           "0.1"

/**< Button used for deleting all stored bond information. */
#define BOND_DELETE_ALL_BUTTON_ID       17
/**< Button used to wake up the application. */
#define WAKEUP_BUTTON_PIN               18
/**< Button used to turn on/off the attached device. */
#define POWER_BUTTON_PIN                19

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          21
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            22
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               23

#define UART_RX_PIN_NUMBER              28
#define UART_TX_PIN_NUMBER              29

#define TOUCH_BUTTON_1                  17
#define TOUCH_BUTTON_2                  18
#define TOUCH_BUTTON_3                  19
#define TOUCH_BUTTON_4                  20

#define RED_LED                         21
#define GREEN_LED                       22

#define STATUS_LED_1                    21

#else

#error "Board type not defined"

#endif

#define BUTTON_PULL                     NRF_GPIO_PIN_PULLUP

/**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)

#endif /* __BOARD_CONF__ */
