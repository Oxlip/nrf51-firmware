/**
 * Board specific configurations.
 */

#ifndef __BOARD_CONF__
#define __BOARD_CONF__

#define BOOTLOADER_REVISION_ID           "0.1 "__DATE__" "__TIME__

#ifdef BOARD_AURA

/**< Button used to enter SW update mode. */
#define BOOTLOADER_BUTTON_PIN           7

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          4
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            5
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               6

#elif BOARD_DEV1

/**< Button used to enter SW update mode. */
#define BOOTLOADER_BUTTON_PIN           18

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          18
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            19
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               18

#elif BOARD_DEV2

/**< Button used to enter SW update mode. */
#define BOOTLOADER_BUTTON_PIN           18

/**< Is on when device is advertising. */
#define ADVERTISING_LED_PIN_NO          18
/**< Is on when device has connected. */
#define CONNECTED_LED_PIN_NO            19
/**< Is on when application has asserted. */
#define ASSERT_LED_PIN_NO               18

#else

#error "Board type not defined"

#endif

#endif /* __BOARD_CONF__ */
