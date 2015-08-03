#ifndef _LYRA_V1_H
#define _LYRA_V1_H

#define LEDS_NUMBER    2

#define LED_START      9
#define LED_1          9
#define LED_2          12
#define LED_STOP       12

#define LEDS_LIST { LED_1, LED_2 }

#define BUTTONS_NUMBER 3

#define BUTTON_START   17
#define BUTTON_1       17
#define BUTTON_2       18
#define BUTTON_3       19
#define BUTTON_STOP    19
#define BUTTON_PULL    NRF_GPIO_PIN_PULLDOWN
#define BUTTON_ACTIVE_STATE  APP_BUTTON_ACTIVE_HIGH

#define BUTTONS_LIST { BUTTON_1, BUTTON_2, BUTTON_3 }

#define RX_PIN_NUMBER  23
#define TX_PIN_NUMBER  22
#define CTS_PIN_NUMBER -1
#define RTS_PIN_NUMBER -1
#define HWFC           false

#define DEVICE_NAME		"Lyra"


#endif // _LYRA_V1_H
