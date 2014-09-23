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


#define UDEVICE_UUID_BASE { 0x0A, 0x95, 0xE8, 0xC0, 0x02, 0x09, 0xA0, 0xAB, 0x85, 0x40, 0x24, 0x93, 0x00, 0x00, 0xF4, 0xC0 }

#define UDEVICE_UUID_SERVICE     0x1000
#define UDEVICE_UUID_INFOS_CHAR  0x1001
#define UDEVICE_UUID_OUTLET_CHAR 0x1002
#define UDEVICE_UUID_SENSOR_CHAR 0x1003

typedef void (*device_on_write)(ble_gatts_evt_write_t *, void *);

typedef struct device_s
{
    uint16_t service_handle;
    uint16_t conn_handle;
    uint8_t uuid_type;
    struct {
        uint16_t        handle;
        void            *data;
        device_on_write on_write;
    } chars[DEVICE_CHARS_NUMBER];
} device_t;

typedef struct char_register_s
{
    uint16_t        type;
    device_on_write on_write;
    void            *data;
} char_register_t;

uint32_t device_init(uint16_t service_uuid);
uint32_t device_add_char(char_register_t char_reg);

#endif // !DEVICE_H_
