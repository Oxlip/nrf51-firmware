/**
 * BLE related macros and functions.
 */

#ifndef __BLE_PLATFORM__
#define __BLE_PLATFORM__

#include <ble_types.h>
#include <ble_advdata.h>
#include <ble_srv_common.h>
#include <ble_advertising.h>

/** @brief Set .type and .uuid fields of ble_uuid_struct to specified uuid value. */
#define BLE_UUID_ASSIGN(instance, value) do {\
            instance.type = oxlip_uuid_type; \
            instance.uuid = value;} while(0)

/** @brief Set .type to BLE_UUID_TYPE_BLE and .uuid fields of ble_uuid_struct to specified uuid value. */
#define BLE_UUID_ASSIGN_TYPE_STD(instance, value) do {\
            instance.type = BLE_UUID_TYPE_BLE; \
            instance.uuid = value;} while(0)

/**< UUID type registered with the SDK */
extern uint8_t oxlip_uuid_type;

void advertising_init(void);
void ble_advertising_service_data_set(ble_advdata_service_data_t *service_data);

#endif