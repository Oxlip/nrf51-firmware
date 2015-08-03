#include <stdio.h>
#include <string.h>
#include <nrf_gpio.h>
#include <nrf_soc.h>
#include <app_timer.h>
#include <app_gpiote.h>
#include <app_button.h>
#include <ble.h>
#include <pstorage.h>

#include <platform.h>
#include <lis2dh.h>
#include <common.h>

#include "battery.h"
#include <boards.h>
#include "lyra.h"

#define GREEN_LED LED_1
#define RED_LED   LED_2


typedef enum {
    ACTION_INFO_INVALID = -1,
    ACTION_INFO_VALID = 1
} action_info_valid;

typedef struct {
    uint8_t address[6];     /* Address of the BLE target device. */
    uint8_t value;          /* Value to send when button pressed. */
    uint8_t is_valid;       /* Set to 1 if the this is valid else -1*/
} action_info_t  __attribute__ ((aligned (4)));

#define MAX_BUTTONS         3
#define ACTIONS_PER_BUTTON  3
#define ALL_ACTION_COUNT    ((MAX_BUTTONS) * (ACTIONS_PER_BUTTON))

action_info_t button_actions[MAX_BUTTONS][ACTIONS_PER_BUTTON];

#define ACTION_INFO_OFFSET(button, sub_index)                               \
        ((button * ACTIONS_PER_BUTTON * sizeof(action_info_t)) +            \
         (sub_index * sizeof(action_info_t)))


/**< Persistent storage handle for blocks requested by the module. */
pstorage_handle_t m_pstorage_handle;

void device_timers_init()
{
}

void device_timers_start()
{
}


/* Read the pstorage and take action for the given button
 */
static void
send_value_to_peer(uint8_t button)
{
    int               i;
    uint16_t          offset;
    uint32_t          err;
    action_info_t     btn_action[ACTIONS_PER_BUTTON];
    pstorage_handle_t block_handle;

    offset = ACTION_INFO_OFFSET(button, 0);

    //Get the block handle.
    err = pstorage_block_identifier_get(&m_pstorage_handle, 0, &block_handle);
    if (err != NRF_SUCCESS) {
        printf("Failed to get block identifier\n");
    }

    err = pstorage_load((uint8_t *)btn_action, &block_handle, sizeof(btn_action), offset);
    if (err != NRF_SUCCESS) {
        printf("Failed to read value from pstorage %ld\n", err);
        //TODO - blink LED
        return;
    }

    for (i = 0; i < ACTIONS_PER_BUTTON; i++) {
        ble_gap_addr_t    peer_addr;
        if ((uint8_t) btn_action[i].is_valid == (uint8_t) ACTION_INFO_INVALID) {
            continue;
        }

        peer_addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;
        memcpy(&peer_addr.addr, btn_action[i].address, BLE_GAP_ADDR_LEN);
        write_to_peer_device(peer_addr, &btn_action[i].value, 1);
    }
}


/**@brief Function for handling button events.
 *
 * @param[in]   pin_no   The pin number of the button pressed.
 */
static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    if (button_action == 0) {
        return;
    }

    /* Measure battery whenever we wakeup */
    battery_measure_start();

    /* Send an event notification to HUB */
    switch (pin_no)
    {
        case BUTTON_1:
            send_value_to_peer(0);
            break;
        case BUTTON_2:
            send_value_to_peer(1);
            break;
        case BUTTON_3:
            send_value_to_peer(2);
            break;
        default:
            APP_ERROR_HANDLER(pin_no);
            break;
    }
}

/**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */
#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)


/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
    uint32_t err_code;
    static app_button_cfg_t buttons[] =
    {
        {BUTTON_1, BUTTON_ACTIVE_STATE, BUTTON_PULL, button_event_handler},
        {BUTTON_2, BUTTON_ACTIVE_STATE, BUTTON_PULL, button_event_handler},
        {BUTTON_3, BUTTON_ACTIVE_STATE, BUTTON_PULL, button_event_handler},
    };

    err_code = app_button_init(buttons, sizeof(buttons) / sizeof(buttons[0]), BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);

    // Start handling button presses immediately.
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}


