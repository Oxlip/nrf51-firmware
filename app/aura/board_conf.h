/**
 * Board specific configurations.
 */

#ifndef __BOARD_CONF__
#define __BOARD_CONF__

#define WAKEUP_BUTTON_PIN               BUTTON_0                                    /**< Button used to wake up the application. */

#define ADVERTISING_LED_PIN_NO          LED_0                                       /**< Is on when device is advertising. */
#define CONNECTED_LED_PIN_NO            LED_1                                       /**< Is on when device has connected. */

#define LEDBUTTON_LED_PIN_NO            LED_0
#define LEDBUTTON_BUTTON_PIN_NO         BUTTON_1

#define DEVICE_NAME                     "uPlug"                                     /**< Name of device. Will be included in the advertising data. */

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50, APP_TIMER_PRESCALER)    /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#endif
