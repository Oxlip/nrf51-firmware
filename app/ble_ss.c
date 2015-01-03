/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/**
 * BLE Sensor Service.
 *
 *   Sensor Service module based on Nordic's Sensor Service module.
 */

#include <ble_ss.h>
#include <string.h>
#include <nordic_common.h>
#include <ble_srv_common.h>
#include <app_util.h>
#include <ble_uuids.h>


#define INVALID_SENSOR_VALUE -1


/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_ss        Sensor Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_ss_t * p_ss, ble_evt_t * p_ble_evt)
{
    p_ss->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_ss        Sensor Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_ss_t * p_ss, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_ss->conn_handle = BLE_CONN_HANDLE_INVALID;
}


/**@brief Function for handling the Write event.
 *
 * @param[in]   p_ss        Sensor Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_ss_t * p_ss, ble_evt_t * p_ble_evt)
{
    if (p_ss->is_notification_supported)
    {
        ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

        if (
            (p_evt_write->handle == p_ss->sensor_value_handles.cccd_handle)
            &&
            (p_evt_write->len == 2)
           )
        {
            // CCCD written, call application event handler
            if (p_ss->evt_write_handler != NULL)
            {
                ble_ss_evt_t evt;

                if (ble_srv_is_notification_enabled(p_evt_write->data))
                {
                    evt.evt_type = BLE_SS_EVT_NOTIFICATION_ENABLED;
                }
                else
                {
                    evt.evt_type = BLE_SS_EVT_NOTIFICATION_DISABLED;
                }

                p_ss->evt_write_handler(p_ss, p_evt_write, &evt);
            }
        }
    }
}

static void on_rw_authorize_request(ble_ss_t * p_ss, ble_evt_t * p_ble_evt)
{
    ble_gatts_evt_rw_authorize_request_t * p_authorize_request;
    uint16_t handle;
    uint8_t  is_write;

    ble_gatts_rw_authorize_reply_params_t write_authorize_reply;
    write_authorize_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
    write_authorize_reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;
    sd_ble_gatts_rw_authorize_reply(p_ss->conn_handle, &write_authorize_reply);

    p_authorize_request = &(p_ble_evt->evt.gatts_evt.params.authorize_request);

    if (p_authorize_request->type == BLE_GATTS_AUTHORIZE_TYPE_WRITE)
    {
        handle = p_authorize_request->request.write.handle;
        is_write = 1;
        ble_gatts_rw_authorize_reply_params_t write_authorize_reply;
    
        write_authorize_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
        write_authorize_reply.params.write.gatt_status = BLE_GATT_STATUS_SUCCESS;
        sd_ble_gatts_rw_authorize_reply(p_ss->conn_handle, &write_authorize_reply);

    } else if (p_authorize_request->type == BLE_GATTS_AUTHORIZE_TYPE_READ)
    {
        handle = p_authorize_request->request.read.handle;
        is_write = 0;
        ble_gatts_rw_authorize_reply_params_t read_authorize_reply;
    
        read_authorize_reply.type = BLE_GATTS_AUTHORIZE_TYPE_WRITE;
        read_authorize_reply.params.read.gatt_status = BLE_GATT_STATUS_SUCCESS;
        sd_ble_gatts_rw_authorize_reply(p_ss->conn_handle, &read_authorize_reply);

    } else
    {
        return;
    }

    if (handle != p_ss->sensor_value_handles.cccd_handle)
    {
        return;
    }

    if (is_write)
    {
        if (p_ss->evt_auth_write_handler) 
        {
            p_ss->evt_auth_write_handler(p_ss, &p_authorize_request->request.write);
        }
    }
    else
    {
        if (p_ss->evt_auth_read_handler)
        {
            p_ss->evt_auth_read_handler(p_ss, &p_authorize_request->request.read);
        }
    }
}


void ble_ss_on_ble_evt(ble_ss_t * p_ss, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_ss, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_ss, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_ss, p_ble_evt);
            break;

        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST:
            on_rw_authorize_request(p_ss, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}


/**@brief Function for adding the Sensor Value characteristic.
 *
 * @param[in]   p_ss        Sensor Service structure.
 * @param[in]   p_ble_uuid  UUID of the service.
 * @param[in]   p_ss_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t sensor_value_char_add(ble_ss_t * p_ss, ble_uuid_t * p_ble_uuid, const ble_ss_init_t * p_ss_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_gatts_attr_md_t attr_md;
    uint32_t            initial_sensor_value;
    uint8_t             encoded_report_ref[BLE_SRV_ENCODED_REPORT_REF_LEN];
    uint8_t             init_len;

    // Add Sensor Value characteristic
    if (p_ss->is_notification_supported)
    {
        memset(&cccd_md, 0, sizeof(cccd_md));

        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
        cccd_md.write_perm = p_ss_init->sensor_value_char_attr_md.cccd_write_perm;
        cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
    }

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.notify = (p_ss->is_notification_supported) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = (p_ss->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md         = NULL;

    memset(&attr_md, 0, sizeof(attr_md));

    attr_md.read_perm  = p_ss_init->sensor_value_char_attr_md.read_perm;
    attr_md.write_perm = p_ss_init->sensor_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    initial_sensor_value = p_ss_init->initial_value;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = p_ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint32_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint32_t);
    attr_char_value.p_value   = (uint8_t *)&initial_sensor_value;

    err_code = sd_ble_gatts_characteristic_add(p_ss->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_ss->sensor_value_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    if (p_ss_init->p_report_ref != NULL)
    {
        ble_uuid_t  ble_report_uuid;
        // Add Report Reference descriptor
        BLE_UUID_BLE_ASSIGN(ble_report_uuid, BLE_UUID_REPORT_REF_DESCR);

        memset(&attr_md, 0, sizeof(attr_md));

        attr_md.read_perm = p_ss_init->sensor_value_report_read_perm;
        BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

        attr_md.vloc    = BLE_GATTS_VLOC_STACK;
        attr_md.rd_auth = 0;
        attr_md.wr_auth = 0;
        attr_md.vlen    = 0;
        
        init_len = ble_srv_report_ref_encode(encoded_report_ref, p_ss_init->p_report_ref);
        
        memset(&attr_char_value, 0, sizeof(attr_char_value));

        attr_char_value.p_uuid    = &ble_report_uuid;
        attr_char_value.p_attr_md = &attr_md;
        attr_char_value.init_len  = init_len;
        attr_char_value.init_offs = 0;
        attr_char_value.max_len   = attr_char_value.init_len;
        attr_char_value.p_value   = encoded_report_ref;

        err_code = sd_ble_gatts_descriptor_add(p_ss->sensor_value_handles.value_handle,
                                               &attr_char_value,
                                               &p_ss->report_ref_handle);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }
    }
    else
    {
        p_ss->report_ref_handle = BLE_GATT_HANDLE_INVALID;
    }

    return NRF_SUCCESS;
}


uint32_t ble_ss_init(ble_ss_t * p_ss, ble_uuid_t * p_service_ble_uuid, ble_uuid_t * p_char_ble_uuid, const ble_ss_init_t * p_ss_init)
{
    uint32_t   err_code;

    // Initialize service structure
    p_ss->evt_write_handler         = p_ss_init->evt_write_handler;
    p_ss->evt_auth_write_handler    = p_ss_init->evt_auth_write_handler;
    p_ss->evt_auth_read_handler     = p_ss_init->evt_auth_read_handler;
    p_ss->conn_handle               = BLE_CONN_HANDLE_INVALID;
    p_ss->is_notification_supported = p_ss_init->support_notification;
    p_ss->sensor_value_last         = INVALID_SENSOR_VALUE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, p_service_ble_uuid, &p_ss->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add sensor value characteristic
    return sensor_value_char_add(p_ss, p_char_ble_uuid, p_ss_init);
}


uint32_t ble_ss_sensor_value_update(ble_ss_t * p_ss, uint32_t sensor_value)
{
    uint32_t err_code = NRF_SUCCESS;

    if (sensor_value != p_ss->sensor_value_last)
    {
        uint16_t len = sizeof(sensor_value);

        // Save new sensor value
        p_ss->sensor_value_last = sensor_value;

        // Update database
        err_code = sd_ble_gatts_value_set(p_ss->sensor_value_handles.value_handle,
                                          0,
                                          &len,
                                          (uint8_t *)&sensor_value);
        if (err_code != NRF_SUCCESS)
        {
            return err_code;
        }

        // Send value if connected and notifying
        if ((p_ss->conn_handle != BLE_CONN_HANDLE_INVALID) && p_ss->is_notification_supported)
        {
            ble_gatts_hvx_params_t hvx_params;

            memset(&hvx_params, 0, sizeof(hvx_params));
            len = sizeof(uint8_t);

            hvx_params.handle = p_ss->sensor_value_handles.value_handle;
            hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
            hvx_params.offset = 0;
            hvx_params.p_len  = &len;
            hvx_params.p_data = (uint8_t *)&sensor_value;

            err_code = sd_ble_gatts_hvx(p_ss->conn_handle, &hvx_params);
        }
        else
        {
            err_code = NRF_ERROR_INVALID_STATE;
        }
    }

    return err_code;
}
