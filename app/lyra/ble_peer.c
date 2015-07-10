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
#include <board_conf.h>

#define SCAN_INTERVAL              0x00A0                             /**< Determines scan interval in units of 0.625 millisecond. */
#define SCAN_WINDOW                0x0050                             /**< Determines scan window in units of 0.625 millisecond. */
#define MIN_CONNECTION_INTERVAL    MSEC_TO_UNITS(50, UNIT_1_25_MS)    /**< Determines maximum connection interval in millisecond. */
#define MAX_CONNECTION_INTERVAL    MSEC_TO_UNITS(100, UNIT_1_25_MS)   /**< Determines maximum connection interval in millisecond. */
#define SLAVE_LATENCY              0                                  /**< Determines slave latency in counts of connection events. */
#define SUPERVISION_TIMEOUT        MSEC_TO_UNITS(4000, UNIT_10_MS)    /**< Determines supervision time-out in units of 10 millisecond. */


/**< Scan parameters requested for scanning and connection. */
static ble_gap_scan_params_t m_scan_param = {
    .active       = 0,            // Active scanning set.
    .selective    = 0,            // Selective scanning not set.
    .interval     = SCAN_INTERVAL,// Scan interval.
    .window       = SCAN_WINDOW,  // Scan window.
    .p_whitelist  = NULL,         // No whitelist provided.
    .timeout      = 0x001E,       // 30 seconds timeout.
};

/**
 * @brief Connection parameters requested for connection.
 */
const ble_gap_conn_params_t m_connection_param =
{
    (uint16_t)MIN_CONNECTION_INTERVAL,   // Minimum connection
    (uint16_t)MAX_CONNECTION_INTERVAL,   // Maximum connection
    0,                                   // Slave latency
    (uint16_t)SUPERVISION_TIMEOUT        // Supervision time-out
};

typedef struct peer_info_ {
    ble_gap_addr_t peer_addr;
    uint8_t buffer[4];                      /* buffer to write to peer device */
    uint8_t buffer_len;                     /* length of the buffer */

    uint8_t curr_state;                     /* current state of the state machine */

    /* start & end handle for services discovered */
    ble_gattc_handle_range_t srv_handle_range;

    uint16_t conn_handle;                   /**< Handle of the current connection. */
    uint8_t  curr_srv_ind;                  /**< Index of the current service being discovered. This is intended for internal use during service discovery.*/
    uint8_t  curr_char_ind;                 /**< Index of the current characteristic being discovered. This is intended for internal use during service discovery.*/
} peer_info_t;

peer_info_t peer_info;                      /* indexed as button * action_index */

#define SRV_DISC_START_HANDLE  0x000b       /**< The start handle value used during service discovery. */

/*
 * This below function should be generic:
 * to take params, peer_addr, buffer & buffer_len
 */
void
write_to_peer_device (ble_gap_addr_t peer_addr, uint8_t *value, uint8_t len)
{
    uint32_t err_code;

    /* Store the value in a location for this peer */
    peer_info.peer_addr = peer_addr;
    memcpy(peer_info.buffer, value, len);
    peer_info.buffer_len = len;

    // Stop scanning.
    err_code = sd_ble_gap_scan_stop();
    if (err_code != NRF_SUCCESS)
    {
        printf("[ADV_REP]: Scan stop failed, reason %ld\n", err_code);
    }

    /* TODO: */
    peer_info.peer_addr.addr[0] = 0xa4;
    peer_info.peer_addr.addr[1] = 0xfb;
    peer_info.peer_addr.addr[2] = 0xd9;
    peer_info.peer_addr.addr[3] = 0x72;
    peer_info.peer_addr.addr[4] = 0x02;
    peer_info.peer_addr.addr[5] = 0x00;
    peer_info.peer_addr.addr_type = BLE_GAP_ADDR_TYPE_PUBLIC;

    /* TODO: set the current state machine to disconnected here */
    peer_info.curr_state = BLE_GAP_EVT_DISCONNECTED;

    err_code = sd_ble_gap_connect(&peer_info.peer_addr,
                                  &m_scan_param, &m_connection_param);
    if (err_code != NRF_SUCCESS)
    {
        printf("[ADV_REP]: Connection Request Failed, reason %ld\n", err_code);
    }
}

