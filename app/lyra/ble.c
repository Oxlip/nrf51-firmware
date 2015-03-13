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

/** UUIDs to advertise. */
ble_uuid_t adv_uuids[] = {
    {BLE_UUID_BUTTON_SERVICE, BLE_UUID_TYPE_BLE}
};

uint8_t ble_get_adv_uuid_array_count()
{
    return sizeof(adv_uuids) / sizeof(ble_uuid_t);
}

ble_uuid_t * ble_get_adv_uuid_array()
{
    return adv_uuids;
}
