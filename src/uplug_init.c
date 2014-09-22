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
    adv_uuid.uuid = LBS_UUID_UPLUG_SERVICE;
    adv_uuid.type = device.uuid_type;
    return &adv_uuid;
}

static void on_write(ble_gatts_evt_write_t *evt)
{
}

uint32_t services_init(void)
{
    uint32_t err_code;

    device_init(LBS_UUID_UPLUG_SERVICE);

    err_code = device_add_char(on_write, DEVICE_CHARS_WRITE);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}
