#include "device.h"

device_t device;
static uint32_t char_number = 0;

static void on_connect(ble_evt_t * p_ble_evt)
{
    device.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}

static void on_disconnect(ble_evt_t * p_ble_evt)
{
    device.conn_handle = BLE_CONN_HANDLE_INVALID;
}

static void on_write(ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    int i;

    for (i = 0; i < DEVICE_CHARS_NUMBER; i++) {
        if ((p_evt_write->handle == device.chars[i].handle) &&
            (p_evt_write->len == 1) &&
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
            on_connect(p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_ble_evt);
            break;

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
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = device.uuid_type;
    ble_uuid.uuid = char_reg.type;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid       = &ble_uuid;
    attr_char_value.p_attr_md    = &attr_md;
    attr_char_value.init_len     = sizeof(uint8_t);
    attr_char_value.init_offs    = 0;
    attr_char_value.max_len      = sizeof(uint8_t);
    attr_char_value.p_value      = NULL;

    error_no = sd_ble_gatts_characteristic_add(device.service_handle, &char_md,
                                               &attr_char_value,
                                               &handle);

    device.chars[char_number].handle = handle.value_handle;
    device.chars[char_number].on_write = char_reg.on_write;
    device.chars[char_number].data = char_reg.data;
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


