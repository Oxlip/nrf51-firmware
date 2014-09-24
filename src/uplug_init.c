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

extern device_t device;

ble_uuid_t adv_uuid;
ble_uuid_t *service_get_uuids(void)
{
    adv_uuid.uuid = UDEVICE_UUID_SERVICE;
    adv_uuid.type = device.uuid_type;
    return &adv_uuid;
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

static void uplug_on_auth_write(ble_gatts_evt_write_t *evt, void *data)
{
    ble_gatts_rw_authorize_reply_params_t write_authorize_reply;
    write_authorize_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
    write_authorize_reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;
    sd_ble_gatts_rw_authorize_reply(device.conn_handle, &write_authorize_reply);
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
