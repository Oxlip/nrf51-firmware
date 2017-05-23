#ifndef _AURA_V1_H
#define _AURA_V1_H

// LEDs definitions for Aura
#define LEDS_NUMBER    2

#define LED_START      27
#define LED_1          27
#define LED_2          29
#define LED_3          LED_1
#define LED_4          LED_2
#define LED_STOP       29

#ifdef BOOTLOADER
#define ADVERTISING_LED_PIN_NO      LED_1
#define CONNECTED_LED_PIN_NO        LED_2
#define ASSERT_LED_PIN_NO           LED_1
#endif

#define LEDS_LIST { LED_1, LED_2 }

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_1
#define BSP_LED_3      LED_2

#define BUTTONS_NUMBER 1

#define BUTTON_START   4
#define BUTTON_1       4
#define BUTTON_STOP    4
#define BUTTON_PULL    NRF_GPIO_PIN_NOPULL
#define BUTTON_ACTIVE_STATE  APP_BUTTON_ACTIVE_HIGH

#define BUTTONS_LIST { BUTTON_1 }

#define RX_PIN_NUMBER  6
#define TX_PIN_NUMBER  7
#define CTS_PIN_NUMBER -1
#define RTS_PIN_NUMBER -1
#define HWFC           false

#define TRIAC_1        12

#define SPIS_MISO_PIN  25
#define SPIS_CSN_PIN   16
#define SPIS_MOSI_PIN  24
#define SPIS_SCK_PIN   26

#define WIFI_ENABLE_PIN 28

#define DEVICE_NAME		"Aura"


#endif // _AURA_V1_H
