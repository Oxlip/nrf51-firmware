#include <string.h>
#include <stdio.h>
#include <nrf_gpio.h>
#include <ble.h>
#include <app_util.h>
#include <app_error.h>
#include <ble_advdata.h>
#include <ble_bas_c.h>

#include <ble_uuids.h>
#include <ble_ss.h>
#include <ble_common.h>
#include <app_timer.h>
#include <boards.h>

#include "battery.h"

#include "lyra.h"

extern void ble_peer_on_ble_evt(ble_evt_t * p_ble_evt);
ble_ss_t button_ss;
ble_ss_t button_config_ss;

void device_on_ble_evt(ble_evt_t * p_ble_evt)
{
    ble_ss_on_ble_evt(&button_ss, p_ble_evt);
    ble_ss_on_ble_evt(&button_config_ss, p_ble_evt);
    ble_ss_on_ble_evt(&battery_ss, p_ble_evt);
    ble_peer_on_ble_evt(p_ble_evt);
}

static void ble_bc_ss_write_event(ble_ss_t * p_ss, ble_gatts_evt_write_t * p_evt_write)
{
    if (p_evt_write->len == 0) {
        return;
    }

    store_button_action((ble_action_info_msg_t *)p_evt_write->data);
}

static inline void button_config_service_init(void)
{
    uint32_t err_code;
    ble_uuid_t ble_service_uuid;
    ble_uuid_t ble_char_uuid;
    ble_ss_init_t bc_ss_param;

    // Initialize lyra_bs Service.
    memset(&bc_ss_param, 0, sizeof(bc_ss_param));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bc_ss_param.sensor_value_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bc_ss_param.sensor_value_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bc_ss_param.sensor_value_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bc_ss_param.sensor_value_report_read_perm);

    bc_ss_param.evt_write_handler      = NULL;
    bc_ss_param.evt_auth_write_handler = ble_bc_ss_write_event;
    bc_ss_param.evt_auth_read_handler  = NULL;
    bc_ss_param.support_notification   = true;
    bc_ss_param.p_report_ref           = NULL;
    bc_ss_param.initial_value          = 0;

    BLE_UUID_ASSIGN(ble_service_uuid, BLE_UUID_BUTTON_CONFIG_SERVICE);
    BLE_UUID_ASSIGN(ble_char_uuid, BLE_UUID_BUTTON_CONFIG_CHAR);
    err_code = ble_ss_init(&button_config_ss, &ble_service_uuid, &ble_char_uuid, &bc_ss_param);
    APP_ERROR_CHECK(err_code);
}

typedef struct {
    uint8_t pressed;
    uint8_t held;
} button_state_bitmap_t;

button_state_bitmap_t bs_bitmap = {0};

void ble_advertising_set_button(uint8_t pressed, uint8_t held)
{
    ble_advdata_service_data_t service_data;

    if (pressed) {
        bs_bitmap.pressed |= (1 << pressed);
    }
    if (held) {
        bs_bitmap.held |= (1 << held);
    }
    service_data.service_uuid = BLE_UUID_BUTTON_SERVICE;
    service_data.data.size = sizeof(bs_bitmap);
    service_data.data.p_data = (uint8_t *)&bs_bitmap;

    ble_advertising_service_data_set(&service_data);
}

static void ble_advertising_clear_button()
{
    bs_bitmap.pressed = 0;
    bs_bitmap.held = 0;

    ble_advertising_set_button(0, 0);
}


static void ble_button_ss_write_event(ble_ss_t * p_ss, ble_gatts_evt_write_t * p_evt_write)
{
    printf("Clearning button service trigger");
    ble_advertising_clear_button();
}

static inline void button_service_init(void)
{
    uint32_t err_code;
    ble_uuid_t ble_service_uuid;
    ble_uuid_t ble_char_uuid;
    ble_ss_init_t bc_ss_param;

    // Initialize lyra_bs Service.
    memset(&bc_ss_param, 0, sizeof(bc_ss_param));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bc_ss_param.sensor_value_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bc_ss_param.sensor_value_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bc_ss_param.sensor_value_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&bc_ss_param.sensor_value_report_read_perm);

    bc_ss_param.evt_write_handler      = NULL;
    bc_ss_param.evt_auth_write_handler = ble_button_ss_write_event;
    bc_ss_param.evt_auth_read_handler  = NULL;
    bc_ss_param.support_notification   = true;
    bc_ss_param.p_report_ref           = NULL;
    bc_ss_param.initial_value          = 0;

    BLE_UUID_ASSIGN(ble_service_uuid, BLE_UUID_BUTTON_SERVICE);
    BLE_UUID_ASSIGN(ble_char_uuid, BLE_UUID_BUTTON_CHAR);
    err_code = ble_ss_init(&button_ss, &ble_service_uuid, &ble_char_uuid, &bc_ss_param);
    APP_ERROR_CHECK(err_code);
}

uint32_t services_init(void)
{
    button_config_service_init();
    button_service_init();
    battery_service_init();

    return NRF_SUCCESS;
}

