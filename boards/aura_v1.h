#ifndef _AURA_V1_H
#define _AURA_V1_H

// LEDs definitions for Aura
#define LEDS_NUMBER    4

#define LED_START      0
#define LED_1          0
#define LED_2          1
#define LED_3          2
#define LED_4          3
#define LED_STOP       3

#define LEDS_LIST { LED_1, LED_2, LED_3, LED_4 }

#define BUTTONS_NUMBER 1

#define BUTTON_START   3
#define BUTTON_1       3
#define BUTTON_STOP    3
#define BUTTON_PULL    NRF_GPIO_PIN_PULLDOWN
#define BUTTON_ACTIVE_STATE  APP_BUTTON_ACTIVE_HIGH

#define BUTTONS_LIST { BUTTON_1 }

#define RX_PIN_NUMBER  23
#define TX_PIN_NUMBER  22
#define CTS_PIN_NUMBER -1
#define RTS_PIN_NUMBER -1
#define HWFC           false

#define TRIAC_1        12

#define DEVICE_NAME		"Aura"


#endif // _AURA_V1_H
