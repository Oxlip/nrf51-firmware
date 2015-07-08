#include <string.h>
#include <stdio.h>
#include <nrf_gpio.h>
#include <ble.h>
#include <app_util.h>
#include <app_error.h>
#include <ble_advdata.h>

#include <ble_uuids.h>
#include <ble_ss.h>
#include <ble_common.h>
#include <blec_common.h>
#include <app_timer.h>
#include <board_conf.h>

#include <pstorage.h>
#include <device_manager_s130.h>
#include <ble_db_discovery_s130.h>
#include <pstorage_platform.h>
#include "ble_dim_c.h"
#include "lyra_devices.h"

extern void on_central_ble_evt(ble_evt_t * p_ble_evt);
extern  ble_dim_c_t m_ble_dim_c;                         /**< Structure used to identify the Battery Service client module. */
ble_ss_t lyra_bs_ss;

/**< Persistent storage handle for blocks requested by the module. */
extern pstorage_handle_t lyra_pstorage_handle;


void device_on_ble_evt(ble_evt_t * p_ble_evt)
{
    ble_ss_on_ble_evt(&lyra_bs_ss, p_ble_evt);
#ifdef USE_CENTRAL_MODE
#if 0
    /* old code: not required to be invokved */
    blec_on_ble_evt(p_ble_evt);
#endif

    on_central_ble_evt(p_ble_evt);
    ble_dim_c_on_ble_evt(&m_ble_dim_c, p_ble_evt);
#endif
}

static void ble_lyra_bs_write_event(ble_ss_t * p_ss, ble_gatts_evt_write_t * p_evt_write)
{
    int i;

    if (p_evt_write->len == 0) {
        return;
    }

    for (i = 0; i < p_evt_write->len; i++) {
        printf(" %02d ", p_evt_write->data[i]);
    }
    printf("\n");

    nrf_gpio_pin_toggle(RED_LED);
    if (p_evt_write->data[1] == 0) {
        nrf_gpio_pin_clear(GREEN_LED);
    } else {
        nrf_gpio_pin_set(GREEN_LED);
    }

    dm_store_write_data_evt(p_evt_write->data, p_evt_write->len);
}

uint32_t services_init(void)
{
    uint32_t err_code;
    ble_uuid_t ble_service_uuid;
    ble_uuid_t ble_char_uuid;
    ble_ss_init_t lyra_bs_param;

    // Initialize lyra_bs Service.
    memset(&lyra_bs_param, 0, sizeof(lyra_bs_param));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&lyra_bs_param.sensor_value_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&lyra_bs_param.sensor_value_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&lyra_bs_param.sensor_value_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&lyra_bs_param.sensor_value_report_read_perm);

    lyra_bs_param.evt_write_handler      = NULL;
    lyra_bs_param.evt_auth_write_handler = ble_lyra_bs_write_event;
    lyra_bs_param.evt_auth_read_handler  = NULL;
    lyra_bs_param.support_notification   = true;
    lyra_bs_param.p_report_ref           = NULL;
    lyra_bs_param.initial_value          = 0;

    BLE_UUID_ASSIGN(ble_service_uuid, BLE_UUID_BUTTON_SERVICE);
    BLE_UUID_ASSIGN(ble_char_uuid, BLE_UUID_BUTTON_CHAR);
    err_code = ble_ss_init(&lyra_bs_ss, &ble_service_uuid, &ble_char_uuid, &lyra_bs_param);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

uint8_t ble_button_state = 0;
void ble_advertising_init()
{
    ble_advdata_service_data_t service_data;
    service_data.service_uuid = BLE_UUID_BUTTON_SERVICE;
    service_data.data.size = sizeof(ble_button_state);
    service_data.data.p_data = &ble_button_state;

    ble_advertising_common_init(&service_data);
}