/* Callback handler from pstorage load/store events */
static void pstorage_cb_handler(pstorage_handle_t * p_handle,
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
    printf("%s: op_code %d result %ld\n", __FUNCTION__, op_code, result);
}

/* Register a new pstorage_handle for storing lyra button actions */
void
register_pstorage_handle (void)
{
    uint32_t          err_code;
    //All context with respect to a particular device is stored contiguously.
    pstorage_module_param_t param;

    err_code = pstorage_init();
    APP_ERROR_CHECK(err_code);

    /* Need the block size to be word aligned */
    param.block_size  = sizeof(action_info_t) * ALL_ACTION_COUNT;
    param.block_count = 1;
    param.cb          = pstorage_cb_handler;

    err_code = pstorage_register(&param, &m_pstorage_handle);
    APP_ERROR_CHECK(err_code);
}

void device_init()
{
    // configure LEDs
    nrf_gpio_cfg_output(RED_LED);
    nrf_gpio_cfg_output(GREEN_LED);

#ifdef BOARD_LYRA
    nrf_gpio_pin_clear(RED_LED);
    nrf_gpio_pin_clear(GREEN_LED);
#else
    nrf_gpio_pin_set(RED_LED);
    nrf_gpio_pin_set(GREEN_LED);
#endif

    buttons_init();

#ifdef BOARD_LYRA
    extern void scan_bus();
    scan_bus();
    lis2dh_init();
#endif

    /* Pstorage handling for lyra */
    register_pstorage_handle();

    battery_measure_start();
}

/* Store button action */
void
store_button_action(ble_action_info_msg_t *msg)
{
    action_info_t action;
    uint32_t offset;
    uint32_t err;
    uint32_t addr, *data;
    pstorage_handle_t block_handle;

    /*
     * Store the data received in RAM and write to pstorage at a specific
     * location based on the button index
     */
    printf("op %d btn %d index %d type %d value %d\n",
            msg->operation, msg->button, msg->sub_index, msg->device_type, msg->value);
    printf("address : ");
    for (int i = 0; i < 6; i++) {
        printf(" %02x ", msg->address[i]);
    }
    printf("\n");

    memcpy(&action.address, msg->address, sizeof(action.address));
    if (msg->operation == BLE_ACTION_INFO_OP_ADD) {
        action.is_valid = ACTION_INFO_VALID;
    } else {
        action.is_valid = ACTION_INFO_INVALID;
    }
    action.value = msg->value;

    /* update RAM with the new values */
    memcpy(&button_actions[msg->button][msg->sub_index], &action, sizeof(action));

    //Get the block handle.
    err = pstorage_block_identifier_get(&m_pstorage_handle, 0, &block_handle);
    if (err != NRF_SUCCESS) {
        printf("Failed to get block identifier\n");
    }

    offset = ACTION_INFO_OFFSET(msg->button, msg->sub_index);
    addr = block_handle.block_id;
    addr += offset;

    printf("Writing value %#x, validity %#x\n", action.value, action.is_valid);

    /* update if there was valid data already stored in flash */
    err = pstorage_load((uint8_t *)&action, &block_handle, sizeof(action), offset);
    if ((uint8_t) action.is_valid != (uint8_t) ACTION_INFO_INVALID) {
        /* Update flash with the new value */
        data = (uint32_t *) &button_actions[msg->button][msg->sub_index];
        err = pstorage_update(&block_handle, (uint8_t *)data, sizeof(action), offset);
    } else {
        /* Store the value in flash */
        data = (uint32_t *) &button_actions[msg->button][msg->sub_index];
        err = pstorage_store(&block_handle, (uint8_t *)data, sizeof(action), offset);
    }

    if (err != NRF_SUCCESS) {
        printf("%s: failed - error: %#lx\n", __FUNCTION__, err);
        /* Retry storing these values in flash */
        return;
    }
}

