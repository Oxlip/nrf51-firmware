/**
 * Board specific configurations.
 */

#ifndef __BOARD_CONF__
#define __BOARD_CONF__

/**< Name of device. Will be included in the advertising data. */
#define DEVICE_NAME                     "Aura"

/**< Button used for deleting all stored bond information. */
#define BOND_DELETE_ALL_BUTTON_ID       15
/**< Button used to wake up the application. */
#define WAKEUP_BUTTON_PIN               16
/**< Button used to turn on/off the attached device. */
#define POWER_BUTTON_PIN                17


/**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          18
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            19
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               20

/**< Number of service to advertise.*/
#define ADV_BLE_SERVICE_COUNT              2

#endif
