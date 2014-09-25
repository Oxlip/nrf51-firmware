/* Copyright (c) 2013 Nordic Semiconductor. All Rights Reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the license.txt file.
 */

#include <string.h>
#include "nordic_common.h"
#include "ble_srv_common.h"
#include "app_util.h"
#include "app_error.h"
#include "device.h"
#include "nrf.h"
#include "boards.h"

extern device_t device;

ble_uuid_t adv_uuid;
ble_uuid_t *service_get_uuids(void)
{
    adv_uuid.uuid = UDEVICE_UUID_SERVICE;
    adv_uuid.type = device.uuid_type;
    return &adv_uuid;
}

static void outlet_notify_power_consume(uint8_t type)
{
    uint8_t data[10];

    switch (type) {

        case 0x1: /* average */
            uint32_encode(1234, data);
            device_notify(OP_CODE_OUTLET_GET_POWER, data, 4, 0);

        case 0x2: /* current */
            uint32_encode(9999, data);
            device_notify(OP_CODE_OUTLET_GET_POWER, data, 4, 0);

        default:
            return;
    }
}

static void uplug_on_write(ble_gatts_evt_write_t *evt, void *data)
{
}

static void infos_on_auth_read(ble_gatts_evt_read_t *evt, void *data)
{
    ble_gatts_rw_authorize_reply_params_t read_authorize_reply;
    read_authorize_reply.type = BLE_GATTS_AUTHORIZE_TYPE_READ;
    read_authorize_reply.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;
    sd_ble_gatts_rw_authorize_reply(device.conn_handle, &read_authorize_reply);
}

static void outlet_on_auth_write(ble_gatts_evt_write_t * p_ble_write_evt, void *data)
{
    uint8_t opcode, dim, type;

    ble_gatts_rw_authorize_reply_params_t write_authorize_reply;
    write_authorize_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
    write_authorize_reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;
    sd_ble_gatts_rw_authorize_reply(device.conn_handle, &write_authorize_reply);

    if (p_ble_write_evt->len == 0)
        return;

    opcode = p_ble_write_evt->data[0];

    switch (opcode) {
        case OP_CODE_OUTLET_SET_DIM:

            if (p_ble_write_evt->len < 2)
                return;

            dim = p_ble_write_evt->data[1];
            if (dim == 0)
                nrf_gpio_pin_clear(LED_1);
            else
                nrf_gpio_pin_set(LED_1);
            break;

        case OP_CODE_OUTLET_GET_POWER:

            if (p_ble_write_evt->len < 2)
                return;

            type = p_ble_write_evt->data[1];
            outlet_notify_power_consume(type);
            break;

        default:
            return;
    }
}



static void uplug_on_connect()
{
}

uint32_t services_init(void)
{
    uint32_t err_code;
    char_register_t char_outlet_reg = {
        .type = UDEVICE_UUID_OUTLET_CHAR,
        /* No need data for the uplug with ble only */
        .data = NULL,
        .on_write = uplug_on_write,
        .on_auth_write  = NULL,
        .on_auth_read  = NULL
    };
    char_register_t char_info_reg = {
        .type = UDEVICE_UUID_INFOS_CHAR,
        /* No need data for the uplug with ble only */
        .data = NULL,
        .on_write = NULL,
        .on_auth_write  = NULL,
        .on_auth_read  = infos_on_auth_read
    };

    device.on_connect = uplug_on_connect;

    device_init(UDEVICE_UUID_SERVICE);

    err_code = device_add_char(char_outlet_reg);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    err_code = device_add_char(char_info_reg);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}
