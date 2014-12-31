#include <nrf_gpio.h>

#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"

#include <ble_uuids.h>

#ifndef DEVICE_CHARS_NUMBER
#error "DEVICE_CHARS_NUMBER not define"
#endif

#define UDEVICE_UUID_BASE { 0x0A, 0x95, 0xE8, 0xC0, 0x02, 0x09, 0xA0, 0xAB, 0x85, 0x40, 0x24, 0x93, 0x00, 0x00, 0xF4, 0xC0 }

#define UDEVICE_UUID_SERVICE     0x1000
#define UDEVICE_UUID_INFOS_CHAR  0x1001
#define UDEVICE_UUID_OUTLET_CHAR 0x1002
#define UDEVICE_UUID_SENSOR_CHAR 0x1003

#define LED_0 18
#define LED_1 19

typedef enum opcode_outlet_e {
    OP_CODE_OUTLET_SET_DIM   = 0x1,
    OP_CODE_OUTLET_GET_POWER = 0x2
} opcode_outlet_e;

typedef void (*device_on_write)(ble_gatts_evt_write_t *, void *);
typedef void (*device_on_auth_read)(ble_gatts_evt_read_t *, void *);
typedef void (*device_on_auth_write)(ble_gatts_evt_write_t *, void *);

typedef struct device_s
{
    uint16_t service_handle;
    uint16_t conn_handle;
    void (*on_connect)(void);
    uint8_t uuid_type;
    struct {
        uint16_t             handle;
        void                *data;
        device_on_write      on_write;
        device_on_auth_write on_auth_write;
        device_on_auth_read  on_auth_read;
    } chars[DEVICE_CHARS_NUMBER];
} device_t;

typedef struct char_register_s
{
    uint16_t             type;
    device_on_write      on_write;
    device_on_auth_read  on_auth_read;
    device_on_auth_write on_auth_write;
    void                *data;
    uint32_t             index;
} char_register_t;

uint32_t device_init(uint16_t service_uuid);
uint32_t device_add_char(char_register_t char_reg);
uint32_t device_notify(uint8_t opcode, void *data, uint32_t len, uint32_t index);


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
            break;

        case 0x2: /* current */
            uint32_encode(9999, data);
            device_notify(OP_CODE_OUTLET_GET_POWER, data, 4, 0);
            break;

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
        .on_write = outlet_on_auth_write,
        .on_auth_write  = outlet_on_auth_write,
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


device_t device;
static uint32_t char_number = 0;

static void device_on_connect(ble_evt_t * p_ble_evt)
{
    device.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

static void device_on_disconnect(ble_evt_t * p_ble_evt)
{
    device.conn_handle = BLE_CONN_HANDLE_INVALID;
}

static void device_on_rw(ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_rw_authorize_request_t * p_authorize_request;
    uint16_t handle;
    uint8_t  is_write;
    uint32_t i;

    p_authorize_request = &(p_ble_evt->evt.gatts_evt.params.authorize_request);

    if (p_authorize_request->type == BLE_GATTS_AUTHORIZE_TYPE_WRITE) {
        handle = p_authorize_request->request.write.handle;
        is_write = 1;
    } else if (p_authorize_request->type == BLE_GATTS_AUTHORIZE_TYPE_READ) {
        handle = p_authorize_request->request.read.handle;
        is_write = 0;
    } else {
        return;
    }

    for (i = 0; i < DEVICE_CHARS_NUMBER; i++) {
        if (handle == device.chars[i].handle)
        {
            if (is_write && device.chars[i].on_auth_write) {
                device.chars[i].on_auth_write(&(p_authorize_request->request.write),
                                              device.chars[i].data);
            } else if (!is_write && device.chars[i].on_auth_read) {
                device.chars[i].on_auth_read(&(p_authorize_request->request.read),
                                             device.chars[i].data);
            }
        }
    }
}

static void on_write(ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    int i;

    for (i = 0; i < DEVICE_CHARS_NUMBER; i++) {
        if ((p_evt_write->handle == device.chars[i].handle) &&
            device.chars[i].on_write != NULL)
        {
            device.chars[i].on_write(p_evt_write, device.chars[i].data);
        }
    }
}


void device_on_ble_evt(ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            device_on_connect(p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            device_on_disconnect(p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_ble_evt);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            device_on_rw(p_ble_evt);
        default:
            // No implementation needed.
            break;
    }
}



uint32_t device_add_char(char_register_t char_reg)
{
    ble_gatts_char_md_t       char_md;
    ble_gatts_attr_t          attr_char_value;
    ble_uuid_t                ble_uuid;
    ble_gatts_attr_md_t       attr_md;
    uint32_t                  error_no = 0;
    ble_gatts_char_handles_t  handle;

    memset(&char_md, 0, sizeof(char_md));


    char_md.char_props.write  = 1;
    char_md.char_props.read   = 1;
    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = device.uuid_type;
    ble_uuid.uuid = char_reg.type;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = 0;
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = BLE_L2CAP_MTU_DEF;
    attr_char_value.p_value      = NULL;

    error_no = sd_ble_gatts_characteristic_add(device.service_handle, &char_md,
                                               &attr_char_value,
                                               &handle);

    device.chars[char_number].handle = handle.value_handle;
    device.chars[char_number].on_write = char_reg.on_write;
    device.chars[char_number].data = char_reg.data;

    char_reg.index = char_number;

    char_number += 1;

    return error_no;
}

uint32_t device_init(uint16_t service_uuid)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    device.conn_handle = BLE_CONN_HANDLE_INVALID;
    char_number = 0;

    // Add service
    ble_uuid128_t base_uuid = LBS_UUID_BASE;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &device.uuid_type);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    ble_uuid.type = device.uuid_type;
    ble_uuid.uuid = service_uuid;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &device.service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    return NRF_SUCCESS;
}

static uint8_t m_notif_buffer[256];
uint32_t device_notify(uint8_t opcode, void *data, uint32_t len, uint32_t char_index)
{
    ble_gatts_hvx_params_t hvx_params;
    uint16_t               index = 0;
    uint32_t               err_code;

    m_notif_buffer[index++] = 0x01;

    memcpy(&m_notif_buffer[index], data, len);
    index += len;

    memset(&hvx_params, 0, sizeof(hvx_params));

    hvx_params.handle = device.chars[char_index].handle;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
    hvx_params.offset = 0;
    hvx_params.p_len  = &index;
    hvx_params.p_data = m_notif_buffer;

    err_code = sd_ble_gatts_hvx(device.conn_handle, &hvx_params);

    return err_code;
}
