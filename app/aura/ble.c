#include <string.h>
#include <stdio.h>
#include <nrf_gpio.h>
#include <ble.h>
#include <app_util.h>
#include <app_error.h>

#include <ble_uuids.h>
#include <ble_ss.h>
#include <ble_common.h>
#include <app_timer.h>

#include "board_conf.h"
#include "smbus.h"
#include "sensor.h"

ble_ss_t dimmer_ss;
ble_ss_t cs_ss;

/** UUIDs to advertise. */
ble_uuid_t adv_uuids[] = {
    {BLE_UUID_DIMMER_SERVICE, BLE_UUID_TYPE_BLE},
    {BLE_UUID_CS_SERVICE, BLE_UUID_TYPE_BLE}
};

uint8_t ble_get_adv_uuid_array_count()
{
    return sizeof(adv_uuids) / sizeof(ble_uuid_t);
}

ble_uuid_t * ble_get_adv_uuid_array()
{
    return adv_uuids;
}


void device_on_ble_evt(ble_evt_t * p_ble_evt)
{
    ble_ss_on_ble_evt(&dimmer_ss, p_ble_evt);
    ble_ss_on_ble_evt(&cs_ss, p_ble_evt);
}


static void ble_dimmer_write_event(ble_ss_t * p_ss, ble_gatts_evt_write_t * p_evt_write) 
{
    printf("Dimmer percentage %d\n\r", p_evt_write->data[1]);
    if (p_evt_write->len == 0)
    {
        return;
    }

    if (p_evt_write->data[1] == 0)
    {
        nrf_gpio_pin_clear(AURA_TOUCH_LED);
        nrf_gpio_pin_clear(AURA_TRIAC_ENABLE);
    }
    else
    {
        nrf_gpio_pin_set(AURA_TOUCH_LED);
        nrf_gpio_pin_set(AURA_TRIAC_ENABLE);
    }
}

uint32_t services_init(void)
{
    uint32_t err_code;
    ble_uuid_t ble_service_uuid;
    ble_uuid_t ble_char_uuid;
    ble_ss_init_t dimmer_param, cs_param;

    // Initialize Dimmer Service.
    memset(&dimmer_param, 0, sizeof(dimmer_param));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dimmer_param.sensor_value_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dimmer_param.sensor_value_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dimmer_param.sensor_value_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&dimmer_param.sensor_value_report_read_perm);

    dimmer_param.evt_write_handler      = NULL;
    dimmer_param.evt_auth_write_handler = ble_dimmer_write_event;
    dimmer_param.evt_auth_read_handler  = NULL;
    dimmer_param.support_notification   = true;
    dimmer_param.p_report_ref           = NULL;
    dimmer_param.initial_value          = 0;

    BLE_UUID_ASTRAL_ASSIGN(ble_service_uuid, BLE_UUID_DIMMER_SERVICE);
    BLE_UUID_ASTRAL_ASSIGN(ble_char_uuid, BLE_UUID_DIMMER_CHAR);
    err_code = ble_ss_init(&dimmer_ss, &ble_service_uuid, &ble_char_uuid, &dimmer_param);
    APP_ERROR_CHECK(err_code);

    // Initialize Current Sensor Service.
    memset(&cs_param, 0, sizeof(cs_param));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cs_param.sensor_value_char_attr_md.cccd_write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cs_param.sensor_value_char_attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&cs_param.sensor_value_char_attr_md.write_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cs_param.sensor_value_report_read_perm);

    cs_param.evt_write_handler      = NULL;
    cs_param.evt_auth_write_handler = NULL;
    cs_param.evt_auth_read_handler  = NULL;
    cs_param.support_notification   = true;
    cs_param.p_report_ref           = NULL;
    cs_param.initial_value          = 0;

    BLE_UUID_ASTRAL_ASSIGN(ble_service_uuid, BLE_UUID_CS_SERVICE);
    BLE_UUID_ASTRAL_ASSIGN(ble_char_uuid, BLE_UUID_CS_CHAR);
    err_code = ble_ss_init(&cs_ss, &ble_service_uuid, &ble_char_uuid, &cs_param);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}
