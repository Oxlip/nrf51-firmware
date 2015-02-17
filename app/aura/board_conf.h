/**
 * Board specific configurations.
 */

#ifndef __BOARD_CONF__
#define __BOARD_CONF__

#define DEVICE_FW_REVISION_ID           "0.1 "__DATE__" "__TIME__


#ifdef BOARD_AURA
/**< Name of device. Will be included in the advertising data. */
#define DEVICE_NAME                     "Aura"
#define DEVICE_HW_REVISION_ID           "0.1"

/**< Button used for deleting all stored bond information. */
#define BOND_DELETE_ALL_BUTTON_ID       15
/**< Button used to wake up the application. */
#define WAKEUP_BUTTON_PIN               16
/**< Button used to turn on/off the attached device. */
#define POWER_BUTTON_PIN                5
/**< Button used to turn on/off Aura LEDs. */
#define AURA_TOUCH_BUTTON               3

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          0
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            1
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               2
/**< Glow LED when Touch button is pressed. */
#define AURA_TOUCH_LED                  4
#define AURA_TRIAC_ENABLE               12

#define UART_RX_PIN_NUMBER              10
#define UART_TX_PIN_NUMBER              8

#define BUTTON_PULL                     NRF_GPIO_PIN_PULLUP

#elif BOARD_DEV1

/* BOARD_AURA_DEV1 */

/**< Name of device. Will be included in the advertising data. */
#define DEVICE_NAME                     "Aura Nrf51EK"
#define DEVICE_HW_REVISION_ID           "0.1"

/**< Button used for deleting all stored bond information. */
#define BOND_DELETE_ALL_BUTTON_ID       15
/**< Button used to wake up the application. */
#define WAKEUP_BUTTON_PIN               16
/**< Button used to turn on/off the attached device. */
#define POWER_BUTTON_PIN                17
/**< Button used to turn on/off Aura LEDs. */
#define AURA_TOUCH_BUTTON               18

/**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          18
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            19
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               18
/**< Glow LED when Touch button is pressed. */
#define AURA_TOUCH_LED                  19

#define AURA_TRIAC_ENABLE               19

#define UART_RX_PIN_NUMBER              9
#define UART_TX_PIN_NUMBER              11

#elif BOARD_DEV2
/* BOARD_AURA_DEV2 */

/**< Name of device. Will be included in the advertising data. */
#define DEVICE_NAME                     "Aura Nrf51DK"
#define DEVICE_HW_REVISION_ID           "0.1"

/**< Button used for deleting all stored bond information. */
#define BOND_DELETE_ALL_BUTTON_ID       17
/**< Button used to wake up the application. */
#define WAKEUP_BUTTON_PIN               18
/**< Button used to turn on/off the attached device. */
#define POWER_BUTTON_PIN                19
/**< Button used to turn on/off Aura LEDs. */
#define AURA_TOUCH_BUTTON               20

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          21
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            22
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               23
/**< Glow LED when Touch button is pressed. */
#define AURA_TOUCH_LED                  24
#define AURA_TRIAC_ENABLE               24

#define UART_RX_PIN_NUMBER              28
#define UART_TX_PIN_NUMBER              29

#else

#error "Board type not defined"

#endif


/* LED Mappings from Schematic */
#define AURA_LED_GREEN                  0
#define AURA_LED_BLUE                   1
#define AURA_LED_RED                    2

#define AURA_ZERO_CROSSING_PIN          13

/**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)

#endif /* __BOARD_CONF__ */
