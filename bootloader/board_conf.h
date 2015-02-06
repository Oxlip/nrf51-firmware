/**
 * Board specific configurations.
 */

#ifndef __BOARD_CONF__
#define __BOARD_CONF__

#ifdef BOARD_AURA

/**< Button used to enter SW update mode. */
#define BOOTLOADER_BUTTON_PIN           3

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          0
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            1
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               2

#define UART_RX_PIN_NUMBER              23
#define UART_TX_PIN_NUMBER              22
#define UART_CTS_PIN_NUMBER             0
#define UART_RTS_PIN_NUMBER             0

#elif BOARD_DEV1

/**< Button used to enter SW update mode. */
#define BOOTLOADER_BUTTON_PIN           18

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          18
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            19
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               18

#define UART_RX_PIN_NUMBER              11
#define UART_TX_PIN_NUMBER              9
#define UART_CTS_PIN_NUMBER             10
#define UART_RTS_PIN_NUMBER             8

#elif BOARD_DEV2

/**< Button used to enter SW update mode. */
#define BOOTLOADER_BUTTON_PIN           18

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          18
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            19
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               18

#define UART_RX_PIN_NUMBER              11
#define UART_TX_PIN_NUMBER              9
#define UART_CTS_PIN_NUMBER             10
#define UART_RTS_PIN_NUMBER             8

#else

#error "Board type not defined"

#endif

#endif /* __BOARD_CONF__ */