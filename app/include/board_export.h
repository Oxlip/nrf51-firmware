/**
 * Functions exported by boards.
 */

#ifndef __BOARD_EXPORT__
#define __BOARD_EXPORT__

#include <ble_types.h>

uint32_t services_init(void);
ble_uuid_t *service_get_uuids(void);
void device_on_ble_evt(ble_evt_t * p_ble_evt);

#endif