/**@brief Function for handling the Application's BLE Stack events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 */
void on_central_ble_evt (ble_evt_t * p_ble_evt)
{
    uint32_t  err_code;
    ble_gattc_evt_prim_srvc_disc_rsp_t *p_prim_srvc_disc_rsp_evt;
    int i;
    ble_gattc_handle_range_t handle_range;
    ble_gattc_evt_char_disc_rsp_t *p_char_disc_rsp_evt;
    ble_gattc_char_t *chars;
    ble_gattc_write_params_t write_params;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:

#if 0
            /* Temporarily comment out peer_addr check */
            /* peer_info.peer_addr = 00:02:72:D9:FB:A4; */
            if (memcmp(&p_ble_evt->evt.gap_evt.params.connected.peer_addr,
                    &peer_info.peer_addr,
                    sizeof(peer_info.peer_addr)) != 0) {
                printf("%s: Not interested in peer: \n", __FUNCTION__);
                for (i = 0; i < 6; i++) {
                    printf(" %#x ", p_ble_evt->evt.gap_evt.params.connected.peer_addr.addr[i]);
                }
                return;
            }

            /* store the current event id: */
            if (peer_info.curr_state != BLE_GAP_EVT_DISCONNECTED) {
                printf("Invalid state: %d\n", peer_info.curr_state);
                return;
            }
#endif

            /* Update the state machine */
            peer_info.curr_state = p_ble_evt->header.evt_id;

            /*
             * Store the connection event handle (for the corresponding button
             * event
             */
            peer_info.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            printf("%s: Got connected event with conn_handle %#x\n",
                    __FUNCTION__, peer_info.conn_handle);

            printf("%s: Starting primary services discovery with handle %d\n",
                    __FUNCTION__, SRV_DISC_START_HANDLE);
            /* Start primary service discovery */
            err_code =
                sd_ble_gattc_primary_services_discover(peer_info.conn_handle,
                                                       SRV_DISC_START_HANDLE, NULL);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP:
            printf("%s: Got primary service discovery response: status %d\n",
                    __FUNCTION__, p_ble_evt->evt.gattc_evt.gatt_status);

            /* store the current event id: */
            if (peer_info.curr_state != BLE_GAP_EVT_CONNECTED) {
                printf("Invalid state: %d\n", peer_info.curr_state);
                return;
            }

            /* Update the state machine only if this is the primary service we
             * are interested in else send out another primary_services_discover with
             * start_handle++.
             */

            /* Update the state machine */
            peer_info.curr_state = p_ble_evt->header.evt_id;

            /* Update state to primary service discovery */
            p_prim_srvc_disc_rsp_evt =
                &p_ble_evt->evt.gattc_evt.params.prim_srvc_disc_rsp;
            for (i = 0; i < p_prim_srvc_disc_rsp_evt->count; i++) {
                printf("%s: Index: %d: uuid: %#x\n", __FUNCTION__,
                        i, p_prim_srvc_disc_rsp_evt->services[0].uuid.uuid);
            }

            peer_info.srv_handle_range = p_prim_srvc_disc_rsp_evt->services[0].handle_range;

            /* Adjust start_handle to get the right characteristic */
            printf("%s: Starting char discovery with start_handle %#x\n",
                    __FUNCTION__, peer_info.srv_handle_range.start_handle);

            /* Look for the primary service we are interested in */
            handle_range.start_handle =
                peer_info.srv_handle_range.start_handle;
            handle_range.end_handle =
                peer_info.srv_handle_range.end_handle;

            //on_primary_srv_discovery_rsp(p_db_discovery, &(p_ble_evt->evt.gattc_evt));
            err_code = sd_ble_gattc_characteristics_discover(peer_info.conn_handle,
                                                             &handle_range);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_CHAR_DISC_RSP:
            p_char_disc_rsp_evt = &(p_ble_evt->evt.gattc_evt.params.char_disc_rsp);
            chars = &p_char_disc_rsp_evt->chars[0];
            printf("%s: Got characteristic discovery response: status %d, count"
                    " %d, UUID: %#x, handle_value %#x\n",
                    __FUNCTION__, p_ble_evt->evt.gattc_evt.gatt_status,
                    p_char_disc_rsp_evt->count,
                    chars->uuid.uuid, chars->handle_value);

            /* store the current event id: */
            if (peer_info.curr_state != BLE_GATTC_EVT_PRIM_SRVC_DISC_RSP) {
                printf("Invalid state: %d\n", peer_info.curr_state);
                return;
            }

            /*
             * TODO: If this is not the characteristic we are interested in, donot
             * update the SM.
             */

            /* Update the state machine */
            peer_info.curr_state = p_ble_evt->header.evt_id;

            printf("%s: Write Params of length %d\n", __FUNCTION__, peer_info.buffer_len);
            handle_range.start_handle =
                p_char_disc_rsp_evt->chars[0].handle_value;
            handle_range.end_handle =
                peer_info.srv_handle_range.end_handle;

            /* Use buffer to send */
            /* send (write) dimmer / button value to peer */
            write_params.write_op = BLE_GATT_OP_WRITE_REQ;
            write_params.handle   = handle_range.start_handle;
            write_params.len      = peer_info.buffer_len;
            write_params.p_value  = (uint8_t *)&peer_info.buffer;
            write_params.offset   = 0;

            /* Write the value to dimmer service */
            err_code = sd_ble_gattc_write(peer_info.conn_handle,
                                          &write_params);
            break;

        case BLE_GATTC_EVT_DESC_DISC_RSP:
            printf("%s: Got desc discovery response: status %d\n",
                    __FUNCTION__, p_ble_evt->evt.gattc_evt.gatt_status);

            /* store the current event id: */
            if (peer_info.curr_state != BLE_GATTC_EVT_CHAR_DISC_RSP) {
                printf("Invalid state: %d\n", peer_info.curr_state);
                return;
            }

            /* Update the state machine */
            peer_info.curr_state = p_ble_evt->header.evt_id;
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            peer_info.conn_handle = BLE_CONN_HANDLE_INVALID;

            /* Update the state machine */
            peer_info.curr_state = p_ble_evt->header.evt_id;
            break;

        default:
            printf("Unknown BLE event %#x\n", p_ble_evt->header.evt_id);
            // No implementation needed.
            break;
    }
}

