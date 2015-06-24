/**
 * BLE related macros and functions.
 */

#ifndef __BLE_PLATFORM__
#define __BLE_PLATFORM__

#include <ble_types.h>

/** @brief Set .type and .uuid fields of ble_uuid_struct to specified uuid value. */
#define BLE_UUID_ASSIGN(instance, value) do {\
            instance.type = oxlip_uuid_type; \
            instance.uuid = value;} while(0)

ble_uuid_t * ble_get_adv_uuid_array();
uint8_t ble_get_adv_uuid_array_count();

/**< UUID type registered with the SDK */
extern uint8_t oxlip_uuid_type;

#endif