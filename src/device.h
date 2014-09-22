#ifndef DEVICE_H_
#define DEVICE_H_

#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "ble_lbs.h"

#ifndef DEVICE_CHARS_NUMBER
#error "DEVICE_CHARS_NUMBER not define"
#endif

#define DEVICE_CHARS_WRITE (1 << 0)
#define DEVICE_CHARS_READ  (1 << 1)

typedef void (*device_on_write)(ble_gatts_evt_write_t *);

typedef struct device_s
{
    uint16_t service_handle;
    uint16_t conn_handle;
    uint8_t uuid_type;
    struct {
        uint16_t        handle;
        device_on_write on_write;
    } chars[DEVICE_CHARS_NUMBER];
} device_t;

uint32_t device_init(uint16_t service_uuid);
uint32_t device_add_char(device_on_write on_write, uint8_t flags);

#endif // !DEVICE_H_
