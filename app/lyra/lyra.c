#include <stdio.h>
#include <string.h>
#include <nrf_gpio.h>
#include <app_timer.h>
#include <app_gpiote.h>
#include <app_button.h>
#include <ble.h>
#include <lis2dh.h>

#include "platform.h"
#include "board_conf.h"
#include "lyra_devices.h"
#include <device_manager_s130.h>
#include "pstorage.h"

pstorage_handle_t lyra_pstorage_handle;                                      /**< Persistent storage handle for blocks requested by the module. */
lyra_button_action_t lyra_button_actions[LYRA_MAX_BUTTONS][LYRA_MAX_ACTIONS];
uint8_t lyra_app_data[20];   /* temporary storage to write to flash */

void device_timers_init()
{
}

void device_timers_start()
{
}

/**@brief Function for handling button events.
 *
 * @param[in]   pin_no   The pin number of the button pressed.
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    printf("Button %d action: %d\n", pin_no, button_action);
    nrf_gpio_pin_toggle(GREEN_LED);
    lis2dh_print_values();

    /* Send an event notification to HUB */
    switch (pin_no)
    {
        case TOUCH_BUTTON_1:
        case TOUCH_BUTTON_2:
        case TOUCH_BUTTON_3:
            printf("Button %d touched\n", pin_no);
            lyra_handle_button_event(pin_no, button_action);
            break;

        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}

/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
    uint32_t err_code;
    static app_button_cfg_t buttons[] =
    {
        {TOUCH_BUTTON_1, BUTTON_ACTIVE_STATE, BUTTON_PIN_PULL, button_event_handler},
        {TOUCH_BUTTON_2, BUTTON_ACTIVE_STATE, BUTTON_PIN_PULL, button_event_handler},
        {TOUCH_BUTTON_3, BUTTON_ACTIVE_STATE, BUTTON_PIN_PULL, button_event_handler},
    };

    APP_BUTTON_INIT(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY, false);

    // Start handling button presses immediately.
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}
extern void scan_bus();


/* Callback handler from pstorage load/store events */
static void lyra_pstorage_cb_handler(pstorage_handle_t * p_handle,
                                   uint8_t             op_code,
                                   uint32_t            result,
                                   uint8_t           * p_data,
                                   uint32_t            data_len)
{

    /*
     * write callback from ble write  we have to do a pstorage_store.
     *     -> APP / sim sends us the write message
     *     -> button_array[button_id][action] = <value><peer_addr><....>
     * upon button press next set of actions
     * - display the pstorage information - 1st item
     */
    printf("%s: callback from flash events\n", __FUNCTION__); 
}

/* Register a new pstorage_handle for storing lyra button actions */
void 
dm_register_new_pstorage_handle (void)
{
    uint32_t          err_code;
    //All context with respect to a particular device is stored contiguously.
    pstorage_module_param_t param;

    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    /* Need the block size to be word aligned */
    param.block_size  = sizeof(lyra_button_action_t) * 4;
    param.block_count = LYRA_MAX_ACTION_COUNT;
    param.cb          = lyra_pstorage_cb_handler;

    err_code = pstorage_register(&param, &lyra_pstorage_handle);
    printf("%s: Pstorage for Lyra registered, Block_id(Start Addr): %x, err %d\n", __FUNCTION__,
            (unsigned int) lyra_pstorage_handle.block_id, (int) err_code);
}

void device_init()
{
    // configure LEDs
    nrf_gpio_cfg_output(STATUS_LED_1);
    nrf_gpio_pin_set(STATUS_LED_1);

    nrf_gpio_cfg_output(RED_LED);
    nrf_gpio_cfg_output(GREEN_LED);

    nrf_gpio_pin_clear(RED_LED);
    nrf_gpio_pin_clear(GREEN_LED);

    nrf_gpio_pin_set(GREEN_LED);
    buttons_init();

#if BOARD_LYRA
    lis2dh_init();
    scan_bus();
#endif

    /* Pstorage handling for lyra */
    dm_register_new_pstorage_handle();
}

/* Handle Button Service write events */
void
dm_store_write_data_evt(uint8_t *data, uint8_t len)
{
    pstorage_handle_t *p_handle;
    lyra_button_action_t *action;
    uint32_t offset;
    int i;
    uint32_t err_code;

    p_handle = &lyra_pstorage_handle;

    /* 
     * Store the data received in RAM and write to pstorage at a specific
     * location based on the button index
     */
    lyra_button_char_event_t *event_data = (lyra_button_char_event_t *) data;

    printf("%s: storing device data: action %d, btn num: %d, index: %d, device"
            "type %d, value %d, len %d\n address: ", __FUNCTION__, event_data->action,
            event_data->button_number, event_data->action_index,
            event_data->device_type, event_data->value, len);
    for (i = 0; i < 6; i++) {
        printf(" %02d ", event_data->address[i]);
    }
    printf("\n");

    for (i = 0; i < len; i++) {
        lyra_app_data[i] = data[i];
    }

    offset = (event_data->button_number) * LYRA_MAX_ACTIONS + event_data->action_index;
    offset = sizeof(lyra_button_action_t) * offset;

    err_code = pstorage_store(p_handle, (uint8_t *)lyra_app_data, len, offset);

    printf("pstorage write to base %#x, offset %u done. err_code: %u\n",
            (unsigned int) p_handle->block_id, (unsigned int) offset, (unsigned int) err_code);

    if (err_code != NRF_SUCCESS)
    {
        printf("%s: failed in store function. Err: %d\n", __FUNCTION__, (int)
                err_code);
    }

    printf("%s: Updating for button %d, index: %d\n", __FUNCTION__,
            event_data->button_number, event_data->action_index);
    action =
        &lyra_button_actions[event_data->button_number][event_data->action_index];
    action->value = event_data->value;
    action->action = event_data->action;
    for (i = 0; i < 6; i++) {
        action->address[i] = event_data->address[i];
    }
}

/* Handle Button press event and try to read from flash for action information */
void
lyra_handle_button_event (uint8_t pin_no, uint8_t action)
{
    pstorage_handle_t *p_handle;
    uint8_t           flash_data[40];
    int               i, j;
    uint16_t          offset;
    uint32_t          err_code;
    lyra_button_action_t *btn_action;
    ble_gap_addr_t    peer_addr;

    p_handle = &lyra_pstorage_handle;

    switch (pin_no) {
    case TOUCH_BUTTON_1:
        pin_no = 0;
        break;
    case TOUCH_BUTTON_2:
        pin_no = 1;
        break;
    case TOUCH_BUTTON_3:
        pin_no = 2;
        break;
    default:
        printf("%s: Invalid button number: %d\n", __FUNCTION__, pin_no);
    }

    for (i = 0; i < LYRA_MAX_ACTIONS; i++) {
        offset = (pin_no) * LYRA_MAX_ACTIONS + i;
        offset = sizeof(lyra_button_action_t) * offset;
        err_code = pstorage_load((uint8_t *)flash_data, (pstorage_handle_t *)p_handle,
                                 sizeof(lyra_button_action_t), offset);


        printf("%s: Data Read from Flash: err %d", __FUNCTION__, (int) err_code);
        for (j = 0; j < sizeof(lyra_button_action_t); j++) {
            printf(" %02d ", flash_data[j]);
        }

        /* Data read from pstorage contains button action and peer address */
        btn_action = (lyra_button_action_t *) flash_data;
        memcpy(&peer_addr.addr, btn_action->address, BLE_GAP_ADDR_LEN);

        printf("%s: Sending button press event to peer\n", __FUNCTION__);
        write_to_peer_device(peer_addr, &btn_action->value, 1);
    }
}

